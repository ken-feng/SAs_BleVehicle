#include "ccc_dk_type.h"
#include "ccc_dk.h"
#include "ccc_timer.h"
#include "ccc_dk_api.h"
//#include "kalman_filtering.h"
#include "../ccc_util/ccc_dk_util.h"
#include "../ccc_util/ccc_dk_util.h"
#include "../ccc_protocol/ccc_apdu.h"
#include "../ccc_protocol/ccc_can.h"
#include "../ccc_msg/ccc_dk_ble_msg.h"
#include "../ccc_msg/ccc_dk_ble_msg_event_notification.h"
#include "../../../aEM/EM00020101_algo/EM000201.h"
#include "../../../aEM/EM00040101_log/EM000401.h"
#include "../../../aEM/EM00040101_log/EM000401.h"


VehicleInfo_t vehicleInfo;
DeviceInfo_t devicesInfo[(1 + MAX_BLE_CONN_NUM)];   //index = 0, for NFC, others for BLE

void ccc_dk_vehicle_init()
{
    u8 index;

    core_mm_set(vehicleInfo.responderInfo, RESPONDER_INFO_LEN*ANCHOR_NUM_MAX, 0x00);  //清SN
    for(index = 0; index < MAX_DIGITAL_KEY_STORAGE; index ++)
    {
        vehicleInfo.dkUrsks[index].slotIdLV[(short)0] = 0x00;
        vehicleInfo.dkUrsks[index].ursks[0].urskStatus = URSK_STATUS_EMPTY;
        vehicleInfo.dkUrsks[index].ursks[1].urskStatus = URSK_STATUS_EMPTY;
    }

    //RKE
    rke_reset_on_finish();

    vehicleInfo.privateBleConnect = FALSE;
    vehicleInfo.chIdOnRanging = 0xFF;
    vehicleInfo.fotaFlag = 0U;
}

void ccc_dk_device_init(u8 connId)
{
    devicesInfo[connId].workMode = DEVICE_WORK_MODE_ON_IDLE;
    devicesInfo[connId].processOn = 0x00;
    devicesInfo[connId].subProcessOn = 0x00;
    devicesInfo[connId].authFlag = FALSE;
    devicesInfo[connId].isBleConnect = FALSE;
    devicesInfo[connId].bleMsgBackupLV[0] = 0x00;
    devicesInfo[connId].bleMsgBackupLV[1] = 0x00;
    
    devicesInfo[connId].rangingSession.rangingAction = RANGING_ACTION_NONE;
    devicesInfo[connId].rangingSession.sessionStatus = RANGING_SESSION_IDLE;
    devicesInfo[connId].rangingSession.lastLoc = LOC_NO_LOCINFO;
    devicesInfo[connId].rangingSession.thisLoc = LOC_NO_LOCINFO;
    devicesInfo[connId].rangingSession.rangingResult = RANGING_RESULT_SUCCESS;
    devicesInfo[connId].rangingSession.pointX[0] = 0xFF;
    devicesInfo[connId].rangingSession.pointX[1] = 0xFF;
    devicesInfo[connId].rangingSession.pointY[0] = 0xFF;
    devicesInfo[connId].rangingSession.pointY[1] = 0xFF;

}

void ccc_dk_devices_init(void)
{
    u8 connId;

    for(connId = 0; connId <= MAX_BLE_CONN_NUM; connId++)
    {
        ccc_dk_device_init(connId);
    }
}

void ccc_dk_init_on_ble_connected(u8 connId, ExtEvent_e extEvent)
{
    u16 outLen = 0;

    if(connId == CHANNEL_ID_BLE_FOB)
    {
        //yq_location_connect();
    }
    
    if(connId >= CHANNEL_ID_BLE_0 && connId <= CHANNEL_ID_BLE_FOB)
    {
        ccc_dk_device_init(connId);
        //devicesInfo[connId].isBleConnect = TRUE;
        devicesInfo[connId].timeSyncTriggerConditions = 0x00;  //Procedure 0: After a CONNECT_IND, the device shall send a Time_Sync message
        devicesInfo[connId].rangingSession.locRetryTime = 0;
        
        if(extEvent == EXT_EVENT_BLE_CONNECT_ON_PAIRING)
        {
            devicesInfo[connId].workMode = DEVICE_WORK_MODE_ON_PAIRING;
        }
        else if(extEvent == EXT_EVENT_BLE_CONNECT_ON_FIRST_APPROACH)
        {
            devicesInfo[connId].workMode = DEVICE_WORK_MODE_ON_FIRST_APPROACH;
        }
        else
        {
            devicesInfo[connId].workMode = DEVICE_WORK_MODE_ON_NORMAL_USE;
        }

        //set device macAddr
        cccConfig->getInfo(INFO_TYPE_DEVICE_ADDR, &connId, 1,  devicesInfo[connId].macAddr, &outLen);

        //打开SE上的逻辑通道
        apdu_to_vehicle_se_open_channel(connId);
    }
    else if(connId == CHANNEL_ID_BLE_PRIVATE)
    {
        vehicleInfo.privateBleConnect = TRUE;

#if CCC_DEBUG_FLAG
        //ccc_timer_start_vehicle(connId, TIMER_ID_LOCATION, TIMER_TYPE_REPEAT, 500, (pFunc_timer_handler)ccc_timer_handler_ranging_result_notice_to_ble);
#endif
    }
}

