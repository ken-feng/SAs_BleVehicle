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
#include "fsl_debug_console.h"
#include "fsl_common.h"
//#include "board.h"
//#include "clock_config.h"
//#include "pin_mux.h"
#include "fsl_flash.h"
#include "bl_flash_memory.h"
#include "hw_flash.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/


#define BUFFER_LEN 4

/*! @brief Set 32B FlexRAM Size(EEESIZE) for EEprom, Subsystem A = B = EEESIZE / 2 */
#define EEPROM_DATA_SET_SIZE_CODE (0x33U)//
// 0x39-->32B
// 0x38-->64B
// 0x37-->128B
// 0x36-->256B
// 0x35-->512B
// 0x34-->1024B
// 0x33-->2048B
// 0x32-->4096B
// 0x31-->8192B
/*! @brief Set EEprom backup memory */
#if ((FSL_FEATURE_FLASH_FLEX_NVM_DFLASH_SIZE_FOR_DEPART_0011 != 0xFFFFFFFF) && \
     (FSL_FEATURE_FLASH_FLEX_NVM_DFLASH_SIZE_FOR_DEPART_0011 != 0x00000000) && \
     (FSL_FEATURE_FLASH_FLEX_NVM_EEPROM_SIZE_FOR_EEESIZE_0011 != 0xFFFF) &&    \
     (FSL_FEATURE_FLASH_FLEX_NVM_EEPROM_SIZE_FOR_EEESIZE_0011 != 0x0000))
#define FLEXNVM_PARTITION_CODE (0x3U)
#elif (FSL_FEATURE_FLASH_FLEX_NVM_DFLASH_SIZE_FOR_DEPART_0100 != 0xFFFFFFFF) && \
    (FSL_FEATURE_FLASH_FLEX_NVM_DFLASH_SIZE_FOR_DEPART_0100 != 0x00000000) &&   \
    (FSL_FEATURE_FLASH_FLEX_NVM_EEPROM_SIZE_FOR_EEESIZE_0100 != 0xFFFF) &&      \
    (FSL_FEATURE_FLASH_FLEX_NVM_EEPROM_SIZE_FOR_EEESIZE_0100 != 0x0000)
#define FLEXNVM_PARTITION_CODE (0x4U)
#elif (FSL_FEATURE_FLASH_FLEX_NVM_DFLASH_SIZE_FOR_DEPART_0101 != 0xFFFFFFFF) && \
    (FSL_FEATURE_FLASH_FLEX_NVM_DFLASH_SIZE_FOR_DEPART_0101 != 0x00000000) &&   \
    (FSL_FEATURE_FLASH_FLEX_NVM_EEPROM_SIZE_FOR_EEESIZE_0101 != 0xFFFF) &&      \
    (FSL_FEATURE_FLASH_FLEX_NVM_EEPROM_SIZE_FOR_EEESIZE_0101 != 0x0000)
#define FLEXNVM_PARTITION_CODE (0x5U)
#elif (FSL_FEATURE_FLASH_FLEX_NVM_DFLASH_SIZE_FOR_DEPART_1001 != 0xFFFFFFFF) && \
    (FSL_FEATURE_FLASH_FLEX_NVM_DFLASH_SIZE_FOR_DEPART_1001 != 0x00000000) &&   \
    (FSL_FEATURE_FLASH_FLEX_NVM_EEPROM_SIZE_FOR_EEESIZE_1001 != 0xFFFF) &&      \
    (FSL_FEATURE_FLASH_FLEX_NVM_EEPROM_SIZE_FOR_EEESIZE_1001 != 0x0000)
