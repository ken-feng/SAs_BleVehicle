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

flash_config_t g_allFlashState;
ftfx_cache_config_t g_allFlashCacheState;
uint32_t g_flashReadMargin = (uint32_t)kFTFx_MarginValueUser;
uint32_t g_verifyWrites;



typedef struct _flash_section_program_info
{
    uint32_t startAddress;                      //!< This address is used to record the address which is used
                                                //!< to write the whole section into flash memory
    uint32_t storedBytes;                       //!< A variable which is used to indicate if the buffer is full.
    //uint8_t buffer[0x400U]; //!< A buffer which is used to buffer a full section of data
    uint8_t buffer[8]; //!< A buffer which is used to buffer a full section of data
    //uint8_t buffer[kFLASH_AccelerationRamSize]; //!< A buffer which is used to buffer a full section of data
} flash_section_program_info_t;
flash_section_program_info_t s_flash_section_program_info;


extern memory_map_entry_t g_memoryMap[];
extern const flash_driver_interface_t g_flashDriverInterface;


// See flash_memory.h for documentation on this function.
status_t flash_mem_init(void)
{
    // Update address range of flash
    memory_map_entry_t *map;
    uint32_t tmp;

    union
    {
        memory_map_entry_t const *map;
        memory_map_entry_t *pMap;
    } map_ptr;

    map_ptr.map = &g_memoryMap[(uint32_t)kIndexFlashArray];
    map = map_ptr.pMap;

    s_flash_section_program_info.storedBytes = 0u;

    return (int32_t)kStatus_Success;
}


status_t normal_mem_read(uint32_t address, uint32_t length, uint8_t *buffer)
{
    union
    {
        uint32_t address;
        const void *pAddress;
    } ptr_1;
    union
    {
        uint8_t *byte;
        void *pBuffer;
    } buffer_ptr;

    ptr_1.address = address;
    buffer_ptr.byte = buffer;
    (void)memcpy(buffer_ptr.pBuffer, ptr_1.pAddress, length);
    return (int32_t)kStatus_Success;
}

status_t flash_mem_read_intern(uint32_t address, uint32_t length, uint8_t *buffer)
{
    status_t status = (int32_t)kStatus_Success;
    {
        status = normal_mem_read(address, length, buffer);
    }
    return status;
}


// See flash_memory.h for documentation on this function.
status_t flash_mem_read(uint32_t address, uint32_t length, uint8_t *buffer)
{  
	return flash_mem_read_intern(address,length,buffer);
}
status_t flash_mem_read_in_rom(uint32_t address, uint32_t length, uint8_t *buffer)
{
    status_t status = (int32_t)kStatus_Success;
    {
        status = normal_mem_read(address, length, buffer);
    }
    return status;
}

