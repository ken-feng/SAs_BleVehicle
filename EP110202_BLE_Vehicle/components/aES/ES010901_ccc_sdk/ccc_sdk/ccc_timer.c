#include "ccc_timer.h"
#include "ccc_dk.h"
#include "../ccc_protocol/ccc_can.h"
#include "../ccc_msg/ccc_dk_ble_msg.h"
#include "../ccc_msg/ccc_dk_ble_msg_event_notification.h"
#include "../ccc_msg/ccc_dk_ble_msg_uwb_ranging_service.h"
#include "../../../aEM/EM00010101_platform/EM000101.h"
#include "../../../aEM/EM00040101_log/EM000401.h"
#include "UDS_App.h"

void ccc_timer_create_on_reset(void)
{
    u8 connId;
    u8 timeNum;

    for(timeNum = 0; timeNum < TIME_NUM_IN_VEHICLE; timeNum++)
    {
        vehicleInfo.timers[timeNum].timerId = cccConfig->timerTool->createTimer();
    }

    for(connId = 0; connId <= MAX_BLE_CONN_NUM; connId++)
    {
        for(timeNum = 0; timeNum < TIME_NUM_IN_DEVICE; timeNum++)
        {
            devicesInfo[connId].timers[timeNum].timerId = cccConfig->timerTool->createTimer();
        }
    }
}

int ccc_timer_start(u8 connId, u8 timeIndex, u8 timerType, u32 timeInMilliseconds, pFunc_timer_handler timerHandler)
{
    devicesInfo[connId].timers[timeIndex].timerPar[0] = connId;
    return cccConfig->timerTool->startTimer(devicesInfo[connId].timers[timeIndex].timerId, timerType, 
        timeInMilliseconds, timerHandler, devicesInfo[connId].timers[timeIndex].timerPar);
}

int ccc_timer_start_vehicle(u8 connId, u8 timeIndex, u8 timerType, u32 timeInMilliseconds, pFunc_timer_handler timerHandler)
{
    vehicleInfo.timers[timeIndex].timerPar[0] = connId;
    return cccConfig->timerTool->startTimer(vehicleInfo.timers[timeIndex].timerId, timerType, 
        timeInMilliseconds, timerHandler, vehicleInfo.timers[timeIndex].timerPar);
}

void ccc_timer_handler_to_task(void* param, TIMER_HANDLER_TYPE_e timeHandlerType)
{
    u8 connId = ((u8*)param)[0];
    u8 par[2]; 

    par[0] = timeHandlerType;
    par[1] = connId;
    cccConfig->eventNotice(connId, SDK_EVENT_TIMER_HANDLER, par, 2);
}

void ccc_timer_handler_pairing_timeOut(void* param)
{
    ccc_timer_handler_to_task(param, TIMER_HANDLER_TYPE_PAIRING_TIMEOUT);
}

void ccc_timer_handler_pairing_timeOut_in_task(u8 connId)
{
    //配对超时，断开蓝牙连接
    //u8 connId = ((u8*)param)[0];
    if(devicesInfo[connId].rangingSession.sessionStatus == RANGING_SESSION_ACTIVE)
    {
        //如果在测试距，则terminate掉；
        ccc_ranging_on_failed(devicesInfo[connId].rangingSession.uwbSessionId, 0);
    }

    cccConfig->eventNotice(connId, SDK_EVENT_BLE_DISCONECT, NULL, 0);
}

void ccc_timer_handler_URSK_Derivation(void* param)
{
    ccc_timer_handler_to_task(param, TIMER_HANDLER_TYPE_URSK_DERIVATION);
}

void ccc_timer_handler_URSK_Derivation_in_task(u8 connId)
{
    //u8 connId = ((u8*)param)[0];
    prepare_and_send_select_aid_to_device_se_with_ble(connId, 0x01);
    devicesInfo[connId].processOn = PROCESS_URSK_DERIVATION_FLOW;
    devicesInfo[connId].subProcessOn = URSK_DERIVATION_STATUS_SELECT;
}

