/*
 * Name: eqhash_inline.h
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: eqlhash_inline.h,v 1.3.4.1 2001/06/07 05:56:38 leei Exp $
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

#include "eqlhashI.h"

exinline int
eqlhash_used (eqlhash_t tbl)
{
  return tbl->num_entries;
}

exinline eqlhash_iter_t *
eqlhash_lookup (eqlhash_t tbl, void const *obj, PRUint32 hash)
{
  eqlhash_iter_t *pp, p;
  PRUint32 idx = hash;

  idx >>= (28 - tbl->log_size);
  idx &= ((1<<tbl->log_size)-1);

  for (pp = &tbl->entries[idx]; (p = *pp); pp = &p->next)
    if (hash == p->hash && (*tbl->eql_f) (p->obj, obj))
      return pp;
  
  return pp;
}

exinline void *
eqlhash_get (eqlhash_t tbl, void const *obj)
{
  PRUint32 hash = (*tbl->hash_f) (obj);
  eqlhash_iter_t lp, *lpp = eqlhash_lookup (tbl, obj, hash);
  return (lp = *lpp) ? lp->val : (void*)0;
}
