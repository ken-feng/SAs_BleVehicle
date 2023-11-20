/* @Encoding: UTF-8 */
/*!
 ********************************************************************
 * @file   : BCanIfCfg.h
 * @brief  : Configurations for the BCanIf module.
 * @author : J.L.
 * @version: A.0.1
 * @release: @currentDateTime
 ********************************************************************
 *                      Description/Information
 * 1.
 * 2.
 ********************************************************************
 */
#ifndef BCANIF_CFG_H
#define BCANIF_CFG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes*********************************************************/
#include <ComStackTypes.h>

/* Macros & Typedef*************************************************/
#define N_BCANIF_APP_BUF  20

#define BCanIf_Copy(pArr1, pArr2, len)    RTE_Copy(pArr1, pArr2, len)

/*Enumerations & Structures*****************************************/
typedef struct
{
    uint8    ReadIndex;
    uint8    WriteIndex;
    boolean  Stuffed;
    CanMsg_t RawMsgs[N_BCANIF_APP_BUF];
} BCanIf_AppBuff_t;

/*Variable Definitions**********************************************/

/* Public Function Prototypes***************************************/

#ifdef __cplusplus
}
#endif /* external "C" */
#endif /* BCANIF_CFG_H */
