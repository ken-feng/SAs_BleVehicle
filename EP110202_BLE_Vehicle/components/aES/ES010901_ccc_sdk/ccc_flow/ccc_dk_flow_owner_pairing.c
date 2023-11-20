#include "ccc_dk_flow_owner_pairing.h"
#include "../ccc_msg/ccc_dk_ble_msg.h"
#include "../ccc_msg/ccc_dk_ble_msg_se.h"
#include "../ccc_msg/ccc_dk_ble_msg_uwb_ranging_service.h"
#include "../ccc_sdk/ccc_dk.h"
#include "../ccc_sdk/ccc_dk_api.h"
#include "../ccc_sdk/ccc_timer.h"
#include "../ccc_protocol/ccc_apdu.h"
#include "../ccc_protocol/ccc_can.h"
#include "../ccc_util/ccc_dk_util.h"
#include "../../../aEM/EM00020101_algo/EM000201.h"



const u8 APDU_GAIN_SCRYPT_PARM[5] = {0x80, 0xB8, 0x00, 0x00, 0x00};
u8 apduBackUpLV[261];

CCCErrCode_e select_digital_key_framework_aid_in_phase2(u8 connId)
{
    prepare_and_send_select_aid_to_device_se_with_ble(connId, 0x00);

    devicesInfo[connId].workMode = DEVICE_WORK_MODE_ON_PAIRING;
    devicesInfo[connId].processOn = PROCESS_OWNER_PAIRING_PHASE2;
    devicesInfo[connId].subProcessOn = PAIRING_PHASE2_STATUS_SELECT_AID_STEP1;
    
    return CCC_ERR_CODE_SUCCESS; 
}

CCCErrCode_e parse_select_framework_aid_resp(u8* apdub, u16 apdubLen)
{
    u16 off = 0;
    u8 tag;
    u8 len;
    u8 off1;

    if(!(apdub[apdubLen - 2] == 0x90 && apdub[apdubLen - 1] == 0x00))
    {
        return core_dcm_mku16(apdub[apdubLen - 2], apdub[apdubLen - 1]);
    }

    do{
        tag = apdub[off];
        len = apdub[off + 1];

        if(tag == 0x5A) //n supported SPAKE2+ protocol versions
        {
            if(((len % 2) != 0) || (len == 0))
            {
                return CCC_ERR_CODE_DATA_ERR;
            }

            for(off1 = 0; off1 < len; off1 += 2)
            {
                if( (apdub[off + 2 + off1] == vehicleInfo.spake2VerTlv[2]) 
                    && (apdub[off + 2 + off1 + 1] == vehicleInfo.spake2VerTlv[3]) )
                    {
                        break;
                    }
            }

            if(off1 == len)
            {
                return CCC_ERR_CODE_UN_MATCH_PROTOCOL;
            }
        }
        else if(tag == 0x5C)  //m supported Digital Key applet protocol versions (ver.high | ver.low)
        {
            if(((len % 2) != 0) || (len == 0))
            {
                return CCC_ERR_CODE_DATA_ERR;
            }

            for(off1 = 0; off1 < len; off1 += 2)
            {
                if( (apdub[off + 2 + off1] == vehicleInfo.dkAppVerTlv[2]) 
                    && (apdub[off + 2 + off1 + 1] == vehicleInfo.dkAppVerTlv[3]) )
                    {
                        break;
                    }
            }

            if(off1 == len)
            {
                return CCC_ERR_CODE_UN_MATCH_PROTOCOL;
            }
        }
        else if(tag == 0xD4)  //00h = not in pairing mode 02h = pairing mode started, and pairing password entered
        {
            if(len != 1)
            {
                return CCC_ERR_CODE_DATA_ERR;
            }

            if(apdub[off + 2] != 2)
            {
                return CCC_ERR_CODE_ACTION_NOT_PERMIT;
            }
        }
        else
        {
            return CCC_ERR_CODE_DATA_ERR;
        }

        off += (2 + len);
    }while(off < apdubLen);

    if(off != apdubLen)
    {
        return CCC_ERR_CODE_DATA_ERR;
    }

    return CCC_ERR_CODE_SUCCESS;
}

CCCErrCode_e prepare_spake2_request_in_phase2(u8 connId)
{
    //gain_scrypt_parm;
    u8 *apduResp = devicesInfo[connId].apduResp;
    u8* apdu = devicesInfo[connId].apdu;
    u16 respLen;
    u8 res = 0;
    u8 p3;

    u16 apduLen = 0;

    res = ccc_dk_func_send_apdu_to_vehicle_se((u8*)APDU_GAIN_SCRYPT_PARM, sizeof(APDU_GAIN_SCRYPT_PARM), apduResp, &respLen);

    if(res != 0 || respLen < 2)
    {
        return CCC_ERR_CODE_SE_COMMUNICATE_ERR;
    }
    else if(apduResp[respLen - 2] == 0x90 && apduResp[respLen - 1] == 0x00)
    {
        //7F50 TLV + D6 TLV
        //prepare SPAKE2+ Request command
        apdu[apduLen++] = 0x80;
        apdu[apduLen++] = 0x30;
        apdu[apduLen++] = 0x00;
        apdu[apduLen++] = 0x00;
        p3 = (4 + 4 + respLen - 2);
        apdu[apduLen++] = p3;

        core_mm_copy(apdu + apduLen, vehicleInfo.spake2VerTlv, 4);
        apduLen += 4;  //Agreed SPAKE2+ protocol version

        core_mm_copy(apdu + apduLen, vehicleInfo.dkAppVerTlv, 4);
        apduLen += 4;  //m supported Digital Key applet protocol versions

        core_mm_copy(apdu + apduLen, apduResp, (respLen - 2));
        apduLen += (respLen - 2); //7F50 TLV  + D6 TLV

        apdu[apduLen++] = 0x00;  //LE

        devicesInfo[connId].subProcessOn = PAIRING_PHASE2_STATUS_SPAKE2_REQUEST;
        ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_SE, DK_MESSAGE_ID_APDU_RQ, apdu, apduLen);
        return CCC_ERR_CODE_SUCCESS;
    }
    else 
    {
        return core_dcm_mku16(apduResp[respLen - 2], apduResp[respLen - 1]);
    }
}

