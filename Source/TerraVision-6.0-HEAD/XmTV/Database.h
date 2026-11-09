/*
 * Name: Database.h
 *
 * Description: 
 *     Controls a series of dialogs to all the user to select datasets
 *     from a global database, manage the contents of the database, and
 *     control various advanced options of the selected datasets.
 *
 * Function List:
 *
 * Dependencies:
 *
 *  ----- BEGIN LICENSE BLOCK -----
 *  Version: MPL 1.1
 * 
 *  The contents of this file are subject to the Mozilla Public License Version
 *  1.1 (the "License"); you may not use this file except in compliance with
 *  the License. You may obtain a copy of the License at
 *  http://www.mozilla.org/MPL/
 * 
 *  Software distributed under the License is distributed on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 *  for the specific language governing rights and limitations under the
 *  License.
 * 
 *  The Original Code is the TerraVision 3D Terrain Visualization System.
 * 
 *  The Initial Developer of the Original Code is SRI International.
 *
 *  Portions created by the Initial Developer are Copyright (C) 1992-2002
 *  the Initial Developer. All Rights Reserved.
 * 
 *  Contributor(s): Yvan Leclerc, Lee Iverson, Martin Reddy, Nat Bletter. 
 *  ----- END LICENSE BLOCK -----
 *
 */

#ifndef DATABASE_H
#define DATABASE_H

#include "TsGeoPyramid.h"
#include <X11/Intrinsic.h>

/* The TVDatabase structure contains all of the state for */
/* managing the global terrain dataset database.          */

struct TVDatabase {
  char                *location;    /* filename of the local database file */
  TsmDatabase         *database;    /* the actual database structure */

  /* the following variable provide state for the Database Options widget */

  int                 db_selected;  /* currently selected server index */

  /* The following variables are for the Open TV widget */

  int                 tv_curr_geo;  /* the currently selected geoset, or -1 */
  int                 tv_curr_oi;   /* the current oi instance, or -1 */
  int                 tv_curr_dem;  /* the current dem instance, or -1 */
  int                 tv_curr_oi_serv;     /* the current oi server, or -1 */
  int                 tv_curr_dem_serv;    /* the current dem server, or -1 */

  TsmDataSetInst      *curr_oi_inst[20];   /* OI instances for each server */
  TsmDataSetInst      *curr_dem_inst[20];  /* DEM instances for each server */

  TsmConnectionParams oi_params;    /* connection parameters for the OI */
  TsmConnectionParams dem_params;   /* connection parameters for the OI */

  Widget              tv_dialog;    /* the main window widget for Open TV */
  Widget              tv_dbase;     /* the database filename text widget */
  Widget              tv_opts;      /* the database options button */
  Widget              tv_geosets;   /* the list for geosets */
  Widget              tv_oiloc;     /* the list for OI pyramid locations */
  Widget              tv_demloc;    /* the list for DEM pyramid locations */
  Widget              tv_oiserv;    /* the list for OI pyramid servers */
  Widget              tv_demserv;   /* the list for DEM pyramid servers */
  Widget              tv_oiadv;     /* the button for OI advanced options */
  Widget              tv_demadv;    /* the button for DEM advanced options */
  Widget              tv_okay;      /* the okay button for Open TV */
  Widget              tv_cancel;    /* the cancle button for Open TV */
};
typedef struct TVDatabase TVDatabase;

/* some defines to control partial refreshing of the Open TV widget */

#define DBASE_REFRESH_NONE         0
#define DBASE_REFRESH_FILENAME     2<<0
#define DBASE_REFRESH_GEOSETS      2<<1
#define DBASE_REFRESH_OI_URLS      2<<2
#define DBASE_REFRESH_OI_SERVERS   2<<3
#define DBASE_REFRESH_DEM_URLS     2<<4
#define DBASE_REFRESH_DEM_SERVERS  2<<5

#define DBASE_REFRESH_ALL          ~0
#define DBASE_REFRESH_ALL_OI       DBASE_REFRESH_OI_URLS | \
                                   DBASE_REFRESH_OI_SERVERS
#define DBASE_REFRESH_ALL_DEM      DBASE_REFRESH_DEM_URLS | \
                                   DBASE_REFRESH_DEM_SERVERS

/* we use a single global instance of the TVDatabase structure */

extern TVDatabase tv_database;

void DBaseInit( Widget parent );
void DBaseShowOpenTV( void );
void DBaseRefreshTV( int );

void DBaseInitOptions( Widget parent );
void DBaseShowOptions( void );
void DBaseHideOptions( void );
void DBaseRefreshOpts( void );

void DBaseSetFullDPSSRead( int do_full_dpss_read );

void DBaseInitAdvanced( Widget parent );
void DBaseShowAdvanced( TsmDataSetType type );
void DBaseHideAdvanced( void );

TsmDatabase *GetDBase( void );

#include <tsm/tsm.h>

extern void TerraVisionLoadDataset(char const *geo_name,
				   char const *oi_url,
				   char const *dem_url,
				   TsmConnectionParams oi_params,
				   TsmConnectionParams dem_params);

#endif
