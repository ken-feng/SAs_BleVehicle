//===========================================================================//
//文件说明：芯片KW36的驱动配置模块
//基于环境MCUXpresso IDE V11.2.1，版本SDK_2.2.5_FRDM-KW36
//===========================================================================//
//修订版本：V0.1
//修订人：
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
#include "EM000401.h"
#include "fsl_gpio.h"
#include "fsl_flash.h"
#include "fsl_flexcan.h"
#include "fsl_ftfx_flash.h"
#include "TimersManager.h"
#include "rtc_handle.h"
#include "fsl_ftfx_controller.h"
#include "fsl_ftfx_flexnvm.h"
#include "hw_flash.h"
#include "user_Task.h"
#include "app_preinclude.h"
#include "fsl_cop.h"
#include "ble_ccc.h"
#include "../../../components/aES/ES000501_uwb/Sources/uwb_SDK_Interface.h"

//#include "SecOC_Auth.h"
//#include "app_RTE.h"
//#include "lpuart_drv.h"
//===========================================================================//
//数据结构定义

//===========================================================================//
//函数列表
static void CAN0_ISR_CallBack (CAN_Type *base, flexcan_handle_t *handle, status_t status, uint32_t result, void *userData);
static void KW36_CAN_RX_ISR (void);
static void Board_DisableCanPinInterrupt(void);
//===========================================================================//
//变量定义
//BLE
//底层测试
unsigned char g_KW36_WakeUp_Source;
//CAN0
flexcan_handle_t g_FlexCAN0_Handle;
flexcan_mb_transfer_t g_FlexCAN0_mb_tx, g_FlexCAN0_mb_rx;

//Modify (Ken):VEHICLE-V0C02 NO.2 -20231225
#if defined __FIT_Aeon_H
	flexcan_frame_t g_FlexCAN0_frame_Tx, g_FlexCAN0_frame_Rx;
#else
	flexcan_fd_frame_t g_FlexCAN0_frame_Tx, g_FlexCAN0_frame_Rx;
#endif

TypeDef_Board_Driver_Info g_BoardDriverInfo;
//BLE
//eeprom
extern flash_config_t gFlashConfig;
uint32_t g_flexramBlockBase = 0;
uint32_t g_eepromTotalSize = 0;
flexnvm_config_t s_flashDriver;
tmrTimerID_t mCanTimerId = gTmrInvalidTimerID_c;


extern ST_UWBSDKInterface   gStUWBSDKInterface;
extern ST_UWBSource 		stSource;
#if (DISABLE_WDOG == 0U)/*开启开门狗*/
tmrTimerID_t mWTGTimerId = gTmrInvalidTimerID_c;
#endif
//===========================================================================//
//常量定义
static const gpioInputPinConfig_t canIRQPin =
{
	.gpioPort = gpioPort_C_c,
	.gpioPin = 4U,
	.pullSelect = pinPull_Up_c,
	.interruptSelect = pinInt_FallingEdge_c
};
static const gpioInputPinConfig_t intUwbIRQPin =
{
	.gpioPort = gpioPort_B_c,
	.gpioPin = 18U,
	.pullSelect = pinPull_Up_c,
	.interruptSelect = pinInt_FallingEdge_c
};


static uint8_t gWakeFlag = 0;
//===========================================================================//
//函数说明：KW36时钟初始化函数
//作者：
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
gpioInputPinConfig_t pinWakeSource[KW38_WAKEUP_SRC_COUNT] = {
	{
		.gpioPort = canIRQPin.gpioPort,
		.gpioPin = canIRQPin.gpioPin,
		.pullSelect = canIRQPin.pullSelect,
		.interruptSelect = canIRQPin.interruptSelect
	}
};

static bool_t Wakeup_CheckIRQ(uint32_t idx)
{
    bool_t status = FALSE;
    const gpioInputPinConfig_t * switchPin;
    switchPin = (const gpioInputPinConfig_t *)(&pinWakeSource[idx]);
    do {
			if(GpioIsPinIntPending(switchPin) != 0u)
			{
				/* set the local variable to mark that the interrupt is caused by one of the keyboard switches */
				GpioClearPinIntFlag(switchPin);
				status = TRUE;
				break;
			}
			if (GpioIsPinCauseOfWakeup(switchPin))
			{
				status = TRUE;
				break;
			}
        } while (FALSE);
    
    return status;
}
volatile u8 uwbIntCnt = 0;

extern volatile u8 intIRQFlag = 0U;

