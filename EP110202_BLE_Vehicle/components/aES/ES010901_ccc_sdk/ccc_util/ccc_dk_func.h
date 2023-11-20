#ifndef __CCC_DK_FUNC_H__
#define __CCC_DK_FUNC_H__

#include "../ccc_sdk/ccc_dk_type.h"

boolean ccc_fdk_unc_is_owner_pairing_permit(void);
void ccc_dk_func_generate_random(u8* pRnd, u16 rndLen);
u8 ccc_dk_func_send_apdu_to_vehicle_se(u8* apdu, u16 apduLen, u8* apduResp, u16* respLen);
u8 ccc_dk_func_get_current_datetime_in_formate_UTC(u8* dateTime);
u16 ccc_dk_func_get_sw_from_apdu_resp(u8* apduResp, u16 respLen);
u8 ccc_dk_func_ranging_session_suspend_permit_check(u8 connId, u8* pIn_sessionId);
#endif