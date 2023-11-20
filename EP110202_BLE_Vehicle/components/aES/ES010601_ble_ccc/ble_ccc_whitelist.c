#include "ble_ccc_whitelist.h"
#include "flash_api_extern.h"
#include "EM000101.h"
/*****白名单操作*****/
#include "Flash_Adapter.h"
#include "gap_interface.h"


u8 whiteBuffer[FLASH_PAGE_SIZE];

void whitelist_remove_ble_bond(u16 deviceId)
{
    bool_t isBonded = FALSE;
    uint8_t nvmIndex = gInvalidNvmIndex_c;
    if ((gBleSuccess_c == Gap_CheckIfBonded(deviceId, &isBonded, &nvmIndex)) &&
                    (isBonded))
    {
        Gap_RemoveBond(nvmIndex);
    }
}

u8 whitelist_flash_erase_page(void)
{
	uint32_t status;
	status = NV_FlashEraseSector((u32)WHITELIST_ADDRESS,FLASH_PAGE_SIZE);
	if(status == kStatus_FLASH_Success)
	{
		return 0;
	}
	return 1;
}

u8 whitelist_flash_write_page(u8* ramAddr)
{
	uint32_t status;
	if(whitelist_flash_erase_page())
	{
		return 1;
	}
	status = NV_FlashProgram((u32)WHITELIST_ADDRESS,FLASH_PAGE_SIZE,ramAddr);
	if(status == kStatus_FLASH_Success)
		return 0;
	return 1;
}

u8 whitelist_flash_read_page(u8* ramAddr)
{
    core_mm_copy(ramAddr,(u8*)WHITELIST_ADDRESS,FLASH_PAGE_SIZE);
	return 1;
}

/**
 * @brief
 *      BLE 配对白名单同步, 注:  上电BLE主动去询问SDK，同步白名单列表
 * @param [slotIdBuffer]     slotId缓存buffer,每个slotId都是LV结构
 * @param [number]   待删除的slotId个数
 * @return
 *        1表示成功，其它失败
 * @note
  */
u8 ble_ccc_syc_whitelist(u8*slotIdBuffer , u8 number)
{
    u8* slotIdLV;
    u16 offset;
    u8 i,j;
    whitelist_flash_read_page(whiteBuffer);
    for (i = 0; i < WHITELIST_MAX_NUMBER; i++)
    {
        offset = 0;
        for (j = 0; j < number; j++)
        {
            slotIdLV = slotIdBuffer + offset;
            offset += slotIdLV[0] + 1U;
            if (whiteBuffer[i*WHITELIST_REC_MAX_SIZE + WHITE_LIST_OFFSET_VALID] == 1U)
            {
                if (core_mm_compare(whiteBuffer+i*WHITELIST_REC_MAX_SIZE + WHITE_LIST_OFFSET_SLOTID_L,slotIdLV,slotIdLV[0]+1U) == 0U)
                {
                    break;
                }
            }
        }
        if (j == number)
        {
            /*当前slotId不存在*/
            /*删除绑定信息，删除白名单记录*/
            whitelist_remove_ble_bond(core_dcm_readBig16(whiteBuffer+i*WHITELIST_REC_MAX_SIZE + WHITE_LIST_OFFSET_DEVICEID));
            core_mm_set(whiteBuffer+i*WHITELIST_REC_MAX_SIZE,0x00,WHITELIST_REC_MAX_SIZE);
        }
    }
    whitelist_flash_write_page(whiteBuffer);
    return 1;
}
/*
 *  BLE 写白名单,
 */
u8 ble_ccc_write_whitelist(u8* slotIdLV, u8* ltk , u16 deviceId ,u8* mac)
{
    whitelist_flash_read_page(whiteBuffer);
    for (u8 i = 0; i < WHITELIST_MAX_NUMBER; i++)
    {
        if (whiteBuffer[i*WHITELIST_REC_MAX_SIZE + WHITE_LIST_OFFSET_VALID] != 1U)
        {
            core_dcm_writeU8(whiteBuffer+i*WHITELIST_REC_MAX_SIZE + WHITE_LIST_OFFSET_VALID,1U);
            core_mm_copy(whiteBuffer+i*WHITELIST_REC_MAX_SIZE + WHITE_LIST_OFFSET_LTK,ltk,WHITE_REC_SIZE_LTK);
            core_mm_copy(whiteBuffer+i*WHITELIST_REC_MAX_SIZE + WHITE_LIST_OFFSET_MAC,mac,WHITE_REC_SIZE_MAC);
            core_mm_copy(whiteBuffer+i*WHITELIST_REC_MAX_SIZE + WHITE_LIST_OFFSET_SLOTID_L,slotIdLV,slotIdLV[0]+1);
            core_dcm_writeBig16(whiteBuffer+i*WHITELIST_REC_MAX_SIZE + WHITE_LIST_OFFSET_DEVICEID,deviceId);
            break;
        }
    }
    whitelist_flash_write_page(whiteBuffer);
    return 1;
}
/*
 *  BLE 删白名单,
 * slotIdBuffer LV结构
 */