CCCErrCode_e parse_spake2_response_and_verify_curveX_in_phase2(u8 connId, u8* apdub, u16 apdubLen)
{
    u16 apduLen = 0;
    u8 res = 0;
    u8* apdu = devicesInfo[connId].apdu;
    u8* apduResp = devicesInfo[connId].apduResp;
    u16 respLen;

    //50 65 PointX 9000
    if(!(apdub[apdubLen - 2] == 0x90 && apdub[apdubLen - 1] == 0x00))
    {
        return core_dcm_mku16(apdub[apdubLen - 2], apdub[apdubLen - 1]);
    }

    if(apdub[0] != 0x50 || apdub[1] != 0x41 || apdubLen != 0x45)
    {
        return CCC_ERR_CODE_DATA_ERR;
    }

    //-----------verify curve x, send to se in vehicle---------------------
    apdu[apduLen++] = 0x80;
    apdu[apduLen++] = 0x40;
    apdu[apduLen++] = 0x00;
    apdu[apduLen++] = connId;
    apdu[apduLen++] = (8 + 0x41 + 0x0D + 4 + 4);
    
    core_mm_copy(apdu + apduLen, apdub, 0x43);
    apduLen += 0x43;

    apdu[apduLen++] = 0x17;
    apdu[apduLen++] = 0x0D;
    ccc_dk_func_get_current_datetime_in_formate_UTC(apdu + apduLen);
    apduLen += 0x0D;

    core_mm_copy(apdu + apduLen, vehicleInfo.spake2VerTlv + 2, 4);
    apduLen += 4;  //Agreed SPAKE2+ protocol version

    core_mm_copy(apdu + apduLen, vehicleInfo.dkAppVerTlv, 4);
    apduLen += 4;  //m supported Digital Key applet protocol versions


    res = ccc_dk_func_send_apdu_to_vehicle_se(apdu, apduLen, apduResp, &respLen);
    if(res != 0 || respLen < 2)
    {
        return CCC_ERR_CODE_SE_COMMUNICATE_ERR;
    }
    else if(apduResp[respLen - 2] == 0x90 && apduResp[respLen - 1] == 0x00)
    {
        //Spaker2+verify[80320000lc][TLV52 + TLV57]
        devicesInfo[connId].subProcessOn = PAIRING_PHASE2_STATUS_SPAKE2_VERIFY;
        ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_SE, DK_MESSAGE_ID_APDU_RQ, apduResp, (respLen - 2));
        return CCC_ERR_CODE_SUCCESS;
    }
    else
    {
        return core_dcm_mku16(apduResp[respLen - 2], apduResp[respLen - 1]);
    }
}

CCCErrCode_e send_write_data_to_se_in_vehicle(u8 connId, u16 tag, u8* apduResp, u16* respLen)
{
    u16 apduLen = 0;
    u8 res = 0;
    u16 sw = 0x9000;
    u8* apdu = devicesInfo[connId].apdu;

    apdu[apduLen++] = 0x80;
    apdu[apduLen++] = 0xC4;
    apdu[apduLen++] = 0x01;
    apdu[apduLen++] = connId;
    apdu[apduLen++] = 0x04;

    apdu[apduLen++] = 0xC1;
    apdu[apduLen++] = 0x02;
    apdu[apduLen++] = core_dcm_u16_hi(tag);
    apdu[apduLen++] = core_dcm_u16_lo(tag);

    res = ccc_dk_func_send_apdu_to_vehicle_se(apdu, apduLen, apduResp, respLen);
    if(res != 0 || *respLen < 2)
    {
        return CCC_ERR_CODE_SE_COMMUNICATE_ERR;
    }
    else
    {
        sw = core_dcm_mku16(apduResp[*respLen - 2], apduResp[*respLen - 1]);
        if(sw == 0x9000)
        {
            return CCC_ERR_CODE_SUCCESS;
        }
        else if(sw == 0x6A88)
        {
            if(tag == 0x7F4C)
            {
                return CCC_ERR_CODE_WRITE_DATA_IS_OPTIONAL;
            }
            else
            {
                return sw;
            }
        }
        else
        {
            return sw;
        }
    }
}

CCCErrCode_e parse_spake2_verify_resp_and_generate_write_data(u8 connId, u8* apdub, u16 apdubLen)
{
    u16 apduLen = 0;
    u8 res = 0;
    u8* apdu = devicesInfo[connId].apdu;
    u8* apduResp = devicesInfo[connId].apduResp;
    u16 respLen;
    CCCErrCode_e errCode = CCC_ERR_CODE_SUCCESS;

    //58 10 Device evidence M[2]
    if(!(apdub[apdubLen - 2] == 0x90 && apdub[apdubLen - 1] == 0x00))
    {
        return core_dcm_mku16(apdub[apdubLen - 2], apdub[apdubLen - 1]);
    }

    if(apdub[0] != 0x58 || apdub[1] != 0x10 || apdubLen != 0x14)
    {
        return CCC_ERR_CODE_DATA_ERR;
    }

    //-----------verify device M2, send to se in vehicle---------------------
    apdu[apduLen++] = 0x80;
    apdu[apduLen++] = 0x20;
    apdu[apduLen++] = 0x00;
    apdu[apduLen++] = connId;

    apdu[apduLen++] = apdubLen - 2;
    core_mm_copy(apdu + apduLen, apdub, apdubLen - 2);
    apduLen += (apdubLen - 2);

    res = ccc_dk_func_send_apdu_to_vehicle_se(apdu, apduLen, apduResp, &respLen);

    if(res != 0 || respLen < 2)
    {
        return CCC_ERR_CODE_SE_COMMUNICATE_ERR;
    }
    else if(apduResp[respLen - 2] == 0x90 && apduResp[respLen - 1] == 0x00)
    {
        devicesInfo[connId].writeOrReadTagOnParing = 0x7F4A;

        errCode = send_write_data_to_se_in_vehicle(connId, devicesInfo[connId].writeOrReadTagOnParing, apduResp, &respLen);
        if(errCode == CCC_ERR_CODE_SUCCESS)
        {
            devicesInfo[connId].subProcessOn = PAIRING_PHASE2_STATUS_WRITE_DATA;
            ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_SE, DK_MESSAGE_ID_APDU_RQ, apduResp, (respLen - 2));

            return CCC_ERR_CODE_SUCCESS;
        }
        else
        {
            return errCode;
        }
    }
    else
    {
        return core_dcm_mku16(apduResp[respLen - 2], apduResp[respLen - 1]);
    }
}

