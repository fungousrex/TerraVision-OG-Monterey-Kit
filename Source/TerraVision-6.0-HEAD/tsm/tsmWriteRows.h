/*
 * Name: tsmWriteRows.h
 *
 * Description: 
 *
 * Function List:
 *
 * Revision History:
 *   Created Jul 2000 by Yong Tze Chi <chi@ai.sri.com>
 *
 *   $Id: tsmWriteRows.h,v 1.7.6.1 2002/03/15 18:01:35 reddy Exp $
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

#ifndef _TSMWRITEROWS_H_
#define _TSMWRITEROWS_H_

#include <tsm/tsm.h>

typedef struct {
  TsmConnection connection;
  Pyramid *pyramid;
  int overlap[2];
  int tileSize[2];
  int tileTotalPixel[2];
  char **buffer;
  uchar *tileBuffer;
  uchar *convBuffer;
  int levels;

  int compsPerPixel;
  int bytesPerComp;
  int bytesPerPixel;
  int outBytesPerComp;
  int lrbt;
  int resample_error;
} TsmWriteRows;

#ifndef CHECK_NOT_NULL0
#define CHECK_NOT_NULL0(this) if ((this)==NULL) { tsmError(funcName, "NULL instance"); return; }
#endif

#ifndef CHECK_NOT_NULL
#define CHECK_NOT_NULL(this, rv) if ((this)==NULL) { tsmError(funcName, "NULL instance"); return (rv); }
#endif


TsmWriteRows *tsmWriteRows_new(TsmConnection connection, TsmTileParams *tile);
void tsmWriteRows_destroy(TsmWriteRows **this);
int tsmWriteRows_write(int level, const void *buffer, int startRow,
  int numberOfRows, int numberOfBytes, void *closure);
static int tsmWriteRows_writeTiles(TsmWriteRows *this, const unsigned char *buffer, int startRow, int numberOfRows, int level);
static FILE *tsmWriteRows_createTileFile(TsmWriteRows *this, int tx, int ty, int level, unsigned char *buffer);
static int tsmWriteRows_writeTile(TsmWriteRows *this, int tx, int ty, int level, const unsigned char *buffer);
static int tsmWriteRows_convertTileFormat(TsmWriteRows *this, const void *inBuffer);



#endif
