#include "ccc_can.h"
#include "../ccc_sdk/ccc_dk_type.h"
#include "../ccc_sdk/ccc_dk_api.h"
#include "../ccc_sdk/ccc_dk.h"
#include "../ccc_sdk/ccc_timer.h"
#include "../ccc_msg/ccc_dk_ble_msg.h"
#include "../ccc_util/ccc_dk_util.h"
#include "../ccc_msg/ccc_dk_ble_msg_event_notification.h"
#include "../../../aEM/EM00040101_log/EM000401.h"

void ccc_can_send_uwb_anchor_wakeup_rq(LocInd_e locInd, u8 connId)
{
    u8 pkg[0x10];
    u8 off = 0;
    pkg[off++] = CAN_PKG_ID_UWB_ANCHOR_WAKEUP_RQ;
    pkg[off++] = locInd;

    cccConfig->sendData(CHANNEL_ID_SPI_UWB, pkg, off);   //通过SPI发给主锚点

    cccConfig->sendData(CHANNEL_ID_CAN, pkg, off);       //通过CANFD发给其他锚点
    vehicleInfo.respondersNum = 0;
    core_mm_set(vehicleInfo.responderInfo, RESPONDER_INFO_LEN*ANCHOR_NUM_MAX, 0x00);
    devicesInfo[connId].processOn = PROCESS_CAPABILITY_EXCHANGE;

    ccc_timer_start(connId, TIMER_ID_RANGING_SET_UP, TIMER_TYPE_ONCE, TIMEOUT_MS_ON_ANCHOR_WAKEUP, (pFunc_timer_handler)ccc_timer_handler_on_anchor_wakeup_finish);
}

/**
 * @brief 锚点唤醒响应
 * 
 * @param pdata 
 * @return CCCErrCode_e 
 */
CCCErrCode_e ccc_can_pkg_process_uwb_ahchors_wakeup_rs(u8* pdata)
{
    u16 off = 0;
    u8 index = 0;

    if(pdata[0x00] == 0x00)  //锚点响应正常
    {
        pdata[0x00] = 0x01;  //wake up正常
        index = ccc_dk_get_index_by_locInd(pdata[0x11]);
        core_mm_copy(vehicleInfo.responderInfo + index * RESPONDER_INFO_LEN, pdata, 0x12);  //responseCode + SN(0x10)  + loc_ind(0x01)
        vehicleInfo.respondersNum++;

        off = 0x13;
        core_mm_copy(vehicleInfo.supportedUWBConfigIdLV, pdata + off, (1 + pdata[off]));
        off += (1 + pdata[off]);

        core_mm_copy(vehicleInfo.supportedPulseShapeComboLV, pdata + off, (1 + pdata[off]));
        off += (1 + pdata[off]);

        vehicleInfo.supportedChannelBitmask = pdata[off++];

        vehicleInfo.supportedRANMultiplierMin = pdata[off++];

        vehicleInfo.supprtedSlotBitMask = pdata[off++];

        core_mm_copy(vehicleInfo.supportedSYNCCodeIndex, pdata + off, 4);
        off += 4;

        vehicleInfo.supportedHoppingConfigBitmask = pdata[off++];

        return CCC_ERR_CODE_SUCCESS;
    }
    else
    {
        return CCC_ERR_CODE_ANCHOR_STATE_ERR;
    }
}

