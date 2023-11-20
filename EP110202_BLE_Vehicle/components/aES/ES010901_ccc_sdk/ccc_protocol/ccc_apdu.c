#include "../ccc_protocol/ccc_apdu.h"
#include "../ccc_msg/ccc_dk_ble_msg.h"
#include "../ccc_util/ccc_dk_util.h"
#include "../ccc_sdk/ccc_dk.h"
#include "../../../aEM/EM00020101_algo/EM000201.h"

//---------------按APDU功能封装发给车端SE的接口-------------------------
const u8 APDU_GET_COS_VERSION_TO_VEHICLE_SE[] = {0xFE,0x31,0x00,0x00,0x00};
const u8 APDU_SELECT_AID_TO_VEHICLE_SE[] = {0x00,0xA4,0x04,0x00,0x0D,0xA0,0x00,0x00,0x08,0x09,0x43,0x43,0x43,0x44,0x4B,0x56,0x76,0x31};
const u8 APDU_GET_CCC_APPLET_VERSION_TO_VEHICLE_SE[] = {0x80,0x33,0x99,0x99,0x06,0x55,0x51,0x54,0x65,0x63,0x68};

CCCErrCode_e apdu_to_vehicle_se_get_cos_version(void)
{
    u8 apduLen = 0;
    u8* apduResp = devicesInfo[0].apduResp;
    u16 respLen;
    u8 res = 0;

    res = ccc_dk_func_send_apdu_to_vehicle_se(APDU_GET_COS_VERSION_TO_VEHICLE_SE, sizeof(APDU_GET_COS_VERSION_TO_VEHICLE_SE), apduResp, &respLen);
    if(res != 0 || respLen < 2)
    {
        return CCC_ERR_CODE_SE_COMMUNICATE_ERR;
    }
    else if(apduResp[respLen - 2] == 0x90 && apduResp[respLen - 1] == 0x00)
    {
        if(respLen > 2)
        {
            //状态字9000,默认响应数据格式正确，不再作详细检查了；5922230551480102 0105
            core_mm_copy(vehicleInfo.cosVer, apduResp + 8, 0x02);
        
            return CCC_ERR_CODE_SUCCESS;
        }
        else
        {
            return CCC_ERR_CODE_SE_RESP_DATA_ERR;
        }
    }
    else
    {
        return core_dcm_mku16(apduResp[respLen - 2], apduResp[respLen - 1]);
    }
}

CCCErrCode_e apdu_to_vehicle_se_select_applet(VehicleInfo_t* vehicleInfo)
{
    u8 apduLen = 0;
    u8* apduResp = devicesInfo[0].apduResp;
    u16 respLen;
    u8 res = 0;

    res = ccc_dk_func_send_apdu_to_vehicle_se(APDU_SELECT_AID_TO_VEHICLE_SE, sizeof(APDU_SELECT_AID_TO_VEHICLE_SE), apduResp, &respLen);
    if(res != 0 || respLen < 2)
    {
        return CCC_ERR_CODE_SE_COMMUNICATE_ERR;
    }
    else if(apduResp[respLen - 2] == 0x90 && apduResp[respLen - 1] == 0x00)
    {
        if(respLen > 2)
        {
            //状态字9000,默认响应数据格式正确，不再作详细检查了；
            core_mm_copy(vehicleInfo->spake2VerTlv, apduResp, 0x04);  //SPAKE2+协议版本	0x5A	0x02
            core_mm_copy(vehicleInfo->dkAppVerTlv, apduResp + 4, 0x04);  //数字钥匙版本号	0x5C	0x02
            core_mm_copy(vehicleInfo->vehicleIdTlv, apduResp + 8, 0x0A);   //车端ID	0x4D	0x08
            core_mm_copy(vehicleInfo->seidTlv, apduResp + 18, 0x12);   //SEID  4E 10 
            vehicleInfo->pairStatus = apduResp[38];   //配对状态信息	0x53	0x01
        
            return CCC_ERR_CODE_SUCCESS;
        }
        else
        {
            return CCC_ERR_CODE_SE_RESP_DATA_ERR;
        }
    }
    else
    {
        return core_dcm_mku16(apduResp[respLen - 2], apduResp[respLen - 1]);
    }
}

