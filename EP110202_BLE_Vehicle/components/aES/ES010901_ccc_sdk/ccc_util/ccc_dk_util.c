#include "../ccc_sdk/ccc_dk_type.h"
#include "ccc_dk_util.h"

#if 0
void core_mm_copy(u8 * dest, const u8 * src, u16 length)
{
    if (dest < src)
	{
        while (length--)
		{
            *dest = *src;
			dest += 1L;
			src += 1L;
		}
	}
    else if (dest > src)
	{
        while (length--)
		{
            dest[length] = src[length];
		}
	}
}

void core_mm_copy_u32(u32 * dest, const u32 * src, u16 length)
{
    if (dest < src)
	{
        while (length--)
		{
            *dest = *src;
			dest += 1L;
			src += 1L;
		}
	}
    else if (dest > src)
	{
        while (length--)
		{
            dest[length] = src[length];
		}
	}
}

void core_mm_set(u8 * dest, u8 val, u16 length)
{
    while (length--)
        dest[length] = val;
}

void core_mm_set_u32(u32 *dest, u32 val, u16 length)
{
	while (length--)
        dest[length] = val;
}


s8 core_mm_compare_unsigned(const u8 * buf1, const u8 * buf2, u16 length)
{
	u16 i;

	if (buf1 == buf2)
	{
		return 0;
	}

    for (i=0; i<length; i++)
	{
        if (buf1[i] > buf2[i])
		{
            return 1;
		}
		else if (buf1[i] < buf2[i])
		{
			return -1;
		}
	}

    return 0;
}

//对原数组左移shitBits，返回溢出值
u8 core_mm_left_shift(u8 * buf, u16 len, u8 shiftBits)
{
	u8 overFlow = 0;
	u16 i = 0;
	
	if(shiftBits > 8 || shiftBits == 0)
	{
		return 0;
	}
	overFlow = (buf[0] >> (8 - shiftBits));

	for(i = 0; i < (len - 1); i++)
	{
		buf[i] = ((buf[i] << shiftBits) + ((buf[i + 1]) >> (8 - shiftBits)));
	}

	buf[i] = (buf[i] << shiftBits);

	return overFlow;
}

s8 core_mm_compare(const s8 * buf1, const s8 * buf2, u16 length)
{
	u16 i;

	if (buf1 == buf2)
	{
		return 0;
	}

    for (i=0; i<length; i++)
	{
        if (buf1[i] > buf2[i])
		{
            return 1;
		}
		else if (buf1[i] < buf2[i])
		{
			return -1;
		}
	}

    return 0;
}

u8 core_mm_compare_u32(const u32 * buf1, const u32 * buf2, u16 length)
{
	u8 tmp8;
	u32 tmp32;
	tmp32 = (u32)buf1;
	tmp8 = (u8)(tmp32>>24); 
	if((tmp8!=0x20) && (tmp8 != 0x0C))	
	{
		return 0;
	}
	while (length--)
	{
		if (*buf1++ != *buf2++)
		{
			return 1;
		}
	}

    return 0;	
}

u8 core_mm_compare_with_byte(const u8 *src, u8 val, u16 length)
{
	while (length--)
		if (src[length] != val)
			return 1;

	return 0;
}

u8 core_mm_compare_with_u32(const u32 *src, u32 val, u16 length)
{
	while (length--)
		if (src[length] != val)
			return 1;

	return 0;	
}

void core_hw_delay(u32 count)
{
	u32 i;
	
	for (i=0; i<count; i++);
}

