/*
 * Name: eqlhashI.h
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: eqlhashI.h,v 1.1.4.1 2001/06/07 05:56:38 leei Exp $
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

#ifndef _eqlhashI_h_
#define _eqlhashI_h_

struct eqlhash_entry {
  struct eqlhash_entry *next;
  void const *obj;
  PRUint32 hash;
  void *val;
};

struct eqlhash {
  int log_size;
  int num_entries;
  eqlhash_iter_t *entries;
  eqlhash_iter_t freelist;

  eqlhash_eql eql_f;
  eqlhash_hash hash_f;
  istack *data;
};

extern eqlhash_iter_t * eqlhash_lookup (eqlhash_t tbl, 
					void const *obj, PRUint32 hash);

#endif /* _eqlhashI_h_ */
