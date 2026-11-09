/*
  Filename: tvutils.h

  Description: This is the main header file to include for access to the
  features of the tvutils library. This library contains platform-specific
  support routines for TerraVision applications, including

    - Parsing of command lines
    - Storing and retrieving configuration information
    - Storing and retrieving viewpoint bookmarks
    - Generic navigation routines
    - Simple entry functions for TerraVision initialisation/closedown

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

#ifndef TVUTILS_H
#define TVUTILS_H

#include <TerraVision/ThreeDCore.h>

/* the follow defines can be passed to the tvuGet functions */
/* (I=int, F=float, S=string, B=boolean, L=string list)     */

#define TVU_ARGS_PARSED        0       /* I: number of cmd line args parsed */
#define TVU_DROP_TO_WIREFRAME  1       /* F: frame rate to reduce complexity */
#define TVU_SCHEMA_URL         2       /* S: url for .geo schema */
#define TVU_TRACE_MASK         3       /* S: trace mask from command line */
#define TVU_NUM_DATASETS       4       /* I: number of -oi and -dem options */
#define TVU_OI_URL             5       /* L: one of the -oi strings */
#define TVU_DEM_URL            6       /* L: one of the -dem strings */
#define TVU_INITIAL_VIEWER     7       /* I: 0=none, 2=2d, 3=3d viewer */
#define TVU_TV_VERSION         8       /* S: release version of TerraVision */
#define TVU_STEREO             9       /* B: request a stereo visual */
#define TVU_SINGLE_BUFFER      10      /* B: render in single buffer mode */
#define TVU_TILE_BY_TILE       11      /* B: flush display after each tile */
#define TVU_NUM_COMMANDS       12      /* I: number of -cmd options */
#define TVU_COMMAND            13      /* L: one of the -cmd strings */
#define TVU_CONFIG_URL         14      /* S: url for the configuration file */
#define TVU_X_ERROR_ABORT      15      /* B: do an abort() if get an x error */
#define TVU_SHOW_LICENSE       16      /* B: show the TerraVision license */
#define TVU_LOG                17      /* B: start logging on start up */
#define TVU_NO_IPC             18      /* B: no commands from stdin */
#define TVU_NS_WIN_ID          19      /* I: the netscape window handle */
#define TVU_WIDTH              20      /* I: requested window width */
#define TVU_HEIGHT             21      /* I: requested window height */
#define TVU_CAVE               22      /* B: is the CAVE on? */
#define TVU_INSTALL_DIR        23      /* S: directory TV is installed in */
#define TVU_USER_DIR           24      /* S: the user's config directory */
#define TVU_MODE               25      /* I: the mode passed to tvuInit() */
#define TVU_PATH_URL           26      /* S: url to a TV path file to load */
#define TVU_VIEWPOINT_URL      27      /* S: url to a TV viewpoint file */
#define TVU_DATABASE_URL       28      /* S: filename of local database file */
#define TVU_LICENSE_ACCEPTED   29      /* F/S: version of accepted license */
#define TVU_VERSION            30      /* F: the tvu library version no. */
#define TVU_TOTAL_ARGS         31      /* I: total number of cmd line args */
#define TVU_PATHS_DIR          32      /* S: default dir to load paths from */
#define TVU_TRACE_FILE         33      /* S: file to output debug info to */
#define TVU_PROFILE_URL        34      /* S: url for .geo profile */
#define TVU_TCL_FILENAME       35      /* S: path to TerraVision.tcl file */
#define TVU_VISUAL_ID          36      /* I: id of the desired OGL visual */

/* the following defines are used by the tvuInit() call */

#define TVU_MOTIF              0
#define TVU_PLUGIN             1
#define TVU_TCLTK              2
#define TVU_NO_GUI             3

#define TVU_DEFAULT            TVU_MOTIF

/* the following utils are used in the ViewpointItem coordsys field */

#define TVU_LVCS               0
#define TVU_GCC                1

/* the following types are used in the tvuGazResponse type field */

#define TVU_GAZ_LATLONG        0
#define TVU_GAZ_CHOICES        1

/* the following typedef holds a single viewpoint specification */

typedef struct {
  char const *name;                    /* name of this viewpoint */
  char const *dataset;                 /* name of the associated dataset */
  double     x, y, z;                  /* geocentric coordinate */
  float      yaw, pitch, roll;         /* Euler orientation */ 
  int        coordsys;                 /* coordinate system of location */
} ViewpointItem;

/* the following structures are used to return results from tvuGazetteer */

typedef struct {
  char *street;
  char *city;
  char *province;
  char *postcode;
  char *country;
} tvuAddress;

typedef struct {
  int        type;                      /* TVU_GAZ_LATLONG/CHOICES */
  double     latitude, longitude;       /* set if type is TVU_GAZ_LATLONG */
  tvuAddress *choice;                   /* set if type is TVU_GAZ_CHOICES */
  int        num_choices;               /* set if type is TVU_GAZ_CHOICES */
} tvuGazResponse;

