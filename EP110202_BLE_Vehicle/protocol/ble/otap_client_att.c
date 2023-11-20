/*! *********************************************************************************
* \addtogroup BLE OTAP Client ATT
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2019 NXP
* All rights reserved.
*
* \file
*
* This file is the source file for the BLE OTAP Client ATT application
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "EmbeddedTypes.h"

/* Framework / Drivers */
#include "RNG_Interface.h"
#include "TimersManager.h"
#include "FunctionLib.h"
#include "Panic.h"
#if (cPWR_UsePowerDownMode)
#include "PWR_Interface.h"
#endif
#include "OtaSupport.h"
#include "MemManager.h"
/* BLE Host Stack */
#include "gatt_interface.h"
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gatt_database.h"
#include "gap_interface.h"
#include "gatt_db_app_interface.h"
#if !defined(MULTICORE_APPLICATION_CORE) || (!MULTICORE_APPLICATION_CORE)
#include "gatt_db_handles.h"
#endif

/* Connection Manager */
#include "ble_conn_manager.h"
#include "gap_types.h"

#include "board.h"
#include "ApplMain.h"
#include "otap_client_att.h"
#include "EM000401.h"
#include "app_preinclude.h"
#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1)
#include "erpc_host.h"
#include "dynamic_gatt_database.h"
#include "mcmgr.h"
#endif
#include "ble_service_discovery.h"
#include "EM000101.h"
#include "EM000401.h"
#include "flash_api_extern.h"
#include "hw_flash.h"
#include "ble_ccc.h"
/************************************************************************************
*************************************************************************************
* Extern functions
*************************************************************************************
************************************************************************************/
/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#define mBatteryLevelReportInterval_c   (10)        /* battery level report interval in seconds  */

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
tmrTimerID_t appTimerId;        /*连接时设置30S不配对成功超时断开连接，连接完成后设置50ms获取rssi*/
extern uint8_t uuid_service_ccc_dk[2];

typedef struct advState_tag{
    bool_t      advOn;
    advType_t   advType;
} advState_t;

typedef enum appEvent_tag
{
    mAppEvt_PeerConnected_c,
    mAppEvt_PairingComplete_c,
    mAppEvt_ServiceDiscoveryComplete_c,
    mAppEvt_ServiceDiscoveryNotFound_c,
    mAppEvt_ServiceDiscoveryFailed_c,
    mAppEvt_GattProcComplete_c,
    mAppEvt_GattProcError_c
} appEvent_t;

gapLeScOobData_t  gCccLeScOobData;/*CCC OOB配对数据*/
gapPhyEvent_t     gCccLePhy;/**/

typedef enum
{
    WORK_MODE_SCAN = 0,         /*扫描模式*/
    WORK_MODE_ADV_CCC,          /*CCC广播模式*/
    WORK_MODE_ADV_PRIVATE,      /*私有蓝牙广播模式*/
    BLE_WORK_MODE_NUMBER,
}bleWorkMode_t;


typedef struct 
{
    bleWorkMode_t   bleWorkMode;    /*工作模式*/
    uint8_t         connectState;   /*1:connected  0:not connect*/
    deviceId_t      deviceId;       /*已连接的设备号ID*/
}ble_work_state_t;

typedef struct 
{
    ble_work_state_t    ble_work_state[BLE_WORK_MODE_NUMBER]; 
    bleWorkMode_t       currentWorkMode;        /*当前工作模式*/
}ble_work_ctx_t;


ble_work_ctx_t  ble_work_ctx;
u8 fobAddress[6] = {0x00, 0x60, 0x37, 0xAA, 0x47, 0x25};

//u8 fobAddress[6] = {0x00,0x60,0x37,0x3A,0xC3,0xC2};
// u8 fobAddress[6] = {0x00,0x60,0x37,0x3A,0xC3,0xB6}; 	//上海目前测试用的
//u8 fobAddress[6] = {0x00,0x60,0x37,0x3A,0xC3,0x78}; 	// 已经寄过去的一块Fob地址
//u8 fobAddress[6] = {0x00,0x60,0x37,0x3A,0xD6,0x7A};
//wozijide u8 fobAddress[6] = {0x00,0x60,0x37,0x3A,0xC3,0xB6};
u8 gPairCompleteFlag = 0U;
extern const gapAdStructure_t rspCCCScanStruct[];
extern const gapAdStructure_t rspPrivateScanStruct[];
/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
appPeerInfo_t mPeerInformation[gAppMaxConnections_c];
appPeerInfo_t* curPeerInformation;
gapRole_t     mGapRole;
uintn8_t mBleDeviceAddress[gcBleDeviceAddressSize_c] = { 0, 0, 0, };
uintn8_t mBleConnectStatus = 0;
uintn8_t mPhoneDeviceAddress[gcBleDeviceAddressSize_c] = { 0, 0, 0, };
uintn8_t mConnectedDeviceId = 0;/*记录当前连接的蓝牙设备索引号*/

/* Adv Parmeters */
advState_t  mAdvState;
tmrTimerID_t switchTimerId;      /*切换工作模式时间*/
tmrTimerID_t connectTimerId;      /*切换工作模式时间*/
//tmrTimerID_t uwbTimer = gTmrInvalidTimerID_c;
static uint16_t SeviceWriteNotifHandles[] = {value_yqdk_1};
#define VALUE_YQDK_IS_WRITEABLE(handle)   ((value_yqdk_1 == handle))
/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/

/* Gatt and Att callbacks */
static void BleApp_AdvertisingCallback (gapAdvertisingEvent_t* pAdvertisingEvent);
void BleApp_ScanningCallback(gapScanningEvent_t *pScanningEvent);

static void BleApp_ConnectionCallback (deviceId_t peerDeviceId, gapConnectionEvent_t* pConnectionEvent);
static void BleApp_GattServerCallback (deviceId_t deviceId, gattServerEvent_t* pServerEvent);

static void BleApp_GattClientCallback
(
    deviceId_t              serverDeviceId,
    gattProcedureType_t     procedureType,
    gattProcedureResult_t   procedureResult,
    bleResult_t             error
);

static void BleApp_ServiceDiscoveryCallback
(
    deviceId_t      peerDeviceId,
    servDiscEvent_t *pEvent
);
static void BleApp_StoreServiceHandles
(
    deviceId_t       peerDeviceId,
    gattService_t   *pService
);

static bool_t BleApp_CheckScanEvent(gapScannedDevice_t *pData);
/* Timer Callbacks */
static void ScanningTimerCallback(void *pParam);

static void BleApp_Config(void);
static void BleApp_Advertise (void);

/************************************************************************************
*************************************************************************************
* App functions
*************************************************************************************
************************************************************************************/
static void BleApp_PairingSuccessCallback(void *pParam);
static void BleApp_GetRssiCallback(void *pParam);
static void BleApp_DisconnectCallBack(void *pParam);
void BleApp_connectTimeroutCallback(void *pParam);
void BleChangeWorkMode(boolean switchFlag);
/***********************************************************************************/
static bleResult_t BleAppSendData(u8 deviceId, u16 serviceHandle,
    u16 charUuid, u16 length, u8 *testData);

/*设置蓝牙广播快慢模式*/
void BleApp_SwitchAdvMode(advType_t mode)
{
    mAdvState.advType = mode;
}
advType_t BleApp_GetAdvMode(void)
{
    return mAdvState.advType;
}


static u8 ascii2u8(u8*  src, u8 length, u8* dest)
{
	u8 i ;
	u8 tmpData;
  for(i = 0;i<length;i++)
  {
    tmpData = (src[i]>>4)&0x0f;
    if((tmpData>=0) && (tmpData<=9))
    {
      dest[2*i] =   tmpData+'0';
    }
    else
    {
      dest[2*i] =   tmpData -0x0A+'A';
    }
    tmpData = src[i]&0x0f;
    if((tmpData>=0) && (tmpData<=9))
    {
      dest[2*i+1] =   tmpData+'0';
    }
    else
    {
      dest[2*i+1] =   tmpData-0x0A+'A';
    }
  }
  return length*2;
}

static void BleAppGetBLEName(u8* bleName)
{
    ascii2u8(mBleDeviceAddress,6,bleName);
    return ;
}
static void BleApp_GetRssiCallback(void *pParam)
{
    uintn8_t index;

    if (pParam == NULL)
    {
        return ;
    }

    index = *(uintn8_t *)pParam;
    if (gInvalidDeviceId_c != mPeerInformation[index].deviceId) 
    {
        Gap_ReadRssi(mPeerInformation[index].deviceId);
    }
}

