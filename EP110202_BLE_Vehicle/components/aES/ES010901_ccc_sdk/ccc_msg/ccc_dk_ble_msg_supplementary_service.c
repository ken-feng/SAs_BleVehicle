#include "../ccc_util/ccc_dk_util.h"
#include "../ccc_util/ccc_dk_func.h"
#include "../ccc_msg/ccc_dk_ble_msg.h"
#include "../ccc_sdk/ccc_dk_type.h"
#include "../ccc_sdk/ccc_dk.h"
#include "../ccc_sdk/ccc_dk_api.h"
#include "../ccc_sdk/ccc_timer.h"
#include "../../../aEM/EM00020101_algo/EM000201.h"
#include "../ccc_protocol/ccc_can.h"
#include "../ccc_protocol/ccc_apdu.h"
#include "ccc_dk_ble_msg_event_notification.h"

CCCErrCode_e msg_supplementary_service_message_first_approach_rq_process(u8 connId, u8* data, u16 len)
{
    u16 apduLen = 0;
    u8 res = 0;
    u8* apdu = devicesInfo[connId].apdu;
    u8* apduResp = devicesInfo[connId].apduResp;
    u16 respLen;
    CCCErrCode_e errCode = CCC_ERR_CODE_SUCCESS;

    u8 Cb[0x10];
    u8 rb[0x10];
    u8 Ca[0x10];

    if(len != 0x46)
    {
        return CCC_ERR_CODE_DATA_ERR;
    }

    if(devicesInfo[connId].processOn != PROCESS_BLE_PAIRING_ENCRYPTION_SETUP)
    {
        return CCC_ERR_CODE_ILLEGAL_STATE;
    }

    //check oob pairing;
    apdu[apduLen++] = 0x80;
    apdu[apduLen++] = 0x7D;
    apdu[apduLen++] = 0x01;  //01：SE端进行生成First_Approach_RS
    apdu[apduLen++] = connId;
    apduLen++;  //skip lc;

    apdu[apduLen++] = 0xC0;
    apdu[apduLen++] = 0x46;
    core_mm_copy(apdu + apduLen, data, len);
    apduLen += len;

    //devcie ble addr
    apdu[apduLen++] = 0xC1;
    apdu[apduLen++] = 0x06;
    core_mm_copy(apdu + apduLen, devicesInfo[connId].macAddr, 0x06);
    apduLen += 6;

    //vehicle ble addr
    apdu[apduLen++] = 0xC2;
    apdu[apduLen++] = 0x06;
    core_mm_copy(apdu + apduLen, vehicleInfo.macAddr, 0x06);   
    apduLen += 6;

    //vehicle ble 配对公钥X坐标
    apdu[apduLen++] = 0xC3;
    apdu[apduLen++] = 0x20;
    cccConfig->getInfo(INFO_TYPE_VEHICLE_ECC_PUBKEY_X_ON_OOB_PAIRING, NULL, 0, apdu + apduLen, &respLen);
    apduLen += 0x20;

    //对APDUB用安全通道密钥进行加密；
    apduLen = ccc_add_padding_8000(apdu + apduLen, (apduLen - 5), TRUE, 0x10);
    apduLen = core_algo_aes_cbc_cipher(MODE_ENCRYPT, vehicleInfo.scKey, 0x10, 0x10, vehicleInfo.scIv, 0x10, 
        apdu + 5, apdu + 5, apduLen);

    apdu[4] = (u8)apduLen;

    res = ccc_dk_func_send_apdu_to_vehicle_se(apdu, (5 + apduLen), apduResp, &respLen);

    if(res != 0 || respLen < 2)
    {
        return CCC_ERR_CODE_SE_COMMUNICATE_ERR;
    }
    else if(apduResp[respLen - 2] == 0x90 && apduResp[respLen - 1] == 0x00)
    {
        //TLV(C1,First_Approach_RS)
        //send First Approach Response Message (FA-RS) to device with ble
        ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_SUPPLEMENTARY_SERVICE, DK_MESSAGE_ID_FIRST_APPROACH_RS, (apduResp + 2), 0x32);

        //通知蓝牙BLE Secure OOB Pairing Prep已完成;
        cccConfig->eventNotice(connId, SDK_EVENT_BLE_SECURE_OOB_PAIRING_PREP_FINISH, NULL, 0);

        return CCC_ERR_CODE_SUCCESS;
    }
    else
    {
        return core_dcm_mku16(apduResp[respLen - 2], apduResp[respLen - 1]);
    }
}

