/*
 * Name: eqhashI.h
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: eqhashI.h,v 1.1 2000/09/29 22:50:31 leei Exp $
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

#ifndef _eqhashI_h_
#define _eqhashI_h_

#include <prtypes.h>

typedef struct eqhash_entry {
  struct eqhash_entry *next;
  void const *obj;
  void *val;
} eqhash_entry_t;

struct eqhash {
  int log_size;
  int num_entries;
  eqhash_entry_t **entries;
  eqhash_entry_t *freelist;

  istack *data;
};

/* Multiplicative hash function. */
#define EQHASH_A ((PRUint32)2654435769U)
#define EQHASH(k) (EQHASH_A*((PRUint32)(k)))

extern PRUint32 eqhash_hash (eqhash_t tbl, void const *obj);
extern eqhash_entry_t ** eqhash_lookup (eqhash_t tbl, void const *obj);

#endif /* _eqhashI_h_ */
