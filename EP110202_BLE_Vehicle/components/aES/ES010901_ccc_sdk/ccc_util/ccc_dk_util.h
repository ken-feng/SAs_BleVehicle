#ifndef __CCC_UTIL_H__
#define __CCC_UTIL_H__

#include "../ccc_sdk/ccc_dk_type.h"
#include "../../../aEM/EM00010101_platform/EM000101.h"

#if 0
#define MBit(BitMask)             (1 << BitMask)
#define BitSet(Value, BitNum)     (Value |= BitNum)
#define BitClr(Value, BitNum)     (Value &= (BitNum ^ 0xFF))
#define IsBitSet(Value, BitNum)   (Value & BitNum)
#define IsBitClr(Value, BitNum)   (!(Value & BitNum))

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

#define core_dcm_lsh1(bitcount)             ((u8)(1<<(bitcount)))

#define /* void */core_bmp_setbit(/* u8* */table, /* u16 */index) do {table[((u16)(index))/8] |=  core_dcm_lsh1(((u8)(index))&0x07);}while(0)
#define /* void */core_bmp_clrbit(/* u8* */table, /* u16 */index) do {table[((u16)(index))/8] &= ~core_dcm_lsh1(((u8)(index))&0x07);}while(0)
#define /* u8   */core_bmp_getbit(/* u8* */table, /* u16 */index) (  (table[((u16)(index))/8] &   core_dcm_lsh1(((u8)(index))&0x07) ) != 0)

extern void core_mm_copy(u8 * dest, const u8 * src, u16 length);
extern void core_mm_copy_u32(u32 * dest, const u32 * src, u16 length);
extern void core_mm_set(u8 * dest, u8 val, u16 length);
extern void core_mm_set_u32(u32 *dest, u32 val, u16 length);
extern s8 core_mm_compare_unsigned(const u8 * buf1, const u8 * buf2, u16 length);
//extern u8 core_mm_left_shift(u8 * buf, u16 len, u8 shiftBits);
extern s8 core_mm_compare(const s8 * buf1, const s8 * buf2, u16 length);
extern u8 core_mm_compare_u32(const u32 * buf1, const u32 * buf2, u16 length);
extern u8 core_mm_compare_with_byte(const u8 *src, u8 val, u16 length);
extern u8 core_mm_compare_with_u32(const u32 *src, u32 val, u16 length);
//extern void core_hw_delay(u32 count);
extern void core_mm_set_mass(u8 * dest, u8 val, u32 length);
extern void core_hw_ram_copy(u8 *pbDest, u8 *pbSrc, u16 wLen);
#endif

boolean ccc_tlv_util_is_ber_tlv(u8 tag);
u8 ccc_tlv_util_get_tag_width(u8* tlv);
u8 ccc_tlv_util_get_length_width(u8* tlv);
u16 ccc_tlv_util_get_value_width(u8* tlv);
u8 ccc_tlv_util_get_tl_width(u8* tlv);
u16 ccc_tlv_util_get_tlv_width(u8* tlv);
u16 ccc_tlv_util_get_tag(u8* tlv);
u16 ccc_add_padding_8000(u8* src, u16 srcLen, boolean needPadOnMultiple, u8 multiple);
u8 get_bit1_index(u16 v);
#endif