void ccc_can_send_uwb_ranging_session_setup_rq(u8 connId)
{
    u8 pkg[0x50];
    u8 off = 0;
    u8 i = 0;
    u8 j = 1;

    pkg[off++] = CAN_PKG_ID_UWB_RANGING_SESSION_SETUP_RQ;

    //devicesInfo[connId].rangingSession.uwbSessionId[0] = 0x00;
    //devicesInfo[connId].rangingSession.uwbSessionId[1] = 0x00;
    //devicesInfo[connId].rangingSession.uwbSessionId[2] = 0x00;
    //devicesInfo[connId].rangingSession.uwbSessionId[3] = 0x03;

    core_mm_copy(pkg + off, devicesInfo[connId].rangingSession.uwbSessionId, 0x04); 
    off += 4;

    core_mm_copy(pkg + off, devicesInfo[connId].rangingSession.selectedUWBConfigId, 0x02); 
    off += 0x02;

    pkg[off++] = devicesInfo[connId].rangingSession.selectedPulseShapeCombo;

    pkg[off++] = devicesInfo[connId].rangingSession.selectedRANMultiplier;

    pkg[off++] = devicesInfo[connId].rangingSession.selectedUWBChannel;

    pkg[off++] = devicesInfo[connId].rangingSession.selectedNChapPerSlot;

    pkg[off++] = devicesInfo[connId].rangingSession.selectedNumSlotsPerRound;

    pkg[off++] = devicesInfo[connId].rangingSession.selectedHoppingConfigBitmask;

    core_mm_copy(pkg + off, devicesInfo[connId].rangingSession.stsIndex0, 0x04); 
    off += 0x04;

    core_mm_copy(pkg + off, devicesInfo[connId].rangingSession.uwbTime0, 0x08); 
    off += 0x08;

    core_mm_copy(pkg + off, devicesInfo[connId].rangingSession.hopModeKey, 0x04); 
    off += 0x04;

    pkg[off++] = devicesInfo[connId].rangingSession.selectedSyncCodeIndex;

    pkg[off++] = devicesInfo[connId].rangingSession.rangingFrequency;

    pkg[off++] = devicesInfo[connId].rangingSession.rangingAnchorNum;

    core_mm_set(devicesInfo->rangingSession.rangingAnchorInfo, 0x00, 2*ANCHOR_NUM_MAX);
    for(i = 0; i < ANCHOR_NUM_MAX; i++)
    {
        if(vehicleInfo.responderInfo[RESPONDER_INFO_LEN*i + OFF_RESPONDER_STATE] == 0x02)  //valid anchor
        {
            pkg[off++] = vehicleInfo.responderInfo[RESPONDER_INFO_LEN*i + OFF_RESPONDER_LOC_IND];   //位置标识
            pkg[off++] = j++;   //responder index;

            devicesInfo->rangingSession.rangingAnchorInfo[i*2] = vehicleInfo.responderInfo[RESPONDER_INFO_LEN*i + OFF_RESPONDER_LOC_IND];
            devicesInfo->rangingSession.rangingAnchorInfo[i*2 + 1] = RANGING_SESSION_SETUP;
        }
    }

    cccConfig->sendData(CHANNEL_ID_SPI_UWB, pkg, off);   //通过SPI发给主锚点
    OSA_TimeDelay(100);
    cccConfig->sendData(CHANNEL_ID_CAN, pkg, off);       //通过CANFD发给其他锚点
}

void ccc_can_send_uwb_ranging_session_start_rq(u8 connId)
{
    u8 pkg[0x30];
    u8 off = 0;

    pkg[off++] = CAN_PKG_ID_UWB_RANGING_SESSION_START_RQ;

    core_mm_copy(pkg + off, devicesInfo[connId].rangingSession.uwbSessionId, 0x04); 
    off += 4;

    core_mm_copy(pkg + off, devicesInfo[connId].rangingSession.ursk, 0x20); 
    off += 0x20;

    cccConfig->sendData(CHANNEL_ID_SPI_UWB, pkg, off);   //通过SPI发给主锚点

    cccConfig->sendData(CHANNEL_ID_CAN, pkg, off);       //通过CANFD发给其他锚点

#ifdef CCC_LOCATION_IN_MCU_SDK
    ccc_loc_func_initial_location_on_ranging_start();
#endif

    devicesInfo[connId].rangingSession.rangingAction = RANGING_ACTION_SETUP;
    ccc_timer_start(connId, TIMER_ID_RANGING_SET_UP, TIMER_TYPE_ONCE, TIMEOUT_MS_ON_RANGING_SETUP, (pFunc_timer_handler)ccc_timer_handler_on_anchor_ranging_setup_finish);

#ifndef CCC_LOCATION_IN_MCU_SDK
    devicesInfo[connId].rangingSession.isLocExpired = 1;
    ccc_timer_start_vehicle(connId, TIMER_ID_LOCATION, TIMER_TYPE_REPEAT, TIMEOUT_MS_ON_TIME_LOCATION_EXPIRED, (pFunc_timer_handler)ccc_timer_handler_ranging_location);
#endif
}

