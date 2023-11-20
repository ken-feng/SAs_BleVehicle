#ifndef __CCC_DK_BLE_MSG_SE_H__
#define __CCC_DK_BLE_MSG_SE_H__

#include "../ccc_sdk/ccc_dk_type.h"
#include "ccc_dk_ble_msg.h"
typedef enum
{
	TRANSCTION_TYPE_DOOR_UNLOCK                                   = 0x01,
	TRANSCTION_TYPE_DOOR_LOCK                                     = 0x02,
	TRANSCTION_TYPE_FIRST_ENGINE_START_ON_FIRST_CONTACT           = 0x03,
	TRANSCTION_TYPE_FIRST_ENGINE_START_ON_SUBSEQUENT_CONTACT      = 0x04,
	TRANSCTION_TYPE_OTHER_AUTHENTICATION_REQUEST                  = 0x05,
	TRANSCTION_TYPE_USER_AUTHENTICATION_REQUEST                   = 0x06,
	TRANSCTION_TYPE_FIRST_STANDARD_TRANSACTION_AT_OWNER_PAIRING   = 0x07,
	TRANSCTION_TYPE_SECOND_STANDARD_TRANSACTION_AT_OWNER_PAIRING  = 0x08,
	
	TRANSCTION_TYPE_DERIVE_RANGING_KEY                            = 0x10,
	TRANSCTION_TYPE_FIRST_APPROACH_AFTER_BLE_PAIRING              = 0x11,
}TransactionCode_e;

typedef enum
{
    OP_CONTROL_FLOW_P1_CONTINUE_FLOW                  = 0x10,
    OP_CONTROL_FLOW_P1_END_FLOW                       = 0x11,
    OP_CONTROL_FLOW_P1_ABORT_FLOW                     = 0x12,
}OpControlFlowP1_e;

typedef enum
{
    //Table 5-22: OP CONTROL FLOW P2 Parameters for P1=10h (continue) 1
    OP_CONTROL_FLOW_P2_KEY_CREATION_DATA_TRANSIMITTED_TO_DEVICE          = 0x01,
    OP_CONTROL_FLOW_P2_KEY_CERTIFICATE_CHAIN_RECEIVED_BY_VEHICLE         = 0x02,
    OP_CONTROL_FLOW_P2_WAITING_FOR_USER_CONFIRMATION_ON_VEHICLE_UI       = 0x03,
    OP_CONTROL_FLOW_P2_VEHICLE_WAITING_TIME_EXTENSION_KEEP_BUSY          = 0x0F,

    //Table 5-23: OP CONTROL FLOW P2 Parameters for P1=11 (end with success)
    OP_CONTROL_FLOW_P2_SUCCESSFUL_END_OF_KEY_CREATION_AND_VERIFICATION   = 0x11,

    //Table 5-24: OP CONTROL FLOW P2 Parameters for P1=12 (end with failure)
    OP_CONTROL_FLOW_P2_NO_SPECIFIC_REASON                                = 0x00,
    OP_CONTROL_FLOW_P2_NO_MATCHING_SPAKE2_VERSION                        = 0x01,
    OP_CONTROL_FLOW_P2_NO_MATCHING_DEGITAL_KEY_APPLET_PROTOCOL_VERSION   = 0x02,
    OP_CONTROL_FLOW_P2_PAIRING_FAILED_DUE_TO_TIMEOUT_OF_TIMER1           = 0x03,
    OP_CONTROL_FLOW_P2_PAIRING_FAILED_DUE_TO_TIMEOUT_OF_TIMER2           = 0x04,
    OP_CONTROL_FLOW_P2_PAIRING_FAILED_DUE_TO_TIMEOUT_OF_TIMER3           = 0x05,
    OP_CONTROL_FLOW_P2_PAIRING_FAILED_DUE_TO_TIMEOUT_OF_TIMER4           = 0x06,
    OP_CONTROL_FLOW_P2_PAIRING_FAILED_DUE_TO_TIMEOUT_OF_TIMERX           = 0x07,
    OP_CONTROL_FLOW_P2_PRECONDITIONS_FOR_OWNER_PAIRING_NOT_FULFILLED     = 0x08,
    OP_CONTROL_FLOW_P2_EVIDENCE_VERIFICATION_ON_VEHICLE_SIDE_FAILED      = 0x09,
    OP_CONTROL_FLOW_P2_WRONG_DIGITAL_KEY_CONFIGURATION                   = 0x0A,
    OP_CONTROL_FLOW_P2_CERTIFICATE_VERIFICATION_FAILED                   = 0x0B,
    OP_CONTROL_FLOW_P2_CURVE_POINT_X_ZERO_OR_INVALID                     = 0x0C,
    OP_CONTROL_FLOW_P2_ERROR_COUNTER_LIMIT_FOR_WRONG_PAIRING_PASSWORD_IS_EXCEEDED = 0x0D,
    OP_CONTROL_FLOW_P2_PAIRING_FAILED_DUE_TO_RESPONSE_DATA_OR_FORMAT_ERR = 0x7F,
}OpControlFlowP2_e;

