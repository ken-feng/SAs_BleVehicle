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
#include "bl_ram_memory.h"


//! See normal_memory.h for documentation on this function.
status_t normal_mem_init(void)
{
	 return (int32_t)kStatus_Success;
}

status_t normal_mem_write(uint32_t address, uint32_t length, const uint8_t *buffer)
{
    {
        union
        {
            uint32_t address;
            void *pAddress;
        } ptr_1;

        union
        {
            const uint8_t *byte;
            const void *pBuffer;
        } buffer_ptr;

        ptr_1.address = address;
        buffer_ptr.byte = buffer;
        (void)memcpy(ptr_1.pAddress, buffer_ptr.pBuffer, length);
    }
    return (int32_t)kStatus_Success;
}

status_t normal_mem_fill(uint32_t address, uint32_t length, uint32_t pattern)
{
    status_t status = (int32_t)kStatus_Success;

    return status;
}

status_t normal_mem_erase(uint32_t address, uint32_t length)
{
    status_t status = (int32_t)kStatus_Success;
    core_mm_set((uint8_t*)address,0x00,length);
    return status;
}
