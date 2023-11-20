/*
 * Copyright (c) 2013-2015 Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _CRC_H_
#define _CRC_H_

#include <stdint.h>
#include "bootloader_common.h"

uint32_t core_algo_iso3309_crc32(uint8_t *buf ,uint32_t length);

//@}

#if __cplusplus
}
#endif

//! @}

#endif
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
