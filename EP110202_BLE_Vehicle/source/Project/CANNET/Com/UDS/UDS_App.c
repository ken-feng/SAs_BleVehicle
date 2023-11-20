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
#include "UDS_App.h"
#include "UDS_ext_api.h"
/* Macros & Typedef*************************************************/

/*Imported Interfaces***********************************************/

/*Enumerations & Structures*****************************************/

/*Variable Definitions**********************************************/

/* Public Function Prototypes***************************************/

uds_ctx_t uds_ctx;
void uds_init(void)
{
    core_mm_set((u8*)&uds_ctx,0x00U,sizeof(uds_ctx_t));
}

void uds_res_append_and_send(uds_cmd_t uds_cmd, u8* buff, u16 length)
{
    u8 tmpBuf[64U] = {0U};
    tmpBuf[0U] = uds_cmd;
    core_mm_copy(tmpBuf+1U,buff,length);
    UDS_ext_send_data(tmpBuf,64U);
}
void uds_set_ensure_cnt_valid(void)
{
}
void uds_increase_cnt(void)
{

}
void uds_process_cnt(void)
{

}

void uds_timer_ctrl(void)
{
    uds_increase_cnt();
    uds_process_cnt();
}


void uds_app_process(u8* buffer, u16 length)
{                       
    if (length != 64U)
    {
        uds_ctx.gBuffer[0U] = UDS_ERR_CODE_ERR_PARAM;
        uds_res_append_and_send(buffer[0U],uds_ctx.gBuffer,1U);
        return;
    }
    switch (buffer[0U])
    {
    case UDS_CMD_SWITCH_MODE:/*切换模式*/
        uds_cmd_switch_mode(buffer,length);
        break;
    case UDS_CMD_DIAG:/*诊断指令*/ 
        uds_cmd_diag(buffer,length);
        break;
    default:
        uds_ctx.gBuffer[0U] = UDS_ERR_CODE_ERR_PARAM;
        uds_res_append_and_send(buffer[0U],uds_ctx.gBuffer,1U);
        break;
    }
    return ;
}
