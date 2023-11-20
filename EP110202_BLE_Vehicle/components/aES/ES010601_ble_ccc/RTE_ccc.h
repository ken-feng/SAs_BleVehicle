#ifndef _RTE_CCC_H_
#define _RTE_CCC_H_
//#include "ES010101.h"
#include "gap_interface.h"


#define RTE_UWB_SDK_Get_Capabilty       //NULL_PTR /*获取UWB SDK中参数*/
#define RTE_UWB_SDK_Notify_Setup_Info   //NULL_PTR /*获取UWB SDK中参数*/
#define RTE_L2ca_SendLeCbData L2ca_SendLeCbData

#define RTE_SDK_BLEDKmsg_recv     /*BLE 透传外部设备下发的全部报文数据*/ 
#define RTE_SDK_BLEDKmsg_notify   /*BLE 通知触发事件通知，用于 BLE 和 SDK 模块组件之前的信息交互*/  

#define RTE_BLE_GAP_LE_READ_PHY     Gap_LeReadPhy    
#define RTE_BLE_GAP_LE_SET_PHY      Gap_LeSetPhy

//#define RTE_BLE_SEND_LE_SET_PHY_REQUEST ble_ccc_send_leSetPhyRequest


#endif