CCCErrCode_e ccc_can_pkg_process_ranging_session_setup_rs(u8* pdata)
{
    u8 connId;
    u8 succssAnchorNum = 0;
    u8 i = 0;
    u8 index = 0;

    if(pdata[0] == 0x00) //response code == success
    {
        for(connId = CHANNEL_ID_BLE_0; connId <= CHANNEL_ID_BLE_FOB; connId++)
        {
            if(core_mm_compare(devicesInfo[connId].rangingSession.uwbSessionId, pdata + 1, 0x04) == 0x00)
            {
                if(devicesInfo[connId].rangingSession.rangingAction == RANGING_ACTION_SETUP)
                {
                	index = ccc_dk_get_index_by_locInd(pdata[5]);
                	devicesInfo[connId].rangingSession.rangingAnchorInfo[index*2 + 1] = URSK_STATUS_ACTIVE;

                	return CCC_ERR_CODE_SUCCESS;
                }
            }
        }
    }
    

    return CCC_ERR_CODE_RANGING_SESSION_UNEIXST;
}

/**
 * @brief 
 * 
 * @param connId 
 * @param actionType: 01: suspend; 02: recover; 3: configurable recover; 04: delete
 */
void ccc_can_send_uwb_ranging_session_suspend_recover_delete_rq(u8 connId, RangingAction_e actionType)
{
    u8 pkg[0x50];
    u8 off = 0;
    pkg[off++] = CAN_PKG_ID_UWB_RANGING_SESSION_SUSPEND_RECOVER_DELETE_RQ;
    pkg[off++] = actionType;

    core_mm_copy(pkg + off, devicesInfo[connId].rangingSession.uwbSessionId, 0x04); 
    off += 4;

    if(actionType == RANGING_ACTION_RECOVER || actionType == RANGING_ACTION_CONFIGURABLE_RECOVER)
    {   
        core_mm_copy(pkg + off, devicesInfo[connId].rangingSession.stsIndex0, 0x04); 
        off += 0x04;

        core_mm_copy(pkg + off, devicesInfo[connId].rangingSession.uwbTime0, 0x08); 
        off += 0x08;

        if(actionType == RANGING_ACTION_CONFIGURABLE_RECOVER)
        {
            pkg[off++] = devicesInfo[connId].rangingSession.selectedRANMultiplier;
        }

#ifdef CCC_LOCATION_IN_MCU_SDK
        ccc_loc_func_initial_location_on_ranging_start();
#endif
    }

    cccConfig->sendData(CHANNEL_ID_SPI_UWB, pkg, off);   //通过SPI发给主锚点
    
    cccConfig->sendData(CHANNEL_ID_CAN, pkg, off);       //通过CANFD发给其他锚点
    devicesInfo[connId].rangingSession.rangingAction = actionType;
    ccc_timer_start(connId, TIMER_ID_RANGING_SET_UP, TIMER_TYPE_ONCE, TIMEOUT_MS_ON_RANGING_SETUP, (pFunc_timer_handler)ccc_timer_handler_on_anchor_ranging_suspend_recover_delete_finish);
}