void ccc_timer_handler_timesync_triggered_by_vehicle(void* param)
{
    ccc_timer_handler_to_task(param, TIMER_HANDLER_TYPE_TIME_SYNC_BY_VEHICLE);
}

void ccc_timer_handler_timesync_triggered_by_vehicle_in_task(u8 connId)
{
    //When the vehicle determines a time synchronization is required, the host (vehicle) shall send an 
    //“LE Set PHY” command which triggers sending the “LL PHY REQ” to the device

    //u8 connId = ((u8*)param)[0];
    devicesInfo[connId].timeSyncTriggerConditions = 1L;  //Procedure 1
    cccConfig->eventNotice(connId, SDK_EVENT_TIME_SYNC_TRIGGER_BY_VEHICLE, NULL, 0);
    devicesInfo[connId].processOn = PROCESS_TIME_SYNC;
}

void ccc_timer_handler_kts_response_received_callback(void* param)
{
    ccc_timer_handler_to_task(param, TIMER_HANDLER_TYPE_KTS_RESP_RECEIVED);
}

void ccc_timer_handler_kts_response_received_callback_in_task(u8 connId)
{
    //u8 connId = ((u8*)param)[0];

    //TODO KTS 默认成功;
    devicesInfo[connId].processOn = PROCESS_OWNER_PAIRING_PHASE4;

    prepare_and_send_select_aid_to_device_se_with_ble(connId, 0x01);
    devicesInfo[connId].processOn = PROCESS_OWNER_PAIRING_PHASE4;
    devicesInfo[connId].subProcessOn = PAIRING_PHASE4_STATUS_SELECT;
}

void ccc_timer_handler_on_device_se_busy(void *param)
{
    ccc_timer_handler_to_task(param, TIMER_HANDLER_TYPE_DEVICE_SE_BUSY);
}

void ccc_timer_handler_on_device_se_busy_in_task(u8 connId)
{
    //u8 connId = ((u8*)param)[0];

    prepare_and_send_select_aid_to_device_se_with_ble(connId, 0x01);

    devicesInfo[connId].processOn = PROCESS_STANDARD_TRANSACTION_FLOW;
    devicesInfo[connId].subProcessOn = STANDARD_TRANSACTION_STATUS_SELECT;
}

void ccc_timer_handler_on_rke_request_confirm_continuation_time_out(void *param)
{
    ccc_timer_handler_to_task(param, TIMER_HANDLER_TYPE_REQUEST_CONFIRM_CONTINUATION_TIMEOUT);
}

/**
 * @brief endring RKE 在接收confirm continuation time out，停掉原来的车控；
 * 
 * @param param 
 * @return u8 
 */
void ccc_timer_handler_on_rke_request_confirm_continuation_time_out_in_task(u8 connId)
{
    //u8 connId = ((u8*)param)[0];

    ccc_can_send_rke_execute_rq(connId, vehicleInfo.rkeRequest_t.functionId, vehicleInfo.rkeRequest_t.actionId, 0x02);  //stop

    send_last_requested_action_execution_status_on_vehicle_status_changed_subevent(connId, DEVICE_RESPONSE_TIMEOUT);  //手机端响应超时

    rke_reset_on_finish();
}

void ccc_timer_handler_on_rke_time_out(void *param)
{
    ccc_timer_handler_to_task(param, TIMER_HANDLER_TYPE_ON_RKE_TIMEOUT);
}

/**
 * @brief 车控车端响应超时
 * 
 * @param param 
 * @return u8 
 */
void ccc_timer_handler_on_rke_time_out_in_task(u8 connId)
{
    //u8 connId = ((u8*)param)[0];

    send_last_requested_action_execution_status_on_vehicle_status_changed_subevent(connId, VEHICLE_RESPONSE_TIMEOUT);  //车端响应超时，状态码0x13可能不对

    rke_reset_on_finish();
}

