/*
 * Name: tsmExtProts.h
 *
 * Description: 
 *   External (public) prototypes
 *
 * Function List:
 *
 * Revision History:
 *
 *   "$Id: tsmExtProts.h,v 1.65.2.6 2002/03/14 05:22:56 reddy Exp $"
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

#ifndef TSMAPIEXTPROTS_H
#define TSMAPIEXTPROTS_H

#include "tsm.h"
#include <vrml2/vrml2.h>
#include <ghttp/http_trans.h>

/* The new Connection-based dataset routines */

TsmConnection tsmConnect( char const *, char const *, TsmConnectionParams );
void tsmDisconnect( TsmConnection );

void tsmTileReqBegin( TsmConnection );
int tsmTileReqAdd( TsmConnection, int, int, int, int );
int tsmTileReqEnd( TsmConnection );

int tsmReadTile( TsmConnection, int, int, int, uchar * );
int tsmReadNextTile( TsmConnection, TsmTileHeader *, uchar * );

int tsmAllocReadTile( TsmConnection, int, int, int, uchar ** );
int tsmAllocReadNextTile( TsmConnection, TsmTileHeader **, uchar ** );

int tsmWriteTile( TsmConnection, int, int, int, uchar * );

int tsmBeginLogging( TsmConnection, int, const char * );
int tsmEndLogging( TsmConnection );
int tsmLog( TsmConnection, const char *, const char *, ... );

TsmConnectionType tsmConnectionType (TsmConnection);

void tsmDescribeConnection( TsmConnection );
void tsmProgressReport( TsmConnection, const char *fmt, ... );

/* Create a tile Id that is unique for this tile (x,y,res)  */
/* we build this by taking 6 bits of the resolution and 12  */
/* bits of both the x and y coordiante. This will be unique */
/* for up to 64 resolution levels and tile (x,y) coords in  */
/* the range 0..4095 - which should be enough!              */

#define tsmMakeTileId(tx,ty,level) (( tx    & 0xfff ) +		\
				    ((ty    & 0xfff ) << 12) + 	\
				    ((level & 0x03f ) << 24))

/* Connection characterstic setting and querying */

void tsmSet( TsmConnection, TsmSetting, ... );
int tsmGet( TsmConnection, TsmSetting );

/* Tspec reading routines for the new connection-based api */

Pyramid *tsmReadTspec( char *, TsmConnectionParams );
int tsmWriteTspec( TsmConnection );
void tsmPrintTspec( TsmConnection );
int tsmSetTspecField( TsmConnection, Pyramid * );
int tsmCopyTspecField( TsmConnection, TsmConnection );
int tsmValidateTspec( Pyramid * );

/* Convenience routines for copy data from one dataset to another */

int tsmCopyTile( TsmConnection, TsmConnection, int, int, int );
int tsmCopyLevelTiles( TsmConnection, TsmConnection, int );
int tsmCopyAllTiles( TsmConnection, TsmConnection );
int tsmCopyTspec( TsmConnection, TsmConnection );

/* Timeouts */

void tsmPause (float s);
void tsmPause_ms (PRUint32 ms);
void tsmPause_us (PRUint32 us);

/* Connection Parameter routines */

int tsmInitParams( TsmConnectionParams * );
void tsmFreeParams( TsmConnectionParams );
void tsmSetParam( TsmConnectionParams, TsmParamCode, ... );
TsmConnectionParams tsmCopyParams( TsmConnectionParams );
void tsmPrintParams( TsmConnectionParams, FILE * );

/* VRML support functions */

int tsmWriteVtile( TsmConnection oiConnect, TsmConnection demConnect, 
		   int tx, int ty, int iLevel, TsmConnectionParams params );
int tsmWriteGeotile( TsmConnection oiConnect, TsmConnection demConnect, 
		     int tx, int ty, int iLevel, TsmConnectionParams params,
		     char const **, char const **, int, char const **,
		     char const **, int );
int tsmWriteAllVtiles( TsmConnection oiConnect, TsmConnection demConnect,
		       TsmConnectionParams params );
int tsmWriteAllTrees( TsmConnection oiConn, TsmConnection demConn, 
		      TsmConnectionParams params );

MFString *tsmGetMetadataNode( vsgWorld *world );
int tsmParseMetadataInfo( MFString *, char *token_match, char result[] );

/* Tile manipulation functions */