/*中断处理例程*/
//**************************************************************************
// UWB Interrupt
//**************************************************************************
void INT_UWB_IRQ_ISR(void)
{
	GpioClearPinIntFlag(&intUwbIRQPin);

#ifndef FIT_DEBUG_NO_UWB 
	//======================================================================
	// Check function enable/disable
	//======================================================================
	if(stSource.stUCIState.stUWBCommu.fpCmmuIsTransmitComplate != NULL)
	{
		//------------------------------------------------------------------
		// Transmit Complete SPI_CS
		//------------------------------------------------------------------
		if (stSource.stUCIState.stUWBCommu.fpCmmuIsTransmitComplate())
		{
			if(intIRQFlag == 0U)
			{
				intIRQFlag = 1U;
				ble_ccc_send_evt(UWB_EVT_INT_NOTICE,0U,NULL,0U);
			}
		}
		//------------------------------------------------------------------
		// Notification State
		//------------------------------------------------------------------
		else
		{
			stUWBSDK.fpUQSetNTFCacheFlag(&stSource);
		}
	}
#endif	
}


uint32_t gWakePortScan = 0;
void PTC2_PrepareExitLowPower(void)
{
	gWakePortScan = 0; 
//	GpioInputPinInit(&nfcIRQPin, 1);
	for(uint8_t i=0; i<KW38_WAKEUP_SRC_COUNT; i++ )
	{
		{
			if( Wakeup_CheckIRQ(i) )
			{
				gWakePortScan |= (uint32_t)1<<i;
			}
		}
	}	
	/*关闭PTC4的GPIO中断：否则会一直进入中断处理逻辑*/
	GpioSetPinInterrupt(pinWakeSource[0].gpioPort, pinWakeSource[0].gpioPin, pinInt_Disabled_c);

}
static void Board_DisableCanPinInterrupt(void)
{
	GpioSetPinInterrupt(canIRQPin.gpioPort, canIRQPin.gpioPin, pinInt_Disabled_c);
}
/*中断处理例程*/
void CANIRQ_PC4_ISR(void)
{
	GpioClearPinIntFlag(&canIRQPin);
	Board_DisableCanPinInterrupt();

	Board_Set_WakeupSource (WAKEUP_SRC_CAN);
	Board_LowPower_Recovery();
}
#if (DISABLE_WDOG == 0U)/*开启开门狗*/
//uint32_t wtdogCnt = 0;
void KW38_Reset_WTG_Counter(void)
{
	//wtdogCnt++;
	COP_Refresh(SIM);
	//rtc_display_time();
	//LOG_L_S(CAN_MD,"wtdogCnt:%d\r\n",wtdogCnt);
}
void KW38_WTG_Callback (void)
{
	KW38_Reset_WTG_Counter();
}
void KW38_Watchdog_Config (void)
{
	/*long mode, 8S 超时reset,last 25%(6~8S)喂狗*/
	cop_config_t configCop;
	COP_GetDefaultConfig(&configCop);
	configCop.timeoutMode = kCOP_LongTimeoutMode;
	configCop.timeoutCycles = kCOP_2Power5CyclesOr2Power13Cycles;
	COP_Init(SIM, &configCop);
	if(mWTGTimerId == gTmrInvalidTimerID_c)
	{
		mWTGTimerId = TMR_AllocateTimer();
	}
	/*低功耗模式下会清除counter*/
	TMR_StartLowPowerTimer(mWTGTimerId, gTmrLowPowerIntervalMillisTimer_c, 2000, KW38_WTG_Callback, NULL);
}
#endif


void KW38_INT_Start(void)
{
	GpioInstallIsr(INT_UWB_IRQ_ISR, gGpioIsrPrioNormal_c, 0x80, &intUwbIRQPin);
}
void KW38_INT_Stop(void)
{
	GpioUninstallIsr(&intUwbIRQPin);
}

//===========================================================================//
//函数说明：KW36端口IO初始化函数
//作者：
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

    //PA16, SPI1_SOUT 	(UWB MOSI)
    //PA17, SPI1_SIN	(UWB MISO)
    //PA18, SPI1_SCK	(UWB CLK)
    //PA19, SPI1_PCS0	(UWB CS)
	/*对应 UWB芯片SPI引脚接口*/
#ifdef FIT_SUPPORT	
    PORT_SetPinMux(PORTA, 16u, kPORT_MuxAlt2);
    PORT_SetPinMux(PORTA, 17u, kPORT_MuxAlt2);
	PORT_SetPinMux(PORTA, 18u, kPORT_MuxAlt2);
