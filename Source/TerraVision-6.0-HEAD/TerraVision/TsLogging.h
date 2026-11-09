/* -*- tab-width: 2; -*-

  Name: TsLogging.h
 
  Description: Header file for logging information
 
  Revision History:
      $Id: TsLogging.h,v 2.18.14.2 2002/10/31 05:26:41 reddy Exp $

  ----- BEGIN LICENSE BLOCK -----
  Version: MPL 1.1
 
  The contents of this file are subject to the Mozilla Public License Version
  1.1 (the "License"); you may not use this file except in compliance with
  the License. You may obtain a copy of the License at
  http://www.mozilla.org/MPL/
 
  Software distributed under the License is distributed on an "AS IS" basis,
  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
  for the specific language governing rights and limitations under the
  License.
 
  The Original Code is the TerraVision 3D Terrain Visualization System.
 
  The Initial Developer of the Original Code is SRI International.

  Portions created by the Initial Developer are Copyright (C) 1992-2002
  the Initial Developer. All Rights Reserved.
 
  Contributor(s): Yvan Leclerc, Lee Iverson, Martin Reddy, Nat Bletter. 
  ----- END LICENSE BLOCK -----
 
*/

#ifndef TSLOGGING_H
#define TSLOGGING_H

#include <prlock.h>
#include <prinrval.h>

typedef enum {
  TV_RENDER_INTERVAL,
  TV_VISIBLE_QT_INTERVAL,
  TV_PREDICT_QT_INTERVAL,
  TV_REQ_TILES_INTERVAL, 
  TV_REQ_TILE,
  TV_REC_TILE,
  TV_TEXTURE_CACHE_ALLOC,
  TV_TEXTURE_CACHE_FREE,
  TV_TILE_CACHE_ALLOC,
  TV_TILE_CACHE_FREE
} TsLoggingKeys;

/* Doc++ Documentation: *//**
 * Enumeration of type of log files.  Values are:
 * <pre>
 * TV_PREDICT_LOG
 * TV_REQUEST_LOG
 * TV_VISIBLE_LOG
 * TV_RENDER_LOG
 * TV_CACHE_LOG
 * TV_TEXTURE_CACHE_LOG
 * TV_READER_LOG
 * </pre>
 *
 */
enum TsLogFiles {
  TV_PREDICT_LOG,
  TV_REQUEST_LOG,
  TV_VISIBLE_LOG,
  TV_RENDER_LOG,
  TV_CACHE_LOG,
  TV_TEXTURE_CACHE_LOG,
  TV_READER_LOG
};
typedef enum TsLogFiles TsLogFiles;


#define TV_NUMLOGS      7
#define MAX_HISTORY_LOG 200

typedef struct {
  PRIntervalTime logTime;
  int            val;
} ValLog;

typedef struct {
  PRLock          *serverLock;
  PRLock          *logLock;
  int             logging[TV_NUMLOGS];
  int             currLog[TV_NUMLOGS];
  PRIntervalTime  logHistory[TV_NUMLOGS][MAX_HISTORY_LOG][2];
  int             requestHistory[MAX_HISTORY_LOG][3];
  ValLog          numTextureLog[MAX_HISTORY_LOG][2];
  int             currNumTexture;
  int             currServerLog;
  ValLog          serverLogs[MAX_HISTORY_LOG];
  float           weight;
  float           averages[TV_NUMLOGS];

  unsigned long   bytesThisPeriod;  /* running byte total in this period */
  unsigned int    tilesThisPeriod;  /* running tile count for this period */
  double          period;           /* length of a period (secs) */
  unsigned long   totalBytesRead;   /* bytes read since app started */
  unsigned long   totalTilesRead;   /* tiles read since app started */
} TsLog;

#ifdef __cplusplus
extern "C" {
#endif

/* initialisation routine for the logging state */

void CreateTsLog( void );

/* routines to start, stop, and check logging status */

int  TsIsLogging( TsLogFiles );
int  TsLogEndLogging( TsLogFiles );
int  TsLogStartLogging( TsLogFiles );

/* routines to log data to the logger state */

int  TsLogRenderData( PRIntervalTime, PRIntervalTime, double, int, int, int );
int  TsLogVisibleData( PRIntervalTime, PRIntervalTime, int );
int  TsLogPredictData( PRIntervalTime, PRIntervalTime, int );
int  TsLogRequestListData( PRIntervalTime, PRIntervalTime, int, int, int );
void TsLogTileRead( int );
void TsLogBurstRate( int );

/* routines to extract information about the current logger state */

double        TsLogGetNetBurstRate( void );       /* Mbit/s */
unsigned long TsLogGetTotalBytesRead( void );
unsigned int  TsLogGetTotalTilesRead( void );

/* utility routine for converting a timeval_t into a number of seconds */

double DoubleSeconds( PRIntervalTime );

/* return the private TsLog structure for direct manipulation. N.B. */
/* this breaks the module interface, but needed by TVLogger.c for now */

TsLog *GetTsLog( void );

#ifdef __cplusplus
}
#endif

#endif
