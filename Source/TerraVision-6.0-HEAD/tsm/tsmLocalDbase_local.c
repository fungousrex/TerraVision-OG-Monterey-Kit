/*
 * Name: tsmLocalDbase_dpss.c
 *
 * Description: 
 *   This file contains the localdisk specific functions for manipulating
 *   local TSM databases on DPSS's (referred to as DSM's).
 *
 * Function List:
 *   int tsmLoadPyramidDatabase_local (TsmHandle *tsmh)
 *   int tsmLoadPyramidDatabase_local1 (TsmHandle *tsmh, char *uri)
 *   void tsmSavePyramidDatabase_local (TsmHandle *tsmh, Purl *basePurl)
 *   void tsmSavePyramidDatabase_local1 (TsmHandle *tsmh, char *uri)
 *
 * Revision History:
 *   Created 12 Jan 1998 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmLocalDbase_local.c,v 1.6 2000/08/28 22:15:56 reddy Exp $
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

int
tsmLoadPyramidDatabase_local (TsmHandle *tsmh)
{
  return (tsmLoadPyramidDatabase_local1 (tsmh, tsmh->pdbPurl->uri));
}

int
tsmLoadPyramidDatabase_local1 (TsmHandle *tsmh, char *url)
{
  /* Read the Pyramid URLs from local disk or an http URL. */
  tsmh->pyramidUrls = tsmReadNameUrlIds (url);
  return ( tsmh->pyramidUrls != NULL );
}

void
tsmSavePyramidDatabase_local (TsmHandle *tsmh, Purl *basePurl)
{
  tsmSavePyramidDatabase_local1 (tsmh, basePurl->uri);
}

void
tsmSavePyramidDatabase_local1 (TsmHandle *tsmh, char *uri)
{
  tsmWriteNameUrlIds (uri, tsmh->pyramidUrls);
}

