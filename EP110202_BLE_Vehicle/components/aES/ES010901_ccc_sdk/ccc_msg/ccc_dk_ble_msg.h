
#ifndef __CCC_DK_BLE_MSG_H__
#define __CCC_DK_BLE_MSG_H__
#include "../ccc_sdk/ccc_dk_type.h"
//#include "ccc_dk.h"

typedef enum{
    DK_MESSAGE_TYPE_FRAMEWORK = 0,
    DK_MESSAGE_TYPE_SE,
    DK_MESSAGE_TYPE_UWB_RANGING_SERVICE,
    DK_MESSAGE_TYPE_DK_EVENT_NOTIFICATION,
    DK_MESSAGE_TYPE_VEHICLE_OEM_APP,
    DK_MESSAGE_TYPE_SUPPLEMENTARY_SERVICE = 5,
    DK_MESSAGE_TYPE_HEAD_UNIT_PAIRING,
    DK_MESSAGE_TYPE_RFU,
}DKMessageType_e;

typedef enum{
    DK_MESSAGE_ID_RANGING_CAPABILITY_RQ        = 0x01,
    DK_MESSAGE_ID_RANGING_CAPABILITY_RS,
    DK_MESSAGE_ID_RANGING_SESSION_RQ,
    DK_MESSAGE_ID_RANGING_SESSION_RS,
    DK_MESSAGE_ID_RANGING_SESSION_SETUP_RQ,
    DK_MESSAGE_ID_RANGING_SESSION_SETUP_RS,
    DK_MESSAGE_ID_RANGING_SUSPEND_RQ,
    DK_MESSAGE_ID_RANGING_SUSPEND_RS,
    DK_MESSAGE_ID_RANGING_RECOVERY_RQ,
    DK_MESSAGE_ID_RANGING_RECOVERY_RS,

    DK_MESSAGE_ID_APDU_RQ                      = 0x0B,
    DK_MESSAGE_ID_APDU_RS                      = 0x0C,

    DK_MESSAGE_ID_TIME_SYNC                    = 0x0D,          //device -> vehicle

    DK_MESSAGE_ID_FIRST_APPROACH_RQ            = 0x0E,   //device -> vehicle
    DK_MESSAGE_ID_FIRST_APPROACH_RS            = 0x0F,   //vehicle -> device

    DK_MESSAGE_ID_PASS_THROUTH                 = 0x10,  //vehicle <-> device

    DK_MESSAGE_ID_EVENT_NOTIFICATION           = 0x11,  //vehicle <-> device

    DK_MESSAGE_ID_RANGING_CONFIGURABLE_RECOVERY_RQ = 0x12,
    DK_MESSAGE_ID_RANGING_CONFIGURABLE_RECOVERY_RS,

    DK_MESSAGE_ID_RKE_AUTH_RQ                      = 0x14,       //vehicle -> device
    DK_MESSAGE_ID_RKE_AUTH_RS                      = 0x15,       //device -> vehicle

    DK_MESSAGE_ID_HEAD_UNIT_PAIRING_PREPARATION    = 0x16,       //

    DK_MESSAGE_ID_HEAD_UNIT_PAIRING_RQ             = 0x17,       //device -> vehicle
    DK_MESSAGE_ID_HEAD_UNIT_PAIRING_RS             = 0x18,       //device -> vehicle
}DKMessageID_e;

typedef enum
{
    SubEvent_Category_Command_Complete = 0x01,
    SubEvent_Category_Ranging_Session_Status_Changed,
    SubEvent_Category_Device_Ranging_Intent,
    SubEvent_Category_Vehicle_Status_Change,
    SubEvent_Category_RKE_Request = 0x05,
    SubEvent_Category_Head_Unit_Pairing,
}DKSubEventCategory_e;