CCCErrCode_e ccc_can_pkg_process_ranging_session_suspend_recover_delete_rs(u8* pdata)
{
    u8 connId;
    u8 index = 0;

    for(connId = CHANNEL_ID_BLE_0; connId <= CHANNEL_ID_BLE_FOB; connId++)
    {
        if(core_mm_compare(devicesInfo[connId].rangingSession.uwbSessionId, pdata + 2, 0x04) == 0x00)
        {
            if(devicesInfo[connId].rangingSession.rangingAction != pdata[1])
            {
                return CCC_ERR_CODE_ACTION_NOT_PERMIT;  //状态与操作不符合
            }

            index = ccc_dk_get_index_by_locInd(pdata[6]);
            if(pdata[0] == 0x00)  //result == success
            {
                if(devicesInfo[connId].rangingSession.rangingAction == RANGING_ACTION_SUSPEND)
                {
                    devicesInfo[connId].rangingSession.rangingAnchorInfo[index*2 + 1] = URSK_STATUS_SUSPENDED;
                }
                else if(devicesInfo[connId].rangingSession.rangingAction == RANGING_ACTION_RECOVER
                    || devicesInfo[connId].rangingSession.rangingAction == RANGING_ACTION_CONFIGURABLE_RECOVER)
                {
                    devicesInfo[connId].rangingSession.rangingAnchorInfo[index*2 + 1] = URSK_STATUS_ACTIVE;
                }
                else
                {
                    devicesInfo[connId].rangingSession.rangingAnchorInfo[index*2 + 1] = URSK_STATUS_EMPTY;
                }
            }
                        
            return CCC_ERR_CODE_SUCCESS;
        }
    }

    return CCC_ERR_CODE_RANGING_SESSION_UNEIXST;
}

void ccc_can_send_time_sync_rq(u8* timeSyncPar, u8 parLen)
{
    u8 pkg[0x20];
    u8 off = 0;
    pkg[off++] = CAN_PKG_ID_TIME_SYNC_RQ;
    core_mm_copy(pkg + off, timeSyncPar, parLen);
    off += parLen;

    cccConfig->sendData(CHANNEL_ID_SPI_UWB, pkg, off);   //通过SPI发给主锚点
    cccConfig->sendData(CHANNEL_ID_CAN, pkg, off);       //通过CANFD发给其他锚点

    //devicesInfo[connId].processOn = PROCESS_CAPABILITY_EXCHANGE;
}

void ccc_can_time_sync_rs_on_procedure1(u8* pData)
{
    u8 index = 0;
    //Response Code + LOC_IND
    if(pData[0] == 0x00) //success
    {
        index = ccc_dk_get_index_by_locInd(pData[1]);
        if(vehicleInfo.responderInfo[RESPONDER_INFO_LEN * index + OFF_RESPONDER_STATE] == 0x01)
        {
            vehicleInfo.responderInfo[RESPONDER_INFO_LEN * index + OFF_RESPONDER_STATE] = 0x02;
        }
    }
}