typedef enum
{
    CONTROL_FLOW_P1_TRANSACTION_FINISHED_WITH_FAILURE   = 0x00,
    CONTROL_FLOW_P1_TRANSACTION_FINISHED_WITH_SUCCESS   = 0x01,

    CONTROL_FLOW_P1_APPLICATION_SPECIFIC                = 0x40,
}ControlFlowP1_e;

typedef enum
{
    //with P1 = 00h, finished with failure, this range(00-7F) is used for the following vehicle error codes:
    CONTROL_FLOW_P1P2_NO_INFORMATION_PROVIDED           = 0x0000,   //with P1 = 00h, 01h, or 40h,
    CONTROL_FLOW_P1P2_PUBLIC_KEY_NOT_FOUND              = 0x0001,
    CONTROL_FLOW_P1P2_PUBLIC_KEY_EXPIRED                = 0x0002,
    CONTROL_FLOW_P1P2_PUBLIC_KEY_NOT_TRUSTED            = 0x0003,
    CONTROL_FLOW_P1P2_INVALID_SIGNATURE                 = 0x0004,
    CONTROL_FLOW_P1P2_INVALID_CHANNEL                   = 0x0005,
    CONTROL_FLOW_P1P2_INVALID_DATA_FORMAT               = 0x0006,
    CONTROL_FLOW_P1P2_INVLAID_DATA_CONTENT              = 0x0007,
    //08-7F RFU

    CONTROL_FLOW_P1P2_KEY_DELETED_IN_OR_NOT_KNOWN_TO_VEHICLE     = 0x00A0,
    CONTROL_FLOW_P1P2_CANNOT_LOCK_NOT_ALL_DOORS_TRANK_ARE_CLOSED = 0x00B1,

    //transaction finished with success p1 = 0x01
    CONTROL_FLOW_P1P2_SUCCESS_ON_STANDARD_AUTH                   = 0x0100,
    CONTROL_FLOW_P1P2_FIRST_APPROACH_SUCCESSFUL                  = 0x01B0,
    CONTROL_FLOW_P1P2_SUCCESSFUL_END_PAIRING_PHASE3              = 0x0181,
    CONTROL_FLOW_P1P2_KEY_TRACKED_WRITE_MAILBOX_SUCCESSFULLY     = 0x0190,
    CONTROL_FLOW_P1P2_KEY_NOT_TRACKED_SHARING_NOT_POSSIBLE       = 0x0191,

    //application specific p1 = 0x40,
    CONTROL_FLOW_P1P2_OPTIONAL_TOKEN_REFILL_START                = 0x4081,
    CONTROL_FLOW_P1P2_CONTINUE_ATTESTATION_PACKAGE_DELETE_START  = 0x4082,
    CONTROL_FLOW_P1P2_OPTIONAL_SERVICE_DATA_WRITE_START          = 0x4083,
    CONTROL_FLOW_P1P2_KEY_TRACKING_RESP_RECEIVED_IN_DEVICE       = 0x4088,
    CONTROL_FLOW_P1P2_KEY_TRACKING_RESP_RECEIVED_IN_CAR          = 0x4089,
    CONTROL_FLOW_P1P2_EXTEND_READER_PROCESSING_TIME              = 0x4090,
    CONTROL_FLOW_P1P2_LONG_PROCESSING_TIME                       = 0x40A0,
}ControlFLowP1P2_e;

