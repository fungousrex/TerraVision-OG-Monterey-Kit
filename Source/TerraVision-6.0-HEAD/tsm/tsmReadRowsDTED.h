/*
 * Name: tsmReadRowsDTED.h
 *
 * Description: 
 *
 * Function List:
 *
 * Revision History:
 *   Created Jul 2000 by Yong Tze Chi <chi@ai.sri.com>
 *
 *   $Id: tsmReadRowsDTED.h,v 1.4 2000/08/28 22:16:05 reddy Exp $
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

#ifndef _TSMREADROWSDTED_H_
#define _TSMREADROWSDTED_H_

#include <stdio.h>

#define INT2 short int

typedef enum {
  DTED0 = 0, DTED1, DTED2
} DTEDLevel;


typedef struct {
    /* User Header Label (UHL) */
    double blockOrigin[2];
    double blockSpacing[2];
    double absVertAccuracy;
    char   security[3];
    int    dimension[2];

    /* Data Set Identification (DSI) */
    DTEDLevel level;
    int    editionNumber;
    char   matchVersion;
    char   maintenanceDate[4];
    char   matchDate[4];
    char   maintenenceDesc[4];
    char   producerCode[8];
    char   productSpec[9];
    int    productSpecAmendment;
    char   productSpecDate[4];
    char   verticalDatum[3];
    char   horizontalDatum[5];
    char   compileDate[4];
    double coordOrg[2];
    double coordSW[2];
    double coordNW[2];
    double coordNE[2];
    double coordSE[2];
    double orientationAngle;
    double pointSpacing[2];
    int    numValidPoint[2];

    /* Internal use */
    FILE *fin;
    long dataOffset;

} TsmReadRowsDTED;

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define MINTODEG 0.0163934426229508196721311475409836
#define SECTODEG 2.7777777777777777777777777777778e-4

#define FREAD(buffer, size, count, fhandle) \
  if (fread((buffer), (size), (count), (fhandle))!=(count)) {\
    tsmError(funcName, "Error encountered while reading file.\n"); \
    return FALSE; \
  }

TsmReadRowsDTED *tsmReadRowsDTED_new(char const *filename);
void tsmReadRowsDTED_destroy(TsmReadRowsDTED **this);
int tsmReadRowsDTED_read(void *buffer, int startRow, int numberOfRows,
			 int numberOfBytes, void *closure);

int tsmParseCreateParams_DTED(char *filename, TsmTileParams *tile, TsmTspecParams *tspec);


#endif
