#include "ccc_dk_api.h"
#include "ccc_dk.h"
#include "ccc_dk_ble_msg.h"
#include "ccc_can.h"
#include "ccc_dk_type.h"
#include "Board_Config.h"
#include "Ble_Spi_Drv.h"
#include "TimersManager.h"
#include "SecLib.h"
#include "rtc_handle.h"
#include "ble_ccc.h"
#include "Board_Config.h"
//#include "Ble_Gpio_Drv.h"
#include "Ble_Spi_Drv.h"
#include "se_common_type.h"
#include "../ES000501_uwb/Sources/uwb_SDK_Interface.h"

extern volatile u8 intIRQFlag;

struct ccc_config_t tempCccConfig;
void ccc_log_print(u8* pdata, u16 length)
{


}

//--------------------------SE-SPI驱动--------------------------------------
int impl_hsm_init(void)
{
	SE_PowerOff();
	// Spi_User_Init_eSE();
	// OSA_TimeDelay(100);
	// SE_PowerOn();

	Ble_ESE_CS_HIGH();
 	Spi_User_Init_eSE();
 	OSA_TimeDelay(20);
 	Ble_ESE_PowerOn();
	SE_PowerOn();
 	OSA_TimeDelay(20);

    return 0;
}
/*Reset ESE function*/
volatile uint8_t geSESpiUseFlg = 0;
volatile uint8_t geSESpiRecvOverFlg = 0;
 int impl_hsm_reset(void)
{
	if(geSESpiUseFlg == 1)
	{
		OSA_TimeDelay(4);
	}
	geSESpiUseFlg = 1;
	OSA_TimeDelay(2);
	Ble_ESE_PowerOff();
	SE_PowerOff();
	Ble_ESE_CS_LOW();
    OSA_TimeDelay(2);
    Ble_ESE_CS_HIGH();
    Ble_ESE_PowerOn();
	SE_PowerOn();
    geSESpiUseFlg = 0;
    return 0;
}

void impl_hsm_delay(u32_t ms)
{
	OSA_TimeDelay(ms);
    return ;
}
 /* HSM spi receive */
 int impl_hsm_spi_recvieve(u8_t *rbuff, u16_t length)
 {
 	// u16_t recv_cnt = 0;
	// spi_rx_data_eSE_Temp2(rbuff,length);
	// recv_cnt = length;
 	uint16_t recv_cnt;
 	uint8_t buffer = 0;

// 	if(geSESpiUseFlg == 1)
// 	{
// 		OSA_TimeDelay(2);
// 	}
// 	if(geSESpiUseFlg == 1)
// 	{
// 		OSA_TimeDelay(2);
// 	}
  	geSESpiUseFlg = 1;
  	//OSA_TimeDelay(1);
  	Ble_ESE_CS_LOW();
// 	OSA_TimeDelay(1);//at least 120 us before transmitting data
 	for (recv_cnt = 0; recv_cnt < length; recv_cnt++)
 	{
 		spi_rx_data_eSE_Temp(&buffer);
 		rbuff[recv_cnt] = buffer;
 	}
 	Ble_ESE_CS_HIGH();
 	geSESpiUseFlg = 0;
 	geSESpiRecvOverFlg = 1;
 	return recv_cnt;

 }
 /* HSM spi send */
 int impl_hsm_spi_send(u8_t *tbuff, u16_t length)
 {
 	// u16_t send_cnt = 0;
	// spi_tx_data_eSE_Temp2(tbuff,tbuff);
	// send_cnt = length;
 	// return send_cnt;
	uint16_t send_cnt;

	if(geSESpiRecvOverFlg == 1)
	{
		OSA_TimeDelay(1);
		geSESpiRecvOverFlg = 0;
	}
// 	if(geSESpiUseFlg == 1)
// 	{
// 		OSA_TimeDelay(2);
// 	}
// 	if(geSESpiUseFlg == 1)
// 	{
// 		OSA_TimeDelay(2);
// 	}
 	geSESpiUseFlg = 1;
// 	OSA_TimeDelay(1);
 	spi_tx_data_eSE_Temp2(tbuff,length);
 	geSESpiUseFlg = 0;

 	send_cnt = length;
 	return send_cnt;
 }
//--------------------------------------------------------------------------------

//
//typedef struct
//{
//	pExtFunc_create_timer					createTimer;            		/**< 创建timer  */
//	pExtFunc_start_timer				    startTimer;					    /**< 启动timer */
	pExtFunc_stop_timer				        stopTimer;					    /**< 停止timer */