CCCErrCode_e apdu_to_vehicle_se_get_app_version(void)
{
    u8 apduLen = 0;
    u8* apduResp = devicesInfo[0].apduResp;
    u16 respLen;
    u8 res = 0;

    res = ccc_dk_func_send_apdu_to_vehicle_se(APDU_GET_CCC_APPLET_VERSION_TO_VEHICLE_SE, sizeof(APDU_GET_CCC_APPLET_VERSION_TO_VEHICLE_SE), apduResp, &respLen);
    if(res != 0 || respLen < 2)
    {
        return CCC_ERR_CODE_SE_COMMUNICATE_ERR;
    }
    else if(apduResp[respLen - 2] == 0x90 && apduResp[respLen - 1] == 0x00)
    {
        if(respLen > 2)
        {
            //状态字9000,默认响应数据格式正确，不再作详细检查了；EA010000xxxx
            core_mm_copy(vehicleInfo.appletVer, apduResp + 4, 0x02);
        
            return CCC_ERR_CODE_SUCCESS;
        }
        else
        {
            return CCC_ERR_CODE_SE_RESP_DATA_ERR;
        }
    }
    else
    {
        return core_dcm_mku16(apduResp[respLen - 2], apduResp[respLen - 1]);
    }
}

CCCErrCode_e apdu_to_vehicle_se_security_channel_setup_rq(u8* tempEccPubKey, u8* tempRndDevice)
{
    u8* apdu = devicesInfo[0].apdu;
    u8 apduLen = 0;

    u8* apduResp = devicesInfo[0].apduResp;
    u16 respLen;
    u8 res = 0;

    apdu[apduLen++] = 0x80;
    apdu[apduLen++] = 0x7B;
    apdu[apduLen++] = 0x00; //安全通道建立请求
    apdu[apduLen++] = 0x00; 
    apdu[apduLen++] = 0x43; 

    apdu[apduLen++] = 0xC0;
    apdu[apduLen++] = 0x41;
    core_mm_copy(apdu + apduLen, tempEccPubKey, 0x41);
    apduLen += 0x41;

    res = ccc_dk_func_send_apdu_to_vehicle_se(apdu, apduLen, apduResp, &respLen);
    if(res != 0 || respLen < 2)
    {
        return CCC_ERR_CODE_SE_COMMUNICATE_ERR;
    }
    else if(apduResp[respLen - 2] == 0x90 && apduResp[respLen - 1] == 0x00)
    {
        if(respLen == 0x57)
        {
            //状态字9000,默认响应数据格式正确，不再作详细检查了；
            core_mm_copy(tempRndDevice, apduResp + 2, 0x10);      //C1 10 rnd
            core_mm_copy(tempEccPubKey, apduResp + 0x14, 0x41);   //C2 41 device ecc temp pk

            return CCC_ERR_CODE_SUCCESS;
        }
        else
        {
            return CCC_ERR_CODE_SE_RESP_DATA_ERR;
        }
    }
    else
    {
        return core_dcm_mku16(apduResp[respLen - 2], apduResp[respLen - 1]);
    }
}

CCCErrCode_e apdu_to_vehicle_se_security_channel_setup_rs(u8* apdub, u8 apdubLen)
{
    u8* apdu = devicesInfo[0].apdu;
    u8 apduLen = 0;

    u8* apduResp = devicesInfo[0].apduResp;
    u16 respLen;
    u8 res = 0;

    apdu[apduLen++] = 0x80;
    apdu[apduLen++] = 0x7B;
    apdu[apduLen++] = 0x01; //安全通道建立确认
    apdu[apduLen++] = 0x00; 
    apdu[apduLen++] = apdubLen;

    core_mm_copy(apdu + apduLen, apdub, apdubLen);
    apduLen += apdubLen;


    res = ccc_dk_func_send_apdu_to_vehicle_se(apdu, apduLen, apduResp, &respLen);
    if(res != 0 || respLen < 2)
    {
        return CCC_ERR_CODE_SE_COMMUNICATE_ERR;
    }
    else if(apduResp[respLen - 2] == 0x90 && apduResp[respLen - 1] == 0x00)
    {
        return CCC_ERR_CODE_SUCCESS;
    }
    else
    {
        return core_dcm_mku16(apduResp[respLen - 2], apduResp[respLen - 1]);
    }
}

