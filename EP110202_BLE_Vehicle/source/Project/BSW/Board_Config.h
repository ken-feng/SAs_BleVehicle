//===========================================================================//
//文件说明：芯片KW36的驱动配置模块头文件声明
//基于环境MCUXpresso IDE V11.2.1，版本SDK_2.2.5_FRDM-KW36


#ifndef _BOARD_CFG_H
#define _BOARD_CFG_H


//文件包含
#include "Board_Config_Para.h"
//#include "alg.h"
#include "fsl_gpio.h"
//DEBUG信息开关
#define DEBUG_PRINT
//网络部分
#define NET_USE_TIMER //网络部分使用定时器中断运行
#define CAN_NET
#ifdef CAN_NET
#include "RTE_CanExport.h"
#include "ComTask.h"
#include <BCanPduCfg.h>
#endif
//SDK部分
//唤醒源定义

#define KW_TRUE 1
#define KW_FALSE 0

#define KW38_WAKEUP_SRC_COUNT	2

#define SE_PowerOn()					GPIO_PinWrite (GPIOC, 16, 1) //PA16
#define SE_PowerOff()					GPIO_PinWrite (GPIOC, 16, 0) //PA16
#define CAN_tranceiver_ON()				GPIO_PinWrite (GPIOC, 2, 0) //PC2
#define CAN_tranceiver_OFF()			GPIO_PinWrite (GPIOC, 2, 1) //PC2
typedef union
{
	unsigned char data[2];
	struct
	{
		unsigned char noACK			:1;
		unsigned char BusOff		:1;
		unsigned char passsiveError	:1;
		unsigned char SendOk		:1;
		unsigned char rsv			:4;
		unsigned char BusOffCounter :8;
	}bits;
}TypeDef_CAN_Driver_Info;
typedef union
{
	uint8_t data;
	struct
	{
		uint8_t tick_1ms			:1;
		uint8_t tick_2ms			:1;
		uint8_t RTC_1s				:1;
		uint8_t rsv					:5;
	}bits;
}TypeDef_Timer_Driver_Info;

typedef struct
{
	TypeDef_CAN_Driver_Info CAN_ch0;
	TypeDef_Timer_Driver_Info tick;
	uint8_t Wakeup;
}TypeDef_Board_Driver_Info;
//变量声明
extern TypeDef_Board_Driver_Info g_BoardDriverInfo;


//eeprom地址范围定义
#define EEPROM_ADDR_START 				0
#define EEPROM_ADDR_END 				4095//2047
#define EEPROM_ACCESS_SUCCESS			00
#define EEPROM_ACCESS_ADDR_INIT_ERR		01
#define EEPROM_ACCESS_ADDR_ERR			02
#define EEPROM_ACCESS_WRITE_FAIL		03


//函数声明
extern void KW36_Timer_Config (void);

extern void KW36_CAN_Config (void);

extern void Board_Config (void);

extern void KW36_CAN_Abort_Send (void);

extern unsigned char KW36_CAN_Send_Frame (unsigned long int id, unsigned char *p_buff, unsigned char len);

extern void KW36_LowPower_Config (void);

extern void KW36_Wakeup (void);

extern unsigned char KW36_DFlash_Erase_Sector (unsigned short int SectorIx);

extern unsigned char KW36_DFlash_Write_Sector (unsigned short int SectorIx, unsigned char *p_data);

extern unsigned char KW36_DFlash_Read_Sector (unsigned short int SectorIx, unsigned char *p_data);

extern void Board_Config (void);

extern uint8_t KW38_Write_eeprom (uint32_t addr, uint8_t *p_data,uint8_t len);

extern uint8_t KW38_Read_eeprom(uint32_t addr, uint8_t *p_data,uint16_t len);



extern void Board_BSW_main (void);

extern void Board_LowPower (void);

extern void Board_LowPower_Recovery (void);

#define WAKEUP_SRC_CAN 		(1 << 0)
#define WAKEUP_SRC_BLE 		(1 << 1)
#define WAKEUP_SRC_NFC 		(1 << 2)
#define WAKEUP_SRC_STARTKEY (1 << 3)
extern void Board_Set_WakeupSource (uint8_t src);

extern status_t KW38_eeprom_Init (void);

#endif


