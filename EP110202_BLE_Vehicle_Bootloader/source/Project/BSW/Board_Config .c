//===========================================================================//
//文件说明：芯片KW36的驱动配置模块
//基于环境MCUXpresso IDE V11.2.1，版本SDK_2.2.5_FRDM-KW36
//===========================================================================//
//修订版本：V0.1
//修订人：张祥
//修订时间：20201106
//修订说明：创建初版，提供API接口和函数要求
//1. 按照硬件完成GPIO初始化
//2. 完成CAN初始化，实现CAN任意报文收发，实现报文接收中断，发送完成中断，busoff中断，被动
//错误中断
//3. 完成PIT初始化，实现2ms中断
//4. 完成RTC初始化，实现秒中断
//===========================================================================//

//===========================================================================//
//文件包含
#include "Board_Config.h"
#include "user_Task.h"
#include "fsl_gpio.h"
#include "fsl_flash.h"
#include "fsl_flexcan.h"
#include "fsl_ftfx_flash.h"
#include "fsl_ftfx_controller.h"
#include "fsl_ftfx_flexnvm.h"
#include "fsl_debug_console.h"
#include "hw_flash.h"
#include "fsl_cop.h"

//===========================================================================//
//数据结构定义

//===========================================================================//
//函数列表
static void CAN0_ISR_CallBack (CAN_Type *base, flexcan_handle_t *handle, status_t status, uint32_t result, void *userData);
static void KW36_CAN_RX_ISR (void);

//===========================================================================//
//CAN0
flexcan_handle_t g_FlexCAN0_Handle;
flexcan_mb_transfer_t g_FlexCAN0_mb_tx, g_FlexCAN0_mb_rx;
flexcan_fd_frame_t g_FlexCAN0_frame_Tx, g_FlexCAN0_frame_Rx;
TypeDef_Board_Driver_Info g_BoardDriverInfo;
//BLE
//eeprom
extern flash_config_t gFlashConfig;
uint32_t g_flexramBlockBase = 0;
uint32_t g_eepromTotalSize = 0;
flexnvm_config_t s_flashDriver;
//===========================================================================//
//常量定义

//===========================================================================//
//函数说明：KW36时钟初始化函数
//
//输入说明：无
//输出说明：无
//配置说明：需要在这里加以说明
//===========================================================================//
void KW36_Clock_Config (void)
{
	CLOCK_EnableClock(kCLOCK_PortA);
	CLOCK_EnableClock(kCLOCK_PortB);
	CLOCK_EnableClock(kCLOCK_PortC);
	CLOCK_EnableClock(kCLOCK_Spi0);
	CLOCK_EnableClock(kCLOCK_Spi1);
}

//===========================================================================//
//函数说明：KW36时钟初始化函数
//输入说明：无
//输出说明：无
//配置说明：需要在这里加以说明
//===========================================================================//
void KW38_Watchdog_Config (void)
{
	cop_config_t configCop;
	/*
     * configCop.enableWindowMode = false;
     * configCop.timeoutMode = kCOP_LongTimeoutMode;
     * configCop.enableStop = false;
     * configCop.enableDebug = false;
     * configCop.clockSource = kCOP_LpoClock;
     * configCop.timeoutCycles = kCOP_2Power10CyclesOr2Power18Cycles;
     */
    COP_GetDefaultConfig(&configCop);
    configCop.timeoutMode = kCOP_LongTimeoutMode;
    configCop.timeoutCycles = kCOP_2Power5CyclesOr2Power13Cycles;
	COP_Init(SIM, &configCop);
}

void KW38_Watchdog_Disable(void)
{
	SIM->COPC &= ~SIM_COPC_COPT_MASK;
}