//    PORT_SetPinMux(PORTA, 19u, kPORT_MuxAlt2);
    PORT_SetPinMux(PORTA, 19u, kPORT_MuxAsGpio);
    GPIO_PinInit(GPIOA, 19u, &(gpio_pin_config_t){kGPIO_DigitalOutput, 1U});
#else
    PORT_SetPinMux(PORTC, 1u, kPORT_MuxAlt8);	/*CLK*/
    PORT_SetPinMux(PORTC, 2u, kPORT_MuxAlt8);	/*MOSI*/
	PORT_SetPinMux(PORTC, 3u, kPORT_MuxAlt8);	/*MISO*/
    PORT_SetPinMux(PORTC, 4u, kPORT_MuxAlt8);	/*CS*/

	PORT_SetPinMux(PORTA, 18u, kPORT_MuxAsGpio); /*VCC*/
    GPIO_PinInit(PORTA, 18u, &(gpio_pin_config_t){kGPIO_DigitalOutput, 0U});
#endif

	//PB18, gpio输入, (INT_UWB)
	//PB0, gpio输入低, (RDY_UWB)
	GpioInputPinInit(&intUwbIRQPin,1);
	// GpioInstallIsr(INT_UWB_IRQ_ISR, gGpioIsrPrioNormal_c, 0x80, &intUwbIRQPin);
	PORT_SetPinMux(PORTB, 0u, kPORT_MuxAsGpio);
    GPIO_PinInit(GPIOB, 0u, &(gpio_pin_config_t){kGPIO_DigitalInput, 1U});

	PORT_SetPinMux(PORTB, 1u, kPORT_MuxAsGpio);
    GPIO_PinInit(GPIOB, 1u, &(gpio_pin_config_t){kGPIO_DigitalOutput, 0U});

    //Modify (Ken):VEHICLE-V0C02 NO.1 -20231218
#if defined __FIT_Aeon_H
	PORT_SetPinMux(PORTB, 2u, kPORT_MuxAsGpio);
    GPIO_PinInit(GPIOB, 2u, &(gpio_pin_config_t){kGPIO_DigitalOutput, 0U});
	PORT_SetPinMux(PORTB, 3u, kPORT_MuxAsGpio);
    GPIO_PinInit(GPIOB, 3u, &(gpio_pin_config_t){kGPIO_DigitalOutput, 0U});
#else
	PORT_SetPinMux(PORTB, 2u, kPORT_MuxAsGpio);
    GPIO_PinInit(GPIOB, 2u, &(gpio_pin_config_t){kGPIO_DigitalOutput, 0U});
#endif
    //PC2, gpio输出高，			(CAN_STB) //0：normal  1:standby
    //PC3, CAN0_TX(功能9)，		(CAN_TX)
    //PC4, CAN0_RX(功能9), 		(CAN_RX)
    PORT_SetPinMux(PORTC, 2u, kPORT_MuxAsGpio);
    GPIO_PinInit(GPIOC, 2u, &(gpio_pin_config_t){kGPIO_DigitalOutput, 0U});

    PORT_SetPinMux(PORTC, 3u, kPORT_MuxAlt9);
    PORT_SetPinMux(PORTC, 4u, kPORT_MuxAlt9);	

    //PC16, SPI0_SCK(功能2)，	(se安全芯片 sclk)   ->  SE GPIO2
    //PC17, SPI0_SO(功能2)，	(se安全芯片MOSI)		    ->	SE GPIO0
    //PC18, SPI0_SIN(功能2)，	(se安全芯片MISO)		->  SE GPIO1
    //PC19, SPI0_CS，			(安全芯片SPI_CS)		->  SE GPIO3
    PORT_SetPinMux(PORTC, 16u, kPORT_MuxAlt2);
    PORT_SetPinMux(PORTC, 17u, kPORT_MuxAlt2);
	PORT_SetPinMux(PORTC, 18u, kPORT_MuxAlt2);
    PORT_SetPinMux(PORTC, 19u, kPORT_MuxAlt2);
