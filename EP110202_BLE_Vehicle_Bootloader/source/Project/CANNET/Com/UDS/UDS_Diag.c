/* @Encoding: UTF-8 */
/*!
 ********************************************************************
 * @file   : UDS_App.c
 * @brief  : Configuration APIs for the UDS_App module.
 * @author : 
 * @version: A.0.1
 * @release: @currentDateTime
 ********************************************************************
 *                      Description/Information
 * 1.
 * 2.
 ********************************************************************
 */

/* Includes*********************************************************/
#include "UDS_Diag.h"
#include "UDS_App.h"
#include "UDS_Cfg.h"
/* Macros & Typedef*************************************************/

/*Imported Interfaces***********************************************/

/*Enumerations & Structures*****************************************/

/*Variable Definitions**********************************************/

/* Public Function Prototypes***************************************/

void uds_cmd_diag(u8* data,u16 length)
{
    u16 diagTag;
    
    diagTag = core_dcm_readBig32(data+1U);

    switch (diagTag)
    {
    case DIAG_TAG_GET_SW_VERSION:
        uds_ctx.gBuffer[0U] = UDS_ERR_CODE_SUCCESS;
        core_mm_copy(uds_ctx.gBuffer+1U,l_TAG_0001_EcuSwVerNum,8U);
        break;
    case DIAG_TAG_GET_HW_VERSION:
        uds_ctx.gBuffer[0U] = UDS_ERR_CODE_SUCCESS;
        core_mm_copy(uds_ctx.gBuffer+1U,l_TAG_0002_EcuHwVerNum,8U);
        break; 
    case DIAG_TAG_WRITE_ANCHOR_SN:
        uds_ctx.gBuffer[0U] = UDS_ERR_CODE_SUCCESS;
        break;  
    default:
        uds_ctx.gBuffer[0U] = UDS_ERR_CODE_ERR_TAG_NOT_FOUND;
        break;
    }
    uds_res_append_and_send(UDS_CMD_DIAG,uds_ctx.gBuffer,63U);
}