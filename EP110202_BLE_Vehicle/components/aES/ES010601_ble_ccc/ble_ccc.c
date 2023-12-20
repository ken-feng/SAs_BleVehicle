#include "ble_ccc.h"
#include "flash_api_extern.h"
#include "ble_general.h"
#include "EM000101.h"
#include "se_apdu_api.h"
//#include "ccc_dk_api.h"
#include "ccc_dk_api.h"
#include "ccc_dk.h"
#include "ccc_can.h"
#include "BCanPduCfg.h"
#include "ble_interface_api.h"
#include "uwb_SDK_Interface.h"
#include "flash_api_extern.h"
#include "hw_flash.h"
#include "../../../source/Project/CANNET/Com/UDS/UDS_App.h"

osaMutexId_t gBleCCCMutexId;

ble_ccc_ctx_t ble_ccc_ctx[BLE_DEIVCE_MAX_NUMBER];
ble_ccc_msg_t ble_ccc_msg;
ble_ccc_evt_t ble_ccc_evt;

u8 gQueueAllocFlag = 0;
QueueHandle_t  ccc_queue;

extern volatile u8 intIRQFlag;
extern u8 fobAddress[6];
extern u8 mBleConnectStatus;

//Modify (Ken):VEHICLE-V0C02 NO.1 -20231218
#if defined __FIT_Aeon_H
extern uint16_t	g_KeylessScopeDist;
#endif


#define BLE_CCC_SLOT_MAX_NUMBER                     15/*白名单最大存储SLOT记录条数为15条*/

#define BLE_MACADDRESS_SIZE                         6

#define BLE_CCC_WHITELIST_RECORD_SLOTID_SIZE        9 /*LV: length+value*/
#define BLE_CCC_WHITELIST_RECORD_LTK_SIZE           16
#define BLE_CCC_WHITELIST_RECORD_MACADDRESS_SIZE    BLE_MACADDRESS_SIZE 
#define BLE_CCC_WHITELIST_RECORD_SIZE               (BLE_CCC_WHITELIST_RECORD_SLOTID_SIZE+BLE_CCC_WHITELIST_RECORD_LTK_SIZE+BLE_CCC_WHITELIST_RECORD_MACADDRESS_SIZE)

#define BLE_CCC_WHITELIST_RECORD_OFFSET_SLOTID      0   
#define BLE_CCC_WHITELIST_RECORD_OFFSET_LTK         (BLE_CCC_WHITELIST_RECORD_OFFSET_SLOTID+BLE_CCC_WHITELIST_RECORD_SLOTID_SIZE)
#define BLE_CCC_WHITELIST_RECORD_OFFSET_MACADDRESS  (BLE_CCC_WHITELIST_RECORD_OFFSET_SLOTID+BLE_CCC_WHITELIST_RECORD_LTK_SIZE)

// u8 gCccGlobleBuffer[2048];


osaTaskId_t gBleCccTaskId = 0;
void BleCccProcess_Task(void* argument);
OSA_TASK_DEFINE(BleCccProcess_Task, 7, 1, 3000, FALSE );

u8 obdUWBSessionCtrl;	/*收到SESSION控制诊断报文*/

void ble_ccc_queue_init(void)
{
   ccc_queue = xQueueCreate(CCC_QUEUE_MAX_NUMBER,sizeof(ble_ccc_queue_msg_t));
   gQueueAllocFlag = 1;
}
boolean ble_ccc_queue_pop(u8* buffer)
{
   BaseType_t result;
   portBASE_TYPE taskToWake = portMAX_DELAY;
   if (__get_IPSR() != 0U)
   {
       result = xQueueReceiveFromISR(ccc_queue, buffer, &taskToWake);
       assert(pdPASS == result);
       portYIELD_FROM_ISR(taskToWake);
   }
   else
   {
       result = xQueueReceive(ccc_queue,buffer, &taskToWake);
   }
   return (result == pdPASS)? TRUE : FALSE;
}

boolean ble_ccc_queue_push(u8* buffer)
{
   BaseType_t result;
   portBASE_TYPE taskToWake = pdFALSE;
   if (__get_IPSR() != 0U)
   {
       result = xQueueSendFromISR(ccc_queue, buffer, &taskToWake);
       assert(pdPASS == result);
       portYIELD_FROM_ISR(taskToWake);
   }
   else
   {
       result = xQueueSend(ccc_queue, buffer, &taskToWake);
   }
   return (result == pdPASS)? TRUE : FALSE;
}

void sqrt_test(void)
{
    u32 v1 = 0;
    u32 v2 = 0;
    u32 v3 = 0;
	TEST_PIN_HIGH();
    v1 = ccc_loc_func_calc_sqrt((u32)0x12345678);
	TEST_PIN_LOW();

	TEST_PIN_HIGH();
	v2 = calc_sqrt_lib(0x12345678);
	TEST_PIN_LOW();

	v3 = (u32)sqrt((u32)0x12345678);
}
void ble_ccc_uwb_int(void)
{
    intIRQFlag = 0x02;
    __disable_irq();
    stUWBSDK.fpUQDeviceInit(stUWBSDK.fpUQSendMSG);
    stSource.stUCIState.stTimerTools.fpOSDelay = OSA_TimeDelay ;//
    __enable_irq();
    stUWBSDK.fpUQDeviceReset(stUWBSDK.fpUQSendMSG);
}
/**初始化***/
void ble_ccc_init(void)
{
//    location_test();
    u8 requestData[64] = {0x00};
    for(u8 i = 0;i<BLE_DEIVCE_MAX_NUMBER;i++)
    {
    	core_mm_set((u8*)&ble_ccc_ctx[i],0x00,sizeof(ble_ccc_ctx_t));
        ble_ccc_ctx[i].oldIndex = 0xFF;
    }
    core_mm_set((u8*)&ble_ccc_msg,0x00,sizeof(ble_ccc_msg_t));
    core_mm_set((u8*)&ble_ccc_evt,0x00,sizeof(ble_ccc_evt_t));
    
    ble_ccc_evt.pairingMode = 1;/*默认非配对模式*/
//    core_mm_set(gCccGlobleBuffer,0x00,2048);
    //se_sdk_init();
    ccc_sdk_init();

    /*上电主动调用一次同步slotId 钥匙槽信息*/
    ble_ccc_evt_notify_sdk(SDK_EVT_TAG_SYCN_SLOTID,0xFF,NULL_PTR,0);

    ble_ccc_queue_init();
    ble_ccc_task_init();

#ifndef FIT_DEBUG_NO_UWB    
    requestData[0]= 0x80U;
	UQ_UWB_SDK_Interface_init(&stUWBSDK);
	
    stUWBSDK.fpUQDeviceInit(stUWBSDK.fpUQSendMSG);
    stSource.stUCIState.stTimerTools.fpOSDelay = OSA_TimeDelay ;//
    stUWBSDK.fpUQDeviceReset(stUWBSDK.fpUQSendMSG);
//    OSA_TimeDelay(5000);//此处等待5秒，再发获取能力级uwb命令时，spi线上显示没发出来。 add by JohnSong
    stUWBSDK.fpUQAnchorWakup(requestData,64U, stUWBSDK.fpUQSendMSG);
#endif
//	 requestData[0] = 0x00;
//	 requestData[1] = 0x00;
//	 requestData[2] = 0x00;
//	 requestData[3] = 0x03;
//	 stUWBSDK.fpUQRangingSessionSetup(requestData,64U);
//	 stUWBSDK.fpUQRangingCtrl(UWBRangingOPType_Start, requestData,64U,stUWBSDK.fpUQSendMSG);
//	 KW38_INT_Start();
    BleApp_Work();
}