int tsmSaveImage( TsmConnection, uchar *, char *, TsmImageType type );

/* JPEG codec */

int tsmJPEGCompress( unsigned char *, unsigned char *, int, int, int, int,int);
int tsmJPEGCompressTile( unsigned char *, unsigned char *, TileTspec *, int );
int tsmJPEGDecompressTile( unsigned char *, unsigned char *, TileTspec * );

/* Tile header and buffer manipulation routines */

uchar *tsmAllocTileBuffer( TsmConnection );
TsmTileHeader *tsmAllocTileHeader( void );
void tsmFreeTileBuffer( uchar * );
void tsmFreeTileHeader( TsmTileHeader * );
void tsmFreeTile( TsmTileHeader *, uchar * );

int tsmTileHeaderX( TsmTileHeader const *tileHeader );
int tsmTileHeaderY( TsmTileHeader const *tileHeader );
int tsmTileHeaderLevel( TsmTileHeader const *tileHeader );
int tsmTileHeaderSetId( TsmTileHeader const *tileHeader );
int tsmTileHeaderSessionId( TsmTileHeader const *tileHeader );
int tsmTileHeaderTileId( TsmTileHeader const *tileHeader );
int tsmTileHeaderSize( TsmTileHeader const *tileHeader );
int tsmTileHeaderChecksum( TsmTileHeader const *tileHeader );
TsmDataSetType tsmTileHeaderType( TsmTileHeader const *tileHeader );
TsmConnection tsmTileHeaderConnection( TsmTileHeader const *tileHeader );
int tsmTileHeaderSizeof( void );

/* Dataset Database manipulation routines */

TsmDatabase* tsmReadDatabase( char * );
int tsmWriteDatabase( TsmDatabase *, char * );
void tsmPrintDatabase( TsmDatabase *database, FILE *stream );
int tsmAddDataSetToDatabase( TsmDatabase *, TsmDataSet * );
NameUrlIds *tsmReadServer( char const *url );
int tsmUpdateDatabaseFromServer( TsmDatabase *, TsmServer *,
				 TsmConnectionParams );
int tsmUpdateDatabaseFromServerCB( TsmDatabase *, TsmServer *,
				   TsmConnectionParams, TsmUpdateProc, void *);
void tsmRemoveServerSetsFromDatabase(TsmDatabase *, TsmServer *, NameUrlIds *);
int tsmAddServerToDatabase( TsmDatabase *, char * );
int tsmAddGeoSetToDatabase( TsmDatabase *, TsmGeoSet * );
int tsmRemoveServerFromDatabase( TsmDatabase *, char * );
void tsmRemoveRedundantGeoSets( TsmDatabase * );

/* Dataset Database structure allocation, freeing, and printing */

TsmDatabase *tsmMakeDatabase( void );
void tsmFreeDatabase( TsmDatabase * );
int tsmPersistDatabase( TsmDatabase *, char const * );
TsmDataSetInst *tsmMakeDataSetInst( void );
void tsmFreeDataSetInst( TsmDataSetInst * );
TsmDataSet *tsmMakeDataSet( void );
void tsmFreeDataSet( TsmDataSet * );
TsmGeoSet *tsmMakeGeoSet( void );
void tsmFreeServer( TsmServer * );
void tsmFreeGeoSet( TsmGeoSet * );
TsmGeoSet *tsmGetGeoSet( TsmDatabase *, char const * );
TsmDataSet *tsmGetDataSet( TsmDatabase *, char const * );
char *tsmGetGeoSetFromURL( TsmDatabase *database, char const *url );
TsmModel *tsmGetModel( TsmDatabase *, char const * );
TsmMap *tsmGetMap( TsmDatabase *, char const * );

/* local PyramidUrls database functions */

int tsmAddToLocalDbase( TsmConnection );

/* Tile generation and subsampling functions */

void tsmInitTileParams( TsmTileParams *, TsmDataSetType );
void tsmInitTspecParams( TsmTspecParams *, TsmDataSetType );
TsmImportFormat tsmFindImportFormat( char const * );
void tsmDescribeImportFormat( TsmTileParams );
int tsmParseCreateParams( char *, TsmTileParams *, TsmTspecParams * );

int tsmCreateTspec( TsmConnection, TsmTspecParams, TsmTileParams, int );
int tsmCreateAllTiles( TsmConnection, const char *, TsmTileParams );