//}timer_tool_t;
u8 es_createTimer(void)
{
	tmrTimerID_t tmpTimerId;
	tmpTimerId = TMR_AllocateTimer();
	return tmpTimerId;
}
u8 es_startTimer(int timeId, u8 timerType, u32 timeInMilliseconds, pFunc_timer_handler timerHandler, void* param)
{
		/* Start advertising timer */
	u8 timerTypeOs;
	if( timerType == TIMER_TYPE_ONCE)   //一次性ms timer;
	{
		timerTypeOs = gTmrLowPowerSingleShotMillisTimer_c;
	}
	else //重复ms timer;
	{
		timerTypeOs = gTmrLowPowerIntervalMillisTimer_c;
	}

	if(TMR_IsTimerActive(timeId))
	{
		TMR_StopTimer(timeId);
	}
		
	(void)TMR_StartLowPowerTimer(timeId,timerTypeOs,timeInMilliseconds,timerHandler, param);
	return 1;
}

u8 es_stopTimer(int timerId)
{
	if(TMR_IsTimerActive(timerId))
	{
		TMR_StopTimer(timerId);
	}
	
	return 1;
}

int es_genKey(u8* pOutPubKey, u8* pOutPriKey)
{
	ecdhPublicKey_t ecdhPublicKey;
	ecdhPrivateKey_t ecdhPrivateKey;
	pOutPubKey[0] = 0x04;
	ECDH_P256_GenerateKeys(&ecdhPublicKey,&ecdhPrivateKey);
	core_algo_swap_u8(pOutPubKey+1,ecdhPublicKey.raw,32);
	core_algo_swap_u8(pOutPubKey+1+32,ecdhPublicKey.raw+32,32);
	core_algo_swap_u8(pOutPriKey,ecdhPrivateKey.raw_8bit,32);

	return 0x00;
}
int es_ecdh(u8* pInPubKey, u8* pInPriKey, u8* pOutAgreeKey)
{
	ecdhPublicKey_t ecdhPublicKey;
	ecdhPrivateKey_t ecdhPrivateKey;
	ecdhDhKey_t ecdhDhKey;
	core_algo_swap_u8(ecdhPublicKey.raw,pInPubKey+1,32);
	core_algo_swap_u8(ecdhPublicKey.raw+32,pInPubKey+1+32,32);
	core_algo_swap_u8(ecdhPrivateKey.raw_8bit,pInPriKey,32);
	ECDH_P256_ComputeDhKey(&ecdhPrivateKey,&ecdhPublicKey,&ecdhDhKey);
	core_algo_swap_u8(pOutAgreeKey,ecdhDhKey.raw,32);
	core_algo_swap_u8(pOutAgreeKey+32,ecdhDhKey.raw+32,32);

	return 0x00;
}
extern ble_ccc_ctx_t ble_ccc_ctx[BLE_DEIVCE_MAX_NUMBER];
int es_getInfo(InfoType_e iType, u8* pInData, u16 pInLen, u8 *pOutData, u16 *pOutLen)
{
	switch (iType)
	{
	case INFO_TYPE_FIRMWARE_VER:/** 集成方固件版本 */
		
		break;
	case INFO_TYPE_VIN: /** VIN号 */
		
		break;
	case INFO_TYPE_CAN_PACK_SIZE:/** CAN上一包大小 */
		
		break;
	case INFO_TYPE_VEHICLE_ADDR: /** 车端BLE地址 Static Address*/
		core_mm_copy(pOutData, ble_ccc_ctx->macAddress,6);
		*pOutLen = 6;
		break;
	case INFO_TYPE_BLE_IRK: /** 车端BLE IRK*/
		
		break;
	case INFO_TYPE_DEVICE_ADDR:/** Device ADDR*/
		
		break;
	case INFO_TYPE_VEHICLE_ECC_PUBKEY_X_ON_OOB_PAIRING:/** OOB配对时车端公钥**/
		
		break;	
	default:
		break;
	}
}

int es_getUTCTime(struct utc_time_t *pOutTime)
{
	rtc_datetime_t date;
	rtc_get_time(&date);
#if 0
	pOutTime->seconds = date.second;
	pOutTime->minutes = date.minute;
	pOutTime->hour = date.hour;
	pOutTime->day = date.day;
	pOutTime->month = date.month;
	pOutTime->year = date.year;
#else
	pOutTime->seconds = 34;
	pOutTime->minutes = 55;
	pOutTime->hour = 20;
	pOutTime->day = 21;
	pOutTime->month = 7;
	pOutTime->year = 2022;
#endif
	return 0;
}
int es_setUTCTime(struct utc_time_t *pOutTime)
{
	rtc_datetime_t date;
	date.second = pOutTime->seconds;
	date.minute = pOutTime->minutes;
	date.hour = pOutTime->hour;
	date.day = pOutTime->day;
	date.month = pOutTime->month;
	date.year = pOutTime->year;
	rtc_set_time(&date);
	return 0;
}