/*任务初始化*/
void ble_ccc_task_init(void)
{
    gBleCCCMutexId = OSA_MutexCreate();

	gBleCccTaskId = OSA_TaskCreate(OSA_TASK(BleCccProcess_Task), NULL);
    if( NULL == gBleCccTaskId )
    {
         panic(0,0,0,0);
         return;
    }
}

/*获取未连接的设备记录号*/
u8 ble_ccc_ctx_get_valid_record(void)
{
    for (u8 i = 0; i < BLE_DEIVCE_MAX_NUMBER; i++)
    {
        if (ble_ccc_ctx[i].validFlag == 0U)
        {
            return i;
        }
    }
    return 0xFF;
}
/*通过设备号查找记录号*/
u8 ble_ccc_ctx_get_record_from_deviceId(u8 deviceId)
{
    for (u8 i = 0; i < BLE_DEIVCE_MAX_NUMBER; i++)
    {
        if ((ble_ccc_ctx[i].deviceId == deviceId)&&ble_ccc_ctx[i].validFlag)
        {
            return i;
        }
    }
    return 0xFF;
}
/*通过设备号查找通道号*/
u16 ble_ccc_ctx_get_channelId_from_deviceId(u8 deviceId)
{
    for (u8 i = 0; i < BLE_DEIVCE_MAX_NUMBER; i++)
    {
        if ((ble_ccc_ctx[i].deviceId == deviceId)&&ble_ccc_ctx[i].validFlag)
        {
            return ble_ccc_ctx[i].channelId;
        }
    }
    return 0xFFFF;
}
/*查找空闲的记录号*/
ChannleID_e ble_ccc_ctx_get_idle_logic_channel(void)
{
    ChannleID_e i;
    u8 j;
    for (i = CHANNEL_ID_BLE_0; i < CHANNEL_ID_BLE_FOB; i++)
    {
        for (u8 j = 0; j < BLE_DEIVCE_MAX_NUMBER; j++)
        {
            if ((ble_ccc_ctx[j].cccLogicChannelId == i)&&ble_ccc_ctx[j].validFlag)
            {
                break;
            }
        }
        if (j == BLE_DEIVCE_MAX_NUMBER)
        {
            return i;
        }
        
    }
    return 0xFF;
}


/*通过逻辑通道号查找设备号*/
u8 ble_ccc_ctx_get_deviceId_from_logic_channelId(ChannleID_e chId)
{
    for (u8 i = 0; i < BLE_DEIVCE_MAX_NUMBER; i++)
    {
        if ((ble_ccc_ctx[i].cccLogicChannelId == chId)&&ble_ccc_ctx[i].validFlag)
        {
            return ble_ccc_ctx[i].deviceId;
        }
    }
    return 0xFF;
}
/*****BLE通讯操作******/
/**
 * @brief
 *      收取L2CAP 逻辑通道上数据
 * @param [deviceId]    当前蓝牙连接通道号
 * @param [inData]      收到的数据缓存(DK Message)
 * @param [length]      收到的数据长度
 * @return
 *        1表示成功，其它失败
 * @note
 */
u8 ble_ccc_l2cap_recv_data(u8 deviceId, u8* inData, u16 length)
{
    // gCccGlobleBuffer[BLE_MSG_SEND_REQDATA_OFFSET_TAG] = 0x00;
    // core_dcm_writeBig16(gCccGlobleBuffer+BLE_MSG_SEND_REQDATA_OFFSET_LENGTH,length+1);
    // gCccGlobleBuffer[BLE_MSG_SEND_REQDATA_OFFSET_V_CHANNELID] = deviceId;
    // core_mm_copy(gCccGlobleBuffer+BLE_MSG_SEND_REQDATA_OFFSET_V_DK_MSG,inData,length);
    // RTE_SDK_BLEDKmsg_recv(OBJ_ID_BLE,OBJ_ID_SDK,ble_ccc_msg.sendSerialNum,gCccGlobleBuffer,length+4);
    // ble_ccc_msg.sendSerialNum++;
    return 1;
}

/**
 * @brief
 *      往L2CAP 逻辑通道上发送数据
 * @param [outData]     待发送的数据缓存(DK Message)
 * @param [length]      待发送的数据长度
 * @return
 *        1表示成功，其它失败
 * @note
 */
u8 ble_ccc_l2cap_send_data(u8 deviceId,u8* outData, u16 length)
{
    bleResult_t     bleResult;
    u8 recordIndex;
    recordIndex = ble_ccc_ctx_get_record_from_deviceId(deviceId);
    if (ble_ccc_ctx[recordIndex].connectStatus == BLE_L2CAP_STATUS_DISCONNECT)
    {
        return 0U;
    }
    LOG_L_S_HEX(BLE_MD,"L2CAP Send Data:",outData,length);
    bleResult =  RTE_L2ca_SendLeCbData (deviceId,ble_ccc_ctx[recordIndex].channelId,outData,length);
    LOG_L_S(BLE_MD,"Send Result:0x%02x \r\n",bleResult);
    if (bleResult == gBleSuccess_c)
    {
        return 1U;
    }
    else
    {
        return 0U;
    }
#if 0
    bleResult_t     bleResult = gBleSuccess_c;

    if (otapServerData.transferMethod == gOtapTransferMethodAtt_c)
    {
        /* GATT Characteristic to be written without response - OTAP Client Data */
        gattCharacteristic_t    otapDataChar;

        /* Only the value handle element of this structure is relevant for this operation. */
        otapDataChar.value.handle = mPeerInformation.customInfo.otapServerConfig.hData;
        otapDataChar.value.valueLength = 0;
        otapDataChar.cNumDescriptors = 0;

        bleResult = GattClient_CharacteristicWriteWithoutResponse (mPeerInformation.deviceId,
                                                                   &otapDataChar,
                                                                   chunkCmdLength,
                                                                   pChunk);
    }
    else if (otapServerData.transferMethod == gOtapTransferMethodL2capCoC_c)
    {
        bleResult =  L2ca_SendLeCbData (mPeerInformation.deviceId,
                                        otapServerData.l2capPsmChannelId,
                                        pChunk,
                                        chunkCmdLength);
    }
    else
    {
        ; /* For MISRA compliance */
    }

    if (gBleSuccess_c != bleResult)
    {
        /*! A BLE error has occurred - Disconnect */
        (void)Gap_Disconnect (otapClientDevId);
    }
#endif    
}



