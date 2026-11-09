/*
 * Name: TsTileSpecExtProts.h
 *
 * Description: 
 *   External (public) prototype definitions for the tspec
 *   management interface.
 *
 * Function List:
 *
 * Revision History:
 *
 *   $Id: TsTileSpecExtProts.h,v 1.14 2000/08/28 22:15:08 reddy Exp $
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

#ifndef TSTILESPECEXTPROTS_H
#define TSTILESPECEXTPROTS_H

#include "TsTileSpecDecs.h"
#include "tsmConnection.h"

/* FUNCTION PROTOYPES */

/* Function to determine a tile-set type from the tile-set.tspec file */

int tspReadTileSetType (char *);

/* Functions to read and write pyramids */

int tspReadPyramid (char *, Pyramid *, ...);
void tspSavePyramid (char *, Pyramid *);

/* Functions to read and write pyramid name/URL associations */

int tspReadPyramidUrls (const char *, NameUrlPairs *);

/* Functions to manipulate pyramid name/URL associations */

int tspUrlToPathName (const char *, char *);
char *tspPyramidUrl (char *, NameUrlPairs *);
void tspTilePathName (const char *, int, int, int, char *);
void tspTIFFFileName (const char *, int, tspSymbol, char *);
void tsmTIFFFilename( TsmConnection, int, char * );
void tspTspecFileName (const char *, char *);

/* Functions to use "symbols" */

tspSymbol tspSymbolFromString (const char *, tspSymbol, tspSymbol);
/* This should really be const char, but Lucid ffi freaks over this.  Oh well. */
const char *tspStringFromSymbol (tspSymbol);

/* coordinate system utility functions */

int tspCalcUTMZone( double longitude );
float tspCalcCentralMeridian( int );
float tspCalcGeoidSeparation( double, double );
float tspCalcK0( void );

#endif