/* Memory management functions */

#define tsmMemMalloc(x)    tsmMemManagerMalloc(x)
#define tsmMemCalloc(x,y)  tsmMemManagerCalloc(x,y)
#define tsmMemRealloc(x,y) tsmMemManagerRealloc(x,y)
#define tsmMemFree(x)      tsmMemManagerFree(x)
#define tsmMemStrdup(x)    tsmMemManagerStrdup(x)

extern void *tsmMemManagerMalloc(size_t);
extern void *tsmMemManagerCalloc(size_t, size_t);
extern void *tsmMemManagerRealloc(void *, size_t);
extern int tsmMemManagerFree(void const *);
extern char *tsmMemManagerStrdup(const char *);

/* integer array management */

IntArray *tsmMakeIntArray( int );
int tsmMember_intArray( IntArray *, int );
void tsmAppend_intArray( IntArray *, int );
void tsmGrow_intArray( IntArray * );

/* command line argument parsing */

void tsmParseInitArgs( int, char ** );
TsmConnectionParams tsmParseReadArgs( int, char ** );
TsmConnectionParams tsmParseWriteArgs( int, char ** );
void tsmParseConnectArgs( int, char **, TsmConnection );
TsmConnectionParams tsmParseVRMLArgs( int argc, char **argv );

void tsmDescribeArgs( int );

int tsmGetArg( int, char **, char * );
int tsmGetArgInt( int, char **, char *, int * );
int tsmGetArgFloat( int, char **, char *, double * );
int tsmGetArgString( int, char **, char *, char ** );
int tsmParsedAllOptions( int );

/* Coordinate system transformations */

int tsmLatLongToUTM( double, double, int *, double *, double * );
int tsmUTMToLatLong( int, double, double, double *, double * );
int tsmLatLongToGeocentric( double, double, double, double *, double *,
			    double * );
int tsmGeocentricToLatLong( double, double, double, double *, double *,
			    double * );

/* Checksum routines */

int tsmCalcTileChecksum( TsmConnection, const uchar * );

/* Http stream utilities */

FILE *tsmOpenStream( char const *, char const * );
int tsmCloseStream( FILE * );
int tsmStreamToFile( FILE *, char * );
int tsmStreamToBuffer( FILE *, char *, int );
int tsmStreamToStdout( FILE * );

/* Misc. utility functions */

char *tsmCvsToString( char * );
int tsmShowUrl( const char * );
int tsmIsUrlValid( char const *url );
int tsmDoesUrlExist( char const *url );


/***********************************************************/
/*************** OLD TSMAPI INTERFACE CALLS ****************/
/***********************************************************/


/* Old Open/Closing a tsm connection */

TsmHandle *tsmOpen( const char *dbUrl );
void tsmSetMasterHostName( TsmHandle *, const char * );
TsmHandle *tsmClose( TsmHandle * );

int tsmGetConnectionsFromTsmHandle( TsmHandle *, TsmConnection **, int * );

/* Old tsmApi routines that are no longer supported */

TsmHandle *tsmReOpen( const char *dbUrl, TsmHandle * );
int tsmNumServers (TsmHandle *);

/* Loading/Saving the PyramidUrls database */

int tsmLoadDatabases (TsmHandle *);
int tsmLoadPyramidDatabase (TsmHandle *, const char *);
int tsmReLoadPyramidDatabase (TsmHandle *, const char *);
void tsmSavePyramidDatabase( TsmHandle *, const char * );
int tsmAddToPyramidDatabase( TsmHandle *, const char *, char * );
int tsmLoadPyramidDatabaseIfExist( TsmHandle *, const char * );

/* Pyramid I/O routines */

Purl *tsmGetPyramidBasePurl( TsmHandle *, const char * );
void tsmPrintServerGroup( FILE *, ServerGroup *, char * );
void tsmPrintPyramidInfo( FILE *, PyramidInfo * );
void tsmPrintPyramidInfos( FILE *, PyramidInfos * );

/* Pyramid I/O routines */

PyramidInfo *tsmGetPyramidInfo( TsmHandle *, const char * );
PyramidInfos *tsmGetPyramidInfos( TsmHandle *, int, char const ** );