void ccc_timer_handler_on_anchor_wakeup_finish(void *param)
{
    ccc_timer_handler_to_task(param, TIMER_HANDLER_TYPE_ON_ANCHOR_WAKEUP_FINISH);
}

void ccc_timer_handler_on_anchor_wakeup_finish_in_task(u8 connId)
{
    //u8 connId = ((u8*)param)[0];

    if(devicesInfo[connId].processOn == PROCESS_CAPABILITY_EXCHANGE)
    {
        if(vehicleInfo.respondersNum < MIN_VALID_ANCHOR_NUM_TO_RANGING)
        {
            LOG_L_S_HEX(CCC_MD,"insuffcient anchor wakeup on ranging!", &connId, 0);
            return;  //准备定位时，正常锚点数不够;
        }
        else
        {
            //devicesInfo[connId].rangingSession.rangingAnchorNum = vehicleInfo.respondersNum;
            ranging_capability_request(connId);
        }
    }
}

void ccc_timer_handler_on_anchor_ranging_setup_finish(void *param)
{
    ccc_timer_handler_to_task(param, TIMER_HANDLER_TYPE_ON_RANGING_SETUP_FINISH);
}

void ccc_timer_handler_on_anchor_ranging_setup_finish_in_task(u8 connId)
{
    //u8 connId = ((u8*)param)[0];
    u8 i = 0;
    u8 succssAnchorNum = 0;

    if(devicesInfo[connId].rangingSession.rangingAction != RANGING_ACTION_SETUP)
    {
        return;
    }

    devicesInfo[connId].rangingSession.rangingAction = RANGING_ACTION_NONE;

    for(i = 0; i < ANCHOR_NUM_MAX; i++)
    {
        if(devicesInfo[connId].rangingSession.rangingAnchorInfo[i*2 + 1] == URSK_STATUS_ACTIVE)
        {
            succssAnchorNum ++;
        }
    }

    if(succssAnchorNum < MIN_VALID_ANCHOR_NUM_TO_RANGING)
    {
        //3个以下锚点成功，本次定位无法确定位置，需要terminate本次定位;
        ccc_ranging_on_failed(devicesInfo[connId].rangingSession.uwbSessionId, 0);

        LOG_L_S_HEX(CCC_MD,"insuffcient anchor setup on ranging!", &connId, 0);
        return;  // CCC_ERR_CODE_INSUFFICIENT_ANCHR;
    }

    ccc_dk_set_ranging_session_status(connId, devicesInfo[connId].rangingSession.uwbSessionId, URSK_STATUS_ACTIVE);
    vehicleInfo.chIdOnRanging = connId;

#ifdef CCC_LOCATION_IN_MCU_SDK
    ccc_timer_start_vehicle(connId, TIMER_ID_LOCATION, TIMER_TYPE_REPEAT, TIMEOUT_MS_ON_TIME_LOCATION, (pFunc_timer_handler)ccc_timer_handler_ranging_location);
#endif
}

void ccc_timer_handler_on_anchor_ranging_suspend_recover_delete_finish(void *param)
{
    ccc_timer_handler_to_task(param, TIMER_HANDLER_TYPE_ON_RANGING_SUSPEND_RECOVER_DELETE_FINISH);
}