/* define some useful subset of the ADL feature type hierarchy, see: */
/* http://alexandria.sdc.ucsb.edu/~lhill/FeatureTypes/index_tt.htm  */

#define TVU_ADL_ALL            ""
#define TVU_ADL_TRANSPORT      "transportation features"
#define TVU_ADL_BEACHES        "beaches"
#define TVU_ADL_BUILDINGS      "buildings"
#define TVU_ADL_HYDROGRAPHIC   "hydrographic structures"
#define TVU_ADL_DESERTS        "deserts"
#define TVU_ADL_FORESTS        "forests"
#define TVU_ADL_HISTORICAL     "historical sites"
#define TVU_ADL_LAKES          "lakes"
#define TVU_ADL_MILITARY       "military areas"
#define TVU_ADL_MONUMENTS      "monuments"
#define TVU_ADL_MOUNTAINS      "mountains"
#define TVU_ADL_PARKS          "parks"
#define TVU_ADL_PLACES         "populated places"
#define TVU_ADL_REC_FACIL      "recreational facilities"
#define TVU_ADL_STREAMS        "streams"
#define TVU_ADL_VALLEYS        "valleys"
#define TVU_ADL_VOLCANIC       "volcanic features"
#define TVU_ADL_COUNTRIES      "countries"
#define TVU_ADL_1ST_REGIONS    "countries, 1st order divisions"
#define TVU_ADL_2ND_REGIONS    "countries, 2nd order divisions"
#define TVU_ADL_3RD_REGIONS    "countries, 3rd order divisions"
#define TVU_ADL_4TH_REGIONS    "countries, 4th order divisions"

typedef struct {
  int        matches;                   /* number of matches returned here */
  int        total_matches;             /* total number of matches found */
  int        *point_source;             /* is each match a point or area */
  double     *north;                    /* list of upper latitude coords */
  double     *south;                    /* list of lower latitude coords */
  double     *east;                     /* list of upper longitude coords */
  double     *west;                     /* list of lower longitude coords */
  char       **title;                   /* list of title strings */
  char       **type;                    /* list of feature types */
  char       **id;                      /* list of ADL metadata IDs */
} tvuADLResponse;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The first function that should be called by the libtvutil 
 * client is the initialize call. Then, when completely 
 * finished, the client should call the shutdown call.
 *
 * tvuInit() takes a parameter to give some information
 * about the TerraVision application that may be used to
 * customize the behaviour of certain operations. Current
 * legal values include TVU_DEFAULT or TVU_PLUGIN.
 *
 */

void tvuInit( int mode );
void tvuFinish( void );

/*
 * Two routines for loading the config.xml configuration file
 * and parsing a list of command line arguments:
 *
 *  - tvuParseArgArray()  works on a C-style argc/argv array
 *  - tvuParseArgString() works on a single string, as used by MS
 *
 * In each case, the functions return the number of arguments
 * that were found and parsed. You can then use the tvuGet...()
 * calls, below, to query the value of settings.
 *
 */

int tvuParseArgArray( int argc, char **argv );
int tvuParseArgString( char const *args );

/*
 * The following routines let the client find out the values 
 * that were specified on the command line or in the config
 * file, as well as other library and system state. The token
 * can be one of the TVU_* defines provided above.
 *
 *   - tvuGetInt()     returns an int/boolean value
 *   - tvuGetFloat()   returns a floating point value
 *   - tvuGetString()  returns a string value (do not free)
 *   - tvuGetStringN() returns one string from a list (do not free) 
 *
 * You can also change the value of these settings using one of
 * the following set functions. Each returns a value of TRUE if
 * the setting was updated, or FALSE if failed.
 *
 *   - tvuSetInt()     set an int/boolean value
 *   - tvuSetFloat()   set a floating point value
 *   - tvuSetString()  set a string value (will be strdup'd)
 *
 * N.B. the comment for each TVU define above defines the type
 * of the token, e.g. I mean integer (use tvuGetInt), F means
 * float (use tvuGetFloat), S means string (use tvuGetString),
 * and L means a string list (use tvuGetStringN). B means 
 * boolean - just use tvuGetInt/tvuSetInt for this.
 *
 */

int tvuGetInt( int token );
float tvuGetFloat( int token );
char *tvuGetString( int token );
char *tvuGetStringN( int token, int n );

int tvuSetInt( int token, int value );
int tvuSetFloat( int token, float value );
int tvuSetString( int token, char const *value );

