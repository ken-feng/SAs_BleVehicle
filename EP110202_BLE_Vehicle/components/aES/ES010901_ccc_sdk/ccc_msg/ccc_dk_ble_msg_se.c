#include "../ccc_sdk/ccc_dk.h"
#include "../ccc_sdk/ccc_dk_type.h"
#include "../ccc_msg/ccc_dk_ble_msg.h"
#include "../ccc_msg/ccc_dk_ble_msg_se.h"
#include "../ccc_flow/ccc_dk_flow_owner_pairing.h"
#include "../ccc_sdk/ccc_timer.h"
#include "../../../aEM/EM00040101_log/EM000401.h"

const u8 AID_DIGITAL_KEY_FRAMEWORK[] = {0xA0,0x00,0x00,0x08,0x09,0x43,0x43,0x43,0x44,0x4B,0x46,0x76,0x31};
const u8 AID_DIGITAL_KEY_APPLET[13] = {0xA0,0x00,0x00,0x08,0x09,0x43,0x43,0x43,0x44,0x4B,0x41,0x76,0x31};

ControlFLowP1P2_e get_controlflow_p1p2_by_se_sw(u16 errCode)
{
    ControlFLowP1P2_e cfP1P2 = CONTROL_FLOW_P1P2_NO_INFORMATION_PROVIDED;

    switch(errCode)
    {
        case SW_MAC_ERROR:
        case SW_INCORRECT_VALUES_IN_COMMAND_DATA:
        case SW_RECEIVED_DATA_INVALID_OR_ZERO:
        case SW_CMAC_ERROR:
            cfP1P2 = CONTROL_FLOW_P1P2_INVLAID_DATA_CONTENT;
            break;
        case SW_CER_EXPIRED:
            cfP1P2 = CONTROL_FLOW_P1P2_PUBLIC_KEY_EXPIRED;
            break;
        case SW_CER_FROZEN:
            break;
        case SW_VERIFY_FAILED:
            cfP1P2 = CONTROL_FLOW_P1P2_INVALID_SIGNATURE;
            break;
        case SW_INVALID_KEY_NOT_IN_SLOTLIST:
        case SW_INVALID_DK_IDENTIFIER:
            cfP1P2 = CONTROL_FLOW_P1P2_PUBLIC_KEY_NOT_TRUSTED;
            break;
        default:
            break; 
    }
    
    return cfP1P2;
}

CCCErrCode_e prepare_op_control_flow(u8 connId, u8 p1, u8 p2)
{
    u8 apduLen = 0;
    u8* apdu = devicesInfo[connId].apdu;

    apdu[apduLen++] = 0x80;
    apdu[apduLen++] = 0x3C;
    apdu[apduLen++] = p1;
    apdu[apduLen++] = p2;
    apdu[apduLen++] = 0x00;

    ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_SE, DK_MESSAGE_ID_APDU_RQ, apdu, apduLen);

    return CCC_ERR_CODE_SUCCESS;
}

CCCErrCode_e prepare_control_flow(u8 connId, ControlFLowP1P2_e p1p2)
{
    u8 apduLen = 0;
    u8* apdu = devicesInfo[connId].apdu;

    apdu[apduLen++] = 0x80;
    apdu[apduLen++] = 0x3C;
    apdu[apduLen++] = core_dcm_u16_hi(p1p2);
    apdu[apduLen++] = core_dcm_u16_lo(p1p2);
    apdu[apduLen++] = 0x00;

    ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_SE, DK_MESSAGE_ID_APDU_RQ, apdu, apduLen);

    return CCC_ERR_CODE_SUCCESS;
}

