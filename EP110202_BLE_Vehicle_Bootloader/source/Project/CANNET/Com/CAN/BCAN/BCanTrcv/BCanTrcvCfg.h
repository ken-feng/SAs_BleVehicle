/* @Encoding: UTF-8 */
/*!
 ********************************************************************
 * @file   : BCanTrcvCfg.h
 * @brief  : Configurations for the BCanTrcv module.
 * @author : J.L.
 * @version: A.0.1
 * @release: @currentDateTime
 ********************************************************************
 *                      Description/Information
 * 1.
 * 2.
 ********************************************************************
 */
#ifndef BCANTRCV_CFG_H

#define BCANTRCV_CFG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes*********************************************************/
#include <BCanIf.h>
#include <BCanPdu.h>

/* Macros & Typedef*************************************************/

/*Imported Interfaces***********************************************/
#define BCanTrcv_TxMsg(id, pData, len)         KW36_CAN_Send_Frame(id, pData, len)

#define BCanTrcv_WriteAppRxMsg(id, pData, len) BCanIf_WriteAppRxMsg(id, pData, len)
#define BCanTrcv_ReadAppTxMsg(pMsg)            BCanIf_ReadAppTxMsg(pMsg)
#define BCanTrcv_AppMsgTxResultProcess()       BCanIf_AppMsgTxResultProcess();
#define BCanTrcv_SetTxResult(id, result)       BCanPdu_SetTxResult(id, result)

#define BCanTrcv_IsAppMsg(id)                  BCanPdu_IsAppMsg(id)

/*Enumerations & Structures*****************************************/

/*Variable Definitions**********************************************/

/* Public Function Prototypes***************************************/

#ifdef __cplusplus
}
#endif /* external "C" */
#endif /* BCANTRCV_CFG_H */

