#ifndef __CCC_TYPE_H__
#define __CCC_TYPE_H__

#ifndef u8
typedef unsigned char						u8;
#endif

#ifndef s8
typedef signed char							s8;
#endif

#ifndef u16
typedef unsigned short						u16;
#endif

#ifndef s16
typedef signed short						s16;
#endif

#ifndef u32
typedef unsigned int						u32;
#endif

#ifndef s32
typedef signed int							s32;
#endif

#ifndef boolean
typedef unsigned char						boolean;
#endif

#ifndef memref
typedef unsigned int						memref;
#endif

#ifndef u64
typedef unsigned long long                  u64;
#endif

#ifndef TRUE
#define TRUE                                0x01
#endif

#ifndef FALSE
#define FALSE                               0x00
#endif

#ifndef UNEXIST
#define UNEXIST                             (-1)
#endif

#ifndef NULL
#define NULL                                0x00
#endif

typedef enum
{
    CCC_ERR_CODE_SUCCESS = 0,
    CCC_ERR_CODE_FORMATE_ERR = 0x1001,            /*格式错误*/
    CCC_ERR_CODE_DATA_ERR,                        /*数据错误*/
    CCC_ERR_CODE_ILLEGAL_STATE,                   /*非法状态*/
    CCC_ERR_CODE_UN_SUPPORT,                      /*不支持该功能*/
    CCC_ERR_CODE_CONN_NUM_EXPAIRED,               /*连接数超限*/
    CCC_ERR_CODE_ACTION_NOT_PERMIT,               /*操作不允许*/
    CCC_ERR_CODE_SE_COMMUNICATE_ERR,              /*车端SE通信异常*/
    CCC_ERR_CODE_UN_MATCH_PROTOCOL,               /*不匹配的协议版本*/
    CCC_ERR_CODE_WRITE_DATA_IS_OPTIONAL,          /*write data 7F4C is optional*/
    CCC_ERR_CODE_ALGORITHM_ERR,                   /*算法模块出错*/
    CCC_ERR_CODE_SECURITY_CHANNEL_SETUP_FAILED,   /*安全通道建立失败*/
    CCC_ERR_CODE_UN_SUPPORT_CHANNEL,              /*不支持的数据通道*/
    CCC_ERR_CODE_UWB_SETUP_FAILED,                /*ranging Session setup failed*/
    CCC_ERR_CODE_ON_PROCESSING,                   /*处理中，请稍候*/
    CCC_ERR_CODE_INSUFFICIENT_ANCHR,              /*Ranging session 创建失败，原因成功响应的锚点不足3个*/  
    CCC_ERR_CODE_RANGING_SESSION_UNEIXST,         /*由ranging session id找不到ranging session */
    CCC_ERR_CODE_RANGING_CONFLICT,                /*多连时Ranging冲突，已有一个活动的ranging session*/
    CCC_ERR_CODE_ANCHOR_STATE_ERR,                /*锚点状态不正常*/
    
    //6xxx-9xxx留给SE响应的SW;
    CCC_ERR_CODE_SE_RESP_DATA_ERR = 0x6A80,       /*SE响应数据错误*/
}CCCErrCode_e;

typedef enum 
{
    TIMER_TYPE_ONCE = 0,
    TIMER_TYPE_REPEAT = 1,
}TimerType_e;

typedef enum
{
    CHANNEL_TYPE_BLE = 0x00,                   
    CHANNEL_TYPE_CAN,                    
    CHANNEL_TYPE_SPI                    
}ChannelType_e;

typedef enum
{
    CHANNEL_ID_NFC      = 0x00,           /*NFC通道，包括NFC手机及卡片*/

    //BLE连接(包括手机及FOB，个数需要跟MAX_BLE_CONN_NUM一致)
    CHANNEL_ID_BLE_0          = 0x01,
    //CHANNEL_ID_BLE_1          = 0x02,
    //CHANNEL_ID_BLE_2          = 0x03,
    CHANNEL_ID_BLE_FOB        = 0x02,

    CHANNEL_ID_BLE_PRIVATE    = 0x03,     /*私有蓝牙协议，仅上传定位信息给手机APP*/

    CHANNEL_ID_CAN      = 0x20,

    CHANNEL_ID_SPI_SE   = 0x30,
    CHANNEL_ID_SPI_UWB  = 0x31,
}ChannleID_e;

