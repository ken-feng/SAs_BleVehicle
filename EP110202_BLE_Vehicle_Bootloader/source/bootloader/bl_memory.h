/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _BL_MEMORY_H_
#define _BL_MEMORY_H_
/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_device_registers.h"
#include "fsl_common.h"
#include "fsl_flash.h"
#include "bootloader_config.h"
#include "target_config.h"
#include "bootloader_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

// !@brief Memory property enum codes
enum
{
    /* Memory property bitfield definition.
     * Bit[0]: 0 -- NotExecutable( None-XIP)
     *         1 -- Executable(XIP)
     * Bit[1] : Reserved.(Reserved for Int/Ext. 0 Internal, 1 External)
     * Bit[3:2] : Reserved.
     * Bit[7:4] : 0000 -- FLASH.
     *            0001 -- RAM.
     *            0010 -- Device.
     *            0100 -- Reserved.
     *            1000 -- Reserved.
     * Bit[15-8] : Reserved.
     * Bit[16] : Reserved.(Reserved for Bufferable. 0 Not bufferable, 1 Bufferable)
     * Bit[17] : Reserved.(Reserved for Cacheable. 0 Not cacheable, 1 Cacheable)
     * Bit[18] : Reserved.(Reserved for Shareable. 0 Not shareable, 1 Shareable)
     * Bit[31-19] : Reserved.
     */
    kMemoryNotExecutable = 0u,  //!< The memory doesn't support executing in place.
    kMemoryIsExecutable = 1u,   //!< The memory supports executing in place.
    kMemoryType_FLASH = 0x00u,  //!< The memory is FLASH device
    kMemoryType_RAM = 0x10u,    //!< The memory is RAM device
    kMemoryType_Device = 0x20u, //!< The memory is device register
    kMemoryAliasAddr = 0x100u,  //!< This memory map is alias memory
    kMemoryType_DFLASH = 0x200u,  //!< The memory is FLASH device
};


//! @brief Interface to memory operations for one region of memory.
typedef struct _memory_region_interface
{
    status_t (*init)(void);
    status_t (*read)(uint32_t address, uint32_t length, uint8_t *buffer);
    status_t (*write)(uint32_t address, uint32_t length, const uint8_t *buffer);
    status_t (*fill)(uint32_t address, uint32_t length, uint32_t pattern);
    status_t (*flush)(void);
    status_t (*erase)(uint32_t address, uint32_t length);
    status_t (*eraseInrom)(uint32_t address, uint32_t length);
    status_t (*eraseAll)(void);
} memory_region_interface_t;

//! @brief Structure of a memory map entry.
typedef struct _memory_map_entry
{
    uint32_t startAddress;
    uint32_t endAddress;
    uint32_t memoryProperty;
    const memory_region_interface_t *memoryInterface;
} memory_map_entry_t;

void bootloader_flash_init(void);

//! @brief Memory Map index constants
enum _memorymap_constants
{
    kIndexFlashArray = 0u,
    kIndexSRAM = 1u,
    kIndexDFlashArray = 2u,  
    kSRAMSeparatrix = (uint32_t)0x20000000u //!< This value is the start address of SRAM_U
};

typedef struct FlashDriverInterface
{
    standard_version_t version; //!< flash driver API version number.
    status_t (*flash_init)(flash_config_t *config);
    status_t (*flash_erase_all)(flash_config_t *config, uint32_t key);
    status_t (*flash_erase_all_unsecure)(flash_config_t *config, uint32_t key);
    status_t (*flash_erase)(flash_config_t *config, uint32_t start, uint32_t lengthInBytes, uint32_t key);
    status_t (*flash_program)(flash_config_t *config, uint32_t start, uint8_t *src, uint32_t lengthInBytes);
    status_t (*flash_get_security_state)(flash_config_t *config, ftfx_security_state_t *state);
    status_t (*flash_security_bypass)(flash_config_t *config, const uint8_t *backdoorKey);
    status_t (*flash_verify_erase_all)(flash_config_t *config, ftfx_margin_value_t margin);
    status_t (*flash_verify_erase)(flash_config_t *config,
                                   uint32_t start,
                                   uint32_t lengthInBytes,
                                   ftfx_margin_value_t margin);
    status_t (*flash_verify_program)(flash_config_t *config,
                                     uint32_t start,
                                     uint32_t lengthInBytes,
                                     const uint8_t *expectedData,
                                     ftfx_margin_value_t margin,
                                     uint32_t *failedAddress,
                                     uint32_t *failedData);
    status_t (*flash_get_property)(flash_config_t *config, flash_property_tag_t whichProperty, uint32_t *value);
    status_t (*flash_program_once)(ftfx_config_t *config, uint32_t index, const uint8_t *src, uint32_t lengthInBytes);
    status_t (*flash_read_once)(ftfx_config_t *config, uint32_t index, uint8_t *dst, uint32_t lengthInBytes);
    status_t (*flash_read_resource)(flash_config_t *config,
                                    uint32_t start,
                                    uint8_t *dst,
                                    uint32_t lengthInBytes,
                                    ftfx_read_resource_opt_t option);
    status_t (*flash_erase_all_execute_only_segments)(flash_config_t *config, uint32_t key);
    status_t (*flash_verify_erase_all_execute_only_segments)(flash_config_t *config, ftfx_margin_value_t margin);
    status_t (*flash_set_flexram_function)(flash_config_t *config, ftfx_flexram_func_opt_t option);
    status_t (*flash_program_section)(flash_config_t *config, uint32_t start, uint8_t *src, uint32_t lengthInBytes);
} flash_driver_interface_t;

