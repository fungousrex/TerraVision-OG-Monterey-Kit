/*
 * Name: tsmReadRowsDOQ.h
 *
 * Description: 
 *
 * Function List:
 *
 * Revision History:
 *   Created Jul 2000 by Yong Tze Chi <chi@ai.sri.com>
 *
 *   $Id: tsmReadRowsDOQ.h,v 1.2 2000/08/28 22:16:04 reddy Exp $
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

#ifndef _TSMREADROWSDOQ_H_
#define _TSMREADROWSDOQ_H_

#include <stdio.h>
#include <stdlib.h>
#include <jpeg/jpeglib.h>
#include "tsmInternal.h"

typedef struct {
  FILE *fin;
  struct jpeg_decompress_struct jinfo;
  struct jpeg_error_mgr jerr;

  int imageWidth, imageHeight;
  int compsPerPixel;
  int bytesPerComp;
} TsmReadRowsDOQ;


#ifndef CHECK_NOT_NULL0
#define CHECK_NOT_NULL0(this) if ((this)==NULL) { tsmError(funcName, "NULL instance"); return; }
#endif

#ifndef CHECK_NOT_NULL
#define CHECK_NOT_NULL(this, rv) if ((this)==NULL) { tsmError(funcName, "NULL instance"); return (rv); }
#endif


TsmReadRowsDOQ *tsmReadRowsDOQ_new(const char *filename);
void tsmReadRowsDOQ_destroy(TsmReadRowsDOQ **this);
static int tsmReadRowsDOQ_readHeader(TsmReadRowsDOQ *this);
int tsmReadRowsDOQ_read(void *buffer, int startRow, int numberOfRows, int numberOfBytes, void *closure);

static char *getBaseName(char *buffer, const char *filename);
static FILE *tsmReadRowsDOQ_getInfoFile(char *filename);
static int tsmReadRowsDOQ_parseInfoFile(char *filename, TsmTileParams *tile, TsmTspecParams *tspec);
int tsmParseCreateParams_DOQ(char *image_name, TsmTileParams *tile, TsmTspecParams *tspec);

#define DOQ_HEADER_SENTINEL   "BEGIN_USGS_DOQ_HEADER"
#define DOQ_TRAILER_SENTINEL  "END_USGS_HEADER"
#define DOQ_QUADRANGLE_NAME   "QUADRANGLE_NAME"
#define DOQ_COORDINATE_ZONE   "COORDINATE_ZONE"
#define DOQ_HORIZONTAL_UNIT   "HORIZONTAL_UNIT"
#define DOQ_XY_ORIGIN         "XY_ORIGIN"
#define DOQ_SW_QUAD_CORNER_XY "SW_QUAD_CORNER_XY"


#endif