/***CCC业务处理流程***/

/**
 * @brief
 *      BLE 数据发送调用接口,用于发送和外部 device 端进行交互的蓝牙报文。
 * @param [srcId]       FXXX # 发起对象
 * @param [destId]      FXXX # 目的对象
 * @param [serialNum]   [0000-FFFE] # 流水号,每次报文发送增 1 操作,到达 FFFE 下一包从 0000 开始
 * @param [reqData]     请求数据指针
 * @param [reqLength]   请求数据长度
 * @return
 *        1表示成功，其它失败
 * @note
 *      场景: 
 *      CCC 定义由 Vehicle 发送 device 的全部报文：
 *      Framework|SE |UWB Ranging Service|DK Event Notification 
 *      |Vehicle OEM App message|Supplementary Service message
 * 
 *      reqdata 数据结构定义： 
 *       名称       说明    长度 
 *        Tag       00      0x01 
 *        Len       var     0x02 
 *        Value Channel_id(0x01-x03) + dk_message(CCC3.0 定义全部 DK Message Format) len 长度
 */
u8 ble_ccc_msg_send(OBJ_ID_T srcId, OBJ_ID_T destId ,u32 serialNum, u8* reqData, u16 reqLength)
{
    u16 sendLength;
    if(ble_ccc_msg.recvSerialNum == serialNum)
    {/*重复报文,不发送*/
        LOG_L_S(BLE_MD,"Send Failed!!! Serial Number Is Last: 0x%04x !!!\r\n",serialNum);
        return 0;
    }
    ble_ccc_msg.recvSerialNum = serialNum;
    ble_ccc_msg.msgChannelId = core_dcm_readBig16(reqData+BLE_MSG_SEND_REQDATA_OFFSET_LENGTH);
    sendLength = core_dcm_readU8(reqData+BLE_MSG_SEND_REQDATA_OFFSET_V_CHANNELID);
    sendLength -= 1;/*减去ChannelID占用的一个字节*/
    return ble_ccc_l2cap_send_data(0,reqData+BLE_MSG_SEND_REQDATA_OFFSET_V_DK_MSG,sendLength);
}
/**
 * @brief
 *      SDK 通知触发事件通知，用于 SDK 和 BLE 模块组件之前的信息交互。
 * @param [srcId]       FXXX # 发起对象
 * @param [destId]      FXXX # 目的对象
 * @param [serialNum]   [0000-FFFE] # 流水号,每次报文发送增 1 操作,到达 FFFE 下一包从 0000 开始
 * @param [reqData]     请求数据指针
 * @param [reqLength]   请求数据长度
 * @return
 *        1表示成功，其它失败
 * @note
 *      场景: 
 *      配对模式开启通知|钥匙认证 OOB 请求|钥匙信息管理同步。
 * 
 *      reqdata 数据结构定义： 
 *      名称 说明                                   长度 
 *      Tag                                         0x01
 *           00：配对模式开关 
 *           01：OOB 配对 
 *           02：当前认证 slotid 对象属性 
 *           03：delete 删除钥匙数据 
 *           04: clear 钥匙数据 
 *           05：断开当前 channel_id 连接 
 *           06：同步当前车端的全部激活钥匙槽信息 
 *           07: 同步当前 channel_id 使用状态        
 *      Len  var                                    0x02
 *      Value                                       len 长度
 *           00: channel_id(0xFF) + flag;       flag: 00 配对模式，01:非配对模式 
 *           01: channel_id(0x01-0x03) + flag;  flag:00 获取 OOB 数据 01：请求 LTK 生成
 *           02: channel_id(0x01-0x03) + 属性值(钥匙槽)，属性值:type(0x01) + slotid 值 
 *                                      type:00 车主,01:分享人 02:FOB KEY 
 *           03：channel_id(0xFF) + num(0x01)+LV(slotid0)……LV(slotidn) 
 *           04：channel_id(0xFF) 
 *           05：channel_id(0x01-0x03) 
 *           06：channel_id(0xFF) + num(0x01)+LV(slotid0)……LV(slotidn) 
 *           07：channel_id(0xFF) + channel_ids //返回使用的 channel_id 全部数值
 */
u8 ble_ccc_event(OBJ_ID_T srcId, OBJ_ID_T destId, u32 serialNum, u8*reqData, u16 reqLength)
{
    u8 tag;
    u8 flag;
    LOG_L_S_HEX(BLE_MD,"CCC Event Data:",reqData,reqLength);
    if(ble_ccc_msg.recvSerialNum == serialNum)
    {/*重复报文,不发送*/
        LOG_L_S(BLE_MD,"EVT Notify Failed!!! Serial Number Is Last: 0x%04x !!!\r\n",serialNum);
        return 0;
    }
    ble_ccc_msg.recvSerialNum = serialNum;
    tag = core_dcm_readU8(reqData+BLE_MSG_SEND_REQDATA_OFFSET_TAG);
    switch (tag)
    {
    case BLE_EVT_TAG_PAIRING_SETUP://00：配对模式开关
        ble_ccc_evt.pairingMode = core_dcm_readU8(reqData+BLE_MSG_SEND_REQDATA_OFFSET_V_DK_MSG);
        /*todo:*/
        break;
    case BLE_EVT_TAG_OOB_PAIRING://01：OOB 配对 
        flag = core_dcm_readU8(reqData+BLE_MSG_SEND_REQDATA_OFFSET_V_DK_MSG);
        if (flag == 0x00)/*获取OOB数据*/
        {
            /*todo:*/
        }
        else if (flag == 0x01)/*请求 LTK 生成*/
        {
            /*todo:*/
        }
        break;
    case BLE_EVT_TAG_SLOTID://02：当前认证 slotid 对象属性
        ble_ccc_evt.slotIdType = core_dcm_readU8(reqData+BLE_MSG_SEND_REQDATA_OFFSET_V_DK_MSG);
        core_mm_copy(ble_ccc_evt.slotId,reqData+BLE_MSG_SEND_REQDATA_OFFSET_V_DK_MSG+1,
                    core_dcm_readU8(reqData+BLE_MSG_SEND_REQDATA_OFFSET_V_DK_MSG+1)+1);
        break;
    case BLE_EVT_TAG_DELETE_KEY://03：delete 删除钥匙数据 
        ble_ccc_delete_whitelist_from_slotId(reqData+BLE_MSG_SEND_REQDATA_OFFSET_V_DK_MSG+1,
                                core_dcm_readU8(reqData+BLE_MSG_SEND_REQDATA_OFFSET_V_DK_MSG));
        break;
    case BLE_EVT_TAG_CLEAR_KEY://04: clear 钥匙数据 
        ble_ccc_delete_all_whitelist();
        break;    
    case BLE_EVT_TAG_DISCONNECT://05：断开当前 channel_id 连接
        /*todo:*/
        break;
    case BLE_EVT_TAG_SYCN_KEYINFO://06：同步当前车端的全部激活钥匙槽信息
        ble_ccc_syc_whitelist(reqData+BLE_MSG_SEND_REQDATA_OFFSET_V_DK_MSG+1,
                                core_dcm_readU8(reqData+BLE_MSG_SEND_REQDATA_OFFSET_V_DK_MSG));
        break;
    case BLE_EVT_TAG_SYCN_CHANNEL://07: 同步当前 channel_id 使用状态   
        /*todo:*/
        break;
    default:
        break;
    }



}

