/*
 * Name: tsmEmulate_dpss.c
 *
 * Description: 
 *   This file contains the localdisk specific functions for the 
 *   emulation layer of the new tsmApi. This provides emulation of
 *   the old server-style calls that the tsmApi used to use.
 *
 * Function List:
 *   void tsmPutPyramid_local (TsmHandle *tsmh, Pyramid *pyramid, Purl *)
 *   void tsmPutPyramid_local1 (TsmHandle *tsmh, Pyramid *pyramid, char *uri)
 *   int tsmIsTIFFPresent_local( Purl *basePurl, Pyramid *, int, tspSymbol )
 *
 * Revision History:
 *   Created 12 Jan 1998 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmEmulate_local.c,v 1.7 2000/08/28 22:15:44 reddy Exp $
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

TsmConnection tsmGetConnectionFromTsmHandle(TsmHandle *tsmh, Pyramid *pyramid);

void
tsmPutPyramid_local (TsmHandle *tsmh, Pyramid *pyramid, Purl *basePurl)
{
  tsmPutPyramid_local1 (tsmh, pyramid, basePurl->uri);
}

void
tsmPutPyramid_local1 (TsmHandle *tsmh, Pyramid *pyramid, char *uri)
{
  tspSavePyramid (uri, pyramid);
}

/*
 * Synopsis:
 *      int tsmIsTIFFPresent_local( Purl *, Pyramid *, int, tspSymbol )
 *
 * Description:
 *      Checks to see if the image tiles for a particular pyramid level
 *      are stored as a single blocked TIFF file, or as a collection of
 *      raw .tile files.
 *
 * Externals: None
 *
 * Returns: TRUE if images stored in a TIFF, FALSE if stored as .tile files
 *
 * Author: Martin Reddy
 * 
 * Date: 7 July 1997
 *
 */

int
tsmIsTIFFPresent_local( Purl *basePurl, Pyramid *pyramid, int level,
			tspSymbol imgFormat )
{
  /*static char const funcName[] = "tsmIsTIFFPresent_local";*/
  char              tiffFileName[1024];
  FILE              *filetest;
  int               flag = FALSE;

  tspTIFFFileName( basePurl->uri, level, imgFormat, tiffFileName );

  if ( ( filetest = fopen( tiffFileName, "r" ) ) != NULL ) {
    flag = TRUE;
    fclose( filetest );
  } else
    flag = FALSE;

  return flag;
}

