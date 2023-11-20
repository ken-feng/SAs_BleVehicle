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
#include "UDS_ext_api.h"

#include "BCanPduCfg.h"
/* Macros & Typedef*************************************************/

/*Imported Interfaces***********************************************/

/*Enumerations & Structures*****************************************/

/*Variable Definitions**********************************************/

/* Public Function Prototypes***************************************/

u8 UDS_set_appmode_valid(u8 flag)
{
    /*调用设置App模式接口*/
	if(flag != 1U)
	{
		bl_enterBootMode();
	}
    return 1;
}

void UDS_ext_reset_system(void)
{
    /*调用芯片复位API*/
    NVIC_SystemReset();
    while (1);
}



void UDS_ext_send_data(u8* buf, u16 length)
{
    BCanPdu_Set_OBD609_Data(buf);
}
