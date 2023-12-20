#ifndef _BLE_CCC_H_
#define _BLE_CCC_H_
#include "EM000101.h"
#include "EM000401.h"
#include "RTE_ccc.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "ble_general.h"
#include "ccc_dk_type.h"
#define	CCC_QUEUE_MAX_NUMBER		50
// #define	CCC_QUEUE_RECORD_LENGTH	


extern gapPhyEvent_t  gCccLePhy;
/***白名单操作****/
/**
 * @brief
 *      根据MAC地址查询设备的LTK
 * @param [ltk]         临时存放LTK的缓存地址
 * @param [macAddress]  MAC地址
 * @return
 *        1表示成功，其它失败
 * @note
 */
u8 ble_ccc_get_ltk_from_macaddress(u8* ltkBuffer,u8* macAddress);



/****L2CAP通道****/
#if defined(gCccL2Cap_d) && (gCccL2Cap_d == 1)
#define gCcc_L2capLePsm_c  				0x0085	//0x004F
#define gCccCmdMaxLength_c 				0x200	//270
#define mAppLeCbInitialCredits_c        (65500)	//(32768)
#endif


#define BLE_L2CAP_STATUS_CONNECT        1
#define BLE_L2CAP_STATUS_DISCONNECT     0

typedef enum
{
	CCC_EVT_IDLE = 0,
	CCC_EVT_STATUS_CONNECT,			/*连接蓝牙通知*/
	CCC_EVT_STATUS_DISCONNECT,		/*断开蓝牙连接通知*/
	CCC_EVT_L2CAP_SETUP_COMPLETE,	/*L2CAP通道建立完成*/
	CCC_EVT_L2CAP_DISCONNECT,		/*L2CAP通道断开*/
	CCC_EVT_STATUS_MAC_UPDATE,		/*MAC地址更新,iphone的地址是随机地址，
										只有配对成功后才能拿到真实地址，
										下一次连接时拿到的也是真是地址*/
	CCC_EVT_CMD_DISCONNECT,			/*主动断开蓝牙连接*/
	CCC_EVT_BLE_PAIR_COMPLETE,		/*蓝牙配对 LTK已生成*/
	CCC_EVT_LESETPHY,				/*设置LE PHY*/
	CCC_EVT_RECV_DATA,				/*收到L2CAP数据*/
	CCC_EVT_SEND_DATA,				/*发送L2CAP数据*/
	CCC_EVT_RECV_CAN_DATA,			/*收到CAN数据*/
	CCC_EVT_RECV_UWB_DATA,			/*收到UWB数据*/

	CCC_EVT_SEND_PRIVATE_DATA,		/*往FFE0的FFE2发送数据*/

	CCC_EVT_WRITE_WHITELIST,		/*写白名单*/
	CCC_EVT_TEST_SET_FOB_ADDR,		/*设置FOBMac地址*/
	CCC_EVT_TIMER_HANDLER,          /*timer handler转task处理*/
	
	
	UWB_EVT_WAKEUP_RQ,				/*唤醒UWB 请求*/
	UWB_EVT_WAKEUP_RS,				/*唤醒UWB 响应*/

	UWB_EVT_TIMER_SYCN_RQ,			/*时间同步 请求*/
	UWB_EVT_TIMER_SYCN_RS,			/*时间同步 响应*/

	UWB_EVT_RANGING_SESSION_SETUP_RQ,	/*测距会话建立 请求*/
	UWB_EVT_RANGING_SESSION_START_RQ,	/*测距会话开始 请求*/
	UWB_EVT_RANGING_SESSION_START_RS,	/*测距会话开始 响应*/

	UWB_EVT_RANGING_SESSION_SUSPEND_RQ,	/*测距会话挂起恢复停止 请求*/
	UWB_EVT_RANGING_SESSION_SUSPEND_RS, /*测距会话挂起恢复停止 响应*/
	
	UWB_EVT_RANGING_RESULT_NOTICE,		/*测距结果通知*/

	UWB_EVT_INT_NOTICE,		/*UWB INT 中断事件*/
	UWB_EVT_RECV_UART,		/*UWB UART*/

	CCC_EVENT_RSSI_UPDATE,   /*RSSI update*/
	OBD_CTRL_UWB_SESSION,   /**/
}ccc_evt_type_t;