#if 1 
//===========================================================================//
//函数说明：KW36端口IO初始化函数
//
//输入说明：无
//输出说明：无
//配置说明：需要在这里加以说明
//===========================================================================//
void KW38_GPIO_Config (void)
{
    //CAN_InitPins();
    //管脚名称， 管脚使用资源， 管脚用途说明
    //PA0, SWD_DIO, 默认不管
    //PA1, SWD_CLK, 默认不管
    //PA2, RESET, 默认不管

    //PC2, gpio输出高，			(CAN_STB) //0：normal  1:standby
    //PC3, CAN0_TX(功能9)，		(CAN_TX)
    //PC4, CAN0_RX(功能9), 		(CAN_RX)
    PORT_SetPinMux(PORTC, 2u, kPORT_MuxAsGpio);
    GPIO_PinInit(GPIOC, 2u, &(gpio_pin_config_t){kGPIO_DigitalOutput, 0U});

    PORT_SetPinMux(PORTC, 3u, kPORT_MuxAlt9);
    PORT_SetPinMux(PORTC, 4u, kPORT_MuxAlt9);	

    CAN_tranceiver_ON ();
}
#endif

//===========================================================================//
//函数说明：KW36 定时器中断服务函数
//
//输入说明：无
//输出说明：无
//===========================================================================//
static uint32_t watchdogCnt = 0;/*开门狗喂狗计时器*/
void KW36_Timer_ISR (void)
{
	/* Clear interrupt flag.*/
	PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
	g_BoardDriverInfo.tick.bits.tick_1ms = 1;

	// watchdogCnt++;
	// if (watchdogCnt == 5000)
	// {/*开门狗喂狗*/
	// 	watchdogCnt = 0;
	// 	COP_Refresh(SIM);
	// }

	user_Task_1ms ();
}
//===========================================================================//
//函数说明：KW36 定时器初始化函数
//
//===========================================================================//
void KW36_Timer_Config (void)
{
  /* Structure of initialize PIT */
  pit_config_t pitConfig;
  /*
  * pitConfig.enableRunInDebug = false;
  */
  PIT_GetDefaultConfig(&pitConfig);
  
  /* Init pit module */
  PIT_Init(PIT, &pitConfig);
  
  /* Set timer period for channel 0 */
  //PIT_SetTimerPeriod(PIT, kPIT_Chnl_0, USEC_TO_COUNT(2000U, PIT_SOURCE_CLOCK)); //2ms
  PIT_SetTimerPeriod(PIT, kPIT_Chnl_0, USEC_TO_COUNT(1000U, PIT_SOURCE_CLOCK));//1ms
  
  /* Enable timer interrupts for channel 0 */
  PIT_EnableInterrupts(PIT, kPIT_Chnl_0, kPIT_TimerInterruptEnable);
  
  /* Enable at the NVIC */
  EnableIRQ(PIT_IRQ_ID);
  
  /* Start channel 0 */
  PIT_StartTimer(PIT, kPIT_Chnl_0);
}

//===========================================================================//
//函数说明：KW38 RTC初始化函数
//输入说明：无
//输出说明：无
//配置说明：无
void KW36_RTC_Config (void)
{
}
//===========================================================================//
//函数说明：KW38 SPI始终初始化函数
//输入说明：无
//输出说明：无
//配置说明：SPI配置要求4M，模式0，8bit，主机模式，CS由GPIO控制，收发采用查询
//===========================================================================//	
void KW36_SPI1_Config(void)
{
}

//===========================================================================//
//函数说明：KW36 CAN 中断回调函数函数接口
//
//输入说明：
//输出说明：无
//测试说明：中断服务函数执行时间如下，
//1. 发送中断执行2us，
//2. 报文接收中断执行7us,
//3. noACK中断执行时间2us
//说明：无
//===========================================================================//
void CAN0_ISR_CallBack (CAN_Type *base, flexcan_handle_t *handle, status_t status, uint32_t result, void *userData)
{
	switch (status)
	{
		case kStatus_FLEXCAN_RxIdle:
			if (RX_MESSAGE_BUFFER_NUM == result)
			{
				//rxComplete = true;
			}
			KW36_CAN_RX_ISR();
		break;

		case kStatus_FLEXCAN_TxIdle:
			if (TX_MESSAGE_BUFFER_NUM == result)
			{
				g_BoardDriverInfo.CAN_ch0.bits.SendOk = 1;
			}
		break;

		case kStatus_FLEXCAN_ErrorStatus:
//			if (result & kFLEXCAN_AckFlag)
//			{
//				g_BoardDriverInfo.CAN_ch0.bits.noACK = 1;
//			}
			if (result & kFLEXCAN_ErrorIntFlag)
			{
				g_BoardDriverInfo.CAN_ch0.bits.passsiveError = 1;
			}
			if (result & kFLEXCAN_BusOffIntFlag)
			{
				g_BoardDriverInfo.CAN_ch0.bits.BusOff = 1;
				if(g_BoardDriverInfo.CAN_ch0.bits.BusOffCounter < 0xFF)
				{
				   g_BoardDriverInfo.CAN_ch0.bits.BusOffCounter++;
				}
			}
		break;

		case kStatus_FLEXCAN_RxOverflow:

		break;

		default:
		break;
	}
	g_FlexCAN0_mb_rx.mbIdx = RX_MESSAGE_BUFFER_NUM;
	// g_FlexCAN0_mb_rx.frame = &g_FlexCAN0_frame_Rx;
	g_FlexCAN0_mb_rx.framefd = &g_FlexCAN0_frame_Rx;
	//   FLEXCAN_TransferReceiveNonBlocking(CAN0, &g_FlexCAN0_Handle, &g_FlexCAN0_mb_rx);
	FLEXCAN_TransferFDReceiveNonBlocking(CAN0, &g_FlexCAN0_Handle, &g_FlexCAN0_mb_rx);
}

