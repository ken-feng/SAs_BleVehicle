/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_device_registers.h"
#include "fsl_common.h"
#include "fsl_flash.h"
#include "bl_flash_memory.h"
#include "bl_ram_memory.h"
#include "bl_dflash_memory.h"
#include "bl_memory.h"
#include "bl_context.h"
#include "bootloader_core.h"
#include "Board_Config_Para.h"
#include "bootloader_config.h"

// See bl_app_crc_check.h for documentation on this function.
uint8_t is_application_crc_check_pass(void)
{
	uint8_t isCrcCheckPassed = 1;
    uint8_t appValidFlag;

	if(kStatus_Success != flash_mem_read_in_rom(kBootloaderConfigAreaAddress + BL_AppValidFlag,1,&appValidFlag))
	{
		return 0;
	}
	if(appValidFlag != 1)
	{
		return 0;
	}
    return isCrcCheckPassed;
}

void bl_check_app_jump(void)
{  /*1: app valid   other: in boot*/
    uint8_t blMode = 0;
    uint8_t rootMode;
     if(kStatus_Success != flash_mem_read_in_rom(kBootloaderConfigAreaAddress + BL_Mode,1,&blMode))
     {
         return;
     }
    if(blMode == 0x01)/*appMode*/
    {
        if(is_application_crc_check_pass())/*检查appValid标志是否有效*/
        {
            Dcm_FblJumpToApplication();
            while(1);
        }
    }  
}


void init_interrupts(void)
{
    // Clear any IRQs that may be enabled, we only want the IRQs we enable to be active
    NVIC_ClearEnabledIRQs();

    // Clear any pending IRQs that may have been set
    NVIC_ClearAllPendingIRQs();
}
// See bl_shutdown_cleanup.h for documentation of this function.
void shutdown_cleanup(shutdown_type_t shutdown)
{
    // If we are permanently exiting the bootloader, there are a few extra things to do.
    if (shutdown == kShutdownType_Shutdown)
    {
        // Turn off global interrupt
        __disable_irq();
        init_interrupts();
        // Set the VTOR to default.
        SCB->VTOR = (uint32_t)kDefaultVectorTableAddress;
        // // De-initialize hardware such as disabling port clock gate
        // deinit_hardware();
        // Restore global interrupt.
        __enable_irq();
    }
    // Memory barriers for good measure.
    __ISB();
    __DSB();
}
/*
uint8_t bl_enterBootMode(void)
{
    uint8_t status;
    uint8_t tmpBuf[0x800];
    flash_mem_read_in_rom(kBootloaderConfigAreaAddress,0x800,tmpBuf);
    status = flash_mem_erase_in_rom(kBootloaderConfigAreaAddress,0x800);
    if(status != kStatus_Success)
    {
    	return status;
    }
    core_dcm_writeU8(tmpBuf+BL_Mode,0x00);
    status = flash_mem_write_in_rom(kBootloaderConfigAreaAddress,0x800,tmpBuf);
    if(status != kStatus_Success)
    {
    	return status;
    }
    return status;
}
*/
void bl_clr_reProgramMode(void)
{
    
    uint8_t status;
    uint8_t tmpBuf[0x100];
    flash_mem_read_in_rom(kBootloaderConfigAreaAddress,0x100,tmpBuf);
    status = flash_mem_erase_in_rom(kBootloaderConfigAreaAddress,0x100);
    if(status != kStatus_Success)
    {
    	return status;
    }
    core_dcm_writeU8(tmpBuf+BL_Mode,1);
    status = flash_mem_write_in_rom(kBootloaderConfigAreaAddress,0x100,tmpBuf);
    if(status != kStatus_Success)
    {
    	return status;
    }
    
   //programFlag = (uint8_t*)PROGRAM_FLAG_RAM_ADDR;
   //*programFlag = 0x01;
   return;
}
uint8_t bl_setBootloaderConfig(uint32_t startAddress, uint32_t size, uint32_t crc)
{
    uint8_t status;
    uint8_t tmpBuf[0x100];
    flash_mem_read_in_rom(kBootloaderConfigAreaAddress,0x100,tmpBuf);
    status = flash_mem_erase_in_rom(kBootloaderConfigAreaAddress,0x100);
    if(status != kStatus_Success)
    {
    	return status;
    }
    core_dcm_writeU8(tmpBuf+BL_Mode,1);
    core_dcm_writeU8(tmpBuf+BL_AppValidFlag,1);
    // core_dcm_writeBig32(tmpBuf+BL_AppStartAddress,startAddress);
    // core_dcm_writeBig32(tmpBuf+BL_AppSize,size);
    // core_dcm_writeBig32(tmpBuf+BL_AppCrc,crc);
    status = flash_mem_write_in_rom(kBootloaderConfigAreaAddress,0x100,tmpBuf);
    if(status != kStatus_Success)
    {
    	return status;
    }
    return status;
}
uint8_t bl_clearAppValidFlag(void)
{
    uint8_t status;
    uint8_t tmpBuf[0x100];
    flash_mem_read_in_rom(kBootloaderConfigAreaAddress,0x100,tmpBuf);
    status = flash_mem_erase_in_rom(kBootloaderConfigAreaAddress,0x100);
    if(status != kStatus_Success)
    {
    	return status;
    }
    core_dcm_writeU8(tmpBuf+BL_AppValidFlag,0);
    // core_dcm_writeBig32(tmpBuf+BL_AppStartAddress,0x00000000);
    // core_dcm_writeBig32(tmpBuf+BL_AppSize,0x00000000);
    // core_dcm_writeBig32(tmpBuf+BL_AppCrc,0x00000000);
    status = flash_mem_write_in_rom(kBootloaderConfigAreaAddress,0x100,tmpBuf);
    if(status != kStatus_Success)
    {
    	return status;
    }
    return status;
}

