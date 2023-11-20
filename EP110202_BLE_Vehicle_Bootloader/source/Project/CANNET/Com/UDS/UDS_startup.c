/* @Encoding: UTF-8 */
/*!
 ********************************************************************
 * @file   : UDS_App.c
 * @brief  : Configuration APIs for the UDS_App module.
 * @author : 
 * @version: A.0.1
 * @release: @currentDateTime
 ********************************************************************
 *                      Description/Information
 * 1.
 * 2.
 ********************************************************************
 */

/* Includes*********************************************************/
#include "UDS_startup.h"
#include "crc32.h"
#include "bl_memory.h"
/* Macros & Typedef*************************************************/

/*Imported Interfaces***********************************************/

/*Enumerations & Structures*****************************************/

/*Variable Definitions**********************************************/

/* Public Function Prototypes***************************************/

extern u8 gEnsureCntValid;
void uds_startup_process(void)
{
    /*获取当前app模式*/
    if (UDS_get_appmode_valid() == 1U)
    {/*app模式*/
        /*获取当前app有效标志位*/
        if (UDS_get_appFlag_valid() == 1U)
        {/*app标志有效*/
            UDS_Jump_To_App();/*跳转到app*/
            while (1);
        }
    }
    else
    {
//        /*获取当前app有效标志位*/
//        if (UDS_get_appFlag_valid()== 1U)
//        {/*app标志有效*/
//            /*App有效标志有效，开始10S计时，超时则将App模式标志置为有效，重启MCU*/
////            uds_set_ensure_cnt_valid();
//            gEnsureCntValid = 1U;
//        }
    }
    /*后续进入boot流程*/
}


void uds_startup_init(void)
{
	if ((UDS_get_appmode_valid() != 1U)&&(UDS_get_appFlag_valid()== 1U))
	{/*App有效标志有效，开始10S计时，超时则将App模式标志置为有效，重启MCU*/
		gEnsureCntValid = 1U;
	}
}