void ccc_dk_init_on_ble_disconnect(u8 connId)
{
    if(connId >= CHANNEL_ID_BLE_0 && connId <= CHANNEL_ID_BLE_FOB)
    {
        if(connId == CHANNEL_ID_BLE_FOB)
        {
            //yq_location_disconnect();
        }
        if(devicesInfo[connId].rangingSession.sessionStatus == RANGING_SESSION_ACTIVE)
        {
#if 0
            //蓝牙断开的时候，还在定位中；直接暂停定位
            ccc_can_send_uwb_ranging_session_suspend_recover_delete_rq(connId, RANGING_ACTION_SUSPEND);  //suspend
            ccc_dk_set_ranging_session_status(connId, devicesInfo[connId].rangingSession.uwbSessionId, URSK_STATUS_SUSPENDED);
#else
            //蓝牙断开的时候，还在定位中；直接废弃定位
            ccc_can_send_uwb_ranging_session_suspend_recover_delete_rq(connId, RANGING_ACTION_DELETE);  //suspend
            ccc_dk_set_ranging_session_status(connId, devicesInfo[connId].rangingSession.uwbSessionId, URSK_STATUS_EMPTY);
#endif
            devicesInfo[connId].rangingSession.sessionStatus = RANGING_SESSION_IDLE;
        }

        ccc_dk_device_init(connId);
    }
    else if(connId == CHANNEL_ID_BLE_PRIVATE)
    {
        vehicleInfo.privateBleConnect = FALSE;
    }
}

void ccc_dk_print_version(void)
{
    u8 sdk_ver[2] = {CCC_SDK_VER_MAJOR, CCC_SDK_VER_MINOR};

    LOG_L_S_HEX(CCC_MD,"COS Version", vehicleInfo.cosVer, 0x02);
    LOG_L_S_HEX(CCC_MD,"CCC Version", vehicleInfo.appletVer, 0x02);
    LOG_L_S_HEX(CCC_MD,"SDK Version", sdk_ver, 0x02);
}

