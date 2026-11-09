/*
 * Name: tsmIntProts.h
 *
 * Description: 
 *   Prototype definitions for all internal API function calls.
 *
 * Function List:
 *
 * Revision History:
 *
 *   $Id: tsmIntProts.h,v 1.48.2.3 2002/03/14 05:22:57 reddy Exp $
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

#ifndef TSMAPIINTPROTS_H
#define TSMAPIINTPROTS_H

/* Connection functions */

int tsmCheckConnectMode( const char *, TsmConnection, TsmAccessMode, int );

int tsmCheckTileFormat( TsmConnection, TsmTileFormat );
int tsmCheckTIFFFormat( TsmConnection, int );

int tsmGenerateUniqueSetID( void );

void tsmProgressReport( TsmConnection, const char *fmt, ... );

/* Tspec reading/writing functions */

Pyramid *tsmReadTspec_local( char *, TsmConnectionParams );
Pyramid *tsmReadTspec_web( Purl *, TsmConnectionParams );
Pyramid *tsmReadTspec_wms( char *, TsmConnectionParams );
Pyramid *tsmReadTspecNEW_web( char * );
Pyramid *tsmReadTspecOLD_web( char * );

int tsmDoesTspecExist_local( char * );

void tsmEnsureGeoNameSet( Pyramid *pyramid );

/* Global Dataset Database functions */

NameUrlIds *tsmReadServer_local( Purl const *purl );
NameUrlIds *tsmReadServer_web( Purl const *purl );
NameUrlIds *tsmReadServer_wms( Purl const *purl );
NameUrlIds *tsmReadServer_dpss( Purl const *purl );

int tsmReadDatabase_local( TsmDatabase *, char * );
int tsmWriteDatabase_local( TsmDatabase *, char * );
int tsmNewDataSetInstance( TsmDataSet * );
void tsmDeleteDataSetInstance( TsmDataSet *, int );
int tsmNewDatabaseDataSet( TsmDatabase * );
void tsmDeleteDatabaseDataSet( TsmDatabase *, int );
int tsmNewDatabaseServer( TsmDatabase * );
void tsmDeleteDatabaseServer( TsmDatabase *, int );
int tsmNewDatabaseGeoSet( TsmDatabase * );
void tsmFreeDataSetContents( TsmDataSet * );
void tsmFreeGeoSetContents( TsmGeoSet * );
void tsmSortGeoSets( TsmDatabase * );
int tsmNewDatabaseModel( TsmDatabase * );
int tsmNewDatabaseMap( TsmDatabase * );
int tsmAddGeoSetModel( TsmGeoSet *, char const *);
int tsmAddGeoSetMap( TsmGeoSet *, char const * );
int tsmRemoveMapFromDatabase( TsmDatabase *database, char *mapname );
int tsmRemoveModelFromDatabase( TsmDatabase *database, char *modelname );
int tsmNewModelLocation( TsmModel *, char const * );
int tsmNewMapLocation( TsmMap *, char const * );

/* Internal VRML related functions */

void tsmDestroyProtoWrl( TsmConnection conn );

/* Tile generation routines */

int tsmCreateLevelTiles_LASRGB(TsmConnection, const char *,TsmTileParams,int);
int tsmCreateLevelTiles_LASELEV(TsmConnection, const char *,TsmTileParams,int);
int tsmCreateLevelTiles_LASGREY(TsmConnection, const char *,TsmTileParams,int);

int tsmParseCreateParams_LAS( char *, TsmTileParams *, TsmTspecParams * );

#define TSM_DOWNSAMPLE_TOP_BORDER    (1 << 0)
#define TSM_DOWNSAMPLE_BOTTOM_BORDER (1 << 1)
#define TSM_DOWNSAMPLE_LEFT_BORDER   (1 << 2)
#define TSM_DOWNSAMPLE_RIGHT_BORDER  (1 << 3)
#define TSM_DOWNSAMPLE_FLOAT         (1 << 4)
#define TSM_DOWNSAMPLE_SIGNED        (1 << 5)
#define TSM_DOWNSAMPLE_DEM           (1 << 6)
#define TSM_DOWNSAMPLE_GAUSSIAN      (1 << 7)

void tsmDownSample_image (void const * const *in, 
			  int in_w, int in_h,
			  int bytes_per_comp, int comps_per_pixel,
			  int flags,
			  void * const *out);

void tsmDownSample_block (void const *in, 
			  int in_w, int in_h, int in_span,
			  int bytes_per_comp, int comps_per_pixel,
			  int flags,
			  void *out, int out_span);

int tsmDownSample_inmemory (int width, int height, int nlevels, int tile_size,
			    int bytes_per_comp, int comps_per_pixel,
			    int flags,
			    int (*read_rows) (void *buf, int w,
					      int row, int n, void *closure),
			    void *rclosure,
			    int (*write_rows) (int level,
					       void const *buf, int w, 
					       int row, int n, void *closure),
			    void *wclosure);

int tsmDownSample_LAS(TsmConnection,const char *,const char *,TsmTileParams,int );
int tsmDownSample_LAS_gaussian(TsmConnection,char *, char *, int,int,int,int,double,double,int);
int tsmDownSample_LAS_psample(TsmConnection,char *, char *, int, int, int,int,double,double,int);

/* Local TSM database manipulations functions */

NameUrlIds *tsmReadNameUrlIds( const char * );
void tsmWriteNameUrlIds( const char *, NameUrlIds * );

