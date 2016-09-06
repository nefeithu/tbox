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
 * Copyright (C) 2009 - 2017, ruki All rights reserved.
 *
 * @author      ruki
 * @file        context.c
 * @ingroup     platform
 */

/* //////////////////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "context.h"

/* //////////////////////////////////////////////////////////////////////////////////////
 * implementation
 */
#if defined(TB_CONFIG_POSIX_HAVE_GETCONTEXT) && \
        defined(TB_CONFIG_POSIX_HAVE_SETCONTEXT) && \
        defined(TB_CONFIG_POSIX_HAVE_MAKECONTEXT)
#   include "posix/context.c"
#else
tb_size_t tb_context_size()
{
    tb_trace_noimpl();
    return 0;
}
tb_bool_t tb_context_get(tb_context_ref_t context)
{
    tb_trace_noimpl();
    return tb_false;
}
tb_bool_t tb_context_set(tb_context_ref_t context)
{
    tb_trace_noimpl();
    return tb_false;
}
tb_bool_t tb_context_make(tb_context_ref_t context, tb_context_ref_t context_link, tb_pointer_t stack, tb_size_t stacksize, tb_context_func_t func, tb_cpointer_t priv)
{
    tb_trace_noimpl();
    return tb_false;
}
#endif

#ifndef TB_CONFIG_POSIX_HAVE_SWAPCONTEXT
tb_bool_t tb_context_swap(tb_context_ref_t context, tb_context_ref_t context_new)
{
    // swap it
    if (tb_context_get(context))
        tb_context_set(context_new);

    // ok
    return tb_true;
}
#endif
