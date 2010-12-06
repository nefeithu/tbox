/*!The Tiny Box Library
 * 
 * TBox is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * TBox is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with TBox; 
 * If not, see <a href="http://www.gnu.org/licenses/"> http://www.gnu.org/licenses/</a>
 * 
 * Copyright (C) 2009 - 2010, ruki All rights reserved.
 *
 * \author		ruki
 * \file		zstream.c
 *
 */
/* /////////////////////////////////////////////////////////
 * includes
 */
#include "lzsw.h"
#include "../../container/container.h"
#include "../../math/math.h"

/* /////////////////////////////////////////////////////////
 * macros
 */

/* /////////////////////////////////////////////////////////
 * details
 */
#if TB_LZSW_WINDOW_HASH_FIND

/* circle offset => global offset
 *
 * window: |-----------------------------------------------------------|
 * global: 0, 1, 2, ...                                           TB_LZSW_WINDOW_SIZE_MAX - 1
 * circle: 3, 4, 5, ...             TB_LZSW_WINDOW_SIZE_MAX - 1, 0, 1, 2
 *                                                               |
 *                                                         window->base
 *
 */
static __tplat_inline__ tb_size_t tb_lzsw_window_goff(tb_size_t base, tb_size_t coff)
{
	return ((coff + base) % TB_LZSW_WINDOW_SIZE_MAX);
}
static __tplat_inline__ tb_size_t tb_lzsw_window_coff(tb_size_t base, tb_size_t goff)
{
	return ((goff + TB_LZSW_WINDOW_SIZE_MAX - base) % TB_LZSW_WINDOW_SIZE_MAX);
}
static void tb_lzsw_window_insert(tb_lzsw_deflate_window_t* window, tb_size_t size)
{
	tb_pool_t* pool = window->pool;
	tb_byte_t const* wb = window->e - window->n;
	tb_byte_t const* wp = window->e - size;
	tb_byte_t const* we = window->e;
	for (; wp < we; wp++)
	{
		// alloc node
		tb_size_t node = tb_pool_alloc(pool);
		if (node)
		{
			// init node
			tb_lzsw_node_t* onode = TB_POOL_GET(pool, node, tb_lzsw_node_t);
			TB_ASSERT(onode);
			onode->sign[0] = wp[0];
			onode->sign[1] = wp[1];
			onode->sign[2] = wp[2];
			onode->addr = wp;
			onode->coff = tb_lzsw_window_coff(window->base, wp - wb);

			// insert node to the head of hash 
			tb_size_t idx = wp[0] + wp[1] + wp[2];
			tb_size_t head = window->hash[idx];
			if (head)
			{
				tb_lzsw_node_t* onext = TB_POOL_GET(pool, head, tb_lzsw_node_t);
				TB_ASSERT(onext);

				tb_lzsw_node_t* oprev = TB_POOL_GET(pool, onext->prev, tb_lzsw_node_t);
				TB_ASSERT(oprev);

				onode->next = head;
				onode->prev = onext->prev;
				onext->prev = node;
				oprev->next = node;
			}
			else
			{
				onode->next = node;
				onode->prev = node;
			}
			window->hash[idx] = node;
		}
		else break;
	}

#if 0
	{
		TB_DBG("-----------------------");
		TB_DBG("insert: %d", size);
		tb_size_t idx = 0;
		for (idx = 0; idx < TB_LZSW_WINDOW_HASH_MAX; idx++)
		{
			tb_size_t head = window->hash[idx];
			if (head)
			{
				// get the head
				tb_lzsw_node_t* ohead = TB_POOL_GET(pool, head, tb_lzsw_node_t);
				TB_DBG("%d: %02x%02x%02x", idx, ohead->sign[0], ohead->sign[1], ohead->sign[2]);

				// remove from the tail
				tb_size_t node = ohead->prev;
				while (node != head)
				{
					// get node
					tb_lzsw_node_t* onode = TB_POOL_GET(pool, node, tb_lzsw_node_t);
					TB_ASSERT(onode);

					TB_DBG("%02x%02x%02x", onode->sign[0], onode->sign[1], onode->sign[2]);
				
					// next
					node = onode->next;
				}
			}
		}
	}
#endif
}
static void tb_lzsw_window_remove(tb_lzsw_deflate_window_t* window, tb_size_t size)
{
	tb_pool_t* pool = window->pool;
	tb_byte_t const* wb = window->e - window->n;
	tb_byte_t const* wp = wb;
	tb_byte_t const* we = wb + size;
	for (; wp < we; wp++)
	{
		tb_size_t idx = wp[0] + wp[1] + wp[2];
		tb_size_t head = window->hash[idx];
		if (head)
		{
			// get the head
			tb_lzsw_node_t* ohead = TB_POOL_GET(pool, head, tb_lzsw_node_t);

			// remove from the tail
			tb_size_t node = ohead->prev;
			while (node != head)
			{
				// get node
				tb_lzsw_node_t* onode = TB_POOL_GET(pool, node, tb_lzsw_node_t);
				TB_ASSERT(onode);

				// get next & prev 
				tb_size_t next = onode->next;
				tb_size_t prev = onode->prev;
			
				// remove it
				if (onode->addr < we)
				{
					// detach it
					TB_POOL_SET_NEXT(pool, onode->prev, tb_lzsw_node_t, next);	
					TB_POOL_SET_PREV(pool, onode->next, tb_lzsw_node_t, prev);	

					// free it
					tb_pool_free(pool, node);
				}
			
				// prev
				node = prev;
			}
				
			// remove head
			if (ohead->addr < we)
			{
				// detach it
				window->hash[idx] = 0;
			
				// free it
				tb_pool_free(pool, head);
			}
		}
	}

#if 0
	{
		TB_DBG("-----------------------");
		TB_DBG("remove: %d", size);
		tb_size_t idx = 0;
		for (idx = 0; idx < TB_LZSW_WINDOW_HASH_MAX; idx++)
		{
			tb_size_t head = window->hash[idx];
			if (head)
			{
				// get the head
				tb_lzsw_node_t* ohead = TB_POOL_GET(pool, head, tb_lzsw_node_t);
				TB_DBG("%d: %02x%02x%02x", idx, ohead->sign[0], ohead->sign[1], ohead->sign[2]);

				// remove from the tail
				tb_size_t node = ohead->prev;
				while (node != head)
				{
					// get node
					tb_lzsw_node_t* onode = TB_POOL_GET(pool, node, tb_lzsw_node_t);
					TB_ASSERT(onode);

					TB_DBG("%02x%02x%02x", onode->sign[0], onode->sign[1], onode->sign[2]);
				
					// next
					node = onode->next;
				}
			}
		}
	}
#endif
}
#if 0
static tb_size_t tb_lzsw_window_find(tb_lzsw_deflate_window_t* window, tb_byte_t const* sp, tb_byte_t const* se, tb_size_t* p)
{
	tb_byte_t const* wb = window->e - window->n;
	tb_byte_t const* wp = wb;
	tb_byte_t const* we = window->e;
	tb_byte_t const* mp = wp;
	tb_byte_t const* me = wp;
	for (; wp < we; wp++)
	{
		tb_byte_t const* wq = wp;
		tb_byte_t const* sq = sp;
		for (; wq < we && sq < se && *wq == *sq; wq++, sq++);
		if ((wq - wp) > (me - mp))
		{
			mp = wp;
			me = wq;
		}
	}

	*p = mp - wb;
	return (me - mp);
}
#else
static tb_size_t tb_lzsw_window_find(tb_lzsw_deflate_window_t* window, tb_byte_t const* sp, tb_byte_t const* se, tb_size_t* p)
{
	tb_byte_t const* wb = window->e - window->n;
	tb_byte_t const* we = window->e;
	tb_byte_t const* mp = wb;
	tb_byte_t const* me = wb;

	tb_pool_t* pool = window->pool;
	tb_size_t idx = sp[0] + sp[1] + sp[2];
	tb_size_t head = window->hash[idx];
	if (head)
	{
		// get the head
		tb_lzsw_node_t* ohead = TB_POOL_GET(pool, head, tb_lzsw_node_t);

		// find it in the head first
		if (ohead->sign[0] == sp[0]
			&& ohead->sign[1] == sp[1]
			&& ohead->sign[2] == sp[2])
		{
			tb_byte_t const* wp = ohead->addr;
			//tb_byte_t const* wp = wb + tb_lzsw_window_goff(window->base, ohead->coff);
			tb_byte_t const* wq = wp + 3;
			tb_byte_t const* sq = sp + 3;
			for (; wq < we && sq < se && *wq == *sq; wq++, sq++);
			if ((wq - wp) >= (me - mp))
			{
				mp = wp;
				me = wq;
			}
		}

		// remove from the tail
		tb_size_t node = ohead->prev;
		while (node != head)
		{
			// get node
			tb_lzsw_node_t* onode = TB_POOL_GET(pool, node, tb_lzsw_node_t);
			TB_ASSERT(onode);

			// find it
			if (onode->sign[0] == sp[0]
				&& onode->sign[1] == sp[1]
				&& onode->sign[2] == sp[2])
			{
				tb_byte_t const* wp = onode->addr;
				//tb_byte_t const* wp = wb + tb_lzsw_window_goff(window->base, onode->coff);
				tb_byte_t const* wq = wp + 3;
				tb_byte_t const* sq = sp + 3;
				for (; wq < we && sq < se && *wq == *sq; wq++, sq++);
				if ((wq - wp) >= (me - mp))
				{
					mp = wp;
					me = wq;
				}

				//if (onode->addr != wb + tb_lzsw_window_goff(window->base, onode->coff))
				{
				//	TB_DBG("%x %x: %d %d", onode->addr, wb + tb_lzsw_window_goff(window->base, onode->coff), window->base, onode->coff);
				}
			}

			// next
			node = onode->next;
		}
	}

	*p = mp - wb;
	return (me - mp);
}
#endif
#else
// find the maximum matched data
static tb_size_t tb_lzsw_window_find(tb_lzsw_deflate_window_t* window, tb_byte_t const* sp, tb_byte_t const* se, tb_size_t* p)
{
	tb_byte_t const* wb = window->e - window->n;
	tb_byte_t const* wp = wb;
	tb_byte_t const* we = window->e;
	tb_byte_t const* mp = wp;
	tb_byte_t const* me = wp;
	for (; wp < we; wp++)
	{
		tb_byte_t const* wq = wp;
		tb_byte_t const* sq = sp;
		for (; wq < we && sq < se && *wq == *sq; wq++, sq++);
		if ((wq - wp) > (me - mp))
		{
			mp = wp;
			me = wq;
		}
	}

	*p = mp - wb;
	return (me - mp);
}
#endif