typedef enum
{
    INFO_TYPE_FIRMWARE_VER = 0,      /** 集成方固件版本 */
    INFO_TYPE_VIN,                   /** VIN号 */
    INFO_TYPE_CAN_PACK_SIZE,         /** CAN上一包大小 */
    INFO_TYPE_VEHICLE_ADDR,          /** 车端BLE地址 Static Address*/
    INFO_TYPE_BLE_IRK,               /** 车端BLE IRK*/
    INFO_TYPE_DEVICE_ADDR,           /** Device ADDR*/
    INFO_TYPE_VEHICLE_ECC_PUBKEY_X_ON_OOB_PAIRING,   /** OOB配对时车端公钥**/
}InfoType_e;

typedef enum
{
    EXT_EVENT_BLE_CONNECT_ON_PAIRING = 0x01,           /** 车主手机连接准备进行配对 */
    EXT_EVENT_BLE_CONNECT_ON_FIRST_APPROACH,           /** Friend Device's First Approach */
    EXT_EVENT_BLE_CONNECT,                             /** 除配对及 first approach之外的蓝牙连接 */
    EXT_EVENT_BLE_DISCONNECT,                          /** Device BLE dis connect */
    EXT_EVENT_CHECK_CONFIRM_RECEIVED_FROM_OOB,         /** 校验 First_Approach_RQ中的E2_Payload中的Ca,流程见Figure 19-3: Bluetooth LE Pairing and Encryption Setup，**/
    EXT_EVENT_BLE_PAIRING_AND_ENCRYPTION_SETUP_FINISH, /** Figure 19-3: Bluetooth LE Pairing and Encryption Setup，完成整个操作**/

    EXT_EVENT_TIMER_HANDLER,                           /** timer handler处理转至task处理 **/
    EXT_EVENT_RSSI_UPDATE,                             /*  RSSI更新*/
}ExtEvent_e;

typedef enum
{   
    SDK_EVENT_BLE_DISCONECT,                       /**请求断开指定的蓝牙连接**/
    SDK_EVENT_TIME_SYNC_TRIGGER_BY_VEHICLE,        /**车端触发时间同步*/
    SDK_EVENT_BLE_SECURE_OOB_PAIRING_PREP_FINISH,  /**Figure 19-16: Bluetooth LE Secure OOB Pairing Prep已完成**/
    SDK_EVENT_BLE_SECURE_OOB_MISMATCH,             /**Device 通知 OOB mismatch **/
    SDK_EVENT_FA_CRYPTO_OPERATION_FAILED,          /**signal the failure in First approach due to cryptography*/

    SDK_EVENT_TIMER_HANDLER,                       /**timer 超时回调事件转task**/
}SDKEvent_e;

/**< RTC time struct YYYYMMDD T HHMMSS Z(或者时区标识)。例如，20100607T152000Z，表示2010年6月7号15点20分0秒，Z表示是标准时间 */
typedef struct utc_time_t
{
    u8 	seconds;  		/**< 0-59 */
    u8 	minutes;  		/**< 0-59 */
    u8 	hour;     		/**< 0-23 */
    u8 	day;      		/**< 1-31 */
    u8 	month;    		/**< 1-12 */
    u16	year;     		/**< 2000-2099 */
};

/**
 * @brief
 *      打印回调函数
 *
 * @param [pdata] 数据指针
 * @param [length] 数据长度
 *
 * @return
 *      0 成功，其他失败
 *
 * @retval 
 *		
 *
 * @exception
 *
 * @note
 */
typedef int (*pExtFunc_debug_printf)(u8 *pdata, u32 length);

/**
 * @brief
 *      向集成方请求数据
 *
 * @param [iType] 见enum InfoType_e
 * @param [pInData] 输入参数，输入参数数据存储地址
 * @param [pInLen]  输入参数，输入参数长度
 * @param [pOutData]输出参数，获取数据存储地址
 * @param [pOutLen] 输出参数，获取数据长度存储地址
 *
 * @return
 *      0 成功，其他失败
 *
 * @retval 
 *		
 *
 * @exception
 *
 * @note
 */
typedef int (*pExtFunc_get_info)(InfoType_e iType, u8* pInData, u16 pInLen, u8 *pOutData, u16 *pOutLen);


