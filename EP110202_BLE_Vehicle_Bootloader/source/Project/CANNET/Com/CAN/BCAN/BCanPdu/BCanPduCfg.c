/* @Encoding: UTF-8 */
/*!
 ********************************************************************
 * @file   : BCanPduCfg.c
 * @brief  : Configuration APIs for the BCanPdu module.
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
#include <BCanPduCfg.h>
#include "app_RTE.h"
#include "UDS_App.h"
/*Private Interface Forward Declaration*****************************/
/*!
 * \defgroup SA 接受报文处理函数
 * \{
 */
static void BCanPdu_RxHandler_SA_0x631(CanPduRxStatus_t status);

static void BCanPdu_TxHandler_OBD_0x639(CanPduTxStatus_t status);
/*Variable Definitions**********************************************/
/*!
 * \defgroup SA发送报文定义
 * \{
 */
BCanPdu_SA_0x631_t g_BCanPdu_SA_0x631 = {{0x00}};  

/*!
 * \defgroup 发送报文定义
 * \{
 */

BCanPdu_OBD_0x639_t g_BCanPdu_OBD_0x639 = {{0x00}};  

/*Variable Definitions**********************************************/

/*!
 * \defgroup BCAN 发送 PDU 配置列表定义
 * \{
 */
CanTxPduHandle_t g_BCanTxPduList[BCANPDU_N_TX] =
{
    { /* 1 */
    	CAN_EVENT_MSG, 0x639, g_BCanPdu_OBD_0x639.Data,
        64, 0, 0, FALSE, 20, 20, FALSE, 1, 0, 
        CANPDU_TX_CLEAR_STATUS, CANPDU_TX_READY, BCanPdu_TxHandler_OBD_0x639
    },
};
/*!
 * \}
 */
static void BCanPdu_TxHandler_OBD_0x639(CanPduTxStatus_t status)
{
    switch (status) {
    	case CANPDU_TX_CLEAR_STATUS:
            BCanPdu_ClearTxPdu(0x639);
            break;
        case CANPDU_TX_UPDATE_STATUS:
        
            break;
			
        default:
            break;
    }
}

/*!
 * \}
 */

/*!
 * \defgroup BCAN 接收 PDU 配置列表定义
 * \{
 */
CanRxPduHandle_t g_BCanRxPduList[BCANPDU_N_RX] =
{
    /* SA事件型报文 */
    { /* 1 */
        CAN_EVENT_MSG, 0x631, g_BCanPdu_SA_0x631.Data,  
	    64, TRUE, FALSE, 20,  0, FALSE,
	    1, 0, CANPDU_RX_LOST_STATUS,
	    BCanPdu_RxHandler_SA_0x631,   NULL_PTR
    },
};



static void BCanPdu_RxHandler_SA_0x631(CanPduRxStatus_t status)
{
    switch(status) {
        case CANPDU_RX_LOST_STATUS:
            break;
        case CANPDU_RX_UPDATE_STATUS:
            uds_app_process(g_BCanPdu_SA_0x631.Data,64U);
            break;
        default:
            break;
    }
}
/*!
 * \}
 */

void BCanPdu_Get_SA601_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_SA_0x631.Data,64U);
}


void BCanPdu_Get_OBD639_Data(uint8* data)
{
    core_mm_copy(data,g_BCanPdu_OBD_0x639.Data,64U);
}

void BCanPdu_Set_OBD639_Data(uint8* data)
{
	core_mm_copy(g_BCanPdu_OBD_0x639.Data,data,64U);
    if (BCanPdu_EnableTxEventMsg(0x639))
    {
    }
}
/*!
 * \brief  BCanPdu_VarInit, 初始化BCAN报文变量
 * \param  None
 * \return None
 */
void BCanPdu_VarInit(void)
{
    uint8 i;
    /* 接收PDU初始化 */
    for (i = 0; i < BCANPDU_N_RX; i++)
    {
        BCanPdu_Fill(g_BCanRxPduList[i].pData, 0x00, g_BCanRxPduList[i].DataLen);
        g_BCanRxPduList[i].RxUpdated = FALSE;
        g_BCanRxPduList[i].LostTick = 0;
        g_BCanRxPduList[i].LostConfirmed = FALSE;
        g_BCanRxPduList[i].RecovConfirmCnt = 0;
        g_BCanRxPduList[i].RxStatus = CANPDU_RX_LOST_STATUS;
       
    }
    //LOG_L_S(CAN_MD,"Send Pdu Init:\n\r");
    /* 发送PDU初始化 */
    for (i = 0; i < BCANPDU_N_TX; i++)
    {
        BCanPdu_Fill(g_BCanTxPduList[i].pData, 0x00, g_BCanTxPduList[i].DataLen);
        g_BCanTxPduList[i].NormalCycleTick = g_BCanTxPduList[i].NormalCycle;
        g_BCanTxPduList[i].ImmediateCycleTick = g_BCanTxPduList[i].ImmediateCycle;
        g_BCanTxPduList[i].ImmediateCnt = 0;
        g_BCanTxPduList[i].TxStatus = CANPDU_TX_CLEAR_STATUS;
    }
}