CCCErrCode_e ccc_dk_init_on_reset(void)
{
    CCCErrCode_e errCode = CCC_ERR_CODE_SUCCESS;
    u8 tempEccPubKey[0x41];
    u8 tempEccPriKey[0x20];

    u8 tempRndDevice[0x10];
    u16 off;

    u8 material[10] = {0x73,0x65,0x63,0x75,0x72,0x69,0x74,0x79,0x5F,0x73};

#ifdef CCC_LOCATION_IN_MCU_SDK
    ccc_loc_func_init_on_reset(ccc_loc_func_result_notice_implement);
#endif

    ccc_dk_vehicle_init();
    ccc_dk_devices_init();
    ccc_timer_create_on_reset();

    errCode = apdu_to_vehicle_se_get_cos_version();
    if(errCode != CCC_ERR_CODE_SUCCESS)
    {
        errCode = apdu_to_vehicle_se_get_cos_version();
        if(errCode != CCC_ERR_CODE_SUCCESS)
        {
            return errCode;
        }
    }

    errCode = apdu_to_vehicle_se_select_applet(&vehicleInfo);
    if(errCode != CCC_ERR_CODE_SUCCESS)
    {
        return errCode;
    }

    errCode = apdu_to_vehicle_se_get_app_version();
    if(errCode != CCC_ERR_CODE_SUCCESS)
    {
        return errCode;
    }

    ccc_dk_print_version();

    if(cccConfig->swAlgorithm->eccGenKey(tempEccPubKey, tempEccPriKey) != 0x00)
    {
        errCode = CCC_ERR_CODE_ALGORITHM_ERR;
    }
    else
    {
        errCode = apdu_to_vehicle_se_security_channel_setup_rq(tempEccPubKey, tempRndDevice);

        if(errCode == CCC_ERR_CODE_SUCCESS)
        {
            //material = 'security_s'
            //Sab = ECDH(BLE_ePK, SE_eSK) =  ECDH(SE_ePK, BLE_eSK)
            //Kdh = sha_256(Sab.x + '00000001' + material)
            //AES_KEY =Kdh1-16 ,AES_ICV = Kdh17-32
            if(cccConfig->swAlgorithm->eccECDH(tempEccPubKey, tempEccPriKey, tempEccPubKey) != 0x00)
            {
                errCode = CCC_ERR_CODE_ALGORITHM_ERR;
            }
            else
            {
                tempEccPubKey[0x20] = 0x00;
                tempEccPubKey[0x21] = 0x00;
                tempEccPubKey[0x22] = 0x00;
                tempEccPubKey[0x23] = 0x01;

                core_mm_copy(tempEccPubKey + 0x24, material, 10);

                core_algo_sha256(tempEccPubKey, 0x24 + 10, tempEccPriKey);

                core_mm_copy(vehicleInfo.scKey, tempEccPriKey, 0x10);
                core_mm_copy(vehicleInfo.scIv, tempEccPriKey + 0x10, 0x10);

                LOG_L_S_HEX(CCC_MD,"scKey", vehicleInfo.scKey, 0x10);
                LOG_L_S_HEX(CCC_MD,"scIv", vehicleInfo.scIv, 0x10);

                off = 0;
                core_mm_set(tempEccPriKey, 0x00, 0x20);
                tempEccPriKey[off++] = 0xC1;
                tempEccPriKey[off++] = 0x10;
                core_mm_copy(tempEccPriKey + off, tempRndDevice, 0x10);
                off += 0x10;
                tempEccPriKey[off++] = 0x80;

                core_algo_aes_cbc_cipher(MODE_ENCRYPT, vehicleInfo.scKey, 0x10, 0x10, vehicleInfo.scIv, 0x10, tempEccPriKey, tempEccPubKey, 0x20);

                errCode = apdu_to_vehicle_se_security_channel_setup_rs(tempEccPubKey, 0x20);
                if(errCode != CCC_ERR_CODE_SUCCESS)
                {
                    errCode = CCC_ERR_CODE_SECURITY_CHANNEL_SETUP_FAILED;
                }
            }
        }
    }

    //ccc_timer_start_vehicle(0x00, TIMER_ID_LOCATION, TIMER_TYPE_REPEAT, 100, (pFunc_timer_handler)ccc_timer_handler_ranging_result_notice_to_ble);

    return errCode;
}

//获取某个设备的URSK个数，包括priderived, active, suspend;
/*****************************************************************************
FUNCTION: 获取某个设备的某状态下的URSK个数；
PARAMETER:
        slotIdLV: 当前钥匙ID的slotId；
        status: URSK_STATUS_EMPTY，仅查找空的；
                URSK_STATUS_PREDERIVED, 仅查找pre-derived;
                URSK_STATUS_ACTIVE，仅查找 Actived;
                URSK_STATUS_SUSPENDED, 仅查找 suspend;
                URSK_STATUS_NOT_EMPTY, 查找非空的，包括：URSK_STATUS_PREDRIVED, URSK_STATUS_ACTIVE, URSK_STATUS_SUSPENDED
RETURN: 符合条件的RURSK个数;
******************************************************************************/
u8 ccc_dk_get_ursk_num_by_slotId(u8* slotIdLV, URSKStatus_e status)
{
    u8 num = 0;

    u8 dkUrsksIndex;
    u8 index;

    for(dkUrsksIndex = 0; dkUrsksIndex < MAX_DIGITAL_KEY_STORAGE; dkUrsksIndex ++)
    {
        if(core_mm_compare(vehicleInfo.dkUrsks[dkUrsksIndex].slotIdLV, slotIdLV, (1 + slotIdLV[0])) == 0)
        {
            break;
        }
    }

    if(dkUrsksIndex != MAX_DIGITAL_KEY_STORAGE)
    {
        for(index = 0; index < NUM_URSK_PER_DIGITAL_KEY; index ++)
        {
            if(status == URSK_STATUS_NOT_EMPTY)
            {
                if(vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].urskStatus != URSK_STATUS_EMPTY)
                {
                    num ++;
                }
            }
            else
            {
                if(vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].urskStatus == status)
                {
                    num ++;
                }
            } 
        }
    }

    return num;
}

