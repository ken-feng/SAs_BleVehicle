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
#include "fsl_ftfx_cache.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
flexnvm_config_t g_dFlashState;
extern ftfx_cache_config_t g_allFlashCacheState;
uint32_t g_dflashReadMargin = (uint32_t)kFTFx_MarginValueUser;
extern memory_map_entry_t g_memoryMap[];
extern const dflash_driver_interface_t g_dflashDriverInterface;
uint32_t g_dverifyWrites;
enum _flashNVM_memory_constants
{
    kFlashMemory_ErasedValue = ~0
};

typedef struct _flashNVM_section_program_info
{
    uint32_t startAddress;                      //!< This address is used to record the address which is used
                                                //!< to write the whole section into flash memory
    uint32_t storedBytes;                       //!< A variable which is used to indicate if the buffer is full.
    uint8_t buffer[0x400U]; //!< A buffer which is used to buffer a full section of data
} flashNVM_section_program_info_t;

static flashNVM_section_program_info_t s_dflash_section_program_info;

// See flash_memory.h for documentation on this function.
status_t flexNVM_mem_init(void)
{

        // Update address range of flash
        memory_map_entry_t *map;
        uint32_t tmp;

        union
        {
            memory_map_entry_t const *address;
            memory_map_entry_t *pMap;
        } map_ptr;

        map_ptr.address = &g_memoryMap[(uint32_t)kIndexDFlashArray];

        map = map_ptr.pMap;

//        (void)g_dflashDriverInterface.flash_get_property(&g_dFlashState,
//                                                        kFLEXNVM_PropertyDflashBlockBaseAddr,
//                                                        &map->startAddress);
//
//        (void)g_dflashDriverInterface.flash_get_property(&g_dFlashState,
//                                                        kFLEXNVM_PropertyDflashTotalSize,
//                                                        &tmp);
//
//        map->endAddress = map->startAddress + tmp - 1u;

        s_dflash_section_program_info.storedBytes = 0u;
    return (int32_t)kStatus_Success;
}

// See flash_memory.h for documentation on this function.
status_t flexNVM_mem_read_intern(uint32_t address, uint32_t length, uint8_t *buffer)
{
    status_t status = (int32_t)kStatus_Success;
    status = normal_mem_read(address, length, buffer);
    return status;
}


status_t flexNVM_mem_read(uint32_t address, uint32_t length, uint8_t *buffer)
{
    return flexNVM_mem_read_intern(address, length, buffer);
}


status_t flexNVM_mem_read_in_rom(uint32_t address, uint32_t length, uint8_t *buffer)
{
    status_t status = (int32_t)kStatus_Success;
    status = normal_mem_read(address, length, buffer);
    return status;
}