CCCErrCode_e prepare_and_send_select_aid_to_device_se_with_ble(u8 connId, u8 aidType)
{
    u8 *apdu = devicesInfo[connId].apdu;
    u8 apduLen = 0;

    apdu[apduLen++] = 0x00;
    apdu[apduLen++] = 0xA4;
    apdu[apduLen++] = 0x04;
    apdu[apduLen++] = 0x00;  

    if(aidType == 0)  //framework
    {
        apdu[apduLen++] = sizeof(AID_DIGITAL_KEY_FRAMEWORK);
        core_mm_copy(apdu + apduLen, (u8*)AID_DIGITAL_KEY_FRAMEWORK, sizeof(AID_DIGITAL_KEY_FRAMEWORK));
        apduLen += sizeof(AID_DIGITAL_KEY_FRAMEWORK);
    }
    else
    {
        apdu[apduLen++] = sizeof(AID_DIGITAL_KEY_APPLET);
        core_mm_copy(apdu + apduLen, (u8*)AID_DIGITAL_KEY_APPLET, sizeof(AID_DIGITAL_KEY_APPLET));
        apduLen += sizeof(AID_DIGITAL_KEY_APPLET);
    }
    
    apdu[apduLen++] = 0x00; 

    ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_SE, DK_MESSAGE_ID_APDU_RQ, apdu, apduLen);
    return CCC_ERR_CODE_SUCCESS; 
}

CCCErrCode_e prepare_and_send_auth0_to_device_se_with_ble(u8 connId, u8 p1, u8 p2)
{
    //send cmd generate transaction materials to vehicle se
    u8* apdu = devicesInfo[connId].apdu;
    u8* apduResp = devicesInfo[connId].apduResp;
    u16 respLen;
    u16 apduLen = 0;
    u8 res = 0;

    apdu[apduLen++] = 0x80;
    apdu[apduLen++] = 0xC2;
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
        //车端临时公钥	0x87	0x41	临时ecc公钥
        //随机交易标识	0x4C	0x10	随机产生
        if(respLen != 0x57)
        {
            return CCC_ERR_CODE_DATA_ERR;
        }
        
        apduLen = 0;
        apdu[apduLen++] = 0x80;
        apdu[apduLen++] = 0x80;
        apdu[apduLen++] = p1;
        apdu[apduLen++] = p2;

        apduLen++;  //skip lc
        core_mm_copy(apdu + apduLen, vehicleInfo.dkAppVerTlv, 0x04);
        apduLen += 4;

        core_mm_copy(apdu + apduLen, apduResp, respLen - 2);  //vehicle_ePK prepended by 04h + transaction_identifier (randomly generated)
        apduLen += (respLen - 2);

        core_mm_copy(apdu + apduLen, vehicleInfo.vehicleIdTlv, 10);
        apduLen += 10;   //vehicle_identifier

        apdu[4] = (apduLen - 5);  //set lc;

        ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_SE, DK_MESSAGE_ID_APDU_RQ, apdu, apduLen);

        devicesInfo[connId].auth0P1 = p1;
        devicesInfo[connId].auth0P2 = p2;
        return CCC_ERR_CODE_SUCCESS;
    }
    else
    {
        return core_dcm_mku16(apduResp[respLen - 2], apduResp[respLen - 1]);
    }
}

CCCErrCode_e parse_auth0_resp_and_send_auth1(u8 connId, u8* apdub, u16 apdubLen)
{
    u8* apdu = devicesInfo[connId].apdu;
    u8* apduResp = devicesInfo[connId].apduResp;
    u16 respLen;
    u16 apduLen = 0;
    u8 res = 0;

    //86 41 pk + (9D 10 cryptogram) + 9000
    if(apdubLen != 0x57 && apdubLen != 0x45)
    {
        return CCC_ERR_CODE_DATA_ERR;
    }

    if(ccc_dk_func_get_sw_from_apdu_resp(apdub, apdubLen) != 0x9000)
    {
        return core_dcm_mku16(apdub[apdubLen - 2], apdub[apdubLen - 1]);
    }

    //verify auth0;
    apdu[apduLen++] = 0x80;
    apdu[apduLen++] = 0x80;
    apdu[apduLen++] = 0x00;
    apdu[apduLen++] = connId;

    apduLen++;  //skip lc

    core_mm_copy(apdu + apduLen, apdub, apdubLen - 2);
    apduLen += (apdubLen - 2);   

    apdu[apduLen++] = 0x38;
    apdu[apduLen++] = 0x02;
    apdu[apduLen++] = devicesInfo[connId].auth0P1;
    apdu[apduLen++] = devicesInfo[connId].auth0P2;

    apdu[apduLen++] = 0x17;
    apdu[apduLen++] = 0x0D;
    ccc_dk_func_get_current_datetime_in_formate_UTC(apdu + apduLen);
    apduLen += 0x0D;

    core_mm_copy(apdu + apduLen, vehicleInfo.dkAppVerTlv, 0x04);   //5C 02 XXXX
    apduLen += 0x04;

    apdu[4] = (apduLen - 5);  //LC

    res = ccc_dk_func_send_apdu_to_vehicle_se(apdu, apduLen, apduResp, &respLen);
    if(res != 0 || respLen < 2)
    {
        return CCC_ERR_CODE_SE_COMMUNICATE_ERR;
    }
    else if(apduResp[respLen - 2] == 0x90 && apduResp[respLen - 1] == 0x00)
    {
        //output	auth1_command[80810000lc][9ETLV]
        ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_SE, DK_MESSAGE_ID_APDU_RQ, apduResp, (respLen - 2));
        return CCC_ERR_CODE_SUCCESS;
    }
    else
    {
        return core_dcm_mku16(apduResp[respLen - 2], apduResp[respLen - 1]);
    }
}

