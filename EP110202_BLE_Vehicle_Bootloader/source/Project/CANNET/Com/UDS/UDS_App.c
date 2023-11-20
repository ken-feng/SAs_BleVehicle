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
u8 gEnsureCntValid = 0U;
uds_ctx_t uds_ctx;
uds_ota_ctx_t uds_ota_ctx;
void uds_init(void)
{
    core_mm_set((u8*)&uds_ctx,0x00U,sizeof(uds_ctx_t));
    core_mm_set((u8*)&uds_ota_ctx,0x00U,sizeof(uds_ota_ctx_t));
    if(gEnsureCntValid)
    {
        gEnsureCntValid = 0U;
    	uds_set_ensure_cnt_valid();
    	if(UDS_get_appSwitchReturn_Flag() == 1U)
    	{
    		uds_ctx.gBuffer[0U] = UDS_ERR_CODE_SUCCESS;
    		uds_res_append_and_send(UDS_CMD_SWITCH_MODE,uds_ctx.gBuffer,1U);
    		UDS_set_appSwitchReturn_Flag(0U);
    		uds_ctx.uds_ensure_status = UDS_ENSURE_STATUS_PRE_ENSURE;
    	}
    }
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
    uds_ctx.ensureCntValid = 1U;
    uds_ctx.ensureCnt = 0U;
}
void uds_clear_ensure_cnt_valid(void)
{
    uds_ctx.ensureCntValid = 0U;
    uds_ctx.ensureCnt = 0U;
}
void uds_increase_cnt(void)
{
    if (uds_ctx.ensureCntValid)
    {
        uds_ctx.ensureCnt++;
    }
    else
    {
    	uds_ctx.ensureCnt = 0U;
    }
    if (uds_ctx.resetFlag)
    {
        uds_ctx.resetCnt++;
    }
    else
    {
    	uds_ctx.resetCnt = 0U;
    }


}
void uds_process_cnt(void)
{
    if ((uds_ctx.ensureCnt == TIME_ENSURE_CNT)&&(UDS_get_appFlag_valid() == 1U))
    {
    	uds_ctx.ensureCntValid = 0;
    	uds_ctx.ensureCnt = 0U;
        /*处理模式确认计时器超时事件*/
        UDS_set_appmode_valid(ECU_MODE_APP);/*设置App模式*/
        UDS_ext_reset_system();
    }
    if (uds_ctx.resetCnt == TIME_RESET_CNT)
    {
    	uds_ctx.resetFlag = 0;
    	uds_ctx.resetCnt = 0U;
        /*处理模式确认计时器超时事件*/
        UDS_ext_reset_system();
    }
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
    case UDS_CMD_ENSURE_MODE:/*确认模式*/
        uds_cmd_ensure_mode(buffer,length);
        break;
    case UDS_CMD_ERASE_FLASH:/*擦除数据*/
        uds_cmd_erase_area(buffer,length);
        break;
    case UDS_CMD_TRANSFER_CONFIG:/*传输配置*/
        uds_cmd_transfer_config(buffer,length);
        break;
    case UDS_CMD_TRANSFER_DATA:/*传输数据*/
        uds_cmd_transfer_data(buffer,length);
        break;
    case UDS_CMD_TRANSFER_END:/*传输结束*/
        uds_cmd_transfer_end(buffer,length);
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