Pyramid *tsmGetPyramid( TsmHandle *, const char * );
Pyramid *tsmGetPyramid_params( TsmHandle *, const char *, TsmConnectionParams);
void tsmPutPyramid( TsmHandle *, Pyramid *, const char * );
Pyramid *tsmReGetPyramid (TsmHandle *, const char *, Pyramid * );
int tsmGetAllPyramids( TsmHandle *, Pyramids * );
PyramidInfos *tsmGetAllPyramidInfos( TsmHandle * );
int tsmPrepareForDataSets( TsmHandle *, int *, int );
Pyramid *tsmCopyPyramid( const Pyramid *, char * );

int tsmSetTileBaseUrl( Pyramid *, const char * );
int tsmTranslateTileBaseUrl( Pyramid * );
const char *tsmPyramidTspecBaseUrl( Pyramid * );
const char *tsmPyramidTileBaseUrl( Pyramid * );
const char *tsmPyramidName( Pyramid * );
TsmProtocol tsmPyramidTileProtocol( Pyramid * );

void tsmPrintPyramidBasePurls( FILE *fp, Pyramid * );

/* PyramidInfo manipulation routines */

PyramidInfo *tsmFreePyramidInfo( PyramidInfo * );
PyramidInfo *tsmMakePyramidInfo( void );
ServerGroup *tsmFreeServerGroupContents( ServerGroup * );
int tsmSetPyramidInfoName( PyramidInfo *, char * );
int tsmSetPyramidInfoPyramid( PyramidInfo *, Pyramid * );
int tsmSetPyramidInfoSetId( PyramidInfo *, int, int );
int tsmSetPyramidInfoMaster( PyramidInfo *, char *, int );
int tsmAddPyramidInfoServer( PyramidInfo *, char *, int );
int tsmAddPyramidInfoMetaData( PyramidInfo *, char *, int );

/* Tile buffer manipulation routines */

unsigned int tsmNumBytesPerTile (Pyramid *);
int tsmTotalNumTiles( Pyramid *, int );
unsigned char *tsmMakeTileBuffer (Pyramid *);
TsmTileHeader *tsmMakeTileHeader( void );
unsigned char *tsmReMakeTileBuffer (Pyramid *, unsigned char *);
int tsmFlipTileVertically(unsigned char *, TileTspec *);
void tsmCopyPyramidLevelToBinaryStream (TsmHandle *tsmh, Pyramid *pyramid, 
					int level, FILE *stream);

/* Get/Set various state variables */

void tsmSetGetTileFlag( int );
int tsmGetGetTileFlag( void );
void tsmSetUseNewTileFlag( int );
int tsmGetUseNewTileFlag( void );
void tsmSetReadTileFormat( tspSymbol );
tspSymbol tsmGetReadTileFormat( void );
void tsmSetHttpToLocalFile( int );

/* Tile I/O routines */

int tsmGetTile( TsmHandle *, Pyramid *, int, int, int, unsigned char *, ... );
int tsmPutTile( TsmHandle *, Pyramid *, int, int, int, const unsigned char *, tspSymbol);
int tsmIsTIFFPresent( Pyramid *, int, tspSymbol );

int tsmImg2Tile( TsmHandle *, char *, char *, int, int, int, int, int, int, int, tspSymbol );
int tsmDem2Tile( TsmHandle *, char *, char *, int, int, int, int, int, int, int, tspSymbol );

/* JPEG related tile routines */

int tsmGetTileJFIF (Pyramid *, int, int, int, int *, unsigned char *);
int tsmGetJPEGTileSize( const Pyramid *, int, int, int );

void tsmSetJPEGQuality( int );
int tsmGetJPEGQuality( void );

/* Checksum routines */

int tsmCheckSumTileBuffer( const unsigned char *, int );
int tsmCheckSumTile( TsmHandle *, Pyramid *, int, int, int, uchar *, int );
int tsmChecksumPyramid( TsmHandle *, Pyramid * );
int tsmChecksumPyramid_col( TsmHandle *, Pyramid * );
int tsmCalcChecksum( TsmHandle *, Pyramid *, tsmChecksumArray * );
int tsmSetPyramidChecksum( Pyramid *, tsmChecksumArray *, tspSymbol );

/* http access routines */

void tsmInitHttp( void );
void tsmCleanupHttp( void );
void tsmSetHttpProxy( char const *proxy );
http_trans_conn *tsmOpenHttpConnection( http_trans_conn *, char const *);
void tsmCloseHttpConnection( http_trans_conn * );

