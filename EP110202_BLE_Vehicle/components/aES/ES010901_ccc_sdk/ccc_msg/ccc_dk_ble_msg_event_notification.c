#include "../ccc_sdk/ccc_dk_type.h"
#include "../ccc_util/ccc_dk_util.h"
#include "../ccc_msg/ccc_dk_ble_msg.h"
#include "../ccc_util/ccc_dk_func.h"
#include "../ccc_sdk/ccc_dk.h"
#include "../ccc_sdk/ccc_timer.h"
#include "../ccc_protocol/ccc_can.h"
#include "../ccc_protocol/ccc_apdu.h"
#include "ccc_dk_ble_msg_event_notification.h"
#include "../ccc_flow/ccc_dk_flow_owner_pairing.h"
#include "../../../aEM/EM00040101_log/EM000401.h"



//Table 19-69: Definition of Function ids and their corresponding Action ids.
const FunIdActionId_t funcIdActionIds[47]= 
{
    //function id: 0001, Central locking
    {FUNCTION_ID_CENTRAL_LOCKING, 0, ACTION_TYPE_EVENT},        //lock   
    {FUNCTION_ID_CENTRAL_LOCKING, 1, ACTION_TYPE_EVENT},        //unlock
    {FUNCTION_ID_CENTRAL_LOCKING, 50, ACTION_TYPE_EXECUTION},   //Vehicle-triggered Lock (only for execution status)
    {FUNCTION_ID_CENTRAL_LOCKING, 51, ACTION_TYPE_EXECUTION},   //Vehicle-triggered Unlock (only for execution status)

    //function id: 0002,  lock and secure
    {FUNCTION_ID_LOCK_AND_SECURE, 0, ACTION_TYPE_EVENT},        //lock and secure
    {FUNCTION_ID_LOCK_AND_SECURE, 1, ACTION_TYPE_EVENT},        //unlock
    {FUNCTION_ID_LOCK_AND_SECURE, 2, ACTION_TYPE_EVENT},        //Lock with partial arming of alarm system
    {FUNCTION_ID_LOCK_AND_SECURE, 50, ACTION_TYPE_EXECUTION},   //Vehicle-triggered Lock & secure (only for execution status)
    {FUNCTION_ID_LOCK_AND_SECURE, 51, ACTION_TYPE_EXECUTION},   //Vehicle-triggered Unlock (only for execution status)

    //function id: 0010, driving readiness
    {FUNCTION_ID_DRIVING_READINESS, 0xFF, ACTION_TYPE_EVENT},   //未定义action id

    //function id: 0011, vehicle low power mode 
    {FUNCTION_ID_VEHICLE_LOW_POWER_MODE, 0xFF, ACTION_TYPE_EVENT},   //未定义action id

    //function id: 0012, low fuel status
    {FUNCTION_ID_LOW_FUEL_STATUS, 0xFF, ACTION_TYPE_EVENT},   // 未定义action id

    //function id: 0100, remote climatization
    {FUNCTION_ID_REMOTE_CLIMATIZATION, 0x00, ACTION_TYPE_EVENT},   //0 stop climatization
    {FUNCTION_ID_REMOTE_CLIMATIZATION, 0x01, ACTION_TYPE_EVENT},   //1 start climatization

    //function id: 0101, panic alarm
    {FUNCTION_ID_PANIC_ALARM, 0x00, ACTION_TYPE_EVENT},   //0: mute panic alarm
    {FUNCTION_ID_PANIC_ALARM, 0x01, ACTION_TYPE_EVENT},   //1: trigger panic alarm

    //function id: 0102, fuel lid
    {FUNCTION_ID_FUEL_LID, 0x01, ACTION_TYPE_EVENT},   //1, release

    //function id: 0110, Manual Trunk/Decklid/Liftgate/Tailgate Controls
    {FUNCTION_ID_MANUAL_TRUNK_DECKLID_LIFTGATE_TAILGATE_CONTROLS, 0x01, ACTION_TYPE_EVENT},   //action id: 1: realse

    //function id: 0111,  Power Trunk/Decklid/Liftgate/Tailgate Controls (with confirmation for close)
    {FUNCTION_ID_POWER_TRUNK_DECKLID_LIFTGATE_TAILGATE_CONTROLS_WITH_CONFIRM, 0x00, ACTION_TYPE_EUDURING_WITH_CONFIRMATION},   //Close: enduring with confirmation
    {FUNCTION_ID_POWER_TRUNK_DECKLID_LIFTGATE_TAILGATE_CONTROLS_WITH_CONFIRM, 0x01, ACTION_TYPE_EUDURING_WITHOUT_CONFIRMATION},   //open: enduring without confirmation

    //function id: 0112, Power Trunk/Decklid/Liftgate/Tailgate Controls (without confirmation for close)
    {FUNCTION_ID_POWER_TRUNK_DECKLID_LIFTGATE_TAILGATE_CONTROLS_WITHOUT_CONFIRM, 0x00, ACTION_TYPE_EUDURING_WITHOUT_CONFIRMATION},   //close: enduring without confirmation
    {FUNCTION_ID_POWER_TRUNK_DECKLID_LIFTGATE_TAILGATE_CONTROLS_WITHOUT_CONFIRM, 0x01, ACTION_TYPE_EUDURING_WITHOUT_CONFIRMATION},   //open: enduring without confirmation

    //function id: 0120, Manual “Frunk” Front Trunk ControlsTraditional vehicle body style
    {FUNCTION_ID_MANUAL_FRUNK, 0x01, ACTION_TYPE_EVENT},   //1: release

    //function id: 0121, Power “Frunk” Front Trunk Controls with confirmation Traditional vehicle body style
    {FUNCTION_ID_POWER_FRUNK_WITH_CONFIRM, 0x00, ACTION_TYPE_EUDURING_WITH_CONFIRMATION},   //Close: enduring with confirmation
    {FUNCTION_ID_POWER_FRUNK_WITH_CONFIRM, 0x01, ACTION_TYPE_EUDURING_WITHOUT_CONFIRMATION},   //open: enduring without confirmation

    //function id: 0122, Power “Frunk” Front Trunk Controls without confirmation Traditional vehicle body style
    {FUNCTION_ID_POWER_GRUNK_WITHOUT_CONFIRM, 0x00, ACTION_TYPE_EUDURING_WITHOUT_CONFIRMATION},   //Close: enduring without confirmation
    {FUNCTION_ID_POWER_GRUNK_WITHOUT_CONFIRM, 0x01, ACTION_TYPE_EUDURING_WITHOUT_CONFIRMATION},   //open: enduring without confirmation

    //0130: Power Front Left with Confirmation
    {FUNCTION_ID_POWER_FRONT_LEFT_WITH_CONFIRM, 0x00, ACTION_TYPE_EUDURING_WITH_CONFIRMATION},   //close: 
    {FUNCTION_ID_POWER_FRONT_LEFT_WITH_CONFIRM, 0x01, ACTION_TYPE_EUDURING_WITH_CONFIRMATION},   //open: 

    //0131: Power Front Left without Confirmation
    {FUNCTION_ID_POWER_FRONT_LEFT_WITHOUT_CONFIRM, 0x00, ACTION_TYPE_EUDURING_WITHOUT_CONFIRMATION},   //close: 
    {FUNCTION_ID_POWER_FRONT_LEFT_WITHOUT_CONFIRM, 0x01, ACTION_TYPE_EUDURING_WITHOUT_CONFIRMATION},   //open: 

    //0132: Power Front right with Confirmation
    {FUNCTION_ID_POWER_FRONT_RIGHT_WITH_CONFIRM, 0x00, ACTION_TYPE_EUDURING_WITH_CONFIRMATION},   //close: 
    {FUNCTION_ID_POWER_FRONT_RIGHT_WITH_CONFIRM, 0x01, ACTION_TYPE_EUDURING_WITH_CONFIRMATION},   //open: 
    
    //0133: Power Front right without Confirmation
    {FUNCTION_ID_POWER_FRONT_RIGHT_WITHOUT_CONFIRM, 0x00, ACTION_TYPE_EUDURING_WITHOUT_CONFIRMATION},   //close: 
    {FUNCTION_ID_POWER_FRONT_RIGHT_WITHOUT_CONFIRM, 0x01, ACTION_TYPE_EUDURING_WITHOUT_CONFIRMATION},   //open: 

    //0134: Power REAR Left with Confirmation
    {FUNCTION_ID_POWER_REAR_LEFT_WITH_CONFIRM, 0x00, ACTION_TYPE_EUDURING_WITH_CONFIRMATION},   //close: 
    {FUNCTION_ID_POWER_REAR_LEFT_WITH_CONFIRM, 0x01, ACTION_TYPE_EUDURING_WITH_CONFIRMATION},   //open: 

    //0135: Power REAR Left without Confirmation
    {FUNCTION_ID_POWER_REAR_LEFT_WITHOUT_CONFIRM, 0x00, ACTION_TYPE_EUDURING_WITHOUT_CONFIRMATION},   //close: 
    {FUNCTION_ID_POWER_REAR_LEFT_WITHOUT_CONFIRM, 0x01, ACTION_TYPE_EUDURING_WITHOUT_CONFIRMATION},   //open: 

    //0136: Power REAR right with Confirmation
    {FUNCTION_ID_POWER_REAR_RIGHT_WITH_CONFIRM, 0x00, ACTION_TYPE_EUDURING_WITH_CONFIRMATION},   //close: 
    {FUNCTION_ID_POWER_REAR_RIGHT_WITH_CONFIRM, 0x01, ACTION_TYPE_EUDURING_WITH_CONFIRMATION},   //open: 
    
    //0137: Power REAR right without Confirmation
    {FUNCTION_ID_POWER_REAR_RIGHT_WITHOUT_CONFIRM, 0x00, ACTION_TYPE_EUDURING_WITHOUT_CONFIRMATION},   //close: 
    {FUNCTION_ID_POWER_REAR_RIGHT_WITHOUT_CONFIRM, 0x01, ACTION_TYPE_EUDURING_WITHOUT_CONFIRMATION},   //open: 

    //0140: Power windows, enduring with confirmation
    {FUNCTION_ID_POWER_WINDOWS, 0x00, ACTION_TYPE_EUDURING_WITH_CONFIRMATION},   //close: 
    {FUNCTION_ID_POWER_WINDOWS, 0x01, ACTION_TYPE_EUDURING_WITH_CONFIRMATION},   //open: 

    //0141: Power roof, enduring with confirmation
    {FUNCTION_ID_POWER_ROOF, 0x00, ACTION_TYPE_EUDURING_WITH_CONFIRMATION},   //close: 
    {FUNCTION_ID_POWER_ROOF, 0x01, ACTION_TYPE_EUDURING_WITH_CONFIRMATION},   //open: 
};

