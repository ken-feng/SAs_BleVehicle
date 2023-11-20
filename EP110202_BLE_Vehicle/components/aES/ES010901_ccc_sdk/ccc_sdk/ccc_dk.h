#ifndef __CCC_DK_H__
#define __CCC_DK_H__
#include "ccc_dk_type.h"
#include "ccc_dk_api.h"
#include "ccc_dk_config.h"
#include "../ccc_msg/ccc_dk_ble_msg.h"

#define VEHICLE_STATUS_WAITING_FOR_PAIRING        0x0F   //等待配对状态
#define VEHICLE_STATUS_PAIRED_WITH_OWNER          0x10   //已完成配对

#define M_SUPPORTED_UWB_CONFIGURATION             0x0A   
#define L_SUPPORTED_PULSESHAPE_COMBO              0x10

#define ANCHOR_NUM_MAX                            0x06  //总锚点个数
#define RESPONDER_INFO_LEN                        0x12

#define LEN_RKE_ARBITRARY_DATA                    0x20

typedef enum device_location
{
    LOC_NO_LOCINFO                =0x00, /** 位置未确认**/
    LOC_NON_WELCOM                =0x01, /** 连接区，迎宾区之外 **/
    LOC_WELCOME_1                 =0x02, /** 一级迎宾区 **/
    LOC_WELCOME_2                 =0x03, /** 二级迎宾区 **/
    LOC_FAREWELL                  =0x04, /** 送宾区 **/
    LOC_PE_LEFT                   =0x05, /** 左PE  **/
    LOC_PE_RIGHT                  =0x06, /** 右PE  **/
    LOC_DRIVER_SEAT               =0x07, /** 主驾位置  **/
    LOC_PS                        =0x08, /** PS区，车内  **/
    LOC_TRUNK                     =0x09, /** 后备箱内 **/
    LOC_TAIL                      =0x0A, /** 车尾外部 **/
} Location_e;

typedef enum
{
    LOC_IND_UNDEFINE     = 0x00,    /**未设置位置标示**/
    
    LOC_IND_LEFT_FRONT   = 0x01,    /**左前**/
    LOC_IND_RIGHT_FRONT  = 0x02,    /**右前**/
    LOC_IND_CENTER       = 0x04,    /**中间锚点1，中控台**/
    LOC_IND_LEFT_BACK    = 0x08,    /**左后**/
    LOC_IND_RIGHT_BACK   = 0x10,    /**右后**/
    LOC_IND_CENTER2      = 0x20,    /**中间锚点2，后座中间**/

    LOC_IND_ALL          = 0xFF,    /**所有位置**/
}LocInd_e;

typedef enum
{
    RANGING_ACTION_SETUP = 0x00,   
    RANGING_ACTION_SUSPEND = 0x01,
    RANGING_ACTION_RECOVER,
    RANGING_ACTION_CONFIGURABLE_RECOVER,
    RANGING_ACTION_DELETE,
    RANGING_ACTION_NONE,
}RangingAction_e;

typedef enum
{
    PAIRING_PHASE2_STATUS_SELECT_AID_STEP1 = 0x01,  
    PAIRING_PHASE2_STATUS_SPAKE2_REQUEST = 0x02,
    PAIRING_PHASE2_STATUS_SPAKE2_VERIFY = 0x03,
    PAIRING_PHASE2_STATUS_WRITE_DATA = 0x04,
    PAIRING_PHASE2_STATUS_OP_CONTROL_FLOW_STEP5,

    PAIRING_PHASE2_STATUS_SELECT_AID_STEP7,
    PAIRING_PHASE2_STATUS_GET_DATA,
    PAIRING_PHASE2_STATUS_GET_RESPONSE,

    PAIRING_PHASE2_STATUS_WRITE_DATA_ATTESTATION,
    PAIRING_PHASE2_STATUS_OP_CONTROL_FLOW_STEP15,
    PAIRING_PHASE2_STATUS_OP_CONTROL_FLOW_STEP17,   //unexist while pairing failed;
}OwnerPairingPhase2Flow_e;

