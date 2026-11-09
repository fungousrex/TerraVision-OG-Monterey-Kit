/*
 * Name: eqlhash_inline.c
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes. This file implements a generic hash table.
 *
 * Revision History:
 *   $Id: eqlhash_inline.c,v 1.2.6.1 2001/06/07 05:56:38 leei Exp $
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

#include "config.h"

#define NO_eqlhash_INLINE
#include "eqlhash.h"

#include "istack.h"

/* Provide externed definitions of inlined functions. */
#define exinline
#include "eqlhash_inline.h"
