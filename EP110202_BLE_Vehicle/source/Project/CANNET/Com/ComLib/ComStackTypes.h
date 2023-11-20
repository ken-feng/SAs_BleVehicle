/* @Encoding: UTF-8 */
/*!
 ********************************************************************
 * @file   : ComStackTypes.h
 * @brief  : Basic data types for network only.
 * @author : J.L.
 * @version: A.0.1
 * @release: @currentDateTime
 ********************************************************************
 *                      Description/Information
 * 1.
 * 2.
 ********************************************************************
 */
#ifndef COM_STACK_TYPES_H
#define COM_STACK_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes*********************************************************/
#include <StandardTypes.h>

/* Macros & Typedef*************************************************/

/* Enumerations & Unions *******************************************/
/*!
 * \brief CAN 报文类型定义 
 */
typedef uint8              CanMsgSendType_t;
#define CAN_CYCLE_MSG      ((CanMsgSendType_t)0x00) /* 周期型报文 */
#define CAN_EVENT_MSG      ((CanMsgSendType_t)0x01) /* 事件型报文 */
#define CAN_CE_MSG         ((CanMsgSendType_t)0x02) /* 周期事件型报文 */

/*!
 * \brief CAN 发送子状态, 在发送 PDU 对应处理函数中使用 
 */
typedef uint8                   CanPduTxStatus_t;
#define CANPDU_TX_CLEAR_STATUS  ((CanPduTxStatus_t)0x00) /* Clear message data */
#define CANPDU_TX_UPDATE_STATUS ((CanPduTxStatus_t)0x01) /* Event triggered */

/*!
 * \brief CAN 发送结果, 在底层发送函数和查询状态调用
 */
typedef uint8                   CanPduTxResult_t;
#define CANPDU_TX_READY         ((CanPduTxStatus_t)0x00) /* 发送就绪， 上电初始化或清除后的状态 */
#define CANPDU_TX_SUCCESS       ((CanPduTxStatus_t)0x01) /* 发送成功 */
#define CANPDU_TX_FAILED        ((CanPduTxStatus_t)0x02) /* 发送失败 */
#define CANPDU_TX_PENDING       ((CanPduTxStatus_t)0x03) /* 底层忙碌, 发送被挂起 */
#define CANPDU_TX_INVALID       ((CanPduTxStatus_t)0xFF) /* 无效状态, 未查询到对应ID的报文则返回该值 */

/*!
 * \brief 接收子状态, 在接收 PDU 对应处理函数中使用 
 */
typedef uint8                   CanPduRxStatus_t;
#define CANPDU_RX_LOST_STATUS   ((CanPduTxStatus_t)0x00) /* Message lost */
#define CANPDU_RX_UPDATE_STATUS ((CanPduTxStatus_t)0x01) /* Message normally received */

/*Structures********************************************************/
/*!
 * \brief CAN原始报文数据类型定义, 用于中断接收 CAN 总线原始数据 
 */
typedef struct
{
    uint32 ID;
    uint8  DataLen;
    uint8  Data[64];
} CanMsg_t;

/*!
 * \brief 发送报文数据结构体定义
 */
typedef void (*CanTxHandleFunc_t)(CanPduTxStatus_t status);
typedef struct
{
    CanMsgSendType_t  SendType;           /*<!-- 报文发送类型 -->*/ 
    uint32            ID;                 /*<!-- 报文 ID -->*/
    uint8*            pData;              /*<!-- 指向报文数据的指针 -->*/
    uint8             DataLen;            /*<!-- 数据长度 -->*/
    uint16            NormalCycle;        /*<!-- 正常周期, 单位ms -->*/
    uint16            NormalCycleTick;    /*<!-- 正常周期计时, 单位ms -->*/
    boolean           NormalTxEnabled;    /*<!-- 正常发送使能 -->*/
    uint16            ImmediateCycle;     /*<!-- 快发周期, 单位ms -->*/
    uint16            ImmediateCycleTick; /*<!-- 快发周期计时, 单位ms -->*/
    boolean           ImmediateTxEnabled; /*<!-- 快发使能, 为 TRUE 时才能启动快发 -->*/
    uint8             ImmediateTimes;     /*<!-- 快发次数 -->*/
    uint8             ImmediateCnt;       /*<!-- 快发计数 -->*/
    CanPduRxStatus_t  TxStatus;           /*<!-- 报文发送处理状态 -->*/
    CanPduTxResult_t  TxResult;           /*<!-- 报文发送结果 -->*/
    CanTxHandleFunc_t TxHandleFunc;       /*<!-- 发送处理函数 -->*/
} CanTxPduHandle_t;

/*!
 * \brief 网络DTC处理结构体定义
 */
typedef uint8 (*CanDtcHandleFunc_t)(void);
typedef struct NetTypes
{
    boolean            Configured;       /*<!-- 记录 DTC 是否需要应用功能配置 -->*/
    boolean            RxUpdated;        /*<!-- 收到报文时需要置为 TRUE -->*/
    uint16             ConfirmTime;      /*<!-- DTC 确认时间, 单位ms -->*/
    uint16             ConfirmTick;      /*<!-- DTC 确认计时, 单位ms -->*/
    boolean            Confirmed;        /*<!-- DTC 记录确认标志 -->*/
    uint8              HealConfirmTimes; /*<!-- DTC 治愈确认次数 -->*/
    uint8              HealConfirmCnt;   /*<!-- DTC 治愈确认计数 -->*/
    CanDtcHandleFunc_t DtcHandleFunc;    /*<!-- DTC 处理函数, 如果无处理则配置为 NULL_PTR -->*/
    uint16   id;
    uint8   dtcid;
    uint8   exist;
} CanRxDtcHandle_t;

/*!
 * \brief 接收报文数据结构体定义
 */
typedef void (*CanRxHandleFunc_t)(CanPduRxStatus_t status);
typedef struct
{
    CanMsgSendType_t  SendType;          /*<!-- 报文发送类型 -->*/  
    uint32            ID;                /*<!-- 报文 ID -->*/
    uint8*            pData;             /*<!-- 指向报文数据的指针, 配置时指向报文 PDU 共同体的 Data -->*/
    uint8             DataLen;           /*<!-- 报文数据长度 -->*/
    boolean           RxEnabled;         /*<!-- 报文是否允许被接收 -->*/
    boolean           RxUpdated;         /*<!-- 收到报文时需要置为 TRUE -->*/
    uint16            NormalCycle;       /*<!-- 正常报文周期, 单位ms -->*/
    uint16            LostTick;          /*<!-- 丢失计时, 单位ms -->*/
    boolean           LostConfirmed;     /*<!-- 丢失标志 -->*/
    uint8             RecovConfirmTimes; /*<!-- 报文接收恢复确认次数 -->*/
    uint8             RecovConfirmCnt;   /*<!-- 报文接收恢复计数 -->*/
    CanPduRxStatus_t  RxStatus;          /*<!-- 报文接收状态 -->*/
    CanRxHandleFunc_t RxHandleFunc;      /*<!-- 接收处理函数, 如果无处理则配置为 NULL_PTR -->*/
    CanRxDtcHandle_t* pDtcHandle;        /*<!-- 指向 CanRxDtcHandle_t 类型的指针 -->*/
} CanRxPduHandle_t;

#ifdef __cplusplus
}
#endif /* external "C" */
#endif /* COM_STACK_TYPES_H */
