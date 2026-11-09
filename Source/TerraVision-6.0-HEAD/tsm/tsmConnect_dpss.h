/*
 * Name: tsmConnect_dpss.h
 *
 * Description: 
 *   Internal definitions for DPSS-based connections.
 *
 * Function List:
 *
 * Revision History:
 *
 *   $Id: tsmConnect_dpss.h,v 1.2.4.2 2001/07/10 06:17:40 leei Exp $
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

#ifndef TSMCONNECTDPSS_H
#define TSMCONNECTDPSS_H

#include "tsmInternal.h"
#include "tsmConnectionInt.h"

/* The TsmConnectionDpss structure holds all of the state for */
/* managing a connection to a dataset on a DPSS.              */

typedef struct TsmConnectionDpss TsmConnectionDpss;


struct TsmConnectionDpss {
  struct TsmConnectionCommon common;

#ifdef DPSS
  /* fields to specify the pyramid on the DPSS */

  int         set_id;              /* The set ID for the DPSS dataset */
  int         numServers;          /* Number of entries in the server array */
  char        **server;            /* list of server names for the dataset */

  /* fields to handle DSM/DPSS connection and tile reading */

  dsmSession  dsmHandle;           /* A handle for the relevant DSM */
  IssHandle   *issHandle;          /* A handle for the DPSS connection */
  dsmSetInfo  set_info;            /* As returned by dsmFetch() */
  RequestList *requestList;        /* A DPSS block request list */
  int         requestThreadRunning;/* TRUE iff the request thread is running */
  PRThread   *requestThread;       /* block request thread for DPSS */
  float       requestTimeout;      /* timeout between sending requests */
  unsigned char *jpegBuffer;       /* temporary JPEG decompress buffer */

  /* fields used for writing to a DPSS */

  BlockMap    *block_table;        /* block table for pyramid on DPSS */
  IssAllocInfo *alloc_info;        /* where data is loaded */
  int         pl_method;           /* 1 = Round Robin, 0 = Chen */
  int         replicate_mode;      /* use replication: TRUE or FALSE */
  int         new_dataset;         /* have we built the block map yet? */
  int         tiles_loaded;        /* count of tiles loaded to DPSS */
  int         total_tiles;         /* total number of tiles to load */
  int         close_on_disconnect; /* do we finish write this session? */
  int         dpssLogging;         /* are we logging DPSS stats? */

  /* fields to define the metadata (tspec) locations */

  int         numMetadata;         /* no. of entries in the metadata array */
  char        **metadata;          /* an array of metadata URLs */

  /* fields to handle the fault-tolerant reconnection thread */

  PRThread   *reconnect_thread;    /* thread that does the reconnect poll */
  PRLock     *reconnect_lock;  /* mutex lock for modifying thread var */ 
#endif /* DPSS */
};

#ifdef DPSS

void tsmDisconnect_dpss( TsmConnection );
void tsmStartRequestThread_dpss ( TsmConnectionDpss * );

int tsmBeginLogging_dpss( TsmConnection, int, const char * );
int tsmEndLogging_dpss( TsmConnection );

Pyramid *tsmReadTspec_dpss( TsmConnectionDpss *, TsmConnectionParams );
Pyramid *tsmReadEmbeddedTspec_dpss( TsmConnectionDpss * );

int tsmWriteTspec_dpss( TsmConnectionDpss * );
int tsmWriteEmbeddedTspec_dpss( TsmConnectionDpss * );

int tsmSendRequestListToDPSS( TsmConnectionDpss* );

int tsmWriteTile_dpss( TsmConnection, int, int, int, uchar * );

int tsmInitDPSSDataset( TsmConnectionDpss *, TsmConnectionParams );
int tsmPrepareDPSSDataset( TsmConnectionDpss * );
int tsmFinishDPSSLoad( TsmConnectionDpss * );
int tsmCalcDPSSBlockSize( TsmConnectionDpss * );
int tsmCalcDPSSSetSize( TsmConnectionDpss * );

int tsmCompareDPSSDates( char const *, PRTime );

int tsmGetSetId( TsmConnectionDpss *, TsmConnectionParams );

#endif /* DPSS */

#endif /* TSMCONNECTDPSS_H */