ActionType_e dk_func_get_action_type_by_func_action_id(u16 funcId, u8 actionId)
{
    u8 index = 0;

    ActionType_e actionType = ACTION_TYPE_FUNCTION_NOT_SUPPORT;

    for(index = 0; index < 47; index ++)
    {
        if(funcIdActionIds[index].funcId == funcId)
        {
            if(funcIdActionIds[index].actionId == actionId)
            {
                return funcIdActionIds[index].actionType;
            }
            else
            {
                actionType = ACTION_TYPE_ACTION_NOT_SUPPORT;
            }
        }
    }

    return actionType;  //not support rke func/action id;
}

CCCErrCode_e dk_event_notification_process_request_owner_pairing(u8 connId)
{
    if(ccc_fdk_unc_is_owner_pairing_permit())
    {
        return CCC_ERR_CODE_ACTION_NOT_PERMIT;
    }

    //配对整个过程30s超时
    ccc_timer_start_vehicle(connId, TIMER_ID_PAIRING_TOTAL_TIME, TIMER_TYPE_ONCE, TIMEOUT_MS_ON_PAIRING, ccc_timer_handler_pairing_timeOut);
    return select_digital_key_framework_aid_in_phase2(connId);
}

CCCErrCode_e msg_event_notification_process_command_complete_subEvent(u8 connId, u8 command_status)
{
    CCCErrCode_e errCode = CCC_ERR_CODE_SUCCESS;
    
    switch(command_status)
    {
        case Command_Status_Deselect_SE:
        case Command_Status_BLE_pairing_ready:
        case Command_Status_OP_Device_not_inside_vehicle:
            return CCC_ERR_CODE_UN_SUPPORT;
        case Command_Status_Require_capability_exchange:
            //TODO:  Upon receiving this, the Vehicle shall trigger ranging capability exchange
            //return ranging_capability_request(connId);
            ccc_can_send_uwb_anchor_wakeup_rq(LOC_IND_ALL, connId);  //Figure 19-34: Required Capability Exchanged.
            break;
        case Command_Status_Request_standard_transaction:
            return ccc_dk_select_aid_to_device_in_standard_transaction(connId);
            break;
        case Command_Status_Request_owner_pairing:
            errCode = dk_event_notification_process_request_owner_pairing(connId);
            if(errCode == CCC_ERR_CODE_ACTION_NOT_PERMIT)
            {
                send_command_complete_subevent_in_dk_event_notification(connId, Command_Status_BLE_pairing_failed);
            }
            break;
                
        case Command_Status_General_error:  
            //TODO: Upon receiving this, the vehicle or device may retry or abort.
            break;
        case Command_Status_Device_SE_busy:
            //TODO Upon receiving this, the vehicle may retry the digital key flow.
            ccc_timer_start(connId, TIMER_ID_OTHER_BUSINESS, TIMER_TYPE_ONCE, 2000, (pFunc_timer_handler)ccc_timer_handler_on_device_se_busy);
            break;
        case Command_Status_Device_SE_transaction_state_lost:
            //TODO Device may send this SubEvent to signal standard transaction state was 
            //lost. Upon receiving this, the vehicle may retry the transaction
            ccc_dk_select_aid_to_device_in_standard_transaction(connId);
            devicesInfo[connId].processOn = PROCESS_STANDARD_TRANSACTION_FLOW;
            devicesInfo[connId].subProcessOn = STANDARD_TRANSACTION_STATUS_SELECT;
            break;
        case Command_Status_Device_busy:
            //Device may send this SubEvent to signal device temporal not able to 
            //respond to the sent request. Upon receiving this, the vehicle may retry the action.
            if(core_dcm_readBig16(devicesInfo[connId].bleMsgBackupLV) > 0)
            {
                //重发一下上一条信息，仅重发一次
                cccConfig->sendData(connId, devicesInfo[connId].bleMsgBackupLV + 2, core_dcm_readBig16(devicesInfo[connId].bleMsgBackupLV));
                devicesInfo[connId].bleMsgBackupLV[0] = 0;
                devicesInfo[connId].bleMsgBackupLV[1] = 0;
            }
            break;
        case Command_Status_Command_temporarily_blocked:
            //Device shall send this SubEvent when the device UA state and UA policy do not allow the requested command.
            break;
        case Command_Status_Unsupported_channel_bitmask:
            //Device shall send this SubEvent if the channels provided in the channel bitmask are currently not supported by the device
            break;
        case Command_Status_OOB_mismatch:
            //Device or vehicle may send this SubEvent to signal the failure in First 
            //approach due to cryptography. This is an error code for debugging purposes.
            cccConfig->eventNotice(connId, SDK_EVENT_BLE_SECURE_OOB_MISMATCH, NULL, 0);
            break;
        case Command_Status_BLE_pairing_failed:
            //Device or vehicle may send this SubEvent to signal the failure in 
            //Bluetooth LE pairing. This is an error code for debugging purposes.
            cccConfig->timerTool->stopTimer(vehicleInfo.timers[TIMER_ID_PAIRING_TOTAL_TIME].timerId);
            cccConfig->eventNotice(connId, SDK_EVENT_BLE_DISCONECT, NULL, 0);
            break;
        case Command_Status_FA_crypto_operation_failed: 
            //Device or vehicle may send this SubEvent to signal the failure in First 
            //approach due to cryptography. This is an error code for debugging purposes.
            cccConfig->eventNotice(connId, SDK_EVENT_FA_CRYPTO_OPERATION_FAILED, NULL, 0);
            break;
        case Command_Status_Wrong_parameters:
            break;
        default:   //RFU
            break;
    }

    return errCode;
}