CCCErrCode_e write_IRK_and_vehicle_addr_to_7F49_in_7F4E(u8 chId)
{
    u8 irk[0x10];
    u16 outLen = 0;

    u8* apdu = devicesInfo[chId].apdu;
    u8 apduOff = 0;
    u8 res;
    u8* apduResp = devicesInfo[chId].apduResp; 
    u16 respLen;

    u8 tempBuff[0x30];
    u8 tempLen = 0;

    cccConfig->getInfo(INFO_TYPE_BLE_IRK, irk, 0x00, irk, &outLen);
    cccConfig->getInfo(INFO_TYPE_VEHICLE_ADDR, vehicleInfo.macAddr, 0x00, vehicleInfo.macAddr, &outLen);

    tempBuff[tempLen++] = 0xD0;  //IRK
    tempBuff[tempLen++] = 0x10;
    core_mm_copy(tempBuff + tempLen, irk, 0x10);
    tempLen += 0x10;

    tempBuff[tempLen++] = 0xD1;  //vehicle addr
    tempBuff[tempLen++] = 0x06;
    core_mm_copy(tempBuff + tempLen, vehicleInfo.macAddr, 0x06);
    tempLen += 0x06;

    //对APDUB用安全通道密钥进行加密；
    tempLen = ccc_add_padding_8000(tempBuff + tempLen, tempLen, TRUE, 0x10);
    tempLen = core_algo_aes_cbc_cipher(MODE_ENCRYPT, vehicleInfo.scKey, 0x10, 0x10, vehicleInfo.scIv, 0x10, tempBuff, apdu + 5, tempLen);
    //---------------------
    apdu[apduOff++] = 0x80;
    apdu[apduOff++] = 0x7C;
    apdu[apduOff++] = 0x01;  //导入BLE_Info
    apdu[apduOff++] = chId;  
    apdu[apduOff++] = (u8)tempLen;
    core_mm_copy(apdu + apduOff, tempBuff, tempLen);
    apduOff += tempLen;

    res = ccc_dk_func_send_apdu_to_vehicle_se(devicesInfo[chId].apdu, apduOff, apduResp, &respLen);
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

CCCErrCode_e parse_write_data_resp_and_generate_write_data(u8 connId, u8* apdub, u16 apdubLen)
{
    u16 apduLen = 0;
    u8 res = 0;
    u8* apdu = devicesInfo[connId].apdu;
    u8* apduResp = devicesInfo[connId].apduResp;
    u16 respLen;
    CCCErrCode_e errCode = CCC_ERR_CODE_SUCCESS;

    //58 10 Device evidence M[2]
    if(!(apdub[apdubLen - 2] == 0x90 && apdub[apdubLen - 1] == 0x00))
    {
        return core_dcm_mku16(apdub[apdubLen - 2], apdub[apdubLen - 1]);
    }

    apdu[apduLen++] = 0x80;
    apdu[apduLen++] = 0xC4;
    apdu[apduLen++] = 0x01;
    apdu[apduLen++] = connId;
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
        if(respLen == 2)  //write next data
        {
            if(devicesInfo[connId].writeOrReadTagOnParing == 0x7F4A)
            {
                devicesInfo[connId].writeOrReadTagOnParing = 0x7F4B;
            }
            else if(devicesInfo[connId].writeOrReadTagOnParing == 0x7F4B)
            {
                devicesInfo[connId].writeOrReadTagOnParing = 0x7F4C;  //7F4C is opinion;
            }
            else if(devicesInfo[connId].writeOrReadTagOnParing == 0x7F4C)
            {
                devicesInfo[connId].writeOrReadTagOnParing = 0x7F4D;
            }
            else if(devicesInfo[connId].writeOrReadTagOnParing == 0x7F4D)
            {
                devicesInfo[connId].writeOrReadTagOnParing = 0x7F4E;
                //first should write 7F4E->7F49-> (D0/IRK, D1, vehicle addr);
                errCode = write_IRK_and_vehicle_addr_to_7F49_in_7F4E(connId);
                if(errCode != CCC_ERR_CODE_SUCCESS)
                {
                    return errCode;
                }
            }
            else if(devicesInfo[connId].writeOrReadTagOnParing == 0x7F4E)
            {
                devicesInfo[connId].writeOrReadTagOnParing = 0x5F5F;
            }
            else if(devicesInfo[connId].writeOrReadTagOnParing = 0x5F5F)
            {
                //write data finish;
                prepare_op_control_flow(connId, OP_CONTROL_FLOW_P1_CONTINUE_FLOW, 
                    OP_CONTROL_FLOW_P2_KEY_CREATION_DATA_TRANSIMITTED_TO_DEVICE);  //P1=continue, P2='01
                devicesInfo[connId].subProcessOn = PAIRING_PHASE2_STATUS_OP_CONTROL_FLOW_STEP5;

                return CCC_ERR_CODE_SUCCESS;
            }
            else
            {
                return CCC_ERR_CODE_DATA_ERR;
            }

            errCode = send_write_data_to_se_in_vehicle(connId, devicesInfo[connId].writeOrReadTagOnParing, apduResp, &respLen);
            if(errCode == CCC_ERR_CODE_SUCCESS)
            {
                ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_SE, DK_MESSAGE_ID_APDU_RQ, apduResp, (respLen - 2));

                return CCC_ERR_CODE_SUCCESS;
            }
            else if(errCode == CCC_ERR_CODE_WRITE_DATA_IS_OPTIONAL)
            {
                devicesInfo[connId].writeOrReadTagOnParing = 0x7F4D;  //7F4C是否选的

                errCode = send_write_data_to_se_in_vehicle(connId, devicesInfo[connId].writeOrReadTagOnParing, apduResp, &respLen);
                if(errCode == CCC_ERR_CODE_SUCCESS)
                {
                    ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_SE, DK_MESSAGE_ID_APDU_RQ, apduResp, (respLen - 2));

                    return CCC_ERR_CODE_SUCCESS;
                }
                else
                {
                    return errCode;
                }
            }
            else
            {
                return errCode;
            }
        }
        else
        {
            ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_SE, DK_MESSAGE_ID_APDU_RQ, apduResp, (respLen - 2));
            return CCC_ERR_CODE_SUCCESS;
        }
    }
    else
    {
        return core_dcm_mku16(apduResp[respLen - 2], apduResp[respLen - 1]);
    }
}