void ccc_timer_handler_on_anchor_ranging_suspend_recover_delete_finish_in_task(u8 connId)
{
    //u8 connId = ((u8*)param)[0];
    u8 failedAnchorNum = 0;
    u8 i = 0;
    RangingAction_e rangingAction = devicesInfo[connId].rangingSession.rangingAction;
    u8 tmpBuf[0x03];

    if(rangingAction == RANGING_ACTION_SUSPEND)
    {
        for(i = 0; i < ANCHOR_NUM_MAX; i++)
        {
            if(devicesInfo[connId].rangingSession.rangingAnchorInfo[2*i + 1] != URSK_STATUS_SUSPENDED)
            {
                failedAnchorNum++;
            }
        }

        ccc_dk_set_ranging_session_status(connId, devicesInfo[connId].rangingSession.uwbSessionId, URSK_STATUS_SUSPENDED);
        vehicleInfo.chIdOnRanging = 0xFF;

//#ifdef CCC_LOCATION_IN_MCU_SDK
        cccConfig->timerTool->stopTimer(vehicleInfo.timers[TIMER_ID_LOCATION].timerId);
//#endif
    }
    else if(rangingAction == RANGING_ACTION_RECOVER 
        || rangingAction == RANGING_ACTION_CONFIGURABLE_RECOVER)
    {
        for(i = 0; i < ANCHOR_NUM_MAX; i++)
        {
            if(devicesInfo[connId].rangingSession.rangingAnchorInfo[2*i + 1] != URSK_STATUS_ACTIVE)
            {
                failedAnchorNum++;
            }
        }

        ccc_dk_set_ranging_session_status(connId, devicesInfo[connId].rangingSession.uwbSessionId, URSK_STATUS_ACTIVE);
        vehicleInfo.chIdOnRanging = connId;
#ifdef CCC_LOCATION_IN_MCU_SDK
        ccc_timer_start_vehicle(connId, TIMER_ID_LOCATION, TIMER_TYPE_REPEAT, TIMEOUT_MS_ON_TIME_LOCATION, (pFunc_timer_handler)ccc_timer_handler_ranging_location);
#else
        devicesInfo[connId].rangingSession.isLocExpired = 1;
        ccc_timer_start_vehicle(connId, TIMER_ID_LOCATION, TIMER_TYPE_REPEAT, TIMEOUT_MS_ON_TIME_LOCATION_EXPIRED, (pFunc_timer_handler)ccc_timer_handler_ranging_location);
#endif
    }
    else if(rangingAction == RANGING_ACTION_DELETE)
    {
        for(i = 0; i < ANCHOR_NUM_MAX; i++)
        {
            if(devicesInfo[connId].rangingSession.rangingAnchorInfo[2*i + 1] != URSK_STATUS_EMPTY)
            {
                failedAnchorNum++;
            }
        }

        ccc_dk_set_ranging_session_status(connId, devicesInfo[connId].rangingSession.uwbSessionId, URSK_STATUS_EMPTY);
        vehicleInfo.chIdOnRanging = 0xFF;

//#ifdef CCC_LOCATION_IN_MCU_SDK
        cccConfig->timerTool->stopTimer(vehicleInfo.timers[TIMER_ID_LOCATION].timerId);
//#endif

        if(vehicleInfo.fotaFlag == 2U)
        {
            tmpBuf[0] = 0x20;
            tmpBuf[1] = 0x04;
            tmpBuf[2] = 0x00;
            uds_res_append_and_send(UDS_CMD_DIAG,tmpBuf,3);

            vehicleInfo.fotaFlag = 0U;
        }
    }

    if(failedAnchorNum != 0)
    {
        if(rangingAction != RANGING_ACTION_DELETE)
        {
            //只要有一个结点响应不是成功，则terminate本次定位；
            ccc_ranging_on_failed(devicesInfo[connId].rangingSession.uwbSessionId, 0);
        }
    }

    devicesInfo[connId].rangingSession.rangingAction = RANGING_ACTION_NONE;

    //return 0x00;
}


void ccc_timer_handler_on_time_sync_on_procedure1(void *param)
{
    ccc_timer_handler_to_task(param, TIMER_HANDLER_TYPE_ON_TIME_SYNC_ON_PROCEDURE1);
}

void ccc_timer_handler_on_time_sync_on_procedure1_in_task(u8 connId)
{
    //u8 connId = ((u8*)param)[0];
    u8 timeSyncSuccNum = 0;
    u8 i = 0;

    if(devicesInfo[connId].timeSyncTriggerConditions == 1)  //Procedure 1
    {
        for(i = 0; i < ANCHOR_NUM_MAX; i++)
        {
            if(vehicleInfo.responderInfo[RESPONDER_INFO_LEN*i + OFF_RESPONDER_STATE] == 0x02)  //time sync success
            {
                timeSyncSuccNum ++;
            }
        }

        //if(timeSyncSuccNum >= MIN_VALID_ANCHOR_NUM_TO_RANGING)
        {
            devicesInfo[connId].rangingSession.rangingAnchorNum = timeSyncSuccNum;
            devicesInfo[connId].rangingSession.rangingFrequency = RANGING_SAMPLE_FREQUENCY;
            //goto set up ranging session;
            ccc_dk_find_ursk_to_ranging(connId, TRUE);
        } 
    }

    //return 0x00;
}