CCCErrCode_e apdu_to_se_to_check_oob_pairing(u8 connId, u8* devicePkX)
{
    u16 apduLen = 0;
    u8 res = 0;
    u8* apdu = devicesInfo[connId].apdu;
    u8* apduResp = devicesInfo[connId].apduResp; 
    u16 respLen;
    CCCErrCode_e errCode = CCC_ERR_CODE_SUCCESS;

    if(devicesInfo[connId].processOn != PROCESS_BLE_PAIRING_ENCRYPTION_SETUP)
    {
        return CCC_ERR_CODE_ILLEGAL_STATE;
    }

    //check oob pairing;
    apdu[apduLen++] = 0x80;
    apdu[apduLen++] = 0x7D;
    apdu[apduLen++] = 0x02;  //02：SE端进行校验First_Approach_RQ
    apdu[apduLen++] = connId;
    apduLen++;  //skip lc;

    apdu[apduLen++] = 0xC0;
    apdu[apduLen++] = 0x20;
    core_mm_copy(apdu + apduLen, devicePkX, 0x20);
    apduLen += 0x20;

    //对APDUB用安全通道密钥进行加密；
    apduLen = ccc_add_padding_8000(apdu + apduLen, (apduLen - 5), TRUE, 0x10);
    apduLen = core_algo_aes_cbc_cipher(MODE_ENCRYPT, vehicleInfo.scKey, 0x10, 0x10, vehicleInfo.scIv, 0x10, 
        apdu + 5, apdu + 5, apduLen);

    apdu[4] = (u8)apduLen;

    res = ccc_dk_func_send_apdu_to_vehicle_se(apdu, (5 + apduLen), apduResp, &respLen);

    if(res != 0 || respLen < 2)
    {
        return CCC_ERR_CODE_SE_COMMUNICATE_ERR;
    }
    else if(apduResp[respLen - 2] == 0x90 && apduResp[respLen - 1] == 0x00)
    {
        return CCC_ERR_CODE_SUCCESS;
    }
    else
    {
        return core_dcm_mku16(apduResp[respLen - 2], apduResp[respLen - 1]);
    }
}

CCCErrCode_e apdu_to_vehicle_se_open_channel(u8 chId)
{
    u8* apdu = devicesInfo[chId].apdu;
    u8 apduLen = 0;

    u8* apduResp = devicesInfo[chId].apduResp; 
    u16 respLen;
    u8 res = 0;

    apdu[apduLen++] = 0x80;
    apdu[apduLen++] = 0x78;
    apdu[apduLen++] = chId; //通道号[01,02，03]
    apdu[apduLen++] = 0x00; //开启
    apdu[apduLen++] = 0x00; 

    res = ccc_dk_func_send_apdu_to_vehicle_se(apdu, apduLen, apduResp, &respLen);
    if(res != 0 || respLen < 2)
    {
        return CCC_ERR_CODE_SE_COMMUNICATE_ERR;
    }
    else if(apduResp[respLen - 2] == 0x90 && apduResp[respLen - 1] == 0x00)
    {
        return CCC_ERR_CODE_SUCCESS;
    }
    else
    {
        return core_dcm_mku16(apduResp[respLen - 2], apduResp[respLen - 1]);
    }
}

