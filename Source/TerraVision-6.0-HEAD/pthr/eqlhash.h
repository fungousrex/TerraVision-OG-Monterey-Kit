/*
 * Name: eqlhash.h
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: eqlhash.h,v 1.3.2.1 2001/06/07 05:56:38 leei Exp $
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

#ifndef _eqlhash_h_
#define _eqlhash_h_

#include <prtypes.h>

typedef struct eqlhash *eqlhash_t;
typedef struct eqlhash_entry *eqlhash_iter_t;

typedef int (*eqlhash_eql) (void const *, void const *);
typedef PRUint32 (*eqlhash_hash) (void const *);

/* Multiplicative hash function.  This can be used to randomize a simple hash
   value. */
#define EQLHASH_A (2654435769U)
#define EQLHASH(k) (EQLHASH_A*((PRUint32)k))

eqlhash_t eqlhash_create (int size, eqlhash_eql eql, eqlhash_hash hash);

void *  eqlhash_get (eqlhash_t tbl, void const *obj);
int     eqlhash_put (eqlhash_t tbl, void const *obj, void *val);
int     eqlhash_used (eqlhash_t tbl);

void    eqlhash_delete (eqlhash_t tbl, void const *obj);
void    eqlhash_destroy (eqlhash_t tbl);
void	eqlhash_clear (eqlhash_t tbl);

void *  eqlhash_use_or_create (eqlhash_t tbl, void const *obj,
			       void *(create_f) (void *),
			       void *closure);

void    eqlhash_forall (eqlhash_t tbl, 
		       void (*func)(void const *obj, void *val, void *closure),
		       void *closure);

#if (defined(__cplusplus) || defined(HAVE_INLINE)) && !defined(NO_eqlhash_INLINE)
# include "istack.h"
# ifdef __GNUC__
#  define exinline extern inline
# elif ( defined(WIN32) && defined(MSVC) )
#  define exinline __inline
# else
#  define exinline inline
# endif
# include "eqlhash_inline.h"
# undef exinline
#endif /* HAVE_INLINE */

#endif /* _eqlhash_h_ */