u8  ccc_dk_clean_up_prederived_ursks(u8* slotIdLV)
{
    u8 dkUrsksIndex, index;

    for(dkUrsksIndex = 0; dkUrsksIndex < MAX_DIGITAL_KEY_STORAGE; dkUrsksIndex ++)
    {
        if(core_mm_compare(vehicleInfo.dkUrsks[dkUrsksIndex].slotIdLV, slotIdLV, (1 + slotIdLV[0])) == 0)
        {
            break;
        }
    }

    if(dkUrsksIndex != MAX_DIGITAL_KEY_STORAGE)
    {
        for(index = 0; index < NUM_URSK_PER_DIGITAL_KEY; index ++)
        {
            if(vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].urskStatus == URSK_STATUS_PREDERIVED)
            {
                vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].urskStatus = URSK_STATUS_EMPTY;
            } 
        }
    }
}

void ccc_dk_add_pre_derived_ursk(u8* slotIdLV, u8* sessionId, u8* ursk)
{
    u8 dkUrsksIndex, ursksIndex;
    u8 rand[1];
    u8 index;

    u8 freeIndex = 0xFF;

    for(dkUrsksIndex = 0; dkUrsksIndex < MAX_DIGITAL_KEY_STORAGE; dkUrsksIndex ++)
    {
        if(core_mm_compare(vehicleInfo.dkUrsks[dkUrsksIndex].slotIdLV, slotIdLV, (1 + slotIdLV[0])) == 0)
        {
            break;
        }

        if(vehicleInfo.dkUrsks[dkUrsksIndex].slotIdLV[0] == 0x00)
        {
            if(freeIndex == 0xFF)
            {
                freeIndex = dkUrsksIndex;
            }
        }
    }

    //已满，随机替换一个；
    if(dkUrsksIndex == MAX_DIGITAL_KEY_STORAGE)
    {
        if(freeIndex == 0xFF)
        {
            ccc_dk_func_generate_random(rand, 1);
            dkUrsksIndex = rand[0] % MAX_DIGITAL_KEY_STORAGE;
        }
        else
        {
            dkUrsksIndex = freeIndex;
        }

        //保存在index 0中；
        core_mm_copy(vehicleInfo.dkUrsks[dkUrsksIndex].ursks[0].sessionId, sessionId, 0x04);
        core_mm_copy(vehicleInfo.dkUrsks[dkUrsksIndex].ursks[0].ursk, ursk, 0x20);
        vehicleInfo.dkUrsks[dkUrsksIndex].ursks[0].usageCountLeft = URSK_TTL_IN_USAGE_COUNT;
        vehicleInfo.dkUrsks[dkUrsksIndex].ursks[0].urskStatus = URSK_STATUS_PREDERIVED;

        //清掉其他index中脏数据
        for(index = 1; index < NUM_URSK_PER_DIGITAL_KEY; index ++)
        {
            core_mm_set(vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].sessionId, 0x00, 0x04);
            core_mm_set(vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].ursk, 0x00, 0x20);
            vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].usageCountLeft = 0;
            vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].urskStatus = URSK_STATUS_EMPTY;
        }

        core_mm_copy(vehicleInfo.dkUrsks[dkUrsksIndex].slotIdLV, slotIdLV, (1 + slotIdLV[0]));
    }
    else  //已保存过该手机的钥匙
    {
        for(index = 0; index < NUM_URSK_PER_DIGITAL_KEY; index ++)
        {
            if(vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].urskStatus == URSK_STATUS_EMPTY)
            {
                break;
            }
        }

        if(index == NUM_URSK_PER_DIGITAL_KEY)  //没有空记录;
        {
            for(index = 0; index < NUM_URSK_PER_DIGITAL_KEY; index ++)
            {
                if(vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].urskStatus == URSK_STATUS_PREDERIVED)
                {
                    break;  //替换第一条pre_drived记录
                }
            }

            if(index == NUM_URSK_PER_DIGITAL_KEY)
            {
                index = 0;  //替换第一条active记录；
            }
        }
        
        core_mm_copy(vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].sessionId, sessionId, 0x04);
        core_mm_copy(vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].ursk, ursk, 0x20);
        vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].usageCountLeft = URSK_TTL_IN_USAGE_COUNT;
        vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].urskStatus = URSK_STATUS_PREDERIVED;

        core_mm_copy(vehicleInfo.dkUrsks[dkUrsksIndex].slotIdLV, slotIdLV, (1 + slotIdLV[0]));
    }
}

