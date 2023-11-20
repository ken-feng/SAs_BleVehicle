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

const flash_driver_interface_t g_flashDriverInterface = {
    {.version = FSL_FLASH_DRIVER_VERSION},
    .flash_init = FLASH_Init,
    .flash_erase_all = FLASH_EraseAll,
    .flash_erase_all_unsecure = NULL,
    .flash_erase = FLASH_Erase,
    .flash_program = FLASH_Program,
    .flash_get_security_state = FLASH_GetSecurityState,
    .flash_security_bypass = FLASH_SecurityBypass,
    .flash_verify_erase_all = FLASH_VerifyEraseAll,
    .flash_verify_erase = FLASH_VerifyErase,
    .flash_verify_program = FLASH_VerifyProgram,
    .flash_get_property = FLASH_GetProperty,
#if !(defined(BL_FEATURE_MIN_PROFILE) && BL_FEATURE_MIN_PROFILE)
    .flash_program_once = FTFx_CMD_ProgramOnce,
    .flash_read_once = FTFx_CMD_ReadOnce,
#if defined(FSL_FEATURE_FLASH_HAS_READ_RESOURCE_CMD) && FSL_FEATURE_FLASH_HAS_READ_RESOURCE_CMD
    .flash_read_resource = FLASH_ReadResource,
#else
    .flash_read_resource = NULL,
#endif
#else
    .flash_program_once = NULL,
    .flash_read_once = NULL,
    .flash_read_resource = NULL,
#endif
    .flash_erase_all_execute_only_segments = NULL,
    .flash_verify_erase_all_execute_only_segments = NULL,
#if defined(BL_IS_FLASH_SECTION_PROGRAMMING_ENABLED) && BL_IS_FLASH_SECTION_PROGRAMMING_ENABLED
#if defined(FSL_FEATURE_FLASH_HAS_SET_FLEXRAM_FUNCTION_CMD) && FSL_FEATURE_FLASH_HAS_SET_FLEXRAM_FUNCTION_CMD
    .flash_set_flexram_function = FLASH_SetFlexramFunction,
#else
    .flash_set_flexram_function = NULL,
#endif
    .flash_program_section = FLASH_ProgramSection,
#else
    .flash_set_flexram_function = NULL,
    .flash_program_section = NULL,
#endif
};



const dflash_driver_interface_t g_dflashDriverInterface = {
    .version = {.name = 'F',
                .major = 3u,
                .minor = 0u,
                .bugfix = 0u },
    .flash_init = FLEXNVM_Init,
    .flash_erase_all = FLEXNVM_EraseAll,
#if defined(BL_FEATURE_ERASEALL_UNSECURE) && BL_FEATURE_ERASEALL_UNSECURE
    .flash_erase_all_unsecure = FLEXNVM_EraseAllUnsecure,
#else
    .flash_erase_all_unsecure = NULL,
#endif
    .flash_erase = FLEXNVM_DflashErase,
    .flash_program = FLEXNVM_DflashProgram,
    .flash_get_security_state = FLEXNVM_GetSecurityState,
    .flash_security_bypass = FLEXNVM_SecurityBypass,
    .flash_verify_erase_all = FLEXNVM_VerifyEraseAll,
    .flash_verify_erase = FLEXNVM_DflashVerifyErase,
    .flash_verify_program = FLEXNVM_DflashVerifyProgram,
    .flash_get_property = FLEXNVM_GetProperty,
#if !(defined(BL_FEATURE_MIN_PROFILE) && BL_FEATURE_MIN_PROFILE)
    .flash_program_once = NULL,  //FTFx_CMD_ProgramOnce,
    .flash_read_once = NULL, //FTFx_CMD_ReadOnce,
#if defined(FSL_FEATURE_FLASH_HAS_READ_RESOURCE_CMD) && FSL_FEATURE_FLASH_HAS_READ_RESOURCE_CMD
    .flash_read_resource = FLEXNVM_ReadResource,
#else
    .flash_read_resource = NULL,
#endif
#else
    .flash_program_once = NULL,
    .flash_read_once = NULL,
    .flash_read_resource = NULL,
#endif
#if defined(FSL_FEATURE_FLASH_HAS_ACCESS_CONTROL) && FSL_FEATURE_FLASH_HAS_ACCESS_CONTROL
    .flash_is_execute_only = NULL,
#endif
    .flash_erase_all_execute_only_segments = NULL,
    .flash_verify_erase_all_execute_only_segments = NULL,
#if defined(BL_IS_FLASH_SECTION_PROGRAMMING_ENABLED) && BL_IS_FLASH_SECTION_PROGRAMMING_ENABLED
#if defined(FSL_FEATURE_FLASH_HAS_SET_FLEXRAM_FUNCTION_CMD) && FSL_FEATURE_FLASH_HAS_SET_FLEXRAM_FUNCTION_CMD
    .flash_set_flexram_function = FLEXNVM_SetFlexramFunction,
#else
    .flash_set_flexram_function = NULL,
#endif
    .flash_program_section = FLEXNVM_DflashProgramSection,
#else
    .flash_set_flexram_function = NULL,
    .flash_program_section = NULL,
#endif
};



