#include "../ccc_sdk/ccc_dk_type.h"
#include "../ccc_sdk/ccc_dk.h"
#include "../ccc_sdk/ccc_timer.h"
#include "../ccc_util/ccc_dk_util.h"
#include "../ccc_util/ccc_dk_func.h"
#include "../ccc_msg/ccc_dk_ble_msg.h"
#include "../ccc_protocol/ccc_can.h"
#include "../ccc_protocol/ccc_apdu.h"
#include "../ccc_flow/ccc_dk_flow_owner_pairing.h"
#include "ccc_dk_ble_msg_uwb_ranging_service.h"
#include "ccc_dk_ble_msg_event_notification.h"
#include "../../../aEM/EM00040101_log/EM000401.h"


const u8 CHAP_PER_SLOT[8] = {3,4,6,8,9,12,24,3};
const u8 NUMBER_SLOTS_PER_RUND[12] = {6,8,9,12,16,18,24,32,36,48,72,96};

CCCErrCode_e ranging_capability_request(u8 connId)
{
    u8 msgData[100];
    u16 msgDataLen = 0;

    //Supported_DK_Protocol_Version_Len + Supported_DK_Protocol_Version
    msgData[msgDataLen++] = 0x02;
    msgData[msgDataLen++] = 0x01;
    msgData[msgDataLen++] = 0x00;

    //Supported_UWB_Config_Id_Len + Supported_UWB_Config_Id
    core_mm_copy(msgData + msgDataLen, vehicleInfo.supportedUWBConfigIdLV, (1 + vehicleInfo.supportedUWBConfigIdLV[0]));
    msgDataLen += (1 + vehicleInfo.supportedUWBConfigIdLV[0]);

    core_mm_copy(msgData + msgDataLen, vehicleInfo.supportedPulseShapeComboLV, (1 + vehicleInfo.supportedPulseShapeComboLV[0]));
    msgDataLen += (1 + vehicleInfo.supportedPulseShapeComboLV[0]);

    ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_UWB_RANGING_SERVICE, DK_MESSAGE_ID_RANGING_CAPABILITY_RQ, msgData, msgDataLen);

    devicesInfo[connId].subProcessOn = RANGING_CAPABILITY_REQUEST;
    return CCC_ERR_CODE_SUCCESS;
}

CCCErrCode_e uwb_ranging_service_ranging_capability_response_process(u8 connId, u8* data, u16 len)
{
    core_mm_copy(devicesInfo[connId].rangingSession.selectedDKProtocolVersion, data, 0x02);
    core_mm_copy(devicesInfo[connId].rangingSession.selectedUWBConfigId, data + 2, 0x02);
    devicesInfo[connId].rangingSession.selectedPulseShapeCombo = data[4];
    devicesInfo[connId].rangingSession.sessionStatus = RANGING_SESSION_IDLE;

    if(ccc_dk_get_active_ranging_session_num() > 0)
    {
        LOG_L_S(CCC_MD, "ranging session expaired!!!!\r\n");
        //不允许两个以上的ranging同时测距，本次ranging仍然状态不变为，pri-derived;
    	send_ranging_session_status_changed_subevent_in_dk_event_notification(connId, Session_Status_Ranging_session_conflict);

        return CCC_ERR_CODE_RANGING_CONFLICT;
    }
    else
    {
        //Procedure 1: Bluetooth LE Timesync Triggered by Vehicle
        LOG_L_S(CCC_MD, "timeSyncTriggerConditions=1\r\n");
        devicesInfo[connId].timeSyncTriggerConditions = 1;  //Procedure 1
        cccConfig->eventNotice(connId, SDK_EVENT_TIME_SYNC_TRIGGER_BY_VEHICLE, data, 0);
        devicesInfo[connId].processOn = PROCESS_TIME_SYNC;

        ccc_timer_start(connId, TIMER_ID_RANGING_SET_UP, TIMER_TYPE_ONCE, TIMEOUT_MS_ON_TIME_SYNC, (pFunc_timer_handler)ccc_timer_handler_on_time_sync_on_procedure1);

        return CCC_ERR_CODE_SUCCESS;
    }
}