//! @brief Flash index constants.
enum _flash_index_constants
{
    kFlashIndex_Main = 0u,
    kFalshIndex_DFlash = 1u 
};


typedef struct DFlashDriverInterface
{
    standard_version_t version; //!< flash driver API version number.
    status_t (*flash_init)(flexnvm_config_t *config);
    status_t (*flash_erase_all)(flexnvm_config_t *config, uint32_t key);
    status_t (*flash_erase_all_unsecure)(flexnvm_config_t *config, uint32_t key);
    status_t (*flash_erase)(flexnvm_config_t *config, uint32_t start, uint32_t lengthInBytes, uint32_t key);
    status_t (*flash_program)(flexnvm_config_t *config, uint32_t start, uint8_t *src, uint32_t lengthInBytes);
    status_t (*flash_get_security_state)(flexnvm_config_t *config, ftfx_security_state_t *state);
    status_t (*flash_security_bypass)(flexnvm_config_t *config, const uint8_t *backdoorKey);
    status_t (*flash_verify_erase_all)(flexnvm_config_t *config, ftfx_margin_value_t margin);
    status_t (*flash_verify_erase)(flexnvm_config_t *config,
                                   uint32_t start,
                                   uint32_t lengthInBytes,
                                   ftfx_margin_value_t margin);
    status_t (*flash_verify_program)(flexnvm_config_t *config,
                                     uint32_t start,
                                     uint32_t lengthInBytes,
                                     const uint8_t *expectedData,
                                     ftfx_margin_value_t margin,
                                     uint32_t *failedAddress,
                                     uint32_t *failedData);
    status_t (*flash_get_property)(flexnvm_config_t *config, flexnvm_property_tag_t whichProperty, uint32_t *value);
    status_t (*flash_program_once)(flexnvm_config_t *config, uint32_t index, uint8_t *src, uint32_t lengthInBytes);
    status_t (*flash_read_once)(flexnvm_config_t *config, uint32_t index, uint8_t *dst, uint32_t lengthInBytes);
    status_t (*flash_read_resource)(flexnvm_config_t *config,
                                    uint32_t start,
                                    uint8_t *dst,
                                    uint32_t lengthInBytes,
                                    ftfx_read_resource_opt_t option);
#if defined(FSL_FEATURE_FLASH_HAS_ACCESS_CONTROL) && FSL_FEATURE_FLASH_HAS_ACCESS_CONTROL
    status_t (*flash_is_execute_only)(flexnvm_config_t *config,
                                      uint32_t start,
                                      uint32_t lengthInBytes,
                                      flash_xacc_state_t *access_state);
#endif
    status_t (*flash_erase_all_execute_only_segments)(flexnvm_config_t *config, uint32_t key);
    status_t (*flash_verify_erase_all_execute_only_segments)(flexnvm_config_t *config, ftfx_margin_value_t margin);
    status_t (*flash_set_flexram_function)(flexnvm_config_t *config, ftfx_flexram_func_opt_t option);
    status_t (*flash_program_section)(flexnvm_config_t *config, uint32_t start, uint8_t *src, uint32_t lengthInBytes);
} dflash_driver_interface_t;


status_t bl_mem_init(void);
status_t bl_mem_read(uint32_t address, uint32_t length, uint8_t *buffer);
status_t bl_mem_write(uint32_t address, uint32_t length, uint8_t *buffer);
status_t bl_mem_fill(uint32_t address, uint32_t length, uint32_t pattern);
status_t bl_mem_flush(void);
status_t bl_mem_erase(uint32_t address, uint32_t length);
status_t bl_mem_erase_all(void);
uint8_t bl_mem_in_memory(uint32_t address);
void flash_lock_acquire(void);
void flash_lock_release(void);
#endif
