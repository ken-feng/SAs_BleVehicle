#ifndef __CCC_DK_FLOW_OWNER_PAIRING_H__
#define __CCC_DK_FLOW_OWNER_PAIRING_H__
#include "../ccc_sdk/ccc_dk_type.h"
#include "../ccc_util/ccc_dk_util.h"
#include "../ccc_util/ccc_dk_func.h"

extern CCCErrCode_e select_digital_key_framework_aid_in_phase2(u8 connId);
extern CCCErrCode_e process_apdu_resp_in_onwer_pairing_phase2(u8 connId, u8* apdub, u16 apdubLen);
CCCErrCode_e process_apdu_resp_in_URSK_Derivation(u8 connId, u8* apdub, u16 apdubLen);
CCCErrCode_e process_apdu_resp_in_phase3_in_onwer_pairing(u8 connId, u8* apdub, u16 apdubLen);
CCCErrCode_e process_apdu_resp_in_phase4_in_onwer_pairing(u8 connId, u8* apdub, u16 apdubLen);
#endif