CCCErrCode_e send_get_data_to_se_in_vehicle(u8 connId, u16 tag, u8* apduResp, u16* respLen)
{
    u16 apduLen = 0;
    u8 res = 0;
    u8* apdu = devicesInfo[connId].apdu;

    apdu[apduLen++] = 0x80;
    apdu[apduLen++] = 0xC4;
    apdu[apduLen++] = 0x00;
    apdu[apduLen++] = connId;
    apdu[apduLen++] = 0x04;

    apdu[apduLen++] = 0xC0;
    apdu[apduLen++] = 0x02;
    apdu[apduLen++] = core_dcm_u16_hi(tag);
    apdu[apduLen++] = core_dcm_u16_lo(tag);

    res = ccc_dk_func_send_apdu_to_vehicle_se(apdu, apduLen, apduResp, respLen);
    if(res != 0 || *respLen < 2)
    {
        return CCC_ERR_CODE_SE_COMMUNICATE_ERR;
    }
    else if(apduResp[*respLen - 2] == 0x90 && apduResp[*respLen - 1] == 0x00)
    {
        return CCC_ERR_CODE_SUCCESS;
    }
    else
    {
        return core_dcm_mku16(apduResp[*respLen - 2], apduResp[*respLen - 1]);
    }
}

CCCErrCode_e parse_select_resp_and_generate_get_data_apdu(u8 connId, u8* apdub, u16 apdubLen)
{
    u8 *apduResp = devicesInfo[connId].apduResp;
    u16 respLen;
    CCCErrCode_e errCode = parse_select_framework_aid_resp(apdub, apdubLen);

    if(errCode == CCC_ERR_CODE_SUCCESS)
    {
        //get data 0x7F20/7F22/7F24/D3
        devicesInfo[connId].writeOrReadTagOnParing = 0x7F20;

        errCode = send_get_data_to_se_in_vehicle(connId, devicesInfo[connId].writeOrReadTagOnParing, apduResp, &respLen);
        if(errCode == CCC_ERR_CODE_SUCCESS)
        {
            devicesInfo[connId].subProcessOn = PAIRING_PHASE2_STATUS_GET_DATA;
            ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_SE, DK_MESSAGE_ID_APDU_RQ, apduResp, (respLen - 2));

            return CCC_ERR_CODE_SUCCESS;
        }
        else
        {
            return errCode;
        }
    }
    else 
    {
        return errCode;
    }
}

CCCErrCode_e verify_get_data_from_device_and_generate_get_response(u8 connId, u8* apdub, u16 apdubLen)
{
    u16 seRespLenV;
    u16 respLen = 0;
    u16 apduLen = 0;
    u8 res = 0;
    u8* apdu = devicesInfo[connId].apdu;
    u8* apduResp = devicesInfo[connId].apduResp; 
    CCCErrCode_e errCode = CCC_ERR_CODE_SUCCESS;
    u16 sw = ccc_dk_func_get_sw_from_apdu_resp(apdub, apdubLen);

    if(sw != 0x9000)
    {
        return sw;
    }

    apdu[apduLen++] = 0x80;
    apdu[apduLen++] = 0xD6;
    apdu[apduLen++] = 0x00;
    apdu[apduLen++] = connId;
    apdu[apduLen++] = apdubLen;

    core_mm_copy(apdu + apduLen, apdub, apdubLen);
    apduLen += apdubLen;

    res = ccc_dk_func_send_apdu_to_vehicle_se(apdu, apduLen, apduResp, &respLen);
    if(res != 0 || respLen < 2)
    {
        return CCC_ERR_CODE_SE_COMMUNICATE_ERR;
    }
    else
    {
        if(apduResp[respLen - 2] == 0x90 && apduResp[respLen - 1] == 0x00)
        {
            if(respLen == 0x02)
            {
                //verify success by vehicle, and generate next get data cmd;
                if(devicesInfo[connId].writeOrReadTagOnParing == 0x7F20)
                {
                    devicesInfo[connId].writeOrReadTagOnParing = 0x7F22;
                }
                else if(devicesInfo[connId].writeOrReadTagOnParing == 0x7F22)
                {
                    devicesInfo[connId].writeOrReadTagOnParing = 0x7F24;
                }
                else if(devicesInfo[connId].writeOrReadTagOnParing == 0x7F24)
                {
                    devicesInfo[connId].writeOrReadTagOnParing = 0xD3;
                }
                else if(devicesInfo[connId].writeOrReadTagOnParing == 0xD3)
                {
                    //get data all, then WRITE_DATA(attestation) <optional>
                    devicesInfo[connId].writeOrReadTagOnParing = 0x5F5A;

                    errCode = send_write_data_to_se_in_vehicle(connId, devicesInfo[connId].writeOrReadTagOnParing, apduResp, &respLen);
                    if(errCode == CCC_ERR_CODE_SUCCESS)
                    {
                        devicesInfo[connId].subProcessOn = PAIRING_PHASE2_STATUS_WRITE_DATA_ATTESTATION;
                        ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_SE, DK_MESSAGE_ID_APDU_RQ, apduResp, (respLen - 2));

                        return CCC_ERR_CODE_SUCCESS;
                    }
                    else
                    {
                        return errCode;
                    }
                }

                errCode = send_get_data_to_se_in_vehicle(connId, devicesInfo[connId].writeOrReadTagOnParing, apduResp, &respLen);
                if(errCode == CCC_ERR_CODE_SUCCESS)
                {
                    devicesInfo[connId].subProcessOn = PAIRING_PHASE2_STATUS_GET_DATA;
                    ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_SE, DK_MESSAGE_ID_APDU_RQ, apduResp, (respLen - 2));

                    return CCC_ERR_CODE_SUCCESS;
                }
                else
                {
                    return errCode;
                }
            }
            else 
            {
                //generate get response by vehicle, and send to device by ble;
                ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_SE, DK_MESSAGE_ID_APDU_RQ, apduResp, (respLen - 2));

                return CCC_ERR_CODE_SUCCESS;
            }
        }
        else
        {
            return core_dcm_mku16(apduResp[respLen - 2], apduResp[respLen - 1]);
        }
    }
}

