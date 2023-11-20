#ifndef HW_FLASH_H

#include "PlatformTypes.h"
#include "bl_dflash_memory.h"

//数据区域配置
#define NVM_FLASH_BLE_AREA                          0U          //存储蓝牙主密钥和蓝牙白名单
#define NVM_FLASH_NFC_AREA                          1U          //存储NFC卡的密钥和NFC卡白名单区域
#define NVM_FLASH_CAN_AREA                          2U          //存储CAN配置区域
#define NVM_FLASH_DATA_AREA_NUM0                    NVM_FLASH_BLE_AREA   //根据实际情况配置
#define NVM_FLASH_DATA_AREA_NUM1                    NVM_FLASH_NFC_AREA   //根据实际情况配置
#define NVM_FLASH_DATA_AREA_NUM2                    NVM_FLASH_CAN_AREA   //根据实际情况配置
#define NVM_FLASH_DATA_AREA_NUM0_ID_SIZE            4U          //根据实际情况配置，见下表
#define NVM_FLASH_DATA_AREA_NUM1_ID_SIZE            6U          //根据实际情况配置，见下表
#define NVM_FLASH_DATA_AREA_NUM2_ID_SIZE            21U         //根据实际情况配置，见下表
#define NVM_FLASH_DATA_AREA_SIZE                    3U          //配置需要分开存储的数据大小(根据实际情况配置)


#define NVM_FLASH_KEY_BLOCK_ADDR               0x00078000U
#define NVM_FLASH_WHITE_LIST_BLOCK_ADDR        0x0007A000U
#define NVM_FLASH_CAN_CONFIG_BLOCK_ADDR        0x0007C000U

//区域ID列表
//NVM_FLASH_DATA_AREA_NUM0
typedef enum
{
    NVM_DATA_ID_NUM0_BLE_K1 = 0x00U,
    NVM_DATA_ID_NUM0_BLE_WHITELIST,
    NVM_DATA_ID_NUM0_BLE_WHITELIST_SYC,
    NVM_DATA_ID_NUM0_NFC_WHITELIST_SYC,
}NVM_FLASH_DATA_ID_AREA_NUM0_e;


//NVM_FLASH_DATA_AREA_NUM1
typedef enum
{
    NVM_DATA_ID_NUM1_NFC_WHITE_LIST0 = 0x00U,
    NVM_DATA_ID_NUM1_NFC_WHITE_LIST1,
    NVM_DATA_ID_NUM1_NFC_WHITE_LIST2,
    NVM_DATA_ID_NUM1_NFC_WHITE_LIST3,
    NVM_DATA_ID_NUM1_NFC_WHITE_LIST4,
    NVM_DATA_ID_NUM1_NFC_NMK,
}NVM_FLASH_DATA_ID_AREA_NUM1_e;


//NVM_FLASH_DATA_AREA_NUM2
typedef enum
{
    NVM_DATA_ID_NUM2_CAN_UDS_TIM0 = 0x00U,
    NVM_DATA_ID_NUM2_CAN_UDS_TIM1,
    NVM_DATA_ID_NUM2_CAN_UDS_TIM2,
    NVM_DATA_ID_NUM2_CAN_UDS_TIM3,
    NVM_DATA_ID_NUM2_CAN_DID_F190,
    NVM_DATA_ID_NUM2_CAN_DID_0500,
    NVM_DATA_ID_NUM2_CAN_DID_F101,
    NVM_DATA_ID_NUM2_CAN_DID_F15A,
    NVM_DATA_ID_NUM2_CAN_DTC_BAT_LOW,
    NVM_DATA_ID_NUM2_CAN_DTC_BAT_HI,
    NVM_DATA_ID_NUM2_CAN_DTC_BCAN_BO,
    NVM_DATA_ID_NUM2_CAN_DTC_NET_NFC,
    NVM_DATA_ID_NUM2_CAN_DTC_NET_BLE,
    NVM_DATA_ID_NUM2_CAN_DTC_NET_MSEM,
    NVM_DATA_ID_NUM2_CAN_DTC_NET_MSIM,
    NVM_DATA_ID_NUM2_CAN_DTC_NET_LH,
    NVM_DATA_ID_NUM2_CAN_APP_RST,
    NVM_DATA_ID_NUM2_CAN_APP_SESSION_REQ,
    NVM_DATA_ID_NUM2_CAN_FBL_RST,
    NVM_DATA_ID_NUM2_CAN_FBL_EXT_PROGM_REQ,
}NVM_FLASH_DATA_ID_AREA_NUM2_e;