//===========================================================================//
//函数说明：KW36 CAN初始化函数
//作者：
//输入说明：无
//输出说明：无
//配置说明：无
//1. 配置要求500Kbps, 16BTL, 81%采样点，
//   quantum = 1 + (PSEG1 + 1) + (PSEG2 + 1) + (PROPSEG + 1) = 1 + (3 + 1) + (2 + 1) + (7 + 1) = 16;
//	 采样点计算 :     (1 + (PROPSEG + 1) + (PSEG1 + 1))/quantum = (1+(7+1)+(3+1))/16 = 13/16 = 81.25%
//2. 报文接收中断/BUSOFF中断/NO ACK中断/被动错误中断使能
//3. CAN 滤波器配置使能
//===========================================================================//
void KW36_CAN_Config (void)
{
	flexcan_config_t flexcanConfig;
	flexcan_rx_mb_config_t mbConfig;

	FLEXCAN_GetDefaultConfig(&flexcanConfig);
	flexcanConfig.baudRate = 500000U;
	flexcanConfig.baudRateFD = 500000U;

	/* Init FlexCAN module. */

	/* If special quantum setting is needed, set the timing parameters. */
	#if (defined(SET_CAN_QUANTUM) && SET_CAN_QUANTUM)
		flexcanConfig.timingConfig.phaseSeg1 = 3;
		flexcanConfig.timingConfig.phaseSeg2 = 2;
		flexcanConfig.timingConfig.propSeg = 7;
	#if (defined(FSL_FEATURE_FLEXCAN_HAS_FLEXIBLE_DATA_RATE) && FSL_FEATURE_FLEXCAN_HAS_FLEXIBLE_DATA_RATE)
		flexcanConfig.timingConfig.fphaseSeg1 = 4;
		flexcanConfig.timingConfig.fphaseSeg2 = 2;
		flexcanConfig.timingConfig.fpropSeg = 7;
	#endif
	#endif

	// FLEXCAN_Init(CAN0, &flexcanConfig, CLOCK_GetFreq(kCLOCK_Osc0ErClk)/2);
	FLEXCAN_FDInit(CAN0, &flexcanConfig, CLOCK_GetFreq(kCLOCK_Osc0ErClk)/2, kFLEXCAN_64BperMB, false);
	/* Create FlexCAN handle structure and set call back function. */
	FLEXCAN_TransferCreateHandle(CAN0, &g_FlexCAN0_Handle, CAN0_ISR_CallBack, NULL);

	/* Set Rx Masking mechanism. */
	FLEXCAN_SetRxMbGlobalMask(CAN0, FLEXCAN_RX_MB_STD_MASK(0, 0, 0)); //接收全部报文
	//FLEXCAN_SetRxIndividualMask(CAN0, RX_MESSAGE_BUFFER_NUM, FLEXCAN_RX_MB_STD_MASK(0x321, 0, 0));

	/* Setup Rx Message Buffer. */
	mbConfig.format = kFLEXCAN_FrameFormatStandard;
	mbConfig.type = kFLEXCAN_FrameTypeData;
	mbConfig.id = FLEXCAN_ID_STD(0);

	FLEXCAN_SetFDRxMbConfig(CAN0, RX_MESSAGE_BUFFER_NUM, &mbConfig, true);
	FLEXCAN_SetFDTxMbConfig(CAN0, TX_MESSAGE_BUFFER_NUM, true);

	/* Start receive data through Rx Message Buffer. */
	g_FlexCAN0_mb_rx.mbIdx = RX_MESSAGE_BUFFER_NUM;
	// g_FlexCAN0_mb_rx.frame = &g_FlexCAN0_frame_Rx;
	g_FlexCAN0_mb_rx.framefd = &g_FlexCAN0_frame_Rx;
	FLEXCAN_TransferFDReceiveNonBlocking(CAN0, &g_FlexCAN0_Handle, &g_FlexCAN0_mb_rx);
}