static bleResult_t BleAppSendData(u8 deviceId, u16 serviceHandle,
    u16 charUuid, u16 length, u8 *pdata)
{
    u16  handle;
    bleResult_t result;
    u16  handleCccd;
    bool_t isNotifActive;
    bleUuid_t uuid;

    if (gInvalidDeviceId_c == mPeerInformation[deviceId].deviceId) 
    {
        return gBleInvalidState_c;
    }

    uuid.uuid16 = charUuid;
    /* Get handle of  characteristic */
    result = GattDb_FindCharValueHandleInService(serviceHandle, gBleUuidType16_c, &uuid, &handle);    
    if (result != gBleSuccess_c) 
    {
        LOG_L_S(BLE_MD, "uuid %x not find\n", charUuid);
        return result;
    }

    /* Write characteristic value */
    result = GattDb_WriteAttribute(handle, length, pdata);
    if (result != gBleSuccess_c) 
    {
        LOG_L_S(BLE_MD, "uuid %x wr err %x\n", charUuid, result);
        return result;
	}

    /* Get handle of CCCD */
    if ((result = GattDb_FindCccdHandleForCharValueHandle(handle, &handleCccd)) != gBleSuccess_c) 
    {
        LOG_L_S(BLE_MD, "uuid %x cccd err %x\n", charUuid, result);
        return result;
	}

    result = Gap_CheckNotificationStatus(deviceId, handleCccd, &isNotifActive);
    if ((gBleSuccess_c == result) && (TRUE == isNotifActive)) 
    {
        result = GattServer_SendInstantValueNotification(deviceId, handle, length, pdata);
        if (result != gBleSuccess_c)
        {
            LOG_L_S(BLE_MD, "ble send value notify: did=[%d], handle=[0x%x] result=[0x%x]\n", \
                deviceId, handle, result);
        }
    } 
    else
    {
    	LOG_L_S(BLE_MD, "NotificationStatus Failed: deviceId=[%d], handleCccd=[0x%x] isNotifActive=[0x%x]\n", \
    			deviceId, handleCccd, isNotifActive);
    }
    return result;
}
/*通过设备号查找记录号*/
u8 ble_get_record_from_deviceId(u8 deviceId)
{
    for (u8 i = 0; i < BLE_WORK_MODE_NUMBER; i++)
    {
        if ((ble_work_ctx.ble_work_state[i].deviceId == deviceId)&&ble_work_ctx.ble_work_state[i].connectState)
        {
            return i;
        }
    }
    return 0xFF;
}
static uint8_t FindAvalidMode(void)
{
    uint8_t cnt = 0U;
    for (bleWorkMode_t i = 0; i < BLE_WORK_MODE_NUMBER; i++)
    {
        if(ble_work_ctx.ble_work_state[i].connectState == 1U)
        {
            cnt++;
        }
    }
    if (cnt == BLE_WORK_MODE_NUMBER)
    {
        return 0U;/*表示已经连接三路了*/
    }
    
    while(ble_work_ctx.ble_work_state[ble_work_ctx.currentWorkMode].connectState == 1U)
    {
        ble_work_ctx.currentWorkMode++;
        if (ble_work_ctx.currentWorkMode == BLE_WORK_MODE_NUMBER)
        {
            ble_work_ctx.currentWorkMode = WORK_MODE_SCAN;
        }
    }
    return 1U;
}


void BleChangeWorkMode(boolean switchFlag)
{
    if (switchFlag)
    {
        switch (ble_work_ctx.currentWorkMode)
        {
        case  WORK_MODE_SCAN:
            LOG_L_S(BLE_MD,"Stop Scan!!! \r\n");
            /* Stop scanning */
            (void)Gap_StopScanning();
            break;
        case  WORK_MODE_ADV_CCC:
            // LOG_L_S(BLE_MD,"Stop Adv CCC!!! \r\n");
            //(void)Gap_StopAdvertising();
            break;
        case  WORK_MODE_ADV_PRIVATE:
            LOG_L_S(BLE_MD,"Stop Adv Private!!! \r\n");
            (void)Gap_StopAdvertising();
            break;
        default:
            break;
        } 
        ble_work_ctx.currentWorkMode++;
    
        if (ble_work_ctx.currentWorkMode == BLE_WORK_MODE_NUMBER)
        {
            ble_work_ctx.currentWorkMode = WORK_MODE_SCAN;
        }
    }
    if(FindAvalidMode() == 0U)
    {
        (void)TMR_StopTimer(switchTimerId);
        return;
    }
    switch (ble_work_ctx.currentWorkMode)
    {
    case  WORK_MODE_SCAN:
        LOG_L_S(BLE_MD,"Start Scan!!! \r\n");
        gPairingParameters.securityModeAndLevel = gSecurityMode_1_Level_2_c;
        gPairingParameters.oobAvailable = FALSE;
        mGapRole = gGapCentral_c;
        BleApp_Start(gGapCentral_c);
        break;
    case  WORK_MODE_ADV_CCC:
        // LOG_L_S(BLE_MD,"Start Adv CCC!!! \r\n");
        // gPairingParameters.securityModeAndLevel = gSecurityMode_1_Level_4_c;
        // gPairingParameters.oobAvailable = TRUE;
        // mGapRole = gGapPeripheral_c;
        // BleApp_Start(gGapPeripheral_c);
        break;
    case  WORK_MODE_ADV_PRIVATE:
        LOG_L_S(BLE_MD,"Start Adv Private!!! \r\n");
        gPairingParameters.securityModeAndLevel = gSecurityMode_1_Level_2_c;
        gPairingParameters.oobAvailable = FALSE;
        mGapRole = gGapPeripheral_c;
        BleApp_Start(gGapPeripheral_c);
        break;
    default:
        break;
    } 
}

/***********************************************************************************/
static void SwitchTimerCallback(void *pParam)
{
   
    BleChangeWorkMode(TRUE);
}

/***********************************************************************************/

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief    Initializes application specific functionality before the BLE stack init.
*
********************************************************************************** */
void BleApp_Init(void)
{
    /* Initialize application support for drivers */
    BOARD_InitAdc();

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1)
    /* Init eRPC host */
    init_erpc_host();
#endif

    /* Initialize application specific peripheral drivers here. */
}


/*! *********************************************************************************
* \brief    Starts the BLE application.
*
********************************************************************************** */
void BleApp_Adv(void)
{
    Gap_ReadPublicDeviceAddress();
}

void BleApp_Work(void)
{
       /* Start advertising timer */
    (void)TMR_StartLowPowerTimer(switchTimerId,
                                    gTmrLowPowerIntervalMillisTimer_c,
                                    gSwitchTime_c,
                                    SwitchTimerCallback, NULL);
}

/*! *********************************************************************************
 * \brief    Starts the BLE application.
 *
 * \param[in]    gapRole    GAP Start Role (Central or Peripheral).
 ********************************************************************************** */
void BleApp_Start(gapRole_t gapRole)
{
    switch (gapRole)
    {
        case gGapCentral_c:
        {
            (void)LOG_L_S(BLE_MD, "\n\rScanning...\n\r");
#if defined(gUseControllerNotifications_c) && (gUseControllerNotifications_c)
            Gap_ControllerEnhancedNotification(gNotifScanEventOver_c | gNotifScanAdvPktRx_c |
                                               gNotifScanRspRx_c | gNotifScanReqTx_c | gNotifConnCreated_c, 0);
#endif
            gPairingParameters.localIoCapabilities = gIoNone_c;
            gPairingParameters.securityModeAndLevel = gSecurityMode_1_Level_2_c;
            (void)App_StartScanning(&gScanParams, BleApp_ScanningCallback, gGapDuplicateFilteringEnable_c, gGapScanContinuously_d, gGapScanPeriodicDisabled_d);
            break;
        }
        case gGapPeripheral_c:
        {
			BleApp_Adv();
            break;
        }
        default:
        {
            ; /* No action required */
            break;
        }
    }
}

#if defined(gUseControllerNotifications_c) && (gUseControllerNotifications_c)
static void BleApp_HandleControllerNotification(bleNotificationEvent_t *pNotificationEvent)
{
    switch(pNotificationEvent->eventType)
    {
        case gNotifEventNone_c:
        {
//            LOG_L_S(BLE_MD, "Configured notification status:0x%02x \r\n", pNotificationEvent->status);
            break;
        }

        case gNotifConnEventOver_c:
        {
            LOG_L_S(BLE_MD, "CONN Event Over device :0x%02x, on channel:0x%02x,with RSSI:0x%02x,and event counter:0x%04x \r\n",
            		pNotificationEvent->deviceId, pNotificationEvent->channel,
					(uint8_t)pNotificationEvent->rssi, (uint16_t)pNotificationEvent->ce_counter);
            break;
        }

        case gNotifConnRxPdu_c:
        {
//            LOG_L_S(BLE_MD, "CONN Rx PDU from device:0x%02x, on channel:0x%02x, with RSSI:0x%02x, with event counter:0x%04x, and timestamp:0x%04x \r\n",
//            		pNotificationEvent->deviceId, pNotificationEvent->channel,(uint8_t)pNotificationEvent->rssi,
//            		(uint16_t)pNotificationEvent->ce_counter, pNotificationEvent->timestamp);
            break;
        }

        case gNotifAdvEventOver_c:
        {
            //LOG_L_S(BLE_MD, "ADV Event Over.\n\r");
            break;
        }

        case gNotifAdvTx_c:
        {
            //LOG_L_S(BLE_MD, "ADV Tx on channel :0x%02x \r\n", pNotificationEvent->channel);
            break;
        }

        case gNotifAdvScanReqRx_c:
        {
            //LOG_L_S(BLE_MD, "ADV Rx Scan Req on channel:0x%02x  with RSSI:0x%02x \r\n", pNotificationEvent->channel, (uint8_t)pNotificationEvent->rssi);
            break;
        }

        case gNotifAdvConnReqRx_c:
        {
            //LOG_L_S(BLE_MD, "ADV Rx Conn Req on channel:0x%02x  with RSSI:0x%02x \r\n", pNotificationEvent->channel, (uint8_t)pNotificationEvent->rssi);
            break;
        }

        case gNotifScanEventOver_c:
        {
            //LOG_L_S(BLE_MD, "SCAN Event Over on channel:0x%02x \r\n ", pNotificationEvent->channel);
            break;
        }

        case gNotifScanAdvPktRx_c:
        {
            //LOG_L_S(BLE_MD, "SCAN Rx Adv Pkt on channel:0x%02x  with RSSI:0x%02x \r\n", pNotificationEvent->channel, (uint8_t)pNotificationEvent->rssi);
            break;
        }

        case gNotifScanRspRx_c:
        {
            //LOG_L_S(BLE_MD, "SCAN Rx Scan Rsp on channel:0x%02x  with RSSI:0x%02x \r\n ", pNotificationEvent->channel, (uint8_t)pNotificationEvent->rssi);
            break;
        }

        case gNotifScanReqTx_c:
        {
            //LOG_L_S(BLE_MD, "SCAN Tx Scan Req on channel:0x%02x \r\n ", pNotificationEvent->channel);
            break;
        }

        case gNotifConnCreated_c:
        {
            LOG_L_S(BLE_MD, "CONN Created with device:0x%02x with timestamp:0x%04x\r\n", pNotificationEvent->deviceId, pNotificationEvent->timestamp);
            break;
        }

        default:
        {
            ; /* No action required */
            break;
        }
    }
}
#endif