CCCErrCode_e parse_auth1_resp_with_vehicle_se(u8 connId, u8* apdub, u16 apdubLen)
{
    u8* apdu = devicesInfo[connId].apdu;
    u8* apduResp = devicesInfo[connId].apduResp;
    u16 respLen;
    u16 apduLen = 0;
    u8 res = 0;

    if(apdubLen < 2)
    {
        return CCC_ERR_CODE_DATA_ERR;
    }

    if(ccc_dk_func_get_sw_from_apdu_resp(apdub, apdubLen) != 0x9000)
    {
        return core_dcm_mku16(apdub[apdubLen - 2], apdub[apdubLen - 1]);
    }

    //verify auth1;
    apdu[apduLen++] = 0x80;
    apdu[apduLen++] = 0x81;
    apdu[apduLen++] = 0x00;
    apdu[apduLen++] = connId;
    apdu[apduLen++] = (apdubLen - 2);
    core_mm_copy(apdu + apduLen, apdub, apdubLen - 2);
    apduLen += (apdubLen - 2);

    res = ccc_dk_func_send_apdu_to_vehicle_se(apdu, apduLen, apduResp, &respLen);
    if(res != 0 || respLen < 2)
    {
        return CCC_ERR_CODE_SE_COMMUNICATE_ERR;
    }
    else if((apduResp[respLen - 2] == 0x90 && apduResp[respLen - 1] == 0x00)
            || (apduResp[respLen - 2] == 0x64 && apduResp[respLen - 1] == 0x02))
    {
        devicesInfo[connId].authFlag = TRUE;
        core_mm_copy(devicesInfo[connId].slotIdLV, apduResp + 1, (1 + apduResp[1]));
        return CCC_ERR_CODE_SUCCESS;
    }
    else
    {
        return core_dcm_mku16(apduResp[respLen - 2], apduResp[respLen - 1]);
    }
}

CCCErrCode_e prepare_and_send_mailbox_operation_inquire(u8 connId, u8* apduForDevice, u16* apduForDeviceLen)
{
    u8* apdu = devicesInfo[connId].apdu;
    u8* apduResp = devicesInfo[connId].apduResp;
    u16 respLen;
    u16 apduLen = 0;
    u8 res = 0;

    apdu[apduLen++] = 0x80;
    apdu[apduLen++] = 0xCA;
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
        core_mm_copy(apduForDevice, apduResp, respLen - 2);
        *apduForDeviceLen = (respLen - 2);
        return CCC_ERR_CODE_SUCCESS;
    }
    else
    {
        return core_dcm_mku16(apduResp[respLen - 2], apduResp[respLen - 1]);
    }
}

