/*
 * Name: plist.c
 *
 * Description: a generic property list system.
 *
 * Library:
 *   This source file is part of the vrml2 library, a library for parsing
 *   and generating VRML97 format files. This library is supplied as part
 *   of the tsmApi distribution, available from:
 *
 *     http://www.tsmApi.com/
 *
 * Revision History:
 *   VRML 2.0 version by Chris Connolly, SRI International
 *   Updated interface by Lee Iverson, SRI International
 *
 *   $Id: plist.c,v 1.10 2000/08/28 23:21:39 reddy Exp $
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
#include <prtypes.h>

#include "plist.h"

#include <pthr/istack.h>

typedef struct entry {
  struct entry *next;
  plist_sym_t sym;
  void *val;
} plist_entry_t;

typedef struct plist_list {
  struct plist_list *next;
  void const *obj;
  plist_entry_t *list;
} plist_list_t;

struct plist_table {
  int log_size;
  int num_plists;
  plist_list_t **plists;
  plist_list_t *freelist;
  plist_entry_t *freeentry;

  istack *data;
};

#define TABLE_ALLOC(tbl,size) istack_alloc(tbl->data, size)

plist_t
plist_create (int size)
{
  plist_t tbl = malloc (sizeof (*tbl));
  int log_size = -1;

  if (! size) size = 128;
  for ( ; size; size >>= 1)
    ++log_size;
  
  tbl->log_size = log_size;
  tbl->num_plists = 0;
  tbl->plists = calloc (1<<log_size, sizeof (tbl->plists[0]));
  tbl->freelist = 0;
  tbl->freeentry = 0;

  tbl->data = istack_create (64*sizeof(plist_entry_t));

  return tbl;
}

void
plist_destroy (plist_t tbl)
{
  istack_destroy (tbl->data);
  free (tbl->plists);
  free (tbl);
}

/* Multiplicative hash function. */
#define A (2654435769U)
#define HASH(k) (A*((PRUint32)k))

static PRUint32
hash_val (plist_t tbl, void const *obj)
{
  PRUint32 hash = HASH (obj);
  hash >>= (32 - tbl->log_size);
  return hash;
}

static plist_list_t **
plist_lookup (plist_t tbl, void const *obj)
{
  plist_list_t **pp, *p;
  PRUint32 hash = hash_val (tbl, obj);

  for (pp = &tbl->plists[hash]; (p = *pp); pp = &p->next)
    if (p->obj == obj)
      return pp;
  
  return pp;
}

static plist_entry_t **
plist_assq (plist_list_t *lp, plist_sym_t sym)
{
  plist_entry_t **epp, *ep;

  for (epp = &lp->list; (ep = *epp); epp = &ep->next)
    if (ep->sym == sym)
      return epp;
  
  return epp;
}

void *
plist_get (plist_t tbl, void const *obj, plist_sym_t sym)
{
  plist_list_t *lp, **lpp = plist_lookup (tbl, obj);
  plist_entry_t *ep, **epp = ((lp = *lpp) ? plist_assq (lp, sym) : NULL);
  return (epp && (ep = *epp)) ? ep->val : NULL;
}

static void
maybe_grow (plist_t tbl)
{
  int old_size = 1<<tbl->log_size;
  if (! tbl->freelist && 2*tbl->num_plists > old_size) {
    plist_list_t **old_plists = tbl->plists;
    plist_list_t **end_plists = old_plists + old_size;
    plist_list_t **opp;

    ++tbl->log_size;
    tbl->plists = calloc (2*old_size, sizeof (*opp));
    for (opp = old_plists; opp < end_plists; ++opp) {
      plist_list_t *lp = *opp;
      while (lp) {
	plist_list_t *next = lp->next;
	plist_list_t **lpp = plist_lookup (tbl, lp->obj);
	lp->next = *lpp;
	*lpp = lp;
	lp = next;
      }
    }
    free (old_plists);
  }
}

static plist_entry_t *
new_entry (plist_t tbl, plist_sym_t sym, void *val)
{
  plist_entry_t *ep = tbl->freeentry;
  if (ep)
    tbl->freeentry = ep->next;
  else
    ep = (plist_entry_t *) TABLE_ALLOC (tbl, sizeof (*ep));

  ep->next = 0;
  ep->sym = sym;
  ep->val = val;
  return ep;
}

void
plist_put (plist_t tbl, void const *obj, plist_sym_t sym, void *val)
{
  plist_list_t **lpp, *lp;

  maybe_grow (tbl);
  lpp = plist_lookup (tbl, obj);

  if ((lp = *lpp)) {
    /* Replace current value. */
    plist_entry_t *ep, **epp = plist_assq (lp, sym);
    if ((ep = *epp))
      ep->val = val;
    else
      *epp = new_entry (tbl, sym, val);
  } else {
    if ((lp = tbl->freelist))
      tbl->freelist = lp->next;
    else
      lp = (plist_list_t *) TABLE_ALLOC (tbl, sizeof (*lp));
    
    lp->next = 0;
    lp->obj = obj;
    lp->list = new_entry (tbl, sym, val);

    *lpp = lp;
    ++tbl->num_plists;
  }
}

void
plist_delete (plist_t tbl, void const *obj, plist_sym_t sym)
{
  plist_list_t **lpp, *lp;

  lpp = plist_lookup (tbl, obj);
  if ((lp = *lpp)) {
    plist_entry_t *ep, **epp = plist_assq (lp, sym);
    if ((ep = *epp)) {
      *epp = ep->next;
      ep->next = tbl->freeentry;
      tbl->freeentry = ep;
      if (epp == &lp->list) {
	*lpp = lp->next;
	lp->next = tbl->freelist;
	tbl->freelist = lp;
	--tbl->num_plists;
      }
    }
  }
}

void
plist_forall (plist_t tbl, 
	      void (*func)(void const *obj, plist_sym_t sym, void *val,
			   void *closure),
	      void *closure)
{
  int size = 1<<tbl->log_size;
  plist_list_t **plists = tbl->plists;
  plist_list_t **end_plists = plists + size;
  plist_list_t **lpp;

  for (lpp = plists; lpp < end_plists; ++lpp) {
    plist_list_t *lp = *lpp;
    for ( ; lp; lp = lp->next) {
      plist_entry_t *ep = lp->list;
      for ( ; ep; ep = ep->next)
	(*func) (lp->obj, ep->sym, ep->val, closure);
    }
  }
}