#if defined(gCccL2Cap_d) && (gCccL2Cap_d == 1)
/*! *********************************************************************************
* \brief        Callback for incoming credit based data.
*
* \param[in]    deviceId        The device ID of the connected peer that sent the data
* \param[in]    lePsm           Channel ID
* \param[in]    pPacket         Pointer to incoming data
* \param[in]    packetLength    Length of incoming data
********************************************************************************** */
static void BleApp_L2capPsmDataCallback (deviceId_t     deviceId,
                                         uint16_t       lePsm,
                                         uint8_t*       pPacket,
                                         uint16_t       packetLength)
{
    LOG_L_S(BLE_MD,"L2CAP lePsm:0x%04x deviceId:0x%02x \r\n",lePsm,deviceId);
    // OtapClient_HandleDataChunk (deviceId,packetLength,pPacket);
    LOG_L_S_HEX(BLE_MD,"L2CAP Receive Data:",pPacket,packetLength);
    ble_ccc_send_evt(CCC_EVT_RECV_DATA,deviceId,pPacket,packetLength);
}

/*! *********************************************************************************
* \brief        Callback for control messages.
*
* \param[in]    pMessage    Pointer to control message
********************************************************************************** */
static void BleApp_L2capPsmControlCallback(l2capControlMessage_t* pMessage)
{
    bool_t isBonded = FALSE;
    uint8_t nvmIndex = gInvalidNvmIndex_c;
    u8 tmpBuffer[32];
    bleResult_t bleResult;
    LOG_L_S(BLE_MD,"l2capControlMessageType: 0x%02x \r\n",pMessage->messageType);
    switch (pMessage->messageType)
    {
        case gL2ca_LePsmConnectRequest_c:
        {
            l2caLeCbConnectionRequest_t *pConnReq = &pMessage->messageData.connectionRequest;

            /* This message is unexpected on the OTAP Client, the OTAP Client sends L2CAP PSM connection
             * requests and expects L2CAP PSM connection responses.
             * Disconnect the peer. */
            //(void)Gap_Disconnect (pConnReq->deviceId);
            /* Respond to the peer L2CAP CB Connection request - send a connection response. */
            bleResult = L2ca_ConnectLePsm (pConnReq->lePsm,
                               pConnReq->deviceId,
                               pConnReq->initialCredits);
            LOG_L_S(BLE_MD,"Result: 0x%02x L2ca_ConnectLePsm: 0x%04x DeviceId: 0x%02x initialCredits: 0x%04x\r\n",
                    bleResult,pConnReq->lePsm,pConnReq->deviceId,pConnReq->initialCredits);                   
            break;
        }
        case gL2ca_LePsmConnectionComplete_c:
        {
            l2caLeCbConnectionComplete_t *pConnComplete = &pMessage->messageData.connectionComplete;

            /* Call the application PSM connection complete handler. */
            // OtapClient_HandlePsmConnectionComplete (pConnComplete);
            if (pMessage->messageData.connectionComplete.result == gSuccessful_c)
            {   
                LOG_L_S(BLE_MD,"L2CAP Connect Success,peerMps: 0x%04x mtu: 0x%04x initialCredits: 0x%04x DeviceId: 0x%02x Channel Id:0x%04x \r\n",
                    pMessage->messageData.connectionComplete.peerMps,pMessage->messageData.connectionComplete.peerMtu,pMessage->messageData.connectionComplete.initialCredits,
                    pMessage->messageData.connectionComplete.deviceId,pMessage->messageData.connectionComplete.cId);
                core_dcm_writeBig16(tmpBuffer,pMessage->messageData.connectionComplete.cId); 
                ble_ccc_send_evt(CCC_EVT_L2CAP_SETUP_COMPLETE,pMessage->messageData.connectionComplete.deviceId,tmpBuffer,2U);
                tmpBuffer[0] = 0x00;//type  0:Mobile, 01:FOB
                if(1U == ble_ccc_get_ltk_from_macaddress(tmpBuffer,mBleDeviceAddress))
                {
                    tmpBuffer[1] = 0x00;//Flag-00:绑定 LTK, 01:LTK 未绑定
                }            
                else
                {
                    tmpBuffer[1] = 0x01;//Flag-00:绑定 LTK, 01:LTK 未绑定
                }         
                ble_ccc_evt_notify_sdk(SDK_EVT_TAG_BLE_CONNECT,pMessage->messageData.connectionComplete.deviceId,tmpBuffer,2);
            
                /*建立成功后马上走配对流程*/
                if ((gBleSuccess_c == Gap_CheckIfBonded(pMessage->messageData.connectionComplete.deviceId, &isBonded, &nvmIndex)) &&
                    (isBonded))
                {
                    LOG_L_S(BLE_MD,"Device Is Bonded !! Enter Gap EncryptLink!!! \r\n");     
                    (void)Gap_EncryptLink(pMessage->messageData.connectionComplete.deviceId);
                }
                else
                {
                    LOG_L_S(BLE_MD,"Device Is Not Bonded !! Enter Gap Pair!!! \r\n"); 
                    (void)Gap_Pair(pMessage->messageData.connectionComplete.deviceId, &gPairingParameters);
                }
            
            }
            else
            {
                LOG_L_S(BLE_MD,"L2CAP Connect Failed, result : 0x%02x \r\n",pMessage->messageData.connectionComplete.result );
                /*如果当前L2CAP建立失败，则断开蓝牙连接*/
                (void)Gap_Disconnect (pMessage->messageData.connectionComplete.deviceId);
                ble_ccc_send_evt(CCC_EVT_L2CAP_DISCONNECT,pMessage->messageData.disconnection.deviceId,NULL,0U);
            }
            break;
        }
        case gL2ca_LePsmDisconnectNotification_c:
        {
            l2caLeCbDisconnection_t *pCbDisconnect = &pMessage->messageData.disconnection;

            /* Call the application PSM disconnection handler. */
            // OtapClient_HandlePsmDisconnection (pCbDisconnect);
            // ble_ccc_l2cap_set_connect_status(BLE_L2CAP_STATUS_DISCONNECT);
            ble_ccc_send_evt(CCC_EVT_L2CAP_DISCONNECT,pMessage->messageData.disconnection.deviceId,NULL,0U);
            break;
        }
        case gL2ca_NoPeerCredits_c:
        {
            l2caLeCbNoPeerCredits_t *pCbNoPeerCredits = &pMessage->messageData.noPeerCredits;
            (void)L2ca_SendLeCredit (pCbNoPeerCredits->deviceId,
                               ble_ccc_ctx_get_channelId_from_deviceId(pCbNoPeerCredits->deviceId),//otapClientData.l2capPsmChannelId,
                               mAppLeCbInitialCredits_c);
            break;
        }
        case gL2ca_LocalCreditsNotification_c:
        {
            l2caLeCbLocalCreditsNotification_t *pMsg = &pMessage->messageData.localCreditsNotification;

            break;
        }
        case gL2ca_Error_c:
        {
            /* Handle error */
            LOG_L_S(BLE_MD,"L2CAP Connect Failed, result : 0x%02x \r\n",pMessage->messageData.error.errorSource );
            /*如果当前L2CAP建立失败，则断开蓝牙连接*/
            (void)Gap_Disconnect (pMessage->messageData.connectionComplete.deviceId);
            break;
        }
        default:
            ; /* For MISRA compliance */
            break;
    }
}
#endif

