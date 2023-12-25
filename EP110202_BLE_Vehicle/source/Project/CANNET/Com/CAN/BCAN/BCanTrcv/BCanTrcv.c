/* @Encoding: UTF-8 */
#include <BCanTrcv.h>
#include <BCanTrcvCfg.h>

/*!
 ********************************************************************
 * @file   : BCanTrcv.c
 * @brief  : Routines of the BCanTrcv module.
 * @author : J.L.
 * @version: A.0.1(Based on the old version by WangHu)
 * @release: @currentDateTime
 ********************************************************************
 *                      Description/Information
 * 1.
 * 2.
 ********************************************************************
 */
/* Includes*********************************************************/
/* Variable Definitoins*********************************************/
/*!
 * \brief  BCanTrcv_RxMsgHandler
 * \param  None
 * \return E_OK-正常; E_NOT_OK-错误
 */
//Modify (Ken):VEHICLE-V0C02 NO.2 -20231225
#if defined __FIT_Aeon_H
uint8 BCanTrcv_RxMsgHandler(flexcan_frame_t* pFrame)			// CAN Standard
#else
uint8 BCanTrcv_RxMsgHandler(flexcan_fd_frame_t* pFrame)			// CAN FD
#endif
{
    uint8 RetFlag = E_OK;
    uint8 i;
    CanMsg_t rawMsg;

    /* 空参断言 */
    if (NULL_PTR == pFrame) {return E_NOT_OK;}
    /* 将FlexCanFrame转换成通用Frame */
    rawMsg.ID = (uint32)pFrame->id; /* 读取ID */
    rawMsg.DataLen = (uint8)pFrame->length; /* 读取数据长度 */
    if (rawMsg.DataLen < 8) {return E_NOT_OK;} /* 数据长度小于8过滤掉 */
    core_mm_set(rawMsg.Data,0x00,64);

    //Modify (Ken):VEHICLE-V0C02 NO.2 -20231225
    #if defined __FIT_Aeon_H
    rawMsg.Data[0] = pFrame->dataByte0;
    rawMsg.Data[1] = pFrame->dataByte1;
    rawMsg.Data[2] = pFrame->dataByte2;
    rawMsg.Data[3] = pFrame->dataByte3;
    rawMsg.Data[4] = pFrame->dataByte4;
    rawMsg.Data[5] = pFrame->dataByte5;
    rawMsg.Data[6] = pFrame->dataByte6;
    rawMsg.Data[6] = pFrame->dataByte7;
    #else
	for ( i = 0; i < 16; i++)
	{
		core_dcm_writeBig32(rawMsg.Data+i*4,pFrame->dataWord[i]);
	}
    #endif

	if (BCanTrcv_IsAppMsg(rawMsg.ID)) {
		rawMsg.DataLen = 64U;
		RetFlag = BCanTrcv_WriteAppRxMsg(rawMsg.ID, rawMsg.Data, rawMsg.DataLen);
	}
    return RetFlag;
}

/*!
 * \brief  BCanTrcv_MsgTxHandler
 * \param  None
 * \return None 
 */
void BCanTrcv_MsgTxHandler(void)
{
    CanMsg_t* pMsg = NULL_PTR;
    CanPduTxResult_t tmpResult = CANPDU_TX_INVALID;

    while (1)
    {
        pMsg = BCanTrcv_ReadAppTxMsg();
        if (NULL_PTR == pMsg)
        {
             break;
        }
        else
        {
        	tmpResult = (CanPduTxResult_t)BCanTrcv_TxMsg(pMsg->ID, pMsg->Data, pMsg->DataLen);
        	if (0 == tmpResult)
        	{
        		tmpResult = CANPDU_TX_SUCCESS;
        	}
        	else if (1 == tmpResult)
        	{
        		tmpResult = CANPDU_TX_FAILED;
        	}
        	else
        	{
        		tmpResult = CANPDU_TX_PENDING;
        	}
        	BCanTrcv_SetTxResult(pMsg->ID, tmpResult);
        	if (CANPDU_TX_SUCCESS == tmpResult)
        	{
        		BCanTrcv_AppMsgTxResultProcess();
        	}
        }

        break;
    }
}