void es_recv_uwb_callback(E_UWBControlMessageIndex type, uint8_t* p_msg, uint16_t* p_msglens)
{
	u8 tmpBuf[65];
	core_mm_set(tmpBuf,0x00,65);
	switch (type)
	{
	case UWB_Anchor_WakeUp_RS:
		tmpBuf[0] = CAN_PKG_ID_UWB_ANCHOR_WAKEUP_RS;
		break;
	case UWB_Timer_Sync_RS:
		tmpBuf[0] = CAN_PKG_ID_TIME_SYNC_RS;
		break;
	case UWB_Ranging_Session_Start_RS:
		KW38_INT_Start();
		tmpBuf[0] = CAN_PKG_ID_UWB_RANGING_SESSION_START_RS;
		break;
	case UWB_Ranging_Result_Notice:
		tmpBuf[0] = CAN_PKG_ID_UWB_RANGING_RESULT_NOTICE;
		break;
	default:
		break;
	}
	core_mm_copy(tmpBuf+1,p_msg,*p_msglens);
	ble_ccc_send_evt(CCC_EVT_RECV_UWB_DATA,0U,tmpBuf,65U);
}
void ble_ccc_uwb_int(void);
void es_sendCANData(ChannleID_e chId,u8* pInData,u16 pInDataLen)
{
	u8 canFDData[64U];
	core_mm_set(canFDData,0x00,64U);
	core_mm_copy(canFDData,pInData+1,pInDataLen-1);
	switch (pInData[0])
	{
	case CAN_PKG_ID_UWB_ANCHOR_WAKEUP_RQ:
		if (chId == CHANNEL_ID_CAN)
		{
			LOG_L_S(CCC_MD,"CAN Send Data ID: 0x100 \r\n");
			BCanPdu_Set_BLE100_Data(canFDData);
		}
		else
		{
#ifndef FIT_DEBUG_NO_UWB 			
			/*UWB SPI*/
			LOG_L_S(CCC_MD,"SPI Send Data ID: 0x100 \r\n");
			stUWBSDK.fpUQAnchorWakup(canFDData,64,es_recv_uwb_callback);
#endif			
		}
		break;
	case CAN_PKG_ID_TIME_SYNC_RQ:
		if (chId == CHANNEL_ID_CAN)
		{
			LOG_L_S(CCC_MD,"CAN Send Data ID: 0x101 \r\n");
			BCanPdu_Set_BLE101_Data(canFDData);
		}
		else
		{
#ifndef FIT_DEBUG_NO_UWB 			
			/*UWB SPI*/
			LOG_L_S(CCC_MD,"SPI Send Data ID: 0x101 \r\n");
			stUWBSDK.fpUQTimeSync(canFDData,64,es_recv_uwb_callback);
#endif			
		}		
		break;
	case CAN_PKG_ID_UWB_RANGING_SESSION_SETUP_RQ:
		if (chId == CHANNEL_ID_CAN)
		{
			LOG_L_S(CCC_MD,"CAN Send Data ID: 0x102 \r\n");
			BCanPdu_Set_BLE102_Data(canFDData);
		}
		else
		{
#ifndef FIT_DEBUG_NO_UWB 			
			/*UWB SPI*/
			LOG_L_S(CCC_MD,"SPI Send Data ID: 0x102 \r\n");
			stUWBSDK.fpUQRangingSessionSetup(canFDData,64);
#endif			
		}	
		break;
	case CAN_PKG_ID_UWB_RANGING_SESSION_START_RQ:
		if (chId == CHANNEL_ID_CAN)
		{
			LOG_L_S(CCC_MD,"CAN Send Data ID: 0x104 \r\n");
			BCanPdu_Set_BLE104_Data(canFDData);
		}
		else
		{
#ifndef FIT_DEBUG_NO_UWB 			
			/*UWB SPI*/
			LOG_L_S(CCC_MD,"SPI Send Data ID: 0x104 \r\n");
			stUWBSDK.fpUQRangingCtrl(UWBRangingOPType_Start,canFDData,64,es_recv_uwb_callback);
			intIRQFlag = 0x00;
#endif			
		}
		break;
	case CAN_PKG_ID_UWB_RANGING_SESSION_SUSPEND_RECOVER_DELETE_RQ:
		if (chId == CHANNEL_ID_CAN)
		{
			LOG_L_S(CCC_MD,"CAN Send Data ID: 0x103 \r\n");
			BCanPdu_Set_BLE103_Data(canFDData);
		}
		else
		{
#ifndef FIT_DEBUG_NO_UWB 			
			/*UWB SPI*/
			LOG_L_S(CCC_MD,"SPI Send Data ID: 0x103 \r\n");
//			stUWBSDK.fpUQRangingCtrl(canFDData,64,NULL);
			//stUWBSDK.fpUQRangingCtrl((E_RangingOPType)(canFDData[0]),canFDData,64,es_recv_uwb_callback);
			ble_ccc_uwb_int();
#endif
		}
		break;
	case CAN_PKG_ID_RKE_EXECUTE_RQ:
		if (chId == CHANNEL_ID_CAN)
		{
			LOG_L_S(CCC_MD,"CAN Send Data ID: 0x180 \r\n");
			BCanPdu_Set_BLE180_Data(canFDData);
		}
		else
		{
			/*UWB SPI*/
		}
		break;
	case CAN_PKG_ID_CALIBRATION_DATA_RQ:
		if (chId == CHANNEL_ID_CAN)
		{
			LOG_L_S(CCC_MD,"CAN Send Data ID: 0x181 \r\n");
			BCanPdu_Set_BLE181_Data(canFDData);
		}
		else
		{
			/*UWB SPI*/
		}
		break;
	default:
		break;
	}
	LOG_L_S_HEX(CCC_MD,"CAN Send Data:",canFDData,64);
}