CCCErrCode_e apdu_to_vehicle_se_get_dk_ranging_key(u8 connId, u8* transaction_identifier, u8* ursk)
{
    u8 *apdu = devicesInfo[connId].apdu;
    u16 apduLen = 0;
    u8 *apduResp = devicesInfo[connId].apduResp;
    u16 respLen;

    CCCErrCode_e errCode = CCC_ERR_CODE_SUCCESS;
    u8 res = 0;

    //从车端SE导出URSK
    apdu[apduLen++] = 0x80;
    apdu[apduLen++] = 0x7C;
    apdu[apduLen++] = 0x00;
    apdu[apduLen++] = connId;
    apdu[apduLen++] = 0x00;

    res = ccc_dk_func_send_apdu_to_vehicle_se(apdu, apduLen, apduResp, &respLen);

    if(res != 0 || respLen < 2)
    {
        return CCC_ERR_CODE_SE_COMMUNICATE_ERR;
    }
    else if(apduResp[respLen - 2] == 0x90 && apduResp[respLen - 1] == 0x00)
    {
        core_algo_aes_cbc_cipher(MODE_DECRYPT, vehicleInfo.scKey, 0x10, 0x10, vehicleInfo.scIv, 0x10, apduResp, apduResp, (respLen - 2));

        //TLV(4E, Slotid) + TLV(4C, transaction_identifier) + TLV(C0,URSK)
        if(core_mm_compare(apduResp + 1, devicesInfo[connId].slotIdLV, (1 + devicesInfo[connId].slotIdLV[0])) != 0x00)
        {
            return CCC_ERR_CODE_DATA_ERR;
        }
        else
        {
            respLen = (2 + apduResp[1]);  // point to T4C
            core_mm_copy(transaction_identifier, apduResp + respLen + 2, 0x10);

            respLen += 0x12;  //point to TC0

            core_mm_copy(ursk, apduResp + respLen + 2, 0x20);
            return CCC_ERR_CODE_SUCCESS;
        }
    }
    else
    {
        return core_dcm_mku16(apduResp[respLen - 2], apduResp[respLen - 1]);
    }
}

CCCErrCode_e apdu_to_vehicle_se_ble_rke_verify_on_get_challenge(u8 connId, u8* pInRkeCmd, u8 rkeLen, u8* pOutRkeChallenge)
{
    u8 *apdu = devicesInfo[connId].apdu;
    u16 apduLen = 0;
    u8* apduResp = devicesInfo[connId].apduResp; 
    u16 respLen;

    CCCErrCode_e errCode = CCC_ERR_CODE_SUCCESS;
    u8 res = 0;

    //获取RKE Challenge
    apdu[apduLen++] = 0x80;
    apdu[apduLen++] = 0x7A;
    apdu[apduLen++] = 0x00;
    apdu[apduLen++] = connId;
    apdu[apduLen++] = rkeLen;

    core_mm_copy(apdu + apduLen, pInRkeCmd, rkeLen);
    apduLen += rkeLen;

    res = ccc_dk_func_send_apdu_to_vehicle_se(apdu, apduLen, apduResp, &respLen);

    if(res != 0 || respLen < 2)
    {
        return CCC_ERR_CODE_SE_COMMUNICATE_ERR;
    }
    else if(apduResp[respLen - 2] == 0x90 && apduResp[respLen - 1] == 0x00)
    {
        core_mm_copy(pOutRkeChallenge, apduResp, 0x10);  

        return CCC_ERR_CODE_SUCCESS;
    }
    else
    {
        return core_dcm_mku16(apduResp[respLen - 2], apduResp[respLen - 1]);
    }
}

CCCErrCode_e apdu_to_vehicle_se_ble_rke_verify_on_verify_signature(u8 connId, u8* pInSignature, u8 sigLen, u8* pOutArbitrary)
{
    u8* apdu = devicesInfo[connId].apdu;
    u16 apduLen = 0;
    u8* apduResp = devicesInfo[connId].apduResp;
    u16 respLen;

    CCCErrCode_e errCode = CCC_ERR_CODE_SUCCESS;
    u8 res = 0;

    //获取RKE Challenge
    apdu[apduLen++] = 0x80;
    apdu[apduLen++] = 0x7A;
    apdu[apduLen++] = 0x01;
    apdu[apduLen++] = connId;
    apdu[apduLen++] = sigLen;

    core_mm_copy(apdu + apduLen, pInSignature, sigLen);
    apduLen += sigLen;

    res = ccc_dk_func_send_apdu_to_vehicle_se(apdu, apduLen, apduResp, &respLen);

    if(res != 0 || respLen < 2)
    {
        return CCC_ERR_CODE_SE_COMMUNICATE_ERR;
    }
    else if(apduResp[respLen - 2] == 0x90 && apduResp[respLen - 1] == 0x00)
    {
        core_mm_copy(pOutArbitrary, apduResp, (respLen - 2));  

        return CCC_ERR_CODE_SUCCESS;
    }
    else
    {
        return core_dcm_mku16(apduResp[respLen - 2], apduResp[respLen - 1]);
    }
}