int tsmLoadPyramidDatabase_dsm( TsmHandle * );
int tsmLoadPyramidDatabase_local( TsmHandle * );
int tsmLoadPyramidDatabase_local1( TsmHandle *, char * );
int tsmLoadPyramidDatabase_web( TsmHandle * );

void tsmSavePyramidDatabase_local( TsmHandle *, Purl * );
void tsmSavePyramidDatabase_local1( TsmHandle *, char * );

PyramidInfo *tsmGetPyramidInfo_dsm( TsmHandle *, const char *, Purl * );
PyramidInfo *tsmGetPyramidInfo_local( TsmHandle *, const char *, Purl * );
PyramidInfo *tsmGetPyramidInfo_web( TsmHandle *, const char *, Purl * );

/* internal functions for the emulation of the old tsmApi interface  */

TsmConnection tsmGetConnectionFromTsmHandle( TsmHandle *, Pyramid *);

int tsmOpen_dsm( TsmHandle *, Purl * );
void tsmClose_dsm( TsmHandle * );

void tsmPutPyramid_iss (TsmHandle *, Pyramid *, Purl *);
void tsmPutPyramid_local (TsmHandle *, Pyramid *, Purl *);
void tsmPutPyramid_local1 (TsmHandle *, Pyramid *, char *);
void tsmPutPyramid_web (TsmHandle *, Pyramid *, Purl *);

void tsmReStartReqTiles( TsmHandle *tsmh );
int tsmAddRequest (TsmConnection conn, tsmReqElem *req);
int tsmRemoveRequest (TsmConnection conn, tsmReqElem *req);
int tsmReprioritizeRequest (TsmConnection conn, tsmReqElem *req);

int tsmIsTIFFPresent_local(Purl *, Pyramid *, int, tspSymbol);
int tsmIsTIFFPresent_web(Purl *, Pyramid *, int, tspSymbol);

/* Routines to read in old style tspecs over an http link */

int tsmReadSimpleTileSet_web (const char *, SimpleTileSet *);
int tsmReadCoordTspec_web (const char *, CoordTspec *);
int tsmReadIdentityTspec_web (const char *, IdentityTspec *);
int tsmReadImageTransformationTspec_web( const char *, ImageTransformationTspec *);
int tsmReadPixelTspec_web (const char *, PixelTspec *);
int tsmReadProjectionTspec_web (const char *, ProjectionTspec *);
int tsmReadPyramidTspec_web (const char *, Pyramid *);
int tsmReadSimpleTspec_web (const char *, SimpleTileSet *);
int tsmReadTileTspec_web (const char *, TileTspec *);

/* Routines from tsmUtils.c */

Purl *tsmParseIdentifier( const char *, NameUrlIds * );
void tsmAppendRelativePathNameToUrl( const char *, const char *, char * );
Purl *tsmAppendRelativePathNameToPurl( const char *, const Purl * );
Purl *tsmAppendLocatorToPurl( const char *, const Purl *, const char * );
int tsmPrintElevationTile( SimpleTileSet *, uchar *, int, int, FILE * );
int tsmPrintAverageTile( SimpleTileSet *, uchar *, int, int, FILE * );
int tsmPrintRgbTile( SimpleTileSet *, uchar *, int, int, FILE * );

int tsmPurlToSetId( Purl *, int );
int tsmAddNameUrlId( NameUrlIds *, char const *, char const *, int );
NameUrlIds *tsmMakeNameUrlIds( int );
void tsmFreeNameUrlIds( NameUrlIds * );
const char *tsmNameToUrl( const char *name, NameUrlIds *nameUrlIds );
const char *tsmIdToName( int setId, NameUrlIds *nameUrlIds );
const char *tsmIdToUrl( int setId, NameUrlIds *nameUrlIds );
int tsmNameToId( const char *name, NameUrlIds *nameUrlIds );
int tsmUrlToId( char *, NameUrlIds * );
void tsmNoProtocolYet( TsmProtocol, const char * );
void tsmNotCompiledIn( TsmProtocol, const char * );

void tsmTranslateEmptyUrlsToNamedUrls(TsmHandle*, NameUrlIds*, Purl*, char*);
char *tsmCopyUrlToTmpFile( char * );

void pthr_trace_intArray( int, const char *, const char *, const IntArray *,
                          int, const char * );

/* Routines from tsmAllocTspecs.c */

void tsmFillSimpleTileSets( Pyramid * );

/* Routines from tsmChecksum.c */

int ip_checksum( register u_short *, register int );
int tsmCalcTileStats( TsmConnection, const uchar *, int *, float *, float * );
int tsmUpdateTileStats( TsmConnection, const uchar *, int );

/* Routines from tsmPackTspecs.c */

int tsmCheckString( char **str_p );

/* Routines from tsmHost.c  */

int tsmIsLocalHost( char * );

/* Routines from rgb_image_to_gif.c */

void rgb_image_to_gif_file( char *, unsigned char *, int, int );

#ifdef USE_HTTP

/* Routines from tsmUrlToFile.c */

#include <ghttp/http_trans.h>

http_trans_conn *tsmOpenHttpConnection(http_trans_conn *, char const*);
void tsmCloseHttpConnection( http_trans_conn * );

int tsmUrlToMemory( http_trans_conn **, char const *url, char **buffer, int *size );
int tsmUrlToFile( http_trans_conn **, char const *url, char const *filename );
char *tsmUrlToTempFile( http_trans_conn **, char const *url );
int tsmUrlToBuffer( http_trans_conn **, char const *, char *, int, int * );
char *tsmUrlMimeType( http_trans_conn **, char const *url );
int tsmUrlExists( http_trans_conn **, char const *url );

#endif /* USE_HTTP */

#endif