CCCErrCode_e parse_write_data_attestation_resp_and_generate_write_data(u8 connId, u8* apdub, u16 apdubLen)
{
    u16 apduLen = 0;
    u8 res = 0;
    u8* apdu = devicesInfo[connId].apdu;
    u8* apduResp = devicesInfo[connId].apduResp;
    u16 respLen;
    CCCErrCode_e errCode = CCC_ERR_CODE_SUCCESS;

    //58 10 Device evidence M[2]
    if(!(apdub[apdubLen - 2] == 0x90 && apdub[apdubLen - 1] == 0x00))
    {
        return core_dcm_mku16(apdub[apdubLen - 2], apdub[apdubLen - 1]);
    }

    apdu[apduLen++] = 0x80;
    apdu[apduLen++] = 0xC4;
    apdu[apduLen++] = 0x01;
    apdu[apduLen++] = connId;
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
        if(respLen == 2)  //write next data
        {
            if(devicesInfo[connId].writeOrReadTagOnParing == 0x5F5A)
            {
                devicesInfo[connId].writeOrReadTagOnParing = 0x5F5F;
            }
            else if(devicesInfo[connId].writeOrReadTagOnParing = 0x5F5F)
            {
                //write data finish;
                prepare_op_control_flow(connId, OP_CONTROL_FLOW_P1_CONTINUE_FLOW, 
                    OP_CONTROL_FLOW_P2_KEY_CERTIFICATE_CHAIN_RECEIVED_BY_VEHICLE);  //P1=continue, P2='02' (key certificate chain received by vehicle)
                devicesInfo[connId].subProcessOn = PAIRING_PHASE2_STATUS_OP_CONTROL_FLOW_STEP15;

                return CCC_ERR_CODE_SUCCESS;
            }
            else
            {
                return CCC_ERR_CODE_DATA_ERR;
            }

            errCode = send_write_data_to_se_in_vehicle(connId, devicesInfo[connId].writeOrReadTagOnParing, apduResp, &respLen);
            if(errCode == CCC_ERR_CODE_SUCCESS)
            {
                //devicesInfo[connId].subProcessOn = PAIRING_PHASE2_STATUS_WRITE_DATA;
                ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_SE, DK_MESSAGE_ID_APDU_RQ, apduResp, (respLen - 2));

                return CCC_ERR_CODE_SUCCESS;
            }
            else
            {
                return errCode;
            }
        }
        else  //write continue
        {
            ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_SE, DK_MESSAGE_ID_APDU_RQ, apduResp, (respLen - 2));
            return CCC_ERR_CODE_SUCCESS;
        }
    }
    else
    {
        return core_dcm_mku16(apduResp[respLen - 2], apduResp[respLen - 1]);
    }
}


CCCErrCode_e process_apdu_resp_in_onwer_pairing_phase2(u8 connId, u8* apdub, u16 apdubLen)
{
    CCCErrCode_e errCode = CCC_ERR_CODE_SUCCESS;
    u16 sw;

    switch(devicesInfo[connId].subProcessOn)
    {
        case PAIRING_PHASE2_STATUS_SELECT_AID_STEP1:
            errCode = parse_select_framework_aid_resp(apdub, apdubLen);

            if(errCode == CCC_ERR_CODE_SUCCESS)
            {
                return prepare_spake2_request_in_phase2(connId);
            }
            else 
            {
                return errCode;
            }
            break;
        case PAIRING_PHASE2_STATUS_SPAKE2_REQUEST:
            errCode =  parse_spake2_response_and_verify_curveX_in_phase2(connId, apdub, apdubLen);
            break;
        case PAIRING_PHASE2_STATUS_SPAKE2_VERIFY:
            errCode = parse_spake2_verify_resp_and_generate_write_data(connId, apdub, apdubLen);
            break;
        case PAIRING_PHASE2_STATUS_WRITE_DATA:
            errCode = parse_write_data_resp_and_generate_write_data(connId, apdub, apdubLen);
            break;
        case PAIRING_PHASE2_STATUS_OP_CONTROL_FLOW_STEP5:
            sw = ccc_dk_func_get_sw_from_apdu_resp(apdub, apdubLen);

            if(sw == 0x9000)
            {
                prepare_and_send_select_aid_to_device_se_with_ble(connId, 0x00);

                devicesInfo[connId].subProcessOn = PAIRING_PHASE2_STATUS_SELECT_AID_STEP7;
    
                errCode = CCC_ERR_CODE_SUCCESS; 
            }
            else
            {
                errCode = sw;
            }
            break;
        case PAIRING_PHASE2_STATUS_SELECT_AID_STEP7:
            errCode = parse_select_resp_and_generate_get_data_apdu(connId, apdub, apdubLen);
            break;
        case PAIRING_PHASE2_STATUS_GET_DATA:
            errCode = verify_get_data_from_device_and_generate_get_response(connId, apdub, apdubLen);
            break;
        case PAIRING_PHASE2_STATUS_WRITE_DATA_ATTESTATION:
            errCode = parse_write_data_attestation_resp_and_generate_write_data(connId, apdub, apdubLen);
            break;
        case PAIRING_PHASE2_STATUS_OP_CONTROL_FLOW_STEP15:
            sw = ccc_dk_func_get_sw_from_apdu_resp(apdub, apdubLen);

            if(sw == 0x9000)
            {
                prepare_op_control_flow(connId, OP_CONTROL_FLOW_P1_END_FLOW, 
                    OP_CONTROL_FLOW_P2_SUCCESSFUL_END_OF_KEY_CREATION_AND_VERIFICATION);  //P1=end, P2='11' (successful end of key creation and verification, key not tracked by vehicle)

                devicesInfo[connId].subProcessOn = PAIRING_PHASE2_STATUS_OP_CONTROL_FLOW_STEP17;

                errCode = CCC_ERR_CODE_SUCCESS;
            }
            else
            {
                errCode = sw;
            }

            break;
        case PAIRING_PHASE2_STATUS_OP_CONTROL_FLOW_STEP17:
            sw = ccc_dk_func_get_sw_from_apdu_resp(apdub, apdubLen);

            if(sw == 0x9000)
            {
                //goto state:Command Complete SubEvent(Bluetooth LE_pairing_ready)

                send_command_complete_subevent_in_dk_event_notification(connId, Command_Status_BLE_pairing_ready);
                
                devicesInfo[connId].processOn = PROCESS_BLE_PAIRING_ENCRYPTION_SETUP;

                errCode = CCC_ERR_CODE_SUCCESS;
            }
            else
            {
                errCode = sw;
            }
            break;
    }

    if(errCode != CCC_ERR_CODE_SUCCESS)
    {
        devicesInfo[connId].processOn = PROCESS_IDLE;
        prepare_op_control_flow(connId, OP_CONTROL_FLOW_P1_ABORT_FLOW, 
            OP_CONTROL_FLOW_P2_NO_SPECIFIC_REASON);   //abort flow,  no specific reason
    }

    return errCode;
}

