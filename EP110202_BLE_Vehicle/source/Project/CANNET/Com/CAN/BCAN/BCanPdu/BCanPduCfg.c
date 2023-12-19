/* @Encoding: UTF-8 */
/*!
 ********************************************************************
 * @file   : BCanPduCfg.c
 * @brief  : Configuration APIs for the BCanPdu module.
 * @author : J.L.
 * @version: A.0.1
 * @release: @currentDateTime
 ********************************************************************
 *                      Description/Information
 * 1.
 * 2.
 ********************************************************************
 */
/* Includes*********************************************************/
#include <BCanPduCfg.h>
#include "app_RTE.h"
#include "ble_ccc.h"
#include "UDS_App.h"
/*Private Interface Forward Declaration*****************************/
/*!
 * \defgroup BLE 发送报文处理函数
 * \{
 */
static void BCanPdu_TxHandler_BLE_0x100(CanPduTxStatus_t status);
static void BCanPdu_TxHandler_BLE_0x101(CanPduTxStatus_t status);
static void BCanPdu_TxHandler_BLE_0x102(CanPduTxStatus_t status);
static void BCanPdu_TxHandler_BLE_0x103(CanPduTxStatus_t status);
static void BCanPdu_TxHandler_BLE_0x104(CanPduTxStatus_t status);
static void BCanPdu_TxHandler_BLE_0x133(CanPduTxStatus_t status);
static void BCanPdu_TxHandler_BLE_0x180(CanPduTxStatus_t status);
static void BCanPdu_TxHandler_BLE_0x181(CanPduTxStatus_t status);

static void BCanPdu_TxHandler_OBD_0x639(CanPduTxStatus_t status);
//Modify (Ken):VEHICLE-V0C02 NO.1 -20231218
#if defined __FIT_Aeon_H
static void BCanPdu_TxHandler_OBD_0x061(CanPduTxStatus_t status);
#endif
/*!
 * \defgroup SA 发送报文处理函数
 * \{
 */
// static void BCanPdu_RxHandler_SA_0x200(CanPduRxStatus_t status);
// static void BCanPdu_RxHandler_SA_0x201(CanPduRxStatus_t status);
// static void BCanPdu_RxHandler_SA_0x202(CanPduRxStatus_t status);
// static void BCanPdu_RxHandler_SA_0x203(CanPduRxStatus_t status);
static void BCanPdu_RxHandler_SA_0x631(CanPduRxStatus_t status);


static void BCanPdu_RxHandler_SA_0x210(CanPduRxStatus_t status);
static void BCanPdu_RxHandler_SA_0x220(CanPduRxStatus_t status);
static void BCanPdu_RxHandler_SA_0x230(CanPduRxStatus_t status);
static void BCanPdu_RxHandler_SA_0x240(CanPduRxStatus_t status);
static void BCanPdu_RxHandler_SA_0x250(CanPduRxStatus_t status);
static void BCanPdu_RxHandler_SA_0x260(CanPduRxStatus_t status);


static void BCanPdu_RxHandler_SA_0x211(CanPduRxStatus_t status);
static void BCanPdu_RxHandler_SA_0x221(CanPduRxStatus_t status);
static void BCanPdu_RxHandler_SA_0x231(CanPduRxStatus_t status);
static void BCanPdu_RxHandler_SA_0x241(CanPduRxStatus_t status);
static void BCanPdu_RxHandler_SA_0x251(CanPduRxStatus_t status);
static void BCanPdu_RxHandler_SA_0x261(CanPduRxStatus_t status);


static void BCanPdu_RxHandler_SA_0x212(CanPduRxStatus_t status);
static void BCanPdu_RxHandler_SA_0x222(CanPduRxStatus_t status);
static void BCanPdu_RxHandler_SA_0x232(CanPduRxStatus_t status);
static void BCanPdu_RxHandler_SA_0x242(CanPduRxStatus_t status);
static void BCanPdu_RxHandler_SA_0x252(CanPduRxStatus_t status);
static void BCanPdu_RxHandler_SA_0x262(CanPduRxStatus_t status);


static void BCanPdu_RxHandler_SA_0x213(CanPduRxStatus_t status);
static void BCanPdu_RxHandler_SA_0x223(CanPduRxStatus_t status);
static void BCanPdu_RxHandler_SA_0x233(CanPduRxStatus_t status);
static void BCanPdu_RxHandler_SA_0x243(CanPduRxStatus_t status);
static void BCanPdu_RxHandler_SA_0x253(CanPduRxStatus_t status);
static void BCanPdu_RxHandler_SA_0x263(CanPduRxStatus_t status);

static void BCanPdu_RxHandler_SA_0x2A0(CanPduRxStatus_t status);
static void BCanPdu_RxHandler_SA_0x2A1(CanPduRxStatus_t status);


static void BCanPdu_RxHandler_ODB_0x58(CanPduRxStatus_t status);
static void BCanPdu_RxHandler_ODB_0x59(CanPduRxStatus_t status);
//Modify (Ken):VEHICLE-V0C02 NO.1 -20231218
#if defined __FIT_Aeon_H
static void BCanPdu_RxHandler_OBD_0x60(CanPduRxStatus_t status);
#endif

#ifdef FIT_DEBUG_NO_SA 
static void BCanPdu_RxHandler_UWB_0x313(CanPduRxStatus_t status);
static void BCanPdu_RxHandler_UWB_0x323(CanPduRxStatus_t status);
static void BCanPdu_RxHandler_UWB_0x333(CanPduRxStatus_t status);
static void BCanPdu_RxHandler_UWB_0x343(CanPduRxStatus_t status);
static void BCanPdu_RxHandler_UWB_0x353(CanPduRxStatus_t status);
static void BCanPdu_RxHandler_UWB_0x363(CanPduRxStatus_t status);
#endif
/*Variable Definitions**********************************************/
/*!
 * \defgroup DKM周期发送报文定义
 * \{
 */
BCanPdu_BLE_0x100_t g_BCanPdu_BLE_0x100 = {{0x00}};   
BCanPdu_BLE_0x101_t g_BCanPdu_BLE_0x101 = {{0x00}};  
BCanPdu_BLE_0x102_t g_BCanPdu_BLE_0x102 = {{0x00}};  
BCanPdu_BLE_0x103_t g_BCanPdu_BLE_0x103 = {{0x00}};  
BCanPdu_BLE_0x104_t g_BCanPdu_BLE_0x104 = {{0x00}}; 
BCanPdu_BLE_0x133_t g_BCanPdu_BLE_0x133 = {{0x00}};   
BCanPdu_BLE_0x180_t g_BCanPdu_BLE_0x180 = {{0x00}};  
BCanPdu_BLE_0x181_t g_BCanPdu_BLE_0x181 = {{0x00}};  