CCCErrCode_e msg_event_notification_process_ranging_session_status_changed_subEvent(u8 connId, u8 session_status)
{
    u8 num = 0;
    URSKStatus_e status = URSK_STATUS_EMPTY;

    CCCErrCode_e errCOde = CCC_ERR_CODE_SUCCESS;

    switch(session_status)
    {
        case Session_Status_Ranging_session_URSK_refresh:  //vehicle -> device
        case Session_Status_Ranging_session_secure_ranging_failed:
            return CCC_ERR_CODE_UN_SUPPORT;
        case Session_Status_Ranging_session_URSK_not_found:  
            //TODO Device shall send this SubEvent to signal, the device failed to find URSK for this UWB_Session_Id
            //vehicle process as Figure 19-35: URSK Not Found.
            ccc_dk_set_ranging_session_status(connId, devicesInfo[connId].rangingSession.uwbSessionId, URSK_STATUS_EMPTY);  //清掉当前使用的key;
            //num = ccc_dk_get_ursk_num_by_slotId(devicesInfo[connId].slotIdLV, URSK_STATUS_NOT_EMPTY);
              
            status = ccc_dk_find_ursk_for_ranging(connId, devicesInfo[connId].rangingSession.uwbSessionId, devicesInfo[connId].rangingSession.ursk);
            if(status == URSK_STATUS_PREDERIVED)
            {
                devicesInfo[connId].rangingSession.sessionStatus = RANGING_SESSION_SETUP;
                return uwb_ranging_service_ranging_session_request(connId);
            }
            else
            {
                //请求device clear pre-derived ursk，重新协商URSK
                devicesInfo[connId].rangingSession.sessionStatus = RANGING_SESSION_IDLE;
                send_ranging_session_status_changed_subevent_in_dk_event_notification(connId, Session_Status_Ranging_session_URSK_refresh);
                ccc_timer_start(connId, TIMER_ID_GENERATE_URSK, TIMER_TYPE_ONCE, 1000, ccc_timer_handler_URSK_Derivation);

                return CCC_ERR_CODE_SUCCESS;
            }
            break;
        case Session_Status_Ranging_session_terminated:
            //TODO Vehicle or device may send this SubEvent if it has stopped the ranging session (e.g. due to URSK TTL expiration)
            if(devicesInfo[connId].rangingSession.sessionStatus == RANGING_SESSION_ACTIVE)
            {
                LOG_L_S(CCC_MD, "TERMINATED11111111111111\r\n");
                ccc_can_send_uwb_ranging_session_suspend_recover_delete_rq(connId, RANGING_ACTION_DELETE);  //delete
            }
            ccc_dk_set_ranging_session_status(connId, devicesInfo[connId].rangingSession.uwbSessionId, URSK_STATUS_EMPTY);
            break;
        case Session_Status_Ranging_session_recovery_failed:
            //TODO Device shall send this SubEvent to signal it failed to recover ranging for this UWB_Session_Id
            //处理接收到的消息，车辆删除与恢复失败的 UWB_Session_Id 相关联的 URSK、配置和任何其他元数据。
            ccc_dk_set_ranging_session_status(connId, devicesInfo[connId].rangingSession.uwbSessionId, URSK_STATUS_EMPTY);

            //goto initate flow Selection(refer to Section 19.5.3.2)
            status = ccc_dk_find_ursk_for_ranging(connId, devicesInfo[connId].rangingSession.uwbSessionId, devicesInfo[connId].rangingSession.ursk);
            if(status == URSK_STATUS_PREDERIVED)
            {
                devicesInfo[connId].rangingSession.sessionStatus = RANGING_SESSION_SETUP;
                return uwb_ranging_service_ranging_session_request(connId);
            }
            else
            {
                //请求device clear pre-derived ursk，重新协商URSK
                devicesInfo[connId].rangingSession.sessionStatus = RANGING_SESSION_IDLE;
                send_ranging_session_status_changed_subevent_in_dk_event_notification(connId, Session_Status_Ranging_session_URSK_refresh);
                ccc_timer_start(connId, TIMER_ID_GENERATE_URSK, TIMER_TYPE_ONCE, 1000, ccc_timer_handler_URSK_Derivation);

                return CCC_ERR_CODE_SUCCESS;
            }
            break;
        case Ranging_session_not_required:   /*Device shall send this SubEvent when it detects the user does not intend 
                to approach the vehicle e.g device is moving away from the vehicle. Upon receiving this subEvent, the vehicle may immediately suspend the ranging session.*/
        case Session_status_Ranging_session_suspended:  /*When vehicle or device receives this, it shall suspend its current ranging 
            session as well without sending a Ranging_Suspend_RQ or a Ranging_session_suspended SubEvent. */
            if(devicesInfo[connId].rangingSession.sessionStatus == RANGING_SESSION_ACTIVE)
            {
                LOG_L_S(CCC_MD, "Suspending\r\n");
                ccc_can_send_uwb_ranging_session_suspend_recover_delete_rq(connId, RANGING_ACTION_SUSPEND);  //delete
            }
            ccc_dk_set_ranging_session_status(connId, devicesInfo[connId].rangingSession.uwbSessionId, URSK_STATUS_SUSPENDED);
            break;
        default:
            break;
    }

    return errCOde;
}

