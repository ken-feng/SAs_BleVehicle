#ifndef _BLE_CCC_H_
#define _BLE_CCC_H_
#include "EM000101.h"
#include "EM000401.h"


#define WHITELIST_ADDRESS       0x7E000

#define WHITELIST_MAX_NUMBER    15U

#define WHITE_REC_SIZE_VALID    1U
#define WHITE_REC_SIZE_MAC      6U
#define WHITE_REC_SIZE_LTK      16U
#define WHITE_REC_SIZE_SLOTID_L 1U
#define WHITE_REC_SIZE_SLOTID_V 8U
#define WHITE_REC_SIZE_DEIVEID  2U


#define WHITELIST_REC_MAX_SIZE      (WHITE_REC_SIZE_VALID+WHITE_REC_SIZE_MAC+WHITE_REC_SIZE_LTK+\
                                    WHITE_REC_SIZE_SLOTID_L+WHITE_REC_SIZE_SLOTID_V+WHITE_REC_SIZE_DEIVEID)

typedef enum
{
    WHITE_LIST_OFFSET_VALID = 0,
    WHITE_LIST_OFFSET_MAC   =       WHITE_LIST_OFFSET_VALID + WHITE_REC_SIZE_VALID,
    WHITE_LIST_OFFSET_LTK   =       WHITE_LIST_OFFSET_MAC + WHITE_REC_SIZE_MAC,
    WHITE_LIST_OFFSET_SLOTID_L  =   WHITE_LIST_OFFSET_LTK + WHITE_REC_SIZE_LTK,
    WHITE_LIST_OFFSET_SLOTID_V  =   WHITE_LIST_OFFSET_SLOTID_L + WHITE_REC_SIZE_SLOTID_L,
    WHITE_LIST_OFFSET_DEVICEID  =   WHITE_LIST_OFFSET_SLOTID_V + WHITE_REC_SIZE_SLOTID_V,
};

/**
 * @brief
 *      BLE 配对白名单同步, 注:  上电BLE主动去询问SDK，同步白名单列表
 * @param [slotIdBuffer]     slotId缓存buffer,每个slotId都是LV结构
 * @param [number]   待删除的slotId个数
 * @return
 *        1表示成功，其它失败
 * @note
  */
u8 ble_ccc_syc_whitelist(u8*slotIdBuffer , u8 number);
/*
 *  BLE 写白名单,
 */
u8 ble_ccc_write_whitelist(u8* slotIdLV, u8* ltk , u16 deviceId ,u8* mac);
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
u8 ble_ccc_delete_whitelist_from_slotId(u8* slotIdLV, u16 number);
/*
 *  BLE 删所有白名单,
 */
u8 ble_ccc_delete_all_whitelist(void);
/**
 * @brief
 *      根据MAC地址查询设备的LTK
 * @param [ltk]         临时存放LTK的缓存地址
 * @param [macAddress]  MAC地址
 * @return
 *        1表示成功，其它失败
 * @note
 */
u8 ble_ccc_get_ltk_from_macaddress(u8* ltkBuffer,u8* macAddress);
/**
 * @brief
 *      检查当前连接的设备的ltk是否存在
 * @param [macAddress]  MAC地址
 * @return
 *        1表示成功，其它失败
 * @note
 */
u8 ble_ccc_check_ltk_from_macaddress(u8* macAddress);
#endif
