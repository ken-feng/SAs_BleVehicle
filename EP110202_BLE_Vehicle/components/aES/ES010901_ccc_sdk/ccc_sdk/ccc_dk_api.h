#ifndef __CCC_DK_API_H__
#define __CCC_DK_API_H__

#include "../ccc_sdk/ccc_dk_type.h"

struct ccc_config_t
{
    hsm_ctrl_attr_t *hsmConfig;
    sw_algorithm_t  *swAlgorithm;
    timer_tool_t    *timerTool;

    pExtFunc_debug_printf debugPrintf;
    pExtFunc_get_info getInfo;
    pExtFunc_get_utc_time getUtcTime;
    pExtFunc_set_utc_time setUtcTime;
    pExtFunc_send_data sendData;
    pExtFunc_sdk_event_notice eventNotice;
};

extern struct ccc_config_t* cccConfig;

CCCErrCode_e ccc_api_module_init(struct ccc_config_t *pConfig);
CCCErrCode_e ccc_api_extern_event_notice(ChannleID_e channleId, ExtEvent_e extEvent, u8* pInData, u16 pInDataLen);
CCCErrCode_e ccc_api_data_request(ChannleID_e chId, u8* pdata, u16 pdataLen);


#endif