//页属性配置
#define NVM_FLASH_PAGE_INIT                         0xFFFFFFFFU      //页处于初始化状态
#define NVM_FLASH_PAGE_START                        0x55AA5A5AU      //页处于正常工作状态
#define NVM_FLASH_PAGE_DELETE                       0x5555AAAAU      //页处于被删除状态

#define NVM_FLASH_PAGE_WRITE_SIZE                   8U          //页单次写最小字节大小
#define NVM_FLASH_PAGE_SIZE                         2048U       //(2 * 1024)  //配置一页大小
#define NVM_FLASH_PAGE_MAX_NUM                      2U          //两个页循环写
#define NVM_FLASH_PAGE_HEAD_SIZE                    16U         //页标签头大小

//数据属性配置
#define NVM_FLASH_DATA_NORMAL                       0x5A5AU     //当前数据正常
#define NVM_FLASH_DATA_DELETE                       0xDADAU     //当前数据被删除
#define NVM_FLASH_DATA_BLOCK                        0xBABAU     //当前数据被冻结

#define NVM_FLASH_DATA_HEAD_SIZE                    16U         //数据头大小
#define NVM_FLASH_DATA_BODY_MAX_SIZE                128U        //单个数据内容最大值（根据实际情况配置，需要是8的整数倍）
#define NVM_FLASH_DATA_MAX_SIZE                     (NVM_FLASH_DATA_HEAD_SIZE + NVM_FLASH_DATA_BODY_MAX_SIZE)                    

typedef enum
{
    NVM_SUCCESS,                        //正确
    NVM_ERR_CRC,                        //CRC校验错误
    NVM_ERR_DATA,                       //数据结构不对
    NVM_ERR_NO_SPACE,                   //当前页空间不足
    NVM_ERR_NO_ID,                      //未找到指定ID数据
    NVM_ERR_PARAMETER,                  //参数错误
    NVM_ERR_NO_DATA,                    //校验数据时当前空间没有数据
    NVM_ERR_WRITE,                      //flash写数据异常
}HW_NVM_ERR_e;

typedef struct 
{
    u32 new_addr;                       //记录最新可用的地址
    u8 active_page_num;                 //记录当前激活的页
    u8 old_page_num;                 	//记录前一个激活的页
    u8 need_swap_flag;                  //表示当前是否需要swap操作
}NVM_FLASH_INFO_t;

u8 hw_flash_init(void);
u8 hw_flash_write(u8 area_num, u16 data_ID, u8 *data, u16 len);
u8 hw_flash_read(u8 area_num, u16 data_ID, u8 *data, u16 *len);


#define core_dcm_u8_lo(b)                   ((u8)((u8)(b)&0x0f))
#define core_dcm_u8_hi(b)                   ((u8)((u8)(b)>>4))

#define core_dcm_u16_lo(w)                   ((u8)((u16)(w)&0x00ff))
#define core_dcm_u16_hi(w)                   ((u8)((u16)(w)>>8))

#define core_dcm_u32_lo(dw)                  ((u16)((u32)(dw)&0xffff))
#define core_dcm_u32_hi(dw)                  ((u16)((u32)(dw)>>16))

#define core_dcm_mku16(h, l)                ((u16)(((u16)(h)<<8) | (l)))
#define core_dcm_mku32(hh, hl, lh, ll)      ((u32)((((u32)hh)<<24) | ((u32)(hl)<<16) | ((u16)(lh)<<8) | (ll)))
#define core_dcm_readU8(src)                ((u8)(*((u8 *)(src))))
#define core_dcm_readBig16(src)             core_dcm_mku16((((u8 *)(src))[0]), (((u8 *)(src))[1]))
#define core_dcm_readBig32(src)             core_dcm_mku32((((u8 *)(src))[0]), (((u8 *)(src))[1]), (((u8 *)(src))[2]), (((u8 *)(src))[3]))
#define core_dcm_writeU8(dest, value)       ((((u8 *)(dest))[0]) = (value))

#define core_dcm_writeBig16(dest, value)    do {(((u8 *)(dest))[0]) = (u8)((value)>>8); (((u8 *)(dest))[1]) = (u8)(value);} while(0);

#define core_dcm_writeBig32(dest, value)    do { \
                                                (((u8 *)(dest))[0]) = (u8)((value)>>24); \
                                                (((u8 *)(dest))[1]) = (u8)((value)>>16); \
                                                (((u8 *)(dest))[2]) = (u8)((value)>>8);  \
                                                (((u8 *)(dest))[3]) = (u8)(value);       \
                                            } while(0);

#endif //HW_FLASH_H