void ccc_timer_handler_ranging_result_notice_to_ble(void *param)
{
    ccc_timer_handler_to_task(param, TIMER_HANDLER_TYPE_RANGING_RESULT_TO_PRIVATE_BLE);
}

void ccc_timer_handler_ranging_result_notice_to_ble_in_task(u8 connId)
{
    //u8 connId = ((u8*)param)[0];
    u8 msg[0x30];
    u8 off = 0;
    u8 offAnchorNum;
    u8 validAnchorNum = 0;
    u8 i;

    if(vehicleInfo.chIdOnRanging >= CHANNEL_ID_BLE_0 && vehicleInfo.chIdOnRanging <= CHANNEL_ID_BLE_FOB)
    {
        if(devicesInfo[vehicleInfo.chIdOnRanging].rangingSession.rangingResult == 0x00)
        {
            msg[off++] = 0x01;  //车端上送FOB位置报文
            core_mm_copy(msg + off, devicesInfo[vehicleInfo.chIdOnRanging].rangingSession.uwbSessionId, 0x04);
            off += 0x04;

            core_mm_copy(msg + off, devicesInfo[vehicleInfo.chIdOnRanging].rangingSession.stsIndexInFinalData, 0x04);
            off += 0x04;

            msg[off++] = devicesInfo[vehicleInfo.chIdOnRanging].rangingSession.thisLoc;

            core_mm_copy(msg + off, devicesInfo[vehicleInfo.chIdOnRanging].rangingSession.pointX, 0x02);
            off += 0x02;

            core_mm_copy(msg + off, devicesInfo[vehicleInfo.chIdOnRanging].rangingSession.pointY, 0x02);
            off += 0x02;

            offAnchorNum = off;

            off++;
            for(i = 0; i < ANCHOR_NUM_MAX; i++)
            {
                if(devicesInfo[vehicleInfo.chIdOnRanging].rangingSession.anchorRangingResult[4*i + 1] == 0x00)
                {
                    validAnchorNum ++;

                    msg[off++] = devicesInfo[vehicleInfo.chIdOnRanging].rangingSession.anchorRangingResult[4*i];  //Loc_Ind
                    core_mm_copy(msg + off, devicesInfo[vehicleInfo.chIdOnRanging].rangingSession.anchorRangingResult + 4* i + 2, 0x02);
                    off += 0x02;
                }
            }

            msg[offAnchorNum] = validAnchorNum;

            msg[off++] = devicesInfo[vehicleInfo.chIdOnRanging].rangingSession.locWithBorderType;

            msg[off++] = (u8)(devicesInfo[vehicleInfo.chIdOnRanging].rssi);

            cccConfig->sendData(connId, msg, off);
            LOG_L_S(CCC_MD, "ranging_location_res_to_app!");
        }
    }

    //return 0x00;
}


void ccc_timer_handler_ranging_location(void *param)
{
    ccc_timer_handler_to_task(param, TIMER_HANDLER_TYPE_LOCATION);
}