// typedef enum
// {
// 	CCC_LESETPHY_STEP0 = 0,
// 	CCC_LESETPHY_STEP1,
// 	CCC_LESETPHY_STEP2,
// };

typedef enum
{
	BLE_DEVICE_FOB = 0,		/*连接的设备为FOB*/
	BLE_DEVICE_CCC,			/*连接的设备为CCC 手机*/
	BLE_DEVICE_PRIVATE,		/*连接的设备为Private 手机*/
	BLE_DEIVCE_MAX_NUMBER, 
}ble_device_type_t;


typedef struct 
{
	u8 validFlag;		/*有效标志*/
	u8 cccLogicChannelId; /*CCC SDK逻辑通道号*/
	ble_device_type_t  deviceType;  /*已连接设备的类型*/
    u8 connectStatus;   /*当前蓝牙连接状态  0:断开  1:连上*/
    u8 deviceId;        /*当前连接的设备Id*/
    u16 channelId;       /*当前连接的L2CAP 逻辑通道Id*/

	u16 start_counter;	 /*开始时间同步起始counter值, ce_counter是从连上就开始计数, 
							实际时间同步时也需要从0开始计数，因此需要减去中间的差值*/
	u16 ce_counter;      /*!< Connection event counter, valid for conn event over or Conn Rx event */
	u16 timestamp;       /*!< Timestamp in 625 us slots, valid for Conn Rx event and Conn Created event */

	u16 DeviceEventCount; /*设备连接时间计数值,从0开始*/

	u8 macAddress[6];	/*连接设备的mac地址*/

	u8 firstConnectFlag; /*已连设备是否第一次连接*/

	u8 oldIndex;
}ble_ccc_ctx_t;

extern ble_ccc_ctx_t ble_ccc_ctx[BLE_DEIVCE_MAX_NUMBER];





typedef struct 
{
    u16 length;
    u8* data;
}ble_ccc_data_t;

typedef struct 
{
    ccc_evt_type_t evtType;	/*消息类型*/
	u8 deviceId;			/*消息设备ID*/
	u16 length;				/*消息数据长度*/
	u8* dataBuff;			/*消息数据体*/
}ble_ccc_queue_msg_t;


/*收取L2CAP 逻辑通道上数据*/
u8 ble_ccc_l2cap_recv_data(u8 deviceId, u8* inData, u16 length);
/*设置L2CAP 连接参数*/
void ble_ccc_l2cap_set_parameter(u8 deviceId, u8 channelId);
/*设置L2CAP 连接状态*/
void ble_ccc_l2cap_set_connect_status(u8 status);
/*获取已连接的L2CAP ChannelId*/
u8 ble_ccc_l2cap_get_connectChannelId(void);


/***CCC业务处理流程 定义***/
typedef enum
{
	OBJ_ID_SDK = 0xF001,	    //0xF001
	OBJ_ID_BLE,			        //0xF002
	OBJ_ID_UWB,		            //0xF003  
	OBJ_ID_NFC_DOOR,		    //0xF004
	OBJ_ID_NFC_CONSOLE,		    //0xF005
	OBJ_ID_MCU,		            //0xF006
    OBJ_ID_4G                   //0xF007
} OBJ_ID_T;

typedef struct 
{
    u16 recvSerialNum;       /*当前接收数据的序列号*/
    u16 sendSerialNum;       /*当前接收数据的序列号*/
    u8 msgChannelId;    /*Channel_ID 为 1 个字节长度，表示当前物理设备的逻辑通道, 当前最大支持 4 个通道，
                        0x00 :固定为NFC使用通道 
                        0x01-0x03 :可以最大同时支持3路BLE通道 
                        0xFF :供内部组件业务使用*/
}ble_ccc_msg_t;

typedef struct 
{
    u8 pairingMode;     /*当前配对模式,0:配对模式   1：非配对模式 */
    
    u8 slotId[9];       /*当前认证的slotId：LV结构*/
    u8 slotIdType;      /*当前slotid对象类型, 0:车主  1:分享人 2: FOB KEY*/
    

}ble_ccc_evt_t;


#define BLE_MSG_SEND_REQDATA_OFFSET_TAG             0
#define BLE_MSG_SEND_REQDATA_OFFSET_LENGTH          1
#define BLE_MSG_SEND_REQDATA_OFFSET_V_CHANNELID     3
#define BLE_MSG_SEND_REQDATA_OFFSET_V_DK_MSG        4


