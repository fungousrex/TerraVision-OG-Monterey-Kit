/*
 * Name: eqhash_inline.h
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: eqhash_inline.h,v 1.11 2000/09/29 22:50:31 leei Exp $
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

#include "eqhashI.h"

exinline PRUint32
eqhash_hash (eqhash_t tbl, void const *obj)
{
  PRUint32 hash = EQHASH (obj);
  hash >>= (28 - tbl->log_size);
  hash &= ((1<<tbl->log_size)-1);
  return hash;
}

exinline int
eqhash_used (eqhash_t tbl)
{
  return tbl->num_entries;
}

exinline eqhash_entry_t **
eqhash_lookup (eqhash_t tbl, void const *obj)
{
  eqhash_entry_t **pp, *p;
  PRUint32 hash = eqhash_hash (tbl, obj);

  for (pp = &tbl->entries[hash]; (p = *pp); pp = &p->next)
    if (p->obj == obj)
      return pp;
  
  return pp;
}

exinline void *
eqhash_get (eqhash_t tbl, void const *obj)
{
  eqhash_entry_t *lp, **lpp = eqhash_lookup (tbl, obj);
  return (lp = *lpp) ? lp->val : (void*)0;
}

