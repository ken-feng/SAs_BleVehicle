/* @Encoding: UTF-8 */
/*!
 ********************************************************************
 * @file   : BCanPdu.c
 * @brief  : Routines of the BCanPdu module.
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
#include <BCanPdu.h>
#include <BCanPduCfg.h>

/* Variable Definitoins*********************************************/
static CanMsg_t s_BCanPduRxMsg;
static CanMsg_t s_BCanPduTxMsg;

/*Private Interfaces************************************************/
static boolean BCanPdu_TxMsg(uint8 index);

/* Macros & Typedef*************************************************/

/* Public Function Prototypes***************************************/
/*!
 * \brief  BCanPdu_IsAppMsg 检查是否为App报文
 * \param  id 报文ID
 * \return FALSE-非App报文; TRUE-App报文
 */
boolean BCanPdu_IsAppMsg(uint16 id)
{
    uint8 i;
    boolean retFlag = FALSE;

    for (i = 0; i < BCANPDU_N_RX; i++) {
        if (id == g_BCanRxPduList[i].ID) {
            retFlag = TRUE;
            break;
        }
    }
    
    return retFlag;
}

/*!
 * \brief  BCanPdu_TxMsg 发送指定报文
 * \param  index 报文索引值
 * \return FALSE-失败; TRUE-成功
 */
static boolean BCanPdu_TxMsg(uint8 index)
{
    s_BCanPduTxMsg.ID = g_BCanTxPduList[index].ID;
    s_BCanPduTxMsg.DataLen = g_BCanTxPduList[index].DataLen;
    BCanPdu_Copy(s_BCanPduTxMsg.Data, 
                 g_BCanTxPduList[index].pData, 
                 g_BCanTxPduList[index].DataLen);
    (void)BCanPdu_WriteAppTxMsg(&s_BCanPduTxMsg);

    return TRUE;
}

/*!
 * \brief  BCanPdu_Init PDU 模块初始化
 * \param  None
 * \return None
 */
void BCanPdu_Init(void)
{
    BCanPdu_VarInit();
}
/*!
 * \brief  BCanPdu_UpdateRxMsg 更新接收报文
 * \param  pMsg 指向 CanMsg_t 类型的指针
 * \return None
 * \note   在接收中断或者接收处理回调中运行
 */
void BCanPdu_UpdateRxMsg(CanMsg_t* pMsg)
{
    uint8 i;

    for (i = 0; i < BCANPDU_N_RX; i++) {
        if (pMsg->ID == g_BCanRxPduList[i].ID)  {
            if (pMsg->DataLen >= g_BCanRxPduList[i].DataLen) {
                if (g_BCanRxPduList[i].RxEnabled) {
                    BCanPdu_Copy(g_BCanRxPduList[i].pData, 
                                 pMsg->Data, 
                                 pMsg->DataLen);
                    g_BCanRxPduList[i].RxUpdated = TRUE;

                    g_BCanRxPduList[i].RxStatus = CANPDU_RX_UPDATE_STATUS;
                    if (NULL_PTR != g_BCanRxPduList[i].RxHandleFunc)
                    {
                        g_BCanRxPduList[i].RxHandleFunc(g_BCanRxPduList[i].RxStatus);
                    }
                }
                break;
            }
        }
    }
}
/*!
 * \brief  BCanPdu_CheckRxMsgLost 检查报文丢失情况
 * \param  None
 * \return None
 */
static void BCanPdu_CheckRxMsgLost(void)
{
    uint8 i;

    for (i = 0; i < BCANPDU_N_RX; i++)
    {
        if (g_BCanRxPduList[i].RxUpdated)
        {
            g_BCanRxPduList[i].RxUpdated = FALSE;
            g_BCanRxPduList[i].LostConfirmed = FALSE;
            g_BCanRxPduList[i].LostTick = 0;
            if (g_BCanRxPduList[i].LostConfirmed)
            {
                if (++g_BCanRxPduList[i].RecovConfirmCnt >=
                    g_BCanRxPduList[i].RecovConfirmTimes)
                {
                    g_BCanRxPduList[i].RecovConfirmCnt = 0;
                    g_BCanRxPduList[i].LostConfirmed = FALSE;
                }
            }
            if (!g_BCanRxPduList[i].LostConfirmed)
            {
                g_BCanRxPduList[i].RecovConfirmCnt = 0;
                g_BCanRxPduList[i].RxStatus = CANPDU_RX_UPDATE_STATUS;
                if (NULL_PTR != g_BCanRxPduList[i].RxHandleFunc)
                {
                	g_BCanRxPduList[i].RxHandleFunc(g_BCanRxPduList[i].RxStatus);
                }
            }
        }
        else
        {
            if (g_BCanRxPduList[i].LostTick >=
                (BCANPDU_RX_LOST_CYCLE_RATE * g_BCanRxPduList[i].NormalCycle))
            {
                g_BCanRxPduList[i].LostTick = 0;
                g_BCanRxPduList[i].LostConfirmed = TRUE;
                g_BCanRxPduList[i].RxStatus = CANPDU_RX_LOST_STATUS;
                if (NULL_PTR != g_BCanRxPduList[i].RxHandleFunc)
                {
                	g_BCanRxPduList[i].RxHandleFunc(g_BCanRxPduList[i].RxStatus);
                }
                g_BCanRxPduList[i].RecovConfirmCnt = 0;
            }
            else
            {
                g_BCanRxPduList[i].LostTick += BCANPDU_T_RX_DISPATCH;
            }
        }
    }
}