//===========================================================================//
//函数说明：KW36 CAN接收报文接收中断服务函数接口
//
//输入说明：无
//输出说明：无
//说明：无
//===========================================================================//
unsigned char KW36_CAN_Send_Frame (unsigned long int id, unsigned char *p_buff, unsigned char len)
{
	unsigned char i;
	unsigned long int tmpId;
	unsigned char Result = 1U;
	tmpId = id;

//	switch (id)
//	{
//	case 0x310U:
//		tmpId = 0x310U + anchorIndex*0x10;
//		break;
//	case 0x311U:
//		tmpId = 0x311U + anchorIndex*0x10;
//		break;
//	case 0x312U:
//		tmpId = 0x312U + anchorIndex*0x10;
//		break;
//	case 0x313U:
//		tmpId = 0x313U + anchorIndex*0x10;
//		break;
//	case 0x314U:
//		tmpId = 0x314U + anchorIndex*0x10;
//		break;
//	case 0x318U:
//		tmpId = 0x318U + anchorIndex*0x10;
//		break;
//	default:
//		break;
//	}

	for(i = 0u;i<16u;i++)
	{
		g_FlexCAN0_frame_Tx.dataWord[i] = core_dcm_mku32(p_buff[i*4u + 0u],p_buff[i*4u + 1u],p_buff[i*4u + 2u],p_buff[i*4u + 3u]);
	}

	g_FlexCAN0_frame_Tx.id = FLEXCAN_ID_STD(tmpId);
	g_FlexCAN0_frame_Tx.format = kFLEXCAN_FrameFormatStandard;
	g_FlexCAN0_frame_Tx.type = kFLEXCAN_FrameTypeData;
	if(len == 64U)
	{
		g_FlexCAN0_frame_Tx.length = 0x0F;
	}
	else
	{
		g_FlexCAN0_frame_Tx.length = len;
	}
	g_FlexCAN0_mb_tx.mbIdx = TX_MESSAGE_BUFFER_NUM;

	g_FlexCAN0_mb_tx.framefd = &g_FlexCAN0_frame_Tx;
	Result = FLEXCAN_TransferFDSendNonBlocking(CAN0, &g_FlexCAN0_Handle, &g_FlexCAN0_mb_tx);
	return Result;
}

//===========================================================================//
//函数说明：KW36 CAN停止正在发送的报文
//作者：
//输入说明：无
//输出说明：无
//说明：无
//===========================================================================//
void KW36_CAN_Abort_Send (void)
{
	FLEXCAN_TransferFDAbortSend (CAN0, &g_FlexCAN0_Handle, TX_MESSAGE_BUFFER_NUM);
}
    


//===========================================================================//
//函数说明：KW36 CAN接收报文接收中断服务函数接口
//
//输入说明：无
//输出说明：无
//说明：无
//===========================================================================//
void KW36_CAN_RX_ISR (void)
{
	/* Start receive data through Rx Message Buffer. */
	g_FlexCAN0_mb_rx.mbIdx = RX_MESSAGE_BUFFER_NUM;
	// g_FlexCAN0_mb_rx.frame = &g_FlexCAN0_frame_Rx;
	g_FlexCAN0_mb_rx.framefd = &g_FlexCAN0_frame_Rx;
	// FLEXCAN_TransferReceiveNonBlocking(CAN0, &g_FlexCAN0_Handle, &g_FlexCAN0_mb_rx);
	FLEXCAN_TransferFDReceiveNonBlocking(CAN0, &g_FlexCAN0_Handle, &g_FlexCAN0_mb_rx);
	g_FlexCAN0_frame_Rx.id = (g_FlexCAN0_frame_Rx.id >> 18) & 0x7ff;
	//LOG_L_S_HEX(CAN_MD,"CAN Receive Frame :",&g_FlexCAN0_frame_Rx,sizeof(g_FlexCAN0_frame_Rx));
	RTE_BCanTrcvRxMsgHandler(&g_FlexCAN0_frame_Rx);
}