#ifdef FIT_SUPPORT
	//PC1, gpio输出, 		(RST_UWB)
	//PC5, SE_EN，			(安全芯片VCC控制)
	PORT_SetPinMux(PORTC, 1u, kPORT_MuxAsGpio);
    GPIO_PinInit(GPIOC, 1u, &(gpio_pin_config_t){kGPIO_DigitalOutput, 1U});
	PORT_SetPinMux(PORTC, 5u, kPORT_MuxAsGpio);
    GPIO_PinInit(GPIOC, 5u, &(gpio_pin_config_t){kGPIO_DigitalOutput, 1U});
#else
	PORT_SetPinMux(PORTA, 17u, kPORT_MuxAsGpio);/*VCC*/
    GPIO_PinInit(PORTA, 17u, &(gpio_pin_config_t){kGPIO_DigitalOutput, 0U});
#endif
    CAN_tranceiver_ON ();
}

//===========================================================================//
//函数说明：KW36 定时器中断服务函数
//作者：
//输入说明：无
//输出说明：无
//===========================================================================//
#if USE_PIT_TIMER
void KW36_Timer_ISR (void)
{
	/* Clear interrupt flag.*/
	PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
	g_BoardDriverInfo.tick.bits.tick_1ms = 1;

	user_Task_1ms ();
}
#else
void KW36_Timer_Callback (void)
{
	g_BoardDriverInfo.tick.bits.tick_1ms = 1;
	user_Task_1ms ();
}
#endif
//===========================================================================//
//函数说明：KW36 定时器初始化函数
//作者：
//输入说明：无
//输出说明：无
//配置说明：要求提供1ms滴答，定时器溢出中断使能
//===========================================================================//
void KW36_Timer_Config (void)
{
#if USE_PIT_TIMER	
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
 #else
  	if(mCanTimerId == gTmrInvalidTimerID_c)
  	{
  		mCanTimerId = TMR_AllocateTimer();
  	}
	TMR_StartLowPowerTimer(mCanTimerId, gTmrLowPowerIntervalMillisTimer_c, 1, KW36_Timer_Callback, NULL);
 #endif 
}


//===========================================================================//
//函数说明：KW38 RTC初始化函数
//输入说明：无
//输出说明：无
//配置说明：无
void KW36_RTC_Config (void)
{
	rtc_init();
}
//===========================================================================//
//函数说明：KW38 SPI始终初始化函数
//输入说明：无
//输出说明：无
//配置说明：SPI配置要求4M，模式0，8bit，主机模式，CS由GPIO控制，收发采用查询
//===========================================================================//	
void KW38_SPI1_Config(void)
{
	dspi_master_config_t masterConfig;
	u32 srcClock_Hz;

	masterConfig.whichCtar = kDSPI_Ctar0;
    masterConfig.ctarConfig.baudRate = 1000000U;/*! Transfer baudrate - 4000k *///TRANSFER_BAUDRATE;
    masterConfig.ctarConfig.bitsPerFrame = 8U;
    masterConfig.ctarConfig.cpol = kDSPI_ClockPolarityActiveLow;
    masterConfig.ctarConfig.cpha = kDSPI_ClockPhaseFirstEdge;
    masterConfig.ctarConfig.direction = kDSPI_MsbFirst;
    masterConfig.ctarConfig.pcsToSckDelayInNanoSec = 1000000000U / 4000000U;
    masterConfig.ctarConfig.lastSckToPcsDelayInNanoSec = 1000000000U / 4000000U;
    masterConfig.ctarConfig.betweenTransferDelayInNanoSec = 1000000000U / 4000000U;

    masterConfig.whichPcs = kDSPI_Pcs0;
    masterConfig.pcsActiveHighOrLow = kDSPI_PcsActiveLow;

    masterConfig.enableContinuousSCK = false;
    masterConfig.enableRxFifoOverWrite = false;
    masterConfig.enableModifiedTimingFormat = false;
    masterConfig.samplePoint = kDSPI_SckToSin0Clock;

    srcClock_Hz = CLOCK_GetFreq(DSPI1_CLK_SRC);//DSPI_MASTER_CLK_FREQ;
    DSPI_MasterInit(SPI1, &masterConfig, srcClock_Hz);
}