CCCErrCode_e uwb_ranging_service_ranging_session_request(u8 connId)
{
    u8 msgData[100];
    u16 msgDataLen = 0;

    core_mm_copy(msgData + msgDataLen, devicesInfo[connId].rangingSession.selectedDKProtocolVersion, 0x02);
    msgDataLen += 2;

    core_mm_copy(msgData + msgDataLen, devicesInfo[connId].rangingSession.selectedUWBConfigId, 0x02);
    msgDataLen += 2;

    //UWB_Session_Id The least significant 4 bytes of the transaction_identifier field
    core_mm_copy(msgData + msgDataLen, devicesInfo[connId].rangingSession.uwbSessionId, 0x04);
    msgDataLen += 4;

    msgData[msgDataLen++] = devicesInfo[connId].rangingSession.selectedPulseShapeCombo;

    msgData[msgDataLen++] = vehicleInfo.supportedChannelBitmask;  // ccc_dk_func_get_uwb_channel_bitmask();

    ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_UWB_RANGING_SERVICE, DK_MESSAGE_ID_RANGING_SESSION_RQ, msgData, msgDataLen);

    devicesInfo[connId].processOn = PROCESS_SECURE_RANGING_SETUP_FLOW;

    return CCC_ERR_CODE_SUCCESS;
}

/**
 * @brief 
 *  由车端跟手机端支持的slot bit mask找一个最小的两者都支持的Nchap_per_slot；
 * @param deviceSlotBitMask 
 * Bitmap of supported values of Slot durations as a multiple of TChap, NChap_per_Slot as defined in Section 20.5.2. 
 * Each “1” in this bit map corresponds to a specific value of NChap_per_Slot where:
 * 0x01 = “3”
 * 0x02 = “4”
 * 0x04= “6”
 * 0x08 =“8”
 * 0x10 =“9”
 * 0x20 = “12”
 * 0x40 = “24”
 * and 0x80 is reserved. Note that 0x40 is used for testing only
 * @param vehicleSlotBitMask : 同上;
 * @return u8: 最小的两者都支持的Nchap_per_slot
 */
static u8 uwb_ranging_select_chap_per_slot_by_slot_bitMask(u8 deviceSlotBitMask, u8 vehicleSlotBitMask)
{
    u8 i = 0;
    u8 temp = 1;

    for(i = 0; i < 8; i++)
    {
        temp = 1<<i;

        if( (deviceSlotBitMask & temp != 0x00) && (vehicleSlotBitMask & temp != 0x00))
        {
            break;
        }
    }

    return CHAP_PER_SLOT[i];
}

/**
 * @brief 
 * 
 * @param deviceHop 
 *  [b7 b6 b5] bitmask of hopping modes, the device offers to use in this ranging session
 * 100 No Hopping
 * 010 Continuous Hopping
 * 001 Adaptive Hopping
 * The mandatory mode is 010 corresponding to Continuous Hopping mode. The support of 100 or 001 is optional.
 * [b4 b3 b2 b1 b0] bit mask of hopping sequences the device offers to use in this ranging session.
 * b4=1 is always set since the default hopping sequence and support thereof is mandatory and defined in Appendix A.
 * b3=1 is set when the optional AES-based hopping sequence, which is defined in 23Appendix F, is supported.
 * All remaining bits reserved for future hopping sequences.
 * @param vehicleHop: 同上
 * @return u8 协商出来的hoppingConfig:
 *  优先级: continuous hopping with default hopping sequence > continuous hopping with AES-based optional hopping sequence 
 *  > adaptive hopping with default hopping sequence > adaptive hopping with AES-based optional hopping sequence > no hopping
 * 
 */