/**
 * @brief
 *      返回从 origin_id 获取数据结果反馈。
 * @param [srcId]       FXXX # 发起对象
 * @param [destId]      FXXX # 目的对象
 * @param [rspdata]     请求数据指针
 * @param [rsplength]   请求数据长度
 * @return
 *        1表示成功，其它失败
 * @note
 *      场景: 
 *      用于对告知 origin_id 来源数据的接收状态。
 */
u8 ble_ccc_ack(OBJ_ID_T srcId, OBJ_ID_T destId, u8*rspdata, u16 rsplength)
{

}

/**
 * @brief
 *      通知SDK模块事件数据
 * @param [sdk_evt_t]    SDK事件
 * @param [channelId]    当前蓝牙连接通道号
 * @param [inData]       发送的数据缓存
 * @param [length]       发送的数据长度
 * @return
 *        1表示成功，其它失败
 * @note
 *          reqdata 数据结构定义： 
 *          名称 说明                           长度 
 *          Tag                                 0x01
 *              00 : 设备接入(蓝牙设备连接) 
 *              01 : 设备断开(蓝牙设备断链) 
 *              02 : LTK 建立成功|失败通知 
 *              03 : 同步当前车端激活钥匙槽列表 
 *              04 : 同步 channel_id 列表  
 *          Len var                             0x02 
 *          Value 
 *              00 : Channel_id(0x01-0x03) + Type + Flag Type-00:Mobile, 01:FOB Flag-00:绑定 LTK, 01:LTK 未绑定 
 *              02 : Channel_id(0x01-0x03)+ Flag Flag-00:建立成功，01：建立失败 
 *              03 : Channel_id(0xFF) 
 *              04 : Channel_id(0xFF)
 * 
 */
u8 ble_ccc_evt_notify_sdk(sdk_evt_t sdk_evt ,u8 channelId,u8* inData,u16 length)
{
    // gCccGlobleBuffer[BLE_MSG_SEND_REQDATA_OFFSET_TAG] = sdk_evt;
    // core_dcm_writeBig16(gCccGlobleBuffer+BLE_MSG_SEND_REQDATA_OFFSET_LENGTH,length+1);  
    // gCccGlobleBuffer[BLE_MSG_SEND_REQDATA_OFFSET_V_CHANNELID] = channelId;    
    // if (length != 0U)
    // {
    //     core_mm_copy(gCccGlobleBuffer+BLE_MSG_SEND_REQDATA_OFFSET_V_DK_MSG,inData,length);
    // }
    // RTE_SDK_BLEDKmsg_notify(OBJ_ID_BLE,OBJ_ID_SDK,ble_ccc_msg.sendSerialNum,gCccGlobleBuffer,length+4);
    // ble_ccc_msg.sendSerialNum++;
}








u8 ble_ccc_send_evt(ccc_evt_type_t evtType, u8 deviceId, u8* pdata, u16 length)
{
    u8* dataBuff;
    ble_ccc_queue_msg_t cccQueue;
    if(gQueueAllocFlag == 0U)
    {
    	return ;
    }
    cccQueue.evtType = evtType;
    if(pdata != NULL)
    {
        (void)OSA_MutexLock(gBleCCCMutexId, osaWaitForever_c);
		dataBuff = core_platform_alloc(length);
        (void)OSA_MutexUnlock(gBleCCCMutexId);
		if (dataBuff == NULL)
		{
			LOG_L_S(NFC_MD,"Alloc Ram Space Failed!!!\r\n");
			return ;
		}
		core_mm_copy(dataBuff,pdata,length);
		cccQueue.dataBuff = dataBuff;
    }
    else
    {
    	cccQueue.dataBuff = pdata;
    }
    cccQueue.deviceId = deviceId;
    cccQueue.length = length;

    if(FALSE == ble_ccc_queue_push((u8*)&cccQueue))
    {
        LOG_L_S(NFC_MD,"Send Evt Failed!!!\r\n");
    }
}

u8 ble_ccc_send_leSetPhyRequest(ChannleID_e chId)
{
	u8 deviceId = ble_ccc_ctx_get_deviceId_from_logic_channelId(chId);
    ble_ccc_send_evt(CCC_EVT_LESETPHY,deviceId,NULL,0);
//	ble_ccc_send_evt(CCC_EVT_LESETPHY,chId,NULL,0);
}


//********************************************************************************
//
//********************************************************************************
u8 ble_ccc_send_data(u8 deviceId, u8* inPut, u16 length)
{
    ble_ccc_send_evt(CCC_EVT_SEND_DATA,deviceId,inPut,length);
}
//********************************************************************************


