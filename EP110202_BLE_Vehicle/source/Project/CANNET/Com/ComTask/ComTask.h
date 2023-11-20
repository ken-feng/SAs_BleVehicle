/* @Encoding: UTF-8 */
/*!
 ********************************************************************
 * @file   : NetTask.h
 * @brief  : Header for the NetTask.c.
 * @author : J.L.
 * @version: A.0.1
 * @release: @currentDateTime
 ********************************************************************
 *                      Description/Information
 * 1.
 * 2.
 ********************************************************************
 */
#ifndef COM_TASK_H
#define COM_TASK_H

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
extern void Com_Init(void);
extern void Com_TimerCtrl(void);
extern void Com_1msTask(void);
extern void Com_2msTask(void);
extern void Com_5msTask(void);

#ifdef __cplusplus
}
#endif /* external "C" */
#endif /* COM_TASK_H */
