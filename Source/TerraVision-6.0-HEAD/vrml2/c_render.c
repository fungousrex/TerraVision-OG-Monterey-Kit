/*
 * Name: c_render.c
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
 *   $Id: c_render.c,v 1.15 2000/08/28 23:21:33 reddy Exp $
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

#include "vrml2.h"
#include "c_nodes.h"

#include <stdio.h>
#include <stddef.h>

int
vsgcSetupRenderGL (void)
{
  fprintf( stderr, "No rendering support - use vsgcSetupCustomRender()\n" );
  return FALSE;
}

void
vsgcSetupCustomRender( vsgCallback_t callback )
{
  vsgDefault( vsgfRender, callback );
}
