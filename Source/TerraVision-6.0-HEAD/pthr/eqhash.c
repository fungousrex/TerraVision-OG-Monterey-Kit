/*
 * Name: eqhash.c
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes. This file is a generic hash function.
 *
 * Revision History:
 *   $Id: eqhash.c,v 1.11 2000/09/29 22:50:31 leei Exp $
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

#include "eqhash.h"
#include "eqhashI.h"

#define TABLE_ALLOC(tbl,size) istack_alloc(tbl->data, size)

eqhash_t
eqhash_create (int size)
{
  eqhash_t tbl = malloc (sizeof (*tbl));
  int log_size = -1;

  if (! size) size = 128;
  for ( ; size; size >>= 1)
    ++log_size;
  
  tbl->log_size = log_size;
  tbl->num_entries = 0;
  tbl->entries = calloc (1<<log_size, sizeof (tbl->entries[0]));
  tbl->freelist = 0;

  tbl->data = istack_create (64*sizeof(eqhash_entry_t));

  return tbl;
}

void
eqhash_destroy (eqhash_t tbl)
{
  istack_destroy (tbl->data);
  free (tbl->entries);
  free (tbl);
}

void
eqhash_clear (eqhash_t tbl)
{
  memset (tbl->entries, 0 , (1<<tbl->log_size) * sizeof (tbl->entries[0]));
  istack_clear (tbl->data);
  tbl->num_entries = 0;
  tbl->freelist = 0;
}

static void
print_table (eqhash_t tbl, FILE *out)
{
  int size = 1<<tbl->log_size;
  eqhash_entry_t **entries = tbl->entries;
  eqhash_entry_t **end_entries = entries + size;
  eqhash_entry_t **pp;

  fprintf(out, "hash %p {\n", tbl);
  for (pp = entries; pp < end_entries; ++pp) {
    eqhash_entry_t *lp = *pp;
    if (lp) {
      fprintf (out, "  [%d:%u]", pp-entries, EQHASH(lp->obj));
      for ( ; lp; lp = lp->next) {
	fprintf (out, " %p{%p:%p}", lp, lp->obj, lp->val);
      }
      fprintf (out, "\n");
    }
  }
  fprintf(out, "}\n");
}

static void
grow_table (eqhash_t tbl, int old_size)
{
  eqhash_entry_t **old_entries = tbl->entries;
  eqhash_entry_t **end_entries = old_entries + old_size;
  eqhash_entry_t **opp;

  ++tbl->log_size;
  tbl->entries = calloc (2*old_size, sizeof (*opp));
  for (opp = old_entries; opp < end_entries; ++opp) {
    eqhash_entry_t *lp = *opp;
    while (lp) {
      eqhash_entry_t *next = lp->next;
      eqhash_entry_t **lpp = eqhash_lookup (tbl, lp->obj);
      lp->next = *lpp;
      *lpp = lp;
      lp = next;
    }
  }
  free (old_entries);
}

static inline void
maybe_grow (eqhash_t tbl)
{
  int old_size = 1<<tbl->log_size;
  if (! tbl->freelist && 2*tbl->num_entries > old_size) 
    grow_table (tbl, old_size);
}

int
eqhash_put (eqhash_t tbl, void const *obj, void *val)
{
  eqhash_entry_t **lpp, *lp;

  maybe_grow (tbl);
  lpp = eqhash_lookup (tbl, obj);

  if ((lp = *lpp)) {
    /* Replace current value. */
    return (lp->val != val) ? (lp->val = val, 0) : 1;
  } else {
    if ((lp = tbl->freelist))
      tbl->freelist = lp->next;
    else
      lp = (eqhash_entry_t *) TABLE_ALLOC (tbl, sizeof (*lp));
    
    lp->next = 0;
    lp->obj = obj;
    lp->val = val;

    *lpp = lp;
    ++tbl->num_entries;

    return 0;
  }
}

int
eqhash_delete (eqhash_t tbl, void const *obj)
{
  eqhash_entry_t **lpp, *lp;

  lpp = eqhash_lookup (tbl, obj);
  if ((lp = *lpp)) {
    *lpp = lp->next;
    lp->next = tbl->freelist;
    tbl->freelist = lp;
    --tbl->num_entries;
    return 1;
  }
  return 0;
}

void
eqhash_forall (eqhash_t tbl, 
	       void (*func)(void const *obj, void *val, void *closure),
	       void *closure)
{
  int size = 1<<tbl->log_size;
  eqhash_entry_t **entries = tbl->entries;
  eqhash_entry_t **end_entries = entries + size;
  eqhash_entry_t **lpp;

  for (lpp = entries; lpp < end_entries; ++lpp) {
    eqhash_entry_t *lp = *lpp;
    for ( ; lp; lp = lp->next) {
      (*func) (lp->obj, lp->val, closure);
    }
  }
}
