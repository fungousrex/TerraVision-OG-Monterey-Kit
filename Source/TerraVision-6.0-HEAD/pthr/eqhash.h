/*
 * Name: eqhash.h
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: eqhash.h,v 1.10.2.1 2001/06/07 05:56:38 leei Exp $
 *
 * License:
 *   The contents of this file are subject to the Open Source Apache
 *   Software License Version 1.1 (the "License"); you may not use
 *   this file except in compliance with the License. You may obtain a
 *   copy of the License at http://www.tsmapi/license.shtml.
 *
 *   Software distributed under the License is distributed on an "AS IS"
 *   basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 *   License for the specific language governing rights and limitations
 *   under the License.
 *
 *   Portions are Copyright (c) SRI International, 1998-2000.
 *
 */

#ifndef _eqhash_h_
#define _eqhash_h_

typedef struct eqhash *eqhash_t;

eqhash_t eqhash_create (int size);

void *  eqhash_get (eqhash_t tbl, void const *obj);
int     eqhash_put (eqhash_t tbl, void const *obj, void *val);
int     eqhash_used (eqhash_t tbl);

int     eqhash_delete (eqhash_t tbl, void const *obj);
void    eqhash_destroy (eqhash_t tbl);
void	eqhash_clear (eqhash_t tbl);

void    eqhash_forall (eqhash_t tbl, 
		       void (*func)(void const *obj, void *val, void *closure),
		       void *closure);

#if (defined(__cplusplus) || defined(HAVE_INLINE)) && !defined(NO_eqhash_INLINE)
# include "istack.h"
# ifdef __GNUC__
#  define exinline extern inline
# elif ( defined(WIN32) && defined(MSVC) )
#  define exinline __inline
# else
#  define exinline inline
# endif
# include "eqhash_inline.h"
# undef exinline
#endif /* HAVE_INLINE */

#endif /* _eqhash_h_ */