//---------------------------------------------------------------------------------------
CCCErrCode_e parse_select_applet_aid_in_device(u8 connId, u8* apdub, u16 apdubLen)
{
    u16 sw = 0x9000;
    u8 off1 = 0;
    //5C xx data(the apdub is A list of supported Digital Key applet protocol versions ordered from highest to lowest)
    if(apdubLen < 6 || ((apdubLen %2) != 0))
    {
        return CCC_ERR_CODE_DATA_ERR;
    }

    sw = ccc_dk_func_get_sw_from_apdu_resp(apdub, apdubLen);
    if(sw != 0x9000)
    {
        return sw;
    }

    //5C 04 AABB CCDD 9000
    for(off1 = 2; off1 < (apdubLen - 2); off1 += 2)
    {
        if( (apdub[off1] == vehicleInfo.dkAppVerTlv[2]) 
            && (apdub[off1 + 1] == vehicleInfo.dkAppVerTlv[3]) )
        {
            break;
        }
    }

    if(off1 == (apdubLen - 2))
    {
        return CCC_ERR_CODE_UN_MATCH_PROTOCOL;
    }

    return CCC_ERR_CODE_SUCCESS;
}

CCCErrCode_e parse_create_range_key_in_device(u8 connId, u8* apdub, u16 apdubLen)
{
    u16 sw = 0x9000;
    u8 transId[0x10];
    u8 ursk[0x20];
    CCCErrCode_e errCode = CCC_ERR_CODE_SUCCESS;

    if(apdubLen != 2)
    {
        return CCC_ERR_CODE_DATA_ERR;
    }

    sw = ccc_dk_func_get_sw_from_apdu_resp(apdub, apdubLen);
    if(sw == 0x6484)
    {
        //如果给定数字钥匙的设备 URSK 插槽已满，它将无法存储任何新钥匙，从而导致设备响 应创建测距密钥命令并使用状态字 6484 表示“URSK 插槽已用完”。 
        if(ccc_dk_get_ursk_num_by_slotId(devicesInfo[connId].slotIdLV, URSK_STATUS_PREDERIVED) > 0)
        {
            //Check if URSK slots filled If yes, prederived URSK shall be used If not, continue
            return CCC_ERR_CODE_SUCCESS;
        }
        else
        {
            //收到此状态字后， 车辆不应请求新的 URSK 派生，除非它首先请求“URSK_refresh”。
            // Clean up allpre-derived URSKs which belong to this digital key
            ccc_dk_clean_up_prederived_ursks(devicesInfo[connId].slotIdLV);
            send_ranging_session_status_changed_subevent_in_dk_event_notification(connId, Session_Status_Ranging_session_URSK_refresh);
            ccc_timer_start(connId, TIMER_ID_GENERATE_URSK, TIMER_TYPE_ONCE, 1000, ccc_timer_handler_URSK_Derivation);
            return CCC_ERR_CODE_ON_PROCESSING;
        }
    }
    else if(sw != 0x9000)
    {
        return sw;
    }

    errCode = apdu_to_vehicle_se_get_dk_ranging_key(connId, transId, ursk);
    if(errCode == CCC_ERR_CODE_SUCCESS)
    {
        ccc_dk_add_pre_derived_ursk(devicesInfo[connId].slotIdLV, transId + 0x0C, ursk);
    }

    return errCode;
}