CCCErrCode_e prepare_and_send_create_ranging_key_to_device_with_ble(u8 connId, u8* arbitraryData, u8 arbitraryDataLen)
{
    u8 *apdu = devicesInfo[connId].apdu;
    u16 apduLen = 0;

    apdu[apduLen++] = 0x80;
    apdu[apduLen++] = 0x71;
    apdu[apduLen++] = 0x00;
    apdu[apduLen++] = 0x00;

    apdu[apduLen++] = arbitraryDataLen;

    if(arbitraryDataLen > 0)
    {
        core_mm_copy(apdu + apduLen, arbitraryData, arbitraryDataLen);
        apduLen += arbitraryDataLen;
    }

    ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_SE, DK_MESSAGE_ID_APDU_RQ, apdu, apduLen);
    return CCC_ERR_CODE_SUCCESS;
}

CCCErrCode_e parse_mailbox_resp_and_generate_mailbox_operation(u8 connId, u8* apdub, u16 apdubLen, u8* apduForDevice, u16* apduForDeviceLen, u16* seSw)
{
    u8* apdu = devicesInfo[connId].apdu;
    u8* apduResp = devicesInfo[connId].apduResp;
    u16 respLen;
    u16 apduLen = 0;
    u8 res = 0;
    u16 sw = 0x9000;

    if(apdubLen < 2)
    {
        return CCC_ERR_CODE_DATA_ERR;
    }

    if(ccc_dk_func_get_sw_from_apdu_resp(apdub, apdubLen) != 0x9000)
    {
        return core_dcm_mku16(apdub[apdubLen - 2], apdub[apdubLen - 1]);
    }

    //Mailbox operate response
    apdu[apduLen++] = 0x80;
    apdu[apduLen++] = 0xCB;
    apdu[apduLen++] = 0x00;
    apdu[apduLen++] = connId;
    apdu[apduLen++] = apdubLen;

    apdu[apduLen++] = 0xC1;
    if((apdubLen - 2) >= 0x80)
    {
        apdu[apduLen++] = 0x81;
    }
    apdu[apduLen++] = (apdubLen - 2);

    core_mm_copy(apdu + apduLen, apdub, apdubLen - 2);
    apduLen += (apdubLen - 2);

    apdu[4] = (apduLen - 5);

    res = ccc_dk_func_send_apdu_to_vehicle_se(apdu, apduLen, apduResp, &respLen);
    if(res != 0 || respLen < 2)
    {
        return CCC_ERR_CODE_SE_COMMUNICATE_ERR;
    }
    else
    {
        sw = core_dcm_mku16(apduResp[respLen - 2], apduResp[respLen - 1]);
        if(sw == 0x9181 || sw == 0x9190 || sw == 0x9191 || sw == 0x9481 || 
            sw == 0x9482 || sw == 0x9483 || sw == 0x9488 || sw == 0x9489 || sw == 0x9000)
    {
        core_mm_copy(apduForDevice, apduResp, respLen - 2);
        *apduForDeviceLen = (respLen - 2);
            *seSw = sw;

        return CCC_ERR_CODE_SUCCESS;
    }
       
        return sw;
    }
}

//-----------------------standard transaction flow-----------------
CCCErrCode_e ccc_dk_select_aid_to_device_in_standard_transaction(u8 connId)
{
    prepare_and_send_select_aid_to_device_se_with_ble(connId, 0x01);
    devicesInfo[connId].processOn = PROCESS_STANDARD_TRANSACTION_FLOW;
    devicesInfo[connId].subProcessOn = STANDARD_TRANSACTION_STATUS_SELECT;
}