// BCanPdu_SA_0x200_t g_BCanPdu_SA_0x200 = {{0x00}};  
// BCanPdu_SA_0x201_t g_BCanPdu_SA_0x201 = {{0x00}};   
// BCanPdu_SA_0x202_t g_BCanPdu_SA_0x202 = {{0x00}};  
// BCanPdu_SA_0x203_t g_BCanPdu_SA_0x203 = {{0x00}};  

BCanPdu_SA_0x210_t g_BCanPdu_SA_0x210 = {{0x00}};  
BCanPdu_SA_0x220_t g_BCanPdu_SA_0x220 = {{0x00}};   
BCanPdu_SA_0x230_t g_BCanPdu_SA_0x230 = {{0x00}};  
BCanPdu_SA_0x240_t g_BCanPdu_SA_0x240 = {{0x00}};  
BCanPdu_SA_0x250_t g_BCanPdu_SA_0x250 = {{0x00}};  
BCanPdu_SA_0x260_t g_BCanPdu_SA_0x260 = {{0x00}};  


BCanPdu_SA_0x211_t g_BCanPdu_SA_0x211 = {{0x00}};  
BCanPdu_SA_0x221_t g_BCanPdu_SA_0x221 = {{0x00}};   
BCanPdu_SA_0x231_t g_BCanPdu_SA_0x231 = {{0x00}};  
BCanPdu_SA_0x241_t g_BCanPdu_SA_0x241 = {{0x00}};  
BCanPdu_SA_0x251_t g_BCanPdu_SA_0x251 = {{0x00}};  
BCanPdu_SA_0x261_t g_BCanPdu_SA_0x261 = {{0x00}}; 


BCanPdu_SA_0x212_t g_BCanPdu_SA_0x212 = {{0x00}};  
BCanPdu_SA_0x222_t g_BCanPdu_SA_0x222 = {{0x00}};   
BCanPdu_SA_0x232_t g_BCanPdu_SA_0x232 = {{0x00}};  
BCanPdu_SA_0x242_t g_BCanPdu_SA_0x242 = {{0x00}};  
BCanPdu_SA_0x252_t g_BCanPdu_SA_0x252 = {{0x00}};  
BCanPdu_SA_0x262_t g_BCanPdu_SA_0x262 = {{0x00}};  


BCanPdu_SA_0x213_t g_BCanPdu_SA_0x213 = {{0x00}};  
BCanPdu_SA_0x223_t g_BCanPdu_SA_0x223 = {{0x00}};   
BCanPdu_SA_0x233_t g_BCanPdu_SA_0x233 = {{0x00}};  
BCanPdu_SA_0x243_t g_BCanPdu_SA_0x243 = {{0x00}};  
BCanPdu_SA_0x253_t g_BCanPdu_SA_0x253 = {{0x00}};  
BCanPdu_SA_0x263_t g_BCanPdu_SA_0x263 = {{0x00}};  

BCanPdu_SA_0x2A0_t g_BCanPdu_SA_0x2A0 = {{0x00}};  
BCanPdu_SA_0x2A1_t g_BCanPdu_SA_0x2A1 = {{0x00}}; 

BCanPdu_ODB_0x58_t g_BCanPdu_ODB_0x58 = {{0x00}};  
BCanPdu_ODB_0x59_t g_BCanPdu_ODB_0x59 = {{0x00}};  
//Modify (Ken):VEHICLE-V0C02 NO.1 -20231218
#if defined __FIT_Aeon_H
BCanPdu_OBD_0x60_t g_BCanPdu_OBD_0x60 = {{0x00}};
BCanPdu_OBD_0x61_t g_BCanPdu_OBD_0x61 = {{0x00}};
#endif

#ifdef FIT_DEBUG_NO_SA 
BCanPdu_UWB_0x313_t g_BCanPdu_UWB_0x313 = {{0x00}};  
BCanPdu_UWB_0x323_t g_BCanPdu_UWB_0x323 = {{0x00}};   
BCanPdu_UWB_0x333_t g_BCanPdu_UWB_0x333 = {{0x00}};  
BCanPdu_UWB_0x343_t g_BCanPdu_UWB_0x343 = {{0x00}}; 
BCanPdu_UWB_0x353_t g_BCanPdu_UWB_0x353 = {{0x00}};  
BCanPdu_UWB_0x363_t g_BCanPdu_UWB_0x363 = {{0x00}};  
#endif


BCanPdu_SA_0x631_t g_BCanPdu_SA_0x631 = {{0x00}};  

BCanPdu_OBD_0x639_t g_BCanPdu_OBD_0x639 = {{0x00}};  
/*Variable Definitions**********************************************/

/*!
 * \defgroup BCAN 发送 PDU 配置列表定义
 * \{
 */
