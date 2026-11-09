/*
 * Name: tsmReadRowsTIFF.h
 *
 * Description: 
 *
 * Function List:
 *
 * Revision History:
 *   Created Jul 2000 by Yong Tze Chi <chi@ai.sri.com>
 *
 *   $Id: tsmReadRowsTIFF.h,v 1.2 2000/08/28 22:16:10 reddy Exp $
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

#ifndef _TSMREADROWSTIFF_H_
#define _TSMREADROWSTIFF_H_

#include "tsmInternal.h"

typedef struct {
  TIFF    *tiffFile;
  int     currentRow;
  uint32  width, height;
  uint16  compsPerPixel;
  uint16  bytesPerComp;
  uint16  photometric;
  tsize_t scanline_size;
  uint16  *r, *g, *b;
} TsmReadRowsTIFF;

TsmReadRowsTIFF *tsmReadRowsTIFF_new( const char *filename );
void tsmReadRowsTIFF_destroy( TsmReadRowsTIFF **this );
int tsmReadRowsTIFF_read( void *buffer, int startRow, int numberOfRows,
			  int numberOfBytes, void *closure );

int tsmParseCreateParams_TIFF( char *image_name, TsmTileParams *tile,
			       TsmTspecParams *tspec );


#endif
