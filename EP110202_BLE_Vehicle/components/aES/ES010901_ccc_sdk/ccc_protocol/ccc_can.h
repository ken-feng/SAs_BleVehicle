#ifndef __CCC_CAN_H__
#define __CCC_CAN_H__
#include "../ccc_sdk/ccc_dk_type.h"
#include "../ccc_sdk/ccc_dk.h"

typedef enum
{
    CAN_PKG_ID_UWB_ANCHOR_WAKEUP_RQ = 0,
    CAN_PKG_ID_UWB_ANCHOR_WAKEUP_RS,
    //CAN_PKG_ID_UWB_ANCHORS_INFO_NOTICE_RQ,
    CAN_PKG_ID_TIME_SYNC_RQ,
    CAN_PKG_ID_TIME_SYNC_RS,
    //CAN_PKG_ID_TIME_SYNC_RS,
    //CAN_PKG_ID_TIME_SUMMARY,
    CAN_PKG_ID_UWB_RANGING_SESSION_SETUP_RQ,
    CAN_PKG_ID_UWB_RANGING_SESSION_START_RQ,
    CAN_PKG_ID_UWB_RANGING_SESSION_START_RS,
    //CAN_PKG_ID_UWB_RANGING_SESSION_SETUP_RQ_RESULT_NOTICE,
    CAN_PKG_ID_UWB_RANGING_SESSION_SUSPEND_RECOVER_DELETE_RQ,
    CAN_PKG_ID_UWB_RANGING_SESSION_SUSPEND_RECOVER_DELETE_RS,
    //CAN_PKG_ID_UWB_RANGING_SESSION_SUSPEND_RECOVER_DELETE_RESULT_NOTICE,
    CAN_PKG_ID_UWB_RANGING_RESULT_NOTICE,
    CAN_PKG_ID_UWB_RANGING_LOCATION_NOTICE,
    CAN_PKG_ID_VEHICLE_STATUS_NOTICE,
    CAN_PKG_ID_CALIBRATION_DATA_RQ,
    CAN_PKG_ID_CALIBRATION_DATA_RS,
    CAN_PKG_ID_RKE_EXECUTE_RQ,
    CAN_PKG_ID_RKE_EXECUTE_RS,
    CNA_PKG_ID_SET_ANCHOR_LOCIND_IN_PATCH_RQ,
    CNA_PKG_ID_SET_ANCHOR_LOCIND_IN_PATCH_RS,
}CanPkgId_e;

typedef enum
{
    HOPPING_NEGOTIATION_FAILED = 0x00,  /**协商Hopping失败**/

    NO_HOPPING = 0x80,  /** **/
    CONTINUOUS_HOPPING_WITH_DEFAULT_HOPPING_SEQUENCE = 0x50,
    ADAPTIVE_HOPPING_WITH_DEFAULT_HOPPING_SEQUENCE = 0x30,
    CONTINUOUS_HOPPING_WITH_AES_BASED_OPTIONAL_HOPPING_SEQUENCE = 0x48,
    ADAPTIVE_HOPPING_WITH_AES_BASED_OPTIONAL_HOPPING_SEQUENCE = 0x28,
}HoppingConfigSelected_e;

typedef enum
{
    BIT_NO_HOPPING = 0x80,
    BIT_CONTINUOUS_HOPPING = 0x40,
    BIT_ADAPTIVE_HOPPING = 0x20,
    BIT_DEFAULT_HOPPING_SEQUENCE = 0x10,
    BIT_AES_BASED_HOPPING_SEQUECE = 0x08,
}HoppingConfigBitmask_e;

void ccc_ranging_on_failed(u8* sessionId, u8 needRestart);

void ccc_can_send_uwb_anchor_wakeup_rq(LocInd_e locInd, u8 connId);
void ccc_can_send_uwb_ranging_session_setup_rq(u8 connId);
void ccc_can_send_uwb_ranging_session_start_rq(u8 connId);
void ccc_can_send_uwb_ranging_session_suspend_recover_delete_rq(u8 connId, RangingAction_e actionType);
void ccc_can_send_time_sync_rq(u8* timeSyncPar, u8 parLen);
void ccc_can_send_rke_execute_rq(u8 connId, u16 funcId, u8 actionId, u8 executionType);
CCCErrCode_e ccc_can_pkg_process(u8* pdata, u16 pdataLen);

#endif