/*!
 * \brief  BCanPdu_RxMainFunction 接收主函数
 * \param  None
 * \return None
 */
void BCanPdu_RxMainFunction(void)
{
    while (1) {
        if (BCanPdu_ReadAppRxMsg(&s_BCanPduRxMsg)) {
            BCanPdu_UpdateRxMsg(&s_BCanPduRxMsg);
        } else {
            break;
        }
    }
    // BCanPdu_CheckRxMsgLost();
}

/*!
 * \brief  BCanPdu_TxCycleMsg 发送周期报文
 * \param  index 发送索引值
 * \return None
 */
static void BCanPdu_TxCycleMsg(uint8 index)
{
    CanTxPduHandle_t *pMsg = NULL_PTR;

    if (index >= BCANPDU_N_TX) {
        return;
    }
    pMsg = &g_BCanTxPduList[index];
    if (pMsg->NormalCycleTick >= pMsg->NormalCycle) {
        pMsg->TxHandleFunc(CANPDU_TX_UPDATE_STATUS);
        BCanPdu_TxMsg(index);
        pMsg->NormalCycleTick = BCANPDU_T_TX_DISPATCH;
    } else {
        pMsg->NormalCycleTick += BCANPDU_T_TX_DISPATCH;
    }
}

/*!
 * \brief  BCanPdu_TxEventMsg 发送事件报文
 * \param  index 发送索引值
 * \return None
 */
static void BCanPdu_TxEventMsg(uint8 index)
{
	CanTxPduHandle_t *pMsg = NULL_PTR;

	if (index >= BCANPDU_N_TX) {
        return;
    }
	pMsg = &g_BCanTxPduList[index];
    if (pMsg->ImmediateTxEnabled) {
        if (0 == pMsg->ImmediateCnt) {
            pMsg->ImmediateCnt = pMsg->ImmediateTimes;
            pMsg->ImmediateTxEnabled = FALSE;
        }
    }
    
    if (pMsg->ImmediateCycleTick >= pMsg->ImmediateCycle) {
        if (pMsg->ImmediateCnt > 0) {
            pMsg->ImmediateCnt--;
            pMsg->TxHandleFunc(CANPDU_TX_UPDATE_STATUS);
            BCanPdu_TxMsg(index);
            pMsg->ImmediateCycleTick = BCANPDU_T_TX_DISPATCH;

            if (0 == pMsg->ImmediateCnt) {
                pMsg->TxHandleFunc(CANPDU_TX_CLEAR_STATUS);
            }
        }
    } else {
        pMsg->ImmediateCycleTick += BCANPDU_T_TX_DISPATCH;
    }
}

/*!
 * \brief  BCanPdu_TxCycleEventMsg 发送周期事件报文
 * \param  index 发送索引值
 * \return None
 */
static void BCanPdu_TxCycleEventMsg(uint8 index)
{
	CanTxPduHandle_t *pMsg = NULL_PTR;

	if (index >= BCANPDU_N_TX) {
        return;
    }
	pMsg = &g_BCanTxPduList[index];
    if (pMsg->NormalCycleTick >= pMsg->NormalCycle) {
        pMsg->NormalTxEnabled = TRUE;
        pMsg->NormalCycleTick = BCANPDU_T_TX_DISPATCH;
    } else {
        pMsg->NormalCycleTick += BCANPDU_T_TX_DISPATCH;
    }

    if (pMsg->ImmediateTxEnabled) {
        if (0 == pMsg->ImmediateCnt) {
            pMsg->ImmediateCnt = pMsg->ImmediateTimes;
            pMsg->ImmediateTxEnabled = FALSE;
        }
    }

    if (pMsg->ImmediateCycleTick >= pMsg->ImmediateCycle) {
        if (pMsg->ImmediateCnt > 0) {
            pMsg->ImmediateCnt--;
            pMsg->TxHandleFunc(CANPDU_TX_UPDATE_STATUS);
            BCanPdu_TxMsg(index);
            pMsg->ImmediateCycleTick = BCANPDU_T_TX_DISPATCH;

            if (0 == pMsg->ImmediateCnt) {
                pMsg->TxHandleFunc(CANPDU_TX_CLEAR_STATUS);
            }
        } else {
            if (pMsg->NormalTxEnabled) {
                pMsg->NormalTxEnabled = FALSE;
                pMsg->TxHandleFunc(CANPDU_TX_UPDATE_STATUS);
                BCanPdu_TxMsg(index);
                pMsg->NormalCycleTick = BCANPDU_T_TX_DISPATCH;
            }
        }
    } else {
        pMsg->ImmediateCycleTick += BCANPDU_T_TX_DISPATCH;
    }
}

/*!
 * \brief  BCanPdu_TxMainFunction 发送主函数
 * \param  index 发送索引值
 * \return None
 */
void BCanPdu_TxMainFunction(void)
{
    uint8 i = 0;

    for (i = 0; i < BCANPDU_N_TX; i++)
    {
		switch (g_BCanTxPduList[i].SendType)
		{
			case CAN_CYCLE_MSG:
				BCanPdu_TxCycleMsg(i);
				break;

			case CAN_EVENT_MSG:
				BCanPdu_TxEventMsg(i);
				break;

			case CAN_CE_MSG:
				BCanPdu_TxCycleEventMsg(i);
				break;

			default:
				break;
		}
    }
}