#define FLEXNVM_PARTITION_CODE (0x9U)
#elif (FSL_FEATURE_FLASH_FLEX_NVM_DFLASH_SIZE_FOR_DEPART_1010 != 0xFFFFFFFF) && \
    (FSL_FEATURE_FLASH_FLEX_NVM_DFLASH_SIZE_FOR_DEPART_1010 != 0x00000000) &&   \
    (FSL_FEATURE_FLASH_FLEX_NVM_EEPROM_SIZE_FOR_EEESIZE_1010 != 0xFFFF) &&      \
    (FSL_FEATURE_FLASH_FLEX_NVM_EEPROM_SIZE_FOR_EEESIZE_1010 != 0x0000)
#define FLEXNVM_PARTITION_CODE (0xaU)
#elif (FSL_FEATURE_FLASH_FLEX_NVM_DFLASH_SIZE_FOR_DEPART_1011 != 0xFFFFFFFF) && \
    (FSL_FEATURE_FLASH_FLEX_NVM_DFLASH_SIZE_FOR_DEPART_1011 != 0x00000000) &&   \
    (FSL_FEATURE_FLASH_FLEX_NVM_EEPROM_SIZE_FOR_EEESIZE_1011 != 0xFFFF) &&      \
    (FSL_FEATURE_FLASH_FLEX_NVM_EEPROM_SIZE_FOR_EEESIZE_1011 != 0x0000)
#define FLEXNVM_PARTITION_CODE (0xbU)
#elif (FSL_FEATURE_FLASH_FLEX_NVM_DFLASH_SIZE_FOR_DEPART_1100 != 0xFFFFFFFF) && \
    (FSL_FEATURE_FLASH_FLEX_NVM_DFLASH_SIZE_FOR_DEPART_1100 != 0x00000000) &&   \
    (FSL_FEATURE_FLASH_FLEX_NVM_EEPROM_SIZE_FOR_EEESIZE_1100 != 0xFFFF) &&      \
    (FSL_FEATURE_FLASH_FLEX_NVM_EEPROM_SIZE_FOR_EEESIZE_1100 != 0x0000)
#define FLEXNVM_PARTITION_CODE (0xcU)
#elif (FSL_FEATURE_FLASH_FLEX_NVM_DFLASH_SIZE_FOR_DEPART_1101 != 0xFFFFFFFF) && \
    (FSL_FEATURE_FLASH_FLEX_NVM_DFLASH_SIZE_FOR_DEPART_1101 != 0x00000000) &&   \
    (FSL_FEATURE_FLASH_FLEX_NVM_EEPROM_SIZE_FOR_EEESIZE_1101 != 0xFFFF) &&      \
    (FSL_FEATURE_FLASH_FLEX_NVM_EEPROM_SIZE_FOR_EEESIZE_1101 != 0x0000)
#define FLEXNVM_PARTITION_CODE (0xdU)
#else
#define FLEXNVM_PARTITION_CODE (0x8U)
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void error_trap(void);
void app_finalize(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief Flash driver Structure */
static flexnvm_config_t s_flashDriver;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*
 * @brief Gets called when an error occurs.
 *
 * @details Print error message and trap forever.
 */
void error_trap(void)
{
    while (1)
    {
    }
}
/*
 * @brief Gets called when the app is complete.
 *
 * @details Print finshed message and trap forever.
 */
void app_finalize(void)
{
    Board_Config ();
    uds_startup_init();
    Com_Init();
    app_RTE_init();
    app_RTE_main ();

    while (1)
    {
    }
}

/*!
 * @brief Use Standard Software Drivers (SSD) to modify eeprom.
 *
 * @details This function uses SSD to demonstrate flash mode:
 *            + Check eeprom information.
 *            + Program a sector.
 */
int main(void)
{
	/*check application is valid*/
	// bl_check_app_jump();
    uds_startup_process();
	/*将Reset_Handler中的处理逻辑放到跳转函数之后处理，防止boot初始化了app的ram*/
	init_data_bss();
	init_interrupts();
	/*bootloader初始化*/
    BOARD_BootClockRUN();
    bootloader_init();
	hw_flash_init();
	/*CAN 升级逻辑处理*/
    app_finalize();
    return 0;
}



void bootloader_init(void)
{
	bl_mem_init();
}
