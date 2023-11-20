/* @Encoding: UTF-8 */
/*!
 ********************************************************************
 * @file   : RTE_General.h
 * @brief  : Interfaces for all mudules.
 * @author : J.L.
 * @version: A.0.1
 * @release: @currentDateTime
 ********************************************************************
 *                      Description/Information
 * 1.
 * 2.
 ********************************************************************
 */
#ifndef	RTE_GENERAL_H
#define	RTE_GENERAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes*********************************************************/
#include <StandardTypes.h>
#include <assert.h>

/* Macros & Typedef*************************************************/
/*!
 * \brief 调试开关, 便于发布正式软件后关闭调试相关
 */
#define RTE_DEBUG_ENABLED     0

/*!
 * \brief 调试断言, 通用接口
 */
#if (RTE_DEBUG_ENABLED)
#define RTE_ASSERT(exp)       assert(exp)
#else 
#define RTE_ASSERT(exp)
#endif

/*!
 * \brief 计算数组中包含的元素数量, 通用接口
 * \param arr 数组指针
 * \return 数组元素数量
 */
#define RTE_Len(arr) ((NULL_PTR==arr) ? \
                      0U : (sizeof(arr)/sizeof(arr[0])))

/* Enumerations & Unions *******************************************/

/* Structures ******************************************************/

/* Public Interfaces************************************************/

/* Public Function Prototypes***************************************/
extern void    RTE_Fill(uint8* pArray, uint8 value, uint16 len);
extern void    RTE_Copy(uint8* pDest, volatile uint8* pSource, uint16 len);
extern boolean RTE_DataValid(uint8* pArray, uint16 len);
extern boolean RTE_Compare(uint8 *p_buff1, uint8 *p_buff2, uint16 len);

#ifdef __cplusplus
}
#endif /* external "C" */
#endif  /* RTE_GENERAL_H */
