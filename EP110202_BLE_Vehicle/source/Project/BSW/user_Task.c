//===========================================================================//
//文件说明：用户任务定义
//===========================================================================//
//修订版本：V0.1
//修订人：
//修订时间：2020/12/03
//修订说明：
//1. 创建任务userTask1_ProductMain和userTask2_SDK
//===========================================================================//

//文件包含
#include "EmbeddedTypes.h"
#include "fsl_os_abstraction.h"
#include "fsl_os_abstraction_free_rtos.h"
#include <string.h>
#include "GenericList.h"
#include "fsl_common.h"
#include "Panic.h"
#include "Board_Config.h"
#include "app_RTE.h"

//数据结构定义

//函数定义和声明
void user_Task1_ProductMain(void* argument);
void user_Task2_SDK(void* argument);


static u8 taskPendFlag = 0;
osaEventId_t mUserTaskEvtId;
osaTaskId_t gUserTask1Id = 0;
//变量定义和声明
OSA_TASK_DEFINE(user_Task1_ProductMain, 7, 1, 1800, 0); //
//OSA_TASK_DEFINE(user_Task2_SDK, 7, 1, 2000, 0); //sdk优先级

//===========================================================================//
//函数说明：用户任务创建
//作者：
//输入：无
//输出：无
//===========================================================================//
void user_Task_Init (void)
{
    mUserTaskEvtId = OSA_EventCreate(TRUE);
	if( NULL == mUserTaskEvtId )
    {
        panic(0,0,0,0);
        return;
    }
	gUserTask1Id = OSA_TaskCreate(OSA_TASK(user_Task1_ProductMain), NULL);
    if( NULL == gUserTask1Id )
    {
        panic(0,0,0,0);
        return;
    }

	//OSA_TaskCreate(OSA_TASK(user_Task2_SDK), NULL);

}

void user_Task_pend(void)
{
	taskPendFlag = 1;
}

void user_Task_reSume(void)
{
	taskPendFlag = 0;
	OSA_EventSet(mUserTaskEvtId, 1);
}

// void user_Task_clearPendEvt(void)
// {
// 	OSA_EventClear(mUserTaskEvtId, 1);
// }


//===========================================================================//
//函数说明：用户任务1，主要实现用户业务功能
//作者：
//输入：无
//输出：无
//===========================================================================//
void user_Task1_ProductMain (void* argument)
{
	static uint8_t tick_5ms;
	//Board_Config ();
	osaEventFlags_t event;
#ifdef CAN_NET
	Com_Init();
#endif
	app_RTE_init();
    while (1)
    {
		if (taskPendFlag == 1U)
		{
			(void)OSA_EventWait(mUserTaskEvtId, osaEventFlagsAll_c, FALSE, osaWaitForever_c , &event);
		}
		
    	OSA_TimeDelay (5);
		
    	//5ms
		//BLE_SDK_App_task ();

    	//10ms
		if (++tick_5ms > 1)
		{
			tick_5ms = 0;
			app_RTE_main ();
			
			//SecOC_Auth_main ();
			//WorkMode_main ();
			//app_APA_main ();
			//sleep_main ();
		}

    }
}


//===========================================================================//
//函数说明：用户任务实时2ms任务，将在定时器ISR以8ms周期执行
//作者：
//输入：无
//输出：无
//===========================================================================//
void user_Task_1ms (void)
{
	static uint8_t tick_2ms = 0;
	//1ms
	Com_1msTask();
	//2ms
	if (++tick_2ms > 1)
	{
		tick_2ms = 0;
		Com_2msTask();
	}
}