/*! *********************************************************************************
* \brief        Handles BLE generic callback.
*
* \param[in]    pGenericEvent    Pointer to gapGenericEvent_t.
********************************************************************************** */
void BleApp_GenericCallback (gapGenericEvent_t* pGenericEvent)
{
    u8 evtByte;
    /* Call BLE Conn Manager */
    BleConnManager_GenericEvent(pGenericEvent);

    //LOG_L_S(BLE_MD, "Generic EVT=[%d]\n", pGenericEvent->eventType);
    switch (pGenericEvent->eventType)
    {
        case gInitializationComplete_c:
        {
            BleApp_Config();
        }
        break;

        case gPublicAddressRead_c:
        {
#if (OTAP_ROLE == OTAP_PERIPHERALS)              
            mBleDeviceAddress[0] = pGenericEvent->eventData.aAddress[5];
            mBleDeviceAddress[1] = pGenericEvent->eventData.aAddress[4];
            mBleDeviceAddress[2] = pGenericEvent->eventData.aAddress[3];
            mBleDeviceAddress[3] = pGenericEvent->eventData.aAddress[2];
            mBleDeviceAddress[4] = pGenericEvent->eventData.aAddress[1];
            mBleDeviceAddress[5] = pGenericEvent->eventData.aAddress[0];
            LOG_L_S(BLE_MD, "BLE MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
                mBleDeviceAddress[0], mBleDeviceAddress[1], \
                mBleDeviceAddress[2], mBleDeviceAddress[3], \
                mBleDeviceAddress[4], mBleDeviceAddress[5]);

            /* Set advertising parameters*/
            gAdvParams.filterPolicy = gProcessAll_c;       
#if (cPWR_UsePowerDownMode)
            PWR_ChangeDeepSleepMode(gAppDeepSleepMode_c);
#endif            
#endif        
            if(ble_work_ctx.currentWorkMode != WORK_MODE_SCAN)
            {
            	BleApp_Advertise();
            }
        }
        break;

        case gAdvertisingParametersSetupComplete_c:
        {
            u32 passkey = 0;//, random = 0;
            u8 device_name[13] = { 0, 0, 0, };
            //u8 status = 0;
			BleAppGetBLEName(device_name);
			advDataUpdate(device_name, 12);
            Gap_SetLocalPasskey(passkey);
            /*设置DEVICE NAME*/
            uint16_t valueHandle = 0, serviceHandle = 0, length;
            bleUuid_t   uuid;
            bleResult_t result;

            u8 deviceName[15] = {'F', 'I', 'T', 
	                             '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0' };                               
            uuid.uuid16 = gBleSig_GenericAccessProfile_d;
            (void)GattDb_FindServiceHandle(1, gBleUuidType16_c, &uuid, &serviceHandle);
            uuid.uuid16 = gBleSig_GapDeviceName_d;
            (void)GattDb_FindCharValueHandleInService(serviceHandle, gBleUuidType16_c, &uuid, &valueHandle);
            BleAppGetBLEName(deviceName+3);
            result =  GattDb_WriteAttribute(valueHandle, 15, deviceName);         
            if (result != gBleSuccess_c)
            {
                LOG_L_S(BLE_MD, "set up device name %x wr err %x\n", valueHandle, result);
            }
            /*开始广播*/
            if (ble_work_ctx.currentWorkMode == WORK_MODE_ADV_PRIVATE)
            {
                gAppScanRspData.aAdStructures = (void*)rspPrivateScanStruct;
            }
            else
            {
                gAppScanRspData.aAdStructures = (void*)rspCCCScanStruct ;
            }
            (void)Gap_SetAdvertisingData(&gAppAdvertisingData, &gAppScanRspData);

            LOG_L_S(BLE_MD, "Advertising Device Name:[EP010201%s]\n", device_name);
            LOG_L_S(BLE_MD, "Start Advertising, advMode:%d \n",mAdvState.advType);
        }
        break;

        case gAdvertisingDataSetupComplete_c:
        {
            (void)App_StartAdvertising(BleApp_AdvertisingCallback, BleApp_ConnectionCallback);
        }
        break;

        case gAdvertisingSetupFailed_c:
        {
            panic(0,0,0,0);
        }
        break;

#if defined(gUseControllerNotifications_c) && (gUseControllerNotifications_c)
        case gControllerNotificationEvent_c:
        {
            BleApp_HandleControllerNotification(&pGenericEvent->eventData.notifEvent);
        }
        break;
#endif
        case gLePhyEvent_c:
        {
            LOG_L_S(BLE_MD,"gLePhyEvent_c....phyEventType:%d deviceId:%d txPhy:%d rxPhy:%d\r\n",
                    pGenericEvent->eventData.phyEvent.phyEventType,
                    pGenericEvent->eventData.phyEvent.deviceId,
                    pGenericEvent->eventData.phyEvent.txPhy,
                    pGenericEvent->eventData.phyEvent.rxPhy);
            core_mm_copy(&gCccLePhy,&pGenericEvent->eventData.phyEvent,sizeof(gapPhyEvent_t));
            ble_ccc_send_evt(CCC_EVT_LESETPHY,pGenericEvent->eventData.phyEvent.deviceId,&gCccLePhy,sizeof(gapPhyEvent_t));
        }
        break;
        case gLeScLocalOobData_c:
        {
            core_mm_copy(&gCccLeScOobData,&pGenericEvent->eventData.localOobData,sizeof(gapLeScOobData_t));
            LOG_L_S_HEX(BLE_MD,"OOB Data:r (Random value)",pGenericEvent->eventData.localOobData.randomValue,gSmpLeScRandomValueSize_c);
            LOG_L_S_HEX(BLE_MD,"OOB Data:Cr (Random Confirm value)",pGenericEvent->eventData.localOobData.confirmValue,gSmpLeScRandomConfirmValueSize_c);
        }
        break;
        case gBondCreatedEvent_c:
        {
            LOG_L_S_HEX(BLE_MD," Phone Pub Address: ",pGenericEvent->eventData.bondCreatedEvent.address,\
                gcBleDeviceAddressSize_c);
            core_algo_swap_u8(mPhoneDeviceAddress, pGenericEvent->eventData.bondCreatedEvent.address, \
                gcBleDeviceAddressSize_c);
            ble_ccc_send_evt(CCC_EVT_STATUS_MAC_UPDATE,0U,mPhoneDeviceAddress,gcBleDeviceAddressSize_c);
        }   
        break;

        
        default:
            ; /* For MISRA compliance */
            break;
    }
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/
static void ScanningTimerCallback(void *pParam)
{
    LOG_L_S(BLE_MD,"Stop Scan!!! \r\n");
    /* Stop scanning */
    (void)Gap_StopScanning();
}
/*! *********************************************************************************
* \brief        Configures BLE Stack after initialization. Usually used for
*               configuring advertising, scanning, white list, services, et al.
*
********************************************************************************** */
static void BleApp_Config(void)
{
    uint8_t tmpBuf[7];
    uint8_t ret;
#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1)
    if (GattDbDynamic_CreateDatabase() != gBleSuccess_c)
    {
        panic(0,0,0,0);
        return;
    }
#endif /* MULTICORE_APPLICATION_CORE */

    /* Common GAP configuration */
    BleConnManager_GapCommonConfig();

 #if defined(gCccL2Cap_d) && (gCccL2Cap_d == 1)
    /* Register CCC L2CAP PSM */
    L2ca_RegisterLePsm (gCcc_L2capLePsm_c,
                        gCccCmdMaxLength_c);  /*!< The negotiated MTU must be higher than the biggest data chunk that will be sent fragmented */

    /* Register stack callbacks */
    (void)App_RegisterLeCbCallbacks(BleApp_L2capPsmDataCallback, BleApp_L2capPsmControlCallback);
#endif

    /* Register stack callbacks */
    (void)App_RegisterGattServerCallback (BleApp_GattServerCallback);
 	(void)App_RegisterGattClientProcedureCallback(BleApp_GattClientCallback);
    GattServer_RegisterHandlesForWriteNotifications (NumberOfElements(SeviceWriteNotifHandles),
                                                         SeviceWriteNotifHandles);
	BleServDisc_RegisterCallback(BleApp_ServiceDiscoveryCallback);													 	
#if (gHidService_c)||(gHidService_c)														 
	GattServer_RegisterHandlesForWriteNotifications(NumberOfElements(cpHandles), cpHandles);
#endif
	for (uint8_t i = 0; i < gAppMaxConnections_c; i++)
	{
		mPeerInformation[i].deviceId= gInvalidDeviceId_c;
	}
#if (gHidService_c)||(gHidService_c)	
	(void)Hid_Start(&hidServiceConfig);
#endif
    /* By default, always start node as GAP central */

	/* Allocate aplication timer */
    switchTimerId = TMR_AllocateTimer();
    appTimerId = TMR_AllocateTimer();
    connectTimerId = TMR_AllocateTimer();
    // uwbTimer = TMR_AllocateTimer();
#if defined(gUseControllerNotifications_c) && (gUseControllerNotifications_c)
#if defined(gUseControllerNotificationsCallback_c) && (gUseControllerNotificationsCallback_c)
    Controller_RegisterEnhancedEventCallback(BleApp_ControllerNotificationCallback);
#endif
#endif 

#if (cPWR_UsePowerDownMode)
    PWR_ChangeDeepSleepMode(gAppDeepSleepMode_c);
#endif

    core_mm_set((u8*)&ble_work_ctx,0x00,sizeof(ble_work_ctx_t));
    core_mm_set((u8*)&gCccLeScOobData,0x00,sizeof(gapLeScOobData_t));


    //     /* Start advertising timer */
    // (void)TMR_StartLowPowerTimer(switchTimerId,
    //                                 gTmrLowPowerIntervalMillisTimer_c,
    //                                 gSwitchTime_c,
    //                                 SwitchTimerCallback, NULL);

    mGapRole = gGapCentral_c;

    Gap_ClearWhiteList();
    core_mm_set(tmpBuf,0x00,7U);

    ret = hw_flash_read_from_ble_area(NVM_DATA_ID_NUM0_BLE_CONNECT_MAC,tmpBuf,7U);
    LOG_L_S_HEX(BLE_MD, "*******hw_flash_read:", tmpBuf, 7);
    if (tmpBuf[0] == 1U)
    {
        Gap_AddDeviceToWhiteList(gBleAddrTypePublic_c,tmpBuf+1);
    }
    BleApp_Start(gGapCentral_c);
}

/*! *********************************************************************************
* \brief        Configures GAP Advertise parameters. Advertise will satrt after
*               the parameters are set.
*
********************************************************************************** */
static void BleApp_Advertise(void)
{
    /* Set advertising parameters*/
    (void)Gap_SetAdvertisingParameters(&gAdvParams);
}

/*! *********************************************************************************
 * \brief        Handles BLE Scanning callback from host stack.
 *
 * \param[in]    pScanningEvent    Pointer to gapScanningEvent_t.
 ********************************************************************************** */
void BleApp_ScanningCallback(gapScanningEvent_t *pScanningEvent)
{
    switch (pScanningEvent->eventType)
    {
        case gDeviceScanned_c:
        {
            if (BleApp_CheckScanEvent(&pScanningEvent->eventData.scannedDevice))
            {
                (void)Gap_StopScanning();
                LOG_L_S_NO_LINE(BLE_MD,"\r\n");                 
                gConnReqParams.peerAddressType = pScanningEvent->eventData.scannedDevice.addressType;
                FLib_MemCpy(gConnReqParams.peerAddress,
                            pScanningEvent->eventData.scannedDevice.aAddress,
                            sizeof(bleDeviceAddress_t));
#if gAppUsePrivacy_d
                gConnReqParams.usePeerIdentityAddress = pScanningEvent->eventData.scannedDevice.advertisingAddressResolved;
#endif
                LOG_L_S(BLE_MD,"Begin Connect!!!\r\n"); 
                (void)App_Connect(&gConnReqParams, BleApp_ConnectionCallback);
            }
        }
        break;

        case gScanStateChanged_c:
        {
            
        }
        break;

        case gScanCommandFailed_c:
        {
            panic(0, 0, 0, 0);
            NVIC_SystemReset();
            while(1);
            break;
        }

        default:
        {
            ; /* No action required */
            break;
        }
    }
}
/*! *********************************************************************************
* \brief        Handles BLE Advertising callback from host stack.
*
* \param[in]    pAdvertisingEvent    Pointer to gapAdvertisingEvent_t.
********************************************************************************** */
static void BleApp_AdvertisingCallback (gapAdvertisingEvent_t* pAdvertisingEvent)
{
    switch (pAdvertisingEvent->eventType)
    {
        case gAdvertisingStateChanged_c:
        {
            mAdvState.advOn = !mAdvState.advOn;

            if(mAdvState.advOn)
            {
            }
        }
        break;

        case gAdvertisingCommandFailed_c:
        {
            panic(0,0,0,0);
        }
        break;

        default:
            ; /* For MISRA compliance */
            break;
    }
}

/*! *********************************************************************************
* \brief        Handles BLE Connection callback from host stack.
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    pConnectionEvent    Pointer to gapConnectionEvent_t.
********************************************************************************** */
static void BleApp_ConnectionCallback (deviceId_t peerDeviceId, gapConnectionEvent_t* pConnectionEvent)
{
    uint8_t recordIndex;
    gapRole_t   tGapRole;
    u8 tmpBuf[10] = {0};
    uintn8_t index = peerDeviceId;
    uint8_t nvmIndex = gInvalidNvmIndex_c; 
    bool_t isBonded = FALSE;
    if (pConnectionEvent->eventType != gConnEvtRssiRead_c)
    {
        LOG_L_S(BLE_MD, "Connection CB: DID=[%d] EVT=[0x%x]\n", peerDeviceId, pConnectionEvent->eventType);
    }
    /* Connection Manager to handle Host Stack interactions */
    //BleConnManager_GapPeripheralEvent(peerDeviceId, pConnectionEvent);
    recordIndex = ble_get_record_from_deviceId(peerDeviceId);
    switch (pConnectionEvent->eventType)
    {
        case gConnEvtConnected_c:
        {
            //if (TMR_IsTimerActive(switchTimerId))
            {
                (void)TMR_StopTimer(switchTimerId);
            }
            if(ble_work_ctx.currentWorkMode == WORK_MODE_ADV_CCC)
            {
                Gap_Disconnect(peerDeviceId);
                return;
			}
            TMR_StartLowPowerTimer(connectTimerId, gTmrLowPowerSingleShotMillisTimer_c,5000, BleApp_connectTimeroutCallback, &peerDeviceId);
            
            ble_work_ctx.ble_work_state[ble_work_ctx.currentWorkMode].bleWorkMode = ble_work_ctx.currentWorkMode;
            ble_work_ctx.ble_work_state[ble_work_ctx.currentWorkMode].deviceId = peerDeviceId;
            ble_work_ctx.ble_work_state[ble_work_ctx.currentWorkMode].connectState = 1U;
            /* Subscribe client*/
            mPeerInformation[peerDeviceId].deviceId = peerDeviceId;
            FLib_MemCpyReverseOrder(mPhoneDeviceAddress, pConnectionEvent->eventData.connectedEvent.peerAddress, \
                gcBleDeviceAddressSize_c);
            LOG_L_S_HEX(BLE_MD, "Phone Address: ", mPhoneDeviceAddress, gcBleDeviceAddressSize_c);
//            LOG_L_S(BLE_MD, "connect: isBonded [%d]\n\n", isBonded);

			curPeerInformation = &mPeerInformation[index];
#if defined(gUseControllerNotifications_c) && (gUseControllerNotifications_c)
            Gap_ControllerEnhancedNotification(gNotifConnRxPdu_c, peerDeviceId);
#endif
#if (cPWR_UsePowerDownMode)
            /* Device does not need to sleep until some information is exchanged with the peer. */
            PWR_DisallowDeviceToSleep();
#endif
            tmpBuf[0] = ble_work_ctx.currentWorkMode;

            if ((gBleSuccess_c == Gap_CheckIfBonded(peerDeviceId, &isBonded, &nvmIndex)) &&
                            (isBonded))
            {
                tmpBuf[1] = 1U;/*第一次连接*/
            }
            else
            {
                tmpBuf[1] = 0U;
            }
            
            core_mm_copy(tmpBuf+2,mPhoneDeviceAddress,gcBleDeviceAddressSize_c);
			ble_ccc_send_evt(CCC_EVT_STATUS_CONNECT,peerDeviceId,tmpBuf,2+gcBleDeviceAddressSize_c);
            /*连接后唤醒CAN网络*/
//            Board_Set_WakeupSource(WAKEUP_SRC_BLE);
            Board_LowPower_Recovery();

            if (mGapRole == gGapCentral_c)
            {
                BleApp_StateMachineHandler(peerDeviceId, mAppEvt_PeerConnected_c);
            }
        }
        break;

        case gConnEvtPairingRequest_c:
        {
#if (defined(gAppUsePairing_d) && (gAppUsePairing_d == 1U))
            LOG_L_S(BLE_MD, "Accept Pairing\n");
#else
            LOG_L_S(BLE_MD, "Reject Pairing\n");
#endif            
        }
        break;

        case gConnEvtParameterUpdateComplete_c:
        {
            LOG_L_S(BLE_MD, "Connect Param Update: %d %d %d %d\n",
                pConnectionEvent->eventData.connectionUpdateComplete.status,
                pConnectionEvent->eventData.connectionUpdateComplete.connInterval,
                pConnectionEvent->eventData.connectionUpdateComplete.connLatency,
                pConnectionEvent->eventData.connectionUpdateComplete.supervisionTimeout);
        }
        break;
        case gConnEvtLeScOobDataRequest_c:
        {
            Gap_LeScSetPeerOobData(peerDeviceId,&gCccLeScOobData);
            LOG_L_S(BLE_MD, "gConnEvtLeScOobDataRequest_c\n");
        }
        break;
        case gConnEvtOobRequest_c:
        {
            LOG_L_S(BLE_MD, "gConnEvtOobRequest_c\n");
        }
        break;
        case gConnEvtLeDataLengthChanged_c:
        {
            LOG_L_S(BLE_MD, "Le Data Length Changed\n");
        }
        break;

        case gConnEvtPasskeyDisplay_c:
        {
            LOG_L_S(BLE_MD, "PasskeyDisplay: %d\n", pConnectionEvent->eventData.passkeyForDisplay);
        }
        break;

        case gConnEvtAuthenticationRejected_c:
        {
            LOG_L_S(BLE_MD, "Authentication Rejected: [0x%x]\n",  pConnectionEvent->eventData.failReason);
            if (mGapRole == gGapCentral_c)
            {
                (void)Gap_Pair(peerDeviceId, &gPairingParameters);
            }
            else
            {
                Gap_Disconnect (peerDeviceId);
            }
        }
        break;

        case gConnEvtDisconnected_c:
        {
            uintn8_t bondedCount = 0;
#if (gHidService_c)||(gHidService_c)
			(void)Hid_Unsubscribe();
#endif
            if ((pConnectionEvent->eventData.disconnectedEvent.reason == 0x122)||(pConnectionEvent->eventData.disconnectedEvent.reason == 0x13E))
            {
                NVIC_SystemReset();
            }
            
            /* Unsubscribe client */
            core_mm_set(&mPeerInformation[peerDeviceId],0x00,sizeof(appPeerInfo_t));
            mPeerInformation[peerDeviceId].deviceId = gInvalidDeviceId_c;

            if (gBleSuccess_c == Gap_GetBondedDevicesCount(&bondedCount)) 
            {
                LOG_L_S(BLE_MD, "Bonded Count=%d\n", bondedCount);
                if (bondedCount >= gMaxBondedDevices_c)
                {
                    static uint8_t nvmIndex = 0;
                
                    bleResult_t ret = Gap_RemoveBond(nvmIndex);
                    LOG_L_S(BLE_MD, "Remove bond=%d %x\n", nvmIndex, ret);
                    nvmIndex = (nvmIndex + 1) % gMaxBondedDevices_c;
                }
            }

            BleApp_DisconnectCallBack(&index);
            PWR_AllowDeviceToSleep();

            ble_ccc_send_evt(CCC_EVT_STATUS_DISCONNECT,peerDeviceId,NULL,0);
            
            recordIndex = ble_get_record_from_deviceId(peerDeviceId);
            if (ble_work_ctx.ble_work_state[recordIndex].bleWorkMode == WORK_MODE_SCAN)
            {
                if (TMR_IsTimerActive(appTimerId))
                {
                    (void)TMR_StopTimer(appTimerId);
                }
                else
                {

                }
            }
            else
            {
                
            }

            ble_work_ctx.ble_work_state[recordIndex].connectState = 0U;
            ble_work_ctx.ble_work_state[recordIndex].deviceId = 0U;
            ble_work_ctx.ble_work_state[recordIndex].bleWorkMode = 0U;
            BleChangeWorkMode(FALSE);
            //if (TMR_IsTimerActive(switchTimerId))
            {
                (void)TMR_StopTimer(switchTimerId);
            }
           // else
            {

            }
            /* Start advertising timer */
            (void)TMR_StartLowPowerTimer(switchTimerId,
                                    gTmrLowPowerIntervalMillisTimer_c,
                                    gSwitchTime_c,
                                    SwitchTimerCallback, NULL);

            LOG_L_S(BLE_MD, "BLE [%d] Disconnected,reason:[0x%x]\n\n", peerDeviceId,pConnectionEvent->eventData.disconnectedEvent.reason);
        }
        break;

#if gAppUsePairing_d
        case gConnEvtPairingComplete_c:
        {
            if (pConnectionEvent->eventData.pairingCompleteEvent.pairingSuccessful) 
            {

                
                // ble_ccc_send_evt(CCC_EVT_BLE_PAIR_COMPLETE,peerDeviceId,NULL,0U);
                
                BleApp_PairingSuccessCallback(&index);
                LOG_L_S(BLE_MD, "Device bonding [0x%x]\n", pConnectionEvent->eventData.pairingCompleteEvent.pairingCompleteData.withBonding);
                
                //BleApp_StateMachineHandler(peerDeviceId,mAppEvt_PairingComplete_c);
            } 
            else 
            {
                // for ios
                LOG_L_S(BLE_MD, "Pair Fail [0x%x]\n", pConnectionEvent->eventData.pairingCompleteEvent.pairingCompleteData.failReason);
                Gap_Disconnect (peerDeviceId);
            }
        }
        break;
        case gConnEvtEncryptionChanged_c:
        {
            if (pConnectionEvent->eventData.encryptionChangedEvent.newEncryptionState) 
            {
                if ((gBleSuccess_c == Gap_CheckIfBonded(peerDeviceId, &isBonded, &nvmIndex)) &&
                                (isBonded))
                {                
                    /*如果是已绑定的设备则通知CCC SDK配对完成*/
                    LOG_L_S(BLE_MD, "Notify CCC SDK\n");
                    // ble_ccc_send_evt(CCC_EVT_BLE_PAIR_COMPLETE,peerDeviceId,NULL,0U);
                }
                LOG_L_S(BLE_MD, "Encryption Success\n");
                BleApp_PairingSuccessCallback(&index);
            } 
            else 
            {
                LOG_L_S(BLE_MD, "Encryption Fail\n");
                Gap_Disconnect(peerDeviceId);
            }

        }
        break;
#endif /* gAppUsePairing_d */

        case gConnEvtRssiRead_c:
        {
            // RSSI for an active connection has been read.
            tmpBuf[0] = pConnectionEvent->eventData.rssi_dBm;
            //LOG_L_S(BLE_MD, "RSSI = %d\n", rssi_dbm);
            ble_ccc_send_evt(CCC_EVENT_RSSI_UPDATE,peerDeviceId,tmpBuf,1);
        }
        break;

    default:
        ; /* For MISRA compliance */
        break;
    }
	    /* Connection Manager to handle Host Stack interactions */
    //switch (mPeerInformation[peerDeviceId].gapRole)
    if (ble_work_ctx.ble_work_state[recordIndex].bleWorkMode == WORK_MODE_SCAN)
    {
        tGapRole = gGapCentral_c;
    }
    else
    {
        tGapRole = gGapPeripheral_c;
    }
	switch(tGapRole)
    {
        case gGapCentral_c:
            BleConnManager_GapCentralEvent(peerDeviceId, pConnectionEvent);
            break;
        case gGapPeripheral_c:
            BleConnManager_GapPeripheralEvent(peerDeviceId, pConnectionEvent);
            break;

        default:
            ; /* No action required */
            break;
    }
}
#define NOP1 __asm volatile ("NOP")
static bool_t MatchDataInAdvElementList(gapAdStructure_t *pElement,
                                        void *pData,
                                        uint8_t iDataLen)
{
    uint8_t i;
    bool_t status = FALSE;

    for (i = 0; i < (pElement->length - 1U); i += iDataLen)
    {
        if (core_mm_compare(pData, &pElement->aData[i], iDataLen) == 0U)
        {
        	// NOP1;
        	// NOP1;
        	// NOP1;
        	// NOP1;
        	// NOP1;
            status = TRUE;
            break;
        }
    }

    return status;
}
extern uint8_t uuid_service_ccc_dk[2];
extern uint8_t CCCServiceDataIntentUUID[19];




static bool_t BleApp_CheckScanEvent(gapScannedDevice_t *pData)
{
    uint8_t index = 0;
    bool_t foundMatch = FALSE;
    bool_t foundMatch2 = FALSE;
    uint8_t name[64];
    u8 macAddress[6];
    core_mm_set(name,0x00,64);
    core_mm_copy(macAddress,pData->aAddress,6U);
    
 

    while (index < pData->dataLength)
    {
        gapAdStructure_t adElement;
        adElement.length = pData->data[index];
        adElement.adType = (gapAdType_t) pData->data[index + 1U];
        adElement.aData = &pData->data[index + 2U];
        /* Search for CCC DK Service */
        if ((adElement.adType == gAdIncomplete16bitServiceList_c)
            || (adElement.adType == gAdComplete16bitServiceList_c))
        {
        	//  LOG_L_S_NO_LINE(BLE_MD,"\r\n");
			// LOG_L_S_NO_LINE(BLE_MD,"Service Uuid:0x%02x%02x    ",adElement.aData[1],adElement.aData[0]);

            foundMatch = MatchDataInAdvElementList(&adElement,uuid_service_ccc_dk, 2);
            if (foundMatch)
            {
//                LOG_L_S_NO_LINE(BLE_MD,"\r\n");
//                LOG_L_S_NO_LINE(BLE_MD,"Service Uuid:0x%02x%02x    ",adElement.aData[1],adElement.aData[0]);
            }
            
        }
        if ((adElement.adType == gAdCompleteLocalName_c)
            || (adElement.adType == gAdShortenedLocalName_c))
        {
            core_mm_copy(name,adElement.aData,adElement.length);
            LOG_L_S_NO_LINE(BLE_MD,"Name:%s    MAC: %02x:%02x:%02x:%02x:%02x:%02x   ",name,macAddress[5],macAddress[4],macAddress[3],macAddress[2],macAddress[1],macAddress[0]);
            LOG_L_S_NO_LINE(BLE_MD,"\r\n"); 
        }
        if (adElement.adType == gAdServiceData128bit_c)
        {
            foundMatch2 = MatchDataInAdvElementList(&adElement,CCCServiceDataIntentUUID, 16);
        }
        
        /* Move on to the next AD element type */
        index += adElement.length + sizeof(uint8_t);
    }
    // core_algo_swap_u8(macAddress,pData->aAddress,6U);
    // if (core_mm_compare(macAddress,fobAddress,6U) == 0U)
    // {
    //     return TRUE;
    // }
    // if (core_mm_compare(macAddress,fobAddressF0B1,6U) == 0U)
    // {
    //     return TRUE;
    // }
    // if (foundMatch2 && foundMatch)
    // {
    //     return TRUE;
    // }
    // return FALSE;
    return TRUE;
}
static void BleApp_ServiceDiscoveryCallback(deviceId_t peerDeviceId, servDiscEvent_t *pEvent)
{
    switch (pEvent->eventType)
    {
        case gServiceDiscovered_c:
        {
        	if (pEvent->eventData.pService->uuidType == gBleUuidType16_c)
			{
				if (FLib_MemCmp((void *)&uuid_service_ccc_dk, (void *)&pEvent->eventData.pService->uuid, 2))
				{
					BleApp_StoreServiceHandles(peerDeviceId, pEvent->eventData.pService);
				}
			}
        }
        break;

        case gDiscoveryFinished_c:
        {
            if (pEvent->eventData.success)
            {
                if (gGattDbInvalidHandleIndex_d != mPeerInformation[peerDeviceId].clientInfo.hService)
                {
                    BleApp_StateMachineHandler(peerDeviceId,
                                               mAppEvt_ServiceDiscoveryComplete_c);
                }
                else
                {
                    BleApp_StateMachineHandler(peerDeviceId,
                                               mAppEvt_ServiceDiscoveryNotFound_c);
                }
            }
            else
            {
                BleApp_StateMachineHandler(peerDeviceId,
                                           mAppEvt_ServiceDiscoveryFailed_c);
            }
        }
        break;

        default:
        {
            ; /* No action required */
        }
        break;
    }
}
/*! *********************************************************************************
* \brief        Handles GATT client callback from host stack.
*
* \param[in]    serverDeviceId      GATT Server device ID.
* \param[in]    procedureType       Procedure type.
* \param[in]    procedureResult     Procedure result.
* \param[in]    error               Callback result.
********************************************************************************** */
static void BleApp_GattClientCallback(
    deviceId_t              serverDeviceId,
    gattProcedureType_t     procedureType,
    gattProcedureResult_t   procedureResult,
    bleResult_t             error
)
{
    switch (procedureResult)
    {
        case gGattProcError_c:
            BleApp_StateMachineHandler(serverDeviceId, mAppEvt_GattProcError_c);
            break;

        case gGattProcSuccess_c:
            BleApp_StateMachineHandler(serverDeviceId, mAppEvt_GattProcComplete_c);
            break;

        default:
            ; /* No action required */
            break;
    }

    /* Signal Service Discovery Module */
    BleServDisc_SignalGattClientEvent(serverDeviceId, procedureType, procedureResult, error);
}
/*! *********************************************************************************
 * \brief        Stores handles used by the application.
 *
 * \param[in]    pService    Pointer to gattService_t.
 ********************************************************************************** */
static void BleApp_StoreServiceHandles(deviceId_t peerDeviceId, gattService_t *pService)
{
	bleResult_t bleResult;
    gattCharacteristic_t*   pCurrentChar; 
    gattCharacteristic_t 	pReadChar;
    for (uint8_t i = 0; i < pService->cNumCharacteristics; i++)
    {
        pCurrentChar = pService->aCharacteristics + i;
        if (0U == core_mm_compare(pCurrentChar->value.uuid.uuid128,uuid_char_ccc_dk,16))
        {
        	pReadChar.value.handle = pCurrentChar->value.handle;
        	pReadChar.value.paValue = MEM_BufferAlloc(20);
        	bleResult = GattClient_ReadCharacteristicValue(mPeerInformation[peerDeviceId].deviceId,&pReadChar,20);
            mPeerInformation[peerDeviceId].lePsm = core_dcm_readBig16(pReadChar.value.paValue);
            LOG_L_S(BLE_MD,"bleResult:0x%02x  adv lePsm:0x%04x \r\n",bleResult,mPeerInformation[peerDeviceId].lePsm);
            return;
        }
    }
    
}
#if 1
void BleApp_SendUartStream(uint8_t *pRecvStream, uint8_t streamSize)
{
    gattCharacteristic_t characteristic = {gGattCharPropNone_c, {0}, 0, 0};
    uint8_t              mPeerId = 0;
    bleResult_t     bleResult;
    /* send UART stream to all peers */
    for (mPeerId = 0; mPeerId < (uint8_t)gAppMaxConnections_c; mPeerId++)
    {
        if (gInvalidDeviceId_c != mPeerInformation[mPeerId].deviceId &&
            mAppRunning_c == mPeerInformation[mPeerId].appState)
        {
            characteristic.value.handle = mPeerInformation[mPeerId].clientInfo.hUartStream;
            bleResult = GattClient_WriteCharacteristicValue(mPeerId, &characteristic,
                    streamSize, pRecvStream, TRUE,
                    FALSE, FALSE, NULL);
            LOG_L_S(BLE_MD,"Send Data bleResult:0x%02x \r\n",bleResult);
        }
    }
}
#endif

void BleApp_StateMachineHandler(deviceId_t peerDeviceId, appEvent_t event)
{
    u16 recordIndex;
    bleResult_t bleResult;
    uint16_t tempMtu = 0;
    bool_t isBonded = FALSE;
    uint8_t nvmIndex = gInvalidNvmIndex_c;
    union
    {
        uint8_t     *pUuidArray;
        bleUuid_t   *pUuidObj;
    } temp; /* MISRA rule 11.3 */
    recordIndex = ble_get_record_from_deviceId(peerDeviceId);


    temp.pUuidArray = uuid_service_ccc_dk;

    /* invalid client information */
    if (gInvalidDeviceId_c == mPeerInformation[peerDeviceId].deviceId)
    {
        return;
    }
//    LOG_L_S(BLE_MD,"appState:0x%02x  event:0x%02x \r\n",mPeerInformation[peerDeviceId].appState,event);
    switch (mPeerInformation[peerDeviceId].appState)
    {
        case mAppIdle_c:
        {
            if (event == mAppEvt_PeerConnected_c)
            {
                /* Let the central device initiate the Exchange MTU procedure*/
                if (mGapRole == gGapCentral_c)
                {
                    /* Moving to Exchange MTU State */
                    mPeerInformation[peerDeviceId].appState = mAppExchangeMtu_c;
                    (void)GattClient_ExchangeMtu(peerDeviceId, gAttMaxMtu_c);
//                    LOG_L_S(BLE_MD,"GattClient_ExchangeMtu!!! \r\n");
                }
                else
                {
                    /* Moving to Service Discovery State*/
                    mPeerInformation[peerDeviceId].appState = mAppServiceDisc_c;

                    /* Start Service Discovery*/
                    (void)BleServDisc_FindService(peerDeviceId,
                                                  gBleUuidType16_c,
                                                  temp.pUuidObj);
//                    LOG_L_S(BLE_MD,"BleServDisc_FindService!!! \r\n");
                }
            }
        }
        break;

        case mAppExchangeMtu_c:
        {
            if (event == mAppEvt_GattProcComplete_c)
            {
                /* update stream length with minimum of maximum MTU's of connected devices */
                (void)Gatt_GetMtu(peerDeviceId, &tempMtu);
                tempMtu = gAttMaxWriteDataSize_d(tempMtu);

                //mAppUartBufferSize = mAppUartBufferSize <= tempMtu ? mAppUartBufferSize : tempMtu;
                mPeerInformation[peerDeviceId].att_mtu = tempMtu;
                /* Moving to Service Discovery State*/
                mPeerInformation[peerDeviceId].appState = mAppServiceDisc_c;

                /* Start Service Discovery*/
                (void)BleServDisc_FindService(peerDeviceId,
                                                  gBleUuidType16_c,
                                                  temp.pUuidObj);
//                LOG_L_S(BLE_MD,"BleServDisc_FindService!!! \r\n");
            }
            else
            {
                if (event == mAppEvt_GattProcError_c)
                {
                    (void)Gap_Disconnect(peerDeviceId);
                    LOG_L_S(BLE_MD,"Gap_Disconnect!!! \r\n");
                }
            }
        }
        break;

        case mAppServiceDisc_c:
        {
            if (event == mAppEvt_ServiceDiscoveryComplete_c)
            {
                /* Moving to Running State*/
                mPeerInformation[peerDeviceId].appState = mAppRunning_c;
#if gAppUseBonding_d
                /* Write data in NVM */
                (void)Gap_SaveCustomPeerInformation(mPeerInformation[peerDeviceId].deviceId,
                                                    (void *) &mPeerInformation[peerDeviceId].clientInfo, 0,
                                                    sizeof(wucConfig_t));
#endif
                if (ble_work_ctx.ble_work_state[recordIndex].bleWorkMode == WORK_MODE_SCAN)
                {
                    /*建立L2CAP连接*/
                    bleResult = L2ca_ConnectLePsm (gCcc_L2capLePsm_c,
                                mPeerInformation[peerDeviceId].deviceId,
                                mAppLeCbInitialCredits_c);
                    LOG_L_S(BLE_MD,"L2CAP CONNECT!!! bleResult:0x%02x \r\n",bleResult); 
                }
            }
            else if (event == mAppEvt_ServiceDiscoveryNotFound_c)
            {
                /* Moving to Service discovery Retry State*/
                mPeerInformation[peerDeviceId].appState = mAppServiceDiscRetry_c;
                /* Restart Service Discovery for all services */
                (void)BleServDisc_Start(peerDeviceId);
//                LOG_L_S(BLE_MD,"BleServDisc_Start!!! \r\n");
            }
            else if (event == mAppEvt_ServiceDiscoveryFailed_c)
            {
                (void)Gap_Disconnect(peerDeviceId);
//                LOG_L_S(BLE_MD,"ServiceDiscoveryFailed,  Gap_Disconnect!!! \r\n");
            }
            else
            {
                /* ignore other event types */
            }
        }
        break;

        case mAppServiceDiscRetry_c:
        {
            if (event == mAppEvt_ServiceDiscoveryComplete_c)
            {
                /* Moving to Running State*/
                mPeerInformation[peerDeviceId].appState = mAppRunning_c;
            }
            else if ((event == mAppEvt_ServiceDiscoveryNotFound_c) ||
                     (event == mAppEvt_ServiceDiscoveryFailed_c))
            {
                (void)Gap_Disconnect(peerDeviceId);
                LOG_L_S(BLE_MD,"Gap_Disconnect!!! \r\n");
            }
            else
            {
                /* ignore other event types */
            }
        }
        break;

        case mAppRunning_c:
            if (event == mAppEvt_PairingComplete_c)
            {
                // /*建立L2CAP连接*/
                // bleResult = L2ca_ConnectLePsm (gCcc_L2capLePsm_c,
                //                mPeerInformation[peerDeviceId].deviceId,
                //                mAppLeCbInitialCredits_c);
                // LOG_L_S(BLE_MD,"L2CAP CONNECT!!! bleResult:0x%02x \r\n",bleResult);   
            }
            
            break;

        default:
        {
            ; /* No action required */
        }
        break;
    }
}

/*! *********************************************************************************
* \brief        Handles GATT server callback from host stack.
*
* \param[in]    deviceId        Peer device ID.
* \param[in]    pServerEvent    Pointer to gattServerEvent_t.
********************************************************************************** */
static void BleApp_GattServerCallback (deviceId_t deviceId, gattServerEvent_t* pServerEvent)
{
    uint8_t nvmIndex = gInvalidNvmIndex_c;
    LOG_L_S(BLE_MD, "GattServer CB: DID=[%d], EVT=[0x%x]\n", deviceId, pServerEvent->eventType);
    switch (pServerEvent->eventType)
    {
        case gEvtMtuChanged_c:
        {
            mPeerInformation[deviceId].att_mtu = pServerEvent->eventData.mtuChangedEvent.newMtu;
		    LOG_L_S(BLE_MD, "MTU %d\r\n", mPeerInformation[deviceId].att_mtu);
        }
        break;

        case gEvtHandleValueConfirmation_c:
        {
            //NONE
        }
        break;        

        case gEvtAttributeWritten_c:
        {
            bleResult_t bleResult;
            uint16_t handle = pServerEvent->eventData.attributeWrittenEvent.handle;
            uint16_t length = pServerEvent->eventData.attributeWrittenEvent.cValueLength;
            uint8_t *pValue = pServerEvent->eventData.attributeWrittenEvent.aValue;

            if (VALUE_YQDK_IS_WRITEABLE(handle)) 
            {
                bleResult = GattServer_SendAttributeWrittenStatus (deviceId,
                                                                handle,
                                                                gAttErrCodeNoError_c);
                if (gBleSuccess_c == bleResult) 
                {
                    GattDb_WriteAttribute(handle, length, pValue);
                    LOG_L_S_HEX(BLE_MD, "DK WriteAttribute", pValue, length);
                    // ble_data_process(BLE_CHANNEL_1,pValue,length);

                    // Ble_SendData(deviceId,pValue,length);
                } 
                else 
                {
                    /*! A BLE error has occurred - Disconnect */
                	Gap_Disconnect (deviceId);
                }
            } 
            else 
            {
                /*! A GATT Server is trying to GATT Write an unknown attribute value.
                *  This should not happen. Disconnect the link. */
            	Gap_Disconnect(deviceId);
                LOG_L_S(BLE_MD, "Attribute Can't Write\n");
            }
        }
        break;

        case gEvtCharacteristicCccdWritten_c:
        {
            LOG_L_S(BLE_MD, "CccdWritten: handle=[%d], Cccd=[%d]\n", 
                                    pServerEvent->eventData.charCccdWrittenEvent.handle,
                                    pServerEvent->eventData.charCccdWrittenEvent.newCccd);
        }
        break;

        case gEvtAttributeWrittenWithoutResponse_c:
        {
        	//Gap_Disconnect(deviceId);
            LOG_L_S(BLE_MD, "attributeWrittenEvent: handle=[%d]\n",pServerEvent->eventData.attributeWrittenEvent.handle);
            LOG_L_S_HEX(BLE_MD,"attributeWrittenEvent Data:",pServerEvent->eventData.attributeWrittenEvent.aValue,pServerEvent->eventData.attributeWrittenEvent.cValueLength);
        }
        break;

        case gEvtError_c:
        {
            
            attErrorCode_t attError = (attErrorCode_t) (pServerEvent->eventData.procedureError.error & 0xFF);
            if (attError == gAttErrCodeInsufficientEncryption_c     ||
                attError == gAttErrCodeInsufficientAuthorization_c  ||
                attError == gAttErrCodeInsufficientAuthentication_c)
            {
#if gAppUsePairing_d
#if gAppUseBonding_d
                bool_t isBonded = FALSE;

                /* Check if the devices are bonded and if this is true than the bond may have
                 * been lost on the peer device or the security properties may not be sufficient.
                 * In this case try to restart pairing and bonding. */
                if (gBleSuccess_c == Gap_CheckIfBonded(deviceId, &isBonded,&nvmIndex) &&
                    TRUE == isBonded)
#endif /* gAppUseBonding_d */
                {
                    (void)Gap_SendSlaveSecurityRequest(deviceId, &gPairingParameters);
                }
#endif /* gAppUsePairing_d */                
            }

            LOG_L_S(BLE_MD, "EvtError: %d\n", pServerEvent->eventData.procedureError.error);
        }
        break;

        case gEvtAttributeRead_c:
        {
            LOG_L_S(BLE_MD,"AttributeRead Handle: %d \r\n",pServerEvent->eventData.attributeReadEvent.handle);
            //GattServer_SendAttributeReadStatus();
        }
        break;

        default:
        break;
    }
}

/*! *********************************************************************************
* \brief        Reads the battery level at mBatteryLevelReportInterval_c time interval.
*
********************************************************************************** */
//static void BatteryMeasurementTimerCallback(void * pParam)
//{
//    basServiceConfig.batteryLevel = BOARD_GetBatteryLevel();
//    (void)Bas_RecordBatteryMeasurement(&basServiceConfig);
//}


bleResult_t BleApp_NotifyDKData(deviceId_t peer_device_id, uint8_t* aValue,uint8_t aValueLength)
{
    // if (mBleConnectStatus == 0)
    // {
    //     return gBleInvalidState_c;
    // }
    
    //LOG_L_S_HEX(BLE_MD, "WriteAttribute Data:", aValue, aValueLength);
	LOG_L_S(BLE_MD, "WriteAttribute Data Device Id:%d \r\n", peer_device_id);
    return BleAppSendData(peer_device_id,service_yqdk,0xFFE2,aValueLength,aValue);
}

static void BleApp_DisconnectCallBack(void *pParam)
{
    uintn8_t index;
    uint8_t recordIndex;
    index = *(uintn8_t *)pParam;
    recordIndex = ble_ccc_ctx_get_record_from_deviceId(index);
#if EM_000101_CONFIG_FEATURE_RAM_MANAGEMENT
    core_platform_ram_reset();
#endif
    /*断开连接让CAN网络进入睡眠*/
    mBleConnectStatus = 0;
    gPairCompleteFlag = 0U;
    if (recordIndex == 0xFF)
    {
        return;
    }
    ble_ccc_ctx[recordIndex].oldIndex = 0xFF;
    LOG_L_S(BLE_MD, "ble_ccc_ctx[recordIndex].oldIndex:%d %d %d\r\n", recordIndex,ble_ccc_ctx[recordIndex].oldIndex);
}
void BleApp_connectTimeroutCallback(void *pParam)
{
    LOG_L_S(BLE_MD,"BLE Connect Pairing Timeout !!!!!! \r\n\r\n\r\n\r\n\r\n");
    Gap_Disconnect (((uint8_t*)pParam)[0]);

    ResetMCU();
}
static void BleApp_PairingSuccessCallback(void *pParam)
{
    uintn8_t index;
    uint8_t recordIndex;
	//u8 status;
    if (pParam == NULL)
    {
        return ;
    }

    index = *(uintn8_t *)pParam;
    recordIndex = ble_ccc_ctx_get_record_from_deviceId(index);
    if (recordIndex == 0xFF)
    {
        LOG_L_S(BLE_MD,"WARNING!!!recordIndex == 0xFF!!!\r\n");
        return;
    }
    if (ble_ccc_ctx[recordIndex].oldIndex == index) 
    {
        LOG_L_S(BLE_MD,"WARNING!!!ble_ccc_ctx[recordIndex].oldIndex:%d,%d!!!\r\n",recordIndex,ble_ccc_ctx[recordIndex].oldIndex);
        return;
    }
    else
    {
        ble_ccc_ctx[recordIndex].oldIndex = index;
    }
    (void)TMR_StopTimer(connectTimerId);
    LOG_L_S(BLE_MD,"BleApp_PairingSuccessCallback !!!!!! \r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
    //if (gPairCompleteFlag == 0U)
    {
        gPairCompleteFlag = 1U;
        ble_ccc_send_evt(CCC_EVT_BLE_PAIR_COMPLETE,index,NULL,0U);
    }
    // if (ble_work_ctx.currentWorkMode == WORK_MODE_SCAN)
    if (mPeerInformation[index].deviceId == ble_work_ctx.ble_work_state[WORK_MODE_SCAN].deviceId)
    {
        TMR_StartLowPowerTimer(appTimerId, gTmrLowPowerIntervalMillisTimer_c,
			200, BleApp_GetRssiCallback, &mPeerInformation[index].deviceId);
    }
    

    if (gInvalidDeviceId_c != mPeerInformation[index].deviceId)
    {
		/* Start rssi measurements */
		Gatt_GetMtu(mPeerInformation[index].deviceId, &mPeerInformation[index].att_mtu);
		LOG_L_S(BLE_MD, "MTU %d\r\n", mPeerInformation[index].att_mtu);
    }
    // /*连接后唤醒CAN网络*/
    mBleConnectStatus = 1;

    BleApp_StateMachineHandler(mPeerInformation[index].deviceId,mAppEvt_PairingComplete_c);

    // if (TMR_IsTimerActive(switchTimerId))
    {
        (void)TMR_StopTimer(switchTimerId);
    }
    /* Start advertising timer */
    (void)TMR_StartLowPowerTimer(switchTimerId,
                            gTmrLowPowerIntervalMillisTimer_c,
                            gSwitchTime_c,
                            SwitchTimerCallback, NULL);
}