typedef enum
{
    PAIRING_PHASE3_STATUS_FREE = 0x00,
    PAIRING_PHASE3_STATUS_SELECT = 0x01,
    PAIRING_PHASE3_STATUS_AUTH_0,
    PAIRING_PHASE3_STATUS_AUTH_1,
    PAIRING_PHASE3_STATUS_EXCHANGE,
    PAIRING_PHASE3_STATUS_OP_CONTROL_FLOW_STEP13,
    PAIRING_PHASE3_STATUS_DESELECT_SE,
}OwnerPairingPhase3Flow_e;

//Figure 6-10: Owner Pairing Flow – Phase 4: Finalization
typedef enum
{
    PAIRING_PHASE4_STATUS_FREE = 0x00,
    PAIRING_PHASE4_STATUS_SELECT = 0x01,
    PAIRING_PHASE4_STATUS_AUTH_0,
    PAIRING_PHASE4_STATUS_AUTH_1,
    PAIRING_PHASE4_STATUS_OP_CONTROL_FLOW_STEP2,  //p1=40, p2=88, Continue, key tracking response received in device, next step is to read the receipt from the mailbox
    PAIRING_PHASE4_STATUS_EXCHANGE_STEP4,
    PAIRING_PHASE4_STATUS_OP_CONTROL_FLOW_STEP6, //p1=40, p2=81, Optional token refill start
    PAIRING_PHASE4_STATUS_EXCHANGE_STEP8,  //optional write friend_immobilizer_token_1 with friend_slot_identifier_1
    PAIRING_PHASE4_STATUS_OP_CONTROL_FLOW_STEP10,  //p1=40,p2=82, Optional attestation package delete start
    PAIRING_PHASE4_STATUS_EXCHANGE_STEP12,  //delete_attestation_package

    //01,90:End, key is tracked, and all data have been written successfully into the mailboxes
    //01,91:End, key is not tracked, key sharing is not possible, and the owner needs to go online to track the key before using it
    PAIRING_PHASE4_STATUS_OP_CONTROL_FLOW_STEP14,  //p1=end,p2=90/91
    PAIRING_PHASE4_STATUS_DESELECT_SE,
}OwnerPairingPhase4Flow_e;

typedef enum
{
    URSK_DERIVATION_STATUS_FREE = 0x00,
    URSK_DERIVATION_STATUS_SELECT = 0x01,
    URSK_DERIVATION_STATUS_AUTH_0,
    URSK_DERIVATION_STATUS_AUTH_1,
    URSK_DERIVATION_STATUS_CREATE_RANGE_KEY,
    URSK_DERIVATION_STATUS_DESELECT_SE,
}URSKDerivationFlow_e;

typedef enum
{
    RANGING_CAPABILITY_REQUEST = 0x01,
    RANGING_SESSION_REQUEST,
    RANGING_SESSION_SETUP_REQUEST,
    RANGING_SUSPEND_REQUEST,
    RANGING_RECOVERY_REQUEST,
    RANGING_CONFIGURABLE_RANGING_RECOVERY_REQUEST,
}RangingServiceFlow_e;

//Figure 7-1;  step 1-8
typedef enum
{
    STANDARD_TRANSACTION_STATUS_FREE = 0x00,
    STANDARD_TRANSACTION_STATUS_SELECT = 0x01,
    STANDARD_TRANSACTION_STATUS_AUTH_0,
    STANDARD_TRANSACTION_STATUS_AUTH_1,
    STANDARD_TRANSACTION_STATUS_EXCHANGE,
	STANDARD_TRANSACTION_STATUS_CONTROL_FLOW,
    STANDARD_TRANSACTION_STATUS_DESELECT_SE,
}StandardTransactionFlow_e;

/** 子流程 **/
typedef enum
{
    PROCESS_IDLE = 0x00,
    PROCESS_OWNER_PAIRING_PHASE0,
    PROCESS_BLE_LINK_LAYER_CONNECTION_ESTABLISHMENT,
    PROCESS_BLE_OWNER_PAIRNG_GATT_FLOW,
    PROCESS_OWNER_PAIRING_PHASE2,
    PROCESS_BLE_PAIRING_ENCRYPTION_SETUP,
    PROCESS_URSK_DERIVATION_FLOW,
    PROCESS_CAPABILITY_EXCHANGE,
    PROCESS_TIME_SYNC,
    PROCESS_SECURE_RANGING_SETUP_FLOW,
    PROCESS_OWNER_PAIRING_PHASE3,
    PROCESS_OWNER_PAIRING_PHASE4,

    PROCESS_STANDARD_TRANSACTION_FLOW, 
    PROCESS_RKE_FLOW,
    PROCESS_FIRST_TRANSACTION_FRIEDN_FLOW,
}Process_e;