/**
 * @brief
 *      获取UTC时间
 * 
 * @param [pOutTime] 输出参数，UTC时间存储地址
 *
 * @return
 *      0表示成功，其它失败。
 *
 * @retval 
 *		
 *
 * @exception
 *
 * @note
 *      此接口用于获取UTC时间
 *
 *
 */
typedef int (*pExtFunc_get_utc_time)(struct utc_time_t *pOutTime);

/**
 * @brief
 *      设置UTC时间
 *
 * @param [pInTime] 输入参数，UTC时间存储地址
 * @return
 *      0表示成功，其它失败。
 *
 * @retval 
 *		
 *
 * @exception
 *
 * @note
 *      此接口用于设置UTC时间
 *
 *
 */
typedef int (*pExtFunc_set_utc_time)(struct utc_time_t *pInTime);


//----------------------extern provide ble func-------------------------------------------------
/**
 * @brief
 *      发送数据给指定通道
 *
 *  @param [chId]: 数据发送的通道ID
 *  @param [pData]: 发送数据存储地址
 *  @param [pDataLen]: 发送数据长度
 *     
 * @return
 *      0表示成功，其它失败。
 *
 * @retval 
 *		
 *
 * @exception
 *
 * @note
 *      此接口用于发送指令数据
 *
 */
typedef int (*pExtFunc_send_data)(ChannleID_e chId, u8* pInData, u16 pInDataLen);


/**
 * @brief
 *      SDK事件通知集成方
 *
 *  @param [chId]: 事件发生源通道号
 *  @param [sdkEvent]: 事件类型
 *  @param [pData]: 事件同时传递的数据存储地址；
 *  @param [pDataLen]: 事件同时传递的数据长度
 *     
 * @return
 *      0表示成功，其它失败。
 *
 * @retval 
 *		
 *
 * @exception
 *
 * @note
 *      此接口用于向指定通道发送数据
 *
 */
typedef int (*pExtFunc_sdk_event_notice)(ChannleID_e chId, SDKEvent_e sdkEvent, u8* pInData, u16 pInDataLen);

/**
 * @brief
 *      timer超时处理函数
 *
 *  @param [chId]: 回调时的输入参数：start时传入的参数
 *    
 * @return
 *      无
 *
 * @retval 
 *		
 *
 * @exception
 *
 * @note
 *      timer超时回调函数，由SDK实现；
 *
 */
typedef void (*pFunc_timer_handler)(void* param);

/**
 * @brief
 *      创建Timer，返回timerId；
 *   
 * @return
 *      >= 0； timer创建成功， timer的编号;
 *      < 0;  timer创建失败；
 *
 * @retval 
 *		
 *
 * @exception
 *
 * @note
 *      此接口用于创建timer，同时注册了超时回调函数
 *
 */
typedef int (*pExtFunc_create_timer)(void);

/**
 * @brief
 *      启动timer
 *
 *  @param [timeId]: 要启动的timerId
 *  @param [timerType]: 0: 一次性timer; 1: 重复timer;
 *  @param [timeInMilliseconds]: timer的超时时间，单位ms;
 *  @param [timerHandler]: timer超时的回调函数 
 *  @param [param]: timer超时回调时需要传递的参数
 *    
 * @return
 *      0：启动成功
 *      others: 启动失败;
 *
 * @retval 
 *		
 *
 * @exception
 *
 * @note
 *      此接口用于创建timer，同时注册了超时回调函数
 *
 */
typedef int (*pExtFunc_start_timer)(int timeId, u8 timerType, u32 timeInMilliseconds, pFunc_timer_handler timerHandler, void* param);

/**
 * @brief
 *      停止正在运行的timer;
 *
 *  @param [timerId]: 要操作的timerId
 * 
 * @return
 *      0: 操作成功；others: 操作失败；
 *
 * @retval 
 *		
 *
 * @exception
 *
 * @note
 *      创建timer的超时回调函数，由SDK实现；
 *
 */
typedef int (*pExtFunc_stop_timer)(int timerId);

typedef struct 
{
	pExtFunc_create_timer					createTimer;            		/**< 创建timer  */
	pExtFunc_start_timer				    startTimer;					    /**< 启动timer */
	pExtFunc_stop_timer				        stopTimer;					    /**< 停止timer */
}timer_tool_t;
//-----------------------------------------------------HSM API------------------------------------------------------------
/**
 * @brief
 *      硬件加密模块初始化
 *
 *
 * @return
 * 	    若成功则返回0，否则返回失败。
 *
 * @retval
 *
 *
 * @exception
 *
 * @note
 *      此接口用于初始化SE.
 *
 *
 */
