/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _BL_RAM_MEMORY_H_
#define _BL_RAM_MEMORY_H_
/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_device_registers.h"
#include "fsl_common.h"
#include "fsl_flash.h"
#include "bootloader_config.h"
#include "target_config.h"

status_t normal_mem_init(void);
status_t normal_mem_write(uint32_t address, uint32_t length, const uint8_t *buffer);
status_t normal_mem_erase(uint32_t address, uint32_t length);

#endif