//===========================================================================//
//函数说明：KW36 DFLASH初始化
//
//输入说明：无
//输出说明：无
//配置说明：无
//===========================================================================//
void KW36_DFLASH_Config (void)
{
	NV_Init();//参见函数 NV_Init();在source\common\ApplMain.c中main_task完成了初始化
}

//===========================================================================//
//函数说明：KW36 DFLASH擦除扇区
//
//输入说明：最小值 0
//输出说明：无
//配置说明：无
//===========================================================================//
unsigned char KW36_DFlash_Erase_Sector (unsigned short int SectorIx)
{
    return 0;
}


//===========================================================================//
//函数说明：KW36 写DFLASH扇区
//
//输入说明：无
//输出说明：无
//配置说明：无
//===========================================================================//
unsigned char KW36_DFlash_Write_Sector (unsigned short int SectorIx, unsigned char *p_data)
{

   return 0;
}

//===========================================================================//
//函数说明：KW36 读取DFLASH扇区
//
//输入说明：无
//输出说明：无
//配置说明：无
//===========================================================================//
unsigned char KW36_DFlash_Read_Sector (unsigned short int SectorIx, unsigned char *p_data)
{

	return 0;
}

//===========================================================================//
//函数说明：KW36 ADC初始化
//作者：王余胜
//输入说明：无
//输出说明：无
//配置说明：ADC分辨率16位，参考电压VrefH引脚，ADC时钟？？
//===========================================================================//
void KW36_ADC_Config (void)
{
	
}

//===========================================================================//
//函数说明：KW36 ADC通道转换
//作者：
//输入说明：ADC_ch，ADC的通道，取值0-31
//输出说明：true，初始化成功
//配置说明：单次采样通道
//===========================================================================//
unsigned char KW36_ADC_Start (uint8_t ADC_ch)
{
	return true;
}

