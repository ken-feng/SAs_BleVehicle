/* @Encoding: UTF-8 */
/*!
 ********************************************************************
 * @file   : BCanTrcv.h
 * @brief  : Header for the BCanTrcv.c.
 * @author : J.L.
 * @version: A.0.1
 * @release: @currentDateTime
 ********************************************************************
 *                      Description/Information
 * 1.
 * 2.
 ********************************************************************
 */
#ifndef BCANTRCV_H

#define BCANTRCV_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes*********************************************************/
#include <ComStackTypes.h>
#include "fsl_flexcan.h"
/* Macros & Typedef*************************************************/

/*Imported Interfaces***********************************************/

/*Enumerations & Structures*****************************************/

/*Variable Definitions**********************************************/

/* Public Function Prototypes***************************************/
extern void BCanTrcv_Init(void);
extern void BCanTrcv_MsgTxHandler(void);
extern uint8 BCanTrcv_RxMsgHandler(flexcan_fd_frame_t* pFrame);

#ifdef __cplusplus
}
#endif /* external "C" */
#endif /* BCANTRCV_H */
