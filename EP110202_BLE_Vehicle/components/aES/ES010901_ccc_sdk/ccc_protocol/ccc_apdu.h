#ifndef __CCC_DK_APDU_TO_VEHICLE_SE_H__
#define __CCC_DK_APDU_TO_VEHICLE_SE_H__
#include "../ccc_sdk/ccc_dk_type.h"
#include "../ccc_sdk/ccc_dk.h"

CCCErrCode_e apdu_to_vehicle_se_get_cos_version(void);
CCCErrCode_e apdu_to_vehicle_se_select_applet(VehicleInfo_t* vehicleInfo);
CCCErrCode_e apdu_to_vehicle_se_get_app_version(void);
CCCErrCode_e apdu_to_vehicle_se_security_channel_setup_rq(u8* tempEccPubKey, u8* tempRndDevice);
CCCErrCode_e apdu_to_vehicle_se_security_channel_setup_rs(u8* apdub, u8 apdubLen);
CCCErrCode_e apdu_to_vehicle_se_open_channel(u8 chId);
CCCErrCode_e apdu_to_se_to_check_oob_pairing(u8 connId, u8* devicePkX);
CCCErrCode_e apdu_to_vehicle_se_get_dk_ranging_key(u8 connId, u8* transaction_identifier, u8* ursk);
CCCErrCode_e apdu_to_vehicle_se_ble_rke_verify_on_get_challenge(u8 connId, u8* pInRkeCmd, u8 rkeLen, u8* pOutRkeChallenge);
CCCErrCode_e apdu_to_vehicle_se_ble_rke_verify_on_verify_signature(u8 connId, u8* pInSignature, u8 sigLen, u8* pOutArbitrary);
#endif
