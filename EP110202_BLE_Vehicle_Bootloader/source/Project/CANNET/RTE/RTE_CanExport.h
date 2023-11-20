/* @Encoding: UTF-8 */
/*!
 ********************************************************************
 * @file   : RTE_CanExport.h
 * @brief  : Interfaces for all modules except CAN.
 * @author : J.L.
 * @version: A.0.1
 * @release: @currentDateTime
 ********************************************************************
 *                      Description/Information
 * 1.
 * 2.
 ********************************************************************
 */
#ifndef	RTE_CAN_EXPORT_H
#define	RTE_CAN_EXPORT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes*********************************************************/
#include <StandardTypes.h>
#include <BCanIfCfg.h>
#include <BCanIf.h>
#include <BCanPduCfg.h>
#include <BCanPdu.h>
#include <BCanTrcvCfg.h>
#include <BCanTrcv.h>

/* Enumerations & Unions *******************************************/

/* Structures ******************************************************/

/* Public Interfaces************************************************/
/*!
 * \brief BCAN报文接收处理函数, 由COM提供
 * \param pFrame MCAL定义的报文帧类型指针, 根据项目不同需要调整
 * \return None
 * \note 一般放在CAN接收中断中运行
 */
#define RTE_BCanTrcvRxMsgHandler(pFrame) \
        BCanTrcv_RxMsgHandler(pFrame)

/*!
 * \brief 读取BCAN当前的接收和发送原始报文, 由COM提供
 * \param id 报文ID
 * \return 需要读取的报文指针
 */
#define RTE_BCanReadRawMsg(id) \
        BCanPdu_ReadRawMsg(id)

/*!
 * \brief 读取BCAN TP报文, 由COM提供
 * \param pId 报文ID指针
 * \param pData 报文数据指针
 * \param pLen 报文数据长度指针
 * \return 需要读取的报文指针
 */
#define RTE_BCanTpReadRxMsg(pId, pData, pLen) \
        BCanTp_RxMsg(pId, pData, pLen)

/*!
 * \brief 读取BCAN TP报文, 由COM提供
 * \param id 报文ID
 * \param pData 报文数据指针
 * \param len 报文数据长度
 * \return 需要读取的报文指针
 */
#define RTE_BCanTpTxMsg(id, pData, len) \
        BCanTp_TxMsg(id, pData, len)


/* Public Function Prototypes***************************************/

#ifdef __cplusplus
}
#endif /* external "C" */
#endif  /* RTE_CAN_EXPORT_H */
