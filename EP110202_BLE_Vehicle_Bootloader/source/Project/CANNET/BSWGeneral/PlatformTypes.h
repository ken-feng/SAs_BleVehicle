/*@Encoding: utf-8*/
/**
**************************************************************************************
* @copyright: 
* @file     : PlatformTypes.h
* @author   :
* @version  : 
* @date     : 
**************************************************************************************
*                                 AUTOSAR Information
* ------------------------------------------------------------------------------------
* Document File            | BSWGeneral\\AUTOSAR_SWS_PlatformTypes.pdf
* Document Title           | Specification of Platform Types
* Document ID NO           | 048
* Document Status          | Final
* Part of AUTOSAR Standard | Classic Platform
* Part of Standard Release | 4.4.0
**************************************************************************************
* @note     :
**************************************************************************************
*/
#ifndef PLATFORM_TYPES_H
#define PLATFORM_TYPES_H
//#include "EM000401.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * ------------------------------------------------------------------------------------
 *                                Type Definitions - General
 * ------------------------------------------------------------------------------------
 */
#define CPU_TYPE_8           8  /* 8-bit CPU */
#define CPU_TYPE_16          16 /* 16-bit CPU */
#define CPU_TYPE_32          32 /* 32-bit CPU */
#define CPU_TYPE_64          64 /* 64-bit CPU */
#define MSB_FIRST            0  /* Most significant bit first order */
#define LSB_FIRST            1  /* Least significant bit first order */
#define HIGH_BYTE_FIRST      0  /* High byte first order */
#define LOW_BYTE_FIRST       1  /* Low byte first order */

/**
 * ------------------------------------------------------------------------------------
 *                                 Type Definitions - CPU
 * ------------------------------------------------------------------------------------
 */
#define CPU_TYPE             CPU_TYPE_32    /* CPU type */
#define CPU_BIT_ORDER        LSB_FIRST      /* CPU bit order */
#define CPU_BYTE_ORDER       LOW_BYTE_FIRST /* CPU byte order */
#define CPU_FPU_AVAILABLE    0              /* CPU has FPU? */

/**
 * ------------------------------------------------------------------------------------
 *                                Type Definitions - Data
 * ------------------------------------------------------------------------------------
 */
typedef unsigned char        boolean; /* Unsigned 8-bit integer */

typedef signed   char        sint8;   /* Signed 8-bit integer */
typedef unsigned char        uint8;   /* Unsigned 8-bit integer */
typedef signed   short       sint16;  /* Signed 16-bit integer */
typedef unsigned short       uint16;  /* Unsigned 16-bit integer */
typedef signed   long        sint32;  /* Signed 32-bit integer */
typedef unsigned long        uint32;  /* Unsigned 32-bit integer */

#define STDINT_INCLUDE 1
#if (!STDINT_INCLUDE)
typedef signed   char        int8_t;
typedef unsigned char        uint8_t;
typedef signed   short       int16_t;
typedef unsigned short       uint16_t;
typedef signed   long        int32_t;
typedef unsigned long        uint32_t;
#endif

typedef signed   char        s8;
typedef unsigned char        u8;
typedef signed   short       s16;
typedef unsigned short       u16;
//typedef signed   long        s32; 
//typedef unsigned long        u32;
typedef unsigned int		u32;
typedef signed int			s32;

typedef signed   short       sint8_least;
typedef unsigned short       uint8_least;
typedef signed   short       sint16_least;
typedef unsigned short       uint16_least;
typedef unsigned long        uint32_least;
typedef signed   long        sint32_least;

#if ((CPU_TYPE_32 == CPU_TYPE) || (CPU_TYPE_64 == CPU_TYPE))
typedef signed   long long   sint64;  /* Signed 64-bit integer */
typedef unsigned long long   uint64;  /* Unsigned 64-bit integer */

typedef signed   long long   int64_t;
typedef unsigned long long   uint64_t;

typedef signed   long long   s64;
typedef unsigned long long   u64;

typedef signed   long long   sint64_least;
typedef unsigned long long   uint64_least;
#endif

#if (CPU_FPU_AVAILABLE)
typedef float                float32; /* 32-bit float */
typedef double               float64; /* 64-bit float */

typedef float                float32_t;
typedef double               float64_t;

typedef float                f32;
typedef double               f64;

typedef float                float32_least;
typedef double               float64_least;
#endif

/**
 * ------------------------------------------------------------------------------------
 *                              Type Definitions - Boolean
 * ------------------------------------------------------------------------------------
 */
#ifndef TRUE 
#define TRUE                 ((boolean)0x1u)
#endif 
#ifndef FALSE 
#define FALSE                ((boolean)0x0u)
#endif

#define core_dcm_u8_lo(b)                   ((u8)((u8)(b)&0x0f))
#define core_dcm_u8_hi(b)                   ((u8)((u8)(b)>>4))

#define core_dcm_u16_lo(w)                   ((u8)((u16)(w)&0x00ff))
#define core_dcm_u16_hi(w)                   ((u8)((u16)(w)>>8))

#define core_dcm_u32_lo(dw)                  ((u16)((u32)(dw)&0xffff))
#define core_dcm_u32_hi(dw)                  ((u16)((u32)(dw)>>16))

#define core_dcm_mku16(h, l)                ((u16)(((u16)(h)<<8) | (l)))
#define core_dcm_mku32(hh, hl, lh, ll)      ((u32)((((u32)hh)<<24) | ((u32)(hl)<<16) | ((u16)(lh)<<8) | (ll)))
#define core_dcm_readU8(src)                ((u8)(*((u8 *)(src))))
#define core_dcm_readBig16(src)             core_dcm_mku16((((u8 *)(src))[0]), (((u8 *)(src))[1]))
#define core_dcm_readBig32(src)             core_dcm_mku32((((u8 *)(src))[0]), (((u8 *)(src))[1]), (((u8 *)(src))[2]), (((u8 *)(src))[3]))
#define core_dcm_readBig24(src)             core_dcm_mku32(                 0, (((u8 *)(src))[0]), (((u8 *)(src))[1]), (((u8 *)(src))[2]))

#define core_dcm_writeU8(dest, value)       ((((u8 *)(dest))[0]) = (value))

#define core_dcm_writeBig16(dest, value)    do {(((u8 *)(dest))[0]) = (u8)((value)>>8); (((u8 *)(dest))[1]) = (u8)(value);} while(0);

#define core_dcm_writeBig32(dest, value)    do { \
                                                (((u8 *)(dest))[0]) = (u8)((value)>>24); \
                                                (((u8 *)(dest))[1]) = (u8)((value)>>16); \
                                                (((u8 *)(dest))[2]) = (u8)((value)>>8);  \
                                                (((u8 *)(dest))[3]) = (u8)(value);       \
                                            } while(0);

#define core_dcm_writeBig24(dest, /* u32 */value)   do { \
                                                (((u8 *)(dest))[0]) = (u8)((value)>>16); \
                                                (((u8 *)(dest))[1]) = (u8)((value)>>8);  \
                                                (((u8 *)(dest))[2]) = (u8)(value);       \
                                            } while(0);


#ifdef __cplusplus
}
#endif /* extern "C" */

#endif /* PLATFORM_TYPES_H */

/*************************** End of File ****************************/