uint8_t bl_isBLModeFlag(void)
{
    uint8_t blMode;
    //programFlag = (uint8_t*)PROGRAM_FLAG_RAM_ADDR;
    if(kStatus_Success != flash_mem_read_in_rom(kBootloaderConfigAreaAddress + BL_Mode,1,&blMode))
    {
        return 0;
    }
    //blMode = *programFlag;
    if(blMode == 0x02)
    {
        return 1;
    }
    return 0;
}
uint8_t bl_GetEepromInitFlag(void)
{
    // uint8_t flag;
    // if(kStatus_Success != flash_mem_read_in_rom(kBootloaderConfigAreaAddress + BL_EepromInitFlag,1,&flag))
    // {
        return 0;
    // }
    // return flag;
}
/********************************************************************************************************
  Function Name:
  Build Author :
  Creat Date   : 2019-10-10
  Call         : --
  Called By    :
  Input        : --
  Output       : None
  Return Value : None
  Description  : 程序跳转接口
********************************************************************************************************/
void Dcm_FblJumpToApplication(void)
{
    uint32_t appEntry;
    uint32_t appStack;
    appEntry = APP_VECTOR_TABLE[(uint32_t)kInitialPC];
    appStack = APP_VECTOR_TABLE[(uint32_t)kInitialSP];

    shutdown_cleanup(kShutdownType_Shutdown);

    // Create the function call to the user application.
    // Static variables are needed since changed the stack pointer out from under the compiler
    // we need to ensure the values we are using are not stored on the previous stack
    static uint32_t s_stackPointer = 0u;
    s_stackPointer = appStack;
    static void (*farewellBootloader)(void) = (void*)0u;
    union
    {
        uint32_t address;
        void (*function)(void);
    } func_ptr;
    func_ptr.address = appEntry;
    farewellBootloader = func_ptr.function;

    // Set the VTOR to the application vector table address.
    SCB->VTOR = (uint32_t)APP_VECTOR_TABLE;

    // Set stack pointers to the application stack pointer.
    __set_MSP(s_stackPointer);
    __set_PSP(s_stackPointer);

    // Jump to the application.
    farewellBootloader();
    // Dummy fcuntion call, should never go to this fcuntion call
    //shutdown_cleanup(kShutdownType_Shutdown);
}
