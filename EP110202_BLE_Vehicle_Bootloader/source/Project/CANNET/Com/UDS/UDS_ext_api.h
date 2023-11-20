/* @Encoding: UTF-8 */
/*!
 ********************************************************************
 * @file   : UDS_App.h
 * @brief  : Configurations for the UDS_App module.
 * @author : 
 * @version: A.0.1
 * @release: @currentDateTime
 ********************************************************************
 *                      Description/Information
 * 1.
 * 2.
 ********************************************************************
 */
#ifndef UDS_EXT_API_H
#define UDS_EXT_API_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes*********************************************************/
#include <ComStackTypes.h>

/* Macros & Typedef*************************************************/
extern void UDS_Jump_To_App(void);

extern u8 UDS_get_appmode_valid(void);

extern u8 UDS_set_appmode_valid(u8 flag);

extern u8 UDS_get_appFlag_valid(void);

extern u8 UDS_set_appFlag_valid(u8 flag);

extern u8 UDS_get_appSwitchReturn_Flag(void);

extern u8 UDS_set_appSwitchReturn_Flag(u8 flag);

extern void UDS_ext_reset_system(void);

extern u8 UDS_ext_erase_is_inFlash(u32 startAddress);

extern u8 UDS_ext_erase_area(u32 startAddress,u32 size);

extern u8 UDS_ext_write_data(u32 startAddress,u32 size, u8* buffer);

extern u32 UDS_ext_calc_crc32(u8* buf,u32 size);

extern void UDS_ext_send_data(u8* buf, u16 length);
#ifdef __cplusplus
}
#endif /* external "C" */
#endif /* UDS_EXT_API_H */