/*!
 * \brief  BCanPdu_ClearTxPdu 清除指定发送报文的控制变量
 * \param  id 发送报文ID
 * \return None
 */
void BCanPdu_ClearTxPdu(uint32 id)
{
    uint8 i = 0;

    for (i = 0; i < BCANPDU_N_TX; i++)
    {
        if (id == g_BCanTxPduList[i].ID)
        {
            BCanPdu_Fill(g_BCanTxPduList[i].pData, 0x00, g_BCanTxPduList[i].DataLen);
            // g_BCanTxPduList[i].NormalCycleTick = g_BCanTxPduList[i].NormalCycle;
            g_BCanTxPduList[i].ImmediateCycleTick = g_BCanTxPduList[i].ImmediateCycle;
            g_BCanTxPduList[i].ImmediateCnt = 0;
            g_BCanTxPduList[i].TxStatus = CANPDU_TX_CLEAR_STATUS;
            break;
        }
    }
}

/*!
 * \brief  BCanPdu_ClearRxPdu 清除指定接收报文的控制变量
 * \param  id 接收报文ID
 * \return None
 */
void BCanPdu_ClearRxPdu(uint32 id)
{
    uint8 i = 0;

    for (i = 0; i < BCANPDU_N_RX; i++)
    {
        if (id == g_BCanRxPduList[i].ID)
        {
            BCanPdu_Fill(g_BCanRxPduList[i].pData, 0x00, g_BCanRxPduList[i].DataLen);
            g_BCanRxPduList[i].RxUpdated = FALSE;
            g_BCanRxPduList[i].RecovConfirmCnt = 0;
            g_BCanRxPduList[i].RxStatus = CANPDU_RX_LOST_STATUS;
            break;
        }
    }
}

/*!
 * \brief  BCanPdu_GetTxStatus 查询发送报文当前状态
 * \param  id 发送报文ID
 * \return CANPDU_TX_CLEAR_STATUS 或 CANPDU_TX_UPDATE_STATUS
 */
CanPduTxStatus_t BCanPdu_GetTxStatus(uint32 id)
{
    uint8 i = 0;
    CanPduTxStatus_t retStatus = CANPDU_TX_CLEAR_STATUS;

    for (i = 0; i < BCANPDU_N_TX; i++) {
        if (id == g_BCanTxPduList[i].ID) {
            retStatus = g_BCanTxPduList[i].TxStatus;
            break;
        }
    }

    return retStatus;
}

/*!
 * \brief  BCanPdu_GetRxStatus 查询接收报文当前状态
 * \param  id 接收报文ID
 * \return CANPDU_RX_LOST_STATUS 或 CANPDU_RX_UPDATE_STATUS
 */
CanPduRxStatus_t BCanPdu_GetRxStatus(uint32 id)
{
    uint8 i = 0;
    CanPduRxStatus_t retStatus = CANPDU_RX_LOST_STATUS;

    for (i = 0; i < BCANPDU_N_RX; i++) {
        if (id == g_BCanRxPduList[i].ID) {
            retStatus = g_BCanRxPduList[i].RxStatus;
            break;
        }
    }

    return retStatus;
}

/*!
 * \brief  BCanPdu_SetTxResult 设置报文发送结果
 * \param  id 待设置发送结果的报文ID
 * \param  result 待设置的发送结果
 * \return TRUE-发送队列包含指定ID; FALSE-发送队列不包含指定ID
 */
boolean BCanPdu_SetTxResult(uint32 id, CanPduTxResult_t result)
{
	uint8 i = 0;
	boolean retFlag = FALSE;

	for (i = 0; i < BCANPDU_N_TX; i++) {
		if (id == g_BCanTxPduList[i].ID) {
			g_BCanTxPduList[i].TxResult = result;
			retFlag = TRUE;
			break;
		}
	}

	return retFlag;
}

/*!
 * \brief  BCanPdu_GetTxResult 查询报文发送结果
 * \param  id 待查询发送结果的报文ID
 * \return None
 */