CanTxPduHandle_t g_BCanTxPduList[BCANPDU_N_TX] =
{
    { /* 1 */
    	CAN_EVENT_MSG, 0x639, g_BCanPdu_OBD_0x639.Data,
        64, 0, 0, FALSE, 20, 20, FALSE, 1, 0, 
        CANPDU_TX_CLEAR_STATUS, CANPDU_TX_READY, BCanPdu_TxHandler_OBD_0x639
    },
    { /* 2 */
    	CAN_EVENT_MSG, 0x100, g_BCanPdu_BLE_0x100.Data,
        64, 0, 0, FALSE, 20, 20, FALSE, 1, 0, 
        CANPDU_TX_CLEAR_STATUS, CANPDU_TX_READY, BCanPdu_TxHandler_BLE_0x100
    },
    { /* 3 */
    	CAN_EVENT_MSG, 0x101, g_BCanPdu_BLE_0x101.Data,
        64, 0, 0, FALSE, 20, 20, FALSE, 1, 0, 
        CANPDU_TX_CLEAR_STATUS, CANPDU_TX_READY, BCanPdu_TxHandler_BLE_0x101
    },
    { /* 4 */
    	CAN_EVENT_MSG, 0x102, g_BCanPdu_BLE_0x102.Data,
        64, 0, 0, FALSE, 20, 20, FALSE, 1, 0, 
        CANPDU_TX_CLEAR_STATUS, CANPDU_TX_READY, BCanPdu_TxHandler_BLE_0x102
    },
    { /* 5 */
    	CAN_EVENT_MSG, 0x103, g_BCanPdu_BLE_0x103.Data,
        64, 0, 0, FALSE, 20, 20, FALSE, 1, 0, 
        CANPDU_TX_CLEAR_STATUS, CANPDU_TX_READY, BCanPdu_TxHandler_BLE_0x103
    },
    { /* 6 */
    	CAN_EVENT_MSG, 0x104, g_BCanPdu_BLE_0x104.Data,
        64, 0, 0, FALSE, 20, 20, FALSE, 1, 0, 
        CANPDU_TX_CLEAR_STATUS, CANPDU_TX_READY, BCanPdu_TxHandler_BLE_0x104
    },
    { /* 7 */
    	CAN_EVENT_MSG, 0x133, g_BCanPdu_BLE_0x133.Data,
        64, 0, 0, FALSE, 20, 20, FALSE, 1, 0, 
        CANPDU_TX_CLEAR_STATUS, CANPDU_TX_READY, BCanPdu_TxHandler_BLE_0x133
    },
    { /* 8 */
    	CAN_EVENT_MSG, 0x180, g_BCanPdu_BLE_0x180.Data,
        64, 0, 0, FALSE, 20, 20, FALSE, 1, 0, 
        CANPDU_TX_CLEAR_STATUS, CANPDU_TX_READY, BCanPdu_TxHandler_BLE_0x180
    },
    { /* 9 */
    	CAN_EVENT_MSG, 0x181, g_BCanPdu_BLE_0x181.Data,
        64, 0, 0, FALSE, 20, 20, FALSE, 1, 0, 
        CANPDU_TX_CLEAR_STATUS, CANPDU_TX_READY, BCanPdu_TxHandler_BLE_0x181
    },
    //Modify (Ken):VEHICLE-V0C02 NO.1 -20231218
    #if defined __FIT_Aeon_H
    { /* 10 */
		CAN_EVENT_MSG, 0x061, g_BCanPdu_OBD_0x61.Data,
        64, 0, 0, FALSE, 20, 20, FALSE, 1, 0,
        CANPDU_TX_CLEAR_STATUS, CANPDU_TX_READY, BCanPdu_TxHandler_OBD_0x061
    }
    #endif
};
/*!
 * \}
 */
/*!
 * \}
 */

/*!
 * \defgroup BCAN 接收 PDU 配置列表定义
 * \{
 */