typedef enum
{
    RANGING_SESSION_IDLE = 0,
    RANGING_SESSION_SETUP,
    RANGING_SESSION_ACTIVE,
    RANGING_SESSION_SUSPEND,
}RangingSessionStatus_e;

typedef enum
{
    URSK_STATUS_EMPTY         = 0x00,
    URSK_STATUS_PREDERIVED    = 0x01,
    URSK_STATUS_ACTIVE        = 0x02,  
    URSK_STATUS_SUSPENDED     = 0x04, 
    URSK_STATUS_NOT_EMPTY     = 0xFF
}URSKStatus_e;

typedef enum
{
    RKE_STATUS_FREE = 0,
    RKE_STATUS_TRIGGERED,
    RKE_STATUS_AUTH_RQ,
    RKE_STATUS_AUTH_VERIFY,
    RKE_STATUS_ENDURING,
    RKE_STATUS_FINISH,
}RKEStatus_e;

typedef enum
{
    DEVICE_WORK_MODE_ON_IDLE = 0x00,
    DEVICE_WORK_MODE_ON_PAIRING,               /**配对中**/
    DEVICE_WORK_MODE_ON_FIRST_APPROACH,        /**朋友钥匙的第一次使用中**/
    DEVICE_WORK_MODE_ON_NORMAL_USE,            /**配对和第一次使用中之外的正常使用**/
}DeviceWorkMode_e;

typedef enum
{
    RANGING_RESULT_SUCCESS = 0x00,
    RANGING_RESULT_URSK_LOST_OR_NOT_EXIST,  /** URSK丢失或未协商过 **/
    RANGING_SESSION_NOT_EXIST,              /** 未找到对应的ranging session **/
    RANGING_ANCHOR_OFFLINE,                 /** 锚点不在线 **/
    RANGING_RESULT_STS_EXCEED,              /** STS Exceed, When this maximum value is reached, the ranging session shall be ended and a new session with a new URSK shall be used. **/
    RANGING_ANCHOR_ABNORMAL_COMMUNICATION,  /** 锚点通信异常 **/
    RANGING_RESULT_FAILED,                  /** 其他失败原因 **/
}RangingResult_e;


#define TAG_REQUEST_RKE_ACTION                                0x7F70
#define TAG_SUBSCRIBE_TO_VEHICLE_FUNCTION_STATUS              0x7F73
#define TAG_GET_STATUS_UPDATE_FOR_FUNCTION_IDS                0x7F74
#define TAG_CONTINUE_ENDURING_RKE_ACTION                      0x7F76
#define TAG_STOP_ENDURING_RKE_ACTION                          0x7F77

typedef struct
{
    u8 sessionId[4];  //auth0 input(tag=4C)
    u8 ursk[32];
    URSKStatus_e urskStatus;  //0: empty, 1: predrived;   2: active;
    u32 usageCountLeft;      //TTL用次数来表示  
}URSK_t;

//Vehicles capable of secure ranging shall support the derivation and storage of at least one pre_derived URSK per Digital Key
typedef struct 
{
    u8 slotIdLV[9]; //slotId的最大长度为8字节
    URSK_t ursks[NUM_URSK_PER_DIGITAL_KEY];
}DKURSK_t;

typedef struct 
{
    int timerId;
    u8 timerPar[2];
}Timer_t;

typedef struct 
{
    RKEStatus_e rkeStatus;
    ActionType_e actionType;
    u16 functionId;
    u8 actionId;
    u32 confirmNum;
    u8 connId;    /**下在执行车控的蓝牙通道号**/
    u8 rkeChallenge[16];
    u8 arbitraryData[LEN_RKE_ARBITRARY_DATA];   //arbitray data = sha256(rke_challenge || function id || action id)
}RKERequest_t;