void ccc_location_result_process(u8 connId)
{
    if(devicesInfo[connId].workMode == DEVICE_WORK_MODE_ON_PAIRING)  //in pairing
    {
        if(!(devicesInfo[connId].rangingSession.lastLoc == LOC_DRIVER_SEAT && 
            devicesInfo[connId].rangingSession.thisLoc == LOC_DRIVER_SEAT) )  //不在主驾  
        {
            send_command_complete_subevent_in_dk_event_notification(connId, Command_Status_OP_Device_not_inside_vehicle);
        }
        else
        {
            //在车内
            prepare_and_send_select_aid_to_device_se_with_ble(connId, 0x01);
            devicesInfo[connId].processOn = PROCESS_OWNER_PAIRING_PHASE3;
            devicesInfo[connId].subProcessOn = PAIRING_PHASE3_STATUS_SELECT;

            //uwb_ranging_service_ranging_session_suspend_request(connId);  //车端请求暂停本次定位
        }
    }
    else if(devicesInfo[connId].workMode == DEVICE_WORK_MODE_ON_FIRST_APPROACH) //no in pairing;
    {
        //TODO
    }

    if(devicesInfo[connId].rangingSession.lastLoc == devicesInfo[connId].rangingSession.thisLoc)
    {
        devicesInfo[connId].rangingSession.locFixNum ++;  //位置固定次数++
    }
    else
    {
        devicesInfo[connId].rangingSession.locFixNum = 0;  //位置有变动,固定次数复位
    }

    if(devicesInfo[connId].rangingSession.sessionStatus == RANGING_SESSION_SUSPEND)
    {
        //由车端或device端已停止定位；
        ccc_can_send_uwb_ranging_session_suspend_recover_delete_rq(connId, RANGING_ACTION_SUSPEND);  //suspend
    }
    else if(devicesInfo[connId].rangingSession.sessionStatus != RANGING_SESSION_ACTIVE)
    {
        LOG_L_S(CCC_MD, "TERMINATED22222222\r\n");
        //ccc_can_send_uwb_ranging_session_suspend_recover_delete_rq(connId, RANGING_ACTION_DELETE);  //delete
    }
    else
    {
#if 0
        if(devicesInfo[connId].rangingSession.locFixNum >= RANGING_LOCATION_FIXED_NUM_TO_SUSPEND)
        {
            uwb_ranging_service_ranging_session_suspend_request(connId);  //一定时间内定位区域不变，车端请求暂停本次定位
        }
        else if(devicesInfo[connId].rangingSession.rangingResultNoticeNum >= RANGING_RESULT_NOTICE_NUM_TO_SUSPEND)
        {
            //本次定位超过总时限，车端请求暂停本次定位
            uwb_ranging_service_ranging_session_suspend_request(connId); 
        }
#endif
    } 

    if(vehicleInfo.privateBleConnect)   //定位结果通知APP
    {
        ccc_timer_handler_ranging_result_notice_to_ble_in_task(CHANNEL_ID_BLE_PRIVATE);
    }
}

/**
 * @brief 定位失败，停止本次定位
 * 
 * @param sessionId 
 * @param needRestart : 停止后是否需要重新开启定位
 */
void ccc_ranging_on_failed(u8* sessionId, u8 needRestart)
{
    u8 connId = CHANNEL_ID_BLE_FOB;

	for(connId = CHANNEL_ID_BLE_0; connId <= CHANNEL_ID_BLE_FOB; connId++)
	{
	    if(core_mm_compare(devicesInfo[connId].rangingSession.uwbSessionId, sessionId, 0x04) == 0x00)
        {
            if(devicesInfo[connId].rangingSession.sessionStatus == RANGING_SESSION_ACTIVE)
            {
                LOG_L_S(CCC_MD, "TERMINATED3333333\r\n");
                send_ranging_session_status_changed_subevent_in_dk_event_notification(connId, Session_Status_Ranging_session_terminated);
	            ccc_dk_set_ranging_session_status(connId, devicesInfo[connId].rangingSession.uwbSessionId, URSK_STATUS_EMPTY);
	            ccc_can_send_uwb_ranging_session_suspend_recover_delete_rq(connId, RANGING_ACTION_DELETE);  //terminate
	            
                if(needRestart)
                {
                    ccc_timer_start(connId, TIMER_ID_GENERATE_URSK, TIMER_TYPE_ONCE, 1000, ccc_timer_handler_URSK_Derivation);
                }
            }
            
            break;
        }
	}
}