typedef enum
{
    SW_PROTOCOL_VERSION_IS_NOT_SUPPORT                   = (short)0x6400,
	SW_INVALID_KEY_NOT_IN_SLOTLIST                       = (short)0x6991,
	SW_INCORRECT_VALUES_IN_COMMAND_DATA                  = (short)0x6A80,
	SW_NOT_ENOUGH_MEMORY                                 = (short)0x6A84,
	SW_CHANNEL_ERROR                                     = (short)0x6984,
	SW_RECEIVED_DATA_INVALID_OR_ZERO                     = (short)0x6A88,
	SW_CMAC_ERROR                                        = (short)0x6300,
	SW_REFERENCE_DATA_NOT_FOUND                          = (short)0x6A88,
	SW_CER_EXPIRED                                       = (short)0x6301,
	SW_CER_FROZEN                                        = (short)0x6302,
	SW_SECURIT_NOT_ACCESS                                = (short)0x6901,
	SW_VERIFY_FAILED                                     = (short)0x6981,
	SW_COMMAND_USED_OUT_OF_SEQUENCE                      = (short)0x6985,
	SW_MAC_ERROR                                         = (short)0x6209,
	SW_STATE_ERROR                                       = (short)0x9401,
	SW_TOKEN_ERROR                                       = (short)0x6307,
	SW_CER_CHAIN_BLOKEN                                  = (short)0x9404,
	SW_DEVICE_NOT_READY_FOR_PAIRING                      = (short)0x9484,
	SW_NO_KTS_ATTR_IN_PAIR                               = (short)0x9501,
	SW_CONDITION_NOT_SUPPORT                             = (short)0x6985,
	SW_INVALID_DK_IDENTIFIER                             = (short)0x6901,
	SW_INVALID_SCRYPT_PAR_ON_PAIRING                     = (short)0x6400,
}SeSW_e;

ControlFLowP1P2_e get_controlflow_p1p2_by_se_sw(u16 errCode);
CCCErrCode_e prepare_op_control_flow(u8 connId, u8 p1, u8 p2);
CCCErrCode_e prepare_control_flow(u8 connId, ControlFLowP1P2_e p1p2);
CCCErrCode_e prepare_and_send_select_aid_to_device_se_with_ble(u8 connId, u8 aidType);
CCCErrCode_e prepare_and_send_auth0_to_device_se_with_ble(u8 connId, u8 p1, u8 p2);
CCCErrCode_e parse_auth0_resp_and_send_auth1(u8 connId, u8* apdub, u16 apdubLen);
CCCErrCode_e parse_auth1_resp_with_vehicle_se(u8 connId, u8* apdub, u16 apdubLen);
CCCErrCode_e prepare_and_send_mailbox_operation_inquire(u8 connId, u8* apduForDevice, u16* apduForDeviceLen);
CCCErrCode_e prepare_and_send_create_ranging_key_to_device_with_ble(u8 connId, u8* arbitraryData, u8 arbitraryDataLen);

CCCErrCode_e ccc_dk_select_aid_to_device_in_standard_transaction(u8 connId);

CCCErrCode_e msg_dk_se_message_process(u8 connId, CCCDkMsg_t * cccDkMsg_t);
#endif