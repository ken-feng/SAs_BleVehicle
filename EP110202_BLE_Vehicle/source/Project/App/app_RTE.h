#ifndef _APP_RTE_H
#define _APP_RTE_H
#include "stdint.h"
#include "BCanPduCfg.h"
//常量宏定义
//结构体定义
typedef struct
{
	u8 deviceId;
	u8 connectStatus;
	u8 linkParam[64];
	u8 restartLink;
}TypeDef_RTE_SDK;


//变量声明

extern TypeDef_RTE_SDK g_app_RTE_SDK;

//函数说明
extern void app_RTE_main (void);


extern void app_RTE_init(void);

#endif


