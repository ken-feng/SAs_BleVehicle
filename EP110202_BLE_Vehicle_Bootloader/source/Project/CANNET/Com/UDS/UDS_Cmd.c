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
	UDS_set_appmode_valid(data[1]);
//	UDS_set_appmode_valid(1U);
	if(data[1U] == 2U)
	{
		uds_ctx.uds_ensure_status = UDS_ENSURE_STATUS_PRE_ENSURE;
	}
	else
	{

	}
	uds_ctx.gBuffer[0U] = UDS_ERR_CODE_SUCCESS;
	uds_res_append_and_send(UDS_CMD_SWITCH_MODE,uds_ctx.gBuffer,1U);
	if(data[1U] == 1U)
	{
		uds_ctx.resetFlag = 1U;
		uds_ctx.resetCnt = 0;
	}
	else
	{

	}
	return ;

}

void uds_cmd_ensure_mode(u8* data,u16 length)
{
    
    if (uds_ctx.uds_ensure_status != UDS_ENSURE_STATUS_PRE_ENSURE)
    {
        uds_ctx.gBuffer[0U] = UDS_ERR_CODE_ERR_PROCESS;
        uds_res_append_and_send(UDS_CMD_ENSURE_MODE,uds_ctx.gBuffer,1U);
        return ;
    }
    uds_clear_ensure_cnt_valid();
    uds_ctx.uds_ensure_status = UDS_ENSURE_STATUS_ENSURED;
    uds_ctx.gBuffer[0U] = UDS_ERR_CODE_SUCCESS;
    uds_res_append_and_send(UDS_CMD_ENSURE_MODE,uds_ctx.gBuffer,1U);
}

void uds_cmd_erase_area(u8* data,u16 length)
{
    u32 startAddress;
    u32 eraseSize;
    
    if (uds_ctx.uds_ensure_status != UDS_ENSURE_STATUS_ENSURED)
    {
        uds_ctx.gBuffer[0U] = UDS_ERR_CODE_ERR_PROCESS;
        uds_res_append_and_send(UDS_CMD_ERASE_FLASH,uds_ctx.gBuffer,1U);
        return ;
    }
    startAddress = core_dcm_readBig32(data+1U);
    eraseSize = core_dcm_readBig32(data+5U);
    /*判断当前是否在Flash的区域范围内*/
    if (0U == UDS_ext_erase_is_inFlash(startAddress))
    {
        uds_ctx.gBuffer[0U] = UDS_ERR_CODE_ERR_START_ADDR;
        uds_res_append_and_send(UDS_CMD_ERASE_FLASH,uds_ctx.gBuffer,1U);
        return ;
    }
    /*判断要擦除的起始地址是否为4K的整数倍*/
    if (startAddress%FLASH_PAGE_SIZE != 0U)
    {
        uds_ctx.gBuffer[0U] = UDS_ERR_CODE_ERR_START_ADDR;
        uds_res_append_and_send(UDS_CMD_ERASE_FLASH,uds_ctx.gBuffer,1U);
        return ;
    }
    /*判断要擦除的大小是否为4K的整数倍*/
    if (eraseSize%FLASH_PAGE_SIZE != 0U)
    {
        uds_ctx.gBuffer[0U] = UDS_ERR_CODE_ERR_ERASE_LENGTH;
        uds_res_append_and_send(UDS_CMD_ERASE_FLASH,uds_ctx.gBuffer,1U);
        return ;
    }
    /*调用擦除接口api*/
    UDS_set_appFlag_valid(0U);
    UDS_ext_erase_area(startAddress,eraseSize);

    uds_ctx.uds_process_status = UDS_PROC_STATUS_ERASE;
    uds_ctx.gBuffer[0U] = UDS_ERR_CODE_SUCCESS;
    uds_res_append_and_send(UDS_CMD_ERASE_FLASH,uds_ctx.gBuffer,1U);
}

