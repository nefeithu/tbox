/*!The Treasure Box Library
 * 
 * TBox is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 * 
 * TBox is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with TBox; 
 * If not, see <a href="http://www.gnu.org/licenses/"> http://www.gnu.org/licenses/</a>
 * 
 * Copyright (C) 2009 - 2011, ruki All rights reserved.
 *
 * \author		ruki
 * \file		bstream.c
 *
 */
/* /////////////////////////////////////////////////////////
 * includes
 */
#include "bstream.h"
#include "gstream.h"
#include "../libc/libc.h"
#include "../math/math.h"
#include "../memory/memory.h"
#include "../string/string.h"
#include "../platform/platform.h"

/* /////////////////////////////////////////////////////////
 * macros
 */

/* /////////////////////////////////////////////////////////
 * attach 
 */
tb_bstream_t* tb_bstream_attach(tb_bstream_t* bst, tb_byte_t* data, tb_size_t size)
{
	tb_assert(bst && data && size);

	bst->p 	= data;
	bst->b 	= 0;
	bst->n 	= size;
	bst->e 	= data + size;

	return bst;
}

/* /////////////////////////////////////////////////////////
 * load & save 
 */
tb_size_t tb_bstream_load(tb_bstream_t* bst, tb_gstream_t* ist)
{
	tb_assert_and_check_return_val(bst && ist, 0);

	// sync it first
	tb_bstream_sync(bst);

	// load
	tb_byte_t 		data[TB_GSTREAM_BLOCK_MAXN];
	tb_size_t 		load = 0;
	tb_int64_t 		time = tb_mclock();
	tb_uint64_t 	left = tb_gstream_left(ist);

	while (1)
	{
		tb_long_t n = tb_gstream_aread(ist, data, TB_GSTREAM_BLOCK_MAXN);
		if (n > 0)
		{
			// update load
			load += n;

			// set data
			if (tb_bstream_set_data(bst, data, n) != n) break;

			// update clock
			time = tb_mclock();
		}
		else if (!n) 
		{
			// timeout?
			if (tb_mclock() - time > tb_gstream_timeout(ist)) break;
			
			// sleep some time
			tb_usleep(ist);
		}
		else break;

		// is end?
		if (left && load >= left) break;
	}

	return load;
}
tb_size_t tb_bstream_save(tb_bstream_t* bst, tb_gstream_t* ost)
{
	tb_assert_and_check_return_val(bst && ost, 0);

	// sync it first
	tb_bstream_sync(bst);

	// load
	tb_byte_t 		data[TB_GSTREAM_BLOCK_MAXN];
	tb_size_t 		save = 0;
	tb_int64_t 		time = tb_mclock();
	while(1)
	{
		// get data
		tb_long_t size = tb_bstream_get_data(bst, data, TB_GSTREAM_BLOCK_MAXN);
		//tb_trace("ret: %d", ret);

		// is end?
		if (size)
		{
			// writ it
			if (!tb_gstream_bwrit(ost, data, size)) break;

			// update size
			save += size;
		}
		else break;
	}

	return save;
}

/* /////////////////////////////////////////////////////////
 * modifiors
 */
tb_void_t tb_bstream_goto(tb_bstream_t* bst, tb_byte_t* data)
{
	tb_assert(bst && data && data <= bst->e);
	bst->b = 0;
	if (data <= bst->e) bst->p = data;
}
tb_void_t tb_bstream_sync(tb_bstream_t* bst)
{
	if (bst->b) 
	{
		bst->p++;
		bst->b = 0;
	}
}
/* /////////////////////////////////////////////////////////
 * position
 */
tb_byte_t const* tb_bstream_beg(tb_bstream_t* bst)
{
	tb_assert(bst && bst->e);
	return (bst->e - bst->n);
}
tb_byte_t const* tb_bstream_pos(tb_bstream_t* bst)
{
	tb_assert(bst && bst->p <= bst->e);
	tb_bstream_sync(bst);
	return bst->p;
}

tb_byte_t const* tb_bstream_end(tb_bstream_t* bst)
{
	tb_assert(bst && bst->e);
	return bst->e;
}

/* /////////////////////////////////////////////////////////
 * attributes
 */