//===========================================================================//
//函数说明：KW36 CAN 中断回调函数函数接口
//作者：
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
			if (result & kFLEXCAN_AckFlag)
			{
				g_BoardDriverInfo.CAN_ch0.bits.noACK = 1;
			}
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
	//Modify (Ken):VEHICLE-V0C02 NO.2 -20231225
	#if defined __FIT_Aeon_H
	g_FlexCAN0_mb_rx.frame = &g_FlexCAN0_frame_Rx;				// Can Standard
	FLEXCAN_TransferReceiveNonBlocking(CAN0, &g_FlexCAN0_Handle, &g_FlexCAN0_mb_rx);
	#else
	g_FlexCAN0_mb_rx.framefd = &g_FlexCAN0_frame_Rx;			// Can FD
	FLEXCAN_TransferFDReceiveNonBlocking(CAN0, &g_FlexCAN0_Handle, &g_FlexCAN0_mb_rx);
	#endif
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

	//Modify (Ken):VEHICLE-V0C02 NO.2 -20231225
	#if defined __FIT_Aeon_H
	FLEXCAN_Init(CAN0, &flexcanConfig, CLOCK_GetFreq(kCLOCK_Osc0ErClk)/2);
	#else
	FLEXCAN_FDInit(CAN0, &flexcanConfig, CLOCK_GetFreq(kCLOCK_Osc0ErClk)/2, kFLEXCAN_64BperMB, false);
	#endif
	/* Create FlexCAN handle structure and set call back function. */
	FLEXCAN_TransferCreateHandle(CAN0, &g_FlexCAN0_Handle, CAN0_ISR_CallBack, NULL);

	/* Set Rx Masking mechanism. */
	FLEXCAN_SetRxMbGlobalMask(CAN0, FLEXCAN_RX_MB_STD_MASK(0, 0, 0)); //接收全部报文
	//FLEXCAN_SetRxIndividualMask(CAN0, RX_MESSAGE_BUFFER_NUM, FLEXCAN_RX_MB_STD_MASK(0x321, 0, 0));

	/* Setup Rx Message Buffer. */
	mbConfig.format = kFLEXCAN_FrameFormatStandard;
	mbConfig.type = kFLEXCAN_FrameTypeData;
	mbConfig.id = FLEXCAN_ID_STD(0);

	// FLEXCAN_SetRxMbConfig(CAN0, RX_MESSAGE_BUFFER_NUM, &mbConfig, true);
	FLEXCAN_SetFDRxMbConfig(CAN0, RX_MESSAGE_BUFFER_NUM, &mbConfig, true);
	// FLEXCAN_SetTxMbConfig(CAN0, TX_MESSAGE_BUFFER_NUM, true);
	FLEXCAN_SetFDTxMbConfig(CAN0, TX_MESSAGE_BUFFER_NUM, true);

	/* Start receive data through Rx Message Buffer. */
	g_FlexCAN0_mb_rx.mbIdx = RX_MESSAGE_BUFFER_NUM;

	//Modify (Ken):VEHICLE-V0C02 NO.2 -20231225
	#if defined __FIT_Aeon_H
	g_FlexCAN0_mb_rx.frame = &g_FlexCAN0_frame_Rx;				// Can Standard
	FLEXCAN_TransferReceiveNonBlocking(CAN0, &g_FlexCAN0_Handle, &g_FlexCAN0_mb_rx);
	#else
	g_FlexCAN0_mb_rx.framefd = &g_FlexCAN0_frame_Rx;			// Can FD
	FLEXCAN_TransferFDReceiveNonBlocking(CAN0, &g_FlexCAN0_Handle, &g_FlexCAN0_mb_rx);
	#endif
}