typedef enum
{
	BLE_EVT_TAG_PAIRING_SETUP = 0x0,	    //00：配对模式开关
	BLE_EVT_TAG_OOB_PAIRING,			    //01：OOB 配对 
	BLE_EVT_TAG_SLOTID,		                //02：当前认证 slotid 对象属性   
	BLE_EVT_TAG_DELETE_KEY,		            //03：delete 删除钥匙数据 
	BLE_EVT_TAG_CLEAR_KEY,		            //04: clear 钥匙数据 
	BLE_EVT_TAG_DISCONNECT,		            //05：断开当前 channel_id 连接 
    BLE_EVT_TAG_SYCN_KEYINFO,               //06：同步当前车端的全部激活钥匙槽信息
    BLE_EVT_TAG_SYCN_CHANNEL,               //07: 同步当前 channel_id 使用状态     
};

typedef enum
{
	SDK_EVT_TAG_BLE_CONNECT = 0x0,	        //00 : 设备接入(蓝牙设备连接)
	SDK_EVT_TAG_BLE_DISCONNECT,			    //01 : 设备断开(蓝牙设备断链)
	SDK_EVT_TAG_LTK_RESULT,		            //02 : LTK 建立成功|失败通知 
	SDK_EVT_TAG_SYCN_SLOTID,		        //03 : 同步当前车端激活钥匙槽列表
	SDK_EVT_TAG_SYCN_CHANNELID,		        //04 : 同步 channel_id 列表 
}sdk_evt_t;

typedef enum
{
	// CANID_SA_0x200 = 0,
	// CANID_SA_0x201,
	// CANID_SA_0x202,
	// CANID_SA_0x203,

	CANID_SA_0x210 = 0,
	CANID_SA_0x220,
	CANID_SA_0x230,
	CANID_SA_0x240,
	CANID_SA_0x250,
	CANID_SA_0x260,

	CANID_SA_0x211,
	CANID_SA_0x221,
	CANID_SA_0x231,
	CANID_SA_0x241,
	CANID_SA_0x251,
	CANID_SA_0x261,

	CANID_SA_0x212,
	CANID_SA_0x222,
	CANID_SA_0x232,
	CANID_SA_0x242,
	CANID_SA_0x252,
	CANID_SA_0x262,

	CANID_SA_0x213,
	CANID_SA_0x223,
	CANID_SA_0x233,
	CANID_SA_0x243,
	CANID_SA_0x253,
	CANID_SA_0x263,

	CANID_SA_0x2A0,
	CANID_SA_0x2A1,

#ifdef FIT_DEBUG_NO_SA 
	CANID_UWB_0x313,
	CANID_UWB_0x323,
	CANID_UWB_0x333,
	CANID_UWB_0x343,
	CANID_UWB_0x353,
	CANID_UWB_0x363,
#endif

	CANID_ODB_0x58,
	CANID_ODB_0x59,
	//Modify (Ken):VEHICLE-V0C02 NO.1 -20231218
	#if defined __FIT_Aeon_H
	CANID_ODB_0x60,
	#endif
}cccCanId_t;
/**
 * @brief
 *      通知SDK模块事件数据
 * @param [sdk_evt_t]    SDK事件
 * @param [channelId]    当前蓝牙连接通道号
 * 
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
 *              01 : Channel_id(0x01-0x03)
 *              02 : Channel_id(0x01-0x03)+ Flag Flag-00:建立成功，01：建立失败 
 *              03 : Channel_id(0xFF) 
 *              04 : Channel_id(0xFF)
 * 
 */
u8 ble_ccc_evt_notify_sdk(sdk_evt_t sdk_evt ,u8 channelId,u8* inData,u16 length);



//
u8 ble_ccc_send_data(u8 deviceId, u8* inPut, u16 length);
u8 ble_ccc_send_leSetPhyRequest(ChannleID_e chId);
u8 ble_ccc_send_evt(ccc_evt_type_t evtType, u8 deviceId, u8* pdata, u16 length);
/*任务初始化*/
void ble_ccc_task_init(void);

/*通过设备号查找通道号*/
u16 ble_ccc_ctx_get_channelId_from_deviceId(u8 deviceId);
#endif