CanRxPduHandle_t g_BCanRxPduList[BCANPDU_N_RX] =
{
    { /* 1 */
        CAN_EVENT_MSG, 0x631, g_BCanPdu_SA_0x631.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_SA_0x631,   NULL_PTR
    },
    /* BCM 周期报文 */
    // { /* 1 */
    //     CAN_EVENT_MSG, 0x200, g_BCanPdu_SA_0x200.Data,  
	//     64, TRUE, FALSE, 20,  0, FALSE,
	//     1, 0, CANPDU_RX_LOST_STATUS,
	//     BCanPdu_RxHandler_SA_0x200,   NULL_PTR
    // },
    // { /* 2 */
    //     CAN_EVENT_MSG, 0x201, g_BCanPdu_SA_0x201.Data,  
	//     64, TRUE, FALSE, 20,  0, FALSE,
	//     1, 0, CANPDU_RX_LOST_STATUS,
	//     BCanPdu_RxHandler_SA_0x201,   NULL_PTR
    // },
    // { /* 3 */
    //     CAN_EVENT_MSG, 0x202, g_BCanPdu_SA_0x202.Data,  
	//     64, TRUE, FALSE, 20,  0, FALSE,
	//     1, 0, CANPDU_RX_LOST_STATUS,
	//     BCanPdu_RxHandler_SA_0x202,   NULL_PTR
    // },
    // { /* 4 */
    //     CAN_EVENT_MSG, 0x203, g_BCanPdu_SA_0x203.Data,  
	//     64, TRUE, FALSE, 20,  0, FALSE,
	//     1, 0, CANPDU_RX_LOST_STATUS,
	//     BCanPdu_RxHandler_SA_0x203,   NULL_PTR
    // },
    { /* 1 */
        CAN_EVENT_MSG, 0x210, g_BCanPdu_SA_0x210.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_SA_0x210,   NULL_PTR
    },
    { /* 2 */
        CAN_EVENT_MSG, 0x220, g_BCanPdu_SA_0x220.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_SA_0x220,   NULL_PTR
    },
    { /* 3 */
        CAN_EVENT_MSG, 0x230, g_BCanPdu_SA_0x230.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_SA_0x230,   NULL_PTR
    },
    { /* 4 */
        CAN_EVENT_MSG, 0x240, g_BCanPdu_SA_0x240.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_SA_0x240,   NULL_PTR
    },
    { /* 5 */
        CAN_EVENT_MSG, 0x250, g_BCanPdu_SA_0x250.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_SA_0x250,   NULL_PTR
    },
    { /* 6 */
        CAN_EVENT_MSG, 0x260, g_BCanPdu_SA_0x260.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_SA_0x260,   NULL_PTR
    },
    { /* 7 */
        CAN_EVENT_MSG, 0x211, g_BCanPdu_SA_0x211.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_SA_0x211,   NULL_PTR
    },
    { /* 8 */
        CAN_EVENT_MSG, 0x221, g_BCanPdu_SA_0x221.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_SA_0x221,   NULL_PTR
    },
    { /* 9 */
        CAN_EVENT_MSG, 0x231, g_BCanPdu_SA_0x231.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_SA_0x231,   NULL_PTR
    },
    { /* 10 */
        CAN_EVENT_MSG, 0x241, g_BCanPdu_SA_0x241.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_SA_0x241,   NULL_PTR
    },
    { /* 11 */
        CAN_EVENT_MSG, 0x251, g_BCanPdu_SA_0x251.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_SA_0x251,   NULL_PTR
    },
    { /* 12 */
        CAN_EVENT_MSG, 0x261, g_BCanPdu_SA_0x261.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_SA_0x261,   NULL_PTR
    },
    { /* 13 */
        CAN_EVENT_MSG, 0x212, g_BCanPdu_SA_0x212.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_SA_0x212,   NULL_PTR
    },
    { /* 14 */
        CAN_EVENT_MSG, 0x222, g_BCanPdu_SA_0x222.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_SA_0x222,   NULL_PTR
    },
    { /* 15 */
        CAN_EVENT_MSG, 0x232, g_BCanPdu_SA_0x232.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_SA_0x232,   NULL_PTR
    },
    { /* 16 */
        CAN_EVENT_MSG, 0x242, g_BCanPdu_SA_0x242.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_SA_0x242,   NULL_PTR
    },
    { /* 17 */
        CAN_EVENT_MSG, 0x252, g_BCanPdu_SA_0x252.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_SA_0x252,   NULL_PTR
    },
    { /* 18 */
        CAN_EVENT_MSG, 0x262, g_BCanPdu_SA_0x262.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_SA_0x262,   NULL_PTR
    },
    { /* 19 */
        CAN_EVENT_MSG, 0x213, g_BCanPdu_SA_0x213.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_SA_0x213,   NULL_PTR
    },
    { /* 20 */
        CAN_EVENT_MSG, 0x223, g_BCanPdu_SA_0x223.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_SA_0x223,   NULL_PTR
    },
    { /* 21 */
        CAN_EVENT_MSG, 0x233, g_BCanPdu_SA_0x233.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_SA_0x233,   NULL_PTR
    },
    { /* 22 */
        CAN_EVENT_MSG, 0x243, g_BCanPdu_SA_0x243.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_SA_0x243,   NULL_PTR
    },
    { /* 23 */
        CAN_EVENT_MSG, 0x253, g_BCanPdu_SA_0x253.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_SA_0x253,   NULL_PTR
    },
    { /* 24 */
        CAN_EVENT_MSG, 0x263, g_BCanPdu_SA_0x263.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_SA_0x263,   NULL_PTR
    },
    { /* 25 */
        CAN_EVENT_MSG, 0x58, g_BCanPdu_ODB_0x58.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_ODB_0x58,   NULL_PTR
    }, 
    { /* 26 */
        CAN_EVENT_MSG, 0x59, g_BCanPdu_ODB_0x59.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_ODB_0x59,   NULL_PTR
    },   
    { /* 27 */
        CAN_EVENT_MSG, 0x2A0, g_BCanPdu_SA_0x2A0.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_SA_0x2A0,   NULL_PTR
    },
    { /* 28 */
        CAN_EVENT_MSG, 0x2A1, g_BCanPdu_SA_0x2A1.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_SA_0x2A1,   NULL_PTR
    },

	//Modify (Ken):VEHICLE-V0C02 NO.1 -20231218
	#if defined __FIT_Aeon_H
    { /* 29 */
        CAN_EVENT_MSG, 0x60, g_BCanPdu_OBD_0x60.Data,
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_OBD_0x60,   NULL_PTR
    },
	#endif
#ifdef FIT_DEBUG_NO_SA    
    { /* 27 */
        CAN_EVENT_MSG, 0x313, g_BCanPdu_UWB_0x313.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_UWB_0x313,   NULL_PTR
    },
    { /* 28 */
        CAN_EVENT_MSG, 0x323, g_BCanPdu_UWB_0x323.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_UWB_0x323,   NULL_PTR
    },
    { /* 29 */
        CAN_EVENT_MSG, 0x333, g_BCanPdu_UWB_0x333.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_UWB_0x333,   NULL_PTR
    }, 
    { /* 30 */
        CAN_EVENT_MSG, 0x343, g_BCanPdu_UWB_0x343.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_UWB_0x343,   NULL_PTR
    },   
    { /* 31 */
        CAN_EVENT_MSG, 0x353, g_BCanPdu_UWB_0x353.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_UWB_0x353,   NULL_PTR
    }, 
    { /* 32 */
        CAN_EVENT_MSG, 0x363, g_BCanPdu_UWB_0x363.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_UWB_0x363,   NULL_PTR
    },         
#endif  

};
/*!
 * \}
 */
static void BCanPdu_TxHandler_BLE_0x100(CanPduTxStatus_t status)
{
    switch (status) {
    	case CANPDU_TX_CLEAR_STATUS:
            BCanPdu_ClearTxPdu(0x100);
            break;
        case CANPDU_TX_UPDATE_STATUS:
            /*收到数据通知*/
            break;
			
        default:
            break;
    }
}
static void BCanPdu_TxHandler_BLE_0x101(CanPduTxStatus_t status)
{
    switch (status) {
    	case CANPDU_TX_CLEAR_STATUS:
            BCanPdu_ClearTxPdu(0x101);
            break;
        case CANPDU_TX_UPDATE_STATUS:
        
            break;
			
        default:
            break;
    }
}
static void BCanPdu_TxHandler_BLE_0x102(CanPduTxStatus_t status)
{
    switch (status) {
    	case CANPDU_TX_CLEAR_STATUS:
            BCanPdu_ClearTxPdu(0x102);
            break;
        case CANPDU_TX_UPDATE_STATUS:
        
            break;
			
        default:
            break;
    }
}
static void BCanPdu_TxHandler_BLE_0x103(CanPduTxStatus_t status)
{
    switch (status) {
    	case CANPDU_TX_CLEAR_STATUS:
            BCanPdu_ClearTxPdu(0x103);
            break;
        case CANPDU_TX_UPDATE_STATUS:
        
            break;
			
        default:
            break;
    }
}
static void BCanPdu_TxHandler_BLE_0x104(CanPduTxStatus_t status)
{
    switch (status) {
    	case CANPDU_TX_CLEAR_STATUS:
            BCanPdu_ClearTxPdu(0x104);
            break;
        case CANPDU_TX_UPDATE_STATUS:
        
            break;
			
        default:
            break;
    }
}

static void BCanPdu_TxHandler_BLE_0x133(CanPduTxStatus_t status)
{
    switch (status) {
    	case CANPDU_TX_CLEAR_STATUS:
            BCanPdu_ClearTxPdu(0x133);
            break;
        case CANPDU_TX_UPDATE_STATUS:
        
            break;
			
        default:
            break;
    }
}