/*****************************************************************************
FUNCTION: 按 active -> suspend -> prederived 查找URSK，先找活动的，没找到再找suspend, 再找predrived，再没有，次优流程是 URSK 推导流程和安全测距设置流程的组合
PARAMETER:
        connId: 当前连接的手机编号 
        pOut_sessionId: transaction的低4字节
RETURN: URSKStatus_e:  URSK_STATUS_SUSPENDED: 找到suspend，后续需要走recover；
                       URSK_STATUS_ACTIVE: 找到Active,可以直接用；
                       URSK_STATUS_PREDERIVED: 找到predreived，需要走ranging session create;
                       URSK_STATUS_EMPTY: 都没找到，需要走URSK推导流程；
******************************************************************************/
URSKStatus_e ccc_dk_find_ursk_for_ranging(u8 connId, u8* pOut_sessionId, u8* pOut_ursk)
{
    u8 dkUrsksIndex, ursksIndex;
    u8 index;

    u8 selectUrskIndex = (short)0xFF;

    for(dkUrsksIndex = 0; dkUrsksIndex < MAX_DIGITAL_KEY_STORAGE; dkUrsksIndex ++)
    {
        if(core_mm_compare(vehicleInfo.dkUrsks[dkUrsksIndex].slotIdLV, devicesInfo[connId].slotIdLV, (1 + devicesInfo[connId].slotIdLV[0])) == 0)
        {
            break;
        }
    }

    //已满，未找到，如果车辆没有可用的预派生 URSK，则应执行次优流程。 次优流程是 URSK 推导流程和安全测距设置流程的组合
    if(dkUrsksIndex == MAX_DIGITAL_KEY_STORAGE)
    {
        //prepare_and_send_select_aid_to_device_se_with_ble(connId, 0x01);
        //devicesInfo[connId].processOn = PROCESS_URSK_DERIVATION_FLOW;
        //devicesInfo[connId].subProcessOn = URSK_DERIVATION_STATUS_SELECT;

        return URSK_STATUS_EMPTY;  //走URSK生成流程；
    }
    else
    {
        for(index = 0; index < NUM_URSK_PER_DIGITAL_KEY; index ++)
        {
            if(vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].urskStatus == URSK_STATUS_ACTIVE)
            {
                selectUrskIndex = index;
                break;
            }
        }

        
        if(selectUrskIndex != 0xFF)  //有活动的URSK
        {
            //TODO 
            //如果存在活动测距会话，并且车辆上的 URSK TTL 即将到期或车辆决定使用具有较短 TTL 的 URSK 来启动发动机（参见第 19.5.5 节），
            //则车辆可以设置新的安全测距会话使用预派生的 URSK。当预派生的 URSK 变为活动状态时，车辆和设备应丢弃先前活动的 URSK（如果有）
            
            core_mm_copy(pOut_sessionId, vehicleInfo.dkUrsks[dkUrsksIndex].ursks[selectUrskIndex].sessionId, 0x04);
            core_mm_copy(pOut_ursk, vehicleInfo.dkUrsks[dkUrsksIndex].ursks[selectUrskIndex].ursk, 0x20);

            return URSK_STATUS_ACTIVE;
        }
        else //没有活动的URSK，则找Predrive  
        {
            for(index = 0; index < NUM_URSK_PER_DIGITAL_KEY; index ++)
            {
                if(vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].urskStatus == URSK_STATUS_SUSPENDED)
                {
                    selectUrskIndex = index;
                    break;
                }
            }
            if(selectUrskIndex != 0xFF)
            {
                core_mm_copy(pOut_sessionId, vehicleInfo.dkUrsks[dkUrsksIndex].ursks[selectUrskIndex].sessionId, 0x04);
                core_mm_copy(pOut_ursk, vehicleInfo.dkUrsks[dkUrsksIndex].ursks[selectUrskIndex].ursk, 0x20);
                return URSK_STATUS_SUSPENDED;  //后续走恢复流程
            }
            else
            {
                for(index = 0; index < NUM_URSK_PER_DIGITAL_KEY; index ++)
                {
                    if(vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].urskStatus == URSK_STATUS_PREDERIVED)
                    {
                        selectUrskIndex = index;
                        break;
                    }
                }

                if(selectUrskIndex != 0xFF)
                {
                    core_mm_copy(pOut_sessionId, vehicleInfo.dkUrsks[dkUrsksIndex].ursks[selectUrskIndex].sessionId, 0x04);
                    core_mm_copy(pOut_ursk, vehicleInfo.dkUrsks[dkUrsksIndex].ursks[selectUrskIndex].ursk, 0x20);
                    return URSK_STATUS_PREDERIVED;  //走rangin session craete;
                }
                else
                {
                    //如果车辆没有可用的预派生 URSK，则应执行次优流程。 次优流程是 URSK 推导流程和安全测距设置流程的组合
                    //prepare_and_send_select_aid_to_device_se_with_ble(connId, 0x01);
                    //devicesInfo[connId].processOn = PROCESS_URSK_DERIVATION_FLOW;
                    //devicesInfo[connId].subProcessOn = URSK_DERIVATION_STATUS_SELECT;

                    return URSK_STATUS_EMPTY;  //走URSK生成流程；
                }
            }
        }
    }
}