CCCErrCode_e msg_event_notification_process_device_ranging_intent_subevent(u8 connId, u8 dr_intent)
{
    CCCErrCode_e errCOde = CCC_ERR_CODE_SUCCESS;

    switch(dr_intent)
    {
        case DR_Intent_Low_approach_confidence:
        case DR_Intent_Medium_approach_confidence:
            //Depending on the power state of the vehicle, the vehicle may decide to ignore ranging intent with low or medium approach confidence.
            break;
        case DR_Intent_High_approach_confidence:
            ccc_dk_find_ursk_to_ranging(connId, FALSE);
            break;
        default:
            break;
    }

    return errCOde;
}

void rke_reset_on_finish()
{
    vehicleInfo.rkeRequest_t.rkeStatus = RKE_STATUS_FREE;
    vehicleInfo.rkeRequest_t.functionId = 0x00;
    vehicleInfo.rkeRequest_t.actionId = 0x00;
    vehicleInfo.rkeRequest_t.confirmNum = 0x00;
    vehicleInfo.rkeRequest_t.connId = 0x00;
    vehicleInfo.rkeRequest_t.actionType = ACTION_TYPE_EVENT;

    core_mm_set(vehicleInfo.rkeRequest_t.rkeChallenge, 0x00, 0x10);
    core_mm_set(vehicleInfo.rkeRequest_t.arbitraryData, 0x00, LEN_RKE_ARBITRARY_DATA);
}

void send_last_requested_action_execution_status_on_vehicle_status_changed_subevent(u8 connId, u8 exeStatus)
{
    u8 msgData[100];
    u16 msgDataLen = 0;

    msgData[msgDataLen++] = SubEvent_Category_Vehicle_Status_Change;  //sub event category;

    core_dcm_writeBig16(msgData + msgDataLen, CCC_TAG_LAST_REQUESTED_ACTION_EXE_STATUS);  //7F71
    msgDataLen += 2;
    msgData[msgDataLen++] = 0x0A;

    msgData[msgDataLen++] = CCC_TAG_FUNCTION_ID;
    msgData[msgDataLen++] = 0x02;
    core_dcm_writeBig16(msgData + msgDataLen, vehicleInfo.rkeRequest_t.functionId);
    msgDataLen += 2;

    msgData[msgDataLen++] = CCC_TAG_ACTION_ID;
    msgData[msgDataLen++] = 0x01;
    msgData[msgDataLen++] = vehicleInfo.rkeRequest_t.actionId;

    msgData[msgDataLen++] = CCC_TAG_EXE_STATUS;
    msgData[msgDataLen++] = 0x01;
    msgData[msgDataLen++] = exeStatus;

    ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_DK_EVENT_NOTIFICATION, DK_MESSAGE_ID_EVENT_NOTIFICATION, msgData, msgDataLen);
}