//********************************************************************************
//
//********************************************************************************
void ble_ccc_can_process(cccCanId_t cccCanId)
{
    u8 tmpData[65U];
    switch (cccCanId)
    {
    // case CANID_SA_0x200:
    //     tmpData[0] = CAN_PKG_ID_UWB_ANCHOR_WAKEUP_RS;
    //     BCanPdu_Get_SA200_Data(tmpData+1);
    //     break;
    // case CANID_SA_0x201:
    //     tmpData[0] = CAN_PKG_ID_TIME_SYNC_RS;
    //     BCanPdu_Get_SA201_Data(tmpData+1);
    //     break;
    // case CANID_SA_0x202:
    //     tmpData[0] = CAN_PKG_ID_UWB_RANGING_SESSION_START_RS;
    //     BCanPdu_Get_SA201_Data(tmpData+1);
    //     break;
    // case CANID_SA_0x203:
    //     tmpData[0] = CAN_PKG_ID_UWB_RANGING_SESSION_SUSPEND_RECOVER_DELETE_RS;
    //     BCanPdu_Get_SA201_Data(tmpData+1);
    //     break;

    case CANID_SA_0x210:
        tmpData[0] = CAN_PKG_ID_UWB_ANCHOR_WAKEUP_RS;
        BCanPdu_Get_SA210_Data(tmpData+1);
        break;
    case CANID_SA_0x220:
        tmpData[0] = CAN_PKG_ID_UWB_ANCHOR_WAKEUP_RS;
        BCanPdu_Get_SA220_Data(tmpData+1);
        break;
    case CANID_SA_0x230:
        tmpData[0] = CAN_PKG_ID_UWB_ANCHOR_WAKEUP_RS;
        BCanPdu_Get_SA230_Data(tmpData+1);
        break;
    case CANID_SA_0x240:
        tmpData[0] = CAN_PKG_ID_UWB_ANCHOR_WAKEUP_RS;
        BCanPdu_Get_SA240_Data(tmpData+1);
        break;
    case CANID_SA_0x250:
        tmpData[0] = CAN_PKG_ID_UWB_ANCHOR_WAKEUP_RS;
        BCanPdu_Get_SA250_Data(tmpData+1);
        break;
    case CANID_SA_0x260:
        tmpData[0] = CAN_PKG_ID_UWB_ANCHOR_WAKEUP_RS;
        BCanPdu_Get_SA260_Data(tmpData+1);
        break;

    case CANID_SA_0x211:
        tmpData[0] = CAN_PKG_ID_UWB_RANGING_SESSION_START_RS;
        BCanPdu_Get_SA211_Data(tmpData+1);
        break;
    case CANID_SA_0x221:
        tmpData[0] = CAN_PKG_ID_UWB_RANGING_SESSION_START_RS;
        BCanPdu_Get_SA221_Data(tmpData+1);
        break;
    case CANID_SA_0x231:
        tmpData[0] = CAN_PKG_ID_UWB_RANGING_SESSION_START_RS;
        BCanPdu_Get_SA231_Data(tmpData+1);
        break;
    case CANID_SA_0x241:
        tmpData[0] = CAN_PKG_ID_UWB_RANGING_SESSION_START_RS;
        BCanPdu_Get_SA241_Data(tmpData+1);
        break;
    case CANID_SA_0x251:
        tmpData[0] = CAN_PKG_ID_UWB_RANGING_SESSION_START_RS;
        BCanPdu_Get_SA251_Data(tmpData+1);
        break;
    case CANID_SA_0x261:
        tmpData[0] = CAN_PKG_ID_UWB_RANGING_SESSION_START_RS;
        BCanPdu_Get_SA261_Data(tmpData+1);
        break;


    case CANID_SA_0x212:
        tmpData[0] = CAN_PKG_ID_UWB_RANGING_SESSION_SUSPEND_RECOVER_DELETE_RS;
        BCanPdu_Get_SA212_Data(tmpData+1);
        break;
    case CANID_SA_0x222:
        tmpData[0] = CAN_PKG_ID_UWB_RANGING_SESSION_SUSPEND_RECOVER_DELETE_RS;
        BCanPdu_Get_SA222_Data(tmpData+1);
        break;
    case CANID_SA_0x232:
        tmpData[0] = CAN_PKG_ID_UWB_RANGING_SESSION_SUSPEND_RECOVER_DELETE_RS;
        BCanPdu_Get_SA232_Data(tmpData+1);
        break;
    case CANID_SA_0x242:
        tmpData[0] = CAN_PKG_ID_UWB_RANGING_SESSION_SUSPEND_RECOVER_DELETE_RS;
        BCanPdu_Get_SA242_Data(tmpData+1);
        break;
    case CANID_SA_0x252:
        tmpData[0] = CAN_PKG_ID_UWB_RANGING_SESSION_SUSPEND_RECOVER_DELETE_RS;
        BCanPdu_Get_SA252_Data(tmpData+1);
        break;
    case CANID_SA_0x262:
        tmpData[0] = CAN_PKG_ID_UWB_RANGING_SESSION_SUSPEND_RECOVER_DELETE_RS;
        BCanPdu_Get_SA262_Data(tmpData+1);
        break;

    case CANID_SA_0x213:
        tmpData[0] = CAN_PKG_ID_TIME_SYNC_RS;
        BCanPdu_Get_SA213_Data(tmpData+1);
        break;
    case CANID_SA_0x223:
        tmpData[0] = CAN_PKG_ID_TIME_SYNC_RS;
        BCanPdu_Get_SA223_Data(tmpData+1);
        break;
    case CANID_SA_0x233:
        tmpData[0] = CAN_PKG_ID_TIME_SYNC_RS;
        BCanPdu_Get_SA233_Data(tmpData+1);
        break;
    case CANID_SA_0x243:
        tmpData[0] = CAN_PKG_ID_TIME_SYNC_RS;
        BCanPdu_Get_SA243_Data(tmpData+1);
        break;
    case CANID_SA_0x253:
        tmpData[0] = CAN_PKG_ID_TIME_SYNC_RS;
        BCanPdu_Get_SA253_Data(tmpData+1);
        break;
    case CANID_SA_0x263:
        tmpData[0] = CAN_PKG_ID_TIME_SYNC_RS;
        BCanPdu_Get_SA263_Data(tmpData+1);
        break;

    case CANID_SA_0x2A0:
        tmpData[0] = CAN_PKG_ID_UWB_RANGING_LOCATION_NOTICE;
        BCanPdu_Get_SA2A0_Data(tmpData+1);
        break;
    case CANID_SA_0x2A1:
        tmpData[0] = CAN_PKG_ID_CALIBRATION_DATA_RS;
        BCanPdu_Get_SA2A1_Data(tmpData+1);
        break;
#ifdef FIT_DEBUG_NO_SA 
    case CANID_UWB_0x313:
        tmpData[0] = CAN_PKG_ID_UWB_RANGING_RESULT_NOTICE;
        BCanPdu_Get_UWB313_Data(tmpData+1);
        break;
    case CANID_UWB_0x323:
        tmpData[0] = CAN_PKG_ID_UWB_RANGING_RESULT_NOTICE;
        BCanPdu_Get_UWB323_Data(tmpData+1);
        break;
    case CANID_UWB_0x333:
        tmpData[0] = CAN_PKG_ID_UWB_RANGING_RESULT_NOTICE;
        BCanPdu_Get_UWB333_Data(tmpData+1);
        break;
    case CANID_UWB_0x343:
        tmpData[0] = CAN_PKG_ID_UWB_RANGING_RESULT_NOTICE;
        BCanPdu_Get_UWB343_Data(tmpData+1);
        break;
    case CANID_UWB_0x353:
        tmpData[0] = CAN_PKG_ID_UWB_RANGING_RESULT_NOTICE;
        BCanPdu_Get_UWB353_Data(tmpData+1);
        break;
    case CANID_UWB_0x363:
        tmpData[0] = CAN_PKG_ID_UWB_RANGING_RESULT_NOTICE;
        BCanPdu_Get_UWB363_Data(tmpData+1);
        break;

#endif
    case CANID_ODB_0x58:
        
        break;
    case CANID_ODB_0x59:
    	BCanPdu_Get_ODB59_Data(tmpData);
        ble_ccc_send_evt(CCC_EVT_TEST_SET_FOB_ADDR,0U,tmpData,6U);
        break;

	//Modify (Ken):VEHICLE-V0C02 NO.1 -20231218
	#if defined __FIT_Aeon_H
    case CANID_ODB_0x60:
    {
    	BCanPdu_Get_ODB60_Data(tmpData);


    	g_KeylessScopeDist = ((uint16_t)tmpData[0]<<8 | tmpData[1]);

        //----------------------------------------------------------------------------
        // [ min ] - 1m
        //----------------------------------------------------------------------------
        if(g_KeylessScopeDist<100)
        {
    		tmpData[0] = 0x00; tmpData[1] = 0x64;
        	g_KeylessScopeDist = 100;
        }
        //----------------------------------------------------------------------------
        // [ max ] - 10m
        //----------------------------------------------------------------------------
        else if(g_KeylessScopeDist > 1000)
        {
    		tmpData[0] = 0x03; tmpData[1] = 0xE8;
        	g_KeylessScopeDist = 1000;
        }
    	//------------------------------------------------------------------------
    	// update successful to setup
    	//------------------------------------------------------------------------
    	if(NVM_SUCCESS==KW38_Write_eeprom(0, tmpData,2))
    	{
    		g_KeylessScopeDist = ((uint16_t)tmpData[0]<<8 | tmpData[1]);
        	BCanPdu_Set_OBD061_Data(tmpData,64U);
    	}
    	//------------------------------------------------------------------------
    	// update fail to 5m
    	//------------------------------------------------------------------------
    	else{
    		tmpData[0] = 0x01; tmpData[1] = 0xF4;
    		g_KeylessScopeDist = 500;
    	}
		BCanPdu_Set_OBD061_Data(tmpData,64U);
    } break;
	#endif

    default:
        break;
    }
    //LOG_L_S_HEX(CCC_MD,"CAN Receive Data:",tmpData,65U);
    ccc_api_data_request(CHANNEL_ID_CAN,tmpData,65U);
}
//********************************************************************************


