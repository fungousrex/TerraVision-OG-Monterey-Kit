/*
 * Name: tsmDatabase.h
 *
 * Description:
 *   Contains all of the data structure definitions for the dataset
 *   database. This is used to give a global database representing all
 *   of the terrain datasets that are available for TerraVision, 
 *   including their various locations and representations.
 *
 *   N.B. the format used for the dataset database file is described at:
 *   http://www.ai.sri.com/~magic/tsmApi/dataset-db.html
 *
 * Structure List:
 *     TsmDatabase
 *     TsmGeoSet
 *     TsmDataSet
 *     TsmDataSetInst
 *     TsmServer
 *     TsmDataSetType
 *     TsmPlanet
 *     TsmModel
 *     TsmMap
 *
 *     Footprint, LatLongCoord, LatLongFootprint, UTMCoord, UTMFootprint
 *     GeoCoord, GeoFootprint, TsmCoordSystem defined in TsTileSpecDecs.x
 *
 * Revision History:
 *   December 1997 - Initial version by Martin Reddy
 *
 *   $Id: tsmDatabase.h,v 1.15.6.1 2001/06/20 22:26:17 reddy Exp $
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

#ifndef TSMDATABASE_H
#define TSMDATABASE_H

#include <prtime.h>
#include "tsmPurl.h"


/* An enumerated type to specify the valid dataset types */

typedef enum {
  TSM_DEM_DATASET,          /* Dataset contains terrain elevation data */
  TSM_IMAGE_DATASET,        /* Dataset contains terrain imagery */
  TSM_VRML_DATASET,         /* Dataset contains VRML code */
  TSM_UNKNOWN_DATASET       /* Unknown dataset type */
} TsmDataSetType;

/* defines to specify image formats that may be combined in a single value */

#define TSM_NONE_FORMAT   0
#define TSM_LRBT_FORMAT   1
#define TSM_JPEG_FORMAT   2
#define TSM_MIPMAP_FORMAT 4
#define TSM_LZW_FORMAT    8

/* the TsmUpdateProc function typedef is for callback */
/* funcations used by tsmUpdateDatabaseFromServerCB() */

typedef void (*TsmUpdateProc)(char const *name, float percent, void *closure);

/*
 * Doc++ Description: *//** 
 *   Holds a database server entry.
 *
 *   A structure to hold a server URL as well as the last date of contact.
 *   A database can contain a number of these structures in order to
 *   specify all of the TSM servers that it knows about.
 *
 * @param location (Purl *) = the parse URL structure for the TSM server URL
 * @param last_contacted (PRTime) = a timestamp value in the format returned
 *                                 by the nspr PR_Now() function.
 *
 * @see TsmDatabase
 *
 */

struct TsmServer {
  Purl   *location;           /* URL for the Tile Set Manager */
  PRTime last_contacted;      /* time/date this server was last contacted */
};

typedef struct TsmServer TsmServer;

/*
 * Doc++ Description: *//** 
 *   Holds a single dataset instance.
 *
 *   This structure holds all of the information to describe a
 *   single dataset instance and how to access it. A dataset can
 *   contain one or more instances.
 *
 * @param location (Purl *) = the parse URL for the instance's location
 * @param formats (int) = formats in which the instances is available. This
 *        is a bit field which can be composed of the values
 *        TSM_LRBT_IMAGE, TSM_JPEG_IMAGE, or TSM_MIPMAP_IMAGE.
 * @param set_id (int) = the set id for an instance if it is on a DPSS 
 * @param numServers (int) = the number of strings in the serverName array
 * @param serverName (char **) = an array of strings describing the combination
 *        of servers for an instance on a DPSS
 *
 * @see TsmDataSet
 *
 */

struct TsmDataSetInst {
  Purl *location;            /* a parsed URL for the dataset's location */
  int  formats;              /* e.g. TSM_LRBT_IMAGE + TSM_JPEG_IMAGE */
  int  set_id;               /* SetID of a dataset on a DPSS */
  int  numServers;           /* number of servers in serverName array */
  char **serverName;         /* array of server host names for a DPSS set */
};

