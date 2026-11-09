/* -*- tab-width: 2; -*-

  Revision History:
      $Id: TsCacheI.h,v 2.5.2.6 2002/10/31 05:26:38 reddy Exp $

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

#ifndef TsCacheI_h
#define TsCacheI_h

#include <prthread.h>
#include <prlock.h>
#include <prmon.h>
#include <prcvar.h>
#include <pratom.h> 
#include <prinrval.h>

#include <tsm/tsm.h>

#include <se_conv.h>
#include <vrml2/vec3.h>

#include "Trace.h"

typedef struct TsCallbackList {
  int n, len;
  struct TsCallback {
    TsCallbackF func;
    void *closure;
    void (*destroyf) (void*);
  } *cb;
} TsCallbackList;

typedef struct TsCacheByRes TsCacheByRes;

struct TsCache {
  PRInt32 used;
  TileSetTspec const *tspec;
  double origin[2];
  TsBBox bbox;
  int tile_size;

  struct TsCacheByRes {
    SimpleTileSet const * tset;
    /*TsTile **tile;*/
    PRMonitor *monitor;
    eqlhash_t tiles;
    float ixiytouv[9];
    float uvtoixiy[9];
    PRUint32 min_pxl[2];
    PRUint32 max_pxl[2];
  } *byRes;
  
  istack *ist;

  TsCallbackList callback[CACHE_NCALLBACKS];

  char const *url;
  TsmConnectionParams params;
  TsmConnection connect;
  
  PRLock   *lock;
  PRThread *reader;

  int readerWait;      /* How long the reader will wait between polling. */

  SE_CONVERT_COORD_SYSTEM_PAIR *UVtoGDC_cs;
  int (*UVtoGDC) (TsCache const*,
		  float u, float v, float z, double gdc[3]);
  
  SE_CONVERT_COORD_SYSTEM_PAIR *UVtoGCC_cs;
  int (*UVtoGCC) (TsCache const*,
		  float u, float v, float z, double gcc[3]);

  SE_CONVERT_COORD_SYSTEM_PAIR *GDCtoUV_cs;
  int (*GDCtoUV) (TsCache const*,
		  double lng, double lat, double z, float uv[3]);

  SE_CONVERT_COORD_SYSTEM_PAIR *GCCtoUV_cs;
  int (*GCCtoUV) (TsCache const*,
		  double x, double y, double z, float uv[3]);

  unsigned short lvcs;
  unsigned short utm_zone;
  double lvcs_origin[3];
  m3f lvcs_rotate;
  float orientation[4];
  unsigned short southern_utm;   /* is this a southern hemi UTM dataset? */
};

#endif /* TsCacheI_h */
