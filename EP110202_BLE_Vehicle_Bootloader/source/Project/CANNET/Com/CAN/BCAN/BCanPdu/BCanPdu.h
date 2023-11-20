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
#ifndef BCANPDU_H

#define BCANPDU_H

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
extern void    BCanPdu_Init(void);
extern void    BCanPdu_RxMainFunction(void);
extern void    BCanPdu_TxMainFunction(void);
extern boolean BCanPdu_IsAppMsg(uint16 id);

#ifdef __cplusplus
}
#endif /* external "C" */
#endif /* BCANPDU_H */