typedef struct TsmDataSetInst TsmDataSetInst;

/*
 * Doc++ Description: *//** 
 *   Holds a single dataset.
 *
 *   This structure holds all of the general information describing a
 *   single dataset, including a list of specific instances.
 *
 * @param name (char *) = an identifying unique name for dataset
 * @param description (char *) = text describing the dataset
 * @param source_type (char *) = "unknown", "spot", "cir-napp", etc.
 * @param type (TsmDataSetType) = TSM_IMAGE_DATASET, TSM_DEM_DATASET,
 *        or TSM_VRML_DATASET
 * @param footprint (Footprint) = the bounding box for the dataset
 * @param min_res (float) = the minimum resolution of the dataset (meters)
 * @param max_res (float) = the maximum resolution of the dataset (meters)
 * @param numInstances (int) = the number of entries in the instance array
 * @param instance (TsmDataSetInst *) = an array of dataset instances
 *
 * @see TsmDataSetInst, TsmDatabase, LatLongFootprint, UTMFootprint
 *
 */

struct TsmDataSet {
  char            *name;          /* identifying unique name for dataset */
  char            *description;   /* description of dataset coverage */
  char            *source_type;   /* "spot", "cir-napp", etc. */
  char            *geoname;       /* the name of the geoset */
  TsmDataSetType  type;           /* image-pyramid, dem-pyramid, etc. */
  Footprint       footprint;      /* footprint of dataset */
  float           min_res;        /* min resolution for dataset (meters) */
  float           max_res;        /* max resolution for dataset (meters) */
  int             tilesize;       /* tilesize in x and y directions */
  int             overlap;        /* tile overlap on right and top edges */
  int             numInstances;   /* no. of entries in the instance array */
  TsmDataSetInst  *instance;      /* an array of dataset instances */
};

typedef struct TsmDataSet TsmDataSet;

/*
 * Doc++ Description: *//** 
 *   Holds a single model definition.
 *
 *   This structure holds all of the information describing a
 *   single model, such as a set of buildings, roads, a weather
 *   simulation, etc.
 *
 * @param name (char *) = an identifying unique name for map
 * @param description (char *) = an optional descriptive text
 * @param location (Purl **) = an array of alternative URL locations
 * @param numLocations (int) = the number of URLs in the location array
 *
 * @see TsmGeoSet
 *
 */

struct TsmModel {
  char  *name;                /* a unique name for this model */
  char  *title;               /* a descriptive title for the model */
  char  *geoname;             /* the name of the geoset to associate with */
  char  *description;         /* an optional description text */
  Purl  **location;           /* an array of alternative URL location */
  int   numLocations;         /* the number of URLs in the array */
};

typedef struct TsmModel TsmModel;

/*
 * Doc++ Description: *//** 
 *   Holds a single map definition.
 *
 *   This structure holds all of the information describing a
 *   single map image. The bounding box is specified by the
 *   lower-left and upper-right (u,v) coordinates. It is assumed
 *   that the map is in the same coordinate system as the associated
 *   pyramid (e.g. as specified in the pyramid tspec file).
 *
 * @param name (char *) = an identifying unique name for map
 * @param location (Purl *) = the URL location of the map image
 * @param min_u (double) = the lower-left u coordinate of the map
 * @param min_v (double) = the lower-left v coordinate of the map
 * @param max_u (double) = the upper-right u coordinate of the map
 * @param max_v (double) = the upper-right v coordinate of the map
 *
 * @see TsmGeoSet
 *
 */

struct TsmMap {
  char    *name;                /* the name of the map image */
  char    *geoname;             /* the name of the geoset to associate with */
  Purl    **location;           /* array of URLs where the map is stored */
  int     numLocations;         /* number of entries in the URL array */
  double  min_u, min_v;         /* lower-left (u,v) coord of map */
  double  max_u, max_v;         /* uuper-right (u,v) coord of map */
}; 