static void BCanPdu_TxHandler_BLE_0x180(CanPduTxStatus_t status)
{
    switch (status) {
    	case CANPDU_TX_CLEAR_STATUS:
            BCanPdu_ClearTxPdu(0x180);
            break;
        case CANPDU_TX_UPDATE_STATUS:
        
            break;
			
        default:
            break;
    }
}
static void BCanPdu_TxHandler_BLE_0x181(CanPduTxStatus_t status)
{
    switch (status) {
    	case CANPDU_TX_CLEAR_STATUS:
            BCanPdu_ClearTxPdu(0x181);
            break;
        case CANPDU_TX_UPDATE_STATUS:
        
            break;
			
        default:
            break;
    }
}

static void BCanPdu_TxHandler_OBD_0x639(CanPduTxStatus_t status)
{
    switch (status) {
    	case CANPDU_TX_CLEAR_STATUS:
            BCanPdu_ClearTxPdu(0x639);
            break;
        case CANPDU_TX_UPDATE_STATUS:
        
            break;
			
        default:
            break;
    }
}

//Modify (Ken):VEHICLE-V0C02 NO.1 -20231218
#if defined __FIT_Aeon_H
static void BCanPdu_TxHandler_OBD_0x061(CanPduTxStatus_t status)
{
    switch (status) {
    	case CANPDU_TX_CLEAR_STATUS:
            BCanPdu_ClearTxPdu(0x061);
            break;
        case CANPDU_TX_UPDATE_STATUS:

            break;

        default:
            break;
    }
}
#endif
/*Private Interfaces***********************************************/

/*!
 * \brief  
 * \param  status
 * \return None
 */

// static void BCanPdu_RxHandler_SA_0x200(CanPduRxStatus_t status)
// {
//     switch(status) {
//         case CANPDU_RX_LOST_STATUS:
//             break;
//         case CANPDU_RX_UPDATE_STATUS:
//             ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x200,NULL,NULL);
//             break;
//         default:
//             break;
//     }
// }
// static void BCanPdu_RxHandler_SA_0x201(CanPduRxStatus_t status)
// {
//     switch(status) {
//         case CANPDU_RX_LOST_STATUS:
//             break;
//         case CANPDU_RX_UPDATE_STATUS:
//             ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x201,NULL,NULL);
//             break;
//         default:
//             break;
//     }
// }
// static void BCanPdu_RxHandler_SA_0x202(CanPduRxStatus_t status)
// {
//     switch(status) {
//         case CANPDU_RX_LOST_STATUS:
//             break;
//         case CANPDU_RX_UPDATE_STATUS:
//             ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x202,NULL,NULL);
//             break;
//         default:
//             break;
//     }
// }
// static void BCanPdu_RxHandler_SA_0x203(CanPduRxStatus_t status)
// {
//     switch(status) {
//         case CANPDU_RX_LOST_STATUS:
//             break;
//         case CANPDU_RX_UPDATE_STATUS:
//             ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x203,NULL,NULL);
//         default:
//             break;
//     }
// }
static void BCanPdu_RxHandler_SA_0x210(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x210,NULL,NULL);
            break;
        default:
            break;
    }
}
static void BCanPdu_RxHandler_SA_0x220(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x220,NULL,NULL);
            break;
        default:
            break;
    }
}
static void BCanPdu_RxHandler_SA_0x230(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x230,NULL,NULL);
            break;
        default:
            break;
    }
}
static void BCanPdu_RxHandler_SA_0x240(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x240,NULL,NULL);
            break;
        default:
            break;
    }
}
static void BCanPdu_RxHandler_SA_0x250(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x250,NULL,NULL);
            break;
        default:
            break;
    }
}
static void BCanPdu_RxHandler_SA_0x260(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x260,NULL,NULL);
            break;
        default:
            break;
    }
}

static void BCanPdu_RxHandler_SA_0x211(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x211,NULL,NULL);
            break;
        default:
            break;
    }
}
static void BCanPdu_RxHandler_SA_0x221(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x221,NULL,NULL);
            break;
        default:
            break;
    }
}
static void BCanPdu_RxHandler_SA_0x231(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x231,NULL,NULL);
            break;
        default:
            break;
    }
}
static void BCanPdu_RxHandler_SA_0x241(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x241,NULL,NULL);
            break;
        default:
            break;
    }
}
static void BCanPdu_RxHandler_SA_0x251(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x251,NULL,NULL);
            break;
        default:
            break;
    }
}
static void BCanPdu_RxHandler_SA_0x261(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x261,NULL,NULL);
            break;
        default:
            break;
    }
}


static void BCanPdu_RxHandler_SA_0x212(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x212,NULL,NULL);
            break;
        default:
            break;
    }
}
static void BCanPdu_RxHandler_SA_0x222(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x222,NULL,NULL);
            break;
        default:
            break;
    }
}
static void BCanPdu_RxHandler_SA_0x232(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x232,NULL,NULL);
            break;
        default:
            break;
    }
}
static void BCanPdu_RxHandler_SA_0x242(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x242,NULL,NULL);
            break;
        default:
            break;
    }
}
static void BCanPdu_RxHandler_SA_0x252(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x252,NULL,NULL);
            break;
        default:
            break;
    }
}
static void BCanPdu_RxHandler_SA_0x262(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x262,NULL,NULL);
            break;
        default:
            break;
    }
}


static void BCanPdu_RxHandler_SA_0x213(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x213,NULL,NULL);
            break;
        default:
            break;
    }
}
static void BCanPdu_RxHandler_SA_0x223(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x223,NULL,NULL);
            break;
        default:
            break;
    }
}
static void BCanPdu_RxHandler_SA_0x233(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x233,NULL,NULL);
            break;
        default:
            break;
    }
}
static void BCanPdu_RxHandler_SA_0x243(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x243,NULL,NULL);
            break;
        default:
            break;
    }
}
static void BCanPdu_RxHandler_SA_0x253(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x253,NULL,NULL);
            break;
        default:
            break;
    }
}
static void BCanPdu_RxHandler_SA_0x263(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x263,NULL,NULL);
            break;
        default:
            break;
    }
}

static void BCanPdu_RxHandler_SA_0x2A0(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x2A0,NULL,NULL);
            break;
        default:
            break;
    }
}
static void BCanPdu_RxHandler_SA_0x2A1(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_SA_0x2A1,NULL,NULL);
            break;
        default:
            break;
    }
}