typedef enum
{
    OFF_RESPONDER_STATE = 0,
    OFF_RESPONDER_INFO_SN = 1,
    OFF_RESPONDER_LOC_IND = 0x11,
}responderInfOff_e;

typedef struct
{
    u8 cosVer[2];         //COS版本号
    u8 appletVer[2];      //车端applet版本
    u8 spake2VerTlv[4];   //5A02 xxxx  SPAKE2+协议版本
    u8 dkAppVerTlv[4];   //5C02 xxxx  数字钥匙版本号
    u8 vehicleIdTlv[10];   //4D08 xxxxxxxxxxxxxxxx 车端ID
    u8 seidTlv[0x12];         //4E10 xxxxxxxxxxxxxxxxxxxx  SEID
    u8 pairStatus;       //0F: unPaired;  0x10: paired

    u8 scIv[0x10];       //security_channel_iv
    u8 scKey[0x10];      //security_channel_session_key;

    u8 macAddr[6];       //车端蓝牙地址
    
    u8 respondersNum;    //能正常响应的anchor个数
    u8 responderInfo[RESPONDER_INFO_LEN*ANCHOR_NUM_MAX];  //anchorState(1字节，0x00不正常，0x01：wakeup正常, 0x02: time sync正常) + SN(0x10) + LOC_IND(1)

    //L:Length byte for supported UWB Configuration Identifiers
    //V: 2*m; UWB_Config_Id is a 2 Byte field which is an identifier for the supported UWB configuration. m is the number of UWB configs supported by vehicle
    u8 supportedUWBConfigIdLV[1 + 2*M_SUPPORTED_UWB_CONFIGURATION]; //Table 21-1: Supported UWB Configurations
    //L:  Length byte for the supported PulseShape_Combos
    //V: 1*l; PulseShape_Combo is a 1 Byte field which is an identifier for the supported initiator/response, transmit/receive pulse shape combinations. 
    //See Section 21.5 for the supported subset.l is the number of PulseShape_Combos supported by the vehicle
    u8 supportedPulseShapeComboLV[1 + L_SUPPORTED_PULSESHAPE_COMBO];  //Table 21-4: Overview of PulseShape_Combo values and the associated transmit pulse shapes.
    u8 supportedChannelBitmask;
    u8 supportedRANMultiplierMin;
    u8 supprtedSlotBitMask;
    u8 supportedSYNCCodeIndex[4];
    u8 supportedHoppingConfigBitmask;

    DKURSK_t dkUrsks[MAX_DIGITAL_KEY_STORAGE];  

    Timer_t timers[TIME_NUM_IN_VEHICLE];  //0: 配对用; 1:KTS用; 2: rke用, 3: 定位结果上送APP, 04: 定位间隔

    RKERequest_t rkeRequest_t;     //rke
    boolean privateBleConnect;  
    ChannleID_e chIdOnRanging;     //正在ranging的BLE连接通道号 

    u8 fotaFlag;         // 0: 空闲状态;  1:未在测距中，开始fota； 2:测距中，发送停止报
}VehicleInfo_t;

extern VehicleInfo_t vehicleInfo;

//----------------------------------------------------------------


#define LEN_MAX_CCC_DKMSG                        300   //(sizeof(CCCDkMsg_t))
#define LEN_MESSAGE_PAYLOAD_HEADER_AND_LENGTH    4

