#ifndef __CCC_TIMER_H__
#define __CCC_TIMER_H__

#include "../ccc_sdk/ccc_dk_type.h"

/**防止timer冲突，按流程区分timer**/
typedef enum
{
    TIMER_ID_OTHER_BUSINESS = 0x00,
    TIMER_ID_GENERATE_URSK = 0x01,
    TIMER_ID_RANGING_SET_UP = 0x02,
}TIMER_ID_e;

typedef enum
{
    TIMER_ID_PAIRING_TOTAL_TIME = 0x00,  /**配对总耗时**/
    TIMER_ID_KTS = 0x01,  /**KTS**/
    TIMER_ID_RKE = 0x02,  /**RKE**/
    //TIMER_ID_LOCATION_TO_APP = 0x03,  /**私有蓝牙协议上送位置信息到APP**/
    TIMER_ID_LOCATION = 0x03,         /**定位算法定位间隔**/
}TIMER_ID_VEHICLE_e;


typedef enum
{
    TIMER_HANDLER_TYPE_PAIRING_TIMEOUT = 0x00, 
    TIMER_HANDLER_TYPE_URSK_DERIVATION,
    TIMER_HANDLER_TYPE_TIME_SYNC_BY_VEHICLE,
    TIMER_HANDLER_TYPE_KTS_RESP_RECEIVED,
    TIMER_HANDLER_TYPE_DEVICE_SE_BUSY,
    TIMER_HANDLER_TYPE_REQUEST_CONFIRM_CONTINUATION_TIMEOUT,
    TIMER_HANDLER_TYPE_ON_RKE_TIMEOUT,
    TIMER_HANDLER_TYPE_ON_ANCHOR_WAKEUP_FINISH,
    TIMER_HANDLER_TYPE_ON_RANGING_SETUP_FINISH,
    TIMER_HANDLER_TYPE_ON_RANGING_SUSPEND_RECOVER_DELETE_FINISH,
    TIMER_HANDLER_TYPE_ON_TIME_SYNC_ON_PROCEDURE1,
    TIMER_HANDLER_TYPE_RANGING_RESULT_TO_PRIVATE_BLE, 
    TIMER_HANDLER_TYPE_LOCATION, 
}TIMER_HANDLER_TYPE_e;

extern void ccc_timer_create_on_reset(void);
extern int ccc_timer_start(u8 connId, u8 timeIndex, u8 timerType, u32 timeInMilliseconds, pFunc_timer_handler timerHandler);
extern int ccc_timer_start_vehicle(u8 connId, u8 timeIndex, u8 timerType, u32 timeInMilliseconds, pFunc_timer_handler timerHandler);
extern void ccc_timer_handler_pairing_timeOut(void* param);
extern void ccc_timer_handler_URSK_Derivation(void* param);
extern void ccc_timer_handler_timesync_triggered_by_vehicle(void* param);
extern void ccc_timer_handler_kts_response_received_callback(void* param);
extern void ccc_timer_handler_on_device_se_busy(void *param);
extern void ccc_timer_handler_on_rke_request_confirm_continuation_time_out(void *param);
extern void ccc_timer_handler_on_rke_time_out(void *param);
extern void ccc_timer_handler_on_anchor_wakeup_finish(void *param);
extern void ccc_timer_handler_on_anchor_ranging_setup_finish(void *param);
extern void ccc_timer_handler_on_anchor_ranging_suspend_recover_delete_finish(void *param);
extern void ccc_timer_handler_on_time_sync_on_procedure1(void *param);
extern void ccc_timer_handler_ranging_result_notice_to_ble(void *param);
extern void ccc_timer_handler_ranging_location(void *param);

extern CCCErrCode_e ccc_timer_handler_in_task(u8* parIn, u16 parInLen);
#endif