//===========================================================================//
//函数说明：KW36 CAN接收报文接收中断服务函数接口
//作者：
//输入说明：无
//输出说明：无
//说明：无
//===========================================================================//
unsigned char KW36_CAN_Send_Frame (unsigned long int id, unsigned char *p_buff, unsigned char len)
{
	unsigned char i;
	unsigned char Result = KW_TRUE;
#ifdef FIT_DEBUG_NO_SA
	unsigned long int tmpId;
#endif	
	if (id == 0x40B)
	{
		g_BoardDriverInfo.CAN_ch0.bits.SendOk = 1;
		return Result;
	}
#ifdef FIT_DEBUG_NO_SA
	switch (id)
	{
	case 0x100:
		tmpId = 0x200;
		break;
	case 0x101:
		tmpId = 0x201;
		break;
	case 0x102:
		tmpId = 0x202;
		break;
	case 0x103:
		tmpId = 0x203;
		break;
	case 0x104:
		tmpId = 0x204;
		break;
	default:
		break;
	}
#endif	
//	SE_SPI_CS_TOG();
//	g_FlexCAN0_frame_Tx.dataByte0 = p_buff[0];
//	g_FlexCAN0_frame_Tx.dataByte1 = p_buff[1];
//	g_FlexCAN0_frame_Tx.dataByte2 = p_buff[2];
//	g_FlexCAN0_frame_Tx.dataByte3 = p_buff[3];
//
//	g_FlexCAN0_frame_Tx.dataByte4 = p_buff[4];
//	g_FlexCAN0_frame_Tx.dataByte5 = p_buff[5];
//	g_FlexCAN0_frame_Tx.dataByte6 = p_buff[6];
//	g_FlexCAN0_frame_Tx.dataByte7 = p_buff[7];

	//Modify (Ken):VEHICLE-V0C02 NO.2 -20231225
	#if defined __FIT_Aeon_H
	g_FlexCAN0_frame_Tx.dataByte0 = p_buff[0];
	g_FlexCAN0_frame_Tx.dataByte1 = p_buff[1];
	g_FlexCAN0_frame_Tx.dataByte2 = p_buff[2];
	g_FlexCAN0_frame_Tx.dataByte3 = p_buff[3];

	g_FlexCAN0_frame_Tx.dataByte4 = p_buff[4];
	g_FlexCAN0_frame_Tx.dataByte5 = p_buff[5];
	g_FlexCAN0_frame_Tx.dataByte6 = p_buff[6];
	g_FlexCAN0_frame_Tx.dataByte7 = p_buff[7];
	#else
	for(i = 0u;i<16u;i++)
	{
		g_FlexCAN0_frame_Tx.dataWord[i] = core_dcm_mku32(p_buff[i*4u + 0u],p_buff[i*4u + 1u],p_buff[i*4u + 2u],p_buff[i*4u + 3u]);
	}
	#endif

#ifdef FIT_DEBUG_NO_SA
	g_FlexCAN0_frame_Tx.id = FLEXCAN_ID_STD(tmpId);
#else
	g_FlexCAN0_frame_Tx.id = FLEXCAN_ID_STD(id);
#endif	
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

	//Modify (Ken):VEHICLE-V0C02 NO.2 -20231225
	#if defined __FIT_Aeon_H
	g_FlexCAN0_mb_tx.frame = &g_FlexCAN0_frame_Tx;					// Can Standard
	Result = FLEXCAN_TransferSendNonBlocking(CAN0, &g_FlexCAN0_Handle, &g_FlexCAN0_mb_tx);
	#else
	g_FlexCAN0_mb_tx.framefd = &g_FlexCAN0_frame_Tx;				// Can FD
	Result = FLEXCAN_TransferFDSendNonBlocking(CAN0, &g_FlexCAN0_Handle, &g_FlexCAN0_mb_tx);
	#endif
	
	//LOG_L_S_HEX(CAN_MD,"CAN Send Frame :",&g_FlexCAN0_frame_Tx,sizeof(g_FlexCAN0_frame_Tx));
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
	// FLEXCAN_TransferAbortSend (CAN0, &g_FlexCAN0_Handle, TX_MESSAGE_BUFFER_NUM);
	FLEXCAN_TransferFDAbortSend (CAN0, &g_FlexCAN0_Handle, TX_MESSAGE_BUFFER_NUM);
}
    