void send_last_requested_action_execution_and_function_status_on_vehicle_status_changed_subevent(u8 connId, u8 exeStatus, u8 functionStatus, u8* proprietaryData, u8 proprietaryDataLen)
{
    u8 msgData[100];
    u16 msgDataLen = 0;

    u8 offLen7F72;
    u8 offLen30In7F72;
    u8 offLenA0In30;

    msgData[msgDataLen++] = SubEvent_Category_Vehicle_Status_Change;  //sub event category;

    core_dcm_writeBig16(msgData + msgDataLen, CCC_TAG_LAST_REQUESTED_ACTION_EXE_STATUS);  //7F71
    msgDataLen += 2;
    msgData[msgDataLen++] = 0x0A;

    msgData[msgDataLen++] = CCC_TAG_FUNCTION_ID;
    msgData[msgDataLen++] = 0x02;
    core_dcm_writeBig16(msgData + msgDataLen, vehicleInfo.rkeRequest_t.functionId);
    msgDataLen += 2;

    msgData[msgDataLen++] = CCC_TAG_ACTION_ID;
    msgData[msgDataLen++] = 0x01;
    msgData[msgDataLen++] = vehicleInfo.rkeRequest_t.actionId;

    msgData[msgDataLen++] = CCC_TAG_EXE_STATUS;
    msgData[msgDataLen++] = 0x01;
    msgData[msgDataLen++] = exeStatus;

    core_dcm_writeBig16(msgData + msgDataLen, CCC_TAG_VEHICLE_FUNCTION_STATUS_SUMMARY);  //7F72
    msgDataLen += 2;
    offLen7F72 = msgDataLen++;

    msgData[msgDataLen++] = 0x30;
    offLen30In7F72 = msgDataLen++;

    msgData[msgDataLen++] = 0xA0;
    offLenA0In30 = msgDataLen++;

    msgData[msgDataLen++] = CCC_TAG_FUNCTION_ID;
    msgData[msgDataLen++] = 0x02;
    core_dcm_writeBig16(msgData + msgDataLen, vehicleInfo.rkeRequest_t.functionId);
    msgDataLen += 2;

    msgData[msgDataLen++] = CCC_TAG_FUNCTION_STATUS;
    msgData[msgDataLen++] = 0x01;
    msgData[msgDataLen++] = functionStatus;

    if(proprietaryDataLen > 0 && proprietaryDataLen <=64)
    {
        msgData[msgDataLen++] = CCC_TAG_VEHICLE_OEM_PROPRIETARY_DATA;
        msgData[msgDataLen++] = proprietaryDataLen;

        core_mm_copy(msgData + msgDataLen, proprietaryData, proprietaryDataLen);
        msgDataLen += proprietaryDataLen;
    }
    
    msgData[offLenA0In30] = (msgDataLen - offLenA0In30 - 1);
    msgData[offLen30In7F72] = (msgDataLen - offLen30In7F72 - 1);
    msgData[offLen7F72] = (msgDataLen - offLen7F72 - 1);

    ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_DK_EVENT_NOTIFICATION, DK_MESSAGE_ID_EVENT_NOTIFICATION, msgData, msgDataLen);
}

void send_vehicle_function_status_summary_on_vehicle_status_changed_subevent(u8 connId, u8 funcStatus, u8* proData, u8 proDataLen)
{
    u8 msgData[100];
    u16 msgDataLen = 0;

    msgData[msgDataLen++] = SubEvent_Category_Vehicle_Status_Change;  //sub event category;

    core_dcm_writeBig16(msgData + msgDataLen, CCC_TAG_VEHICLE_FUNCTION_STATUS_SUMMARY);  //7F72
    msgDataLen += 2;
    msgData[msgDataLen++] = (13 + proDataLen);

    msgData[msgDataLen++] = CCC_TAG_SEQUENCE;
    msgData[msgDataLen++] = (11 + proDataLen);

    msgData[msgDataLen++] = CCC_TAG_VEHICLE_FUNCTION_STATUS;
    msgData[msgDataLen++] = (9 + proDataLen);

    msgData[msgDataLen++] = CCC_TAG_FUNCTION_ID;
    msgData[msgDataLen++] = 0x02;

    core_dcm_writeBig16(msgData + msgDataLen, vehicleInfo.rkeRequest_t.functionId);
    msgDataLen += 2;

    msgData[msgDataLen++] = CCC_TAG_FUNCTION_STATUS;
    msgData[msgDataLen++] = 0x01;
    msgData[msgDataLen++] = funcStatus;

    msgData[msgDataLen++] = CCC_TAG_VEHICLE_OEM_PROPRIETARY_DATA;
    msgData[msgDataLen++] = proDataLen;
    core_mm_copy(msgData + msgDataLen, proData, proDataLen);
    msgDataLen += proDataLen;

    ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_DK_EVENT_NOTIFICATION, DK_MESSAGE_ID_EVENT_NOTIFICATION, msgData, msgDataLen);
}

void send_start_and_request_continuation_confimrmation_for_enduring_RKE(u8 connId, u8* pArbitraryData, u8 arbitraryDataLen)
{
    u8 msgData[100];
    u16 msgDataLen = 0;

    //-----------------------7F71---------------------------------
    msgData[msgDataLen++] = SubEvent_Category_Vehicle_Status_Change;  //sub event category;

    core_dcm_writeBig16(msgData + msgDataLen, CCC_TAG_LAST_REQUESTED_ACTION_EXE_STATUS);  
    msgDataLen += 2; 
    msgData[msgDataLen++] = 0x0A;

    msgData[msgDataLen++] = CCC_TAG_FUNCTION_ID;
    msgData[msgDataLen++] = 0x02;
    core_dcm_writeBig16(msgData + msgDataLen, vehicleInfo.rkeRequest_t.functionId);
    msgDataLen += 2;

    msgData[msgDataLen++] = CCC_TAG_ACTION_ID;
    msgData[msgDataLen++] = 0x01;
    msgData[msgDataLen++] = vehicleInfo.rkeRequest_t.actionId;

    msgData[msgDataLen++] = CCC_TAG_EXE_STATUS;
    msgData[msgDataLen++] = 0x01;
    msgData[msgDataLen++] = 0x01;   //01h: Execution started

    //----------------------------------7F75----------------------------------------------
    core_dcm_writeBig16(msgData + msgDataLen, CCC_TAG_REQUEST_CONFIRMATION_FOR_ENDURING_RKE);
    msgDataLen += 2;
    msgData[msgDataLen++] = (9 + arbitraryDataLen);

    msgData[msgDataLen++] = CCC_TAG_FUNCTION_ID;
    msgData[msgDataLen++] = 0x02;
    core_dcm_writeBig16(msgData + msgDataLen, vehicleInfo.rkeRequest_t.functionId);
    msgDataLen += 2;

    msgData[msgDataLen++] = CCC_TAG_ACTION_ID;
    msgData[msgDataLen++] = 0x01;
    msgData[msgDataLen++] = vehicleInfo.rkeRequest_t.actionId;

    msgData[msgDataLen++] = CCC_TAG_ARBITRARY;
    msgData[msgDataLen++] = arbitraryDataLen;
    core_mm_copy(msgData + msgDataLen, pArbitraryData, arbitraryDataLen);
    msgDataLen += arbitraryDataLen;

    ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_DK_EVENT_NOTIFICATION, DK_MESSAGE_ID_EVENT_NOTIFICATION, msgData, msgDataLen);
}

/**
 * @brief Request confirmation for enduring RKE action. Only present, when enduring RKE action is in progress.
 * 
 * @param connId 
 * @param pArbitraryData 
 * @param arbitraryDataLen 
 */
