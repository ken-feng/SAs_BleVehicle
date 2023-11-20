#include <ComTask.h>
#include <BCanPdu.h>
#include <BCanPduCfg.h>
#include <BCanTrcv.h>
#include "UDS_App.h"
/* Variable Definitoins*********************************************/

/* Macros & Typedef*************************************************/

/* Public Function Prototypes***************************************/
/*!
 * \brief  Com_Init 网络任务初始化
 * \param  None
 * \return None
 */
void Com_Init(void)
{
  BCanPdu_Init();
  uds_init();
}

/*!
 * \brief  Com_1msTask 网络任务1
 * \param  None
 * \return None
 * \note   
 */
void Com_1msTask(void)
{
  uds_timer_ctrl();
  BCanTrcv_MsgTxHandler();
  BCanPdu_TxMainFunction();
}

/*!
 * \brief  Com_2msTask 网络任务2
 * \param  None
 * \return None
 * \note   
 */
void Com_2msTask(void)
{
  BCanPdu_RxMainFunction();
  
}