//--------------------------------------------------------------------------------------------------
//List of Command_Status for Command Complete SubEvent
typedef enum
{
    Command_Status_Deselect_SE = 0x00,    // vehicle -> device
    Command_Status_BLE_pairing_ready,
    Command_Status_Require_capability_exchange,
    Command_Status_Request_standard_transaction,
    Command_Status_Request_owner_pairing,

    Command_Status_General_error = 0x80,
    Command_Status_Device_SE_busy = 0x81,
    Command_Status_Device_SE_transaction_state_lost,
    Command_Status_Device_busy,
    Command_Status_Command_temporarily_blocked,
    Command_Status_Unsupported_channel_bitmask,
    Command_Status_OP_Device_not_inside_vehicle,

    Command_Status_OOB_mismatch = 0xFC,
    Command_Status_BLE_pairing_failed = 0xFD,
    Command_Status_FA_crypto_operation_failed = 0xFE,
    Command_Status_Wrong_parameters = 0xFF,
}CommandStatusCode_e;

//List of Session_Status for Ranging Session Status Changed SubEvent
typedef enum
{
    Session_Status_Ranging_session_URSK_refresh           = 0x00,
    Session_Status_Ranging_session_URSK_not_found         = 0x01,
    Ranging_session_not_required                          = 0x02,
    Session_Status_Ranging_session_secure_ranging_failed  = 0x03,
    Session_Status_Ranging_session_terminated             = 0x04,
    Session_Status_Ranging_session_recovery_failed        = 0x06,

    Session_status_Ranging_session_suspended              = 0x07,  /*When vehicle or device receives this, it shall suspend its current ranging 
        session as well without sending a Ranging_Suspend_RQ or a Ranging_session_suspended SubEvent. */

    //RFU自定义状态字
    Session_Status_Ranging_session_conflict,   /**已有一个在定位中**/
}SessionStatusCode_e;

typedef enum
{
    DR_Intent_Low_approach_confidence = 0x00,
    DR_Intent_Medium_approach_confidence = 0x01,
    DR_Intent_High_approach_confidence = 0x02,
}DRIntentCode_e;

//Table 19-68: List of Vehicle Status Changed SubEvent Tags.
typedef enum
{
    FUNCTION_SUCCESSFULLY_EXECUTED = 0,
    EXECUTION_STARTED,
    EXECUTION_STOPPED_SUCCESSFULLY, //(upon user stop request)
    EXECUTION_ENDED, //(e.g., limit reached)

    EXECUTION_NOT_POSSIBLE_OR_CANCELLED = 0x10,
    EXECUTION_NOT_POSSIBLE_DUE_TO_VEHICLE_STATE,
    AUTHENTICATION_ERROR,
    DEVICE_RESPONSE_TIMEOUT,
    DEVICE_OUT_OF_EXECUTION_RANGE,

    //50h–EFh: Execution not possible or cancelled with OEM specific meaning. 
    //The meaning of these unsuccessful status codes is proprietary and may be used to 
    //send a more specific error reason to the vehicle OEM app. The definition is up to the vehicle OEM.
    VEHICLE_RESPONSE_TIMEOUT = 0x50,
    DEVICE_CONTINUE_CONFIRM_DATA_ERR = 0x51,

    //Table 19-70: Definition of Standardized Function Status Values to Indicate the (Temporary) Unavailability of Function or Errors.
    Function_Status_Function_not_supported_by_vehicle = 0xF0,
    Function_Status_Action_not_supported_by_vehicle = 0xF1,
    Function_Status_Function_temporarily_not_available = 0xF2,
    Function_Status_Action_temporarily_not_available = 0xF3,
    Function_Status_Function_not_purchased = 0xF4,
    Function_Status_Action_not_purchased = 0xF5,
    Function_Status_Insufficient_entitlements = 0xF6,
    Function_Status_Use_of_wrong_RKE_security_policy_for_requested_action = 0xF7,
    Function_Status_Generic_error = 0xFE,
    Function_Status_no_explicit_status = 0xFF,
}ExecutionStatus_e;


