/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _BL_FLASH_MEMORY_H_
#define _BL_FLASH_MEMORY_H_
/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_device_registers.h"
#include "fsl_common.h"
#include "fsl_flash.h"
#include "bl_memory.h"

status_t flash_mem_init(void);
status_t flash_mem_read(uint32_t address, uint32_t length, uint8_t *buffer);
status_t flash_mem_read_in_rom(uint32_t address, uint32_t length, uint8_t *buffer);
status_t flash_mem_write(uint32_t address, uint32_t length, const uint8_t *buffer);
status_t flash_mem_write_in_rom(uint32_t address, uint32_t length, const uint8_t *buffer);
status_t flash_mem_fill(uint32_t address, uint32_t length, uint32_t pattern);
status_t flash_mem_flush(void);
status_t flash_mem_erase(uint32_t address, uint32_t length);
status_t flash_mem_erase_in_rom(uint32_t address, uint32_t length);
status_t flash_mem_erase_all(void);
status_t flash_mem_erase_all_unsecure(void);
uint32_t flash_get_instance(uint32_t address, uint32_t length);
bool mem_is_erased(uint32_t address, uint32_t length);
status_t flash_check_access_before_programming(uint32_t address, uint32_t length, bool *verifyWrites);

status_t normal_mem_read(uint32_t address, uint32_t length, uint8_t *buffer);
#endif;