tb_size_t tb_bstream_size(tb_bstream_t* bst)
{
	tb_assert(bst);
	return bst->n;
}
tb_size_t tb_bstream_offset(tb_bstream_t* bst)
{
	tb_assert(bst);
	return (((bst->p + bst->n) > bst->e)? (bst->p + bst->n - bst->e) : 0);
}
tb_size_t tb_bstream_left(tb_bstream_t* bst)
{
	tb_assert(bst && bst->p <= bst->e);
	tb_bstream_sync(bst);
	return (bst->e - bst->p);
}
tb_size_t tb_bstream_left_bits(tb_bstream_t* bst)
{
	tb_assert(bst);
	return ((bst->p < bst->e)? (((bst->e - bst->p) << 3) - bst->b) : 0);
}
tb_bool_t tb_bstream_valid(tb_bstream_t* bst)
{
	if (!bst || !bst->p || !bst->e || !bst->n || bst->p > bst->e) return TB_FALSE;
	return TB_TRUE;
}
/* /////////////////////////////////////////////////////////
 * skip
 */
tb_void_t tb_bstream_skip(tb_bstream_t* bst, tb_size_t size)
{
	tb_assert(bst && bst->p <= bst->e);
	tb_bstream_sync(bst);
	bst->p += size;
}
tb_void_t tb_bstream_skip_bits(tb_bstream_t* bst, tb_size_t nbits)
{
	tb_assert(bst && bst->p <= bst->e);
	bst->p += (bst->b + nbits) >> 3;
	bst->b = (bst->b + nbits) & 0x07;
}
tb_char_t const* tb_bstream_skip_string(tb_bstream_t* bst)
{
	return tb_bstream_get_string(bst);
}

/* /////////////////////////////////////////////////////////
 * get
 */
tb_uint8_t tb_bstream_get_u1(tb_bstream_t* bst)
{
	tb_uint8_t val = ((*bst->p) >> (7 - bst->b)) & 1;
	bst->b++;
	if (bst->b >= 8) 
	{
		bst->p++;
		bst->b = 0;
	}
	return val;
}
tb_uint8_t tb_bstream_get_u8(tb_bstream_t* bst)
{
	tb_assert(!bst->b);
	return *(bst->p++);
}
tb_sint8_t tb_bstream_get_s8(tb_bstream_t* bst)
{
	tb_assert(!bst->b);
	return *(bst->p++);
}
tb_uint16_t tb_bstream_get_u16_be(tb_bstream_t* bst)
{
	tb_assert(!bst->b);
	
	tb_uint16_t val = tb_bits_get_u16_be(bst->p);
	bst->p += 2;
	return val;
	
}
tb_sint16_t tb_bstream_get_s16_be(tb_bstream_t* bst)
{
	tb_assert(!bst->b);
	
	tb_sint16_t val = tb_bits_get_s16_be(bst->p);
	bst->p += 2;
	return val;
	
}
tb_uint16_t tb_bstream_get_u16_le(tb_bstream_t* bst)
{
	tb_assert(!bst->b);
	
	tb_uint16_t val = tb_bits_get_u16_le(bst->p);
	bst->p += 2;
	return val;
	
}
tb_sint16_t tb_bstream_get_s16_le(tb_bstream_t* bst)
{
	tb_assert(!bst->b);
	
	tb_sint16_t val = tb_bits_get_s16_le(bst->p);
	bst->p += 2;
	return val;
	
}
tb_uint32_t tb_bstream_get_u24_be(tb_bstream_t* bst)
{
	tb_assert(!bst->b);
	
	tb_uint32_t val = tb_bits_get_u24_be(bst->p);
	bst->p += 3;
	return val;
	
}
tb_sint32_t tb_bstream_get_s24_be(tb_bstream_t* bst)
{
	tb_assert(!bst->b);
	
	tb_sint32_t val = tb_bits_get_s24_be(bst->p);
	bst->p += 3;
	return val;
	
}
tb_uint32_t tb_bstream_get_u32_be(tb_bstream_t* bst)
{
	tb_assert(!bst->b);
	
	tb_uint32_t val = tb_bits_get_u32_be(bst->p);;
	bst->p += 4;
	return val;
	
}
tb_sint32_t tb_bstream_get_s32_be(tb_bstream_t* bst)
{
	tb_assert(!bst->b);
	
	tb_sint32_t val = tb_bits_get_s32_be(bst->p);
	bst->p += 4;
	return val;
	
}
tb_uint32_t tb_bstream_get_u24_le(tb_bstream_t* bst)
{
	tb_assert(!bst->b);
	
	tb_uint32_t val = tb_bits_get_u24_le(bst->p);
	bst->p += 3;
	return val;
	
}
tb_sint32_t tb_bstream_get_s24_le(tb_bstream_t* bst)
{
	tb_assert(!bst->b);
	
	tb_sint32_t val = tb_bits_get_s24_le(bst->p);
	bst->p += 3;
	return val;
	
}
tb_uint32_t tb_bstream_get_u32_le(tb_bstream_t* bst)
{
	tb_assert(!bst->b);
	
	tb_uint32_t val = tb_bits_get_u32_le(bst->p);
	bst->p += 4;
	return val;
	
}
tb_sint32_t tb_bstream_get_s32_le(tb_bstream_t* bst)
{
	tb_assert(!bst->b);
	
	tb_sint32_t val = tb_bits_get_s32_le(bst->p);
	bst->p += 4;
	return val;
	
}

