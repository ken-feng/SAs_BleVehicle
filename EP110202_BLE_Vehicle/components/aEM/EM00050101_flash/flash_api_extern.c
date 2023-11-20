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
#include "Flash_Adapter.h"
#include "EM000101.h"
#include "EM000401.h"
#include "hw_flash.h"
u32 flexNVM_mem_write(u32 address, u32 length, const u8 *buffer)
{
	u32 status;
	OSA_DisableIRQGlobal();
    status = NV_FlashProgramUnaligned(address,length,buffer);
	OSA_EnableIRQGlobal();
	return status;
}
u32 flexNVM_mem_erase(u32 address, u32 length)
{
	u32 status;
	OSA_DisableIRQGlobal();	
    status = NV_FlashEraseSector(address,length);
	OSA_EnableIRQGlobal();
	return status;
}
u8 hw_flash_write_for_ble_area(u16 id, u8 *p_data,u16 len)
{
	u8 tmpData[300] = {0};
	u16 tmpLength;
	u8 ret;
	ret = hw_flash_read(NVM_FLASH_BLE_AREA,id,tmpData,&tmpLength);
	if (ret == NVM_SUCCESS)
	{
		if (tmpLength == len)
		{			
			if (0U == core_mm_compare(tmpData,p_data,tmpLength))
			{
				return NVM_SUCCESS;//待更新的数据和保存的数据一致，不写
			}
		}
	}
	else
	{
		//LOG_L_S()
	}
	ret = hw_flash_write(NVM_FLASH_BLE_AREA,id,p_data,len);
	if (ret != NVM_SUCCESS)
	{
		LOG_L_S(CAN_MD,"Key Flash Write Faid!!!  ID: %0.4x, Len:%d \r\n",id,len);
	}
    return ret;
}
u8 hw_flash_read_from_ble_area(u16 id, u8 *p_data,u16 len)
{
	u8 tmpData[512] = {0};
	u16 tmpLength;
	u8 ret;
	ret = hw_flash_read(NVM_FLASH_BLE_AREA,id,tmpData,&tmpLength);
	if (ret == NVM_SUCCESS)
	{
		if (len <= tmpLength)
		{
			core_mm_copy(p_data,tmpData,len);
            return ret;
		}
	}
	core_mm_set(p_data,0x00,len);
    return ret;
}

u8 hw_flash_write_for_uwb_area(u16 id, u8 *p_data,u16 len)
{
	u8 tmpData[128] = {0};
	u16 tmpLength;
	u8 ret;
	ret = hw_flash_read(NVM_FLASH_UWB_AREA,id,tmpData,&tmpLength);
	if (ret == NVM_SUCCESS)
	{
		if (tmpLength == len)
		{			
			if (0U == core_mm_compare(tmpData,p_data,tmpLength))
			{
				return NVM_SUCCESS;//待更新的数据和保存的数据一致，不写
			}
		}
	}
	else
	{
		//LOG_L_S()
	}
	ret = hw_flash_write(NVM_FLASH_UWB_AREA,id,p_data,len);
	if (ret != NVM_SUCCESS)
	{
		LOG_L_S(CAN_MD,"Key Flash Write Faid!!!  ID: %0.4x, Len:%d \r\n",id,len);
	}
    return ret;
}
u8 hw_flash_read_from_uwb_area(u16 id, u8 *p_data,u16 len)
{
	u8 tmpData[128] = {0};
	u16 tmpLength;
	u8 ret;
	ret = hw_flash_read(NVM_FLASH_UWB_AREA,id,tmpData,&tmpLength);
	if (ret == NVM_SUCCESS)
	{
		if (len <= tmpLength)
		{
			core_mm_copy(p_data,tmpData,len);
            return ret;
		}
	}
	core_mm_set(p_data,0x00,len);
    return ret;
}
