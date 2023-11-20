/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _BL_DFLASH_MEMORY_H_
#define _BL_DFLASH_MEMORY_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_device_registers.h"
#include "fsl_common.h"
#include "fsl_flash.h"



status_t flexNVM_mem_init(void);
status_t flexNVM_mem_read(uint32_t address, uint32_t length, uint8_t *buffer);
status_t flexNVM_mem_read_in_rom(uint32_t address, uint32_t length, uint8_t *buffer);
status_t flexNVM_mem_write(uint32_t address, uint32_t length, const uint8_t *buffer);
status_t flexNVM_mem_write_in_rom(uint32_t address, uint32_t length, const uint8_t *buffer);
status_t flexNVM_mem_fill(uint32_t address, uint32_t length, uint32_t pattern);
status_t flexNVM_mem_erase(uint32_t address, uint32_t length);
status_t flexNVM_mem_erase_in_rom(uint32_t address, uint32_t length);
status_t flexNVM_mem_erase_all(void);
status_t flexNVM_mem_erase_all_unsecure(void);

//status_t flexNVM_check_access_before_programming(uint32_t address, uint32_t length, bool *verifyWrites);
#endif