// See flash_memory.h for documentation on this function.
status_t flash_mem_write_intern(uint32_t address, uint32_t length, const uint8_t *buffer)
{
    // Note: the check for "length != 0" and "range not in reserved region" is done in mem_write().
    assert(length);
    assert(buffer);

    status_t status = (int32_t)kStatus_Success;
    uint32_t flashMemoryIndex = flash_get_instance(address, length);
    uint32_t alignmentBaseUnit = g_allFlashState.ftfxConfig[flashMemoryIndex].opsConfig.addrAligment.sectionCmd;

    while (length != 0u)
    {
        // Set start address when storing first byte into section program buffer
        if ((s_flash_section_program_info.storedBytes == 0u) &&
            (s_flash_section_program_info.startAddress == 0u))
        {
            // Check address alignment
            if ((address & (alignmentBaseUnit - 1u)) != 0u)
            {
                status = kStatus_FTFx_AlignmentError;
                break;
            }
            s_flash_section_program_info.startAddress = address;
        }
        else
        {
            // Start section programming operation when meet discontinuous address
            if ((s_flash_section_program_info.startAddress + s_flash_section_program_info.storedBytes) != address)
            {
                // flush cached data into target memory,
                status = flash_mem_flush();
                if (status != (int32_t)kStatus_Success)
                {
                    break;
                }
                continue;
            }
        }

        uint32_t storeBytes;
        // Check to see if section program buffer will be filled with current data packet
        if ((s_flash_section_program_info.storedBytes + length) <= sizeof(s_flash_section_program_info.buffer))
        {
            storeBytes = length;
        }
        else
        {
            storeBytes = sizeof(s_flash_section_program_info.buffer) - s_flash_section_program_info.storedBytes;
        }

        // Copy data to section program buffer
        if (buffer != &s_flash_section_program_info.buffer[s_flash_section_program_info.storedBytes])
        {
            union
            {
                uint8_t *bytes;
                void *pBuffer;
            } buffer_ptr;
            union
            {
                const uint8_t *bytes;
                const void *pBuffer;
            } buffer_ptr1;
            buffer_ptr.bytes = &s_flash_section_program_info.buffer[s_flash_section_program_info.storedBytes];
            buffer_ptr1.bytes = buffer;
            if (storeBytes != 0u)
            {
                (void)memcpy(buffer_ptr.pBuffer, buffer_ptr1.pBuffer, storeBytes);
            }
        }

        s_flash_section_program_info.storedBytes += storeBytes;
        buffer += storeBytes;
        address += storeBytes;
        length -= storeBytes;

        // Start section programming operation when section program buffer is full
        if (s_flash_section_program_info.storedBytes == sizeof(s_flash_section_program_info.buffer))
        {
            // flush cached data into target memory,
            status = flash_mem_flush();
            if (status != (int32_t)kStatus_Success)
            {
                break;
            }
        }
    }

    return status;
}

status_t flash_mem_write(uint32_t address, uint32_t length, const uint8_t *buffer)
{    
	return flash_mem_write_intern(address, length, buffer);
}


status_t flash_mem_write_in_rom(uint32_t address, uint32_t length, const uint8_t *buffer)
{
    // Note: the check for "length != 0" and "range not in reserved region" is done in mem_write().
    assert(length);
    assert(buffer);

    status_t status = (int32_t)kStatus_Success;
    uint32_t flashMemoryIndex = flash_get_instance(address, length);
    uint32_t alignmentBaseUnit = g_allFlashState.ftfxConfig[flashMemoryIndex].opsConfig.addrAligment.sectionCmd;

    while (length != 0u)
    {
        // Set start address when storing first byte into section program buffer
        if ((s_flash_section_program_info.storedBytes == 0u) &&
            (s_flash_section_program_info.startAddress == 0u))
        {
            // Check address alignment
            if ((address & (alignmentBaseUnit - 1u)) != 0u)
            {
                status = kStatus_FTFx_AlignmentError;
                break;
            }
            s_flash_section_program_info.startAddress = address;
        }
        else
        {
            // Start section programming operation when meet discontinuous address
            if ((s_flash_section_program_info.startAddress + s_flash_section_program_info.storedBytes) != address)
            {
                // flush cached data into target memory,
                status = flash_mem_flush();
                if (status != (int32_t)kStatus_Success)
                {
                    break;
                }
                continue;
            }
        }

        uint32_t storeBytes;
        // Check to see if section program buffer will be filled with current data packet
        if ((s_flash_section_program_info.storedBytes + length) <= sizeof(s_flash_section_program_info.buffer))
        {
            storeBytes = length;
        }
        else
        {
            storeBytes = sizeof(s_flash_section_program_info.buffer) - s_flash_section_program_info.storedBytes;
        }

        // Copy data to section program buffer
        if (buffer != &s_flash_section_program_info.buffer[s_flash_section_program_info.storedBytes])
        {
            union
            {
                uint8_t *bytes;
                void *pBuffer;
            } buffer_ptr;
            union
            {
                const uint8_t *bytes;
                const void *pBuffer;
            } buffer_ptr1;
            buffer_ptr.bytes = &s_flash_section_program_info.buffer[s_flash_section_program_info.storedBytes];
            buffer_ptr1.bytes = buffer;
            if (storeBytes != 0u)
            {
                (void)memcpy(buffer_ptr.pBuffer, buffer_ptr1.pBuffer, storeBytes);
            }
        }

        s_flash_section_program_info.storedBytes += storeBytes;
        buffer += storeBytes;
        address += storeBytes;
        length -= storeBytes;

        // Start section programming operation when section program buffer is full
        if (s_flash_section_program_info.storedBytes == sizeof(s_flash_section_program_info.buffer))
        {
            // flush cached data into target memory,
            status = flash_mem_flush();
            if (status != (int32_t)kStatus_Success)
            {
                break;
            }
        }
    }

    return status;
}