#ifdef TB_CONFIG_TYPE_FLOAT
tb_float_t tb_bstream_get_float_le(tb_bstream_t* bst)
{
	tb_assert(!bst->b);
	
	tb_float_t val = tb_bits_get_float_le(bst->p);
	bst->p += 4;
	return val;
}
tb_float_t tb_bstream_get_float_be(tb_bstream_t* bst)
{
	tb_assert(!bst->b);
	
	tb_float_t val = tb_bits_get_float_be(bst->p);
	bst->p += 4;
	return val;
}

tb_float_t tb_bstream_get_double_ble(tb_bstream_t* bst)
{
	tb_assert(!bst->b);
	
	tb_float_t val = tb_bits_get_double_ble(bst->p);
	bst->p += 8;
	return val;
}
tb_float_t tb_bstream_get_double_bbe(tb_bstream_t* bst)
{
	tb_assert(!bst->b);
	
	tb_float_t val = tb_bits_get_double_bbe(bst->p);
	bst->p += 8;
	return val;
}
tb_float_t tb_bstream_get_double_bne(tb_bstream_t* bst)
{
	tb_assert(!bst->b);
	
	tb_float_t val = tb_bits_get_double_bne(bst->p);
	bst->p += 8;
	return val;
}

tb_float_t tb_bstream_get_double_lle(tb_bstream_t* bst)
{
	tb_assert(!bst->b);
	
	tb_float_t val = tb_bits_get_double_lle(bst->p);
	bst->p += 8;
	return val;
}
tb_float_t tb_bstream_get_double_lbe(tb_bstream_t* bst)
{
	tb_assert(!bst->b);
	
	tb_float_t val = tb_bits_get_double_lbe(bst->p);
	bst->p += 8;
	return val;
}
tb_float_t tb_bstream_get_double_lne(tb_bstream_t* bst)
{
	tb_assert(!bst->b);
	
	tb_float_t val = tb_bits_get_double_lne(bst->p);
	bst->p += 8;
	return val;
}

tb_float_t tb_bstream_get_double_nle(tb_bstream_t* bst)
{
	tb_assert(!bst->b);
	
	tb_float_t val = tb_bits_get_double_nle(bst->p);
	bst->p += 8;
	return val;
}
tb_float_t tb_bstream_get_double_nbe(tb_bstream_t* bst)
{
	tb_assert(!bst->b);
	
	tb_float_t val = tb_bits_get_double_nbe(bst->p);
	bst->p += 8;
	return val;
}
tb_float_t tb_bstream_get_double_nne(tb_bstream_t* bst)
{
	tb_assert(!bst->b);
	
	tb_float_t val = tb_bits_get_double_nne(bst->p);
	bst->p += 8;
	return val;
}

#endif

