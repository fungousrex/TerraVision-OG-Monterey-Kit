/*
 * Name: symtab.c
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
 *   $Id: symtab.c,v 1.10 2000/08/28 23:21:41 reddy Exp $
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

#include "symtab.h"

#include <pthr/istack.h>

typedef struct symtab_entry {
  struct symtab_entry *next;
  symbol_t sym;
} *symtab_entry;

struct symtab {
  int log_size;
  int num_entries;
  symtab_entry *entries;
  istack *ist;
};

#define SYMTAB_ISTACK(st) ((st)->ist)

symtab_t
symtab_create (int size)
{
  symtab_t symtab = malloc (sizeof (*symtab));
  int log_size = -1;

  if (! size) size = 512;
  for ( ; size; size >>= 1)
    ++log_size;
  
  symtab->log_size = log_size;
  symtab->num_entries = 0;
  symtab->entries = calloc (1<<log_size, sizeof (*symtab->entries));
  
  symtab->ist = istack_create (2048);

  return symtab;
}

istack *
symtab_istack (symtab_t s)
{
  return s->ist;
}

void
symtab_destroy (symtab_t symtab)
{
  free (symtab->entries);
  istack_destroy (symtab->ist);
  free (symtab);
}

/* Multiplicative hash function. */
#define A (2654435769U)
#define HASH(k) (A*((PRUint32)k))

static PRUint32
hash_val (symtab_t symtab, char const *str)
{
  PRUint32 hash;
  PRUint32 k = 0;
  int shft = 0;

  for ( ; *str; ++str) {
    k ^= (*(unsigned char *)str)<<shft;
    shft += 8;
    shft %= 32;
  }

  hash = HASH (k);
  hash >>= (32 - symtab->log_size);
  return hash;
}

static symtab_entry *
symtab_lookup (symtab_t symtab, char const *str)
{
  symtab_entry *entpp, entp;
  PRUint32 hash;

  hash = hash_val (symtab, str);

  for (entpp = &symtab->entries[hash]; (entp = *entpp); entpp = &entp->next) {
    if (! strcmp (entp->sym, str))
      return entpp;
  }
  return entpp;
}

static void
maybe_grow (symtab_t symtab)
{
  int old_size = 1<<symtab->log_size;
  if (2*symtab->num_entries > old_size) {
    symtab_entry *old_entries = symtab->entries;
    symtab_entry *end_entries = old_entries + old_size;
    symtab_entry *opp;

    ++symtab->log_size;
    symtab->entries = calloc (2*old_size, sizeof (*opp));
    
    for (opp = old_entries; opp < end_entries; ++opp) {
      symtab_entry ep = *opp;
      while (ep) {
	symtab_entry next = ep->next;
	symtab_entry *epp = symtab_lookup (symtab, ep->sym);
	ep->next = *epp;
	*epp = ep;
	ep = next;
      }
    }
    free (old_entries);
  }
}

symbol_t
sym_intern1 (symtab_t symtab, char const *str, int copy)
{
  symtab_entry *epp, ep;

  maybe_grow (symtab);
  epp = symtab_lookup (symtab, str);

  if (! (ep = *epp)) {
    ep = (symtab_entry) istack_alloc (SYMTAB_ISTACK (symtab), sizeof (*ep));
    ep->sym = 
      (copy 
       ? (symbol_t) istack_copy0 (SYMTAB_ISTACK (symtab), str, strlen (str))
       : (symbol_t) str);
    ep->next = *epp;
    *epp = ep;
    ++symtab->num_entries;
  }

  return ep->sym;
}

symbol_t
sym_intern_nocopy (symtab_t symtab, char const *str)
{
  return sym_intern1 (symtab, str, 0);
}

symbol_t
sym_intern (symtab_t symtab, char const *str)
{
  return sym_intern1 (symtab, str, 1);
}