static void BCanPdu_RxHandler_ODB_0x58(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_ODB_0x58,NULL,NULL);
            break;
        default:
            break;
    }
}


static void BCanPdu_RxHandler_ODB_0x59(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_ODB_0x59,NULL,NULL);
            break;
        default:
            break;
    }
}

//Modify (Ken):VEHICLE-V0C02 NO.1 -20231218
#if defined __FIT_Aeon_H
static void BCanPdu_RxHandler_OBD_0x60(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_ODB_0x60,NULL,NULL);
            break;
        default:
            break;
    }
}
#endif


#ifdef FIT_DEBUG_NO_SA 
static void BCanPdu_RxHandler_UWB_0x313(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_UWB_0x313,NULL,NULL);
            break;
        default:
            break;
    }
}
static void BCanPdu_RxHandler_UWB_0x323(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_UWB_0x323,NULL,NULL);
            break;
        default:
            break;
    }
}
static void BCanPdu_RxHandler_UWB_0x333(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_UWB_0x333,NULL,NULL);
            break;
        default:
            break;
    }
}
static void BCanPdu_RxHandler_UWB_0x343(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_UWB_0x343,NULL,NULL);
            break;
        default:
            break;
    }
}
static void BCanPdu_RxHandler_UWB_0x353(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_UWB_0x353,NULL,NULL);
            break;
        default:
            break;
    }
}
static void BCanPdu_RxHandler_UWB_0x363(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            ble_ccc_send_evt(CCC_EVT_RECV_CAN_DATA,CANID_UWB_0x363,NULL,NULL);
            break;
        default:
            break;
    }
}
#endif

static void BCanPdu_RxHandler_SA_0x631(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            uds_app_process(g_BCanPdu_SA_0x631.Data,64U);
            break;
        default:
            break;
    }
}
/*!
 * \}
 */

void BCanPdu_Get_SA601_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_SA_0x631.Data,64U);
}


void BCanPdu_Get_OBD609_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_OBD_0x639.Data,64U);
}

void BCanPdu_Set_OBD609_Data(uint8* data)
{
	core_mm_copy(g_BCanPdu_OBD_0x639.Data,data,64U);
    if (BCanPdu_EnableTxEventMsg(0x639))
    {
    }
}
void BCanPdu_Set_BLE100_Data(uint8* data)
{
    if (BCanPdu_EnableTxEventMsg(0x100))
    {
        core_mm_copy(g_BCanPdu_BLE_0x100.Data,data,64U);
    }
}
void BCanPdu_Get_BLE100_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_BLE_0x100.Data,64U);
}
void BCanPdu_Set_BLE101_Data(uint8* data)
{
    if (BCanPdu_EnableTxEventMsg(0x101))
    {
        core_mm_copy(g_BCanPdu_BLE_0x101.Data,data,64U);
    }
}
void BCanPdu_Get_BLE101_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_BLE_0x101.Data,64U);
}

void BCanPdu_Set_BLE102_Data(uint8* data)
{
    if (BCanPdu_EnableTxEventMsg(0x102))
    {
        core_mm_copy(g_BCanPdu_BLE_0x102.Data,data,64U);
    }
}
void BCanPdu_Get_BLE102_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_BLE_0x102.Data,64U);
}
void BCanPdu_Set_BLE103_Data(uint8* data)
{
    if (BCanPdu_EnableTxEventMsg(0x103))
    {
        core_mm_copy(g_BCanPdu_BLE_0x103.Data,data,64U);
    }
}
void BCanPdu_Get_BLE103_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_BLE_0x103.Data,64U);
}
void BCanPdu_Set_BLE104_Data(uint8* data)
{
    if (BCanPdu_EnableTxEventMsg(0x104))
    {
        core_mm_copy(g_BCanPdu_BLE_0x104.Data,data,64U);
    }
}
void BCanPdu_Get_BLE104_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_BLE_0x104.Data,64U);
}

void BCanPdu_Set_BLE133_Data(uint8* data)
{
    if (BCanPdu_EnableTxEventMsg(0x133))
    {
        core_mm_copy(g_BCanPdu_BLE_0x133.Data,data,64U);
    }
}
void BCanPdu_Get_BLE133_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_BLE_0x133.Data,64U);
}

void BCanPdu_Set_BLE180_Data(uint8* data)
{
    if (BCanPdu_EnableTxEventMsg(0x180))
    {
        core_mm_copy(g_BCanPdu_BLE_0x180.Data,data,64U);
    }
}
void BCanPdu_Get_BLE180_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_BLE_0x180.Data,64U);
}

void BCanPdu_Set_BLE181_Data(uint8* data)
{
    if (BCanPdu_EnableTxEventMsg(0x181))
    {
        core_mm_copy(g_BCanPdu_BLE_0x181.Data,data,64U);
    }
}
void BCanPdu_Get_BLE181_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_BLE_0x181.Data,64U);
}

//Modify (Ken):VEHICLE-V0C02 NO.1 -20231218
#if defined __FIT_Aeon_H
void BCanPdu_Set_OBD061_Data(uint8* data)
{
    if (BCanPdu_EnableTxEventMsg(0x061))
    {
        core_mm_copy(g_BCanPdu_OBD_0x61.Data,data,64U);
    }
}
void BCanPdu_Get_OBD061_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_OBD_0x61.Data,64U);
}
#endif

// void BCanPdu_Get_SA200_Data(uint8* data)
// {
//     core_mm_copy(data,g_BCanPdu_SA_0x200.Data,64U);
// }
// void BCanPdu_Get_SA201_Data(uint8* data)
// {
//     core_mm_copy(data,g_BCanPdu_SA_0x201.Data,64U);
// }
// void BCanPdu_Get_SA202_Data(uint8* data)
// {
//     core_mm_copy(data,g_BCanPdu_SA_0x202.Data,64U);
// }
// void BCanPdu_Get_SA203_Data(uint8* data)
// {
//     core_mm_copy(data,g_BCanPdu_SA_0x203.Data,64U);
// }

void BCanPdu_Get_SA210_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_SA_0x210.Data,64U);
}
void BCanPdu_Get_SA220_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_SA_0x220.Data,64U);
}
void BCanPdu_Get_SA230_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_SA_0x230.Data,64U);
}
void BCanPdu_Get_SA240_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_SA_0x240.Data,64U);
}
void BCanPdu_Get_SA250_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_SA_0x250.Data,64U);
}
void BCanPdu_Get_SA260_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_SA_0x260.Data,64U);
}