CCCErrCode_e ccc_dk_find_ursk_to_ranging(u8 connId, boolean rangingSetUpOnPrederived)
{
    //goto set up ranging session;
    URSKStatus_e status;

    if(vehicleInfo.fotaFlag != 0x00)
    {
        return CCC_ERR_CODE_ACTION_NOT_PERMIT;
    }

    status =  ccc_dk_find_ursk_for_ranging(connId, devicesInfo[connId].rangingSession.uwbSessionId, devicesInfo[connId].rangingSession.ursk);
    if(status == URSK_STATUS_PREDERIVED)
    {
        if(rangingSetUpOnPrederived)
        {
            devicesInfo[connId].rangingSession.sessionStatus = RANGING_SESSION_SETUP;
            uwb_ranging_service_ranging_session_request(connId);
        }
        else
        {
            //准备进行capability之前的操作；
            ccc_can_send_uwb_anchor_wakeup_rq(LOC_IND_ALL, connId);
        } 
    }
    else if(status == URSK_STATUS_ACTIVE)
    {
        devicesInfo[connId].rangingSession.sessionStatus = RANGING_SESSION_ACTIVE;
    }
    else if(status == URSK_STATUS_SUSPENDED)
    {
        //goto recover;
        devicesInfo[connId].rangingSession.sessionStatus = RANGING_SESSION_SUSPEND;
        uwb_ranging_service_ranging_session_recover_request(connId);
    }
    else
    {
        //请求device clear pre-derived ursk，重新协商URSK
        devicesInfo[connId].rangingSession.sessionStatus = RANGING_SESSION_IDLE;
        send_ranging_session_status_changed_subevent_in_dk_event_notification(connId, Session_Status_Ranging_session_URSK_refresh);
        ccc_timer_start(connId, TIMER_ID_GENERATE_URSK, TIMER_TYPE_ONCE, 1000, ccc_timer_handler_URSK_Derivation);
    } 

    return CCC_ERR_CODE_SUCCESS;                     
}

void ccc_dk_set_ranging_session_status(u8 connId, u8* pIn_sessionId, URSKStatus_e newState)
{
    u8 dkUrsksIndex, index;

    for(dkUrsksIndex = 0; dkUrsksIndex < MAX_DIGITAL_KEY_STORAGE; dkUrsksIndex ++)
    {
        if(core_mm_compare(vehicleInfo.dkUrsks[dkUrsksIndex].slotIdLV, devicesInfo[connId].slotIdLV, (1 + devicesInfo[connId].slotIdLV[0])) == 0)
        {
            break;
        }
    }

    if(dkUrsksIndex == MAX_DIGITAL_KEY_STORAGE)
    {
        return;
    }
    else
    {
        for(index = 0; index < NUM_URSK_PER_DIGITAL_KEY; index ++)
        {
            if(core_mm_compare(vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].sessionId, pIn_sessionId, 0x04) == 0x00)
            {
                devicesInfo[connId].rangingSession.rangingResultNoticeNum = 0;
                devicesInfo[connId].rangingSession.locFixNum = 0;

                if(newState == URSK_STATUS_EMPTY)
                {
                    core_mm_set(vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].sessionId, 0x00, 0x04);
                    core_mm_set(vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].ursk, 0x00, 0x20);
                    vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].urskStatus = newState;
                    vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].usageCountLeft = 0;
                    
                    devicesInfo[connId].rangingSession.sessionStatus = RANGING_SESSION_IDLE;
                }
                else if(newState == URSK_STATUS_ACTIVE) 
                {
                    devicesInfo[connId].rangingSession.sessionStatus = RANGING_SESSION_ACTIVE;           
                    vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].urskStatus = newState;
                }
                else if(newState == URSK_STATUS_SUSPENDED)
                {
                    devicesInfo[connId].rangingSession.sessionStatus = RANGING_SESSION_SUSPEND;
                    vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].urskStatus = newState;
                }
                else if(newState == URSK_STATUS_PREDERIVED)
                {
                    devicesInfo[connId].rangingSession.sessionStatus = RANGING_SESSION_SETUP;
                    vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].urskStatus = newState;
                }

                break;
            }
        }
    }
}