// See flash_memory.h for documentation on this function.
status_t flash_mem_fill(uint32_t address, uint32_t length, uint32_t pattern)
{
    // Note: the check for "length != 0" and "range not in reserved region" is done in mem_fill().
    assert(length);

    status_t status = (int32_t)kStatus_Success;

    // Pre-fill section program buffer with pattern

    union
    {
        uint8_t *address;
        uint32_t *buffer;
    } buffer_ptr;

    buffer_ptr.address = s_flash_section_program_info.buffer;
    uint32_t *buffer = buffer_ptr.buffer;

    uint32_t maxPatterns = sizeof(s_flash_section_program_info.buffer) >> 2u;
    for (uint32_t i = 0u; i < maxPatterns; i++)
    {
        *buffer++ = pattern;
    }

    while (length != 0u)
    {
        uint32_t bytes;

        s_flash_section_program_info.storedBytes = 0u;

        // Check to see if remaining address range can hold whole section program buffer
        if (length < sizeof(s_flash_section_program_info.buffer))
        {
            bytes = length;
        }
        else
        {
            bytes = sizeof(s_flash_section_program_info.buffer);
        }

        // flush cached data into target memory,
        status = flash_mem_write(address, bytes, s_flash_section_program_info.buffer);
        if (status != (int32_t)kStatus_Success)
        {
            break;
        }

        address += bytes;
        length -= bytes;
    }

    if (status == (int32_t)kStatus_Success)
    {
        // flush cached data into target memory,
        status = flash_mem_flush();
    }

    return status;
}

// See memory.h for documentation on this function.
status_t flash_mem_flush(void)
{
    status_t status = (int32_t)kStatus_Success;

    if (s_flash_section_program_info.storedBytes != 0u)
    {
        uint32_t address = s_flash_section_program_info.startAddress;
        uint32_t length = s_flash_section_program_info.storedBytes;
        uint32_t flashMemoryIndex = flash_get_instance(address, length);
        uint32_t alignmentBaseUnit = g_allFlashState.ftfxConfig[flashMemoryIndex].opsConfig.addrAligment.sectionCmd;

        // Clear related states no matter following operations are executed successfully or not.
        s_flash_section_program_info.startAddress = 0;
        s_flash_section_program_info.storedBytes = 0;

        // Align length to section program unit
        uint32_t alignedLength = ALIGN_UP(length, alignmentBaseUnit);

        bool verifyWrites;
        status = flash_check_access_before_programming(address, length, &verifyWrites);
        if (status == (int32_t)kStatus_Success)
        {
            // Fill unused region with oxFFs.
            assert(alignedLength <= sizeof(s_flash_section_program_info.buffer));
            assert(length <= sizeof(s_flash_section_program_info.buffer));
            if (length < alignedLength)
            {
                (void)memset(&s_flash_section_program_info.buffer[length], 0xFF, alignedLength - length);
            }

            flash_lock_acquire();
            (void)FTFx_CACHE_ClearCachePrefetchSpeculation(&g_allFlashCacheState, true);
            // Write data of aligned length to flash
            status = FLASH_ProgramSection(&g_allFlashState, address,
                                          (uint8_t *)s_flash_section_program_info.buffer, alignedLength);
            (void)FTFx_CACHE_ClearCachePrefetchSpeculation(&g_allFlashCacheState, false);
            flash_lock_release();

            if (status == (int32_t)kStatus_Success)
            {
// Verify wether the data has been programmed to flash successfully.
#if !(defined(BL_FEATURE_FLASH_VERIFY_DISABLE) && BL_FEATURE_FLASH_VERIFY_DISABLE)
                // Verify flash program
                if (verifyWrites)
                {
                    uint32_t failedAddress;
                    uint32_t failedData;

                    flash_lock_acquire();
                    status = g_flashDriverInterface.flash_verify_program(
                        &g_allFlashState, address, alignedLength,
                        (uint8_t *)&s_flash_section_program_info.buffer,
                        (ftfx_margin_value_t)g_flashReadMargin, &failedAddress,
                        &failedData);
                    flash_lock_release();
                    if (status != (int32_t)kStatus_Success)
                    {
                        debug_printf("Error: flash verify failed at address: 0x%x\r\n", failedAddress);
                    }
                }
#endif // !BL_FEATURE_FLASH_VERIFY_DISABLE
            }
        }
    }

    return status;
}



