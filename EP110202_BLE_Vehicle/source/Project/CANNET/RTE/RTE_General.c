/* @Encoding: UTF-8 */
/*!
 ********************************************************************
 * @file   : RTE_General.h
 * @brief  : 
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
#include <RTE_General.h>

/* Macros & Typedef*************************************************/

/*Imported Interfaces***********************************************/

/*Enumerations & Structures*****************************************/

/*Variable Definitions**********************************************/

/* Public Function Prototypes***************************************/
/*!
 * \brief  RTE_Fill, 向数组填充给定值, 通用接口
 * \param  pArray 需要填充的数组
 * \param  value 需要填充的值
 * \param  len 需要填充的长度
 * \return None
 */
void RTE_Fill(uint8* pArray, uint8 value, uint16 len)
{
    RTE_ASSERT(NULL_PTR == pArray);
    RTE_ASSERT(0 == len);

	if ((NULL_PTR == pArray) || (0 == len)) {
		return;
	}

    while (len--) {
        *(pArray++) = value;
    }
}

/*!
 * \brief  RTE_Copy, 拷贝数组, 通用接口
 * \param  pDest 目标数组
 * \param  pSource 原始数组
 * \param  len 需要填充的长度
 * \return None
 */
void RTE_Copy(uint8* pDest, volatile uint8* pSource, uint16 len)
{
    RTE_ASSERT(NULL_PTR == pDest);
    RTE_ASSERT(NULL_PTR == pSource);
    RTE_ASSERT(0 == len);

	if ((NULL_PTR == pDest) 
     || (NULL_PTR == pSource) 
     || (0 == len)) {
		return;
	}

    while (len--) {
        *(pDest++) = *(pSource++);
    }
}

/*!
 * \brief  RTE_DataValid 检查数组数据是否有效, 适用接口
 * \param  pArray 需要检查的数组
 * \param  len 需要填充的长度
 * \return FALSE-无效; TRUE-有效
 */
boolean RTE_DataValid(uint8* pArray, uint16 len)
{
    RTE_ASSERT(NULL_PTR == pArray);
    RTE_ASSERT(0 == len);

	if ((NULL_PTR == pArray) || (0 == len)) {
		return FALSE;
	}

    while (len--) {
        if ((0x00 == pArray[len]) 
         || (0xFF == pArray[len])) {
            return FALSE;
        }
    }

    return TRUE;
}

/*!
 * \brief  RTE_Compare 比较两个数组的值是否一致, 通用接口
 * \param  pArray1 需要对比的数组1
 * \param  pArray2 需要对比的数组2
 * \param  len 需要填充的长度
 * \return FALSE-不同; TRUE-相同
 */
boolean RTE_Compare(uint8 *pArray1, uint8 *pArray2, uint16 len)
{
    RTE_ASSERT(NULL_PTR == pArray1);
    RTE_ASSERT(NULL_PTR == pArray2);
    RTE_ASSERT(0 == len);

	if ((NULL_PTR == pArray1) 
     || (NULL_PTR == pArray2) 
     || (0 == len)) {
		return FALSE;
	}

    while (len--) {
        if (*(pArray1++) != *(pArray2++)) {
            return FALSE;
        }
    }

    return TRUE;
}