typedef struct TsmMap TsmMap;

/*
 * Doc++ Description: *//** 
 *   Holds a information to tie a DEM and an OI dataset together.
 *
 *   The TsmGeoSet structure holds all of the meta-information for
 *   describing one region of terrain. That is, it gives a name for
 *   the DEM dataset providing the elevation data, and a name for
 *   the ortho-imagery dataset providing the texture for that region.
 *
 *   Using the Ft Irwin dataset as an example, a geoset might be
 *   called (i.e. its name field is) "ntc-1-utm-2scene", and it 
 *   may have DEM and OI datasets of "ntc-1-utm-2scene.33.dem" and
 *   "ntc-1-utm-2scene.128.oi". 
 *
 *   The information stored in a TsmGeoSet is largely dervied from
 *   the old GeoPyramid structure.
 *
 * @param name (char *) = an identifying unique name for geoset
 * @param oi_name (char *) = an identifying unique name for the imagery dataset
 * @param dem_name (char *) = an identifying unique name for the DEM dataset
 * @param planet (TsmPlanet) = one of TSM_EARTH, TSM_MARS, TSM_VENUS, etc.
 *
 * @see TsmDatabase, TsmDataSet
 *
 */

struct TsmGeoSet {
  char       *name;           /* the name of the geoset (not a URL) */
  char       *oi_name;        /* the name of the OI dataset (not a URL) */
  char       *dem_name;       /* the name of the DEM dataset (not a URL) */
  char       **map_name;      /* an array of map image names (not URLs) */
  int        numMaps;         /* number of maps associated with dataset */
  char       **model_name;    /* an array of model names (not URLs) */
  int        numModels;       /* number of models associated with dataset */
  TsmPlanet  planet;          /* TSM_EARTH, TSM_MARS, TSM_VENUS, etc. */
};

typedef struct TsmGeoSet TsmGeoSet;

/*
 * Doc++ Description: *//** 
 *   Holds all information for a dataset database.
 *
 *   This structure contains an array of datasets, along with information
 *   about all of the servers used to build the database, and the URL
 *   of the dataset database file that was parsed to give that array.
 *
 *   The format used for the dataset database file is described at:
 *   \URL{http://www.ai.sri.com/~magic/tsmApi/dataset-db.html}
 *
 * @param numDataSets (int) = the number of entries in the dataset array
 * @param dataset (TsmDataSet *) = an array of dataset entries
 * @param numServers (int) = the number of entries in the server array
 * @param server (TsmServer *) = an array of server entries
 * @param numGeoSets (int) = the number of entries in the geoset array
 * @param geoset (TsmGeoSet *) = an array of geoset entries
 * @param numModels (int) = the number of entries in the model array
 * @param model (TsmModel *) = an array of model entries
 * @param numMaps (int) = the number of entries in the map array
 * @param map (TsmMap *) = an array of map entries
 * @param location (Purl *) = the parsed URL for the database file location
 *
 * @see tsmReadDatabase, tsmWriteDatabase, tsmMakeDatabase, 
 *      tsmFreeDatabase, TsmServer, TsmDataSet, TsmGeoSet
 *
 */

struct TsmDatabase {
  int        numDataSets;        /* number of entries in dataset array */
  TsmDataSet *dataset;           /* an array of dataset structures */
  int        numServers;         /* number of entries in server array */
  TsmServer  *server;            /* an array of server structures */
  int        numGeoSets;         /* number of entries in geoset array */
  TsmGeoSet  *geoset;            /* an array of geoset structures */
  int        numModels;          /* number of entries in model array */
  TsmModel   *model;             /* an array of model structures */
  int        numMaps;            /* number of entries in map array */
  TsmMap     *map;               /* an array of map structures */
  Purl       *location;          /* URL for the database that was parsed */
};

typedef struct TsmDatabase TsmDatabase;

#endif
