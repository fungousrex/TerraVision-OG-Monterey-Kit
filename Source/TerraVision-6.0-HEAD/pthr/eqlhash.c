/*
 * Name: eqlhash.c
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes. This file implements a generic hash table.
 *
 * Revision History:
 *   $Id: eqlhash.c,v 1.3 2000/09/29 22:50:31 leei Exp $
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "eqlhash.h"
#include "eqlhashI.h"

#define TABLE_ALLOC(tbl,size) istack_alloc(tbl->data, size)

eqlhash_t
eqlhash_create (int size, eqlhash_eql eql_f, eqlhash_hash hash_f)
{
  eqlhash_t tbl = malloc (sizeof (*tbl));
  int log_size = -1;

  if (! size) size = 128;
  for ( ; size; size >>= 1)
    ++log_size;
  
  tbl->log_size = log_size;
  tbl->num_entries = 0;
  tbl->entries = calloc (1<<log_size, sizeof (tbl->entries[0]));
  tbl->freelist = 0;

  tbl->eql_f = eql_f;
  tbl->hash_f = hash_f;

  tbl->data = istack_create (64*sizeof(tbl->data[0]));

  return tbl;
}

void
eqlhash_destroy (eqlhash_t tbl)
{
  istack_destroy (tbl->data);
  free (tbl->entries);
  free (tbl);
}

void
eqlhash_clear (eqlhash_t tbl)
{
  memset (tbl->entries, 0 , (1<<tbl->log_size) * sizeof (tbl->entries[0]));
  istack_clear (tbl->data);
  tbl->num_entries = 0;
  tbl->freelist = 0;
}

static void
grow_table (eqlhash_t tbl, int old_size)
{
  eqlhash_iter_t *old_entries = tbl->entries;
  eqlhash_iter_t *end_entries = old_entries + old_size;
  eqlhash_iter_t *opp;

  ++tbl->log_size;
  tbl->entries = calloc (2*old_size, sizeof (*opp));
  for (opp = old_entries; opp < end_entries; ++opp) {
    eqlhash_iter_t lp = *opp;
    while (lp) {
      eqlhash_iter_t next = lp->next;
      eqlhash_iter_t *lpp = eqlhash_lookup (tbl, lp->obj, lp->hash);
      lp->next = *lpp;
      *lpp = lp;
      lp = next;
    }
  }
  free (old_entries);
}

static inline void
maybe_grow (eqlhash_t tbl)
{
  int old_size = 1<<tbl->log_size;
  if (! tbl->freelist && 2*tbl->num_entries > old_size) 
    grow_table (tbl, old_size);
}

int
eqlhash_put (eqlhash_t tbl, void const *obj, void *val)
{
  PRUint32 hash = (*tbl->hash_f) (obj);
  eqlhash_iter_t *lpp, lp;

  maybe_grow (tbl);
  lpp = eqlhash_lookup (tbl, obj, hash);

  if ((lp = *lpp)) {
    /* Replace current value. */
    return (lp->val != val) ? (lp->val = val, 0) : 1;
  } else {
    if ((lp = tbl->freelist))
      tbl->freelist = lp->next;
    else
      lp = (eqlhash_iter_t) TABLE_ALLOC (tbl, sizeof (*lp));
    
    lp->next = 0;
    lp->obj = obj;
    lp->val = val;
    lp->hash = hash;

    *lpp = lp;
    ++tbl->num_entries;

    return 0;
  }
}

void *
eqlhash_use_or_create (eqlhash_t tbl, void const *obj,
		       void *(create_f) (void *),
		       void *closure)
{
  PRUint32 hash = (*tbl->hash_f) (obj);
  eqlhash_iter_t *lpp, lp;

  maybe_grow (tbl);
  lpp = eqlhash_lookup (tbl, obj, hash);

  if ((lp = *lpp)) {
    return lp->val;
  } else {
    if ((lp = tbl->freelist))
      tbl->freelist = lp->next;
    else
      lp = (eqlhash_iter_t) TABLE_ALLOC (tbl, sizeof (*lp));
    
    lp->obj = obj;
    lp->val = (*create_f) (closure);
    lp->hash = hash;

    *lpp = lp;
    ++tbl->num_entries;

    return 0;
  }
}

void
eqlhash_delete (eqlhash_t tbl, void const *obj)
{
  PRUint32 hash = (*tbl->hash_f) (obj);
  eqlhash_iter_t *lpp, lp;

  lpp = eqlhash_lookup (tbl, obj, hash);
  if ((lp = *lpp)) {
    *lpp = lp->next;
    lp->next = tbl->freelist;
    tbl->freelist = lp;
    --tbl->num_entries;
  }
}

void
eqlhash_forall (eqlhash_t tbl, 
	       void (*func)(void const *obj, void *val, void *closure),
	       void *closure)
{
  int size = 1<<tbl->log_size;
  eqlhash_iter_t *entries = tbl->entries;
  eqlhash_iter_t *end_entries = entries + size;
  eqlhash_iter_t *lpp;

  for (lpp = entries; lpp < end_entries; ++lpp) {
    eqlhash_iter_t lp = *lpp;
    for ( ; lp; lp = lp->next) {
      (*func) (lp->obj, lp->val, closure);
    }
  }
}