//! @brief Interface to main flash memory operations.
const memory_region_interface_t g_flashMemoryInterface = {
    .init = &flash_mem_init,
    .read = &flash_mem_read,
    .write = &flash_mem_write,
    .fill = &flash_mem_fill,
#if defined(BL_IS_FLASH_SECTION_PROGRAMMING_ENABLED) && BL_IS_FLASH_SECTION_PROGRAMMING_ENABLED
    .flush = flash_mem_flush,
#else
    .flush = (void *)0u,
#endif
    .erase = flash_mem_erase,
    .eraseInrom = flash_mem_erase_in_rom,
    .eraseAll = flash_mem_erase_all,
};

const memory_region_interface_t g_normalMemoryInterface = {
    .init = &normal_mem_init,
    .read = &normal_mem_read,
    .write = &normal_mem_write,
    //.fill = &normal_mem_fill,
    .fill = (void *)0u,
    .flush = (void *)0u,
    .erase = &normal_mem_erase,
    .eraseInrom = &normal_mem_erase,
    .eraseAll = (void *)0u,};

//! @brief Interface to dflash memory operations.
const memory_region_interface_t g_dFlashMemoryInterface = {
    .init = &flexNVM_mem_init,
    .read = &flexNVM_mem_read,
    .write = &flexNVM_mem_write,
#if !(defined(BL_FEATURE_MIN_PROFILE) && BL_FEATURE_MIN_PROFILE) || \
    defined(BL_FEATURE_FILL_MEMORY) && BL_FEATURE_FILL_MEMORY
    .fill = &flexNVM_mem_fill,
#endif // !BL_FEATURE_MIN_PROFILE
#if defined(BL_IS_FLASH_SECTION_PROGRAMMING_ENABLED) && BL_IS_FLASH_SECTION_PROGRAMMING_ENABLED
    .flush = flexNVM_mem_flush,
#else
    .flush = (void *)0u,
#endif
    .erase = flexNVM_mem_erase,
    .eraseInrom = flexNVM_mem_erase_in_rom,
    .eraseAll = flexNVM_mem_erase_all,
};

memory_map_entry_t g_memoryMap[] = {
    { 0x00010000u, 0x0003ffffu, (uint32_t)kMemoryIsExecutable | (uint32_t)kMemoryType_FLASH, &g_flashMemoryInterface },   // Flash array (256KB)
    { 0x1fffc000u, 0x2000bfffu, (uint32_t)kMemoryIsExecutable | (uint32_t)kMemoryType_RAM, &g_normalMemoryInterface },  // SRAM (64KB)
	{ 0x00040000u, 0x00077fffu, (uint32_t)kMemoryIsExecutable | (uint32_t)kMemoryType_DFLASH | (uint32_t)kMemoryAliasAddr, &g_dFlashMemoryInterface }, // DFlash array (256KB)
    { 0u }                                                                       // Terminator
};

static uint32_t s_regPrimask = 0U;

void flash_lock_acquire(void)
{
    // Ensure that the program operation cannots be interrupted.
    s_regPrimask = __get_PRIMASK();
    __disable_irq();
}

void flash_lock_release(void)
{
    // Release lock after the write operation completes.
    __set_PRIMASK(s_regPrimask);
}

extern flash_config_t g_allFlashState;
extern ftfx_cache_config_t g_allFlashCacheState;
extern flexnvm_config_t g_dFlashState;

