/*
 * Name: tsmTileInt.h
 *
 * Description: 
 *   Internal API definitions about the internal structure of tile
 *   headers.
 *
 * Function List:
 *
 * Revision History:
 *
 *   "$Id"
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

#ifndef TSMTILEINT_H
#define TSMTILEINT_H

#include "tsmDatabase.h"
#include "tsmConnection.h"

/* The information about a tile is divided up into two structures: one */  
/* a tile header, containing all of the information about that tile    */
/* (e.g. tile coordinates, bytes size, resolution, etc.), the other    */
/* structure contains the raw data for the tile. Here we define the    */
/* structure for the header. This is hidden from external clients.     */
/* The structure is based largely on the DPSS's TileHeader, but it     */
/* gives us the possibility of storing extra data, e.g. data type and  */
/* and a pointer to the connection handle for the tile's dataset.      */

#ifndef TIMECOUNT
#define TIMECOUNT   10            /* number of time stamps in tile header */
#endif

struct TsmTileHeader {
  int            tx;              /* the x coordinate of the tile */
  int            ty;              /* the y coordinate of the tile */
  int            level;           /* the level number of the tile */
  int            pad;             /* for possible future expansion */
  int            set_id;          /* the set ID of the tile's dataset */
  int            session_id;      /* the session ID of the connection */
  int            tile_id;         /* unique ID for each requested tile */
  int            size;            /* the size of the tile in bytes */
  int            checksum;        /* the tile's checksum */
  PRTime         tv[TIMECOUNT];   /* an array of timestamps */
  TsmDataSetType type;            /* type of dataset: TSM_DEM_DATASET, etc. */
  TsmConnection  connect;         /* connection handle for the tile */
};


#endif
