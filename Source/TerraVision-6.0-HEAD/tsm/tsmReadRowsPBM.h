/*
 * Name: tsmReadRowsPBM.h
 *
 * Description: 
 *
 * Function List:
 *
 * Revision History:
 *   Created Jul 2000 by Yong Tze Chi <chi@ai.sri.com>
 *
 *   $Id: tsmReadRowsPBM.h,v 1.3 2000/08/28 22:16:08 reddy Exp $
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

#ifndef _TSMREADROWSPBM_H_
#define _TSMREADROWSPBM_H_

typedef struct {
  FILE *fin;
  int fileType;
  int imageWidth, imageHeight;
  int compsPerPixel;
  int bytesPerComp;
  int maxValue;
  long dataOffset;
  long *rowOffsets;
} TsmReadRowsPBM;

TsmReadRowsPBM *tsmReadRowsPBM_new(const char *filename);
void tsmReadRowsPBM_destroy(TsmReadRowsPBM **this);
int tsmReadRowsPBM_read(void *buffer, int startRow, int numberOfRows,
			int numberOfBytes, void *closure);

int tsmParseCreateParams_PPM( char *, TsmTileParams *, TsmTspecParams * );
int tsmParseCreateParams_PGM( char *, TsmTileParams *, TsmTspecParams * );


#ifndef CHECK_NOT_NULL0
#define CHECK_NOT_NULL0(this) if ((this)==NULL) { tsmError(funcName, "NULL instance"); return; }
#endif

#ifndef CHECK_NOT_NULL
#define CHECK_NOT_NULL(this, rv) if ((this)==NULL) { tsmError(funcName, "NULL instance"); return (rv); }
#endif

#endif
