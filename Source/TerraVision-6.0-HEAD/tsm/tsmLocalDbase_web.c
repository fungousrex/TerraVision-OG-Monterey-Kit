/*
 * Name: tsmLocalDbase_web.c
 *
 * Description: 
 *   Contains routines for manipulating local TSM databases, such as
 *   the PyramidUrls file. These are the web (http)
 *   specific routines and are private to the tsmApi.
 *
 *   If USE_HTTP is not #defined, then the dummy functions at the end are
 *   defined.  These will print error messages on stderr and return the
 *   appropriate error code.
 *
 * Function List:
 *   int tsmLoadPyramidDatabase_web (TsmHandle *tsmh)
 *
 * Revision History:
 *   Created 12 Jan 1998 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmLocalDbase_web.c,v 1.7 2000/08/28 22:15:56 reddy Exp $
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

#include "tsmInternal.h"

#ifdef USE_HTTP

int
tsmLoadPyramidDatabase_web (TsmHandle *tsmh)
{
  return tsmLoadPyramidDatabase_local1( tsmh, tsmh->pdbPurl->url );
}

#else

int
tsmLoadPyramidDatabase_web (TsmHandle *tsmh)
{
  tsmNotCompiledIn (tsmh->dbPurl->protocol, "tsmGetAllPyramidUrls");
  return (FALSE);
}

#endif