static HoppingConfigSelected_e uwb_ranging_select_hopping_config_bitmask(u8 deviceHop, u8 vehicleHop)
{
    if( ((deviceHop & BIT_CONTINUOUS_HOPPING) != 0x00) && ((vehicleHop & BIT_CONTINUOUS_HOPPING) != 0x00))
    {
        //The mandatory mode is 010 corresponding to Continuous Hopping mode. The support of 100 or 001 is optional.
       
        //两者同时支持 continuous hopping
        if( ((deviceHop & BIT_DEFAULT_HOPPING_SEQUENCE) != 0x00) && ((vehicleHop & BIT_DEFAULT_HOPPING_SEQUENCE) != 0x00))
        {
            return CONTINUOUS_HOPPING_WITH_DEFAULT_HOPPING_SEQUENCE;
        }
        else if(((deviceHop & BIT_AES_BASED_HOPPING_SEQUECE) != 0x00) && ((vehicleHop & BIT_AES_BASED_HOPPING_SEQUECE) != 0x00))
        {
            return CONTINUOUS_HOPPING_WITH_AES_BASED_OPTIONAL_HOPPING_SEQUENCE;
        }
        else
        {
            //两者不同时支持CONTINUOUS hopping
            if(((deviceHop & BIT_ADAPTIVE_HOPPING) != 0x00) && ((vehicleHop & BIT_ADAPTIVE_HOPPING) != 0x00))
            {
                //两者同时支持adaptive hopping
                if( ((deviceHop & BIT_DEFAULT_HOPPING_SEQUENCE) != 0x00) && ((vehicleHop & BIT_DEFAULT_HOPPING_SEQUENCE) != 0x00))
                {
                    return ADAPTIVE_HOPPING_WITH_DEFAULT_HOPPING_SEQUENCE;
                }
                else if(((deviceHop & BIT_AES_BASED_HOPPING_SEQUECE) != 0x00) && ((vehicleHop & BIT_AES_BASED_HOPPING_SEQUECE) != 0x00))
                {
                    return ADAPTIVE_HOPPING_WITH_AES_BASED_OPTIONAL_HOPPING_SEQUENCE;
                }
                else
                {
                    if(((deviceHop & BIT_NO_HOPPING) != 0x00) && ((vehicleHop & BIT_NO_HOPPING) != 0x00))
                    {
                        return NO_HOPPING;
                    }
                }
            }
            else
            {
                //两者不同时支持adaptive hopping
                if(((deviceHop & BIT_NO_HOPPING) != 0x00) && ((vehicleHop & BIT_NO_HOPPING) != 0x00))
                {
                    return NO_HOPPING;
                }
            }
        }
    }
    else if(((deviceHop & BIT_ADAPTIVE_HOPPING) != 0x00) && ((vehicleHop & BIT_ADAPTIVE_HOPPING) != 0x00))
    {
        //两者同时支持adaptive hopping
        if( ((deviceHop & BIT_DEFAULT_HOPPING_SEQUENCE) != 0x00) && ((vehicleHop & BIT_DEFAULT_HOPPING_SEQUENCE) != 0x00))
        {
            return ADAPTIVE_HOPPING_WITH_DEFAULT_HOPPING_SEQUENCE;
        }
        else if(((deviceHop & BIT_AES_BASED_HOPPING_SEQUECE) != 0x00) && ((vehicleHop & BIT_AES_BASED_HOPPING_SEQUECE) != 0x00))
        {
            return ADAPTIVE_HOPPING_WITH_AES_BASED_OPTIONAL_HOPPING_SEQUENCE;
        }
        else
        {
            if(((deviceHop & BIT_NO_HOPPING) != 0x00) && ((vehicleHop & BIT_NO_HOPPING) != 0x00))
            {
                return NO_HOPPING;
            }
        }
    }
    else if(((deviceHop & BIT_NO_HOPPING) != 0x00) && ((vehicleHop & BIT_NO_HOPPING) != 0x00))
    {
        return NO_HOPPING;
    }

    return HOPPING_NEGOTIATION_FAILED;
}

//Number_Slots_per_Round : responder-device selects the number of slots that is greater than or equal toNResponder k + 4 ( )out of all possible values of slots
static u8 uwb_ranging_get_slot_per_round(u8 responderNum)
{
    u8 num = responderNum + 4;
    u8 i = 0;

    for(i = 0; i < 12; i++)
    {
        if(NUMBER_SLOTS_PER_RUND[i] >= num)
        {
            return NUMBER_SLOTS_PER_RUND[i];
        }
    }

    return num;
}

CCCErrCode_e uwb_ranging_service_ranging_session_setup_request(u8 connId)
{
    u8 msgData[100];
    u16 msgDataLen = 0;

    msgData[msgDataLen++] = devicesInfo[connId].rangingSession.selectedRANMultiplier;
    
    msgData[msgDataLen++] = devicesInfo[connId].rangingSession.selectedNChapPerSlot;

    msgData[msgDataLen++] = devicesInfo[connId].rangingSession.rangingAnchorNum;

    devicesInfo[connId].rangingSession.selectedNumSlotsPerRound = uwb_ranging_get_slot_per_round(devicesInfo[connId].rangingSession.rangingAnchorNum);
    msgData[msgDataLen++] = devicesInfo[connId].rangingSession.selectedNumSlotsPerRound;

    core_mm_copy(msgData + msgDataLen, vehicleInfo.supportedSYNCCodeIndex, 0x04);
    msgDataLen += 0x04;

    msgData[msgDataLen++] = devicesInfo[connId].rangingSession.selectedHoppingConfigBitmask;

    ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_UWB_RANGING_SERVICE, DK_MESSAGE_ID_RANGING_SESSION_SETUP_RQ, msgData, msgDataLen);

    //devicesInfo[connId].processOn = PROCESS_SECURE_RANGING_SETUP_FLOW;

    return CCC_ERR_CODE_SUCCESS;
}