CCCErrCode_e process_apdu_resp_in_standard_transaction(u8 connId, u8* apdub, u16 apdubLen)
{
    CCCErrCode_e errCode = CCC_ERR_CODE_SUCCESS;
    u16 sw;
    u16 apduLen;

    switch(devicesInfo[connId].subProcessOn)
    {
        case STANDARD_TRANSACTION_STATUS_SELECT:
            errCode = parse_select_applet_aid_in_device(connId, apdub, apdubLen);
            if(errCode == CCC_ERR_CODE_SUCCESS)
            {
                //-------------------prepare and send auth0------------------
                //p1 = 0x00: Standard(0) transaction request, exchange cmd may be send,
                //p2 = 0x06: User authentication request by vehicle
                errCode = prepare_and_send_auth0_to_device_se_with_ble(connId, 0x00, 
                    devicesInfo[connId].workMode == DEVICE_WORK_MODE_ON_FIRST_APPROACH ? TRANSCTION_TYPE_FIRST_APPROACH_AFTER_BLE_PAIRING : TRANSCTION_TYPE_USER_AUTHENTICATION_REQUEST);
                if(errCode == CCC_ERR_CODE_SUCCESS)
                {
                    devicesInfo[connId].subProcessOn = STANDARD_TRANSACTION_STATUS_AUTH_0;
                }
            }
            break;
        case STANDARD_TRANSACTION_STATUS_AUTH_0:
            errCode = parse_auth0_resp_and_send_auth1(connId, apdub, apdubLen);
            if(errCode == CCC_ERR_CODE_SUCCESS)
            {
                devicesInfo[connId].subProcessOn = STANDARD_TRANSACTION_STATUS_AUTH_1;
            }
#if 1
            else
            {
                prepare_control_flow(connId, get_controlflow_p1p2_by_se_sw(errCode)); 
                devicesInfo[connId].authFlag = FALSE;
                devicesInfo[connId].subProcessOn = STANDARD_TRANSACTION_STATUS_CONTROL_FLOW;
            }
#endif
            break;
        case STANDARD_TRANSACTION_STATUS_AUTH_1:
            errCode = parse_auth1_resp_with_vehicle_se(connId, apdub, apdubLen);

            if(errCode == CCC_ERR_CODE_SUCCESS)
            {
                devicesInfo[connId].authFlag = TRUE;
                
                errCode = prepare_and_send_mailbox_operation_inquire(connId, apdub, &apduLen);
                if(errCode == CCC_ERR_CODE_SUCCESS)
                {
                    if(apduLen > 0)
                    {
                        ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_SE, DK_MESSAGE_ID_APDU_RQ, apdub, apduLen);

                        devicesInfo[connId].subProcessOn = STANDARD_TRANSACTION_STATUS_EXCHANGE;
                    }
                    else  //没有exchange指令
                    {
                        prepare_control_flow(connId, CONTROL_FLOW_P1P2_SUCCESS_ON_STANDARD_AUTH);
                        devicesInfo[connId].subProcessOn = STANDARD_TRANSACTION_STATUS_CONTROL_FLOW;
                    }
                }
                else
                {
                    prepare_control_flow(connId, get_controlflow_p1p2_by_se_sw(errCode)); 
                    devicesInfo[connId].authFlag = FALSE;
                    devicesInfo[connId].subProcessOn = STANDARD_TRANSACTION_STATUS_CONTROL_FLOW;
                }
            }
#if 1
            else
            {
                prepare_control_flow(connId, get_controlflow_p1p2_by_se_sw(errCode)); 
                devicesInfo[connId].authFlag = FALSE;
                devicesInfo[connId].subProcessOn = STANDARD_TRANSACTION_STATUS_CONTROL_FLOW;
            }
#endif
            break;
        case STANDARD_TRANSACTION_STATUS_EXCHANGE:
            errCode = parse_mailbox_resp_and_generate_mailbox_operation(connId, apdub, apdubLen, apdub, &apduLen, &sw);

            if(errCode == CCC_ERR_CODE_SUCCESS)
            {
                if(apduLen > 0)
                {
                    ccc_dk_msg_prepare_and_send(connId, DK_MESSAGE_TYPE_SE, DK_MESSAGE_ID_APDU_RQ, apdub, apduLen);
                    devicesInfo[connId].subProcessOn = STANDARD_TRANSACTION_STATUS_EXCHANGE;
                }
                else  //no more exchange cmd;  CONTROL_FLOW_Command(P1='01', P2='81')
                {
                    //send_command_complete_subevent_in_dk_event_notification(connId, Command_Status_Deselect_SE);
                    //devicesInfo[connId].subProcessOn = STANDARD_TRANSACTION_STATUS_FREE;

                    prepare_op_control_flow(connId, ( (core_dcm_u16_hi(sw) == 0x94) ? 0x40 : 0x01 ), core_dcm_u16_lo(sw));
                    devicesInfo[connId].subProcessOn = STANDARD_TRANSACTION_STATUS_CONTROL_FLOW;
                }
            }
            else  //falied
            {
                //transaction finished with failure
                prepare_control_flow(connId, get_controlflow_p1p2_by_se_sw(errCode)); 
                devicesInfo[connId].authFlag = FALSE;
                devicesInfo[connId].subProcessOn = STANDARD_TRANSACTION_STATUS_CONTROL_FLOW;
            }
            break;
        case STANDARD_TRANSACTION_STATUS_CONTROL_FLOW:
            send_command_complete_subevent_in_dk_event_notification(connId, Command_Status_Deselect_SE);

            if(devicesInfo[connId].authFlag)
            {
                //sucess
                if(devicesInfo[connId].workMode == DEVICE_WORK_MODE_ON_FIRST_APPROACH)  //friend firtst approach
                {
                    //goto ursk derivation;
                    ccc_timer_start(connId, TIMER_ID_GENERATE_URSK, TIMER_TYPE_ONCE, 1000, ccc_timer_handler_URSK_Derivation);
                }
                else if(devicesInfo[connId].workMode == DEVICE_WORK_MODE_ON_NORMAL_USE)
                {
                    errCode = ccc_dk_find_ursk_to_ranging(connId, FALSE);
                }
            }
            else
            {
                devicesInfo[connId].processOn = PROCESS_IDLE;
                devicesInfo[connId].subProcessOn = 0x00;
            }
            break;
    }

    return errCode;
}   