int es_sendData(ChannleID_e chId, u8* pInData, u16 pInDataLen)
{
	// ble_ccc_send_data(chId - CHANNEL_ID_BLE_0,pInData,pInDataLen);
	switch (chId)
	{
	case CHANNEL_ID_BLE_0:
	//case CHANNEL_ID_BLE_1:
	//case CHANNEL_ID_BLE_2:
	case CHANNEL_ID_BLE_FOB:
		ble_ccc_send_evt(CCC_EVT_SEND_DATA,chId,pInData,pInDataLen);
		break;
	case CHANNEL_ID_BLE_PRIVATE:	
		ble_ccc_send_evt(CCC_EVT_SEND_PRIVATE_DATA,chId,pInData,pInDataLen);
		break;
	case CHANNEL_ID_CAN:
	case CHANNEL_ID_SPI_UWB:
		es_sendCANData(chId,pInData,pInDataLen);
		break;
	default:
		break;
	}
	
	return 0;
}
int es_event_notice(ChannleID_e chId, SDKEvent_e sdkEvent, u8* pInData, u16 pInDataLen)
{
	switch (sdkEvent)
	{
	case SDK_EVENT_BLE_DISCONECT:/**请求断开指定的蓝牙连接**/
		ble_ccc_send_evt(CCC_EVT_CMD_DISCONNECT,chId,pInData,pInDataLen);
		break;
	case SDK_EVENT_TIME_SYNC_TRIGGER_BY_VEHICLE:/**车端触发时间同步*/
		ble_ccc_send_leSetPhyRequest(chId);
		break;
	case SDK_EVENT_BLE_SECURE_OOB_PAIRING_PREP_FINISH: /**Figure 19-16: Bluetooth LE Secure OOB Pairing Prep已完成**/
		
		break;
	case SDK_EVENT_BLE_SECURE_OOB_MISMATCH: /**Device 通知 OOB mismatch **/
		
		break;	
	case SDK_EVENT_FA_CRYPTO_OPERATION_FAILED: /**signal the failure in First approach due to cryptography*/
		ble_ccc_send_evt(CCC_EVT_CMD_DISCONNECT,chId,pInData,pInDataLen);
		break;	
	case SDK_EVENT_TIMER_HANDLER:
		ble_ccc_send_evt(CCC_EVT_TIMER_HANDLER,chId,pInData,pInDataLen);
		break;		
	default:
		break;
	}
	
	return 0;
}
hsm_ctrl_attr_t hsm_ctrl_attr;
sw_algorithm_t swAlgorithm;
timer_tool_t timerTool;
void ccc_sdk_init(void)
{
	hsm_ctrl_attr.init = impl_hsm_init;
	hsm_ctrl_attr.reset = impl_hsm_reset;
	hsm_ctrl_attr.delay = impl_hsm_delay;
	hsm_ctrl_attr.send = impl_hsm_spi_send;
	hsm_ctrl_attr.recv = impl_hsm_spi_recvieve;

	timerTool.createTimer = es_createTimer;
	timerTool.startTimer = es_startTimer;
	timerTool.stopTimer = es_stopTimer;

	swAlgorithm.eccGenKey = es_genKey;
	swAlgorithm.eccECDH = es_ecdh;

	tempCccConfig.hsmConfig = &hsm_ctrl_attr;
    tempCccConfig.swAlgorithm = &swAlgorithm;
    tempCccConfig.timerTool = &timerTool;
    // tempCccConfig.debugPrintf = ccc_log_print;
    tempCccConfig.getInfo = es_getInfo;
    tempCccConfig.getUtcTime = es_getUTCTime;
    tempCccConfig.setUtcTime = es_setUTCTime;

    tempCccConfig.sendData = es_sendData;
    tempCccConfig.eventNotice = es_event_notice;
    ccc_api_module_init(&tempCccConfig);
}