//===========================================================================//
//函数说明：KW36 CAN接收报文接收中断服务函数接口
//作者：
//输入说明：无
//输出说明：无
//说明：无
//===========================================================================//
void KW36_CAN_RX_ISR (void)
{
	/* Start receive data through Rx Message Buffer. */
	g_FlexCAN0_mb_rx.mbIdx = RX_MESSAGE_BUFFER_NUM;


	//Modify (Ken):VEHICLE-V0C02 NO.2 -20231225
	#if defined __FIT_Aeon_H
	g_FlexCAN0_mb_rx.frame = &g_FlexCAN0_frame_Rx;						// Can Standard
	FLEXCAN_TransferReceiveNonBlocking(CAN0, &g_FlexCAN0_Handle, &g_FlexCAN0_mb_rx);
	#else
	g_FlexCAN0_mb_rx.framefd = &g_FlexCAN0_frame_Rx;					// Can FD
	FLEXCAN_TransferFDReceiveNonBlocking(CAN0, &g_FlexCAN0_Handle, &g_FlexCAN0_mb_rx);
	#endif
	g_FlexCAN0_frame_Rx.id = (g_FlexCAN0_frame_Rx.id >> 18) & 0x7ff;
	//LOG_L_S_HEX(CAN_MD,"CAN Receive Frame :",&g_FlexCAN0_frame_Rx,sizeof(g_FlexCAN0_frame_Rx));

#ifdef FIT_DEBUG_NO_SA
	switch (g_FlexCAN0_frame_Rx.id)
	{
	case 0x310:
		g_FlexCAN0_frame_Rx.id = 0x210;
		break;
	case 0x320:
		g_FlexCAN0_frame_Rx.id = 0x220;
		break;
	case 0x330:
		g_FlexCAN0_frame_Rx.id = 0x230;
		break;
	case 0x340:
		g_FlexCAN0_frame_Rx.id = 0x240;
		break;

	case 0x311:
		g_FlexCAN0_frame_Rx.id = 0x211;
		break;
	case 0x321:
		g_FlexCAN0_frame_Rx.id = 0x221;
		break;
	case 0x331:
		g_FlexCAN0_frame_Rx.id = 0x231;
		break;
	case 0x341:
		g_FlexCAN0_frame_Rx.id = 0x241;
		break;

	case 0x312:
		g_FlexCAN0_frame_Rx.id = 0x212;
		break;
	case 0x322:
		g_FlexCAN0_frame_Rx.id = 0x222;
		break;
	case 0x332:
		g_FlexCAN0_frame_Rx.id = 0x232;
		break;
	case 0x342:
		g_FlexCAN0_frame_Rx.id = 0x242;
		break;

	case 0x314:
		g_FlexCAN0_frame_Rx.id = 0x213;
		break;
	case 0x324:
		g_FlexCAN0_frame_Rx.id = 0x223;
		break;
	case 0x334:
		g_FlexCAN0_frame_Rx.id = 0x233;
		break;
	case 0x344:
		g_FlexCAN0_frame_Rx.id = 0x243;
		break;
	default:
		break;
	}

#endif

#ifdef CAN_NET
	RTE_BCanTrcvRxMsgHandler(&g_FlexCAN0_frame_Rx);
#endif
}

//===========================================================================//
//函数说明：KW36 DFLASH初始化
//作者：
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
//作者：
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
//作者：
//输入说明：无
//输出说明：无
//配置说明：无
//===========================================================================//
unsigned char KW36_DFlash_Write_Sector (unsigned short int SectorIx, unsigned char *p_data)
{
#if 0
#endif
   return 0;
}

//===========================================================================//
//函数说明：KW36 读取DFLASH扇区
//作者：
//输入说明：无
//输出说明：无
//配置说明：无
//===========================================================================//
unsigned char KW36_DFlash_Read_Sector (unsigned short int SectorIx, unsigned char *p_data)
{
#if 0
#endif
	return 0;
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
	uint8_t ret;
	ret = hw_flash_read(NVM_FLASH_CAN_AREA,(u16)addr,tmpData,&tmpLength);
	if (ret == NVM_SUCCESS)
	{
		//LOG_L_S()
		if (tmpLength == len)
		{			
			if (0U == core_mm_compare(tmpData,p_data,tmpLength))
			{
				//-------------------------------------------------------------
				//待更新的数据和保存的数据一致，不写
				//-------------------------------------------------------------
				//Modify (Ken):VEHICLE-V0C02 NO.1 -20231218
				#if defined __FIT_Aeon_H
				return NVM_SUCCESS;
				#else
				return;
				#endif
			}
		}
	}
	else
	{
		//LOG_L_S()
	}
	ret = hw_flash_write(NVM_FLASH_CAN_AREA,(u16)addr,p_data,len);
	if (ret != NVM_SUCCESS)
	{
		LOG_L_S(CAN_MD,"CAN Flash Write Faid!!!  ID: %0.4x, Len:%d \r\n",(u16)addr,len);
	}
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
	ret = hw_flash_read(NVM_FLASH_CAN_AREA,(u16)addr,tmpData,&tmpLength);
	if (ret == NVM_SUCCESS)
	{
		if (len <= tmpLength)
		{
			core_mm_copy(p_data,tmpData,len);
			return Result;
		}
	}
	core_mm_set(p_data,0x00,len);
    return Result;
}
//===========================================================================//
//函数说明：KW36初始化函数
//作者：
//输入说明：无
//输出说明：无
//配置说明：无
//===========================================================================//
void Board_Config (void)
{
#ifdef APP_DEBUG_MODE    
	BOARD_InitLPUART();
	serial_debug_init();
	//serial_init();
#endif	
	KW36_Clock_Config ();
	KW38_GPIO_Config ();
	KW36_CAN_Config ();
	CAN_tranceiver_ON ();

	// KW36_RTC_Config ();
	//KW38_SPI1_Config ();/*UWB 初始化在UWB SDK中调用*/
#ifdef NET_USE_TIMER
	KW36_Timer_Config ();
#endif
#if (DISABLE_WDOG == 0U)/*开启开门狗*/ 
	KW38_Watchdog_Config();
#endif	
}

