/*
 * Name: tsmReadRowsJPEG.h
 *
 * Description: 
 *
 * Function List:
 *
 * Revision History:
 *   Created Jul 2000 by Yong Tze Chi <chi@ai.sri.com>
 *
 *   $Id: tsmReadRowsJPEG.h,v 1.4 2000/08/28 22:16:07 reddy Exp $
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

#ifndef _TSMREADROWSJPEG_H_
#define _TSMREADROWSJPEG_H_

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
} TsmReadRowsJPEG;


#ifndef CHECK_NOT_NULL0
#define CHECK_NOT_NULL0(this) if ((this)==NULL) { tsmError(funcName, "NULL instance"); return; }
#endif

#ifndef CHECK_NOT_NULL
#define CHECK_NOT_NULL(this, rv) if ((this)==NULL) { tsmError(funcName, "NULL instance"); return (rv); }
#endif


TsmReadRowsJPEG *tsmReadRowsJPEG_new(const char *filename);
void tsmReadRowsJPEG_destroy(TsmReadRowsJPEG **this);
static int tsmReadRowsJPEG_readHeader(TsmReadRowsJPEG *this);
int tsmReadRowsJPEG_read(void *buffer, int startRow, int numberOfRows, int numberOfBytes, void *closure);

int tsmParseCreateParams_JPEG(char *image_name, TsmTileParams *tile, TsmTspecParams *tspec);

#endif