//===========================================================================//
//函数说明：KW36 获取ADC转换结果
//作者：
//输入说明：无
//输出说明：ADC转换结果
//配置说明：无
//===========================================================================//
unsigned char KW36_ADC_GetResult (unsigned short int*p_Result)
{
	return true;
}
//#define EEPROM_DATA_SET_SIZE_CODE (0x33U)//2KB
#define EEPROM_DATA_SET_SIZE_CODE (0x32U)//4KB
#define FLEXNVM_PARTITION_CODE (0x3U)
//===========================================================================//
//函数说明：KW36初始化内部自带的eeeprom（使用DFLASH模拟）
//输入说明：无
//输出说明：无
//配置说明：无
//===========================================================================//
status_t KW38_eeprom_Init (void)
{
	ftfx_security_state_t securityStatus = kFTFx_SecurityStateNotSecure; /* Return protection status */
	status_t result; /* Return code from each flash driver function */

    uint32_t dflashTotalSize  = 0;
    uint32_t dflashBlockBase  = 0;
    /* Clean up Flash driver Structure*/
	memset(&s_flashDriver, 0, sizeof(flexnvm_config_t));

    /* Setup flash driver structure for device and initialize variables. */
    result = FLEXNVM_Init(&s_flashDriver);
    if (kStatus_FTFx_Success != result)
    {
        return result;
    }
    /* Check security status. */
    result = FLEXNVM_GetSecurityState(&s_flashDriver, &securityStatus);
    if (kStatus_FTFx_Success != result)
    {
    	return result;
    }
    /* Print security status. */
    switch (securityStatus)
    {
        case kFTFx_SecurityStateNotSecure:
            break;
        case kFTFx_SecurityStateBackdoorEnabled:
            break;
        case kFTFx_SecurityStateBackdoorDisabled:
            break;
        default:
            break;
    }

    /* Debug message for user. */
    /* Test flexnvm dflash feature only if flash is unsecure. */
    if (kFTFx_SecurityStateNotSecure != securityStatus)
    {
        return 1;
    }
    else
    {
        uint32_t flexramTotalSize = 0;
        /* Get flash properties*/
        FLEXNVM_GetProperty(&s_flashDriver, kFLEXNVM_PropertyDflashBlockBaseAddr, &dflashBlockBase);
        FLEXNVM_GetProperty(&s_flashDriver, kFLEXNVM_PropertyFlexRamBlockBaseAddr, &g_flexramBlockBase);
        FLEXNVM_GetProperty(&s_flashDriver, kFLEXNVM_PropertyFlexRamTotalSize, &flexramTotalSize);
        FLEXNVM_GetProperty(&s_flashDriver, kFLEXNVM_PropertyEepromTotalSize, &g_eepromTotalSize);
        if (!g_eepromTotalSize)
        {
            /* Note: The EEPROM backup size must be at least 16 times the EEPROM partition size in FlexRAM. */
            uint32_t eepromDataSizeCode = EEPROM_DATA_SET_SIZE_CODE;//gEEPROM_DATA_SET_SIZE_CODE_c
            uint32_t flexnvmPartitionCode = FLEXNVM_PARTITION_CODE;//gFLEXNVM_PARTITION_CODE_c
            result = FLEXNVM_ProgramPartition(&s_flashDriver, kFTFx_PartitionFlexramLoadOptLoadedWithValidEepromData,
                                              eepromDataSizeCode, flexnvmPartitionCode);
            if (kStatus_FLASH_Success != result)
            {
            	return result;
            }
            /* Reset MCU */
            NVIC_SystemReset();
        }
        FLEXNVM_GetProperty(&s_flashDriver, kFLEXNVM_PropertyDflashTotalSize, &dflashTotalSize);

        result = FLEXNVM_SetFlexramFunction(&s_flashDriver, kFTFx_FlexramFuncOptAvailableForEeprom);
        if (kStatus_FLASH_Success != result)
        {
        	return result;
        }
    }
}

//===========================================================================//
//函数说明：KW36写内部自带的eeeprom（使用DFLASH模拟）
//输入说明：addr, eeprom地址，取值 EEPROM_ADDR_START - EEPROM_ADDR_END
//		   p_data, 待写入eeprom的数据
//		   len, 数据的长度
//输出说明：EEPROM_ACCESS_xxxx. 0，写入成功；=1，地址超限；=2，写入失败
//配置说明：无
//===========================================================================//
uint8_t KW38_Write_eeprom (uint32_t addr, uint8_t *p_data,uint8_t len)
{
	//status_t DriverResult = EEPROM_ACCESS_WRITE_FAIL; /* Return code from each flash driver function */
	uint8_t Result;
	uint8_t tmpData[128] = {0};
	uint16_t tmpLength;
	uint8_t ret = 0;
    return ret;
}

//===========================================================================//
//函数说明：KW36读内部自带的eeeprom（使用DFLASH模拟）
//输入说明：addr, eeprom地址，取值 EEPROM_ADDR_START - EEPROM_ADDR_END
//		   p_data, eeprom数据读出来保存的地址
//		   len, 读取数据的长度
//输出说明：EEPROM_ACCESS_xxxx. 0，读取成功；=1，地址超限；=2，读取失败
//配置说明：无
//===========================================================================//
uint8_t KW38_Read_eeprom(uint32_t addr, uint8_t *p_data,uint16_t len)
{	
	// uint16_t i;
	uint8_t Result = EEPROM_ACCESS_SUCCESS;
	uint8_t ret;
	uint16_t tmpLength;
	uint8_t tmpData[128] = {0};
	core_mm_set(p_data,0x00,len);
    return Result;
}
//===========================================================================//
//函数说明：KW36初始化函数
//
//输入说明：无
//输出说明：无
//配置说明：无
//===========================================================================//
void Board_Config (void)
{
	KW36_Clock_Config ();
	KW38_GPIO_Config ();
	KW36_CAN_Config ();
	CAN_tranceiver_ON ();

	KW36_Timer_Config ();
	//KW38_Watchdog_Config();
}