// See flash_memory.h for documentation on this function.
status_t flexNVM_mem_write_intern(uint32_t address, uint32_t length, const uint8_t *buffer)
{
    status_t status = (int32_t)kStatus_Success;

    // Note: the check for "length != 0" and "range not in reserved region" is done in mem_write().
    assert(length);
    assert(buffer);

    uint32_t alignedLength;
    uint32_t extraBytes;
    uint32_t extraData[2];
    uint32_t alignmentBaseUnit = g_dFlashState.ftfxConfig.opsConfig.addrAligment.blockWriteUnitSize;

    assert(sizeof(extraData) >= sizeof(uint8_t) * alignmentBaseUnit);

    bool verifyWrites;
    //status = flexNVM_check_access_before_programming(address, length, &verifyWrites);
    if (status == (int32_t)kStatus_Success)
    {
        // Align length to whole words.
        alignedLength = ALIGN_DOWN(length, sizeof(uint8_t) * alignmentBaseUnit);
        extraBytes = length - alignedLength;
        assert(extraBytes < sizeof(uint8_t) * alignmentBaseUnit);

        // Pre-fill word buffer with flash erased value.
        extraData[0] = (uint32_t)kFlashMemory_ErasedValue;
        extraData[1] = (uint32_t)kFlashMemory_ErasedValue;
        if (extraBytes != 0u)
        {
            // Copy extra bytes to word buffer.
            (void)memcpy((uint8_t *)extraData, &buffer[alignedLength], extraBytes);
        }

        flash_lock_acquire();
        (void)FTFx_CACHE_ClearCachePrefetchSpeculation(&g_allFlashCacheState, true);
        // Program whole words from the user's buffer.
        if (alignedLength != 0u)
        {
            status = g_dflashDriverInterface.flash_program(&g_dFlashState, address,
                                                                                (uint8_t *)buffer, alignedLength);
        }
        if ((status == (int32_t)kStatus_Success) && extraBytes != 0u)
        {
            // Program trailing word.
            status = g_dflashDriverInterface.flash_program(
                &g_dFlashState, address + alignedLength, (uint8_t *)extraData,
                alignmentBaseUnit);
        }
        (void)FTFx_CACHE_ClearCachePrefetchSpeculation(&g_allFlashCacheState, false);
        flash_lock_release();
        if (status == (int32_t)kStatus_Success)
        {
#if !(defined(BL_FEATURE_FLASH_VERIFY_DISABLE) && BL_FEATURE_FLASH_VERIFY_DISABLE)
            if (verifyWrites)
            {
                uint32_t failedAddress;
                uint32_t failedData;

                flash_lock_acquire();
                if (alignedLength != 0u)
                {
                    status = g_dflashDriverInterface.flash_verify_program(
                        &g_dFlashState, address, alignedLength, (uint8_t *)buffer,
                        (ftfx_margin_value_t)g_dflashReadMargin, &failedAddress,
                        &failedData);
                }
                if ((status == (int32_t)kStatus_Success) && extraBytes != 0u)
                {
                    status = g_dflashDriverInterface.flash_verify_program(
                        &g_dFlashState, address + alignedLength, sizeof(extraData), (uint8_t *)extraData,
                        (ftfx_margin_value_t)g_dflashReadMargin, &failedAddress,
                        &failedData);
                }
                flash_lock_release();
                if (status != (int32_t)kStatus_Success)
                {
                    debug_printf("Error: flash verify failed at address: 0x%x\r\n", failedAddress);
                }
            }
#endif // !BL_FEATURE_FLASH_VERIFY_DISABLE
        }
    }
    return status;
}



status_t flexNVM_mem_write(uint32_t address, uint32_t length, const uint8_t *buffer)
{   
    return flexNVM_mem_write_intern(address, length, buffer);
}