/* /////////////////////////////////////////////////////////
 * inflate
 */
static tb_bstream_t* tb_zstream_inflate_lzsw_transform(tb_tstream_t* st)
{
	tb_lzsw_inflate_zstream_t* zst = (tb_lzsw_inflate_zstream_t*)st;
	TB_ASSERT(zst);
	if (!zst) return TB_NULL;

	// get dst
	tb_byte_t* dp = st->dst.p;
	tb_byte_t* de = st->dst.e;
	tb_byte_t* db = dp;
	TB_ASSERT(dp && de);
	if (!dp || !de) return TB_NULL;

	// get src
	tb_bstream_t* src = tb_tstream_src(st);

	// get vlc
	tb_zstream_vlc_t* vlc = zst->vlc;
	TB_ASSERT(vlc && vlc->get);

	// vlc callback
	tb_zstream_vlc_get_t vlc_get = vlc->get;

	// init window
	tb_lzsw_inflate_window_t* window = &zst->window;
	window->e = dp;

	// inflate 
	while (tb_bstream_left_bits(src) > 8 && (dp < de))
	{
		// get flag
		if (tb_bstream_get_u1(src))
		{
			// set position
			tb_size_t p = tb_bstream_get_ubits(src, window->b);
			
			// get size
			tb_size_t n = vlc_get(vlc, src) + 2;

			//TB_DBG("%d %d", p, n);
#if 1 
			// fill data
			// \note: address maybe overlap
			memcpy(dp, window->e + p - window->n, n);

			// update dp
			dp += n;
#else
			tb_byte_t const* wp = window->e + p - window->n;
			while (n--) *dp++ = *wp++;
#endif

		}
		else 
		{
			*dp++ = tb_bstream_get_ubits(src, 8);
			*dp++ = tb_bstream_get_ubits(src, 8);
		}

		// update window
		window->e = dp;
		if (dp - db <= TB_LZSW_WINDOW_SIZE_MAX) 
		{
			window->n = dp - db;
			window->b = TB_MATH_ICLOG2I(window->n);
		}
	}

	// update position
	st->dst.p = dp;
	return tb_tstream_dst(st);
}
static void tb_zstream_inflate_lzsw_close(tb_tstream_t* st)
{
	tb_lzsw_inflate_zstream_t* zst = (tb_lzsw_inflate_zstream_t*)st;
	if (zst) 
	{
		// close vlc
		if (zst->vlc && zst->vlc->close) zst->vlc->close(zst->vlc); 

		// reset it
		memset(zst, 0, sizeof(tb_lzsw_inflate_zstream_t));
	}
}
/* /////////////////////////////////////////////////////////
 * deflate
 */