CanPduTxResult_t BCanPdu_GetTxResult(uint32 id)
{
	uint8 i = 0;
	CanPduTxResult_t retVal = CANPDU_TX_INVALID;

	for (i = 0; i < BCANPDU_N_TX; i++) {
		if (id == g_BCanTxPduList[i].ID) {
			retVal = g_BCanTxPduList[i].TxResult;
			break;
		}
	}

	return retVal;
}

/*!
 * \brief  BCanPdu_ClearTxResult 清除报文发送结果
 * \note   调用BCanPdu_GetTxResult函数后需要调用此函数, 否则会造成下一次读取发送结果错误
 * \param  id 待设置发送结果的报文ID
 * \return TRUE-发送队列包含指定ID; FALSE-发送队列不包含指定ID
 */
boolean BCanPdu_ClearTxResult(uint32 id)
{
	uint8 i = 0;
	boolean retFlag = FALSE;

	for (i = 0; i < BCANPDU_N_TX; i++) {
		if (id == g_BCanTxPduList[i].ID) {
			g_BCanTxPduList[i].TxResult = CANPDU_TX_READY;
			retFlag = TRUE;
			break;
		}
	}

	return retFlag;
}

/*!
 * \brief  BCanPdu_EnableTxEventMsg 使能事件报文发送
 * \param  id 发送报文ID
 * \return TRUE-成功; FALSE-失败
 */
boolean BCanPdu_EnableTxEventMsg(uint32 id)
{
    uint8 i = 0;
    boolean retFlag = FALSE;

    for (i = 0; i < BCANPDU_N_TX; i++) {
        if (id == g_BCanTxPduList[i].ID) {
            if (CANPDU_TX_CLEAR_STATUS == g_BCanTxPduList[i].TxStatus) {
                g_BCanTxPduList[i].ImmediateTxEnabled = TRUE;
                retFlag = TRUE;
            } else {
                retFlag = FALSE;
            }
            break;
        }
    }

    return retFlag;
}

/*!
 * \brief  BCanPdu_DisableTxEventMsg 停止事件报文发送
 * \param  id 发送报文ID
 * \return TRUE-成功; FALSE-失败
 */
boolean BCanPdu_DisableTxEventMsg(uint32 id)
{
    uint8 i = 0;
    boolean retFlag = FALSE;

    for (i = 0; i < BCANPDU_N_TX; i++) {
        if (id == g_BCanTxPduList[i].ID) {
            g_BCanTxPduList[i].ImmediateTxEnabled = FALSE;
            g_BCanTxPduList[i].ImmediateCnt = 0;
            g_BCanTxPduList[i].TxStatus = CANPDU_TX_CLEAR_STATUS;
            g_BCanTxPduList[i].ImmediateCycleTick = g_BCanTxPduList[i].ImmediateCycle;
            retFlag = TRUE;
            break;
        }
    }
    return retFlag;
}

/*!
 * \brief  BCanPdu_ReadRawMsg, 根据报文ID读取报文
 * \param  id 输入ID
 * \return 指向CanRawMsg_t类型的指针
 */
CanMsg_t* BCanPdu_ReadRawMsg(uint32 id)
{
    uint8 i, j;
    static CanMsg_t msg;
    CanMsg_t* pRetMsg = NULL_PTR;
    boolean found = FALSE;

    /* 扫描发送列表 */
    for (i = 0; i < BCANPDU_N_TX; i++) {
        if (g_BCanTxPduList[i].ID == id) {
        	msg.ID = g_BCanTxPduList[i].ID;
        	msg.DataLen = g_BCanTxPduList[i].DataLen;
        	for (j = 0; j < g_BCanTxPduList[i].DataLen; j++) {
        		msg.Data[j] = g_BCanTxPduList[i].pData[j];
        	}
        	found = TRUE;
            break;
        }
    }
     
    /* 扫描接收列表 */
    if (!found) {
        for (i = 0; i < BCANPDU_N_RX; i++) {
            if (g_BCanRxPduList[i].ID == id) {
            	msg.ID = g_BCanRxPduList[i].ID;
            	msg.DataLen = g_BCanRxPduList[i].DataLen;
            	for (j = 0; j < g_BCanRxPduList[i].DataLen; j++) {
            		msg.Data[j] = g_BCanRxPduList[i].pData[j];
            	}
                found = TRUE;
                break;
            }
        }
    }

    /* 返回找到结果 */
    if (found) {
        pRetMsg = &msg;
    }
    return pRetMsg;
}