void uds_cmd_transfer_config(u8* data,u16 length)
{
    u32 startAddress;
    u32 totalSize;
    u32 blockSize;
    
    if ((uds_ctx.uds_ensure_status != UDS_ENSURE_STATUS_ENSURED)||(uds_ctx.uds_process_status != UDS_PROC_STATUS_ERASE))
    {
        uds_ctx.gBuffer[0U] = UDS_ERR_CODE_ERR_PROCESS;
        uds_res_append_and_send(UDS_CMD_TRANSFER_CONFIG,uds_ctx.gBuffer,1U);
        return ;
    }
    startAddress = core_dcm_readBig32(data+1U);
    totalSize = core_dcm_readBig32(data+5U);
    blockSize = core_dcm_readBig32(data+9U);
    /*判断当前是否在Flash的区域范围内*/
    if (0U == UDS_ext_erase_is_inFlash(startAddress))
    {
        uds_ctx.gBuffer[0U] = UDS_ERR_CODE_ERR_START_ADDR;
        uds_res_append_and_send(UDS_CMD_TRANSFER_CONFIG,uds_ctx.gBuffer,1U);
        return ;
    }
    /*判断当前是否在Flash的区域范围内*/
    if (0U == UDS_ext_erase_is_inFlash(startAddress+totalSize))
    {
        uds_ctx.gBuffer[0U] = UDS_ERR_CODE_ERR_START_ADDR;
        uds_res_append_and_send(UDS_CMD_TRANSFER_CONFIG,uds_ctx.gBuffer,1U);
        return ;
    }
    /*判断要擦除的起始地址是否为4K的整数倍*/
    if (startAddress%FLASH_PAGE_SIZE != 0U)
    {
        uds_ctx.gBuffer[0U] = UDS_ERR_CODE_ERR_START_ADDR;
        uds_res_append_and_send(UDS_CMD_TRANSFER_CONFIG,uds_ctx.gBuffer,1U);
        return ;
    }
    uds_ota_ctx.startAddress = startAddress;
    uds_ota_ctx.totalSize = totalSize;
    uds_ota_ctx.blockSN = 0U;
    uds_ota_ctx.totalOffset = 0U;
    uds_ota_ctx.packageOffset = 0U;
    uds_ota_ctx.packSN = 0U;
    uds_ota_ctx.blockSize = blockSize;

    uds_ctx.uds_process_status = UDS_PROC_STATUS_TRANS_CFG;
    uds_ctx.gBuffer[0U] = UDS_ERR_CODE_SUCCESS;
    uds_res_append_and_send(UDS_CMD_TRANSFER_CONFIG,uds_ctx.gBuffer,1U);
}

