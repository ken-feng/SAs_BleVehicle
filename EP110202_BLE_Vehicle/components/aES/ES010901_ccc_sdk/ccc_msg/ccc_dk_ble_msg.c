
#include "../ccc_sdk/ccc_dk_type.h"
#include "../ccc_sdk/ccc_dk.h"
#include "../ccc_sdk/ccc_timer.h"
#include "../ccc_sdk/ccc_dk_api.h"
#include "../ccc_util/ccc_dk_util.h"
#include "../ccc_util/ccc_dk_func.h"
#include "../../../aEM/EM00040101_log/EM000401.h"

#include "ccc_dk_ble_msg.h"
#include "ccc_dk_ble_msg_supplementary_service.h"


u16 ccc_dk_msg_get_data_len(CCCDkMsg_t* cccDkMsg_t)
{
    return core_dcm_mku16(cccDkMsg_t->length[0], cccDkMsg_t->length[1]);
}

CCCErrCode_e ccc_dk_msg_process(ChannleID_e connId, u8* p_cccDkMsg, u16 len_cccDKMsg)
{
    CCCErrCode_e errCode = CCC_ERR_CODE_SUCCESS;

    if(len_cccDKMsg != (LEN_MESSAGE_PAYLOAD_HEADER_AND_LENGTH + core_dcm_mku16(p_cccDkMsg[2], p_cccDkMsg[3]))
        || len_cccDKMsg > LEN_MAX_CCC_DKMSG)
    {
        return CCC_ERR_CODE_DATA_ERR;
    }

    if(connId < CHANNEL_ID_BLE_0 || connId > MAX_BLE_CONN_NUM)
    {
        return CCC_ERR_CODE_CONN_NUM_EXPAIRED;
    }

    core_mm_copy((u8*)(&devicesInfo[connId].cccDkMsg_t), p_cccDkMsg, LEN_MESSAGE_PAYLOAD_HEADER_AND_LENGTH);  //copy Message header(1byte) payload header(1byte) length(2byte)
    devicesInfo[connId].cccDkMsg_t.data = (p_cccDkMsg + LEN_MESSAGE_PAYLOAD_HEADER_AND_LENGTH);

    switch(devicesInfo[connId].cccDkMsg_t.MessageHeader_u.messageHeaderB_t.message_type)
    {
        case DK_MESSAGE_TYPE_FRAMEWORK:
            break;
        case DK_MESSAGE_TYPE_SE:
            errCode = msg_dk_se_message_process(connId, &(devicesInfo[connId].cccDkMsg_t));
            break;
        case DK_MESSAGE_TYPE_UWB_RANGING_SERVICE:
            errCode = ccc_dk_ble_msg_uwb_ranging_service_msg_process(connId, &(devicesInfo[connId].cccDkMsg_t));
            break;
        case DK_MESSAGE_TYPE_DK_EVENT_NOTIFICATION:
            errCode = ccc_dk_ble_msg_event_notification_process(connId, &(devicesInfo[connId].cccDkMsg_t));
            break;
        case DK_MESSAGE_TYPE_VEHICLE_OEM_APP:
            break;
        case DK_MESSAGE_TYPE_SUPPLEMENTARY_SERVICE:
            errCode = ccc_dk_ble_msg_supplementary_service_process(connId, &(devicesInfo[connId].cccDkMsg_t));
            break;
        case DK_MESSAGE_TYPE_HEAD_UNIT_PAIRING:
            break;
        default:
            break;
    }

    return errCode;
}

void ccc_dk_msg_prepare_and_send(u8 connId, u8 messageHeader, u8 payloadHeader, u8* data, u16 len)
{
    u16 off = 2;

    if((len + 4) > LEN_MAX_CCC_DKMSG)
    {
        return;
    }

    devicesInfo[connId].bleMsgBackupLV[off++] = messageHeader;
    devicesInfo[connId].bleMsgBackupLV[off++] = payloadHeader;

    core_dcm_writeBig16(devicesInfo[connId].bleMsgBackupLV + off, len);
    off += 2;

    core_mm_copy(devicesInfo[connId].bleMsgBackupLV + off, data, len);
    off += len;

    //LOG_L_S_HEX(CCC_MD,"ble_send_request", devicesInfo[connId].bleMsgBackupLV + 2, (off - 2));

    cccConfig->sendData(connId, devicesInfo[connId].bleMsgBackupLV + 2, (off - 2));
    devicesInfo[connId].bleMsgBackupLV[0] = core_dcm_u16_hi(off - 2);
    devicesInfo[connId].bleMsgBackupLV[1] = core_dcm_u16_lo(off - 2);
}
