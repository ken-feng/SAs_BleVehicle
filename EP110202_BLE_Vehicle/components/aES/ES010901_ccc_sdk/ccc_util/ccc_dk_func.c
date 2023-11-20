#include "ccc_dk_func.h"
#include "ccc_dk_util.h"
#include "../ccc_sdk/ccc_dk_type.h"
#include "../ccc_sdk/ccc_dk.h"
#include "../../../aEM/EM00040101_log/EM000401.h"
#include "../../ES010701_se_gp_spi/se_apdu_api.h"


boolean ccc_fdk_unc_is_owner_pairing_permit(void)
{
    //TODO
    if(vehicleInfo.pairStatus == VEHICLE_STATUS_WAITING_FOR_PAIRING)
    {
        return TRUE;
    }
    
    return FALSE;
}

void ccc_dk_func_generate_random(u8* pRnd, u16 rndLen)
{
    //TODO
    //core_mm_set(pRnd, rndLen%0x100, rndLen);
    core_algo_genRandom(pRnd, rndLen);
}

u8 ccc_dk_func_send_apdu_to_vehicle_se(u8* apdu, u16 apduLen, u8* apduResp, u16* respLen)
{
    //TODO
    LOG_L_S_HEX(CCC_MD,"vehicle_se_cmd", apdu, apduLen);

    int rcvLen = api_apdu_transceive(apdu, apduLen, apduResp, 258);
    if(rcvLen >= 2)
    {
        //core_mm_copy(apduResp, gApduRespBuf, 261);
        *respLen = rcvLen;
        LOG_L_S_HEX(CCC_MD,"vehicle_se_resp", apduResp, 5);
        return 0;
    }
    else
    {
        LOG_L_S_HEX(CCC_MD,"vehicle_se_resp failed!!!", apdu, 0);

        *respLen = 0;
        return 1;
    }
}

u8 ccc_dk_func_get_current_datetime_in_formate_UTC(u8* dateTime)
{
    //UTC格式  YYMMDDHHMMSSZ  3137303431393130333433385A(170419103438Z)  //UTCTime  
    struct utc_time_t  utcTime;
    cccConfig->getUtcTime(&utcTime);
    dateTime[0] = (0x30 + ((utcTime.year)%100)/10); //Y
    dateTime[1] = (0x30 + ((utcTime.year)%100)%10); //Y
    dateTime[2] = (0x30 + ((utcTime.month)/10));    //M
    dateTime[3] = (0x30 + ((utcTime.month)%10));    //M
    dateTime[4] = (0x30 + ((utcTime.day)/10));      //D
    dateTime[5] = (0x30 + ((utcTime.day)%10));      //D
    dateTime[6] = (0x30 + ((utcTime.hour)/10));     //H
    dateTime[7] = (0x30 + ((utcTime.hour)%10));     //H
    dateTime[8] = (0x30 + ((utcTime.minutes)/10));  //M
    dateTime[9] = (0x30 + ((utcTime.minutes)%10));  //M
    dateTime[10] = (0x30 + ((utcTime.seconds)/10)); //S
    dateTime[11] = (0x30 + ((utcTime.seconds)%10)); //S
    dateTime[12] = 0x5A;                             //Z
    
    return 0x0D;
}

u16 ccc_dk_func_get_sw_from_apdu_resp(u8* apduResp, u16 respLen)
{
    if(respLen < 2)
    {
        return 0x6999;
    }
    
    return core_dcm_mku16(apduResp[respLen - 2], apduResp[respLen - 1]);
}


/*****************************************************************************
FUNCTION: check the condition to accept or delay the suspend request from device;  
PARAMETER:
        connId: the connId of the device;
        pIn_sessionId: the session id of ranging to suspend;
RETURN: 0: accept;  1: To delay the suspend and keep ranging active; others: failed
******************************************************************************/
u8 ccc_dk_func_ranging_session_suspend_permit_check(u8 connId, u8* pIn_sessionId)
{
    return 0;
}