CCCErrCode_e uwb_ranging_service_ranging_session_response_process(u8 connId, u8* data, u16 len)
{
    if(len < 8)
    {
        return CCC_ERR_CODE_DATA_ERR;
    }

    HoppingConfigSelected_e hoppingSelected;

    //TODO
    //通过串口将参数传给UWB模块，响应怎么获取?
    //Value selected by vehicle must be greater than or equal to RAN_Multiplier sent by mobile device
    devicesInfo[connId].rangingSession.selectedRANMultiplier = data[0];  //RAN_Multiplier

    devicesInfo[connId].rangingSession.selectedNChapPerSlot = uwb_ranging_select_chap_per_slot_by_slot_bitMask(data[1], vehicleInfo.supprtedSlotBitMask);

    //SYNC_Code_Index_BitMask应该不用处理吧；

    devicesInfo[connId].rangingSession.selectedUWBChannel = data[6];

    hoppingSelected = uwb_ranging_select_hopping_config_bitmask(data[7], vehicleInfo.supportedHoppingConfigBitmask);

    if(hoppingSelected != HOPPING_NEGOTIATION_FAILED)
    {
        devicesInfo[connId].rangingSession.selectedHoppingConfigBitmask = hoppingSelected;

        uwb_ranging_service_ranging_session_setup_request(connId);
        return CCC_ERR_CODE_SUCCESS;
    }
    else
    {
        send_ranging_session_status_changed_subevent_in_dk_event_notification(connId, Session_Status_Ranging_session_secure_ranging_failed);
        return CCC_ERR_CODE_UWB_SETUP_FAILED;
    }
}



CCCErrCode_e uwb_ranging_service_ranging_session_setup_response_process(u8 connId, u8* data, u16 len)
{
    if(len < 17)
    {
        return CCC_ERR_CODE_DATA_ERR;
    }

    core_mm_copy(devicesInfo[connId].rangingSession.stsIndex0, data, 0x04);
    core_mm_copy(devicesInfo[connId].rangingSession.uwbTime0, data + 0x04, 0x08);
    core_mm_copy(devicesInfo[connId].rangingSession.hopModeKey, data + 12, 0x04);

    devicesInfo[connId].rangingSession.selectedSyncCodeIndex = data[16];

    //TODO 
    //ccc_dk_set_ranging_session_status(connId, devicesInfo[connId].rangingSession.uwbSessionId, URSK_STATUS_ACTIVE);
    if(ccc_dk_get_active_ranging_session_num() > 0)
    {
        //不允许两个以上的ranging同时测距，本次ranging仍然状态不变为，pri-derived;
        send_ranging_session_status_changed_subevent_in_dk_event_notification(connId, Session_Status_Ranging_session_conflict);
         
        return CCC_ERR_CODE_RANGING_CONFLICT;
    }
    ccc_can_send_uwb_ranging_session_setup_rq(connId);
    OSA_TimeDelay(100);
    ccc_can_send_uwb_ranging_session_start_rq(connId);

    return CCC_ERR_CODE_SUCCESS;
}

CCCErrCode_e uwb_ranging_service_ranging_session_suspend_request(u8 connId)
{
    u8 msgData[0x10];
    u16 msgDataLen = 0;

    core_mm_copy(msgData + msgDataLen, devicesInfo[connId].rangingSession.uwbSessionId, 0x04);
    msgDataLen += 4;

    ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_UWB_RANGING_SERVICE, DK_MESSAGE_ID_RANGING_SUSPEND_RQ, msgData, msgDataLen);

    return CCC_ERR_CODE_SUCCESS;
}

CCCErrCode_e uwb_ranging_service_ranging_session_suspend_response(u8 connId, u8 supendResp)
{
    u8 msgData[100];
    u16 msgDataLen = 0;

    msgData[msgDataLen++] = supendResp;

    ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_UWB_RANGING_SERVICE, DK_MESSAGE_ID_RANGING_SUSPEND_RS, msgData, msgDataLen);

    //devicesInfo[connId].processOn = PROCESS_SECURE_RANGING_SETUP_FLOW;

    return CCC_ERR_CODE_SUCCESS;
}