status_t flexNVM_mem_write_in_rom(uint32_t address, uint32_t length, const uint8_t *buffer)
{
    status_t status = (int32_t)kStatus_Success;

    // Note: the check for "length != 0" and "range not in reserved region" is done in mem_write().
    assert(length);
    assert(buffer);

    uint32_t alignedLength;
    uint32_t extraBytes;
    uint32_t extraData[2];
    uint32_t alignmentBaseUnit = g_dFlashState.ftfxConfig.opsConfig.addrAligment.blockWriteUnitSize;

    assert(sizeof(extraData) >= sizeof(uint8_t) * alignmentBaseUnit);

    bool verifyWrites;
    //status = flexNVM_check_access_before_programming(address, length, &verifyWrites);
    if (status == (int32_t)kStatus_Success)
    {
        // Align length to whole words.
        alignedLength = ALIGN_DOWN(length, sizeof(uint8_t) * alignmentBaseUnit);
        extraBytes = length - alignedLength;
        assert(extraBytes < sizeof(uint8_t) * alignmentBaseUnit);

        // Pre-fill word buffer with flash erased value.
        extraData[0] = (uint32_t)kFlashMemory_ErasedValue;
        extraData[1] = (uint32_t)kFlashMemory_ErasedValue;
        if (extraBytes != 0u)
        {
            // Copy extra bytes to word buffer.
            (void)memcpy((uint8_t *)extraData, &buffer[alignedLength], extraBytes);
        }

        flash_lock_acquire();
        (void)FTFx_CACHE_ClearCachePrefetchSpeculation(&g_allFlashCacheState, true);
        // Program whole words from the user's buffer.
        if (alignedLength != 0u)
        {
            status = g_dflashDriverInterface.flash_program(&g_dFlashState, address,
                                                                                (uint8_t *)buffer, alignedLength);
        }
        if ((status == (int32_t)kStatus_Success) && extraBytes != 0u)
        {
            // Program trailing word.
            status = g_dflashDriverInterface.flash_program(
                &g_dFlashState, address + alignedLength, (uint8_t *)extraData,
                alignmentBaseUnit);
        }
        (void)FTFx_CACHE_ClearCachePrefetchSpeculation(&g_allFlashCacheState, false);
        flash_lock_release();
        if (status == (int32_t)kStatus_Success)
        {
#if !(defined(BL_FEATURE_FLASH_VERIFY_DISABLE) && BL_FEATURE_FLASH_VERIFY_DISABLE)
            if (verifyWrites)
            {
                uint32_t failedAddress;
                uint32_t failedData;

                flash_lock_acquire();
                if (alignedLength != 0u)
                {
                    status = g_dflashDriverInterface.flash_verify_program(
                        &g_dFlashState, address, alignedLength, (uint8_t *)buffer,
                        (ftfx_margin_value_t)g_dflashReadMargin, &failedAddress,
                        &failedData);
                }
                if ((status == (int32_t)kStatus_Success) && extraBytes != 0u)
                {
                    status = g_dflashDriverInterface.flash_verify_program(
                        &g_dFlashState, address + alignedLength, sizeof(extraData), (uint8_t *)extraData,
                        (ftfx_margin_value_t)g_dflashReadMargin, &failedAddress,
                        &failedData);
                }
                flash_lock_release();
                if (status != (int32_t)kStatus_Success)
                {
                    debug_printf("Error: flash verify failed at address: 0x%x\r\n", failedAddress);
                }
            }
#endif // !BL_FEATURE_FLASH_VERIFY_DISABLE
        }
    }
    return status;
}





// See flash_memory.h for documentation on this function.
status_t flexNVM_mem_fill(uint32_t address, uint32_t length, uint32_t pattern)
{
    status_t status = (int32_t)kStatus_Success;
    // Note: the check for "length != 0"
    assert(length);

	uint32_t patternBuffer[8];

	// Pre-fill pattern buffer with pattern.
	for (uint32_t i = 0u; i < 8u; i++)
	{
		patternBuffer[i] = pattern;
	}

	// Program patterns from the pattern buffer.
	while (length != 0u)
	{
		uint32_t bytes;
		if (length < sizeof(patternBuffer))
		{
			bytes = length;
		}
		else
		{
			bytes = sizeof(patternBuffer);
		}

		status = flexNVM_mem_write(address, bytes, (uint8_t *)patternBuffer);
		if (status != (int32_t)kStatus_Success)
		{
			break;
		}

		address += bytes;
		length -= bytes;
	}
    return status;
}

// See memory.h for documentation on this function.
status_t flexNVM_mem_erase_intern(uint32_t address, uint32_t length)
{
    status_t status = (int32_t)kStatus_Success;

	flash_lock_acquire();
	(void)FTFx_CACHE_ClearCachePrefetchSpeculation(&g_allFlashCacheState, true);
	status = g_dflashDriverInterface.flash_erase(&g_dFlashState, address, length,
																   kFLASH_ApiEraseKey);
	(void)FTFx_CACHE_ClearCachePrefetchSpeculation(&g_allFlashCacheState, false);
	flash_lock_release();

#if !(defined(BL_FEATURE_FLASH_VERIFY_DISABLE) && BL_FEATURE_FLASH_VERIFY_DISABLE)
	if ((status == (int32_t)kStatus_Success) && (g_dverifyWrites != 0u))
	{
		flash_lock_acquire();
		status = g_dflashDriverInterface.flash_verify_erase(
			&g_dFlashState, address, length,
			(ftfx_margin_value_t)g_dflashReadMargin);
		flash_lock_release();
		if (status != (int32_t)kStatus_Success)
		{
			debug_printf("Error: flash_verify_erase failed\r\n");
		}
	}
#endif // !BL_FEATURE_FLASH_VERIFY_DISABLE
    return status;
}