#ifdef CCC_LOCATION_IN_MCU_SDK
void ccc_can_pkg_process_ranging_result_notice(u8* pdata)
{
    //response code(1) + UWB_Session_Id(4) + STS index(4) + LOC_IND(1)  + Distance(2) * 5
    u8 i;
    u16 distancs[NUM_DISTANCES_IN_BATCH];
    for(i = 0; i < NUM_DISTANCES_IN_BATCH; i++)
    {
#if 1
        if(pdata[10 + i * 2] == 0xFF && pdata[11 + i * 2] == 0xFF)
        {
            distancs[i] = core_dcm_mku16(pdata[10 + i * 2], pdata[11 + i * 2]);
        }
        else
        {
            distancs[i] = core_dcm_mku16((pdata[10 + i * 2] & 0x7F), pdata[11 + i * 2]);
        }
#else
        distancs[i] = core_dcm_mku16(pdata[10 + i * 2], pdata[11 + i * 2]);
#endif
    }

    ccc_loc_func_ranging_distance_notice_in_batch(pdata + 1, pdata[9], distancs);
}
#endif

void ccc_can_process_ranging_location_notice(u8* pdata)
{
    u8 connId;
    u8 anchorNum;
    u8 index;
    for(connId = CHANNEL_ID_BLE_0; connId <= CHANNEL_ID_BLE_FOB; connId++)
    {
        if(core_mm_compare(devicesInfo[connId].rangingSession.uwbSessionId, pdata, 0x04) == 0x00)
        {
            devicesInfo[connId].rangingSession.isLocExpired = 0;  //收到定位结果响应，定位未超时

#if 1
            if(check_ttl_on_ranging(connId) == 0x02 || core_dcm_readBig32(pdata + 4) >= 0x5FFFA0) //TTL Exceed
            {
                //terminate the ranging session
                LOG_L_S(CCC_MD, "TERMINATED666666666\r\n");          
                ccc_ranging_on_failed(devicesInfo[connId].rangingSession.uwbSessionId, 1);
            }
            else
#endif
            {
                core_mm_copy(devicesInfo[connId].rangingSession.stsIndexInFinalData, pdata + 4, 4);
            
                devicesInfo[connId].rangingSession.rangingResult = pdata[8];

                switch (devicesInfo[connId].rangingSession.rangingResult)
                {
                case RANGING_RESULT_SUCCESS:
                    devicesInfo[connId].rangingSession.lastLoc = devicesInfo[connId].rangingSession.thisLoc;
                    devicesInfo[connId].rangingSession.thisLoc = pdata[9];

                    core_mm_copy(devicesInfo[connId].rangingSession.pointX, pdata + 10, 0x02);
                    core_mm_copy(devicesInfo[connId].rangingSession.pointY, pdata + 12, 0x02);

                    for(index = 0; index < ANCHOR_NUM_MAX; index++)
                    {
                        devicesInfo[connId].rangingSession.anchorRangingResult[4* index + 1] = 0x01;  //默认无测距数据，不然会上送APP之前的数据
                    }

                    for(anchorNum = 0; anchorNum < pdata[14] && anchorNum < ANCHOR_NUM_MAX; anchorNum++)
                    {
                        index = pdata[15 + 4*anchorNum];
                        index = get_bit1_index(index);

                        if(index != 0xFF)
                        {
                            core_mm_copy(devicesInfo[connId].rangingSession.anchorRangingResult + 4* index, pdata + 15 + 4*anchorNum, 0x04);
                        }
                    }

                    devicesInfo[connId].rangingSession.locWithBorderType = pdata[15 + 4*anchorNum];
                    ccc_location_result_process(connId);
                    break;
                case RANGING_RESULT_URSK_LOST_OR_NOT_EXIST:
                case RANGING_RESULT_STS_EXCEED:
                case RANGING_SESSION_NOT_EXIST:
                    //清除当前URSK，重新协商URSK
                    LOG_L_S(CCC_MD, "TERMINATED777777777\r\n");
                    ccc_ranging_on_failed(devicesInfo[connId].rangingSession.uwbSessionId, 1);
                    break;
                case RANGING_ANCHOR_OFFLINE:
                case RANGING_ANCHOR_ABNORMAL_COMMUNICATION:
                default:  //TODO
                    break;
                }
            } 

            break;
        }
    }
}