/*
 * The following routines let you manage the loading, updating,
 * and saving of the TerraVision configuration file, config.xml.
 * Under Unix, this is stored in the $HOME/.tv directory. Under
 * Windows, the directory is found from the registry. Use the
 * tvuGet...() calls to query the value of settings after
 * loading the config file.
 *
 * N.B. you will not normally need to call tvuLoadConfig() as
 * this is performed for you in the tvuParseArg...() calls. That
 * way, the settings in the config.xml file may be overrided by
 * those on the command line.
 *
 *   - tvuLoadConfig()  loads the config.xml file
 *   - tvuSaveConfig()  saves the config.xml file
 *
 * You can also update the contents of the config file by using
 * the tvuAddConfig?() calls. The value of "option" in each case
 * can be one of the following.
 *
 *   option          value1           value2
 *   ------          ------           ------
 *   "dataset"       OI url           DEM url
 *   "geovrml"       .wrl url         n/a
 *   "option"        option name      option value
 *   "schema"        schema url       n/a
 *   "database"      dbase url        n/a
 *   "proxy"         proxy url        n/a
 *   "viewpoint"     bookmark url     n/a
 *   "path"          path url         n/a
 *   "license"       accepted ver     n/a
 *   "oaa"           hostname         n/a
 *   "command"       cmd string       n/a
 *
 */

int tvuLoadConfig( void );
int tvuSaveConfig( void );

int tvuAddConfig1( char const *option, char const *value );
int tvuAddConfig2( char const *option, char const *value1, char const *value2);

/*
 * These next functions are used to manage TerraVision's
 * viewpoint bookmarks file. This file contains a list of
 * saved viewpoints that TerraVision can be made to return to.
 * Provided here are routines to load and save the viewpoint
 * file, as well as add, delete, and query the loaded list.
 *
 *   - tvuLoadViewpoints()   Loads the viewpoints.xml file
 *   - tvuSaveViewpoints()   Saves the viewpoints.xml file
 *   - tvuFreeViewpoints()   Frees all memory for the viewpoints
 *   - tvuAddViewpoint()     Adds a viewpoint to list (strdup all strings)
 *   - tvuDeleteViewpoint()  Removes and frees the specified viewpoint
 *   - tvuFindViewpoint()    Find and return the names viewpoint
 *   - tvuViewpoint()        Return the n'th viewpoint in the list
 *   - tvuViewpointCount()   Return the number of viewpoints in the list
 *
 */

int tvuLoadViewpoints( void );
int tvuSaveViewpoints( void );
void tvuFreeViewpoints( void );

int tvuAddViewpoint( ViewpointItem *new_view );
int tvuDeleteViewpoint( char const *geoname, char const *del_string );

ViewpointItem *tvuFindViewpoint( char const *geoname, char const *search );
ViewpointItem *tvuViewpoint( int i );
int tvuViewpointCount( void );

/*
 * The following function is used to initialise a tvBase structure.
 * e.g. with information about the .geo infrastructure to connect to.
 * This includes a URL to the .geo schema file to download, and
 * optionally a profile file. The actual URLs are
 * specified on the command line (-schema and -profile), or in
 * the user's configuration file. If these are not specified then
 * no action is taken. You should call this function prior to doing
 * a tvBaseStart() on the tvBase structure.
 *
 *  - tvuInitCore()   Set the .geo schema and profile for a tvBase
 *
 */

void tvuInitCore( tvBase *tvbase );

/*
 * The tvuGetContext() call will return you an glmContext structure
 * that sets up an appropriate visual for doing OpenGL rendering.
 * This function will try to find the best accelerated visual that
 * is available. It will also check any command line arguments or
 * configuration file entries to see if a visual has been specified
 * there and use that.
 *
 *  - tvuGetContext()   Find an accelerated OpenGL visual
 *
 * Under X/Motif, handle should be the Widget of the main form.
 * Under Windows, handle should be the HWND of the window.
 *
 */

glmContext tvuGetContext( void *handle );

/*
 * The tvutils library also provides a "gazetteer" service, that is,
 * the ability to convert a street address into a latitude/longitude
 * location. It does this by using a web service to do that actual
 * translation. The result can either be an exact match, in which 
 * case you get a lat/long back, or a list of places that matched
 * your query.
 *
 *  - tvuGazetteer()        convert a street address to a lat/long location
 *  - tvuFreeGazResponse()  frees the structure returned by tvuGazetteer
 *
 * You can set any of the input strings to NULL if you don't know it.
 * Currently, the street string is ignored.
 *
 */

tvuGazResponse *tvuGazetteer( char *street, char *city, char *province,
			      char *postcode, char *country );

tvuGazResponse *tvuRetryGazetteer( tvuGazResponse *response, int choice );

void tvuFreeGazResponse( tvuGazResponse *response );


tvuADLResponse *tvuADLQuery( char *query, char *feature_type, double area[4],
			     int max_matches );

void tvuADLFreeResponse( tvuADLResponse *response );


/*
 * Finally, we have a section for generally useful miscellany.
 * Nothing here yet.
 *
 */



#ifdef __cplusplus
}
#endif

#endif