CCCErrCode_e process_apdu_resp_in_URSK_Derivation(u8 connId, u8* apdub, u16 apdubLen)
{
    CCCErrCode_e errCode = CCC_ERR_CODE_SUCCESS;
    u16 sw;

    switch(devicesInfo[connId].subProcessOn)
    {
        case URSK_DERIVATION_STATUS_SELECT:
            errCode = parse_select_applet_aid_in_device(connId, apdub, apdubLen);
            if(errCode == CCC_ERR_CODE_SUCCESS)
            {
                //-------------------prepare and send auth0------------------
                //p1 = 0x00: Standard(0) transaction request, may be no exchange cmd,
                //p2 = 0x10: Derive ranging key (standard transaction only)
                errCode = prepare_and_send_auth0_to_device_se_with_ble(connId, 00, TRANSCTION_TYPE_DERIVE_RANGING_KEY);
                if(errCode == CCC_ERR_CODE_SUCCESS)
                {
                    devicesInfo[connId].subProcessOn = URSK_DERIVATION_STATUS_AUTH_0;
                }
            }
            break;
        case URSK_DERIVATION_STATUS_AUTH_0:
            errCode = parse_auth0_resp_and_send_auth1(connId, apdub, apdubLen);
            if(errCode == CCC_ERR_CODE_SUCCESS)
            {
                devicesInfo[connId].subProcessOn = URSK_DERIVATION_STATUS_AUTH_1;
            }
            break;
        case URSK_DERIVATION_STATUS_AUTH_1:
            errCode = parse_auth1_resp_with_vehicle_se(connId, apdub, apdubLen);
            if(errCode == CCC_ERR_CODE_SUCCESS)
            {
                //The arbitrary_data field (up to 127 bytes) is optional and reserved for future usage.
                prepare_and_send_create_ranging_key_to_device_with_ble(connId, apdub, 0);
                devicesInfo[connId].subProcessOn = URSK_DERIVATION_STATUS_CREATE_RANGE_KEY;
            }
            break;
        case URSK_DERIVATION_STATUS_CREATE_RANGE_KEY:
            //9. Command Complete SubEvent (Deselect SE)
            send_command_complete_subevent_in_dk_event_notification(connId, Command_Status_Deselect_SE);

            errCode = parse_create_range_key_in_device(connId, apdub, apdubLen);
            if(errCode == CCC_ERR_CODE_SUCCESS)
            {
                ccc_can_send_uwb_anchor_wakeup_rq(LOC_IND_ALL, connId);
            }
            else if(errCode == 0x6484)
            {
                //此情况，先发ursk_refresh，再定时走URSK生成
                errCode = CCC_ERR_CODE_SUCCESS;
            }
            break;
    }

    return errCode;
}

//-----------------------------------------------------------------------------------------
CCCErrCode_e process_apdu_resp_in_phase3_in_onwer_pairing(u8 connId, u8* apdub, u16 apdubLen)
{
    CCCErrCode_e errCode = CCC_ERR_CODE_SUCCESS;
    u16 sw;
    u16 apduLen;

    switch(devicesInfo[connId].subProcessOn)
    {
        case PAIRING_PHASE3_STATUS_SELECT:
            errCode = parse_select_applet_aid_in_device(connId, apdub, apdubLen);
            if(errCode == CCC_ERR_CODE_SUCCESS)
            {
                //-------------------prepare and send auth0------------------
                //p1=0x04:Standard(0) transaction request, exchange cmd will send,
                //p2=0x07:First standard transaction at owner pairing (owner immobilizer token provisioning)
                errCode = prepare_and_send_auth0_to_device_se_with_ble(connId, 0x04, 
                    TRANSCTION_TYPE_FIRST_STANDARD_TRANSACTION_AT_OWNER_PAIRING);
                if(errCode == CCC_ERR_CODE_SUCCESS)
                {
                    devicesInfo[connId].subProcessOn = PAIRING_PHASE3_STATUS_AUTH_0;
                }
            }
            break;
        case PAIRING_PHASE3_STATUS_AUTH_0:
            errCode = parse_auth0_resp_and_send_auth1(connId, apdub, apdubLen);
            if(errCode == CCC_ERR_CODE_SUCCESS)
            {
                devicesInfo[connId].subProcessOn = PAIRING_PHASE3_STATUS_AUTH_1;
            }
            break;
        case PAIRING_PHASE3_STATUS_AUTH_1:
            errCode = parse_auth1_resp_with_vehicle_se(connId, apdub, apdubLen);
            if(errCode == CCC_ERR_CODE_SUCCESS)
            {
                errCode = prepare_and_send_mailbox_operation_inquire(connId, apdub, &apduLen);
                if(errCode == CCC_ERR_CODE_SUCCESS)
                {
                    ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_SE, DK_MESSAGE_ID_APDU_RQ, apdub, apduLen);

                    devicesInfo[connId].subProcessOn = PAIRING_PHASE3_STATUS_EXCHANGE;
                }
            }
            break;
        case PAIRING_PHASE3_STATUS_EXCHANGE:
            errCode = parse_mailbox_resp_and_generate_mailbox_operation(connId, apdub, apdubLen, apdub, &apduLen, &sw);

            if(errCode == CCC_ERR_CODE_SUCCESS)
            {
                if(apduLen > 0)
                {
                    ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_SE, DK_MESSAGE_ID_APDU_RQ, apdub, apduLen);
                }
                else  //no more exchange cmd;  CONTROL_FLOW_Command(P1='01', P2='81')
                {
                    prepare_control_flow(connId, CONTROL_FLOW_P1P2_SUCCESSFUL_END_PAIRING_PHASE3);  //0x0181, Successful end, owner immobilizer token provisioned
                    devicesInfo[connId].subProcessOn = PAIRING_PHASE3_STATUS_OP_CONTROL_FLOW_STEP13;
                }
            }
            
            break;
        case PAIRING_PHASE3_STATUS_OP_CONTROL_FLOW_STEP13:
            send_command_complete_subevent_in_dk_event_notification(connId, Command_Status_Deselect_SE);

            //kts response received check;
            ccc_timer_start_vehicle(connId, TIMER_ID_KTS, TIMER_TYPE_ONCE, TIMEOUT_MS_ON_KTS, (pFunc_timer_handler)ccc_timer_handler_kts_response_received_callback);
            break;
    }

    return errCode;
}

