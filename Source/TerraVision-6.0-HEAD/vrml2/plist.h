/*
 * Name: plist.h
 *
 * Library:
 *   This source file is part of the vrml2 library, a library for parsing
 *   and generating VRML 97 format files. This library is supplied as part
 *   of the tsmApi distribution, available from:
 *
 *     http://www.tsmApi.com/
 *
 * Revision History:
 *   VRML 2.0 version by Chris Connolly, SRI International
 *   Updated interface by Lee Iverson, SRI International
 *
 *   $Id: plist.h,v 1.3 2000/08/28 23:21:39 reddy Exp $
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

#ifndef _plist_h_
#define _plist_h_

#ifdef _symtab_h_
 typedef symbol_t plist_sym_t;
#else
 typedef unsigned int plist_sym_t;
#endif

typedef struct plist_table *plist_t;

plist_t plist_create (int size);
void *  plist_get (plist_t tbl, void const *obj, plist_sym_t sym);
void    plist_put (plist_t tbl, void const *obj, plist_sym_t sym, void *val);
void    plist_delete (plist_t tbl, void const *obj, plist_sym_t sym);
void    plist_destroy (plist_t tbl);
void    plist_forall (plist_t tbl, 
		      void (*func)(void const *obj, plist_sym_t sym, void *val,
				   void *closure),
		      void *closure);

#endif