//===========================================================================//
//函数说明：KW36进入低功耗模式8
//作者：
//输入说明：无
//输出说明：无
//配置说明：无
//===========================================================================//
void Board_LowPower (void)
{
	gWakeFlag = 1U;
	user_Task_pend();
#ifdef NET_USE_TIMER
#if USE_PIT_TIMER
	PIT_StopTimer(PIT, kPIT_Chnl_0);//停止PIT
#else
	TMR_StopTimer(mCanTimerId);
#endif
#endif
#if (DISABLE_WDOG == 0U)/*开启开门狗*/
	TMR_StopTimer(mWTGTimerId);
#endif
	//LIN

	//SE安全芯片
	SE_SPI_CS_Low ();
	SE_Reset_Set_Low ();
	SE_PowerOff ();
	// KW36_SPI0_Config (false);

	//CAN

	FLEXCAN_Deinit (CAN0);
	CAN_tranceiver_OFF ();

//	//打印口
#ifdef DEBUG_PRINT
	 LOG_L_S(CAN_MD,"Board_LowPower\r\n");
#endif
	rtc_display_time();
	//CAN_RX唤醒，PC4
	// PORT_SetPinMux(PORTC, 4u, kPORT_MuxAsGpio);
	//GPIO_PinInit(GPIOC, 4u, &(gpio_pin_config_t ) { kGPIO_DigitalInput, 1U });
	// GpioInputPinInit(&canIRQPin,1);
	// GpioInstallIsr(CANIRQ_PC4_ISR, gGpioIsrPrioNormal_c, 0x80, &canIRQPin);

	PWR_AllowDeviceToSleep();

}
void Board_LowPower_Real(void)
{
	//CAN_RX唤醒，PC4
	// PORT_SetPinMux(PORTC, 4u, kPORT_MuxAsGpio);
	//GPIO_PinInit(GPIOC, 4u, &(gpio_pin_config_t ) { kGPIO_DigitalInput, 1U });
	GpioInputPinInit(&canIRQPin,1);
	GpioInstallIsr(CANIRQ_PC4_ISR, gGpioIsrPrioNormal_c, 0x80, &canIRQPin);
}
//===========================================================================//
//函数说明：KW36从低功耗模式恢复
//作者：
//输入说明：无
//输出说明：无
//配置说明：无
//===========================================================================//
extern void  BOARD_BootClockRUN(void);
void Board_LowPower_Recovery (void)
{
#if 1
	if (gWakeFlag == 0U)/*当前如果处于工作模式下，不允许重复调用*/
	{
		return;
	}
	gWakeFlag = 0U;

	user_Task_reSume();

// 	//唤醒后恢复配置
 	Board_Config ();

// 	//通知睡眠模块
 	PWR_DisallowDeviceToSleep();

	rtc_display_time();
	// LOG_L_S(CAN_MD,"WakeUp Source: %d\r\n",Get_Wakeup_Reason());
	// LOG_L_S(CAN_MD,"LPCD Status: %d\r\n",g_nfcBuzStatus.lpcdStatus);
	// LOG_L_S(CAN_MD,"gWakePortScan: %d\r\n",gWakePortScan);
	// LOG_L_S(CAN_MD,"PWRLib_MCU_WakeupReason: %0.2x\r\n",PWRLib_MCU_WakeupReason);
	
//	LOG_L_S(CAN_MD,"\r\n WakeUp Source: %d\r\n LPCD Status: %d\r\n gWakePortScan: %d\r\n PWRLib_MCU_WakeupReason: %x\r\n",Get_Wakeup_Reason(),g_nfcBuzStatus.lpcdStatus,gWakePortScan,PWRLib_MCU_WakeupReason);
	g_BoardDriverInfo.Wakeup = 1;
#endif
}

//===========================================================================//
//函数说明：配置KW36的唤醒源
//作者：
//输入说明：无
//输出说明：无
//配置说明：无
//===========================================================================//
void Board_Set_WakeupSource (uint8_t src)
{
	BSW_Set_Sleep_Wakeup_Reason (src);
}