CCCErrCode_e msg_supplementary_service_message_time_sync_process(u8 connId, u8* data, u16 len)
{
    u8 success = *(data + 20);
    u8 sessionId[4];

    URSKStatus_e status = URSK_STATUS_EMPTY;

    if(len != 23)
    {
        return CCC_ERR_CODE_DATA_ERR;
    }

    if(success == 0)
    {
        //The “Bluetooth LE Timesync” procedure has failed and the timesync message parameters are invalid. The 
        //vehicle should ask again to perform the “Bluetooth LE Timesync” after RetryDelay
        ccc_timer_start(connId, TIMER_ID_RANGING_SET_UP, TIMER_TYPE_ONCE, core_dcm_mku16(data[21], data[22]), (pFunc_timer_handler)ccc_timer_handler_timesync_triggered_by_vehicle);
        return CCC_ERR_CODE_SUCCESS;
    }
    else if(success == 1)
    {
        //The “Bluetooth LE Timesync” procedure on device side is successfully able to map the UWB clock and all 
        //parameters in the timesync message are valid.
        ccc_can_send_time_sync_rq(data, 20);

        if(devicesInfo[connId].timeSyncTriggerConditions == 1)  //Procedure 1
        {
#if 0
            ccc_timer_start(connId, TIMER_ID_RANGING_SET_UP, TIMER_TYPE_ONCE, TIMEOUT_MS_ON_TIME_SYNC, (pFunc_timer_handler)ccc_timer_handler_on_time_sync_on_procedure1);
#endif
        }
        return CCC_ERR_CODE_SUCCESS;
    }
    else if(success == 2)
    {
        //The “Bluetooth LE Timesync” procedure has failed and 
        //the Procedure 0 is not available. This only applies to Procedure 0.

        return CCC_ERR_CODE_ACTION_NOT_PERMIT;
    }
}

CCCErrCode_e msg_supplementary_service_message_rke_auth_rs_process(u8 connId, u8* data, u16 len)
{
    CCCErrCode_e errCode = CCC_ERR_CODE_SUCCESS;

    if(vehicleInfo.rkeRequest_t.rkeStatus != RKE_STATUS_AUTH_RQ)
    {
        return CCC_ERR_CODE_ILLEGAL_STATE;
    }

    cccConfig->timerTool->stopTimer(vehicleInfo.timers[TIMER_ID_RKE].timerId);

    errCode = apdu_to_vehicle_se_ble_rke_verify_on_verify_signature(connId, data, len, vehicleInfo.rkeRequest_t.arbitraryData);
    if(errCode != CCC_ERR_CODE_SUCCESS)
    {
        send_last_requested_action_execution_status_on_vehicle_status_changed_subevent(connId,
            Function_Status_Use_of_wrong_RKE_security_policy_for_requested_action);
        
        rke_reset_on_finish();
        return errCode;
    }

    vehicleInfo.rkeRequest_t.rkeStatus = RKE_STATUS_AUTH_VERIFY;

    //start execute RKE
    ccc_can_send_rke_execute_rq(connId, vehicleInfo.rkeRequest_t.functionId, vehicleInfo.rkeRequest_t.actionId, 0x01);

    if(vehicleInfo.rkeRequest_t.actionType == ACTION_TYPE_EUDURING_WITHOUT_CONFIRMATION ||
        vehicleInfo.rkeRequest_t.actionType == ACTION_TYPE_EUDURING_WITH_CONFIRMATION)
    {
        vehicleInfo.rkeRequest_t.confirmNum = 0;
        send_start_and_request_continuation_confimrmation_for_enduring_RKE(connId, vehicleInfo.rkeRequest_t.arbitraryData, LEN_RKE_ARBITRARY_DATA);

        ccc_timer_start_vehicle(connId, TIMER_ID_RKE, TIMER_TYPE_ONCE, TIMEOUT_MS_ON_REQUIRE_CONFIRM, (pFunc_timer_handler)ccc_timer_handler_on_rke_request_confirm_continuation_time_out);
    }
    else  //event
    {
        ccc_timer_start_vehicle(connId, TIMER_ID_RKE, TIMER_TYPE_ONCE, TIMEOUT_MS_ON_RKE_EVENT, (pFunc_timer_handler)ccc_timer_handler_on_rke_time_out);
    }

    vehicleInfo.rkeRequest_t.rkeStatus = RKE_STATUS_ENDURING;

    return errCode;
}

CCCErrCode_e ccc_dk_ble_msg_supplementary_service_process(u8 connId, CCCDkMsg_t * cccDkMsg_t)
{
    CCCErrCode_e errCode = CCC_ERR_CODE_SUCCESS;

    switch (cccDkMsg_t->payload_header_MsgId)
    {
    case DK_MESSAGE_ID_FIRST_APPROACH_RQ:
        errCode = msg_supplementary_service_message_first_approach_rq_process(connId, cccDkMsg_t->data, ccc_dk_msg_get_data_len(cccDkMsg_t));
        break;
    case DK_MESSAGE_ID_TIME_SYNC:
        errCode = msg_supplementary_service_message_time_sync_process(connId, cccDkMsg_t->data, ccc_dk_msg_get_data_len(cccDkMsg_t));
        break; 
    case DK_MESSAGE_ID_RKE_AUTH_RS:
        errCode = msg_supplementary_service_message_rke_auth_rs_process(connId, cccDkMsg_t->data, ccc_dk_msg_get_data_len(cccDkMsg_t));
        break;
    default:
        break;
    }

    return errCode;
}