CCCErrCode_e msg_dk_se_message_process(u8 connId, CCCDkMsg_t * cccDkMsg_t)
{
    CCCErrCode_e errCode = CCC_ERR_CODE_SUCCESS;
    u16 dataLen = ccc_dk_msg_get_data_len(cccDkMsg_t);

    if(devicesInfo[connId].workMode == DEVICE_WORK_MODE_ON_PAIRING)
    {
        switch(devicesInfo[connId].processOn)
        {
            case PROCESS_OWNER_PAIRING_PHASE2:
                errCode = process_apdu_resp_in_onwer_pairing_phase2(connId, cccDkMsg_t->data, dataLen);
                break;
            case PROCESS_URSK_DERIVATION_FLOW:
                errCode = process_apdu_resp_in_URSK_Derivation(connId, cccDkMsg_t->data, dataLen);
                break;
            case PROCESS_OWNER_PAIRING_PHASE3:
                errCode = process_apdu_resp_in_phase3_in_onwer_pairing(connId, cccDkMsg_t->data, dataLen);
                break;
            case PROCESS_OWNER_PAIRING_PHASE4:
                errCode = process_apdu_resp_in_phase4_in_onwer_pairing(connId, cccDkMsg_t->data, dataLen);
                break; 
            default:   //not in pairing;
                errCode = process_apdu_resp_in_standard_transaction(connId, cccDkMsg_t->data, dataLen);
                break;
        }
    }
    else
    {
        if(devicesInfo[connId].processOn == PROCESS_URSK_DERIVATION_FLOW)
        {
            errCode = process_apdu_resp_in_URSK_Derivation(connId, cccDkMsg_t->data, dataLen);
        }
        else
        {
            errCode = process_apdu_resp_in_standard_transaction(connId, cccDkMsg_t->data, dataLen);
        }
    }
    
    if(errCode != CCC_ERR_CODE_SUCCESS)
    {
        devicesInfo[connId].processOn = PROCESS_IDLE;

        if(devicesInfo[connId].rangingSession.sessionStatus == RANGING_SESSION_ACTIVE)
        {
            //如果在测试距，则terminate掉；
            LOG_L_S(CCC_MD, "TERMINATED55555555555\r\n");
            ccc_ranging_on_failed(devicesInfo[connId].rangingSession.uwbSessionId, 0);
        }

        cccConfig->eventNotice(connId, SDK_EVENT_BLE_DISCONECT, NULL, 0);
    }
}
