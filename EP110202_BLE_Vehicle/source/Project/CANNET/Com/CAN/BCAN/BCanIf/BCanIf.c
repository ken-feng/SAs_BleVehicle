/* @Encoding: UTF-8 */
/*!
 ********************************************************************
 * @file   : BCanIf.c
 * @brief  : Routines of the BCanIf module.
 * @author : J.L.
 * @version: A.0.1
 * @release: @currentDateTime
 ********************************************************************
 *                      Description/Information
 * 1.
 * 2.
 ********************************************************************
 */
/* Includes*********************************************************/
#include <BCanIf.h>
#include <BCanIfCfg.h>

/*Variables*********************************************************/
static BCanIf_AppBuff_t l_BCanIfAppRxBuf;
static BCanIf_AppBuff_t l_BCanIfAppTxBuf;

/* Macros & Typedef*************************************************/

/* Public Function Prototypes***************************************/
/*!
 * \brief  BCanIf_ClearAppTxBuff 清空App报文发送缓存
 * \param
 * \return None
 */
void BCanIf_ClearAppTxBuff(void)
{
    l_BCanIfAppTxBuf.Stuffed = FALSE;
    l_BCanIfAppTxBuf.ReadIndex = 0;
    l_BCanIfAppTxBuf.WriteIndex = 0;
}

/*!
 * \brief  BCanIf_ClearAllTxBuff 清空所有报文发送缓存
 * \param
 * \return None
 */
void BCanIf_ClearAllTxBuff(void)
{
    l_BCanIfAppTxBuf.Stuffed = FALSE;
    l_BCanIfAppTxBuf.ReadIndex = 0;
    l_BCanIfAppTxBuf.WriteIndex = 0;
}


/*!
 * \brief  BCanIf_WriteAppRxMsg App接收报文写入队列
 * \param  id 报文ID
 * \param  pData 数据指针
 * \param  len 数据长度
 * \return None
 */
unsigned char g_CANAppMsgBuffUsage;
uint8 BCanIf_WriteAppRxMsg(uint16 id, volatile uint8* pData, uint8 len)
{
    {
        l_BCanIfAppRxBuf.RawMsgs[l_BCanIfAppRxBuf.WriteIndex].ID = id;
        l_BCanIfAppRxBuf.RawMsgs[l_BCanIfAppRxBuf.WriteIndex].DataLen = len;
        BCanIf_Copy(l_BCanIfAppRxBuf.RawMsgs[l_BCanIfAppRxBuf.WriteIndex].Data, pData, len);
        if (++l_BCanIfAppRxBuf.WriteIndex >= N_BCANIF_APP_BUF)
        {
            l_BCanIfAppRxBuf.WriteIndex = 0;
        }
        if (l_BCanIfAppRxBuf.WriteIndex == l_BCanIfAppRxBuf.ReadIndex)
        {
            l_BCanIfAppRxBuf.Stuffed = TRUE;
        }
        if (l_BCanIfAppRxBuf.WriteIndex > l_BCanIfAppRxBuf.ReadIndex)
        {
            g_CANAppMsgBuffUsage = l_BCanIfAppRxBuf.WriteIndex - l_BCanIfAppRxBuf.ReadIndex;
        }
        else
        {
            g_CANAppMsgBuffUsage = l_BCanIfAppRxBuf.WriteIndex + N_BCANIF_APP_BUF - l_BCanIfAppRxBuf.ReadIndex;
        }
        return 1;
    }
}

/*!
 * \brief  BCanIf_ReadAppRxMsg 从App接收队列读取报文
 * \param  pMsg 指向报文的指针
 * \return None
 */
uint8 BCanIf_ReadAppRxMsg(CanMsg_t* pMsg)
{
    if ((l_BCanIfAppRxBuf.ReadIndex != l_BCanIfAppRxBuf.WriteIndex)
     || l_BCanIfAppRxBuf.Stuffed)
    {
        pMsg->ID = l_BCanIfAppRxBuf.RawMsgs[l_BCanIfAppRxBuf.ReadIndex].ID;
        pMsg->DataLen = l_BCanIfAppRxBuf.RawMsgs[l_BCanIfAppRxBuf.ReadIndex].DataLen;
        BCanIf_Copy(pMsg->Data, l_BCanIfAppRxBuf.RawMsgs[l_BCanIfAppRxBuf.ReadIndex].Data, pMsg->DataLen);
        if(++l_BCanIfAppRxBuf.ReadIndex >= N_BCANIF_APP_BUF)
        {
            l_BCanIfAppRxBuf.ReadIndex = 0;
        }
        l_BCanIfAppRxBuf.Stuffed = FALSE;
        return 1;
    }
    else
    {
        return 0;
    }
}

/*!
 * \brief  BCanIf_WriteAppTxMsg App发送报文写入发送队列
 * \param  pMsg 指向报文的指针
 * \return None
 */
uint8 BCanIf_WriteAppTxMsg(CanMsg_t* pMsg)
{
	if(!l_BCanIfAppTxBuf.Stuffed)
    {
        l_BCanIfAppTxBuf.RawMsgs[l_BCanIfAppTxBuf.WriteIndex].ID = pMsg->ID;
        l_BCanIfAppTxBuf.RawMsgs[l_BCanIfAppTxBuf.WriteIndex].DataLen = pMsg->DataLen;
        BCanIf_Copy(l_BCanIfAppTxBuf.RawMsgs[l_BCanIfAppTxBuf.WriteIndex].Data, pMsg->Data, pMsg->DataLen);
        if (++l_BCanIfAppTxBuf.WriteIndex >= N_BCANIF_APP_BUF)
        {
            l_BCanIfAppTxBuf.WriteIndex = 0;
        }
        if (l_BCanIfAppTxBuf.WriteIndex == l_BCanIfAppTxBuf.ReadIndex)
        {
            l_BCanIfAppTxBuf.Stuffed = TRUE;
        }
        return 1;
    }
    else
    {
        return 0;
    }
}

/*!
 * \brief  BCanIf_ReadAppTxMsg 从App发送队列中读取报文
 * \param  None
 * \return 指向 CanMsg_t 类型的指针
 */
CanMsg_t* BCanIf_ReadAppTxMsg(void)
{
    CanMsg_t* pMsg = NULL_PTR;

    if ((l_BCanIfAppTxBuf.ReadIndex != l_BCanIfAppTxBuf.WriteIndex) || l_BCanIfAppTxBuf.Stuffed)
    {
    	pMsg = &l_BCanIfAppTxBuf.RawMsgs[l_BCanIfAppTxBuf.ReadIndex];
//        if (++l_BCanIfAppTxBuf.ReadIndex >= N_BCANIF_APP_BUF)
//        {
//            l_BCanIfAppTxBuf.ReadIndex = 0;
//        }
//        l_BCanIfAppTxBuf.Stuffed = FALSE; //cmt by shawn, 20210629, 底层发送失败也会清除缓存导致丢报文
    }
    return pMsg;
}

/*!
 * \brief  BCanIf_AppMsgTxResultProcess App报文发送结果处理
 * \param  None
 * \return None
 */
void BCanIf_AppMsgTxResultProcess(void)
{
	if ((l_BCanIfAppTxBuf.ReadIndex != l_BCanIfAppTxBuf.WriteIndex) || l_BCanIfAppTxBuf.Stuffed)
	{
		if (++l_BCanIfAppTxBuf.ReadIndex >= N_BCANIF_APP_BUF)
		{
			l_BCanIfAppTxBuf.ReadIndex = 0;
		}
		l_BCanIfAppTxBuf.Stuffed = FALSE;
	}
}