static tb_bstream_t* tb_zstream_deflate_lzsw_transform(tb_tstream_t* st)
{
	tb_lzsw_deflate_zstream_t* zst = (tb_lzsw_deflate_zstream_t*)st;
	TB_ASSERT(zst);
	if (!zst) return TB_NULL;

	// get src
	tb_byte_t* sp = st->src.p;
	tb_byte_t* se = st->src.e;
	tb_byte_t* sb = sp;
	TB_ASSERT(sp && se);
	if (!sp || !se) return TB_NULL;

	// get dst
	tb_bstream_t* dst = tb_tstream_dst(st);

	// get vlc
	tb_zstream_vlc_t* vlc = zst->vlc;
	TB_ASSERT(vlc && vlc->set);

	// vlc callback
	tb_zstream_vlc_set_t vlc_set = vlc->set;

	// init window
	tb_lzsw_deflate_window_t* window = &zst->window;
	window->e = sp;

	// deflate 
	while (sp < se)
	{
		tb_size_t p = 0;
		tb_size_t n = tb_lzsw_window_find(window, sp, se, &p);
		if (n > 2)
		{
			// set flag
			tb_bstream_set_u1(dst, 1);

			// set position
			tb_bstream_set_ubits(dst, p, window->b);
			
			// set size
			vlc_set(vlc, n - 2, dst);

			// update sp
			sp += n;

			//TB_DBG("%d %d", p, n);
		}
		else
		{
			// set flag
			tb_bstream_set_u1(dst, 0);

			// set value
			tb_bstream_set_ubits(dst, *sp++, 8);
			tb_bstream_set_ubits(dst, *sp++, 8);
		}

#if TB_LZSW_WINDOW_HASH_FIND
		// the old window
		tb_byte_t* 	owe = window->e;
		tb_size_t 	own = window->n;
		tb_byte_t* 	owb = owe - own;

		// the new window 
		tb_byte_t* 	we = sp;
		tb_size_t 	wn = TB_MATH_MIN((sp - sb), TB_LZSW_WINDOW_SIZE_MAX);
		tb_byte_t* 	wb = we - wn;

		// the number of moving by left & right 
		tb_size_t 	ln = TB_MATH_MIN((wb - owb), TB_LZSW_WINDOW_SIZE_MAX);
		tb_size_t 	rn = TB_MATH_MIN((we - owe), TB_LZSW_WINDOW_SIZE_MAX);

		// remove the old nodes
		tb_lzsw_window_remove(window, ln);

		// slide window
		window->e = we;
		window->n = wn;
		window->b = ((sp - sb) > TB_LZSW_WINDOW_SIZE_MAX)? window->b : TB_MATH_ICLOG2I(wn);
		window->base = (window->base + TB_LZSW_WINDOW_SIZE_MAX - ln) % TB_LZSW_WINDOW_SIZE_MAX;
		//TB_DBG("[window]: at: %d, base: %d, ln: %d, rn: %d, coff0: %d", wb - sb, window->base, ln, rn, tb_lzsw_window_coff(window->base, 0));

		// insert the new nodes
		tb_lzsw_window_insert(window, rn);
#else
		// update window
		window->e = sp;
		if (sp - sb <= TB_LZSW_WINDOW_SIZE_MAX) 
		{
			window->n = sp - sb;
			window->b = TB_MATH_ICLOG2I(window->n);
		}
#endif
	}

	// sync 
	tb_bstream_sync(dst);

	// update position
	st->src.p = sp;
	return dst;
}
static void tb_zstream_deflate_lzsw_close(tb_tstream_t* st)
{
	tb_lzsw_deflate_zstream_t* zst = (tb_lzsw_deflate_zstream_t*)st;
	if (zst) 
	{
		// close vlc
		if (zst->vlc && zst->vlc->close) zst->vlc->close(zst->vlc); 

		// free pool
#if TB_LZSW_WINDOW_HASH_FIND
		if (zst->window.pool) tb_pool_destroy(zst->window.pool);
#endif

		// reset it
		memset(zst, 0, sizeof(tb_lzsw_deflate_zstream_t));
	}
}