/**
 * @brief 车端处理device发起的suspend请求;
 * 
 * @param connId 
 * @param data 
 * @param len 
 * @return CCCErrCode_e 
 */
CCCErrCode_e uwb_ranging_suspend_request_message_process(u8 connId, u8* data, u16 len)
{
    u8 acceptFlag = 0x00;

    if(len < 4)
    {
        return CCC_ERR_CODE_DATA_ERR;
    }

    if(devicesInfo[connId].rangingSession.sessionStatus != RANGING_SESSION_ACTIVE)
    {
        return CCC_ERR_CODE_ILLEGAL_STATE;
    }

    acceptFlag = ccc_dk_func_ranging_session_suspend_permit_check(connId, data);
    if(acceptFlag == 0)  //accept
    {
        LOG_L_S(CCC_MD, "TERMINATED2222222222222222222\r\n");
        ccc_can_send_uwb_ranging_session_suspend_recover_delete_rq(connId, RANGING_ACTION_SUSPEND);  //suspend
        ccc_dk_set_ranging_session_status(connId, data, URSK_STATUS_SUSPENDED);

        uwb_ranging_service_ranging_session_suspend_response(connId, 0x00);  //accept
    }
    else if(acceptFlag == 1)  //- To delay the suspend and keep ranging active
    {
        uwb_ranging_service_ranging_session_suspend_response(connId, 0x01);  //delay
    }
    else
    {
        return CCC_ERR_CODE_ACTION_NOT_PERMIT;
    }

    return CCC_ERR_CODE_SUCCESS;
}

/**
 * @brief 车端发起suspend请求，device端响应接受还是delay 
 * 
 * @param connId 
 * @param data 
 * @param len 
 * @return CCCErrCode_e 
 */
CCCErrCode_e uwb_ranging_suspend_response_message_proccess(u8 connId, u8* data, u16 len)
{
    if(data[0] == 0x00)  //0x00 - For accepted suspend request
    {
        LOG_L_S(CCC_MD, "TERMINATED33333333333\r\n");
        ccc_can_send_uwb_ranging_session_suspend_recover_delete_rq(connId, RANGING_ACTION_SUSPEND);  //suspend
        ccc_dk_set_ranging_session_status(connId, data, URSK_STATUS_SUSPENDED);
    }
    else if(data[1] == 0x01) //0x01 - To delay the suspend and keep ranging active
    {
        return CCC_ERR_CODE_SUCCESS;
    }
    else
    {
        return CCC_ERR_CODE_DATA_ERR;
    }
}

//This message is sent by vehicle to recover a suspended ranging session for a given UWB_Session_Id.
CCCErrCode_e uwb_ranging_service_ranging_session_recover_request(u8 connId)
{
    u8 msgData[100];
    u16 msgDataLen = 0;

    if(devicesInfo[connId].rangingSession.sessionStatus != RANGING_SESSION_SUSPEND)
    {
        return CCC_ERR_CODE_ILLEGAL_STATE;
    }

    core_mm_copy(msgData + msgDataLen, devicesInfo[connId].rangingSession.uwbSessionId, 0x04);
    msgDataLen += 4;

    ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_UWB_RANGING_SERVICE, DK_MESSAGE_ID_RANGING_RECOVERY_RQ, msgData, msgDataLen);

    //devicesInfo[connId].processOn = PROCESS_SECURE_RANGING_SETUP_FLOW;

    return CCC_ERR_CODE_SUCCESS;
}

CCCErrCode_e uwb_ranging_service_ranging_session_recover_response_process(u8 connId, u8* data, u16 len)
{
    if(len < 0x0C)
    {
        return CCC_ERR_CODE_DATA_ERR;
    }

    if(devicesInfo[connId].rangingSession.sessionStatus == RANGING_SESSION_SUSPEND)
    {
        core_mm_copy(devicesInfo[connId].rangingSession.stsIndex0, data, 0x04);
        core_mm_copy(devicesInfo[connId].rangingSession.uwbTime0, data + 0x04, 0x08);
        
        ccc_can_send_uwb_ranging_session_suspend_recover_delete_rq(connId, RANGING_ACTION_RECOVER);  //recover rq;

        //ccc_dk_set_ranging_session_status(connId, devicesInfo[connId].rangingSession.uwbSessionId, URSK_STATUS_ACTIVE);    
        return CCC_ERR_CODE_SUCCESS;
    }
    else
    {
        return CCC_ERR_CODE_ILLEGAL_STATE;
    }
}