/**
 * @brief 
 * 
 * @param connId 
 * @return URSKStatus_e : 返回顺序为：URSK_STATUS_ACTIVE > URSK_STATUS_SUSPENDED > URSK_STATUS_PREDERIVED > URSK_STATUS_EMPTY;
 */
URSKStatus_e ccc_dk_get_ursk_status_by_connId(u8 connId)
{
    u8 dkUrsksIndex, index;

    for(dkUrsksIndex = 0; dkUrsksIndex < MAX_DIGITAL_KEY_STORAGE; dkUrsksIndex ++)
    {
        if(core_mm_compare(vehicleInfo.dkUrsks[dkUrsksIndex].slotIdLV, devicesInfo[connId].slotIdLV, (1 + devicesInfo[connId].slotIdLV[0])) == 0)
        {
            break;
        }
    }

    if(dkUrsksIndex == MAX_DIGITAL_KEY_STORAGE)
    {
        return URSK_STATUS_EMPTY;
    }
    else
    {
        for(index = 0; index < NUM_URSK_PER_DIGITAL_KEY; index ++)
        {
             if(vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].urskStatus == URSK_STATUS_ACTIVE)
             {
                return URSK_STATUS_ACTIVE;
             }
        }

        for(index = 0; index < NUM_URSK_PER_DIGITAL_KEY; index ++)
        {
             if(vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].urskStatus == URSK_STATUS_SUSPENDED)
             {
                return URSK_STATUS_SUSPENDED;
             }
        }

        for(index = 0; index < NUM_URSK_PER_DIGITAL_KEY; index ++)
        {
             if(vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].urskStatus == URSK_STATUS_PREDERIVED)
             {
                return URSK_STATUS_PREDERIVED;
             }
        }

        return URSK_STATUS_EMPTY;
    }
}

/**
 * @brief TTL用使用次数来模拟实现
 * 
 * @param connId 
 * @return u8 : 0:success; 1:未找到； 2：TTL exceed;
 */
