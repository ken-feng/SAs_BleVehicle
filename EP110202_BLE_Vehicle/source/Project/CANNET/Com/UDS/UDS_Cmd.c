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
#include "UDS_Cmd.h"
#include "UDS_App.h"
#include "UDS_ext_api.h"
/* Macros & Typedef*************************************************/

/*Imported Interfaces***********************************************/

/*Enumerations & Structures*****************************************/

/*Variable Definitions**********************************************/

/* Public Function Prototypes***************************************/
void uds_cmd_switch_mode(u8* data,u16 length)
{
    /*设置Flash区, 模式状态标志位为App状态*/
    if (data[1] != 0x02)
    {
        uds_ctx.gBuffer[0U] = UDS_ERR_CODE_ERR_STATUS;
        uds_res_append_and_send(UDS_CMD_SWITCH_MODE,uds_ctx.gBuffer,1U);
        return ;
    }
    UDS_set_appmode_valid(data[1]);
    /*调用reset芯片接口*/
    UDS_ext_reset_system();
    while(1);

}