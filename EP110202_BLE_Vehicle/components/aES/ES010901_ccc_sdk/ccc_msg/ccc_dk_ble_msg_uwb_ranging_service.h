#ifndef __CCC_DK_BLE_MSG_UWB_RANGING_SERVICE_H_
#define __CCC_DK_BLE_MSG_UWB_RANGING_SERVICE_H_

#include "../ccc_sdk/ccc_dk_type.h"
#include "ccc_dk_ble_msg.h"

CCCErrCode_e ranging_capability_request(u8 connId);
CCCErrCode_e ccc_dk_ble_msg_uwb_ranging_service_msg_process(u8 connId, CCCDkMsg_t * cccDkMsg_t);
CCCErrCode_e uwb_ranging_service_ranging_session_request(u8 connId);
CCCErrCode_e uwb_ranging_service_ranging_session_suspend_request(u8 connId);

#endif