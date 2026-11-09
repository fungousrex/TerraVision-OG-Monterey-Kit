/*
 * Name: TsTileSpecIntProts.h
 *
 * Description: 
 *   Internal prototype definitions for the tspec management interface
 *
 * Function List:
 *
 * Revision History:
 *
 *   $Id: TsTileSpecIntProts.h,v 1.23 2000/08/28 22:15:11 reddy Exp $
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

#ifndef TSTILESPECINTPROTS_H
#define TSTILESPECINTPROTS_H

#include "tsmInternal.h"

int tspSaveTspec (char *, Pyramid *);
int tspReadTspec (char *, Pyramid *);
int tspReadTspecStream( FILE *tspecStream, Pyramid *pyramid );

Pyramid * tspCreatePyramid( char *, tspSymbol, TsmTspecParams, TsmTileParams );

int tspReadLineFromTspec( FILE *, char * );
void tspExtractTokenAndData( char *, char **, char ** );
char *tspExtractNthString( char *, char *, int );
int tspExtractNthInt( char *, char *, int );
double tspExtractNthFloat( char *, char *, int );
tspSymbol tspExtractNthSymbol( char *, char *, int );
char *tspExtractNthStrdup( char *, char *, int );
char *tspExtractAllStrdup( char * );
void tspStripQuotes( char *string );

int tspMatchPyramidName (char *, NameUrlPairs *);

int tspReadOldPyramid (char *, Pyramid *, int);
int tspReadPyramids (int, char **, NameUrlPairs *, Pyramid *);

int tspReadIdentityTspec (char *, IdentityTspec *);
int tspReadIdentityTspec1 (char *, IdentityTspec *);
int tspReadTileTspec (char *, TileTspec *);
int tspReadTileTspec1 (char *, TileTspec *);
int tspReadPixelTspec (char *, PixelTspec *);
int tspReadPixelTspec1 (char *, PixelTspec *);
int tspReadImageTransformationTspec (char *, ImageTransformationTspec *);
int tspReadImageTransformationTspec1 (char *, ImageTransformationTspec *);
int tspReadProjectionTspec (char *, ProjectionTspec *);
int tspReadProjectionTspec1 (char *, ProjectionTspec *);
int tspReadCoordTspec (char *, CoordTspec *);
int tspReadCoordTspec1 (char *, CoordTspec *);
int tspReadSimpleTileSet (char *, SimpleTileSet *);
int tspReadPyramidTspec (char *, Pyramid *);
int tspReadPyramidTspec1 (char *, Pyramid *);
int tspReadSimpleTspec (char *, SimpleTileSet *);
int tspReadSimpleTspec1 (char *, SimpleTileSet *);

char *tspAppendFileNameToPathName (const char *, const char *);
int tspStringIndexFromSet (const char *, int, int, const char **, int);

int tspReadListOfStrings (FILE *, int *, char ***stringArray, const char *, const char *);
int tspReadListOfQuotedStrings (FILE *, u_int *, char ***stringArray, const char *, const char *);

void tspPrintListOfStrings (FILE *, int, const char **, const char *, const char *);
void tspPrintListOfQuotedStrings (FILE *, int, char **, const char *, const char *);

tspSymbol tspReadSymbol (FILE *, const char *, int, ...);
int tspReadListOfSymbols (FILE *, u_int *, tspSymbol **, const char *, const char *, int , ...);

void tspPrintSymbol (FILE *, const tspSymbol, const char *);
void tspPrintListOfSymbols (FILE *, const int, const tspSymbol *, const char *, const char *);

void tspPrintFloatVector (FILE *, int, float *);

void tspTileFileName (int, int, char *);
void tspLevelTileFileName (int, int, int, char *);

int tspIsFileAValidTspec( char * );

FILE *tspOpenForWrite (const char *, const char *);
void tspCloseForWrite (FILE *);

int tspError (const char *, const char *);
int tspErrorAndClose (const char *, const char *, FILE *);

char *ftoa( double, char * );

void tspFreeNameUrlPairs (NameUrlPairs *);
int tspMatchName (const char *, NameUrlPairs *);
int tspReadNameUrlPairs (const char *, const char *, NameUrlPairs *);
int tspMatchName (const char *, NameUrlPairs *);
char *tspNameToUrl (const char *, NameUrlPairs *);
void tspSaveNameUrlPairs (const char *, const char *, NameUrlPairs *);

void tspLFToSpace( char * );

float tspLinearInterpolate( float, float, float, float, float );
float tspBiLinearInterpolate( float, float, float, float, float, float,
    float, float, float, float, float, float, float, float );

void tspParseCharArray( char *inbuffer, char *outbuffer, u_int *length, 
			charPtr **array );
void tspParseFloatArray( char *inbuffer, char *outbuffer, int *length, 
			 float *array, u_int expected_size );
void tspParseIntArray( char *inbuffer, char *outbuffer, u_int *length, 
		       int **array, u_int expected_size );
void tspParseSymbolArray( char *inbuffer, char *outbuffer, u_int *length, 
			  tspSymbol **array );

int tspReadPyramid( char *pathName, Pyramid *pyramid, ... );

#endif