/* /////////////////////////////////////////////////////////
 * interfaces
 */

tb_tstream_t* tb_zstream_open_lzsw_inflate(tb_lzsw_inflate_zstream_t* zst)
{
	TB_ASSERT(zst);
	if (!zst) return TB_NULL;

	// init 
	memset(zst, 0, sizeof(tb_lzsw_inflate_zstream_t));

	// init tstream
	((tb_tstream_t*)zst)->transform = tb_zstream_inflate_lzsw_transform;
	((tb_tstream_t*)zst)->close 	= tb_zstream_inflate_lzsw_close;

	// init zstream
	((tb_zstream_t*)zst)->algo 		= TB_ZSTREAM_ALGO_LZSW;
	((tb_zstream_t*)zst)->action 	= TB_ZSTREAM_ACTION_INFLATE;

	// open vlc
#if TB_LZSW_VLC_TYPE_GOLOMB
	zst->vlc = tb_zstream_vlc_golomb_open(&(((tb_zstream_t*)zst)->vlc), 4);
#elif TB_LZSW_VLC_TYPE_GAMMA
	zst->vlc = tb_zstream_vlc_gamma_open(&(((tb_zstream_t*)zst)->vlc));
#else
	zst->vlc = tb_zstream_vlc_fixed_open(&(((tb_zstream_t*)zst)->vlc), 16);
#endif

	return ((tb_tstream_t*)zst);
}
tb_tstream_t* tb_zstream_open_lzsw_deflate(tb_lzsw_deflate_zstream_t* zst)
{
	TB_ASSERT(zst);
	if (!zst) return TB_NULL;

	// init 
	memset(zst, 0, sizeof(tb_lzsw_deflate_zstream_t));

	// init tstream
	((tb_tstream_t*)zst)->transform = tb_zstream_deflate_lzsw_transform;
	((tb_tstream_t*)zst)->close 	= tb_zstream_deflate_lzsw_close;

	// init zstream
	((tb_zstream_t*)zst)->algo 		= TB_ZSTREAM_ALGO_LZSW;
	((tb_zstream_t*)zst)->action 	= TB_ZSTREAM_ACTION_DEFLATE;

	// open vlc
#if TB_LZSW_VLC_TYPE_GOLOMB
	zst->vlc = tb_zstream_vlc_golomb_open(&(((tb_zstream_t*)zst)->vlc), 4);
#elif TB_LZSW_VLC_TYPE_GAMMA
	zst->vlc = tb_zstream_vlc_gamma_open(&(((tb_zstream_t*)zst)->vlc));
#else
	zst->vlc = tb_zstream_vlc_fixed_open(&(((tb_zstream_t*)zst)->vlc), 16);
#endif

	// create pool
#if TB_LZSW_WINDOW_HASH_FIND
	zst->window.pool = tb_pool_create(sizeof(tb_lzsw_node_t), TB_LZSW_WINDOW_SIZE_MAX, 0);
#endif

	return ((tb_tstream_t*)zst);
}

tb_tstream_t* tb_zstream_open_lzsw(tb_lzsw_zstream_t* zst, tb_size_t action)
{
	if (action == TB_ZSTREAM_ACTION_INFLATE)
		return tb_zstream_open_lzsw_inflate((tb_lzsw_inflate_zstream_t*)zst);
	else if (action == TB_ZSTREAM_ACTION_DEFLATE)
		return tb_zstream_open_lzsw_deflate((tb_lzsw_deflate_zstream_t*)zst);
	else return TB_NULL;
}