u8 check_ttl_on_ranging(u8 connId)
{
    u8 dkUrsksIndex, index;

    for(dkUrsksIndex = 0; dkUrsksIndex < MAX_DIGITAL_KEY_STORAGE; dkUrsksIndex ++)
    {
        if(core_mm_compare(vehicleInfo.dkUrsks[dkUrsksIndex].slotIdLV, devicesInfo[connId].slotIdLV, (1 + devicesInfo[connId].slotIdLV[0])) == 0)
        {
            break;
        }
    }

    if(dkUrsksIndex == MAX_DIGITAL_KEY_STORAGE)
    {
        return 1;
    }
    else
    {
        for(index = 0; index < NUM_URSK_PER_DIGITAL_KEY; index ++)
        {
            if(core_mm_compare(vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].sessionId, devicesInfo[connId].rangingSession.uwbSessionId, 0x04) == 0x00)
            {
                if(vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].usageCountLeft == 0)
                {
                    return 2;
                }

                vehicleInfo.dkUrsks[dkUrsksIndex].ursks[index].usageCountLeft --;
                devicesInfo[connId].rangingSession.rangingResultNoticeNum++;
                break;
            }
        }

        if(index == NUM_URSK_PER_DIGITAL_KEY)
        {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief 当前是活动的ranging session数量;
 * 
 * @return u8 
 */
u8 ccc_dk_get_active_ranging_session_num(void)
{
    u8 connId;
    u8 activeRangingNum = 0;
    for(connId = CHANNEL_ID_BLE_0; connId <= MAX_BLE_CONN_NUM; connId++)
    {
        if(devicesInfo[connId].isBleConnect)
        {
            if(devicesInfo[connId].rangingSession.sessionStatus == RANGING_SESSION_ACTIVE)
            {
                activeRangingNum++; 
            }
        }
    }

    return activeRangingNum;
}

CCCErrCode_e ccc_dk_ble_pairing_and_encryption_setup_finsh(u8 connId)
{
    CCCErrCode_e errCode = CCC_ERR_CODE_SUCCESS;

    if(!(connId >= CHANNEL_ID_BLE_0 && connId <= CHANNEL_ID_BLE_FOB))
    {
        return CCC_ERR_CODE_UN_SUPPORT_CHANNEL;
    }

    if(devicesInfo[connId].processOn != PROCESS_IDLE)
    {
        return CCC_ERR_CODE_ILLEGAL_STATE;
    }

    devicesInfo[connId].isBleConnect = TRUE;
    
    if(devicesInfo[connId].workMode == DEVICE_WORK_MODE_ON_PAIRING)
    {
        //next step to: URSK Derivation Flow (See Figure 19-19)
        prepare_and_send_select_aid_to_device_se_with_ble(connId, 0x01);
       
        devicesInfo[connId].processOn = PROCESS_URSK_DERIVATION_FLOW;
        devicesInfo[connId].subProcessOn = URSK_DERIVATION_STATUS_SELECT;
    }
    else if(devicesInfo[connId].workMode == DEVICE_WORK_MODE_ON_FIRST_APPROACH)
    {
        //next step to: Friend First Transaction (See Section 11.8.7) over Bluetooth LE
        prepare_and_send_select_aid_to_device_se_with_ble(connId, 0x01);

        //devicesInfo[connId].processOn = PROCESS_FIRST_TRANSACTION_FRIEDN_FLOW;
        //devicesInfo[connId].subProcessOn = STANDARD_TRANSACTION_STATUS_SELECT;

        devicesInfo[connId].processOn = PROCESS_URSK_DERIVATION_FLOW;
        devicesInfo[connId].subProcessOn = URSK_DERIVATION_STATUS_SELECT;
    }
    else
    {
        prepare_and_send_select_aid_to_device_se_with_ble(connId, 0x01);

        //devicesInfo[connId].processOn = PROCESS_STANDARD_TRANSACTION_FLOW;
        //devicesInfo[connId].subProcessOn = STANDARD_TRANSACTION_STATUS_SELECT;

        devicesInfo[connId].processOn = PROCESS_URSK_DERIVATION_FLOW;
        devicesInfo[connId].subProcessOn = URSK_DERIVATION_STATUS_SELECT;
    }

    return errCode;
}

CCCErrCode_e ccc_dk_rssi_update(u8 connId, s8 rssi)
{
    //devicesInfo[connId].rssi = yq_update_rssi(rssi);
}

u8 ccc_dk_get_index_by_locInd(LocInd_e locInd)
{
    if(locInd == LOC_IND_LEFT_FRONT)
    {
        return 0;
    }
    else if(locInd == LOC_IND_RIGHT_FRONT)
    {
        return 1;
    }
    else if(locInd == LOC_IND_CENTER)
    {
        return 2;
    }
    else if(locInd == LOC_IND_LEFT_BACK)
    {
        return 3;
    }
    else if(locInd == LOC_IND_RIGHT_BACK)
    {
        return 4;
    }
    else
    {
        return 0;
    }
}

/*ID:0x09：调试信息上传通道，用于标定等内部调试用*/
u8 es_api_msg_debug_upload(u8 cmd,u8* data,u16 length)
{

}

/**
 * @brief 检查FOTA中是否有活跃的测距，有的话terminate掉
 * 
 * @return u8 : 0: 当前无测距，1：有测距，停止测距
 */
u8 ccc_dk_stop_ranging_on_fota(void)
{
    u8 connId;

    for(connId = 1; connId < (1 + MAX_BLE_CONN_NUM); connId++)
    {
        if(devicesInfo[connId].rangingSession.sessionStatus == RANGING_SESSION_ACTIVE)
        {
            if(vehicleInfo.chIdOnRanging != 0xFF)
            {
                send_ranging_session_status_changed_subevent_in_dk_event_notification(connId, Session_Status_Ranging_session_terminated);
	            ccc_dk_set_ranging_session_status(connId, devicesInfo[connId].rangingSession.uwbSessionId, URSK_STATUS_EMPTY);
	            ccc_can_send_uwb_ranging_session_suspend_recover_delete_rq(connId, RANGING_ACTION_DELETE);  //terminate
        
                //cccConfig->eventNotice(connId, SDK_EVENT_BLE_DISCONECT, NULL, 0);
                vehicleInfo.fotaFlag = 2U;
                return 1U;
            }
        }
    }

    vehicleInfo.fotaFlag = 1U;
    return 0U;
}

u8 ccc_dk_start_ranging_after_fota(void)
{
    u8 connId;

    vehicleInfo.fotaFlag = 0U;
    for(connId = 1; connId < (1 + MAX_BLE_CONN_NUM); connId++)
    {
        if(devicesInfo[connId].isBleConnect)
        {
            prepare_and_send_select_aid_to_device_se_with_ble(connId, 0x01);
       
            devicesInfo[connId].processOn = PROCESS_URSK_DERIVATION_FLOW;
            devicesInfo[connId].subProcessOn = URSK_DERIVATION_STATUS_SELECT;

            return 1U;
        }
    }

    return 0U;
}


