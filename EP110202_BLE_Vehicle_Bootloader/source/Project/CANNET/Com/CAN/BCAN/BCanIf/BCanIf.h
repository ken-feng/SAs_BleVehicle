/* @Encoding: UTF-8 */
/*!
 ********************************************************************
 * @file   : BCanIf.h
 * @brief  : Header for the BCanIf.c.
 * @author : J.L.
 * @version: A.0.1
 * @release: @currentDateTime
 ********************************************************************
 *                      Description/Information
 * 1.
 * 2.
 ********************************************************************
 */
#ifndef BCANIF_H
#define BCANIF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes*********************************************************/
#include <ComStackTypes.h>

/* Macros & Typedef*************************************************/

/*Imported Interfaces***********************************************/

/*Enumerations & Structures*****************************************/

/*Variable Definitions**********************************************/

/* Public Function Prototypes***************************************/
extern void      BCanIf_ClearAppTxBuff(void);
extern void      BCanIf_ClearAllTxBuff(void);
extern uint8     BCanIf_ReadAppRxMsg(CanMsg_t* p_msg);
extern uint8     BCanIf_WriteAppTxMsg(CanMsg_t* p_msg);
extern uint8     BCanIf_WriteAppRxMsg(uint16 id, volatile uint8* pData, uint8 len);
extern CanMsg_t* BCanIf_ReadAppTxMsg(void);
extern void      BCanIf_AppMsgTxResultProcess(void);
#ifdef __cplusplus
}
#endif /* external "C" */
#endif