void ccc_timer_check_location_result_expired(u8 connId)
{
#if 1
    if(devicesInfo[connId].rangingSession.isLocExpired == 1)   //超时收到定位结果
    {
        cccConfig->timerTool->stopTimer(vehicleInfo.timers[TIMER_ID_LOCATION].timerId);
        devicesInfo[connId].rangingSession.isLocExpired = 0;

        devicesInfo[connId].rangingSession.locRetryTime = 3;   //强制断开；

        if(devicesInfo[connId].rangingSession.locRetryTime < 3)
        {
            devicesInfo[connId].rangingSession.locRetryTime ++;
            LOG_S("location expired with retry: %d \r\n", devicesInfo[connId].rangingSession.locRetryTime);
            ccc_ranging_on_failed(devicesInfo[connId].rangingSession.uwbSessionId, 1);
        }
        else
        {
            LOG_S("location expired without retry \r\n");
            ccc_ranging_on_failed(devicesInfo[connId].rangingSession.uwbSessionId, 0);

            //OSA_TimeDelay(500);
            //cccConfig->eventNotice(connId, SDK_EVENT_BLE_DISCONECT, NULL, 0);   //断开蓝牙
        }
    }
    else
    {
        devicesInfo[connId].rangingSession.isLocExpired = 1;
    }
#endif
}

void ccc_timer_handler_ranging_location_in_task(u8 connId)
{
#ifdef CCC_LOCATION_IN_MCU_SDK
    ccc_loc_func_start_ranging_location();
#else
    ccc_timer_check_location_result_expired(connId);
#endif
}

CCCErrCode_e ccc_timer_handler_in_task(u8* parIn, u16 parInLen)
{
    CCCErrCode_e errCode = CCC_ERR_CODE_SUCCESS;

    switch(parIn[0])
    {
        case TIMER_HANDLER_TYPE_PAIRING_TIMEOUT:
            ccc_timer_handler_pairing_timeOut_in_task(parIn[1]);
            break;
        case TIMER_HANDLER_TYPE_URSK_DERIVATION:
            ccc_timer_handler_URSK_Derivation_in_task(parIn[1]);
            break;
        case TIMER_HANDLER_TYPE_TIME_SYNC_BY_VEHICLE:
            ccc_timer_handler_timesync_triggered_by_vehicle_in_task(parIn[1]);
            break;
        case TIMER_HANDLER_TYPE_KTS_RESP_RECEIVED:
            ccc_timer_handler_kts_response_received_callback_in_task(parIn[1]);
            break;
        case TIMER_HANDLER_TYPE_DEVICE_SE_BUSY:
            ccc_timer_handler_on_device_se_busy_in_task(parIn[1]);
            break;
        case TIMER_HANDLER_TYPE_REQUEST_CONFIRM_CONTINUATION_TIMEOUT:
            ccc_timer_handler_on_rke_request_confirm_continuation_time_out_in_task(parIn[1]);
            break;
        case TIMER_HANDLER_TYPE_ON_RKE_TIMEOUT:
            ccc_timer_handler_on_rke_time_out_in_task(parIn[1]);
            break;
        case TIMER_HANDLER_TYPE_ON_ANCHOR_WAKEUP_FINISH:
            ccc_timer_handler_on_anchor_wakeup_finish_in_task(parIn[1]);
            break;
        case TIMER_HANDLER_TYPE_ON_RANGING_SETUP_FINISH:
            ccc_timer_handler_on_anchor_ranging_setup_finish_in_task(parIn[1]);
            break;
        case TIMER_HANDLER_TYPE_ON_RANGING_SUSPEND_RECOVER_DELETE_FINISH:
            ccc_timer_handler_on_anchor_ranging_suspend_recover_delete_finish_in_task(parIn[1]);
            break;
        case TIMER_HANDLER_TYPE_ON_TIME_SYNC_ON_PROCEDURE1:
            ccc_timer_handler_on_time_sync_on_procedure1_in_task(parIn[1]);
            break;
        case TIMER_HANDLER_TYPE_RANGING_RESULT_TO_PRIVATE_BLE:
            //ccc_timer_handler_ranging_result_notice_to_ble_in_task(parIn[1]);
            //ccc_loc_test_in_loop();  //调试定位库用
            break;
        case TIMER_HANDLER_TYPE_LOCATION:
            ccc_timer_handler_ranging_location_in_task(parIn[1]);
            break;
    }

    return errCode;
}