typedef struct 
{
    RangingSessionStatus_e sessionStatus;
    RangingResult_e rangingResult;
    Location_e lastLoc;    //上一次定位位置
    Location_e thisLoc;    //本次定位位置
    u32 rangingResultNoticeNum;  //定位结果通知次数, 假设在300次的时候，可以暂停下定位;
    u16 locFixNum;               //同一位置次数，假设在30次都在同一位置，可以暂停下定位;
    u8 rangingAnchorNum;         //Number of logical responder nodes participating in this ranging session as selected by the vehicle.
    u8 rangingFrequency;         //期望的采集频率，测距次数平均值 默认为3次测距触发一次距离上报，RangingResultNotice中
    u8 rangingAnchorInfo[2*ANCHOR_NUM_MAX];     //字节1：位置标识, 字节2：sessionStatus
    RangingAction_e rangingAction;
    u8 locRetryTime;    //定位开启后未收到定位结果时的重试次数
    u8 isLocExpired;    //是否超时未收到定位响应

    u8 pointX[2];
    u8 pointY[2];
    u8 locWithBorderType; /* 0:用车身左边;  1:用车身上边;  2:用车身右边； 3: 用车身下边 */
    u8 anchorRangingResult[4*ANCHOR_NUM_MAX];  //每三字节一组（字节1位置标识，字节2为ranging result, 后两字节表示该位置标识到FOB的距离)

    u8 uwbSessionId[4];
    u8 stsIndexInFinalData[4]; 
    u8 ursk[0x20];   

    u8 selectedDKProtocolVersion[2];
    u8 selectedUWBConfigId[2];  //Table 19-17: Definition of the parameters for Ranging_Capability_RS.
    u8 selectedPulseShapeCombo;
    u8 selectedRANMultiplier;
    u8 selectedUWBChannel;
    u8 selectedNChapPerSlot;  //Selected Slot duration as a multiple of TChap 3, 4, 6, 8,9, 12, or 24 Note that the last value is used only for testing
    u8 selectedNumSlotsPerRound; //Selected number of slots per round
    u8 selectedSyncCodeIndex; //Bitmap of SYNC code indices that the vehicle can use (this may be a smaller set of the SYNC codes support by the vehicle).
    u8 selectedHoppingConfigBitmask;  

    u8 stsIndex0[4];
    u8 uwbTime0[8];
    u8 hopModeKey[4];
}RangingSession_t;

typedef struct
{
    u8 macAddr[6];
    boolean authFlag;

    u8 apdu[262];
    u8 apduResp[261];
    u8 slotIdLV[9];
    boolean isBleConnect;

    u8 auth0P1;
    u8 auth0P2;
    u8 transactionIdentifier[16];  //auth0 input(tag=4C)

    Timer_t timers[TIME_NUM_IN_DEVICE]; 

    DeviceWorkMode_e workMode;  //手机所处工作模式
    Process_e processOn;
    u8 subProcessOn;

    u8 timeSyncTriggerConditions;   //Procedure0, procedure 1
    u16 writeOrReadTagOnParing;

    RangingSession_t rangingSession;
    u8 rssi;
    
    CCCDkMsg_t cccDkMsg_t;  //ble msg;
    u8 bleMsgBackupLV[(2+LEN_MAX_CCC_DKMSG)];   //   L(2bytes) V(ble5.0及以上蓝牙包最大长度为512字节)
}DeviceInfo_t;

extern DeviceInfo_t devicesInfo[1 + MAX_BLE_CONN_NUM];
//--------------------------------------------
CCCErrCode_e ccc_dk_init_on_reset(void);
void ccc_dk_init_on_ble_connected(u8 connId, ExtEvent_e extEvent);
u8 ccc_dk_get_ursk_num_by_slotId(u8* slotIdLV, URSKStatus_e status);
u8  ccc_dk_clean_up_prederived_ursks(u8* slotIdLV);
void ccc_dk_add_pre_derived_ursk(u8* slotIdLV, u8* sessionId, u8* ursk);
URSKStatus_e ccc_dk_find_ursk_for_ranging(u8 connId, u8* pOut_sessionId, u8* ursk);
CCCErrCode_e ccc_dk_find_ursk_to_ranging(u8 connId, boolean rangingSetUpOnPrederived);
void ccc_dk_set_ranging_session_status(u8 connId, u8* pIn_sessionId, URSKStatus_e newState);
u8 ccc_dk_get_active_ranging_session_num(void);
CCCErrCode_e ccc_dk_ble_pairing_and_encryption_setup_finsh(u8 connId);
u8 check_ttl_on_ranging(u8 connId);
CCCErrCode_e ccc_dk_rssi_update(u8 connId, s8 rssi);
u8 ccc_dk_get_index_by_locInd(LocInd_e locInd);

u8 ccc_dk_stop_ranging_on_fota(void);
u8 ccc_dk_start_ranging_after_fota(void);
#endif
