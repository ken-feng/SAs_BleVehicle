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
#include "UDS_ext_api.h"
#include "crc32.h"
#include "bl_memory.h"
#include "bl_context.h"
#include "bl_flash_memory.h"
#include "bl_dflash_memory.h"
#include "BCanPduCfg.h"
/* Macros & Typedef*************************************************/

/*Imported Interfaces***********************************************/

/*Enumerations & Structures*****************************************/

/*Variable Definitions**********************************************/

/* Public Function Prototypes***************************************/

void UDS_Jump_To_App(void)
{
    Dcm_FblJumpToApplication();
}

u8 UDS_get_appmode_valid(void)
{
    /*调用获取App模式接口*/
    u8 blMode;
    blMode = core_dcm_readU8((u8*)(kBootloaderConfigAreaAddress + BL_Mode));
    return blMode;
}

u8 UDS_set_appmode_valid(u8 flag)
{
    /*调用设置App模式接口*/
    u8 status;
    u8 tmpBuf[0x100U];
    core_mm_copy(tmpBuf,kBootloaderConfigAreaAddress,0x100U);
    status = flash_mem_erase(kBootloaderConfigAreaAddress,0x800U);
    if(status != 0x00)
    {
    	return status;
    }
    core_dcm_writeU8(tmpBuf+BL_Mode,flag);
    status = flash_mem_write(kBootloaderConfigAreaAddress,0x100U,tmpBuf);
    return status;
}

u8 UDS_get_appFlag_valid(void)
{
    /*调用获取App模式接口*/
    u8 appValidFlag;
    appValidFlag = core_dcm_readU8((u8*)(kBootloaderConfigAreaAddress + BL_AppValidFlag));
	return appValidFlag;
}

u8 UDS_set_appFlag_valid(u8 flag)
{
    /*调用设置App模式接口*/
	u8 status;
	u8 tmpBuf[0x100U];
    core_mm_copy(tmpBuf,kBootloaderConfigAreaAddress,0x100U);
    status = flash_mem_erase(kBootloaderConfigAreaAddress,0x800U);
    if(status != 0x00)
    {
    	return status;
    }
    core_dcm_writeU8(tmpBuf+BL_AppValidFlag,flag);
    status = flash_mem_write(kBootloaderConfigAreaAddress,0x100U,tmpBuf);
    return status;
}
u8 UDS_get_appSwitchReturn_Flag(void)
{
    /*调用获取App模式接口*/
    u8 appValidFlag = 0U;
    appValidFlag = core_dcm_readU8((u8*)(kBootloaderConfigAreaAddress + BL_SwitchRtn));
	return appValidFlag;
}

u8 UDS_set_appSwitchReturn_Flag(u8 flag)
{
    /*调用设置App模式接口*/
	u8 status;
	u8 tmpBuf[0x100U];
    core_mm_copy(tmpBuf,kBootloaderConfigAreaAddress,0x100U);
    status = flash_mem_erase(kBootloaderConfigAreaAddress,0x800U);
    if(status != 0x00)
    {
    	return status;
    }
    core_dcm_writeU8(tmpBuf+BL_SwitchRtn,flag);
    status = flash_mem_write(kBootloaderConfigAreaAddress,0x100U,tmpBuf);
    return status;
    return status;
}

void UDS_ext_reset_system(void)
{
    /*调用芯片复位API*/
    NVIC_SystemReset();
    while (1);
}

u8 UDS_ext_erase_is_inFlash(u32 startAddress)
{
    /*判断当前地址是否在flash范围内*/
    if(bl_mem_in_memory(startAddress))
    {
        return 1U;
    }
    return 0U;
}

u8 UDS_ext_erase_area(u32 startAddress,u32 size)
{
    /*调用擦除接口api*/
    bl_mem_erase(startAddress,size);
    return 1U;
}

u8 UDS_ext_write_data(u32 startAddress,u32 size, u8* buffer)
{
    /*调用写接口*/
    bl_mem_write(startAddress,size,buffer);
    return 1U;
}

u32 UDS_ext_calc_crc32(u8* buf,u32 size)
{
    /*调用CRC32接口*/
    u32 crc;
    crc = core_algo_iso3309_crc32(buf,size);
    return crc;
}

void UDS_ext_send_data(u8* buf, u16 length)
{
    BCanPdu_Set_OBD639_Data(buf);
}
