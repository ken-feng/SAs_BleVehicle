/*@Encoding: utf-8*/
/**
**************************************************************************************
* @copyright: 
* @file     : StandardTypes.h
* @author   : J.L.
* @version  : 
* @date     : 
**************************************************************************************
*                                 AUTOSAR Information
* ------------------------------------------------------------------------------------
* Document File            | BSWGeneral\\AUTOSAR_SWS_StandardTypes.pdf
* Document Title           | Specification of Standard Types
* Document ID NO           | 049
* Document Status          | Final
* Part of AUTOSAR Standard | Classic Platform
* Part of Standard Release | 4.4.0
**************************************************************************************
* @note     :
**************************************************************************************
*/
#ifndef STD_TYPES_H
#define STD_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "PlatformTypes.h"
#include "Compiler.h"

/**
 * ------------------------------------------------------------------------------------
 *                                  Type Definitions
 * ------------------------------------------------------------------------------------
 */
/**
 * @brief: 8.1.1 Std_ReturnType 
 * @ref  : [SWS_Std_00005]/(SRS_BSW_00357)
 * @note : This type can be used as standard API return type
 *         which is shared between the RTE and the BSW modules. 
 *         It shall be defined as follows:
 */
typedef uint8 Std_ReturnType;

/**
 * @brief: 8.1.2 Std_VersionInfoType 
 * @ref  : [SWS_Std_00015]/(SRS_BSW_00004)
 * @note : This type shall be used to request the version of a 
 *         BSW module using the <Module name>_GetVersionInfo() function.
 */
typedef struct {
    uint16 vendorID;
    uint16 moduleID;
    uint8  sw_major_version;
    uint8  sw_minor_version;
    uint8  sw_patch_version;
} Std_VersionInfoType;

/**
 * @brief : Bit field type definition
 * @author: J.L.
 */
#ifndef bits_t
typedef unsigned bits_t;
#endif

/**
 * ------------------------------------------------------------------------------------
 *                                  Symbol Definitions
 * ------------------------------------------------------------------------------------
 */
/**
 * @brief: 8.2.1 E_OK, E_NOT_OK
 * @ref  : [SWS_Std_00006]/(SRS_BSW_00357)
 * @note : Because E_OK is already defined within OSEK, the symbol E_OK has to be shared. 
 *         To avoid name clashes and redefinition problems, the symbols have to be 
 *         defined in the following way (approved within implementation):
 */
#ifndef STATUSTYPEDEFINED 
#define STATUSTYPEDEFINED 
#define E_OK       0x00u 
typedef unsigned char StatusType; /* OSEK compliance */ 
#endif 
#define E_NOT_OK   0x01u

/**
 * @brief: 8.2.2 STD_HIGH, STD_LOW
 * @ref  : [SWS_Std_00007]/(SRS_BSW_00348)
 * @note : The symbols STD_HIGH and STD_LOW shall be defined as follows:
 */
#define STD_HIGH   0x01u /* Physical state 5V or 3.3V */ 
#define STD_LOW    0x00u /* Physical state 0V */

/**
 * @brief: 8.2.3 STD_ACTIVE, STD_IDLE
 * @ref  : [SWS_Std_00013]/(SRS_BSW_00348)
 * @note : The symbols STD_ACTIVE and STD_IDLE shall be defined as follows:
 */
#define STD_ACTIVE 0x01u /* Logical state active */ 
#define STD_IDLE   0x00u /* Logical state idle */

/**
 * @brief: 8.2.4 STD_ON, STD_OFF
 * @ref  : [SWS_Std_00010]/(SRS_BSW_00348)
 * @note : The symbols STD_ON and STD_OFF shall be defined as follows:
 */
#define STD_ON     0x01u 
#define STD_OFF    0x00u

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif /* STD_TYPES_H */

/*************************** End of File ****************************/
