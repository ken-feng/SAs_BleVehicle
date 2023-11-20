//===========================================================================//
//文件说明：应用层需要的接口都在这里转接
//===========================================================================//
//修订版本：V0.1
//修订人：
//修订时间：2021/06/28
//修订说明：
//1. 创建初版，增加UDS读取的接口, SecOC部分的接口
//===========================================================================//

//文件包含
#include "Board_Config.h"
#include "BCanPduCfg.h"
#include "app_RTE.h"

//数据结构定义
//函数定义和声明

//变量定义
TypeDef_RTE_SDK g_app_RTE_SDK;

//===========================================================================//
//函数说明：获取CAN上的数据
//作者：
//输入：无
//输出：无
//说明：
//===========================================================================//
void app_RTE_CAN_main (void)
{
}




//===========================================================================//
//函数说明：RTE主函数
//
//输入：无
//输出：无
//说明：
//===========================================================================//
void app_RTE_main (void)
{
	app_RTE_CAN_main ();
}

void app_RTE_init(void)
{
}



// __DATA(RAM4) uint8_t programFlag;// __attribute__ ((section("PROGRAM_RAM")));
/*进入boot模式*/
uint8_t bl_enterBootMode(void)
{
    uint8_t status;
    uint8_t tmpBuf[0x100];
    core_mm_copy(tmpBuf,kBootloaderConfigAreaAddress,0x100);
    status = NV_FlashEraseSector(kBootloaderConfigAreaAddress,0x800);
    core_dcm_writeU8(tmpBuf+0,0x02);
    core_dcm_writeU8(tmpBuf+16,0x01);
    status = NV_FlashProgram(kBootloaderConfigAreaAddress,0x100,tmpBuf);
    return status;
}
