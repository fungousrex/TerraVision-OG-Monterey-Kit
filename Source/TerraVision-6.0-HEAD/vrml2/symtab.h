/*
 * Name: symtab.h
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
 *   $Id: symtab.h,v 1.4 2000/08/28 23:21:41 reddy Exp $
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

#ifndef _symtab_h_
#define _symtab_h_

typedef struct symtab *symtab_t;
typedef char const *symbol_t;

extern symtab_t symtab_create (int size);
extern struct istack *symtab_istack (symtab_t s);
extern void symtab_destroy (symtab_t s);

extern symbol_t sym_intern (symtab_t s, char const *str);
extern symbol_t sym_intern_nocopy (symtab_t s, char const *str);

#endif