void core_mm_set_mass(u8 * dest, u8 val, u32 length)
{
#if DISABLE_FAST_UTIL
	core_mm_set(dest, val, length);
#else
	u16 val16 = (val<<8)|val;
	u32 val32 = (val16<<16)|val16;

	if (length < 0x20)
	{
		core_mm_set(dest, val, length);
		return;
	}
	switch(((u32)dest)%4)
	{
	case 1:
		dest[0] = val;
		length--;
		dest++;
	case 2:
		dest[0] = val;
		length--;
		dest++;
	case 3:
		dest[0] = val;
		length--;
		dest++;
	}
	while(length >= 16)
	{
		((u32*)dest)[0] = val32;
		((u32*)dest)[1] = val32;
		((u32*)dest)[2] = val32;
		((u32*)dest)[3] = val32;
		dest += 16;
		length -= 16;
	}
	while(length >= 4)
	{
		((u32*)dest)[0] = val32;
		dest += 4;
		length -= 4;
	}
	switch(length)
	{
	case 3:
		dest[0] = val;
		length--;
		dest++;
	case 2:
		dest[0] = val;
		length--;
		dest++;
	case 1:
		dest[0] = val;
		length--;
		dest++;
	}
#endif
}
void core_hw_ram_copy(u8 *pbDest, u8 *pbSrc, u16 wLen)
{
	u16 i;
	if ((pbSrc + wLen) > pbDest && (pbDest > pbSrc))
	{
		if (((u32)pbDest & 0x03) || ((u32)pbSrc & 0x03) || (wLen &0x03))
		{
			for (i = wLen; i > 0; i--)
			{
				*(pbDest+i-1) = *(pbSrc+i-1);
			}
		}
		else
		{
			for (i = wLen/4; i > 0; i--)
			{
				*((u32 *)(pbDest) + i-1) = *((u32 *)(pbSrc) + i - 1);
			}
		}
	}
	else 
	{
		if (((u32)pbDest & 0x03) || ((u32)pbSrc & 0x03) || (wLen &0x03))
		{
			for (i = 0; i < wLen; i++)
			{
				*(pbDest+i) = *(pbSrc+i);
			}
		}
		else
		{
			for (i = 0; i < wLen/4; i++)
			{
				*((u32 *)(pbDest)+i) = *((u32 *)(pbSrc)+i);
			}
		}
	}
}
#endif
//------------------util tlv-----------------
boolean ccc_tlv_util_is_ber_tlv(u8 tag)
{
    if((tag & 0x20) != 0x00)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

u8 ccc_tlv_util_get_tag_width(u8* tlv)
{
    if((*tlv & 0x1f) == 0x1F)
    {
        return 0x02;
    }

    return 0x01;
}

u8 ccc_tlv_util_get_length_width(u8* tlv)
{
    u8 tagWidth = ccc_tlv_util_get_tag_width(tlv);
    u16 len = *(tlv + tagWidth);

    if(len > 0 && len <= 0x7F)
    {
        return 0x01;
    }
    else if(len == 0x81)
    {
        return 0x02; //81 xx
    }
    else
    {
        return 0x03;  //82 xxxx
    }
}

u16 ccc_tlv_util_get_value_width(u8* tlv)
{
    u8 tagWidth = ccc_tlv_util_get_tag_width(tlv);
    u16 len = *(tlv + tagWidth);

    if(len > 0 && len <= 0x7F)
    {
        return len;
    }
    else if(len == 0x81)
    {
        return *(tlv + len + 1); //81 xx
    }
    else
    {
        return core_dcm_mku16(*(tlv + len + 1), *(tlv + len + 2));  //82 xxxx
    }
}

u8 ccc_tlv_util_get_tl_width(u8* tlv)
{
    return (ccc_tlv_util_get_tag_width(tlv) + ccc_tlv_util_get_length_width(tlv));
}

u16 ccc_tlv_util_get_tlv_width(u8* tlv)
{
    return (ccc_tlv_util_get_value_width(tlv) + ccc_tlv_util_get_tl_width(tlv));
}

u16 ccc_tlv_util_get_tag(u8* tlv)
{
    if((*tlv & 0x1F) == 0x1F)
    {
        return core_dcm_mku16(*tlv, *(tlv + 1));
    }
    else
    {
        return *tlv;
    }
}

/**
 * @brief 对源数据padding 80000000;
 * 
 * @param src: 要padding的源数据
 * @param srcLen : 要padding的源数据长度
 * @param needPadOnMultiple : 是multiple的倍数的时候是否还要padding 8000...00到倍数
 * @param multiple: 倍数值  
 * @return u16 : padding后的数据长度
 */
u16 ccc_add_padding_8000(u8* src, u16 srcLen, boolean needPadOnMultiple, u8 multiple)
{
	u8 left = srcLen % multiple;
	u8 padLen = 0;
	u8 needPadLen = 0;

	if(needPadOnMultiple)
	{
		if(left == 0)  //已是倍数长度
		{
			needPadLen = multiple;
		}
		else
		{
			needPadLen = (multiple - left);
		}
	}
	else //倍数长度时不需要padding;
	{
		if(left == 0)  //已是倍数长度
		{
			needPadLen = 0;
		}
		else
		{
			needPadLen = (multiple - left);
		}
	}

	if(needPadLen > 0)
	{
		src[padLen++] = 0x80;
		needPadLen --;

		while(needPadLen > 0)
		{
			src[padLen++] = 0x00;
			needPadLen --;
		}
	}
			
	return (srcLen + padLen);
}

/**
 * @brief Get the bit1 index, 从最低bit开始查找，找到后就返回；有多个bit为1的情况下，也只是返回最低bit为1的index;
 * 
 * @param v 
 * @return u8 : 最低bit为1所在的bit index, 0-7; 0xFF无 bit位为1；
 */
u8 get_bit1_index(u16 v)
{
	u8 index = 0;

	for(index = 0; index < 16; index++)
	{
		if(((1 << index) & v) != 0x00)
		{
			break;
		}
	}

	if(index == 16)
	{
		index = 0xFF;
	}

	return index;
}