tb_uint32_t tb_bstream_get_ubits32(tb_bstream_t* bst, tb_size_t nbits)
{
	if (!nbits || !bst) return 0;

	tb_uint32_t val = tb_bits_get_ubits32(bst->p, bst->b, nbits);
	bst->p += (bst->b + nbits) >> 3;
	bst->b = (bst->b + nbits) & 0x07;

	return val;
}
tb_sint32_t tb_bstream_get_sbits32(tb_bstream_t* bst, tb_size_t nbits)
{
	if (!nbits || !bst) return 0;

	tb_sint32_t val = tb_bits_get_sbits32(bst->p, bst->b, nbits);
	bst->p += (bst->b + nbits) >> 3;
	bst->b = (bst->b + nbits) & 0x07;

	return val;
}
tb_char_t const* tb_bstream_get_string(tb_bstream_t* bst)
{
	tb_assert(bst && bst->p <= bst->e);
	tb_bstream_sync(bst);

	// find '\0'
	tb_byte_t const* p = bst->p;
	while (*p && p < bst->e) p++;

	// is string with '\0' ?
	if ((*p)) return TB_NULL;

	tb_char_t const* s = (tb_char_t const*)bst->p;
	bst->p += p - bst->p + 1;

	return s;
}
tb_size_t tb_bstream_get_data(tb_bstream_t* bst, tb_byte_t* data, tb_size_t size)
{
	tb_assert(bst->e >= bst->p);
	tb_bstream_sync(bst);
	
	tb_size_t get_n = size;
	if (bst->e - bst->p < get_n) get_n = bst->e - bst->p;
	if (get_n)
	{
		tb_memcpy(data, bst->p, get_n);
		bst->p += get_n;
	}
	return (get_n > 0? get_n : 0);
}
/* /////////////////////////////////////////////////////////
 * set
 */
tb_void_t tb_bstream_set_u1(tb_bstream_t* bst, tb_uint8_t val)
{
	// set
	*(bst->p) &= ~(0x1 << (7 - bst->b));
	*(bst->p) |= ((val & 0x1) << (7 - bst->b));

	// next
	bst->b++;
	if (bst->b >= 8) 
	{
		bst->p++;
		bst->b = 0;
	}
}
tb_void_t tb_bstream_set_u8(tb_bstream_t* bst, tb_uint8_t val)
{
	tb_assert(!bst->b);
	*(bst->p++) = val;
}
tb_void_t tb_bstream_set_s8(tb_bstream_t* bst, tb_sint8_t val)
{
	tb_assert(!bst->b);
	*(bst->p++) = val;
}
tb_void_t tb_bstream_set_u16_le(tb_bstream_t* bst, tb_uint16_t val)
{
	tb_assert(!bst->b);
	tb_bits_set_u16_le(bst->p, val);
	bst->p += 2;
}
tb_void_t tb_bstream_set_s16_le(tb_bstream_t* bst, tb_sint16_t val)
{
	tb_assert(!bst->b);
	tb_bits_set_s16_le(bst->p, val);
	bst->p += 2;
}
tb_void_t tb_bstream_set_u32_le(tb_bstream_t* bst, tb_uint32_t val)
{
	tb_assert(!bst->b);
	tb_bits_set_u32_le(bst->p, val);
	bst->p += 4;
}
tb_void_t tb_bstream_set_s32_le(tb_bstream_t* bst, tb_sint32_t val)
{
	tb_assert(!bst->b);
	tb_bits_set_s32_le(bst->p, val);
	bst->p += 4;
}
tb_void_t tb_bstream_set_u16_be(tb_bstream_t* bst, tb_uint16_t val)
{
	tb_assert(!bst->b);
	tb_bits_set_u16_be(bst->p, val);
	bst->p += 2;
}
tb_void_t tb_bstream_set_s16_be(tb_bstream_t* bst, tb_sint16_t val)
{
	tb_assert(!bst->b);
	tb_bits_set_s16_be(bst->p, val);
	bst->p += 2;
}
tb_void_t tb_bstream_set_u32_be(tb_bstream_t* bst, tb_uint32_t val)
{
	tb_assert(!bst->b);
	tb_bits_set_u32_be(bst->p, val);
	bst->p += 4;
}
tb_void_t tb_bstream_set_s32_be(tb_bstream_t* bst, tb_sint32_t val)
{
	tb_assert(!bst->b);
	tb_bits_set_s32_be(bst->p, val);
	bst->p += 4;
}

