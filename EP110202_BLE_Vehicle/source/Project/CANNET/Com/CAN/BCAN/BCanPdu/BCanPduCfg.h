/* @Encoding: UTF-8 */
/*!
 ********************************************************************
 * @file   : BCanPduCfg.h
 * @brief  : Configuration for the BCanPdu module.
 * @author : J.L.
 * @version: A.0.1
 * @release: @currentDateTime
 ********************************************************************
 *                      Description/Information
 * 1.
 * 2.
 ********************************************************************
 */
#ifndef BCAN_PDU_CFG_H
#define BCAN_PDU_CFG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes*********************************************************/
#include <BCanIf.h>
#include <BCanTrcv.h>
/*User Includes Begin***********************************************/

/*User Includes End*************************************************/

/* Macros & Typedef*************************************************/
#define BCANPDU_T_RX_DISPATCH         ((uint16)2U) /* 接收调度时间, 单位 ms */
#define BCANPDU_T_TX_DISPATCH         ((uint16)1U) /* 发送调度时间, 单位 ms */
#ifdef FIT_DEBUG_NO_SA    
	#define BCANPDU_N_RX                  ((uint8)33) //((uint8)16)    /* 接收报文数量 */
	#define BCANPDU_N_TX                  ((uint8)9)     			   /* 发送报文数量 */
//Modify (Ken):VEHICLE-V0C02 NO.1 -20231218
#elif defined 	__FIT_Aeon_H
	#define BCANPDU_N_RX                  ((uint8)30) //((uint8)16)    /* 接收报文数量 */
	#define BCANPDU_N_TX                  ((uint8)10)     			   /* 发送报文数量 */
#else
	#define BCANPDU_N_RX                  ((uint8)29) //((uint8)16)    /* 接收报文数量 */
	#define BCANPDU_N_TX                  ((uint8)9)     			   /* 发送报文数量 */
#endif

#define BCANPDU_N_CLR_DTC_CONFIRM     ((uint8)5)     /* 清除报文 DTC 确认次数 */
#define BCANPDU_N_RECOVER_CONFIRM     ((uint8)5)     /* 报文恢复确认次数 */
#define BCANPDU_RX_LOST_CYCLE_RATE    ((uint16)10)   /* 普通报文丢失周期倍数, 用于确认报文丢失 */

/*User Macros Begin*************************************************/

/*User Macros End***************************************************/

/*Imported Interfaces***********************************************/



#define BCanPdu_Compare(pArray1, pArray2, len) \
        RTE_Compare(pArray1, pArray2, len)

#define BCanPdu_Copy(pDest, pSource, len) \
        RTE_Copy(pDest, pSource, len)

#define BCanPdu_Fill(pArray, value, len) \
        RTE_Fill(pArray, value, len)

#define BCanPdu_ReadAppRxMsg(pRawMsg) \
        BCanIf_ReadAppRxMsg(pRawMsg)

#define BCanPdu_WriteAppTxMsg(pRawMsg) \
        BCanIf_WriteAppTxMsg(pRawMsg)

#define BCanPdu_CheckAppMsgTxAllowed() \
		BcanNM_IfAllowTxMsg()

/*User Imported Interfaces Begin************************************/

/*User Imported Interfaces End**************************************/

/*Enumerations & Structures*****************************************/
/*!
 * \defgroup 周期发送PDU定义
 * \{
 */
 typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_SA_0x631_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_BLE_0x100_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_BLE_0x101_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_BLE_0x102_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_BLE_0x103_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_BLE_0x104_t;

typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_BLE_0x133_t;

typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_BLE_0x180_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_BLE_0x181_t;

void BCanPdu_Get_SA601_Data(uint8* data);

void BCanPdu_Set_BLE100_Data(uint8* data);
void BCanPdu_Get_BLE100_Data(uint8* data);

void BCanPdu_Set_BLE101_Data(uint8* data);
void BCanPdu_Get_BLE101_Data(uint8* data);

void BCanPdu_Set_BLE102_Data(uint8* data);
void BCanPdu_Get_BLE102_Data(uint8* data);

void BCanPdu_Set_BLE103_Data(uint8* data);
void BCanPdu_Get_BLE103_Data(uint8* data);