/* Tile data and requesting */

typedef struct tsmRequestList_ tsmRequestList;

tsmRequestList * tsmConnectRequestList (TsmConnection connect);

/* FIXME: This should have the received parameter added.
   That changes the API though, so we can wait. -- leei */
typedef int (*tsmRequestNotifyCB) (tsmRequestList *req,
				   TsmTileHeader *tileHeader,
				   uchar *tileBuffer,
				   /*int received,*/
				   void *cl);

void tsmRequestNotifyReceive (tsmRequestList *req,
			      tsmRequestNotifyCB receive_cb,
			      void *closure);
void tsmRequestAdd (tsmRequestList *req, int x, int y, int res, int prio);
void tsmRequestSubmit (tsmRequestList *req);
int tsmRequestPending (tsmRequestList *req);
void tsmRequestDestroy (tsmRequestList *req);

void tsmStartReqTiles( TsmHandle * );
int tsmReqTile( TsmHandle *, Pyramid *, int, int, int, int );
int tsmStopReqTiles( TsmHandle * );
int tsmGetNextTile( TsmHandle *, TsmTileHeader *, unsigned char *, ... );
int tsmGetNextTileHeader( TsmHandle *, TsmTileHeader *, ... );
int tsmGetNextTileData( TsmHandle *, TsmTileHeader *, unsigned char *, ... );
int tsmAllocGetNextTile( TsmHandle *, TsmTileHeader **, unsigned char **, ...);

int tsmTileToRgb (unsigned char *, TileTspec *, PixelTspec *, 
		  unsigned char *, int, int, int, int, int, int, int, int);
char *tsmRgbBufferToGifFile (char *, unsigned char *, int, int);


char *tsmSetIdToPyramidTspecUrl (char const *, int);
char *tsmPyramidUrlFromSetId (int, char const *, char const *, int);
char *tsmPyramidUrlFromString (int, char const *, char const *, char const *);

char const *tsmPyramidNameToUrl (TsmHandle *, char const *);
int tsmPyramidNameToId (TsmHandle *, char const *);

void tsmFilenameToUrl( const char *, char *, int );
void tsmAddFilenameExtension( char *, char const * );
int tsmCheckFilenameExtension( char const * , char const * );
void tsmMakeDirIfNotExist( const char *dirname, int isDir );
char *tsmNameFromPathname( char * );

int tsmExtractTsmAndPyramidName( const char *, char *, char * );
int tsmStreamToFile (FILE *, char *);
int tsmStreamToBuffer (FILE *, char *, int);
int tsmStreamToStdout (FILE *);

Pyramid *tsmAllocTspec( void );
Pyramid *tsmFreeTspec (Pyramid *);

Pyramid *tsmMakePyramid (void);
Pyramid *tsmAllocatePyramid (char *, int, int );
Pyramid *tsmFreePyramid (Pyramid *);
SimpleTileSet *tsmMakeSimpleTileSetArray (SimpleTileSetArray *arr,
					  int minLevel, int maxLevel);
int tsmEnlargeSimpleTileSetArray (SimpleTileSetArray *tspecLevel,
				  int numLevels);
void tsmFreeSimpleTileSetArray (SimpleTileSetArray *arr);
void tsmFreeIdentityTspecContents (IdentityTspec *);
void tsmFreePixelTspecContents( PixelTspec *tspec );
void tsmFreeSimpleTileSetContents (SimpleTileSet *);
int *tsmMakeChecksum (tsmChecksumArray *arr, int minLevel, int maxLevel);

int tsmStartLogging( TsmHandle *, Pyramid *, int, const char * );
int tsmStopLogging( TsmHandle *, Pyramid * );
int tsmPutLog( TsmHandle *, Pyramid *, const char *, const char *, ... );

/* tsmUtils.c */

double tsmElapsedTime (PRIntervalTime tstart, PRIntervalTime tstop);
char* tsmDateString( PRTime );
char *tsmStringCat( char const *string, char const *suffix );
int tsmPrintTile (SimpleTileSet *simpleTileSet, 
		  unsigned char *tileBuffer, int tx, int ty, FILE *file);
int tsmInverse3x3( float *in, float *out );
char *tsmPlanetToString( TsmPlanet );
TsmPlanet tsmStringToPlanet( const char * );
int tsmMakePathAbsolute( char *pathname, char const *absdir, int buffer_size );

#endif