//********************************************************************************
//
//********************************************************************************
void ble_ccc_uwb_process(u8* inBuf , u16 length)
{
    LOG_L_S_HEX(CCC_MD,"UWB Receive Data:",inBuf,length);
    ccc_api_data_request(CHANNEL_ID_SPI_UWB,inBuf,length);
}
//********************************************************************************


//********************************************************************************
//
//********************************************************************************
ble_ccc_queue_msg_t gRecvQueuePtr;
void ble_ccc_process(void)
{
    boolean result;
    u8 recordIndex;
    u8 deviceId;
    u8 tmpBuf[16];
    gapPhyEvent_t* tCccLePhy;

//    TP_PB2_Toggle();
    //============================================================================
    //
    //============================================================================
    if(gQueueAllocFlag == 0)
    {
    	return;
    }
    //============================================================================
    //
    //============================================================================
    result = ble_ccc_queue_pop((u8*)&gRecvQueuePtr);
    if (result == FALSE)
    {   /*从消息序列中未读出数据，则退出*/
        return ;
    }
    //============================================================================
	//
	//============================================================================
    if (gRecvQueuePtr.evtType == CCC_EVT_IDLE)
    {
        goto LableOut;
    }

    //============================================================================
	// [ ] - Connect
	//============================================================================
    if (gRecvQueuePtr.evtType == CCC_EVT_STATUS_CONNECT)
    {
        recordIndex = gRecvQueuePtr.dataBuff[0];
        ble_ccc_ctx[recordIndex].validFlag = 1U;   
        ble_ccc_ctx[recordIndex].deviceId = gRecvQueuePtr.deviceId;
        ble_ccc_ctx[recordIndex].deviceType = gRecvQueuePtr.dataBuff[0];
        ble_ccc_ctx[recordIndex].firstConnectFlag = gRecvQueuePtr.dataBuff[1];
        core_mm_copy(ble_ccc_ctx[recordIndex].macAddress,gRecvQueuePtr.dataBuff+2U, 0x06);

        if (ble_ccc_ctx[recordIndex].deviceType == BLE_DEVICE_PRIVATE)
        {
            ble_ccc_ctx[recordIndex].cccLogicChannelId = CHANNEL_ID_BLE_PRIVATE;
        }
        else if(ble_ccc_ctx[recordIndex].deviceType == BLE_DEVICE_FOB)
        {
            ble_ccc_ctx[recordIndex].cccLogicChannelId = CHANNEL_ID_BLE_FOB;
        }
        else
        {
            ble_ccc_ctx[recordIndex].cccLogicChannelId = ble_ccc_ctx_get_idle_logic_channel();
            if (ble_ccc_ctx[recordIndex].cccLogicChannelId == 0xFF)
            {
                LOG_L_S(CCC_MD,"Warnning !!! cccLogicChannelId Is Fully!!! \r\n");
                goto LableOut;
            }
        }
        LOG_L_S(CCC_MD,"CCC Logic ChannelId : %d \r\n",ble_ccc_ctx[recordIndex].cccLogicChannelId);
        /*通知CCC SDK*/
        if (ble_ccc_check_ltk_from_macaddress(ble_ccc_ctx[recordIndex].macAddress))
        {
            /*已配对过*/
            ccc_api_extern_event_notice(ble_ccc_ctx[recordIndex].cccLogicChannelId,EXT_EVENT_BLE_CONNECT,NULL,0U);
        }
        else
        {
            /*未配对过*/
            ccc_api_extern_event_notice(ble_ccc_ctx[recordIndex].cccLogicChannelId,EXT_EVENT_BLE_CONNECT_ON_FIRST_APPROACH,NULL,0U);
        }
    }
    //============================================================================
	//
	//============================================================================
    else if (gRecvQueuePtr.evtType == CCC_EVT_STATUS_DISCONNECT)
    {
        recordIndex = ble_ccc_ctx_get_record_from_deviceId(gRecvQueuePtr.deviceId);
        if (recordIndex == 0xFF)
        {
            LOG_L_S(CCC_MD,"BLE Connected DeviceId Not Found, Please Check!!!\r\n");
            goto LableOut;
        }
        ccc_api_extern_event_notice(ble_ccc_ctx[recordIndex].cccLogicChannelId,EXT_EVENT_BLE_DISCONNECT,NULL,0U);
        core_mm_set((u8*)&ble_ccc_ctx[recordIndex],0x00,sizeof(ble_ccc_ctx_t));  
    }
    //============================================================================
	//
	//============================================================================
    else if (gRecvQueuePtr.evtType == CCC_EVT_L2CAP_SETUP_COMPLETE)
    {
        recordIndex = ble_ccc_ctx_get_record_from_deviceId(gRecvQueuePtr.deviceId);
        if (recordIndex == 0xFF)
        {
            LOG_L_S(CCC_MD,"BLE Connected DeviceId Not Found, Please Check!!!\r\n");
            goto LableOut;
        }
        ble_ccc_ctx[recordIndex].channelId = core_dcm_readBig16(gRecvQueuePtr.dataBuff);
        ble_ccc_ctx[recordIndex].connectStatus = BLE_L2CAP_STATUS_CONNECT;
    }
    //============================================================================
	//
	//============================================================================
    else if (gRecvQueuePtr.evtType == CCC_EVT_L2CAP_DISCONNECT)
    {
        recordIndex = ble_ccc_ctx_get_record_from_deviceId(gRecvQueuePtr.deviceId);
        if (recordIndex == 0xFF)
        {
            LOG_L_S(CCC_MD,"BLE Connected DeviceId Not Found, Please Check!!!\r\n");
            goto LableOut;
        }
        ble_ccc_ctx[recordIndex].channelId = 0x0000;
        ble_ccc_ctx[recordIndex].connectStatus = BLE_L2CAP_STATUS_DISCONNECT;
        ccc_api_extern_event_notice(ble_ccc_ctx[recordIndex].cccLogicChannelId,EXT_EVENT_BLE_DISCONNECT,NULL,0U);
    }
    //============================================================================
	//
	//============================================================================
    else if (gRecvQueuePtr.evtType == CCC_EVT_BLE_PAIR_COMPLETE)
    {
        recordIndex = ble_ccc_ctx_get_record_from_deviceId(gRecvQueuePtr.deviceId);
        if (recordIndex == 0xFF)
        {
            LOG_L_S(CCC_MD,"BLE Connected DeviceId Not Found, Please Check!!!\r\n");
            goto LableOut;
        }
        ccc_api_extern_event_notice(ble_ccc_ctx[recordIndex].cccLogicChannelId,EXT_EVENT_BLE_PAIRING_AND_ENCRYPTION_SETUP_FINISH,NULL,0U);
    }
    //============================================================================
	//
	//============================================================================
    else if (gRecvQueuePtr.evtType == CCC_EVT_LESETPHY)
    {
        deviceId = ble_ccc_ctx_get_record_from_deviceId(gRecvQueuePtr.deviceId);
        if (deviceId == 0xFF)
        {
            LOG_L_S(CCC_MD,"BLE Connected DeviceId Not Found, Please Check!!!\r\n");
            goto LableOut;
        }
        deviceId = gRecvQueuePtr.deviceId;
        if (gRecvQueuePtr.dataBuff == NULL)
        {
            RTE_BLE_GAP_LE_READ_PHY(deviceId);
        }
        else
        {
            tCccLePhy = (gapPhyEvent_t* )gRecvQueuePtr.dataBuff;
            if (tCccLePhy->phyEventType == gPhyRead_c)
            {
                RTE_BLE_GAP_LE_SET_PHY(FALSE,tCccLePhy->deviceId,0x03,tCccLePhy->txPhy,tCccLePhy->rxPhy,gLeCodingNoPreference_c);
            }
            else if (tCccLePhy->phyEventType == gPhyUpdateComplete_c)
            {
                /*收到LeSetPhy指令*/    
            }
            else
            {

            }
        }
    }
    //============================================================================
	//
	//============================================================================
    else if (gRecvQueuePtr.evtType == CCC_EVT_RECV_DATA)
    {
        recordIndex = ble_ccc_ctx_get_record_from_deviceId(gRecvQueuePtr.deviceId);
        if (recordIndex == 0xFF)
        {
            LOG_L_S(CCC_MD,"BLE Connected DeviceId Not Found, Please Check!!!\r\n");
            goto LableOut;
        }
        // ble_ccc_l2cap_recv_data(gRecvQueuePtr.deviceId, gRecvQueuePtr.dataBuff, gRecvQueuePtr.length);
        ccc_api_data_request(ble_ccc_ctx[recordIndex].cccLogicChannelId,gRecvQueuePtr.dataBuff, gRecvQueuePtr.length);
        // ble_ccc_send_leSetPhyRequest();
    }
    //============================================================================
	//
	//============================================================================
    else if (gRecvQueuePtr.evtType == CCC_EVT_SEND_DATA)
    {
        deviceId = ble_ccc_ctx_get_deviceId_from_logic_channelId(gRecvQueuePtr.deviceId);
        if (deviceId == 0xFF)
        {
            LOG_L_S(CCC_MD,"BLE Connected DeviceId Not Found, Please Check!!!\r\n");
            goto LableOut;
        }
        ble_ccc_l2cap_send_data(deviceId, gRecvQueuePtr.dataBuff, gRecvQueuePtr.length);
    }
    //============================================================================
	//
	//============================================================================
    else if (gRecvQueuePtr.evtType == CCC_EVT_RECV_CAN_DATA)
    {
        ble_ccc_can_process(gRecvQueuePtr.deviceId);
    }
    //============================================================================
	//
	//============================================================================
    else if(gRecvQueuePtr.evtType == CCC_EVT_RECV_UWB_DATA)
    {
        ble_ccc_uwb_process(gRecvQueuePtr.dataBuff, gRecvQueuePtr.length);
    }
    //============================================================================
	//
	//============================================================================
    else if (gRecvQueuePtr.evtType == CCC_EVT_TEST_SET_FOB_ADDR)
    {
        core_mm_copy(fobAddress,gRecvQueuePtr.dataBuff, gRecvQueuePtr.length);

        tmpBuf[0] = 1u;
        core_algo_swap_u8(tmpBuf+1,fobAddress,6U);

        hw_flash_write_for_ble_area(NVM_DATA_ID_NUM0_BLE_CONNECT_MAC, tmpBuf, 7U);

        Gap_ClearWhiteList();
        Gap_AddDeviceToWhiteList(gBleAddrTypePublic_c,tmpBuf+1);
        LOG_L_S_HEX(CCC_MD,"BLE Connected  FOB MAC:",fobAddress,6U);
        if(mBleConnectStatus)
        {
            Gap_Disconnect (gRecvQueuePtr.deviceId);
        }
        else
        {
            //(void)Gap_StopScanning();
        }
    }
    //============================================================================
	// Setup Distance Trigger
	//============================================================================
	//Modify (Ken):VEHICLE-V0C02 NO.1 -20231218
	#if defined __FIT_Aeon_H
    else if (gRecvQueuePtr.evtType == CCC_EVT_TEST_SET_DISTANCE)
    {
//        core_mm_copy(g_KeylessScopeDist,gRecvQueuePtr.dataBuff, gRecvQueuePtr.length);
    }
	#endif
    //============================================================================
	//
	//============================================================================
    else if(gRecvQueuePtr.evtType == CCC_EVT_SEND_PRIVATE_DATA)
    {
        deviceId = ble_ccc_ctx_get_deviceId_from_logic_channelId(gRecvQueuePtr.deviceId);
        if (deviceId == 0xFF)
        {
            LOG_L_S(CCC_MD,"BLE Connected DeviceId Not Found, Please Check!!!\r\n");
            goto LableOut;
        }
        Ble_SendData(deviceId,gRecvQueuePtr.dataBuff,gRecvQueuePtr.length);
    }
    //============================================================================
	//
	//============================================================================
    else if(gRecvQueuePtr.evtType == CCC_EVT_WRITE_WHITELIST)
    {
        ble_ccc_write_whitelist(gRecvQueuePtr.dataBuff,gRecvQueuePtr.dataBuff,gRecvQueuePtr.deviceId,gRecvQueuePtr.dataBuff);
    }
    //============================================================================
	//
	//============================================================================
    else if(gRecvQueuePtr.evtType == CCC_EVT_STATUS_MAC_UPDATE)
    {
        for (u8 i = 0; i < BLE_DEIVCE_MAX_NUMBER; i++)
        {
            if (ble_ccc_ctx[i].firstConnectFlag == 1U)
            {
                core_mm_copy(ble_ccc_ctx[i].macAddress,gRecvQueuePtr.dataBuff,6U);
                ble_ccc_ctx[i].firstConnectFlag = 0U;
                break;
            }
        }
    }
    //============================================================================
	//
	//============================================================================
    else if (gRecvQueuePtr.evtType == CCC_EVT_CMD_DISCONNECT)
    {
        deviceId = ble_ccc_ctx_get_deviceId_from_logic_channelId(gRecvQueuePtr.deviceId);
        if (deviceId == 0xFF)
        {
            LOG_L_S(CCC_MD,"BLE Connected DeviceId Not Found, Please Check!!!\r\n");
            goto LableOut;
        }
        Gap_Disconnect (deviceId);
    }
    //============================================================================
	//
	//============================================================================
    else if(gRecvQueuePtr.evtType == CCC_EVT_TIMER_HANDLER)
    {
        ccc_api_extern_event_notice(ble_ccc_ctx[recordIndex].cccLogicChannelId,EXT_EVENT_TIMER_HANDLER,gRecvQueuePtr.dataBuff,gRecvQueuePtr.length);
    }
    //============================================================================
	// [ UWB ] - Ranging Session Setup Request
	//============================================================================
    else if (gRecvQueuePtr.evtType == UWB_EVT_RANGING_SESSION_SETUP_RQ)/*测距会话建立 请求*/
    {
		#ifndef FIT_DEBUG_NO_UWB
        stUWBSDK.fpUQRangingSessionSetup(gRecvQueuePtr.dataBuff, gRecvQueuePtr.length);
        // KW38_INT_Start();
		#endif
    }
    //============================================================================
	// [ UWB ] - Ranging Session Start Request
	//============================================================================
    else if (gRecvQueuePtr.evtType == UWB_EVT_RANGING_SESSION_START_RQ)/*测距会话开始 请求*/
    {
		#ifndef FIT_DEBUG_NO_UWB
        stUWBSDK.fpUQRangingCtrl(UWBRangingOPType_Start, gRecvQueuePtr.dataBuff, gRecvQueuePtr.length,stUWBSDK.fpUQSendMSG);
        // KW38_INT_Start();
        intIRQFlag = 0x00;
		#endif
    }
    //============================================================================
	// [ UWB ] - Ranging Session Start Response
	//============================================================================
    else if (gRecvQueuePtr.evtType == UWB_EVT_INT_NOTICE)/*测距会话开始 响应*/
    {
		#ifndef FIT_DEBUG_NO_UWB
        if(intIRQFlag == 0x01)
        {
            intIRQFlag = 0U;
            //TP_PB1_Toggle();
            stUWBSDK.fpUQRangingNTFCache(stUWBSDK.fpUQSendMSG);
        }
		#endif
    }
    //============================================================================
	// [ UWB ] - Ranging Session Suspend Request
	//============================================================================
    else if (gRecvQueuePtr.evtType == UWB_EVT_RANGING_SESSION_SUSPEND_RQ)/*测距会话挂起恢复停止 请求*/
    {
        
    }
    //============================================================================
    // [ UWB ] - Ranging Session Suspend Response
	//============================================================================
    else if (gRecvQueuePtr.evtType == UWB_EVT_RANGING_SESSION_SUSPEND_RS)/*测距会话挂起恢复停止 响应*/
    {
        
    }
    //============================================================================
	// [ Dialog ] - For UDS on CanBus
	//============================================================================
    else if (gRecvQueuePtr.evtType == OBD_CTRL_UWB_SESSION)/**/
    {
        LOG_L_S_HEX(CCC_MD,"UDS_CMD:",gRecvQueuePtr.dataBuff,5U);
        if (obdUWBSessionCtrl == 1U)
        {
            /*重启测距*/
            if(ccc_dk_start_ranging_after_fota() == 1)
            {
                tmpBuf[0] = 0x20;
                tmpBuf[1] = 0x04;
                tmpBuf[2] = 0x00;
                LOG_L_S_HEX(CCC_MD,"UDS_RESP0:",tmpBuf,3U);
                uds_res_append_and_send(UDS_CMD_DIAG,tmpBuf,3);
            }
            else //当前未连接FOB
            {
                tmpBuf[0] = 0x20;
                tmpBuf[1] = 0x04;
                tmpBuf[2] = 0x01;
                LOG_L_S_HEX(CCC_MD,"UDS_RESP1:",tmpBuf,3U);
                uds_res_append_and_send(UDS_CMD_DIAG,tmpBuf,3);
            }
        }
        else
        {
            /*停止测距*/
            if(ccc_dk_stop_ranging_on_fota() == 0)  //没有在测距中
            {
                tmpBuf[0] = 0x20;
                tmpBuf[1] = 0x04;
                tmpBuf[2] = 0x00;
                LOG_L_S_HEX(CCC_MD,"UDS_RESP2:",tmpBuf,3U);
                uds_res_append_and_send(UDS_CMD_DIAG,tmpBuf,3);
            }
        }
    }
    //============================================================================
	//
	//============================================================================
    else if (gRecvQueuePtr.evtType == UWB_EVT_RECV_UART)/*测距结果通知*/
    {
        
    }
    //============================================================================
	//
	//============================================================================
    else if(gRecvQueuePtr.evtType == CCC_EVENT_RSSI_UPDATE)  //rssi
    {
        ccc_api_extern_event_notice(ble_ccc_ctx[recordIndex].cccLogicChannelId, EXT_EVENT_RSSI_UPDATE, gRecvQueuePtr.dataBuff, gRecvQueuePtr.length);
    }
    //============================================================================
	//
	//============================================================================
    else
    {
        /*TODO*/
    }
    //============================================================================
	//
	//============================================================================
LableOut:
    /*出栈了需要释放已申请的空间*/
    if(gRecvQueuePtr.dataBuff != NULL)
    {
        (void)OSA_MutexLock(gBleCCCMutexId, osaWaitForever_c);        
    	core_platform_free(gRecvQueuePtr.dataBuff);
        (void)OSA_MutexUnlock(gBleCCCMutexId);
    }
}

//********************************************************************************
//
//********************************************************************************
u8 cnt = 0;
void BleCccProcess_Task(void* argument)
{
	// ble_ccc_queue_init();
//    ble_ccc_init();
    while(1)
    {
        // OSA_TimeDelay(10);
        ble_ccc_process();
        cnt++;
        if (cnt ==5)
        {
           cnt = 0;
           //LOG_L_S(DLTP_MD,"BleCccProcess_Task mini stack size:%d\n\r", uxTaskGetStackHighWaterMark(gBleCccTaskId));
        }
    }
}
//********************************************************************************