/**
 * @brief
 *      BLE 删白名单
 * @param [slotIdBuffer]     slotId缓存buffer,每个slotId都是LV结构
 * @param [number]   待删除的slotId个数
 * @return
 *        1表示成功，其它失败
 * @note
  */
u8 ble_ccc_delete_whitelist_from_slotId(u8* slotIdBuf, u16 number)
{
    u8* slotIdLV;
    u16 offset;
    whitelist_flash_read_page(whiteBuffer);
    offset = 0;
    slotIdLV = slotIdBuf;
    for (u16 j = 0; j < number; j++)
    {
        slotIdLV += offset;
        offset += slotIdLV[0] + 1U;
        for (u8 i = 0; i < WHITELIST_MAX_NUMBER; i++)
        {
            if (whiteBuffer[i*WHITELIST_REC_MAX_SIZE + WHITE_LIST_OFFSET_VALID] == 1U)
            {
                if (core_mm_compare(whiteBuffer+i*WHITELIST_REC_MAX_SIZE + WHITE_LIST_OFFSET_SLOTID_L,slotIdLV,slotIdLV[0]+1U) == 0U)
                {
                    /*当前slotId不存在*/
                    /*删除绑定信息，删除白名单记录*/
                    whitelist_remove_ble_bond(core_dcm_readBig16(whiteBuffer+i*WHITELIST_REC_MAX_SIZE + WHITE_LIST_OFFSET_DEVICEID));
                    core_mm_set(whiteBuffer+i*WHITELIST_REC_MAX_SIZE,0x00,WHITELIST_REC_MAX_SIZE);
                    break;
                }
            }
        }
    }
    whitelist_flash_write_page(whiteBuffer);    
    return 1;
}
/*
 *  BLE 删所有白名单,
 */
u8 ble_ccc_delete_all_whitelist(void)
{
    core_mm_set(whiteBuffer,0x00,FLASH_PAGE_SIZE);
    whitelist_flash_write_page(whiteBuffer);    
    return 1;
}
/**
 * @brief
 *      根据MAC地址查询设备的LTK
 * @param [ltk]         临时存放LTK的缓存地址
 * @param [macAddress]  MAC地址
 * @return
 *        1表示成功，其它失败
 * @note
 */
u8 ble_ccc_get_ltk_from_macaddress(u8* ltkBuffer,u8* macAddress)
{
    for (u8 i = 0; i < WHITELIST_MAX_NUMBER; i++)
    {
        if (whiteBuffer[i*WHITELIST_REC_MAX_SIZE + WHITE_LIST_OFFSET_VALID] == 1U)
        {
            if (core_mm_compare(whiteBuffer+i*WHITELIST_REC_MAX_SIZE + WHITE_LIST_OFFSET_MAC,macAddress,WHITE_REC_SIZE_MAC) == 0U)
            {
                core_mm_copy(ltkBuffer,whiteBuffer+i*WHITELIST_REC_MAX_SIZE + WHITE_LIST_OFFSET_LTK,WHITE_REC_SIZE_LTK);
                break;
            }
        }
    }    
    return 1;
}
/**
 * @brief
 *      检查当前连接的设备的ltk是否存在
 * @param [macAddress]  MAC地址
 * @return
 *        1表示成功，其它失败
 * @note
 */
u8 ble_ccc_check_ltk_from_macaddress(u8* macAddress)
{
    for (u8 i = 0; i < WHITELIST_MAX_NUMBER; i++)
    {
        if (whiteBuffer[i*WHITELIST_REC_MAX_SIZE + WHITE_LIST_OFFSET_VALID] == 1U)
        {
            if (core_mm_compare(whiteBuffer+i*WHITELIST_REC_MAX_SIZE + WHITE_LIST_OFFSET_MAC,macAddress,WHITE_REC_SIZE_MAC) == 0U)
            {
                return 1U;
            }
        }
    }    
    return 0U;
}