//This message is sent by vehicle to recover a suspended ranging session for a given UWB_Session_Id.
CCCErrCode_e uwb_ranging_service_configurable_ranging_session_recover_request(u8 connId, u8 RANMultiplier)
{
    u8 msgData[100];
    u16 msgDataLen = 0;

    if(devicesInfo[connId].rangingSession.sessionStatus != RANGING_SESSION_SUSPEND)
    {
        return CCC_ERR_CODE_ILLEGAL_STATE;
    }

    core_mm_copy(msgData + msgDataLen, devicesInfo[connId].rangingSession.uwbSessionId, 0x04);
    msgDataLen += 4;

    msgData[msgDataLen++] = RANMultiplier;

    ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_UWB_RANGING_SERVICE, DK_MESSAGE_ID_RANGING_CONFIGURABLE_RECOVERY_RQ, msgData, msgDataLen);

    //devicesInfo[connId].processOn = PROCESS_SECURE_RANGING_SETUP_FLOW;

    return CCC_ERR_CODE_SUCCESS;
}

CCCErrCode_e uwb_ranging_service_configurabel_ranging_session_recover_response_process(u8 connId, u8* data, u16 len)
{
    if(len < 0x0D)
    {
        return CCC_ERR_CODE_DATA_ERR;
    }

    if(devicesInfo[connId].rangingSession.sessionStatus == RANGING_SESSION_SUSPEND)
    {
        devicesInfo[connId].rangingSession.selectedRANMultiplier = data[0];
        core_mm_copy(devicesInfo[connId].rangingSession.stsIndex0, data + 1, 0x04);
        core_mm_copy(devicesInfo[connId].rangingSession.uwbTime0, data + 0x05, 0x08);

        ccc_can_send_uwb_ranging_session_suspend_recover_delete_rq(connId, RANGING_ACTION_CONFIGURABLE_RECOVER);  //configurable recover rq;

        //ccc_dk_set_ranging_session_status(connId, devicesInfo[connId].rangingSession.uwbSessionId, URSK_STATUS_ACTIVE);
        return CCC_ERR_CODE_SUCCESS;
    }
    else
    {
        return CCC_ERR_CODE_ILLEGAL_STATE;
    }
}

CCCErrCode_e ccc_dk_ble_msg_uwb_ranging_service_msg_process(u8 connId, CCCDkMsg_t * cccDkMsg_t)
{
    CCCErrCode_e errCode = CCC_ERR_CODE_SUCCESS;
    u16 dataLen = ccc_dk_msg_get_data_len(cccDkMsg_t);

    switch (cccDkMsg_t->payload_header_MsgId)
    {
        case DK_MESSAGE_ID_RANGING_CAPABILITY_RS:
            errCode = uwb_ranging_service_ranging_capability_response_process(connId, cccDkMsg_t->data, dataLen);
            break;
        case DK_MESSAGE_ID_RANGING_SESSION_RS:
            errCode = uwb_ranging_service_ranging_session_response_process(connId, cccDkMsg_t->data, dataLen);
            break;
        case DK_MESSAGE_ID_RANGING_SESSION_SETUP_RS:
            errCode = uwb_ranging_service_ranging_session_setup_response_process(connId, cccDkMsg_t->data, dataLen);
            break;
        case DK_MESSAGE_ID_RANGING_SUSPEND_RQ:  //device发起ranging suspend
            errCode = uwb_ranging_suspend_request_message_process(connId, cccDkMsg_t->data, dataLen);
            break;
        case DK_MESSAGE_ID_RANGING_SUSPEND_RS:  //vehicle 发起ranging suspend
            errCode = uwb_ranging_suspend_response_message_proccess(connId, cccDkMsg_t->data, dataLen);
            break;
        case DK_MESSAGE_ID_RANGING_RECOVERY_RS:
            errCode = uwb_ranging_service_ranging_session_recover_response_process(connId, cccDkMsg_t->data, dataLen);
            break;
        case DK_MESSAGE_ID_RANGING_CONFIGURABLE_RECOVERY_RS:
            errCode = uwb_ranging_service_configurabel_ranging_session_recover_response_process(connId, cccDkMsg_t->data, dataLen);
            break;
    }

    return errCode;
}
