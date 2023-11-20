//===========================================================================//
//文件说明：芯片KW36的驱动配置相关参数定义
//基于环境MCUXpresso IDE V11.2.1，版本SDK_2.2.5_FRDM-KW36


#ifndef _BOARD_CFG_PARA_H
#define _BOARD_CFG_PARA_H


//文件包含
//driver
#include "fsl_port.h"
//#include "GPIO_Adapter.h"
#include "fsl_flexcan.h"
//#include "fsl_dspi.h"
//#include "board.h"
//#include "pin_mux.h"
#include "fsl_pit.h"
//#include "SerialManager.h"
////#include "fsl_debug_console.h"
//#include "SecLib.h"
//#include "fsl_lin.h"
//#include "fsl_rtc.h"
//#include "FreeRTOS.h"
//#include "Flash_Adapter.h"
//#include "FunctionLib.h"
////#include "fsl_lpuart.h"
//#include "fsl_lin_lpuart.h"
#include "fsl_adc16.h"
//#include "wireless_uart.h"
//#include "Flash_Adapter.h"
//#include "task.h"
//#include "PWR_Interface.h"
#include "PlatformTypes.h"
//===========================================================================//
//宏定义
// //SPI
// #define DSPI_MASTER_CLK_SRC DSPI0_CLK_SRC
// #define DSPI_MASTER_CLK_FREQ CLOCK_GetFreq(DSPI0_CLK_SRC)
// #define EXAMPLE_DSPI_MASTER_PCS_FOR_INIT kDSPI_Pcs0
// #define EXAMPLE_DSPI_MASTER_PCS_FOR_TRANSFER kDSPI_MasterPcs0
// #define TRANSFER_BAUDRATE 500000U /*! Transfer baudrate - 4M */
// //#define TRANSFER_BAUDRATE 1000000U /*! Transfer baudrate - 4M */


//PIT timer
#define KW36_Timer_ISR PIT_IRQHandler
#define PIT_IRQ_ID PIT_IRQn
/* Get source clock for PIT driver */
#define PIT_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)


//CAN
#define RX_MESSAGE_BUFFER_NUM (1)//(9)
#define TX_MESSAGE_BUFFER_NUM (0)
#define EXAMPLE_CAN CAN0
#define DEMO_FORCE_CAN_SRC_OSC (1)
#define SET_CAN_QUANTUM     1


// //RTC
// #define EXAMPLE_CAP_LOAD_VALUE RTC_CR_SC8P_MASK


// //BLE
// extern bleResult_t Wireless_Uart_SendData (uint8_t *p_data, uint16_t len);
// extern gapAdvertisingData_t gAppAdvertisingData;
// extern gapScanResponseData_t gAppScanRspData;
// extern unsigned char g_BLE_adv_name[26];
// extern unsigned char g_BLE_adv_name_len;

//Assembly
#define nop() __asm("NOP")


#define core_dcm_mku16(h, l)                ((u16)(((u16)(h)<<8) | (l)))
#define core_dcm_mku32(hh, hl, lh, ll)      ((u32)((((u32)hh)<<24) | ((u32)(hl)<<16) | ((u16)(lh)<<8) | (ll)))
#define core_dcm_readU8(src)                ((u8)(*((u8 *)(src))))
#define core_dcm_readBig16(src)             core_dcm_mku16((((u8 *)(src))[0]), (((u8 *)(src))[1]))
#define core_dcm_readBig32(src)             core_dcm_mku32((((u8 *)(src))[0]), (((u8 *)(src))[1]), (((u8 *)(src))[2]), (((u8 *)(src))[3]))
#define core_dcm_readBig24(src)             core_dcm_mku32(                 0, (((u8 *)(src))[0]), (((u8 *)(src))[1]), (((u8 *)(src))[2]))

#define core_dcm_writeU8(dest, value)       ((((u8 *)(dest))[0]) = (value))

#define core_dcm_writeBig16(dest, value)    do {(((u8 *)(dest))[0]) = (u8)((value)>>8); (((u8 *)(dest))[1]) = (u8)(value);} while(0);

#define core_dcm_writeBig32(dest, value)    do { \
                                                (((u8 *)(dest))[0]) = (u8)((value)>>24); \
                                                (((u8 *)(dest))[1]) = (u8)((value)>>16); \
                                                (((u8 *)(dest))[2]) = (u8)((value)>>8);  \
                                                (((u8 *)(dest))[3]) = (u8)(value);       \
                                            } while(0);

#define core_dcm_writeBig24(dest, /* u32 */value)   do { \
                                                (((u8 *)(dest))[0]) = (u8)((value)>>16); \
                                                (((u8 *)(dest))[1]) = (u8)((value)>>8);  \
                                                (((u8 *)(dest))[2]) = (u8)(value);       \
                                            } while(0);
#endif