status_t bl_mem_init(void)
{
	status_t status = (int32_t)kStatus_Success;
    g_flashDriverInterface.flash_init(&g_allFlashState);
    (void)FTFx_CACHE_Init(&g_allFlashCacheState);
    g_dflashDriverInterface.flash_init(&g_dFlashState);
    
    const memory_map_entry_t *map = &g_memoryMap[0];
    while (map->memoryInterface != (void *)0u)
    {
        if (map->memoryInterface->init != (void *)0u)
        {
            (void)map->memoryInterface->init();
        }
        ++map;
    }
    return status;
}
status_t bl_mem_read(uint32_t address, uint32_t length, uint8_t *buffer)
{
    uint32_t lessLenth;
    uint32_t tmpAddr;
    uint8_t* tmpBuf;
    tmpAddr = address;
    lessLenth = length;
    tmpBuf = buffer;
    uint32_t status = (uint32_t)kStatus_FLASH_Success;
    const memory_map_entry_t *map = &g_memoryMap[0];
    
    while (map->memoryInterface != (void *)0u)
    {
        if ((tmpAddr >= map->startAddress) && (tmpAddr<= map->endAddress))
        {
            if ((tmpAddr+lessLenth)<= (map->endAddress+1))
            {
                status = map->memoryInterface->read(tmpAddr,lessLenth,tmpBuf);
                return status;
            }
            else
            {
                status = map->memoryInterface->read(tmpAddr,map->endAddress - tmpAddr + 1,tmpBuf);
                if (status != kStatus_Success)
                {
                    return status;
                }
                tmpBuf +=  map->endAddress - tmpAddr + 1;
                lessLenth = tmpAddr + lessLenth - (map->endAddress + 1);
                tmpAddr +=  map->endAddress - tmpAddr + 1;
            }                 
        }
        ++map;
    }
    return status;
}
status_t bl_mem_write(uint32_t address, uint32_t length, uint8_t *buffer)
{
    uint32_t lessLenth;
    uint32_t tmpAddr;
    uint8_t* tmpBuf;
    tmpAddr = address;
    lessLenth = length;
    tmpBuf = buffer;
    uint32_t status = (uint32_t)kStatus_FLASH_Success;
    const memory_map_entry_t *map = &g_memoryMap[0];

    while (map->memoryInterface != (void *)0u)
    {
        if ((tmpAddr >= map->startAddress) && (tmpAddr<= map->endAddress))
        {
            if ((tmpAddr+lessLenth)<= (map->endAddress+1))
            {
                status = map->memoryInterface->write(tmpAddr,lessLenth,tmpBuf);
                return status;
            }
            else
            {
                status = map->memoryInterface->write(tmpAddr,map->endAddress - tmpAddr + 1,tmpBuf);
                if (status != kStatus_Success)
                {
                    return status;
                }
                tmpBuf +=  map->endAddress - tmpAddr + 1;
                lessLenth = tmpAddr + lessLenth - (map->endAddress + 1);
                tmpAddr +=  map->endAddress - tmpAddr + 1;

            }          
        }
        ++map;
    }
    return status;
}
status_t bl_mem_fill(uint32_t address, uint32_t length, uint32_t pattern)
{
    uint32_t status = (uint32_t)kStatus_FLASH_Success;
    return status;
}

status_t bl_mem_erase(uint32_t address, uint32_t length)
{
    uint32_t lessLenth;
    uint32_t tmpAddr;
    lessLenth = length;
    tmpAddr = address;
    uint32_t status = (uint32_t)kStatus_FLASH_Success;
    const memory_map_entry_t *map = &g_memoryMap[0];
    
    while (map->memoryInterface != (void *)0u)
    {
        if ((tmpAddr >= map->startAddress) && (tmpAddr<= map->endAddress))
        {
            if ((tmpAddr+lessLenth)<= (map->endAddress+1))
            {
                status = map->memoryInterface->eraseInrom(tmpAddr,lessLenth);
                return status;
            }
            else
            {
                status = map->memoryInterface->eraseInrom(tmpAddr,map->endAddress - tmpAddr + 1);
                if (status != kStatus_Success)
                {
                    return status;
                }
                lessLenth = tmpAddr + lessLenth - (map->endAddress + 1);
                tmpAddr +=  map->endAddress - tmpAddr + 1;
            }
        }
        ++map;
    }
    return status;
}
status_t bl_mem_erase_all(void)
{
    uint32_t status = (uint32_t)kStatus_FLASH_Success;
    const memory_map_entry_t *map = &g_memoryMap[0];
    while (map->memoryInterface != (void *)0u)
    {
        if (map->memoryInterface->eraseAll != 0)
        {
            status = map->memoryInterface->eraseAll();
            if (status != kStatus_FLASH_Success)
            {
                return status;
            }
        }
        ++map;
    }
    return status;
}

uint8_t bl_mem_in_memory(uint32_t address)
{
    uint32_t status = (uint32_t)kStatus_FLASH_Success;
    const memory_map_entry_t *map = &g_memoryMap[0];
    while (map->memoryInterface != (void *)0u)
    {
        if ((address >= map->startAddress) && (address<= map->endAddress))
        {
            return 1;
        }
        ++map;
    }
    return 0;
}

/*
void bootloader_flash_test(void)
{
    uint32_t startAddr = 0x3FFF0;
    uint32_t size = 128;
    uint32_t endAddr = startAddr + size;
    
    uint8_t writeBuffer[128];
    uint8_t readBuffer[128];
    for (uint8_t i = 0; i < 128; i++)
    {
        writeBuffer[i] = i;
        readBuffer[i] = 0;
    }
    bl_mem_erase(startAddr,size);
	bl_mem_write(startAddr,128,writeBuffer);
	bl_mem_read(startAddr,128,readBuffer);
    //bl_mem_erase_all();
//    for (uint32_t addr = startAddr; addr < endAddr; addr+=128)
//    {
//    	bl_mem_write(addr,128,writeBuffer);
//    	bl_mem_read(addr,128,readBuffer);
//        if (memcmp(writeBuffer,readBuffer,128))
//        {
//
//        }
//        memset(readBuffer,0x00,128);
//    }
//
    



}
*/
