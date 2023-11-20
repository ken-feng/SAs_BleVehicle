
/* Includes -----------------------------------------------------------------*/
#include "se_iso7816_3_t1.h"
#include "se_transport.h"
#include "se_common_type.h"
#include "se_apdu_api.h"


u8_t api_get_sdk_version(u8_t* pOutVer)
{
	pOutVer[0] = SDK_VER_MAJOR;
	pOutVer[1] = SDK_VER_MINOR;

	return 0x02;
}

void apdu_1_init(struct t1_state_t *t1, u8_t src, u8_t dst)
{
	isot1_init(t1);
	isot1_bind(t1, src, dst);

#ifdef CONFIG_FEATURE_GP_SPI
	if(t1->pcfg->delay)
	{
		t1->pcfg->delay(100);
	}
	isot1_request_cip(t1);
	if(t1->pcfg->delay)
	{
		t1->pcfg->delay(100);
	}
	isot1_negotiate_ifsd(t1, t1->ifsc);

	//isot1_reset(t1);
#endif
}


struct t1_state_t t1_attr;

/**< 上电初始化 */
void api_power_on_init(hsm_ctrl_attr_t* hsm_ctrl_attr)
{
	t1_attr.pcfg = hsm_ctrl_attr;
	t1_attr.spiResetNum = 0;
	t1_attr.spiResetStatus = 0;

	if(t1_attr.pcfg->init)
	{
		t1_attr.pcfg->init();
	}
	apdu_1_init(&t1_attr, 2, 1);  /* NAD= 0x12 */
}

void api_t1_recover()
{
	apdu_1_init(&t1_attr, 2, 1);  /* NAD= 0x12 */
}

static int api_apdu_transceive_1(u8_t* sendBuffer, u16_t sendLength,u8_t* recvRspBuffer, u16_t recvLength)
{
	return isot1_transceive(&t1_attr,sendBuffer,sendLength,recvRspBuffer,recvLength);
}

static int func_se_reset()
{
	u8_t apdu[] = {0xFE, 0x31, 0x00, 0x00, 0x00};
	u8_t apduResp[0x20];
	if(t1_attr.pcfg->reset)
	{
		t1_attr.pcfg->reset();
		if(t1_attr.pcfg->delay)
		{
			t1_attr.pcfg->delay(100);
		}

		apdu_1_init(&t1_attr, 2, 1);  /* NAD= 0x12 */

		return api_apdu_transceive(apdu, 5, apduResp, 0x20);
	}
	else
	{
		return -1;
	}
}

int api_apdu_transceive(u8_t* sendBuffer, u16_t sendLength,u8_t* recvRspBuffer, u16_t recvLength)
{
	int dlCode = 0;
	int resLen = isot1_transceive(&t1_attr,sendBuffer,sendLength,recvRspBuffer,recvLength);
#if 1
	if(resLen < 2)
	{
		if(t1_attr.spiResetNum == 0)
        {
            t1_attr.spiResetStatus = 0;
            t1_attr.spiResetNum++;

            dlCode = func_se_reset();
            if(dlCode >= 2)
            {
				if(t1_attr.spiResetNum >= 1)
				{
					resLen = 0;  //第一次复位成功;
				}
                t1_attr.spiResetNum = 0;
            }
        }
        else
        {
            t1_attr.spiResetStatus = 1;
        }
	}
	else
	{
		if (t1_attr.spiResetStatus)
        {
            t1_attr.spiResetStatus = 0;
            //func_se_reset();
        }
	}
#endif

	return resLen;
}

