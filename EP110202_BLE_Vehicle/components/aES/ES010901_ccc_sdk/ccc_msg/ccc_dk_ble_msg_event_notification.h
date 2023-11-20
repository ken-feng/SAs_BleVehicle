#ifndef __CCC_DK_BLE_MSG_EVENT_NOTIFICATION_H__
#define __CCC_DK_BLE_MSG_EVENT_NOTIFICATION_H__
#include "../ccc_sdk/ccc_dk_type.h"
#include "ccc_dk_ble_msg.h"

typedef struct
{
    FunctionID_e funcId;
    u8 actionId;
    ActionType_e actionType;
}FunIdActionId_t;

void rke_reset_on_finish();
void send_command_complete_subevent_in_dk_event_notification(u8 connId, u8 commandStatus);
void send_ranging_session_status_changed_subevent_in_dk_event_notification(u8 connId, u8 sessionStatus);
void send_last_requested_action_execution_status_on_vehicle_status_changed_subevent(u8 connId, u8 exeStatus);
void send_vehicle_function_status_summary_on_vehicle_status_changed_subevent(u8 connId, u8 funcStatus, u8* proData, u8 proDataLen);
void send_last_requested_action_execution_and_function_status_on_vehicle_status_changed_subevent(u8 connId, u8 exeStatus, u8 functionStatus, u8* proprietaryData, u8 proprietaryDataLen);
void send_start_and_request_continuation_confimrmation_for_enduring_RKE(u8 connId, u8* pArbitraryData, u8 arbitraryDataLen);
CCCErrCode_e ccc_dk_ble_msg_event_notification_process(u8 connId, CCCDkMsg_t * cccDkMsg_t);
#endif