void BCanPdu_Set_BLE104_Data(uint8* data);
void BCanPdu_Get_BLE104_Data(uint8* data);

void BCanPdu_Set_BLE133_Data(uint8* data);
void BCanPdu_Get_BLE133_Data(uint8* data);


void BCanPdu_Set_BLE180_Data(uint8* data);
void BCanPdu_Get_BLE180_Data(uint8* data);

void BCanPdu_Set_BLE181_Data(uint8* data);
void BCanPdu_Get_BLE181_Data(uint8* data);



// typedef union
// {
//     uint8 Data[64];
//     struct
//     {
//         uint8 Data[64];
//     } Layout;
// } BCanPdu_SA_0x200_t;

// typedef union
// {
//     uint8 Data[64];
//     struct
//     {
//         uint8 Data[64];
//     } Layout;
// } BCanPdu_SA_0x201_t;
// typedef union
// {
//     uint8 Data[64];
//     struct
//     {
//         uint8 Data[64];
//     } Layout;
// } BCanPdu_SA_0x202_t;
// typedef union
// {
//     uint8 Data[64];
//     struct
//     {
//         uint8 Data[64];
//     } Layout;
// } BCanPdu_SA_0x203_t;

typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_SA_0x210_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_SA_0x220_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_SA_0x230_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_SA_0x240_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_SA_0x250_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_SA_0x260_t;

typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_SA_0x211_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_SA_0x221_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_SA_0x231_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_SA_0x241_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_SA_0x251_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_SA_0x261_t;


typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_SA_0x212_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_SA_0x222_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_SA_0x232_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_SA_0x242_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_SA_0x252_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_SA_0x262_t;

typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_SA_0x213_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_SA_0x223_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_SA_0x233_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_SA_0x243_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_SA_0x253_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_SA_0x263_t;

typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_SA_0x2A0_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_SA_0x2A1_t;


// void BCanPdu_Get_SA200_Data(uint8* data);
// void BCanPdu_Get_SA201_Data(uint8* data);
// void BCanPdu_Get_SA202_Data(uint8* data);
// void BCanPdu_Get_SA203_Data(uint8* data);

void BCanPdu_Get_SA210_Data(uint8* data);
void BCanPdu_Get_SA220_Data(uint8* data);
void BCanPdu_Get_SA230_Data(uint8* data);
void BCanPdu_Get_SA240_Data(uint8* data);
void BCanPdu_Get_SA250_Data(uint8* data);
void BCanPdu_Get_SA260_Data(uint8* data);

void BCanPdu_Get_SA211_Data(uint8* data);
void BCanPdu_Get_SA221_Data(uint8* data);
void BCanPdu_Get_SA231_Data(uint8* data);
void BCanPdu_Get_SA241_Data(uint8* data);
void BCanPdu_Get_SA251_Data(uint8* data);
void BCanPdu_Get_SA261_Data(uint8* data);

void BCanPdu_Get_SA212_Data(uint8* data);
void BCanPdu_Get_SA222_Data(uint8* data);
void BCanPdu_Get_SA232_Data(uint8* data);
void BCanPdu_Get_SA242_Data(uint8* data);
void BCanPdu_Get_SA252_Data(uint8* data);
void BCanPdu_Get_SA262_Data(uint8* data);

void BCanPdu_Get_SA213_Data(uint8* data);
void BCanPdu_Get_SA223_Data(uint8* data);
void BCanPdu_Get_SA233_Data(uint8* data);
void BCanPdu_Get_SA243_Data(uint8* data);
void BCanPdu_Get_SA253_Data(uint8* data);
void BCanPdu_Get_SA263_Data(uint8* data);

void BCanPdu_Get_SA2A0_Data(uint8* data);
void BCanPdu_Get_SA2A1_Data(uint8* data);
#ifdef FIT_DEBUG_NO_SA 

typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_UWB_0x313_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_UWB_0x323_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_UWB_0x333_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_UWB_0x343_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_UWB_0x353_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_UWB_0x363_t;
void BCanPdu_Get_UWB313_Data(uint8* data);
void BCanPdu_Get_UWB323_Data(uint8* data);
void BCanPdu_Get_UWB333_Data(uint8* data);
void BCanPdu_Get_UWB343_Data(uint8* data);
void BCanPdu_Get_UWB353_Data(uint8* data);
void BCanPdu_Get_UWB363_Data(uint8* data);
#endif

typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_ODB_0x58_t;
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_ODB_0x59_t;

//Modify (Ken):VEHICLE-V0C02 NO.1 -20231218
#if defined __FIT_Aeon_H
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_OBD_0x60_t;
#endif

//Modify (Ken):VEHICLE-V0C02 NO.1 -20231218
#if defined __FIT_Aeon_H
typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_OBD_0x61_t;
#endif

void BCanPdu_Get_ODB58_Data(uint8* data);
void BCanPdu_Get_ODB59_Data(uint8* data);
//Modify (Ken):VEHICLE-V0C02 NO.1 -20231218
#if defined __FIT_Aeon_H
void BCanPdu_Get_ODB60_Data(uint8* data);
#endif

typedef union
{
    uint8 Data[64];
    struct
    {
        uint8 Data[64];
    } Layout;
} BCanPdu_OBD_0x639_t;

void BCanPdu_Set_OBD609_Data(uint8* data);
void BCanPdu_Get_OBD609_Data(uint8* data);

/*!
 * \defgroup BCAN 发送 PDU 配置列表定义
 * \{
 */
extern CanTxPduHandle_t g_BCanTxPduList[BCANPDU_N_TX];
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
extern CanRxPduHandle_t g_BCanRxPduList[BCANPDU_N_RX];
/*!
 * \}
 */


typedef void (*CanSetValueFunc_t)(uint8 value);
typedef struct
{
    boolean             TxEnabled;          /*<!-- 发送使能标志 -->*/
    uint8               cycleCnt;           /*<!-- 发送计数器 -->*/
    uint8               cycleMaxCnt;        /*<!-- 发送最大计数周期 -->*/
    CanSetValueFunc_t   canSetValueFunc;    /*<!-- 获取发送值函数 -->*/
    uint8               factValue;          /*<!-- 发送真实值 -->*/
    uint8               defaultValue;       /*<!-- 发送默认值 -->*/
} CanTxValueHandle_t;

typedef uint8 (*CanGetValueFunc_t)(void);
typedef void (*CanSetStatusCallBack_t_t)(uint8 value);
typedef void (*CanClearValueFunc_t)(void);
typedef struct
{
    uint8                   cycleCnt;                   /*<!-- 接送计数器 -->*/
    uint8                   cycleMaxCnt;                /*<!-- 接送最大计数周期 -->*/
    CanGetValueFunc_t       canGetValueFunc;            /*<!-- 获取接送值函数 -->*/
    CanSetStatusCallBack_t_t CanSetStatusCallBack_t;    /*<!-- 获取到新的状态回调函数 -->*/
    uint8                   lastValue;                  /*<!-- 上一次接收值 -->*/
    CanClearValueFunc_t     canClearValueFunc;            /*<!-- 获取接送值函数 -->*/
} CanRxValueHandle_t;


/* Public Interfaces************************************************/

/* Public Function Prototypes***************************************/
extern void             BCanPdu_VarInit(void);
extern void             BCanPdu_ClearTxPdu(uint32 id);
extern void             BCanPdu_ClearRxPdu(uint32 id);
extern CanPduTxStatus_t BCanPdu_GetTxStatus(uint32 id);
extern CanPduRxStatus_t BCanPdu_GetRxStatus(uint32 id);
extern boolean          BCanPdu_SetTxResult(uint32 id, CanPduTxResult_t result);
extern CanPduTxResult_t BCanPdu_GetTxResult(uint32 id);
extern boolean          BCanPdu_ClearTxResult(uint32 id);
extern boolean          BCanPdu_EnableTxEventMsg(uint32 id);
extern boolean          BCanPdu_DisableTxEventMsg(uint32 id);
extern CanMsg_t*        BCanPdu_ReadRawMsg(uint32 id);
#ifdef __cplusplus
}
#endif /* external "C" */
#endif /* BCAN_PDU_CFG_H */
