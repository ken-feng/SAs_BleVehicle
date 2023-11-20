#ifndef __CCC_DK_CONFIG_H__
#define __CCC_DK_CONFIG_H__

#define TIMEOUT_MS_ON_PAIRING               30000   //配对超时，30s
#define TIMEOUT_MS_ON_KTS                   5000    //请求server响应KTS超时时间
#define TIMEOUT_MS_ON_RANGING               200    //拿定位结果间隔
#define TIMEOUT_MS_ON_RKE_EVENT             10000   //Event类型的车控超时时间
#define TIMEOUT_MS_ON_REQUIRE_CONFIRM       100     //if the vehicle does not receive a continuation confirmation within 100ms after a continuation confirmation request was sent, it may stop the function
#define TIMEOUT_MS_ON_RKE_ENDURING          20000   //整个enduring rke超时时间；
#define TIMEOUT_MS_ON_ANCHOR_WAKEUP         500    //锚点唤醒响应超时时间
#define TIMEOUT_MS_ON_RANGING_SETUP         500    //Ranging setup响应超时时间
#define TIMEOUT_MS_ON_TIME_SYNC             500     //时间同步响应超时时间
#define TIMEOUT_MS_ON_TIME_LOCATION_TO_APP  1000   //定时结果通知APP间隔
#define TIMEOUT_MS_ON_TIME_LOCATION         200    //定时间隔
#define TIMEOUT_MS_ON_TIME_LOCATION_EXPIRED 10000   //定位超时没响应时间10s

#define TIMEOUT_CONFIMR_NUM_ON_ENDURING_RKE (TIMEOUT_MS_ON_RKE_ENDURING/TIMEOUT_MS_ON_REQUIRE_CONFIRM)     //多少次confirm enduring之后，enduring with confirm rke超时;

#define MAX_DIGITAL_KEY_STORAGE             0x0C
#define NUM_URSK_PER_DIGITAL_KEY            0x02  //one preDrived, one active;
#define MAX_BLE_CONN_NUM                    2     //BLE连接个数，包括手机及FOB
#define TIME_NUM_IN_DEVICE                  3     //当前已用到3个
#define TIME_NUM_IN_VEHICLE                 4     //当前已用到4个

//URSK TTL(Time-To-Live) in usage count, The vehicle shall enforce an URSK TTL lower or equal to 12 hours (vehicle OEM specific)
#define URSK_TTL_IN_USAGE_COUNT             ( (80* 60 * 1000) / TIMEOUT_MS_ON_RANGING )  //TTL以URSK使用2小时来算
#define RANGING_RESULT_NOTICE_NUM_TO_SUSPEND 1500   //假设200ms一次定位结果通知，则在5分钟内暂停定位
#define RANGING_LOCATION_FIXED_NUM_TO_SUSPEND 600   //600 * 200 ms次定位结果通知，120s内位置区域固定，则可以暂停定位

#define CCC_SDK_VER_MAJOR                   0x01  //SDK版本高字节
#define CCC_SDK_VER_MINOR                   0x04  //SDK版本低字节

#define MIN_VALID_ANCHOR_NUM_TO_RANGING     0x01  //最少几个锚点有效才允许测距
#define RANGING_SAMPLE_FREQUENCY            0x03  //采样频率

#define CCC_DEBUG_FLAG                      0
//#define CCC_LOCATION_IN_MCU_SDK             //由MCU SDK来定位，不由SAController定位；            

#endif
