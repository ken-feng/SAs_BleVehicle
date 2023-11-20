/* @Encoding: UTF-8 */
/*!
 ********************************************************************
 * @file   : UDS_App.h
 * @brief  : Configurations for the UDS_App module.
 * @author : 
 * @version: A.0.1
 * @release: @currentDateTime
 ********************************************************************
 *                      Description/Information
 * 1.
 * 2.
 ********************************************************************
 */
#ifndef UDS_CMD_H
#define UDS_CMD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes*********************************************************/
#include <ComStackTypes.h>

/* Macros & Typedef*************************************************/

/*Enumerations & Structures*****************************************/

/*Variable Definitions**********************************************/

/* Public Function Prototypes***************************************/
extern void uds_cmd_switch_mode(u8* data,u16 length);

#ifdef __cplusplus
}
#endif /* external "C" */
#endif /* UDS_CMD_H */
