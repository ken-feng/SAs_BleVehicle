/*@Encoding: utf-8*/
/**
**************************************************************************************
* @copyright: 
* @file     : Compiler.h
* @author   : J.L.
* @version  : 
* @date     : 
**************************************************************************************
*                                 AUTOSAR Information
* ------------------------------------------------------------------------------------
* Document File            | BSWGeneral\\AUTOSAR_SWS_CompilerAbstraction.pdf
* Document Title           | Specification of Compiler Abstraction
* Document ID NO           | 051
* Document Status          | Final
* Part of AUTOSAR Standard | Classic Platform
* Part of Standard Release | 4.4.0
**************************************************************************************
* @note     :
**************************************************************************************
*/
#ifndef COMPILER_H
#define COMPILER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ------------------------------------------------------------------------------------
 *                                 Macro Definitions
 * ------------------------------------------------------------------------------------
 */
/**
 * @brief: 8.2.1.1 Memory class AUTOMATIC 
 * @ref  : [SWS_COMPILER_00046]/(SRS_BSW_00361)
 * @range: “empty”
 * @note : The memory class AUTOMATIC shall be provided as empty 
 *         definition, used for the declaration of local pointers.
 */
#define AUTOMATIC

/**
 * @brief: 8.2.1.2 Memory class TYPEDEF 
 * @ref  : [SWS_COMPILER_00059]/(SRS_BSW_00404)
 * @range: “empty”
 * @note : The memory class TYPEDEF shall be provided as empty definition. 
 *         This memory class shall be used within type definitions, 
 *         where no memory qualifier can be specified. This can be necessary
 *         for defining pointer types, with e.g. P2VAR, where the macros 
 *         require two parameters. First parameter can be specified in the 
 *         type definition (distance to the memory location referenced by the pointer),
 *         but the second one (memory allocation of the pointer itself) cannot be 
 *         defined at this time. Hence, memory class TYPEDEF shall be applied.
 */
#define TYPEDEF

/**
 * @brief: 8.2.1.3 NULL_PTR
 * @ref  : [SWS_COMPILER_00051]/(SRS_BSW_00480)
 * @range: void pointer
 * @note : The compiler abstraction shall provide the NULL_PTR
 *         define with a void pointer to zero definition.
 */
#define NULL_PTR    ((void *)0)

/**
 * @brief: 8.2.1.4 INLINE
 * @ref  : [SWS_COMPILER_00057]/(SRS_BSW_00361)
 * @range: inline/”empty”
 * @note : The compiler abstraction shall provide the INLINE 
 *         define for abstraction of the keyword inline.
 */
#define INLINE

/**
 * @brief: 8.2.1.5 LOCAL_INLINE
 * @ref  : [SWS_COMPILER_00060]/(SRS_BSW_00361)
 * @range: inline/”empty”
 * @note : The compiler abstraction shall provide the LOCAL_INLINE
 *         define for abstraction of the keyword inline in functions 
 *         with “static” scope.
 *         Different compilers may require a different sequence of the 
 *         keywords “static” and “inline” if this is supported at all.
 */
#define LOCAL_INLINE

/**
 * ------------------------------------------------------------------------------------
 *                              Function definitions
 * ------------------------------------------------------------------------------------
 */
/**
 * @brief : 8.2.2.1 FUNC identification information
 * @ref   : [SWS_COMPILER_00001]/(SRS_BSW_00003)
 * @note  : The compiler abstraction shall define the FUNC macro for the
 *          declaration and definition of functions that ensures correct 
 *          syntax of function declarations as required by a specific compiler.
 * @param : rettype-return type of the function
 * @param : memclass-classification of the function itself
 * @retval: None
 */
#define FUNC(rettype, memclass)

/**
 * @brief : 8.2.2.2 FUNC_P2CONST
 * @ref   : [SWS_COMPILER_00061]/(SRS_BSW_00361)
 * @note  : The compiler abstraction shall define the FUNC_P2CONST macro 
 *          for the declaration and definition of functions returning a 
 *          pointer to a constant. This shall ensure the correct syntax of 
 *          function declarations as required by a specific compiler.
 * @param : rettype-return type of the function
 * @param : ptrclass-defines the classification of the pointer’s distance
 * @param : memclass-classification of the function itself
 * @retval: None
 */
#define FUNC_P2CONST(rettype, ptrclass, memclass)

/**
 * @brief : 8.2.2.3 FUNC_P2VAR
 * @ref   : [SWS_COMPILER_00063]/(SRS_BSW_00361)
 * @note  : The compiler abstraction shall define the FUNC_P2VAR macro for
 *          the declaration and definition of functions returning a pointer
 *          to a variable. This shall ensure the correct syntax of function 
 *          declarations as required by a specific compiler.
 * @param : rettype-return type of the function
 * @param : ptrclass-defines the classification of the pointer’s distance
 * @param : memclass-classification of the function itself
 * @retval: None
 */
#define FUNC_P2VAR(rettype, ptrclass, memclass)

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif /* COMPILER_H */

/*************************** End of File ****************************/