CCCErrCode_e process_apdu_resp_in_phase4_in_onwer_pairing(u8 connId, u8* apdub, u16 apdubLen)
{
    CCCErrCode_e errCode = CCC_ERR_CODE_SUCCESS;
    u16 sw;
    u16 apduLen;

    switch(devicesInfo[connId].subProcessOn)
    {
        case PAIRING_PHASE4_STATUS_SELECT:
            errCode = parse_select_applet_aid_in_device(connId, apdub, apdubLen);
            if(errCode == CCC_ERR_CODE_SUCCESS)
            {
                //-------------------prepare and send auth0------------------
                //p1=0x04:Standard(0) transaction request, exchange cmd will send,
                //p1=0x08:Second standard transaction at owner pairing (read KTS receipt, provision friend immobilizer tokens)
                errCode = prepare_and_send_auth0_to_device_se_with_ble(connId, 0x04, 
                    TRANSCTION_TYPE_SECOND_STANDARD_TRANSACTION_AT_OWNER_PAIRING);
                if(errCode == CCC_ERR_CODE_SUCCESS)
                {
                    devicesInfo[connId].subProcessOn = PAIRING_PHASE4_STATUS_AUTH_0;
                }
            }
            break;
        case PAIRING_PHASE4_STATUS_AUTH_0:
            errCode = parse_auth0_resp_and_send_auth1(connId, apdub, apdubLen);
            if(errCode == CCC_ERR_CODE_SUCCESS)
            {
                devicesInfo[connId].subProcessOn = PAIRING_PHASE4_STATUS_AUTH_1;
            }
            break;
        case PAIRING_PHASE4_STATUS_AUTH_1:
            errCode = parse_auth1_resp_with_vehicle_se(connId, apdub, apdubLen);
            if(errCode == CCC_ERR_CODE_SUCCESS)
            {
                //p1=40, p2=88, Continue, key tracking response received in device, next step is to read the receipt from the mailbox
                prepare_control_flow(connId, CONTROL_FLOW_P1P2_KEY_TRACKING_RESP_RECEIVED_IN_DEVICE);  
                devicesInfo[connId].subProcessOn = PAIRING_PHASE4_STATUS_OP_CONTROL_FLOW_STEP2;
            }
            break;
        case PAIRING_PHASE4_STATUS_OP_CONTROL_FLOW_STEP2:
            errCode = prepare_and_send_mailbox_operation_inquire(connId, apdub, &apduLen);
            if(errCode == CCC_ERR_CODE_SUCCESS)
            {
                ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_SE, DK_MESSAGE_ID_APDU_RQ, apdub, apduLen);

                devicesInfo[connId].subProcessOn = PAIRING_PHASE4_STATUS_EXCHANGE_STEP4;
            }
            break;
        case PAIRING_PHASE4_STATUS_EXCHANGE_STEP4:
            errCode = parse_mailbox_resp_and_generate_mailbox_operation(connId, apdub, apdubLen, apduBackUpLV + 2, &apduLen, &sw);

            if(errCode == CCC_ERR_CODE_SUCCESS)
            {
                if(apduLen > 0)
                {
                    apduBackUpLV[0] = core_dcm_u16_hi(apduLen);
                    apduBackUpLV[1] = core_dcm_u16_lo(apduLen);
                    
                    devicesInfo[connId].subProcessOn = PAIRING_PHASE4_STATUS_OP_CONTROL_FLOW_STEP6;
                }
                else  //no more exchange cmd;  CONTROL_FLOW_Command(P1='01', P2='81')
                {
                    devicesInfo[connId].subProcessOn = PAIRING_PHASE4_STATUS_OP_CONTROL_FLOW_STEP14;
                }

                prepare_op_control_flow(connId, ( (core_dcm_u16_hi(sw) == 0x94) ? 0x40 : 0x01 ), core_dcm_u16_lo(sw));
            }
            
            break;
        case PAIRING_PHASE4_STATUS_OP_CONTROL_FLOW_STEP6:
            ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_SE, DK_MESSAGE_ID_APDU_RQ, apduBackUpLV + 2, core_dcm_mku16(apduBackUpLV[0], apduBackUpLV[1]));
            devicesInfo[connId].subProcessOn = PAIRING_PHASE4_STATUS_EXCHANGE_STEP8;
            break;
        case PAIRING_PHASE4_STATUS_EXCHANGE_STEP8:
            errCode = parse_mailbox_resp_and_generate_mailbox_operation(connId, apdub, apdubLen, apduBackUpLV + 2, &apduLen, &sw);

            if(errCode == CCC_ERR_CODE_SUCCESS)
            {
                if(apduLen > 0)
                {
                    apduBackUpLV[0] = core_dcm_u16_hi(apduLen);
                    apduBackUpLV[1] = core_dcm_u16_lo(apduLen);
                    
                    devicesInfo[connId].subProcessOn = PAIRING_PHASE4_STATUS_OP_CONTROL_FLOW_STEP10;
                }
                else  //no more exchange cmd;  CONTROL_FLOW_Command(P1='01', P2='81')
                {
                    devicesInfo[connId].subProcessOn = PAIRING_PHASE4_STATUS_OP_CONTROL_FLOW_STEP14;
                }

                prepare_op_control_flow(connId, ( (core_dcm_u16_hi(sw) == 0x94) ? 0x40 : 0x01 ), core_dcm_u16_lo(sw));
            }
            break;
        case PAIRING_PHASE4_STATUS_OP_CONTROL_FLOW_STEP10:
            ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_SE, DK_MESSAGE_ID_APDU_RQ, apduBackUpLV + 2, core_dcm_mku16(apduBackUpLV[0], apduBackUpLV[1]));
            devicesInfo[connId].subProcessOn = PAIRING_PHASE4_STATUS_EXCHANGE_STEP12;
            break;
        case PAIRING_PHASE4_STATUS_EXCHANGE_STEP12:
            errCode = parse_mailbox_resp_and_generate_mailbox_operation(connId, apdub, apdubLen, apduBackUpLV + 2, &apduLen, &sw);

            if(errCode == CCC_ERR_CODE_SUCCESS)
            {
                prepare_op_control_flow(connId, ( (core_dcm_u16_hi(sw) == 0x94) ? 0x40 : 0x01 ), core_dcm_u16_lo(sw));

                devicesInfo[connId].subProcessOn = PAIRING_PHASE4_STATUS_OP_CONTROL_FLOW_STEP14;
            }
            break;
        case PAIRING_PHASE4_STATUS_OP_CONTROL_FLOW_STEP14:
            vehicleInfo.pairStatus = VEHICLE_STATUS_PAIRED_WITH_OWNER;
            cccConfig->timerTool->stopTimer(vehicleInfo.timers[TIMER_ID_PAIRING_TOTAL_TIME].timerId);  //完成配对
            send_command_complete_subevent_in_dk_event_notification(connId, Command_Status_Deselect_SE);
            devicesInfo[connId].processOn = PROCESS_IDLE;
            
            break;
    }

    return errCode;
}




