#include "ccc_dk_api.h"
#include "ccc_dk.h"
#include "../ccc_msg/ccc_dk_ble_msg.h"
#include "../ccc_protocol/ccc_can.h"
#include "../../../aEM/EM00040101_log/EM000401.h"

struct ccc_config_t* cccConfig;

CCCErrCode_e ccc_api_module_init(struct ccc_config_t *pConfig)
{
    CCCErrCode_e code = CCC_ERR_CODE_SUCCESS;

    if(pConfig == NULL)
    {
        code = CCC_ERR_CODE_DATA_ERR;
    }
    else
    {
#if 1
        api_power_on_init(pConfig->hsmConfig);
#else
        api_power_on_init();
#endif
        cccConfig = pConfig;
        code = ccc_dk_init_on_reset();
    }

    return code;
}

u8 ccc_api_get_version(u8* pOutVersion)
{
    pOutVersion[0] = CCC_SDK_VER_MAJOR;
    pOutVersion[1] = CCC_SDK_VER_MINOR;

    pOutVersion[2] = vehicleInfo.appletVer[0];
    pOutVersion[3] = vehicleInfo.appletVer[1];

    pOutVersion[4] = vehicleInfo.cosVer[0];
    pOutVersion[5] = vehicleInfo.cosVer[1];

    return 6;
}

CCCErrCode_e ccc_api_extern_event_notice(ChannleID_e channleId, ExtEvent_e extEvent, u8* pInData, u16 pInDataLen)
{
    CCCErrCode_e errCode = CCC_ERR_CODE_SUCCESS;

    switch (extEvent)
    {
    case EXT_EVENT_BLE_CONNECT_ON_PAIRING:
        ccc_dk_init_on_ble_connected(channleId, extEvent);
        break;
    case EXT_EVENT_BLE_CONNECT_ON_FIRST_APPROACH:
        ccc_dk_init_on_ble_connected(channleId, extEvent);
        break;
    case EXT_EVENT_BLE_CONNECT:
        ccc_dk_init_on_ble_connected(channleId, extEvent);
        break;
    case EXT_EVENT_BLE_DISCONNECT:
        ccc_dk_init_on_ble_disconnect(channleId);
        break;
    case EXT_EVENT_CHECK_CONFIRM_RECEIVED_FROM_OOB:
        if(pInDataLen != 0x20 || pInData == NULL)
        {
            errCode = CCC_ERR_CODE_DATA_ERR;
        }
        else
        {
            errCode = apdu_to_se_to_check_oob_pairing(pInData);
            if(errCode != CCC_ERR_CODE_SUCCESS)
            {
                send_command_complete_subevent_in_dk_event_notification(channleId, Command_Status_OOB_mismatch);
                cccConfig->eventNotice(channleId, SDK_EVENT_BLE_SECURE_OOB_MISMATCH, NULL, 0);
            }
        }
        break;
    case EXT_EVENT_BLE_PAIRING_AND_ENCRYPTION_SETUP_FINISH:  //Figure 19-3: Bluetooth LE Pairing and Encryption Setup
        errCode = ccc_dk_ble_pairing_and_encryption_setup_finsh(channleId);
        break;
    case EXT_EVENT_TIMER_HANDLER:  
        errCode = ccc_timer_handler_in_task(pInData, pInDataLen);
        break;
    case EXT_EVENT_RSSI_UPDATE:
        errCode = ccc_dk_rssi_update(channleId, (s8)pInData[0]);
        break;
    default:
        break;
    }

    return errCode;
}

CCCErrCode_e ccc_api_data_request(ChannleID_e chId, u8* pdata, u16 pdataLen)
{
    if(chId >= CHANNEL_ID_BLE_0 && chId <= CHANNEL_ID_BLE_FOB)
    {
        LOG_L_S_HEX(CCC_MD,"ble_data_request", pdata, pdataLen);
        return ccc_dk_msg_process(chId, pdata, pdataLen);
    }
    else if(chId == CHANNEL_ID_NFC)
    {

    }
    else if((chId == CHANNEL_ID_CAN)||(chId == CHANNEL_ID_SPI_UWB))
    {
        return ccc_can_pkg_process(pdata, pdataLen);
    }
    // else if(chId == CHANNEL_ID_SPI_UWB)
    // {

    // }
    else
    {
        return CCC_ERR_CODE_UN_SUPPORT_CHANNEL;
    }
}

