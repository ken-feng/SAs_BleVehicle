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
#ifndef UDS_DIAG_H
#define UDS_DIAG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes*********************************************************/
#include <ComStackTypes.h>

/* Macros & Typedef*************************************************/

/*Enumerations & Structures*****************************************/
typedef enum
{
    DIAG_TAG_GET_SW_VERSION = 0x0001U,      /*查询当前软件版本号*/
    DIAG_TAG_GET_HW_VERSION = 0x0002U,      /*查询当前硬件版本号*/

    DIAG_TAG_WRITE_ANCHOR_SN = 0x1003U,     /*写入当前锚点编号*/

    DIAG_TAG_UWB_SESSION_CTRL = 0x2004U,     /*重启测距/停止测距*/
}diag_info_tag_t;


/*Variable Definitions**********************************************/

/* Public Function Prototypes***************************************/

extern void uds_cmd_diag(u8* data,u16 length);

#ifdef __cplusplus
}
#endif /* external "C" */
#endif /* UDS_DIAG_H */