/**
 * @brief 
 * 
 * @param connId 
 * @param funcId 
 * @param actionId 
 * @param executionType : 01: start rke; 02:stop rke;
 */
void ccc_can_send_rke_execute_rq(u8 connId, u16 funcId, u8 actionId, u8 executionType)
{
    u8 pkg[0x10];
    u8 off = 0;
    pkg[off++] = CAN_PKG_ID_RKE_EXECUTE_RQ;
    pkg[off++] = core_dcm_u16_hi(funcId);
    pkg[off++] = core_dcm_u16_lo(funcId);
    pkg[off++] = core_dcm_u16_lo(actionId);
    pkg[off++] = core_dcm_u16_lo(executionType);

    cccConfig->sendData(CHANNEL_ID_CAN, pkg, off);
}

void ccc_can_process_rke_execute_rs(u8* pdata)
{
    if(pdata[0] == 0x00)
    {
        if(vehicleInfo.rkeRequest_t.rkeStatus == RKE_STATUS_ENDURING)
    {
        if(core_dcm_readBig16(pdata + 1) == vehicleInfo.rkeRequest_t.functionId &&
            pdata[3] == vehicleInfo.rkeRequest_t.actionId)
        {
            cccConfig->timerTool->stopTimer(vehicleInfo.timers[TIMER_ID_RKE].timerId);

            send_last_requested_action_execution_and_function_status_on_vehicle_status_changed_subevent(
                vehicleInfo.rkeRequest_t.connId, pdata[4], pdata[5], NULL, 0);
            vehicleInfo.rkeRequest_t.rkeStatus = RKE_STATUS_FINISH;
        }
    }
    }
}

CCCErrCode_e ccc_can_pkg_process(u8* pdata, u16 pdataLen)
{
    CCCErrCode_e errCode = CCC_ERR_CODE_SUCCESS;

    u8 canId = pdata[0];
    u8 chId;

    switch (canId)
    {
    case CAN_PKG_ID_UWB_ANCHOR_WAKEUP_RS:
        errCode = ccc_can_pkg_process_uwb_ahchors_wakeup_rs(pdata + 1);
        break;
    //case CAN_PKG_ID_UWB_ANCHORS_INFO_NOTICE_RQ:
        //break;
    case CAN_PKG_ID_TIME_SYNC_RS:
        ccc_can_time_sync_rs_on_procedure1(pdata + 1);
        break;
    case CAN_PKG_ID_UWB_RANGING_SESSION_START_RS:
        ccc_can_pkg_process_ranging_session_setup_rs(pdata + 1);
        break;
    //case CAN_PKG_ID_UWB_RANGING_SESSION_SETUP_RQ_RESULT_NOTICE:
        //break;
    case CAN_PKG_ID_UWB_RANGING_SESSION_SUSPEND_RECOVER_DELETE_RS:
        ccc_can_pkg_process_ranging_session_suspend_recover_delete_rs(pdata + 1);
        break;
    //case CAN_PKG_ID_UWB_RANGING_SESSION_SUSPEND_RECOVER_DELETE_RESULT_NOTICE:
        //break;
#ifdef CCC_LOCATION_IN_MCU_SDK
    case CAN_PKG_ID_UWB_RANGING_RESULT_NOTICE:
        ccc_can_pkg_process_ranging_result_notice(pdata + 1);
        break;
#endif
    case CAN_PKG_ID_UWB_RANGING_LOCATION_NOTICE:
        ccc_can_process_ranging_location_notice(pdata + 1);
        break;
    case CAN_PKG_ID_VEHICLE_STATUS_NOTICE:
        break;
    case CAN_PKG_ID_RKE_EXECUTE_RS:
        ccc_can_process_rke_execute_rs(pdata + 1);
        break;
    default:
        break;
    }

    return errCode;
}