void BCanPdu_Get_SA211_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_SA_0x211.Data,64U);
}
void BCanPdu_Get_SA221_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_SA_0x221.Data,64U);
}
void BCanPdu_Get_SA231_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_SA_0x231.Data,64U);
}
void BCanPdu_Get_SA241_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_SA_0x241.Data,64U);
}
void BCanPdu_Get_SA251_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_SA_0x251.Data,64U);
}
void BCanPdu_Get_SA261_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_SA_0x261.Data,64U);
}

void BCanPdu_Get_SA212_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_SA_0x212.Data,64U);
}
void BCanPdu_Get_SA222_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_SA_0x222.Data,64U);
}
void BCanPdu_Get_SA232_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_SA_0x232.Data,64U);
}
void BCanPdu_Get_SA242_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_SA_0x242.Data,64U);
}
void BCanPdu_Get_SA252_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_SA_0x252.Data,64U);
}
void BCanPdu_Get_SA262_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_SA_0x262.Data,64U);
}




void BCanPdu_Get_SA213_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_SA_0x213.Data,64U);
}
void BCanPdu_Get_SA223_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_SA_0x223.Data,64U);
}
void BCanPdu_Get_SA233_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_SA_0x233.Data,64U);
}
void BCanPdu_Get_SA243_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_SA_0x243.Data,64U);
}
void BCanPdu_Get_SA253_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_SA_0x253.Data,64U);
}
void BCanPdu_Get_SA263_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_SA_0x263.Data,64U);
}


void BCanPdu_Get_SA2A0_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_SA_0x2A0.Data,64U);
}
void BCanPdu_Get_SA2A1_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_SA_0x2A1.Data,64U);
}



#ifdef FIT_DEBUG_NO_SA 
void BCanPdu_Get_UWB313_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_UWB_0x313.Data,64U);
}
void BCanPdu_Get_UWB323_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_UWB_0x323.Data,64U);
}
void BCanPdu_Get_UWB333_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_UWB_0x333.Data,64U);
}
void BCanPdu_Get_UWB343_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_UWB_0x343.Data,64U);
}
void BCanPdu_Get_UWB353_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_UWB_0x353.Data,64U);
}
void BCanPdu_Get_UWB363_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_UWB_0x363.Data,64U);
}
#endif

void BCanPdu_Get_ODB58_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_ODB_0x58.Data,64U);
}
void BCanPdu_Get_ODB59_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_ODB_0x59.Data,64U);
}

//Modify (Ken):VEHICLE-V0C02 NO.1 -20231218
#if defined __FIT_Aeon_H
void BCanPdu_Get_ODB60_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_OBD_0x60.Data,64U);
}
#endif


/*!
 * \brief  BCanPdu_VarInit, 初始化BCAN报文变量
 * \param  None
 * \return None
 */
void BCanPdu_VarInit(void)
{
    uint8 i;
    /* 接收PDU初始化 */
    for (i = 0; i < BCANPDU_N_RX; i++)
    {
        BCanPdu_Fill(g_BCanRxPduList[i].pData, 0x00, g_BCanRxPduList[i].DataLen);
        g_BCanRxPduList[i].RxUpdated = FALSE;
        g_BCanRxPduList[i].LostTick = 0;
        g_BCanRxPduList[i].LostConfirmed = FALSE;
        g_BCanRxPduList[i].RecovConfirmCnt = 0;
        g_BCanRxPduList[i].RxStatus = CANPDU_RX_LOST_STATUS;
       
    }
    //LOG_L_S(CAN_MD,"Send Pdu Init:\n\r");
    /* 发送PDU初始化 */
    for (i = 0; i < BCANPDU_N_TX; i++)
    {
        BCanPdu_Fill(g_BCanTxPduList[i].pData, 0x00, g_BCanTxPduList[i].DataLen);
        g_BCanTxPduList[i].NormalCycleTick = g_BCanTxPduList[i].NormalCycle;
        g_BCanTxPduList[i].ImmediateCycleTick = g_BCanTxPduList[i].ImmediateCycle;
        g_BCanTxPduList[i].ImmediateCnt = 0;
        g_BCanTxPduList[i].TxStatus = CANPDU_TX_CLEAR_STATUS;
    }
}

/*!
 * \brief  BCanPdu_ClearTxPdu 清除指定发送报文的控制变量
 * \param  id 发送报文ID
 * \return None
 */
void BCanPdu_ClearTxPdu(uint32 id)
{
    uint8 i = 0;

    for (i = 0; i < BCANPDU_N_TX; i++)
    {
        if (id == g_BCanTxPduList[i].ID)
        {
            BCanPdu_Fill(g_BCanTxPduList[i].pData, 0x00, g_BCanTxPduList[i].DataLen);
            // g_BCanTxPduList[i].NormalCycleTick = g_BCanTxPduList[i].NormalCycle;
            g_BCanTxPduList[i].ImmediateCycleTick = g_BCanTxPduList[i].ImmediateCycle;
            g_BCanTxPduList[i].ImmediateCnt = 0;
            g_BCanTxPduList[i].TxStatus = CANPDU_TX_CLEAR_STATUS;
            break;
        }
    }
}

/*!
 * \brief  BCanPdu_ClearRxPdu 清除指定接收报文的控制变量
 * \param  id 接收报文ID
 * \return None
 */
void BCanPdu_ClearRxPdu(uint32 id)
{
    uint8 i = 0;

    for (i = 0; i < BCANPDU_N_RX; i++)
    {
        if (id == g_BCanRxPduList[i].ID)
        {
            BCanPdu_Fill(g_BCanRxPduList[i].pData, 0x00, g_BCanRxPduList[i].DataLen);
            g_BCanRxPduList[i].RxUpdated = FALSE;
            g_BCanRxPduList[i].RecovConfirmCnt = 0;
            g_BCanRxPduList[i].RxStatus = CANPDU_RX_LOST_STATUS;
            break;
        }
    }
}

/*!
 * \brief  BCanPdu_GetTxStatus 查询发送报文当前状态
 * \param  id 发送报文ID
 * \return CANPDU_TX_CLEAR_STATUS 或 CANPDU_TX_UPDATE_STATUS
 */