void send_request_continuation_confirmation_for_enduring_RKE(u8 connId, u8* pArbitraryData, u8 arbitraryDataLen)
{
    u8 msgData[100];
    u16 msgDataLen = 0;

    msgData[msgDataLen++] = SubEvent_Category_Vehicle_Status_Change;  //sub event category;

    core_dcm_writeBig16(msgData + msgDataLen, CCC_TAG_REQUEST_CONFIRMATION_FOR_ENDURING_RKE);
    msgDataLen += 2;
    msgData[msgDataLen++] = (9 + arbitraryDataLen);

    msgData[msgDataLen++] = CCC_TAG_FUNCTION_ID;
    msgData[msgDataLen++] = 0x02;
    core_dcm_writeBig16(msgData + msgDataLen, vehicleInfo.rkeRequest_t.functionId);
    msgDataLen += 2;

    msgData[msgDataLen++] = CCC_TAG_ACTION_ID;
    msgData[msgDataLen++] = 0x01;
    msgData[msgDataLen++] = vehicleInfo.rkeRequest_t.actionId;

    msgData[msgDataLen++] = CCC_TAG_ARBITRARY;
    msgData[msgDataLen++] = arbitraryDataLen;
    core_mm_copy(msgData + msgDataLen, pArbitraryData, arbitraryDataLen);
    msgDataLen += arbitraryDataLen;

    ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_DK_EVENT_NOTIFICATION, DK_MESSAGE_ID_EVENT_NOTIFICATION, msgData, msgDataLen);
}

void send_subscription_status_changed_on_vehicle_status_changed_subevent(u8 connId, u8 status)
{
    u8 msgData[100];
    u16 msgDataLen = 0;

    msgData[msgDataLen++] = SubEvent_Category_Vehicle_Status_Change;  //sub event category;

    core_dcm_writeBig16(msgData + msgDataLen, CCC_TAG_SUBSCRIPTION_STATUS_CHANGED);
    msgDataLen += 2;
    msgData[msgDataLen++] = 0x01;
    msgData[msgDataLen++] = status;

    ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_DK_EVENT_NOTIFICATION, DK_MESSAGE_ID_EVENT_NOTIFICATION, msgData, msgDataLen);
}

void send_command_complete_subevent_in_dk_event_notification(u8 connId, u8 commandStatus)
{
    u8 msgData[10];
    u16 msgDataLen = 0;

    msgData[msgDataLen++] = SubEvent_Category_Command_Complete;  //sub event category;

    msgData[msgDataLen++] = commandStatus;

    ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_DK_EVENT_NOTIFICATION, DK_MESSAGE_ID_EVENT_NOTIFICATION, msgData, msgDataLen);
}


/*****************************************************************************
FUNCTION: send session status changed subevnet;
PARAMETER:
        connId: the connId of the device;
        sessionStatus: 
            Ranging_session_URSK_refresh 0x00 Vehicle may send this SubEvent to request clean-up for pre_derived URSKs.
            Ranging_session_secure_ranging_failed 0x03 Vehicle shall send this SubEvent to signal the Vehicle failed to establish secure ranging.
            Ranging_session_terminated 0x04 Vehicle or device may send this SubEvent if it has stopped the ranging session (e.g. due to URSK TTL expiration)
RETURN: none
******************************************************************************/
void send_ranging_session_status_changed_subevent_in_dk_event_notification(u8 connId, u8 sessionStatus)
{
    u8 msgData[10];
    u16 msgDataLen = 0;

    msgData[msgDataLen++] = SubEvent_Category_Ranging_Session_Status_Changed;  //sub event category;

    msgData[msgDataLen++] = sessionStatus;

    if(sessionStatus == Session_Status_Ranging_session_secure_ranging_failed || sessionStatus == Session_Status_Ranging_session_terminated)
    {
        ccc_dk_set_ranging_session_status(connId, devicesInfo[connId].rangingSession.uwbSessionId, URSK_STATUS_EMPTY);
    }

    ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_DK_EVENT_NOTIFICATION, DK_MESSAGE_ID_EVENT_NOTIFICATION, msgData, msgDataLen);
}

CCCErrCode_e rke_request_subevent_process_request_rke_action(u8 connId, u8* p_reqPke)
{
    //7F70 07 80 02 fucId 81 01 actionId
    u16 functionId;
    u8 actionId;
    ActionType_e actionType;
    u8 timerType = TIMER_TYPE_ONCE;
    u32 timeOutMs;
    
    if( (*(p_reqPke + 2) != 0x07) || (*(p_reqPke + 3) != 0x80) || (*(p_reqPke + 4) != 0x02)
        || (*(p_reqPke + 7) != 0x81) || (*(p_reqPke + 8) != 0x01))
    {
        return CCC_ERR_CODE_DATA_ERR;
    }

    functionId = core_dcm_mku16(*(p_reqPke + 5), *(p_reqPke + 6));
    actionId = *(p_reqPke + 9);
    actionType = dk_func_get_action_type_by_func_action_id(functionId, actionId);

    if(!devicesInfo[connId].authFlag)
    {
        //Vehicle Status Changed SubEvent
        send_last_requested_action_execution_status_on_vehicle_status_changed_subevent(connId, AUTHENTICATION_ERROR);

        return CCC_ERR_CODE_ACTION_NOT_PERMIT;
    }

    if(vehicleInfo.rkeRequest_t.rkeStatus != RKE_STATUS_FREE && vehicleInfo.rkeRequest_t.rkeStatus != RKE_STATUS_FINISH)
    {
        //the last rke not finish;
        if(vehicleInfo.rkeRequest_t.functionId == functionId && vehicleInfo.rkeRequest_t.actionId == actionId)
        {
            return CCC_ERR_CODE_SUCCESS;
        }
        else //the last rke is going;
        {
            send_last_requested_action_execution_status_on_vehicle_status_changed_subevent(connId, Function_Status_Function_temporarily_not_available);
            
            return CCC_ERR_CODE_ON_PROCESSING;  //上一条车控正在处理中
        }
    }

    if(actionType == ACTION_TYPE_FUNCTION_NOT_SUPPORT)
    {
        send_last_requested_action_execution_status_on_vehicle_status_changed_subevent(connId, Function_Status_Function_not_supported_by_vehicle);
            
        return CCC_ERR_CODE_UN_SUPPORT;  //上一条车控正在处理中
    }
    else if(actionType == ACTION_TYPE_ACTION_NOT_SUPPORT)
    {
        send_last_requested_action_execution_status_on_vehicle_status_changed_subevent(connId, Function_Status_Action_not_supported_by_vehicle);
         
        return CCC_ERR_CODE_UN_SUPPORT;
    }

    //TODO车控权限检查

    apdu_to_vehicle_se_ble_rke_verify_on_get_challenge(connId, p_reqPke, 10, vehicleInfo.rkeRequest_t.rkeChallenge);
    vehicleInfo.rkeRequest_t.functionId = functionId;
    vehicleInfo.rkeRequest_t.actionId = actionId;
    vehicleInfo.rkeRequest_t.actionType = dk_func_get_action_type_by_func_action_id(functionId, actionId);
    vehicleInfo.rkeRequest_t.rkeStatus = RKE_STATUS_AUTH_RQ;
    vehicleInfo.rkeRequest_t.connId = connId;
    
    ccc_timer_start_vehicle(connId, TIMER_ID_RKE, TIMER_TYPE_ONCE, TIMEOUT_MS_ON_RKE_EVENT, (pFunc_timer_handler)ccc_timer_handler_on_rke_time_out);

    ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_SUPPLEMENTARY_SERVICE, DK_MESSAGE_ID_RKE_AUTH_RQ, vehicleInfo.rkeRequest_t.rkeChallenge, 0x10);

    return CCC_ERR_CODE_SUCCESS;
}