// See memory.h for documentation on this function.
status_t flash_mem_erase_intern(uint32_t address, uint32_t length)
{
    status_t status;

    flash_lock_acquire();
    (void)FTFx_CACHE_ClearCachePrefetchSpeculation(&g_allFlashCacheState, true);
    status = g_flashDriverInterface.flash_erase(&g_allFlashState, address, length,
                                                                   kFLASH_ApiEraseKey);
    (void)FTFx_CACHE_ClearCachePrefetchSpeculation(&g_allFlashCacheState, false);
    flash_lock_release();

#if !(defined(BL_FEATURE_FLASH_VERIFY_DISABLE) && BL_FEATURE_FLASH_VERIFY_DISABLE)
    if ((status == (int32_t)kStatus_Success) && (g_verifyWrites != 0u))
    {
        flash_lock_acquire();
        status = g_flashDriverInterface.flash_verify_erase(
            &g_allFlashState, address, length,
            (ftfx_margin_value_t)g_flashReadMargin);
        flash_lock_release();
        if (status != (int32_t)kStatus_Success)
        {
            debug_printf("Error: flash_verify_erase failed\r\n");
        }
    }
#endif // !BL_FEATURE_FLASH_VERIFY_DISABLE

    return status;
}


status_t flash_mem_erase(uint32_t address, uint32_t length)
{
	return flash_mem_erase_intern(address, length);
}