#ifdef TB_CONFIG_TYPE_FLOAT
tb_void_t tb_bstream_set_float_le(tb_bstream_t* bst, tb_float_t val)
{
	tb_assert(!bst->b);
	
	tb_bits_set_float_le(bst->p, val);
	bst->p += 4;
}
tb_void_t tb_bstream_set_float_be(tb_bstream_t* bst, tb_float_t val)
{
	tb_assert(!bst->b);
	
	tb_bits_set_float_be(bst->p, val);
	bst->p += 4;
}
tb_void_t tb_bstream_set_double_ble(tb_bstream_t* bst, tb_float_t val)
{
	tb_assert(!bst->b);
	
	tb_bits_set_double_ble(bst->p, val);
	bst->p += 8;
}
tb_void_t tb_bstream_set_double_bbe(tb_bstream_t* bst, tb_float_t val)
{
	tb_assert(!bst->b);
	
	tb_bits_set_double_bbe(bst->p, val);
	bst->p += 8;
}
tb_void_t tb_bstream_set_double_bne(tb_bstream_t* bst, tb_float_t val)
{
	tb_assert(!bst->b);
	
	tb_bits_set_double_bne(bst->p, val);
	bst->p += 8;
}
tb_void_t tb_bstream_set_double_lle(tb_bstream_t* bst, tb_float_t val)
{
	tb_assert(!bst->b);
	
	tb_bits_set_double_lle(bst->p, val);
	bst->p += 8;
}
tb_void_t tb_bstream_set_double_lbe(tb_bstream_t* bst, tb_float_t val)
{
	tb_assert(!bst->b);
	
	tb_bits_set_double_lbe(bst->p, val);
	bst->p += 8;
}
tb_void_t tb_bstream_set_double_lne(tb_bstream_t* bst, tb_float_t val)
{
	tb_assert(!bst->b);
	
	tb_bits_set_double_lne(bst->p, val);
	bst->p += 8;
}
tb_void_t tb_bstream_set_double_nle(tb_bstream_t* bst, tb_float_t val)
{
	tb_assert(!bst->b);
	
	tb_bits_set_double_nle(bst->p, val);
	bst->p += 8;
}
tb_void_t tb_bstream_set_double_nbe(tb_bstream_t* bst, tb_float_t val)
{
	tb_assert(!bst->b);
	
	tb_bits_set_double_nbe(bst->p, val);
	bst->p += 8;
}
tb_void_t tb_bstream_set_double_nne(tb_bstream_t* bst, tb_float_t val)
{
	tb_assert(!bst->b);
	
	tb_bits_set_double_nne(bst->p, val);
	bst->p += 8;
}
#endif

tb_void_t tb_bstream_set_ubits32(tb_bstream_t* bst, tb_uint32_t val, tb_size_t nbits)
{
	if (!nbits || !bst) return ;

	tb_bits_set_ubits32(bst->p, bst->b, val, nbits);
	bst->p += (bst->b + nbits) >> 3;
	bst->b = (bst->b + nbits) & 0x07;
}

tb_void_t tb_bstream_set_sbits32(tb_bstream_t* bst, tb_sint32_t val, tb_size_t nbits)
{
	if (!nbits || !bst) return ;

	tb_bits_set_sbits32(bst->p, bst->b, val, nbits);
	bst->p += (bst->b + nbits) >> 3;
	bst->b = (bst->b + nbits) & 0x07;
}
tb_size_t tb_bstream_set_data(tb_bstream_t* bst, tb_byte_t const* data, tb_size_t size)
{
	tb_assert(bst->e >= bst->p);
	tb_bstream_sync(bst);

	tb_size_t set_n = size;
	if (bst->e - bst->p < set_n) set_n = bst->e - bst->p;
	if (set_n)
	{
		tb_memcpy(bst->p, data, set_n);
		bst->p += set_n;
	}
	return (set_n > 0? set_n : 0);
}

tb_char_t* tb_bstream_set_string(tb_bstream_t* bst, tb_char_t const* s)
{
	tb_assert(bst->e >= bst->p);
	tb_bstream_sync(bst);

	tb_char_t* b = bst->p;
	tb_char_t* p = bst->p;
	tb_char_t* e = bst->e - 1;
	while (*s && p < e) *p++ = *s++;
	*p++ = '\0';
	bst->p = p;

	return b;
}
/* /////////////////////////////////////////////////////////
 * peek
 */
tb_uint32_t tb_bstream_peek_ubits32(tb_bstream_t* bst, tb_size_t nbits)
{
	if (!nbits || !bst) return 0;

	// save status
	tb_byte_t const* p = bst->p;
	tb_size_t b = bst->b;

	// peek value
	tb_uint32_t val = tb_bstream_get_ubits32(bst, nbits);

	// restore status
	bst->p = p;
	bst->b = b;

	return val;
}
tb_sint32_t tb_bstream_peek_sbits32(tb_bstream_t* bst, tb_size_t nbits)
{
	if (!nbits || !bst) return 0;

	// save status
	tb_byte_t const* p = bst->p;
	tb_size_t b = bst->b;

	// peek value
	tb_sint32_t val = tb_bstream_get_sbits32(bst, nbits);

	// restore status
	bst->p = p;
	bst->b = b;

	return val;
}