/**
 * @brief 
 * 
 * @param pIn_continueRke : 7F76(80 functionId, 81 Actioon id, 88 arbitrary data)
 * @return boolean 
 */
boolean check_continue_confirm_info(u8* pIn_continueRke)
{
    u16 off = 0;

    u16 offFuncId = 0;
    u16 offActionId = 0;
    u16 offArbitraryData = 0;

    off = 3;  //

    do
    {
        if(pIn_continueRke[off] == CCC_TAG_FUNCTION_ID)
        {
            if(pIn_continueRke[off + 1] == 0x02)
            {
                offFuncId = (off + 2);
            }
            else
            {
                return FALSE;
            }
        }
        else if(pIn_continueRke[off] == CCC_TAG_ACTION_ID)
        {
            if(pIn_continueRke[off + 1] == 0x01)
            {
                offActionId = (off + 2);
            }
            else
            {
                return FALSE;
            }
        }
        else if(pIn_continueRke[off] == 0x88)
        {
            if(pIn_continueRke[off + 1] == LEN_RKE_ARBITRARY_DATA)
            {
                offArbitraryData = (off + 2);
            }
            else
            {
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }

        off += (2 + pIn_continueRke[off + 1]);
    } while (off < (3 + pIn_continueRke[2]));

    if(off != (3 + pIn_continueRke[2]))
    {
        return FALSE;
    }

    if(offFuncId == 0 || (core_dcm_readBig16(pIn_continueRke + offFuncId) != vehicleInfo.rkeRequest_t.functionId))
    {
        return FALSE;
    }

    if(offActionId == 0 || vehicleInfo.rkeRequest_t.actionId != pIn_continueRke[offActionId])
    {
        return FALSE;
    }
    
    if(offArbitraryData != 0 && 
        core_mm_compare(pIn_continueRke + offArbitraryData, vehicleInfo.rkeRequest_t.arbitraryData, LEN_RKE_ARBITRARY_DATA) != 0)
    {
        return FALSE;
    }

    return TRUE;
}

/**
 * @brief Continue enduring RKE action. Only present, when enduring RKE action is in progress.
 * 
 * @param connId 
 * @param pIn_continueRke ： 7F76(80 functionId, 81 Actioon id, 88 arbitrary data)
 * @return CCCErrCode_e 
 */
CCCErrCode_e rke_request_subevent_process_continue_enduring_rke_action(u8 connId, u8* pIn_continueRke)
{
    if(vehicleInfo.rkeRequest_t.actionType == ACTION_TYPE_EUDURING_WITH_CONFIRMATION)
    {
        if(vehicleInfo.rkeRequest_t.rkeStatus == RKE_STATUS_ENDURING)
        {
            cccConfig->timerTool->stopTimer(vehicleInfo.timers[TIMER_ID_RKE].timerId);

            if(check_continue_confirm_info(pIn_continueRke))
            {
                vehicleInfo.rkeRequest_t.confirmNum++;

                if(vehicleInfo.rkeRequest_t.confirmNum >= TIMEOUT_CONFIMR_NUM_ON_ENDURING_RKE)
                {
                    //车端执行结束响应超时;
                    send_last_requested_action_execution_status_on_vehicle_status_changed_subevent(connId, VEHICLE_RESPONSE_TIMEOUT);
                    rke_reset_on_finish();
                }
                else
                {
                    send_request_continuation_confirmation_for_enduring_RKE(connId, vehicleInfo.rkeRequest_t.arbitraryData, LEN_RKE_ARBITRARY_DATA);
                    ccc_timer_start_vehicle(connId, TIMER_ID_RKE, TIMER_TYPE_ONCE, TIMEOUT_MS_ON_REQUIRE_CONFIRM, (pFunc_timer_handler)ccc_timer_handler_on_rke_request_confirm_continuation_time_out);
                }
            }
            else
            {
                ccc_can_send_rke_execute_rq(connId, vehicleInfo.rkeRequest_t.functionId, vehicleInfo.rkeRequest_t.actionId, 0x02);  //stop
                send_last_requested_action_execution_status_on_vehicle_status_changed_subevent(connId, DEVICE_CONTINUE_CONFIRM_DATA_ERR);
                rke_reset_on_finish();
            }
        }
        else
        {
            send_last_requested_action_execution_status_on_vehicle_status_changed_subevent(connId, DEVICE_OUT_OF_EXECUTION_RANGE);
        } 
    }
    else if(vehicleInfo.rkeRequest_t.actionType == ACTION_TYPE_EUDURING_WITHOUT_CONFIRMATION)
    {
        if(vehicleInfo.rkeRequest_t.rkeStatus == RKE_STATUS_ENDURING)
        {
            if(vehicleInfo.rkeRequest_t.confirmNum == 0)
            {
                if(check_continue_confirm_info(pIn_continueRke))
                {
                    cccConfig->timerTool->stopTimer(vehicleInfo.timers[TIMER_ID_RKE].timerId);

                    ccc_timer_start_vehicle(connId, TIMER_ID_RKE, TIMER_TYPE_ONCE, TIMEOUT_MS_ON_RKE_ENDURING, (pFunc_timer_handler)ccc_timer_handler_on_rke_time_out);
                    vehicleInfo.rkeRequest_t.confirmNum = 1;
                }
                else
                {
                    ccc_can_send_rke_execute_rq(connId, vehicleInfo.rkeRequest_t.functionId, vehicleInfo.rkeRequest_t.actionId, 0x02);  //stop
                    send_last_requested_action_execution_status_on_vehicle_status_changed_subevent(connId, DEVICE_CONTINUE_CONFIRM_DATA_ERR);
                    rke_reset_on_finish();
                }
            }
            else
            {
                send_last_requested_action_execution_status_on_vehicle_status_changed_subevent(connId, DEVICE_OUT_OF_EXECUTION_RANGE);
            }
        }
        else
        {
            send_last_requested_action_execution_status_on_vehicle_status_changed_subevent(connId, DEVICE_OUT_OF_EXECUTION_RANGE);
        }
    }
    else
    {
        send_last_requested_action_execution_status_on_vehicle_status_changed_subevent(connId, DEVICE_OUT_OF_EXECUTION_RANGE);
    }

    return CCC_ERR_CODE_SUCCESS;
}
    
/**
 * @brief 
 * 
 * @param connId 
 * @param pIn_stopRke: 7F77 07 (80 02 function id 81 01 action id)
 * @return CCCErrCode_e 
 */
CCCErrCode_e rke_request_subevent_process_stop_enduring_rke_action(u8 connId, u8* pIn_stopRke)
{
    if(vehicleInfo.rkeRequest_t.actionType == ACTION_TYPE_EUDURING_WITHOUT_CONFIRMATION)
    {
        if(vehicleInfo.rkeRequest_t.rkeStatus == RKE_STATUS_ENDURING)
        {
            cccConfig->timerTool->stopTimer(vehicleInfo.timers[TIMER_ID_RKE].timerId);

            ccc_can_send_rke_execute_rq(connId, vehicleInfo.rkeRequest_t.functionId, vehicleInfo.rkeRequest_t.actionId, 0x02);  //stop

            send_last_requested_action_execution_status_on_vehicle_status_changed_subevent(connId, EXECUTION_STOPPED_SUCCESSFULLY);

            rke_reset_on_finish();

            return CCC_ERR_CODE_SUCCESS;
        }
        else
        {
            send_last_requested_action_execution_status_on_vehicle_status_changed_subevent(connId, EXECUTION_NOT_POSSIBLE_DUE_TO_VEHICLE_STATE);
            return CCC_ERR_CODE_ACTION_NOT_PERMIT;
        }  
    }
    else
    {
        send_last_requested_action_execution_status_on_vehicle_status_changed_subevent(connId, EXECUTION_NOT_POSSIBLE_DUE_TO_VEHICLE_STATE);
        return CCC_ERR_CODE_ACTION_NOT_PERMIT;
    }
}

CCCErrCode_e msg_event_notification_process_rke_request_subevent(u8 connId, u8* par, u16 parLen)
{
    u16 offRequestRKE = UNEXIST;  //7F70;
    u16 offSubscribeFuncId = UNEXIST;   //7F73
    u16 offGetStatusFuncID = UNEXIST;   //7F74
    u16 offContinueEnduringRKE = UNEXIST;       //7F76;
    u16 offStopEnduringRKE = UNEXIST;    //7F77

    u8* p = par + 1;
    u16 tag = 0;
    do
    {
        tag = ccc_tlv_util_get_tag(p);

        if(tag == TAG_REQUEST_RKE_ACTION)
        {
            offRequestRKE = (p - par);
        }
        else if(tag == TAG_SUBSCRIBE_TO_VEHICLE_FUNCTION_STATUS)
        {
            offSubscribeFuncId = (p - par);
        }
        else if(tag == TAG_GET_STATUS_UPDATE_FOR_FUNCTION_IDS)
        {
            offGetStatusFuncID = (p - par);
        }
        else if(tag == TAG_CONTINUE_ENDURING_RKE_ACTION)
        {
            offContinueEnduringRKE = (p - par);
        }
        else if(tag == TAG_STOP_ENDURING_RKE_ACTION)
        {
            offStopEnduringRKE = (p - par);
        }

        p += ccc_tlv_util_get_tlv_width(p);
    } while (p < par + parLen);

    if( (p != (par + parLen))
        || (offRequestRKE == UNEXIST && offSubscribeFuncId == UNEXIST && offGetStatusFuncID == UNEXIST && 
            offContinueEnduringRKE == UNEXIST && offStopEnduringRKE == UNEXIST))
    {
        return CCC_ERR_CODE_DATA_ERR;
    }

    if(offRequestRKE != UNEXIST)  //Request RKE action  
    {
        return rke_request_subevent_process_request_rke_action(connId, par + offRequestRKE);
    }

    if(offSubscribeFuncId != UNEXIST)  //Subscribe to vehicle function status events
    {

    }

    if(offGetStatusFuncID != UNEXIST)   //Get status update for function ids. Only present, when explicit status updates shall be requested.
    {

    }

    if(offContinueEnduringRKE != UNEXIST)  //7F76
    {
        return rke_request_subevent_process_continue_enduring_rke_action(connId, par + offContinueEnduringRKE);
    }

    if(offStopEnduringRKE != UNEXIST)    //7F77 stop
    {
        return rke_request_subevent_process_stop_enduring_rke_action(connId, par + offStopEnduringRKE);
    }
}

CCCErrCode_e ccc_dk_ble_msg_event_notification_process(u8 connId, CCCDkMsg_t * cccDkMsg_t)
{
    CCCErrCode_e errCOde = CCC_ERR_CODE_SUCCESS;

    if(cccDkMsg_t->payload_header_MsgId != DK_MESSAGE_ID_EVENT_NOTIFICATION)
    {
        return CCC_ERR_CODE_DATA_ERR;
    }

    switch(cccDkMsg_t->data[0])  //DK SubEvent Category
    {
        case SubEvent_Category_Command_Complete:
            return msg_event_notification_process_command_complete_subEvent(connId, cccDkMsg_t->data[1]);
        case SubEvent_Category_Ranging_Session_Status_Changed:
            return msg_event_notification_process_ranging_session_status_changed_subEvent(connId, cccDkMsg_t->data[1]);
        case SubEvent_Category_Device_Ranging_Intent:
            return msg_event_notification_process_device_ranging_intent_subevent(connId, cccDkMsg_t->data[1]);
        case SubEvent_Category_Vehicle_Status_Change:
            return CCC_ERR_CODE_UN_SUPPORT;
        case SubEvent_Category_RKE_Request:
            return msg_event_notification_process_rke_request_subevent(connId, cccDkMsg_t->data, ccc_dk_msg_get_data_len(cccDkMsg_t));
        case SubEvent_Category_Head_Unit_Pairing:
            break;
        default:  //rfu;
            break;
    }

    return errCOde;
}