typedef enum
{
    FUNCTION_ID_CENTRAL_LOCKING = 0x01,
    FUNCTION_ID_LOCK_AND_SECURE,

    FUNCTION_ID_DRIVING_READINESS = 0x10,
    FUNCTION_ID_VEHICLE_LOW_POWER_MODE = 0x11,
    FUNCTION_ID_LOW_FUEL_STATUS = 0x12,

    FUNCTION_ID_REMOTE_CLIMATIZATION = 0x0100,
    FUNCTION_ID_PANIC_ALARM,
    FUNCTION_ID_FUEL_LID,

    FUNCTION_ID_MANUAL_TRUNK_DECKLID_LIFTGATE_TAILGATE_CONTROLS = 0x0110,
    FUNCTION_ID_POWER_TRUNK_DECKLID_LIFTGATE_TAILGATE_CONTROLS_WITH_CONFIRM = 0x0111,
    FUNCTION_ID_POWER_TRUNK_DECKLID_LIFTGATE_TAILGATE_CONTROLS_WITHOUT_CONFIRM = 0x0112,

    FUNCTION_ID_MANUAL_FRUNK = 0x0120,
    FUNCTION_ID_POWER_FRUNK_WITH_CONFIRM = 0x0121,
    FUNCTION_ID_POWER_GRUNK_WITHOUT_CONFIRM = 0x0122,

    FUNCTION_ID_POWER_FRONT_LEFT_WITH_CONFIRM = 0x0130,
    FUNCTION_ID_POWER_FRONT_LEFT_WITHOUT_CONFIRM = 0x0131,
    FUNCTION_ID_POWER_FRONT_RIGHT_WITH_CONFIRM = 0x0132,
    FUNCTION_ID_POWER_FRONT_RIGHT_WITHOUT_CONFIRM = 0x0133,
    FUNCTION_ID_POWER_REAR_LEFT_WITH_CONFIRM = 0x0134,
    FUNCTION_ID_POWER_REAR_LEFT_WITHOUT_CONFIRM = 0x0135,
    FUNCTION_ID_POWER_REAR_RIGHT_WITH_CONFIRM = 0x0136,
    FUNCTION_ID_POWER_REAR_RIGHT_WITHOUT_CONFIRM = 0x0137,

    FUNCTION_ID_POWER_WINDOWS = 0x0140,
    FUNCTION_ID_POWER_ROOF = 0x0141,
}FunctionID_e;

typedef enum
{
    ACTION_TYPE_FUNCTION_NOT_SUPPORT = 0,
    
    ACTION_TYPE_EVENT,
    ACTION_TYPE_EXECUTION,
    ACTION_TYPE_EUDURING_WITHOUT_CONFIRMATION,
    ACTION_TYPE_EUDURING_WITH_CONFIRMATION,

    ACTION_TYPE_ACTION_NOT_SUPPORT = 0xFF,
}ActionType_e;

typedef enum
{
    CCC_TAG_SEQUENCE = 0x30,
    CCC_TAG_FUNCTION_ID = 0x80,
    CCC_TAG_ACTION_ID   = 0x81,
    CCC_TAG_EXE_STATUS = 0x82,
    CCC_TAG_FUNCTION_STATUS = 0x83,

    CCC_TAG_ARBITRARY = 0x87,

    CCC_TAG_VEHICLE_FUNCTION_STATUS = 0xA0,

    CCC_TAG_VEHICLE_OEM_PROPRIETARY_DATA = 0xC0,

    CCC_TAG_LAST_REQUESTED_ACTION_EXE_STATUS = 0x7F71,
    CCC_TAG_VEHICLE_FUNCTION_STATUS_SUMMARY = 0x7F72,

    CCC_TAG_REQUEST_CONFIRMATION_FOR_ENDURING_RKE = 0x7F75,

    CCC_TAG_SUBSCRIPTION_STATUS_CHANGED = 0x5F78,
}CCCTag_e;

#pragma pack(1)  //align to 1 byte
typedef struct
{
    union
    {
        u8 message_header;

        struct
        {
            u8 message_type:5;
            u8 type:3;
        }messageHeaderB_t;
    }MessageHeader_u;

    u8 payload_header_MsgId;
    u8 length[2];
    u8* data;  //[512];
}CCCDkMsg_t;
#pragma pack()

CCCErrCode_e ccc_dk_msg_process(ChannleID_e connId, u8* p_cccDkMsg, u16 len_cccDKMsg);
void ccc_dk_msg_prepare_and_send(u8 connId, u8 messageHeader, u8 payloadHeader, u8* data, u16 len);
u16 ccc_dk_msg_get_data_len(CCCDkMsg_t* cccDkMsg_t);

//-------------------------------extern function---------------------------------


#endif
