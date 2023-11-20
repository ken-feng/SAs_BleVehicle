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
#define FLASH_PAGE_SIZE     4096U
#define OTA_PACK_SIZE       60U

#define TIME_ENSURE_CNT    	10000       /*模式确认超时时间为10S,单位ms*/
#define TIME_RESET_CNT    	100       /*模式确认超时时间为100ms,单位ms*/

/*Enumerations & Structures*****************************************/

typedef enum
{
    ECU_MODE_BOOT = 0,
    ECU_MODE_APP,
};


typedef enum
{
    UDS_CMD_SWITCH_MODE = 0x10U,                    /*切换模式*/
    UDS_CMD_ENSURE_MODE = 0x11U,                    /*确认模式*/
    UDS_CMD_ERASE_FLASH = 0x33U,                    /*擦除数据*/
    UDS_CMD_TRANSFER_CONFIG = 0x34U,                /*传输配置*/
    UDS_CMD_TRANSFER_DATA = 0x36U,                  /*传输数据*/
    UDS_CMD_TRANSFER_END = 0x37U,                   /*传输结束*/
    UDS_CMD_DIAG = 0x40U,                           /*诊断指令*/
}uds_cmd_t;

typedef enum
{
    UDS_ENSURE_STATUS_INIT =  0U,               /*初始状态*/
    UDS_ENSURE_STATUS_PRE_ENSURE = 1U,          /*还未确认状态*/
    UDS_ENSURE_STATUS_ENSURED = 2U,             /*已确认状态*/
}uds_ensure_status_t;


typedef enum
{
    UDS_PROC_STATUS_INIT =  0U,                 /*初始状态*/
    UDS_PROC_STATUS_ERASE = 1U,                 /*已完成擦除操作*/
    UDS_PROC_STATUS_TRANS_CFG = 2U,             /*已完成传输配置操作*/
    UDS_PROC_STATUS_TRANS_DATA = 3U,            /*已完成传输数据操作*/
    UDS_PROC_STATUS_TRANS_END = 4U,             /*已完成传输结束操作*/
}uds_proc_status_t;

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
    UDS_ERR_CODE_ERR_ERASE,                     /*擦除失败*/
    UDS_ERR_CODE_ERR_START_ADDR,                /*起始地址不合法*/
    UDS_ERR_CODE_ERR_ERASE_LENGTH,              /*擦除长度不合法*/
    UDS_ERR_CODE_ERR_CRC,                       /*CRC校验失败*/
    UDS_ERR_CODE_ERR_BLOCK,                     /*块号错误*/
    UDS_ERR_CODE_ERR_PACKEAGE,                  /*包号错误*/
    UDS_ERR_CODE_ERR_WRITE_LENGTH,              /*写入长度不合法*/
    UDS_ERR_CODE_ERR_PROCESS,                   /*流程错误*/
    UDS_ERR_CODE_ERR_TAG_NOT_FOUND,             /*Tag不存在*/
}uds_errCode_t;

typedef struct 
{
    uds_ensure_status_t uds_ensure_status;      /*当前确认状态*/
    uds_proc_status_t uds_process_status;       /*当前流程状态*/

    u8  ensureCntValid;                         /*确认模式计时器是否开启*/
    u32 ensureCnt;                              /*确认模式计时器*/

    u32 resetFlag;
    u8 resetCnt;
    u8 gBuffer[64U];                            /*发送数据缓存*/
}uds_ctx_t;


typedef struct 
{
    u32 startAddress;   /*OTA起始地址*/
    u32 totalSize;      /*OTA总数据大小*/
    u32 totalOffset;    /*OTA更新当前偏移*/
    u16 packageOffset;  /*OTA包内数据偏移*/
    u32 blockSize;      /*OTA块大小*/
    u16 blockSN;        /*当前块号*/
    u16 packSN;         /*当前包号*/
    u8  blockBuf[FLASH_PAGE_SIZE];
}uds_ota_ctx_t;

/*Variable Definitions**********************************************/
extern u8 gEnsureCntValid;
extern uds_ctx_t uds_ctx;
extern uds_ota_ctx_t uds_ota_ctx;
/* Public Function Prototypes***************************************/
extern void uds_init(void);

extern void uds_res_append_and_send(uds_cmd_t uds_cmd, u8* buff, u16 length);

extern void uds_increase_cnt(void);

extern void uds_set_ensure_cnt_valid(void);

extern void uds_timer_ctrl(void);

extern void uds_app_process(u8* buffer, u16 length);
#ifdef __cplusplus
}
#endif /* external "C" */
#endif /* UDS_APP_H */