typedef int (*pExtFunc_hsm_init)(void);


/**
 * @brief
 *      硬件加密模块复位
 *
 *
 * @return
 * 	    若成功则返回0，否则返回失败。
 *
 * @retval
 *
 *
 * @exception
 *
 * @note
 *      此接口用于硬件加密模块的复位操作
 *
 *
 */
typedef int (*pExtFunc_hsm_reset)(void);


/**
 * @brief
 *      OS系统延时
 *
 * @param [ms] 延时时间-单位毫秒
 *
 * @return
 * 	    若成功则返回0，否则返回失败。
 *
 * @retval
 *
 *
 * @exception
 *
 * @note
 *      此接口用于阻塞时释放CPU资源。
 *
 *
 */
typedef void (*pExtFunc_hsm_delay)(u32 ms);




/**
 * @brief
 *      SPI发送
 *
 * @param [tbuff] 发送数据指针
 * @param [length] 发送数据长度
 *
 * @return
 * 	    若成功则返回实际发送长度，其它表示失败。
 *
 * @retval
 *
 *
 * @exception
 *
 * @note
 *      此接口用于安全芯片通信的SPI数据发送
 *
 *
 */
typedef int (*pExtFunc_hsm_spi_send)(u8 *tbuff, u16 length);


/**
 * @brief
 *      SPI发送
 *
 * @param [rbuff] 接收数据指针
 * @param [length] 接收数据长度
 *
 * @return
 * 	    若成功则返回实际接收长度，其它表示失败。
 *
 * @retval
 *
 *
 * @exception
 *
 * @note
 *      此接口用于安全芯片通信的SPI数据接收
 *
 *
 */
typedef int (*pExtFunc_hsm_spi_recv)(u8 *rbuff, u16 length);


/**< HSM Control Attributes Type */
typedef struct
{
	pExtFunc_hsm_init					init;            		/**< 初始化  */
	pExtFunc_hsm_reset				    reset;					/**< 复位 */
	pExtFunc_hsm_delay				    delay;					/**< 系统延时 */
	pExtFunc_hsm_spi_send				send;					/**< 发送 */
	pExtFunc_hsm_spi_recv				recv;					/**< 接收 */
}hsm_ctrl_attr_t;

//-------------------------------------需要集成方提供的软算法接口------------------------------------------------------------
/**
 * @brief
 *      ECC生成公私钥，椭圆参数固定为：SECP256R1
 *
 * @param [pOutPubKey] 生成公钥存储地址，公钥格式为：04 + X(0x20字节) + Y(0x20字节)
 * @param [pOutPriKey] 生成私钥存储地址，长度为0x20字节
 *
 * @return
 * 	    0: 生成成功；others： 生成失败
 *
 * @retval
 *
 *
 * @exception
 *
 * @note
 *      此接口用于生成ECC公钥私
 *
 *
 */
typedef int (*pExtFunc_ecc_generate_key)(u8* pOutPubKey, u8* pOutPriKey);

/**
 * @brief
 *      ECDH算法
 *
 * @param [pInPubKey] 参与ECDH公钥存储地址，公钥格式为：04 + X(0x20字节) + Y(0x20字节)
 * @param [pInPriKey] 参与ECDH私钥存储地址，长度为0x20字节
 * @param [pOutAgreeKey] 协商结果存储地址, X(0x20字节) + Y(0x20字节)
 *
 * @return
 * 	    0: 协商成功；others： 协商失败
 *
 * @retval
 *
 *
 * @exception
 *
 * @note
 *      此接口用于生成ECDH协商会话密钥
 *
 *
 */
typedef int (*pExtFunc_key_agreement_ecdh)(u8* pInPubKey, u8* pInPriKey, u8* pOutAgreeKey);

/** 集成方软算法提供接口 **/
typedef struct 
{
	pExtFunc_ecc_generate_key			eccGenKey;               /**< ecc生成公私钥对  */
	pExtFunc_key_agreement_ecdh		    eccECDH;				 /**< 密钥协商ecdh */
}sw_algorithm_t;

#endif