CanPduTxStatus_t BCanPdu_GetTxStatus(uint32 id)
{
    uint8 i = 0;
    CanPduTxStatus_t retStatus = CANPDU_TX_CLEAR_STATUS;

    for (i = 0; i < BCANPDU_N_TX; i++) {
        if (id == g_BCanTxPduList[i].ID) {
            retStatus = g_BCanTxPduList[i].TxStatus;
            break;
        }
    }

    return retStatus;
}

/*!
 * \brief  BCanPdu_GetRxStatus 查询接收报文当前状态
 * \param  id 接收报文ID
 * \return CANPDU_RX_LOST_STATUS 或 CANPDU_RX_UPDATE_STATUS
 */
CanPduRxStatus_t BCanPdu_GetRxStatus(uint32 id)
{
    uint8 i = 0;
    CanPduRxStatus_t retStatus = CANPDU_RX_LOST_STATUS;

    for (i = 0; i < BCANPDU_N_RX; i++) {
        if (id == g_BCanRxPduList[i].ID) {
            retStatus = g_BCanRxPduList[i].RxStatus;
            break;
        }
    }

    return retStatus;
}

/*!
 * \brief  BCanPdu_SetTxResult 设置报文发送结果
 * \param  id 待设置发送结果的报文ID
 * \param  result 待设置的发送结果
 * \return TRUE-发送队列包含指定ID; FALSE-发送队列不包含指定ID
 */
boolean BCanPdu_SetTxResult(uint32 id, CanPduTxResult_t result)
{
	uint8 i = 0;
	boolean retFlag = FALSE;

	for (i = 0; i < BCANPDU_N_TX; i++) {
		if (id == g_BCanTxPduList[i].ID) {
			g_BCanTxPduList[i].TxResult = result;
			retFlag = TRUE;
			break;
		}
	}

	return retFlag;
}

/*!
 * \brief  BCanPdu_GetTxResult 查询报文发送结果
 * \param  id 待查询发送结果的报文ID
 * \return None
 */
CanPduTxResult_t BCanPdu_GetTxResult(uint32 id)
{
	uint8 i = 0;
	CanPduTxResult_t retVal = CANPDU_TX_INVALID;

	for (i = 0; i < BCANPDU_N_TX; i++) {
		if (id == g_BCanTxPduList[i].ID) {
			retVal = g_BCanTxPduList[i].TxResult;
			break;
		}
	}

	return retVal;
}

/*!
 * \brief  BCanPdu_ClearTxResult 清除报文发送结果
 * \note   调用BCanPdu_GetTxResult函数后需要调用此函数, 否则会造成下一次读取发送结果错误
 * \param  id 待设置发送结果的报文ID
 * \return TRUE-发送队列包含指定ID; FALSE-发送队列不包含指定ID
 */
boolean BCanPdu_ClearTxResult(uint32 id)
{
	uint8 i = 0;
	boolean retFlag = FALSE;

	for (i = 0; i < BCANPDU_N_TX; i++) {
		if (id == g_BCanTxPduList[i].ID) {
			g_BCanTxPduList[i].TxResult = CANPDU_TX_READY;
			retFlag = TRUE;
			break;
		}
	}

	return retFlag;
}

/*!
 * \brief  BCanPdu_EnableTxEventMsg 使能事件报文发送
 * \param  id 发送报文ID
 * \return TRUE-成功; FALSE-失败
 */
boolean BCanPdu_EnableTxEventMsg(uint32 id)
{
    uint8 i = 0;
    boolean retFlag = FALSE;

    for (i = 0; i < BCANPDU_N_TX; i++) {
        if (id == g_BCanTxPduList[i].ID) {
            if (CANPDU_TX_CLEAR_STATUS == g_BCanTxPduList[i].TxStatus) {
                g_BCanTxPduList[i].ImmediateTxEnabled = TRUE;
                retFlag = TRUE;
            } else {
                retFlag = FALSE;
            }
            break;
        }
    }

    return retFlag;
}

/*!
 * \brief  BCanPdu_DisableTxEventMsg 停止事件报文发送
 * \param  id 发送报文ID
 * \return TRUE-成功; FALSE-失败
 */
boolean BCanPdu_DisableTxEventMsg(uint32 id)
{
    uint8 i = 0;
    boolean retFlag = FALSE;

    for (i = 0; i < BCANPDU_N_TX; i++) {
        if (id == g_BCanTxPduList[i].ID) {
            g_BCanTxPduList[i].ImmediateTxEnabled = FALSE;
            g_BCanTxPduList[i].ImmediateCnt = 0;
            g_BCanTxPduList[i].TxStatus = CANPDU_TX_CLEAR_STATUS;
            g_BCanTxPduList[i].ImmediateCycleTick = g_BCanTxPduList[i].ImmediateCycle;
            retFlag = TRUE;
            break;
        }
    }
    return retFlag;
}

/*!
 * \brief  BCanPdu_ReadRawMsg, 根据报文ID读取报文
 * \param  id 输入ID
 * \return 指向CanRawMsg_t类型的指针
 */
CanMsg_t* BCanPdu_ReadRawMsg(uint32 id)
{
    uint8 i, j;
    static CanMsg_t msg;
    CanMsg_t* pRetMsg = NULL_PTR;
    boolean found = FALSE;

    /* 扫描发送列表 */
    for (i = 0; i < BCANPDU_N_TX; i++) {
        if (g_BCanTxPduList[i].ID == id) {
        	msg.ID = g_BCanTxPduList[i].ID;
        	msg.DataLen = g_BCanTxPduList[i].DataLen;
        	for (j = 0; j < g_BCanTxPduList[i].DataLen; j++) {
        		msg.Data[j] = g_BCanTxPduList[i].pData[j];
        	}
        	found = TRUE;
            break;
        }
    }
     
    /* 扫描接收列表 */
    if (!found) {
        for (i = 0; i < BCANPDU_N_RX; i++) {
            if (g_BCanRxPduList[i].ID == id) {
            	msg.ID = g_BCanRxPduList[i].ID;
            	msg.DataLen = g_BCanRxPduList[i].DataLen;
            	for (j = 0; j < g_BCanRxPduList[i].DataLen; j++) {
            		msg.Data[j] = g_BCanRxPduList[i].pData[j];
            	}
                found = TRUE;
                break;
            }
        }
    }

    /* 返回找到结果 */
    if (found) {
        pRetMsg = &msg;
    }
    return pRetMsg;
}
