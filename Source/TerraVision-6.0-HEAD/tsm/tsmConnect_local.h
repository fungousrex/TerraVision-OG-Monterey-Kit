/*
 * Name: tsmConnectLocal.h
 *
 * Description: 
 *   Internal definitions for local file connections.
 *
 * Function List:
 *
 * Revision History:
 *
 *   $Id: tsmConnect_local.h,v 1.4.2.2 2001/06/19 22:04:39 leei Exp $
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

#ifndef TSMCONNECTLOCAL_H
#define TSMCONNECTLOCAL_H

#include "tsmInternal.h"
#include "tsmConnectionInt.h"

#include <pthr/heap.h>

/* The TsmConnectionLocal structure holds all of the state for */
/* managing a connection to a dataset on a local filesystem.   */

typedef struct TsmConnectionLOCAL *TsmConnectionLOCAL;
typedef struct TsmConnectionLocal TsmConnectionLocal;

struct TsmConnectionLocal {
  TsmConnectionLOCAL conn;

  TIFF      **TIFFhandle;        /* libtiff handle for a writing .tiff file */
  int       *tiles_written;      /* count of tiles written to .tiff file */
  unsigned char *jpegBuffer;       /* temporary JPEG decompress buffer */

  heap_t *heap;
  PRLock *lock;
  PRThread *thread;
  PRCondVar *ready_cond;
};

struct TsmConnectionLOCAL {
  struct TsmConnectionCommon common;

  PRLock **readLevel_lock;
  TIFF   **readLevel_tiff;

  PRLock             *lock;
  int                numReaders;
  int                nextReader;
  TsmConnectionLocal **readers;
  TsmConnectionLocal *writer;
};

extern void tsmReaderThread_local (void *ptr);

extern int tsmAddRequest_local (TsmConnection conn, tsmReqElem *req);
extern int tsmRemoveRequest_local (TsmConnection conn, tsmReqElem *req);
extern int tsmReprioRequest_local (TsmConnection conn, tsmReqElem *req);

extern int tsmWriteTile_local( TsmConnection, int, int, int, uchar * );

extern int tsmCheckTileFormat_local( TsmConnection, TsmTileFormat );
extern int tsmCheckTIFFFormat_local( TsmConnection, int );

extern int tsmReadTIFF_local( TsmConnectionLocal*, int, int, int, uchar * );
extern int tsmReadTILE_local( TsmConnectionLocal*, int, int, int, uchar * );

extern int tsmWriteTIFF_local( TsmConnectionLocal*, int, int, int, const uchar * );
extern int tsmWriteTILE_local( TsmConnectionLocal*, int, int, int, const uchar * );

#endif /* TSMCONNECTLOCAL_H */
