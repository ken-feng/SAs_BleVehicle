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
#ifndef UDS_APP_H
#define UDS_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes*********************************************************/
#include <ComStackTypes.h>

/* Macros & Typedef*************************************************/

/*Flash每页的大小*/
#define FLASH_PAGE_SIZE     2048U
#define OTA_PACK_SIZE       60U

/*Enumerations & Structures*****************************************/

typedef enum
{
    ECU_MODE_BOOT = 0,
    ECU_MODE_APP,
};


typedef enum
{
    UDS_CMD_SWITCH_MODE = 0x10U,                    /*切换模式*/
    UDS_CMD_DIAG = 0x40U,                           /*诊断指令*/
}uds_cmd_t;

/*响应报文偏移说明*/
typedef enum
{
    UDS_RES_OFFSET_CMD = 0U,                /*指令功能类型*/
    UDS_RES_OFFSET_STATUS,                  /*响应状态码*/
    UDS_RES_OFFSET_DATA,                    /*响应数据*/
}uds_res_offset_t;

typedef enum
{
    UDS_ERR_CODE_SUCCESS = 0U,                  /*执行成功*/
    UDS_ERR_CODE_ERR_PARAM,                     /*指令参数不正确*/
    UDS_ERR_CODE_ERR_STATUS,                    /*状态异常（已经处于所切换的模式）*/
    UDS_ERR_CODE_ERR_TAG_NOT_FOUND,             /*Tag不存在*/
}uds_errCode_t;

typedef struct 
{
    u8 gBuffer[64U];                            /*发送数据缓存*/
}uds_ctx_t;


/*Variable Definitions**********************************************/
extern uds_ctx_t uds_ctx;
/* Public Function Prototypes***************************************/
extern void uds_init(void);

extern void uds_res_append_and_send(uds_cmd_t uds_cmd, u8* buff, u16 length);

extern void uds_timer_ctrl(void);

extern void uds_app_process(u8* buffer, u16 length);
#ifdef __cplusplus
}
#endif /* external "C" */
#endif /* UDS_APP_H */