void uds_cmd_transfer_data(u8* data,u16 length)
{
    u16 blockSn;
    u16 packSn;
    u16 totalBlockSN;
    u16 totalPackSN;
    u16 packLeftSize;
    u8* packgeBuf;
    u16 lastBlockSize;
    u16 lastPackSize;

    u8 flag = 0U;
    if ((uds_ctx.uds_ensure_status != UDS_ENSURE_STATUS_ENSURED)||(uds_ctx.uds_process_status != UDS_PROC_STATUS_TRANS_CFG))
    {
        uds_ctx.gBuffer[0U] = UDS_ERR_CODE_ERR_PROCESS;
        uds_res_append_and_send(UDS_CMD_TRANSFER_DATA,uds_ctx.gBuffer,1U);
        return ;
    }
    blockSn = core_dcm_readBig16(data+1U);
    packSn = core_dcm_readU8(data+3U);
    packgeBuf = data+4U;

    lastBlockSize = uds_ota_ctx.totalSize%uds_ota_ctx.blockSize;
    lastPackSize = lastBlockSize%OTA_PACK_SIZE;

    totalPackSN = (uds_ota_ctx.blockSize + OTA_PACK_SIZE - 1U)/OTA_PACK_SIZE;
    totalBlockSN = (uds_ota_ctx.totalSize + uds_ota_ctx.blockSize - 1U)/uds_ota_ctx.blockSize;

    /*判断当前块号是否合法*/
    if (blockSn != uds_ota_ctx.blockSN)
    {
        uds_ctx.gBuffer[0U] = UDS_ERR_CODE_ERR_BLOCK;
        uds_res_append_and_send(UDS_CMD_TRANSFER_DATA,uds_ctx.gBuffer,1U);
        return ;
    }
    /*判断当前包号是否合法*/
    if ((packSn&0x7FU) != uds_ota_ctx.packSN)
    {
        uds_ctx.gBuffer[0U] = UDS_ERR_CODE_ERR_PACKEAGE;
        uds_res_append_and_send(UDS_CMD_TRANSFER_DATA,uds_ctx.gBuffer,1U);
        return ;
    }

    // lastBlockSize = uds_ota_ctx.totalSize%uds_ota_ctx.blockSize;
    // lastPackSize = lastBlockSize%OTA_PACK_SIZE;

    // totalPackSN = (uds_ota_ctx.blockSize + OTA_PACK_SIZE - 1U)/OTA_PACK_SIZE;
    // totalBlockSN = (uds_ota_ctx.totalSize + uds_ota_ctx.blockSize - 1U)/uds_ota_ctx.blockSize;

    if ((totalBlockSN - 1U) == blockSn)
    {	/*最后一块数据传输*/
        if ((packSn&0x80U) == 0x80U)
        {	/*最后一块的最后一包数据*/
            core_mm_copy(uds_ota_ctx.blockBuf+uds_ota_ctx.packSN*OTA_PACK_SIZE,packgeBuf,lastPackSize);
            /*调用写接口*/
            UDS_ext_write_data(uds_ota_ctx.startAddress+uds_ota_ctx.blockSN*uds_ota_ctx.blockSize,uds_ota_ctx.blockSize,uds_ota_ctx.blockBuf);
            uds_ota_ctx.blockSN++;
            uds_ota_ctx.packSN = 0U;
            flag = 1U;
            uds_ctx.uds_process_status = UDS_PROC_STATUS_TRANS_DATA;
        }
        else
        {
            core_mm_copy(uds_ota_ctx.blockBuf+uds_ota_ctx.packSN*OTA_PACK_SIZE,packgeBuf,OTA_PACK_SIZE);
            uds_ota_ctx.packSN++;
        }
    }
    else
    {	/*中间块数据传输*/
        /*当前包号达到最大，表示一包数据已传完*/
        if ((packSn&0x7FU) == (totalPackSN - 1U))
        {
            packLeftSize = uds_ota_ctx.blockSize%OTA_PACK_SIZE;
            core_mm_copy(uds_ota_ctx.blockBuf+uds_ota_ctx.packSN*OTA_PACK_SIZE,packgeBuf,packLeftSize);
            /*调用写接口*/
            UDS_ext_write_data(uds_ota_ctx.startAddress+uds_ota_ctx.blockSN*uds_ota_ctx.blockSize,uds_ota_ctx.blockSize,uds_ota_ctx.blockBuf);
            uds_ota_ctx.blockSN++;
            uds_ota_ctx.packSN = 0U;
            flag = 1U;
            core_mm_set(uds_ota_ctx.blockBuf,0xFF,FLASH_PAGE_SIZE);
        }
        else
        {
            core_mm_copy(uds_ota_ctx.blockBuf+uds_ota_ctx.packSN*OTA_PACK_SIZE,packgeBuf,OTA_PACK_SIZE);
            uds_ota_ctx.packSN++;
        }
    }


    if(flag)
    {
    	uds_ctx.gBuffer[0U] = UDS_ERR_CODE_SUCCESS;
    	uds_ctx.gBuffer[1U] = blockSn;
    	uds_res_append_and_send(UDS_CMD_TRANSFER_DATA,uds_ctx.gBuffer,2U);
    }
}

void uds_cmd_transfer_end(u8* data,u16 length)
{
    u8 stats;
    u32 readCrc32;
    u32 calCrc32;
    
    if ((uds_ctx.uds_ensure_status != UDS_ENSURE_STATUS_ENSURED)&&(uds_ctx.uds_process_status != UDS_PROC_STATUS_TRANS_DATA))
    {
        uds_ctx.gBuffer[0U] = UDS_ERR_CODE_ERR_PROCESS;
        uds_res_append_and_send(UDS_CMD_TRANSFER_END,uds_ctx.gBuffer,1U);
        return ;
    }
    readCrc32 = core_dcm_readBig32(data+1U);

    calCrc32 = UDS_ext_calc_crc32((u8*)uds_ota_ctx.startAddress,uds_ota_ctx.totalSize);
    if (readCrc32 != calCrc32)
    {
        uds_ctx.gBuffer[0U] = UDS_ERR_CODE_ERR_CRC;
        uds_res_append_and_send(UDS_CMD_TRANSFER_END,uds_ctx.gBuffer,1U);
        return ;
    }
    UDS_set_appFlag_valid(1U);
    uds_ctx.gBuffer[0U] = UDS_ERR_CODE_SUCCESS;
    uds_res_append_and_send(UDS_CMD_TRANSFER_END,uds_ctx.gBuffer,1U);
}