status_t flexNVM_mem_erase(uint32_t address, uint32_t length)
{        
    return flexNVM_mem_erase_intern(address, length);
}


status_t flexNVM_mem_erase_in_rom(uint32_t address, uint32_t length)
{
    status_t status = (int32_t)kStatus_Success;

	flash_lock_acquire();
	(void)FTFx_CACHE_ClearCachePrefetchSpeculation(&g_allFlashCacheState, true);
	status = g_dflashDriverInterface.flash_erase(&g_dFlashState, address, length,
																   kFLASH_ApiEraseKey);
	(void)FTFx_CACHE_ClearCachePrefetchSpeculation(&g_allFlashCacheState, false);
	flash_lock_release();

#if !(defined(BL_FEATURE_FLASH_VERIFY_DISABLE) && BL_FEATURE_FLASH_VERIFY_DISABLE)
	if ((status == (int32_t)kStatus_Success) && (g_dverifyWrites != 0u))
	{
		flash_lock_acquire();
		status = g_dflashDriverInterface.flash_verify_erase(
			&g_dFlashState, address, length,
			(ftfx_margin_value_t)g_dflashReadMargin);
		flash_lock_release();
		if (status != (int32_t)kStatus_Success)
		{
			debug_printf("Error: flash_verify_erase failed\r\n");
		}
	}
#endif // !BL_FEATURE_FLASH_VERIFY_DISABLE
    return status;
}

// See memory.h for documentation on this function.
status_t flexNVM_mem_erase_all(void)
{
    status_t status = (int32_t)kStatus_Success;

	union
	{
		memory_map_entry_t const *address;
		memory_map_entry_t *pMap;
	} map_ptr;

	// Erase FlexNVM flash
	map_ptr.address = &g_memoryMap[(uint32_t)kIndexDFlashArray];
	memory_map_entry_t *map_dflash = map_ptr.pMap;
	status = flexNVM_mem_erase(map_dflash->startAddress, (map_dflash->endAddress + 1u) - map_dflash->startAddress);

    return status;
}

// See memory.h for documentation on this function.
status_t flexNVM_mem_erase_all_unsecure(void)
{
    return flexNVM_mem_erase_all();
}
/*
status_t flexNVM_check_access_before_programming(uint32_t address, uint32_t length, bool *verifyWrites)
{
    status_t status = (int32_t)kStatus_Success;
//     uint32_t alignmentBaseUnit = &g_dFlashState->ftfxConfig.opsConfig.addrAligment.blockWriteUnitSize;

// #if !(defined(BL_FEATURE_FLASH_VERIFY_DISABLE) && BL_FEATURE_FLASH_VERIFY_DISABLE)
//     *verifyWrites = (g_dverifyWrites != 0u)? true : (_Bool)false;
// #endif // BL_FEATURE_FLASH_VERIFY_DISABLE

// // Do cumulative write check
// #if defined(BL_FEATURE_FLASH_CHECK_CUMULATIVE_WRITE) && BL_FEATURE_FLASH_CHECK_CUMULATIVE_WRITE || \
//     defined(FSL_FEATURE_SOC_FTFE_COUNT) && FSL_FEATURE_SOC_FTFE_COUNT

//     uint32_t actualLength = ALIGN_UP(length, alignmentBaseUnit);
//     if (!mem_is_erased(address, actualLength))
//     {
//         status = (int32_t)kStatusMemoryCumulativeWrite;
//     }
// #endif // BL_FEATURE_FLASH_CHECK_CUMULATIVE_WRITE || FSL_FEATURE_SOC_FTFE_COUNT

    return status;
}
*/
