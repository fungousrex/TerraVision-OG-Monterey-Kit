/*
 * Name: tsmEmulate_web.c
 *
 * Description: 
 *   Contains routines to emulate the old server-centric tsmApi interface
 *   in terms of the new dataset-centric (connection) interface. These
 *   are the web (http) specific routines and are private to the tsmApi.
 *
 *   If USE_HTTP is not #defined, then the dummy functions at the end are
 *   defined.  These will print error messages on stderr and return the
 *   appropriate error code.
 *
 * Function List:
 *   int tsmIsTIFFPresent_web( Purl *, Pyramid *, int, tspSymbol imgFormat )
 *   void tsmPutPyramid_web( TsmHandle *tsmh, Pyramid *pyramid, Purl * )
 *
 * Revision History:
 *   Created 12 Jan 1998 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmEmulate_web.c,v 1.9 2000/09/23 01:03:28 leei Exp $
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

#include "tsmConnect_web.h"

int
tsmIsTIFFPresent_web( Purl *basePurl, Pyramid *pyramid, int level,
		      tspSymbol imgFormat )
{
  /*static char const funcName[] = "tsmIsTIFFPresent_web";*/
  char              tiffURL[1024];
  unsigned int      magic_numb[2];  /* give overflow for libwww's padding */
  int               flag = FALSE, bytes_read;

  /* In order to check whether the TIFF file exists at the  */
  /* end of an http URL, we have to actually read the first */
  /* few bytes of the buffer returned and check that this   */
  /* coincides with the TIFF magic number.                  */
  
  sprintf( tiffURL, "%s/%d/tiles_%s.tiff", basePurl->url, level, 
	   ( imgFormat == tspLrbtRaster ) ? "lrbt" : "jpeg" );
  
  if ( tsmUrlToBuffer( NULL, tiffURL, (char *) magic_numb, 4, &bytes_read ) )
    if ( magic_numb[0] == 0x4949002a || magic_numb[0] == 0x4d4d002a )
      flag = TRUE;

  return flag;
}

#else

int
tsmIsTIFFPresent_web( Purl *basePurl, Pyramid *pyramid, int level,
		      tspSymbol imgFormat )
{
  tsmNotCompiledIn (basePurl->protocol, "tsmIsTIFFPresent");
  return FALSE;
}

#endif

void
tsmPutPyramid_web( TsmHandle *tsmh, Pyramid *pyramid, Purl *basePurl )
{
  tsmNoProtocolYet( basePurl->protocol, "tsmPutPyramid" );
}