status_t flash_mem_erase_in_rom(uint32_t address, uint32_t length)
{
    status_t status;

    flash_lock_acquire();
    (void)FTFx_CACHE_ClearCachePrefetchSpeculation(&g_allFlashCacheState, true);
    status = g_flashDriverInterface.flash_erase(&g_allFlashState, address, length,
                                                                   kFLASH_ApiEraseKey);
    (void)FTFx_CACHE_ClearCachePrefetchSpeculation(&g_allFlashCacheState, false);
    flash_lock_release();

#if !(defined(BL_FEATURE_FLASH_VERIFY_DISABLE) && BL_FEATURE_FLASH_VERIFY_DISABLE)
    if ((status == (int32_t)kStatus_Success) && (g_verifyWrites != 0u))
    {
        flash_lock_acquire();
        status = g_flashDriverInterface.flash_verify_erase(
            &g_allFlashState, address, length,
            (ftfx_margin_value_t)g_flashReadMargin);
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

status_t flash_mem_erase_all(void)
{
    status_t status = (int32_t)kStatus_Success;
    uint32_t length = g_memoryMap[(uint32_t)kIndexFlashArray].endAddress - g_memoryMap[(uint32_t)kIndexFlashArray].startAddress + 1;
    status = flash_mem_erase_in_rom(g_memoryMap[(uint32_t)kIndexFlashArray].startAddress, length);
    return status;
    //     status_t status = (int32_t)kStatus_Success;
    // uint16_t pageSize = 0x800;
    // uint32_t length = g_memoryMap[(uint32_t)kIndexFlashArray].endAddress - g_memoryMap[(uint32_t)kIndexFlashArray].startAddress + 1;
    
    // for (uint32_t addr = g_memoryMap[(uint32_t)kIndexFlashArray].startAddress; addr < g_memoryMap[(uint32_t)kIndexFlashArray].endAddress; addr+=pageSize)
    // {
    //     status = flash_mem_erase(addr, pageSize);
    //     if (status != kStatus_Success)
    //     {
    //         return status;
    //     }
        
    // }
}

// See memory.h for documentation on this function.
status_t flash_mem_erase_all_unsecure(void)
{
    status_t status;

    flash_lock_acquire();
    (void)FTFx_CACHE_ClearCachePrefetchSpeculation(&g_allFlashCacheState, true);
    status = g_flashDriverInterface.flash_erase_all_unsecure(&g_allFlashState,kFLASH_ApiEraseKey);
    (void)FTFx_CACHE_ClearCachePrefetchSpeculation(&g_allFlashCacheState, false);
    flash_lock_release();

    return status;
}

uint32_t flash_get_instance(uint32_t address, uint32_t length)
{
    uint32_t instance = (uint32_t)kFlashIndex_Main;
    return instance;
}

// See memory.h for documentation on this function.
bool mem_is_erased(uint32_t address, uint32_t length)
{
    const uint8_t *start = (const uint8_t *)address;
    bool isMemoryErased = true;

    while (length != 0u)
    {
        if (*start != 0xFFu)
        {
            isMemoryErased = false;
            break;
        }
        else
        {
            length--;
            start++;
        }
    }

    return isMemoryErased;
}
status_t flash_check_access_before_programming(uint32_t address, uint32_t length, bool *verifyWrites)
{
    status_t status = (int32_t)kStatus_Success;
//    uint32_t flashMemoryIndex = flash_get_instance(address, length);
//    uint32_t alignmentBaseUnit = g_allFlashState.ftfxConfig[flashMemoryIndex].opsConfig.addrAligment.blockWriteUnitSize;
//
//    uint32_t actualLength = ALIGN_UP(length, alignmentBaseUnit);
//
//    const memory_map_entry_t *mapEntry;
//    status = find_map_entry(address, actualLength, &mapEntry);
//    if (status != (int32_t)kStatus_Success)
//    {
//        return status;
//    }
//
//#if !(defined(BL_FEATURE_FLASH_VERIFY_DISABLE) && BL_FEATURE_FLASH_VERIFY_DISABLE)
//    *verifyWrites = (g_verifyWrites != 0u)? true: (_Bool)false;
//#endif // BL_FEATURE_FLASH_VERIFY_DISABLE
//    {
//// Do cumulative write check
//#if defined(BL_FEATURE_FLASH_CHECK_CUMULATIVE_WRITE) && BL_FEATURE_FLASH_CHECK_CUMULATIVE_WRITE || \
//    defined(FSL_FEATURE_SOC_FTFE_COUNT) && FSL_FEATURE_SOC_FTFE_COUNT
//        {
//            uint32_t actualLength = ALIGN_UP(length, alignmentBaseUnit);
//            if (!mem_is_erased(address, actualLength))
//            {
//                status = (int32_t)kStatusMemoryCumulativeWrite;
//            }
//        }
//#endif // BL_FEATURE_FLASH_CHECK_CUMULATIVE_WRITE || FSL_FEATURE_SOC_FTFE_COUNT
//    }
    return status;
}
