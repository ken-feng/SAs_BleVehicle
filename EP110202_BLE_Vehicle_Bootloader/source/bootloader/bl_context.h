/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _BL_CONTEXT_H_
#define _BL_CONTEXT_H_
/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_device_registers.h"
#include "fsl_common.h"
#include "fsl_flash.h"
#include "bootloader_config.h"
#include "target_config.h"
#include "vector_table_info.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

// !@brief Memory property enum codes

//! @brief Shutdown types.
typedef enum _shutdown_types
{
    kShutdownType_Shutdown = 0,
    kShutdownType_Cleanup = 1,
    kShutdownType_Reset = 2,
} shutdown_type_t;
void shutdown_cleanup(shutdown_type_t shutdown);


typedef enum _bootloader_configs
{
    BL_Mode = 0,			/*1： appMode   other: bootMode  {2: 重编程模式  other:boot下无重编程模式}*/
	BL_AppValidFlag = 8,	/*1： AppValid  other: AppInValid*/
    BL_SwitchRtn = 16,		/*1： return  other: No return*/

    
    // BL_AppStartAddress = 24,
    // BL_AppSize = 32,
    // BL_AppCrc = 40,
	// BL_EepromInitFlag = 44,         /*1: 已经初始化EEPROM  other : 未初始化eeprom*/

}bootloader_config_t;

void bl_check_app_jump(void);

uint8_t bl_setBootloaderConfig(uint32_t startAddress, uint32_t size, uint32_t crc);
uint8_t bl_clearAppValidFlag(void);
uint8_t bl_isBLModeFlag(void);

uint8_t bl_GetEepromInitFlag(void);
// uint8_t bl_SetEepromInitFlag(uint8_t flag);
void bl_clr_reProgramMode(void);
#endif
