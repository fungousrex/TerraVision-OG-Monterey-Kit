/*
 * Name: tsmConnectionParams.c
 *
 * Description: 
 *   Contains user-level routines for controlling an extensible system
 *   of providing optional hints to tsmConnect. These let you specify
 *   things like the particular set ID that you want to connect to (DPSS), or
 *   the image format of the dataset you want to open, or the particular
 *   server combination (DPSS).
 *
 * Function List:
 *
 * Revision History:
 *   Created 12 Jan 1998 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmConnectionParams.c,v 1.33.2.3 2001/06/20 22:26:15 reddy Exp $
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

#include "tsmInternal.h"
#include "tsmConnectionParamsInt.h"

/*
 * Synopsis:
 *   int tsmInitParams( TsmConnectionParams *params )
 *
 * Doc++ Description: *//** 
 *   Initialises a connection params structure. 
 * 
 *   The connection params structure is an extensible means to provide
 *   optional hints to tsmConnect. These are used to specify a continuum
 *   from the specific dataset you want to open to general properties 
 *   about a dataset that you are looking for.
 *
 *   You must call this function before calling tsmSetParam, and you should
 *   free up the param structure when you're finished with it by using
 *   tsmFreeParams.
 *
 * @param    params a pointer to a TsmConnectionParams to allocate.
 *
 * @return   TRUE if all went just peachy, FALSE if it went pear shaped
 *
 * @author   Martin Reddy
 * @version  6 February 1998
 *
 * @see      tsmConnect, tsmSetParam, tsmFreeParams, tsmPrintParams
 *
 */

int
tsmInitParams( TsmConnectionParams *params )
{
  const char *funcName = "tsmInitParam";
  const int  size = sizeof( struct TsmConnectionParamsStruct );

  if ( params == NULL ) return FALSE;

  if ( ! ( *params = (TsmConnectionParams) tsmMemCalloc( size, 1 ) ) ) {
    tsmError( funcName, "Couldn't allocate a connection param structure" );
    return FALSE;
  }

  (*params)->flag           = TSM_PARAM_NONE;
  (*params)->numServers     = 0;
  (*params)->server         = NULL;
  (*params)->set_id         = -1;
  (*params)->format         = TSM_NONE_FORMAT;
  (*params)->tile_format    = TSM_TIFF_TILES;
  (*params)->fallback       = TRUE;
  (*params)->validate_tspec = TRUE;
  (*params)->dpss_tspec     = TRUE;
  (*params)->new_tspec      = TRUE;
  (*params)->vrml_dir       = tsmMemStrdup( "." );
  (*params)->texUrns        = NULL;
  (*params)->nTexUrns       = 0;
  (*params)->lcs            = NULL;
  (*params)->num_lcs        = 0;
  (*params)->num_polys      = 8;
  (*params)->num_lods       = 4;
  (*params)->tree_skip      = 1;
  (*params)->gaussian       = FALSE;
  (*params)->collision      = FALSE;
  (*params)->test_vtiles    = FALSE;
  (*params)->start_level    = -1;
  (*params)->end_level      = -1;
  (*params)->texel_scale    = 1.0;
  (*params)->world_scale    = 1.0;
  (*params)->elev_scale     = 1.0;
  (*params)->solid          = FALSE;
  (*params)->flat           = TSM_GEOCNTRC_VTILES;
  (*params)->geotiles       = TRUE;
  (*params)->normals        = FALSE;
  (*params)->vrml_models    = TRUE;
  (*params)->vrml_textures  = TRUE;
  (*params)->fast_bbox      = FALSE;
  (*params)->vrml_url       = tsmMemStrdup( "../.." );
  (*params)->blocking_read  = FALSE;
  (*params)->root_level     = 0;
  (*params)->include_vtiles = FALSE;
  (*params)->full_dpss_read = FALSE;
  (*params)->wms            = NULL;

  return TRUE;
}

/*
 * Synopsis:
 *   void tsmSetParam(TsmConnectionParams params, TsmParamCode param_code,...)
 *
 * Doc++ Description: *//** 
 *   Set a connection param field.
 * 
 *   The connection params structure is an extensible means to provide
 *   optional hints to tsmConnect. These are used to specify a continuum
 *   from the specific dataset you want to open to general properties 
 *   about a dataset that you are looking for.
 *
 *   This function uses varargs for the actual parameter value so that you
 *   can pass in values of any time. This does mean however that you should
 *   be careful to supply a value of the correct type for the parameter code
 *   that you specify, e.g. tsmSetParam( p, TSM_PARAM_SETID, 69 ) because
 *   TSM_PARAM_SETID is expecting an int.
 *
 *   You must call tsmInitParams before calling this function.
 * 
 *   Currently you can set the following optional hints. The type of the
 *   third parameter specifying the value is given in square parentheses.
 *
 *   \begin{itemize}
 *     \item {\bf TSM_PARAM_SETID}: the set ID of a dataset on a DPSS. This
 *           fully qualifies the dataset so no other hints are required if you 
 *           supply this hint. [int]
 *     \item {\bf TSM_PARAM_SERVERS}: you can specify a list of server names
 *           that are used by the dataset on a DPSS. You supply server names
 *           one at a time, i.e. multiple tsmSetParam calls are required to
 *           define multiple server names. [char *]
 *     \item {\bf TSM_PARAM_FORMAT}: you can specify the image format that
 *           you want the dataset to be in. This can be: TSM_LRBT_FORMAT or
 *           TSM_JPEG_FORMAT. [int]
 *     \item {\bf TSM_PARAM_TIFF}: if you are opening a localdisk dataset,
 *           then you can use this hint to specify whether you want to read
 *           (or write, if opening a write connection)
 *           raw .tile format tiles (FALSE), or the blocked .tiff format
 *           tiles (TRUE). [int]
 *     \item {\bf TSM_PARAM_VALIDATE_TSPEC}: if this is TRUE, then whenever
 *           the tspec file is loaded for the connection, the tsmApi will
 *           validate it (with tsmValidateTspec) to see if it contains any
 *           potential errors. If you set this value to FALSE, then the 
 *           validate routine is by-passed (TRUE). [int]
 *     \item {\bf TSM_PARAM_FALLBACK}: this hint lets you specify what
 *           should be done if your specified dataset does not exist. If
 *           this value is TRUE, then tsmApi will attempt to find another
 *           version of the same dataset for you to use. If this value is
 *           FALSE, then tsmApi will immediately fail if the requested
 *           dataset is not available. [int]
 *     \item {\bf TSM_PARAM_NEW_TSPEC}: this hint controls the reading of
 *           tspec data from localdisk. If this value is set to TRUE, then
 *           tsmConnect will try to load the tiles.tspec single-file
 *           tspec. If this value is FALSE, then the old multi-file tspec
 *           format will be used. [int]
 *     \item {\bf TSM_PARAM_DPSS_TSPEC}: controls the reading of tspec
 *           data from a DPSS. If this value is set to TRUE, then tsmConnect
 *           will try to load the tspec file from the DPSS dataset itself.
 *           If this value is FALSE, then the various metadata URLs 
 *           associated with that set are tried to see if they lead to
 *           a valid tspec file. [int]
 *     \item {\bf TSM_PARAM_BLOCKING_READ} : if set to TRUE, tsmReadTile will
 *           block if there are no tiles to read. It will unblock once you
 *           have requested new tiles, i.e. in a different thread. If you
 *           use blocking reads, then make sure that you have a separate 
 *           reader thread for each connection. The default is FALSE. [int]
 *     \item {\bf TSM_PARAM_REQUEST_CHECK} : if set to TRUE, tsmReqTileAdd
 *           will insert a tile request only once per request list no matter
 *           how many times it is requested.
 *   \item {\bf TSM_PARAM_VRML_DIR}: directory where this dataset will be 
 *           written to. If you supply a trailing '/', then it will be
 *           stripped. [char *]
 *   \item {\bf TSM_PARAM_TEX_URNS}: URN of terrain texture map (multiple 
 *           instances allowed) [char *]
 *   \item {\bf TSM_PARAM_LCS}: start of local coordinate system in meters
 *               (multiple instances allowed) [double]
 *   \item {\bf TSM_PARAM_NUM_POLYS}: per side on a vtile [int]
 *   \item {\bf TSM_PARAM_NUM_LODS}: number of lods in one tree file [int]
 *   \item {\bf TSM_PARAM_TREE_SKIP}: levels to skip between tree files [int]
 *   \item {\bf TSM_PARAM_GAUSSIAN}: use gaussian elevation interpolation
 *           instead of linear (TRUE or FALSE) [int]
 *   \item {\bf TSM_PARAM_COLLISION}: turn on collision detection for VRML
 *           geometry  (TRUE or FALSE) [int]
 *   \item {\bf TSM_PARAM_TEST_VTILES}: create flat, randomly-colored vtiles 
 *           (TRUE or FALSE) [int]
 *   \item {\bf TSM_PARAM_START_LEVEL}: TV start pyramid level (def=lowest-res)
 *         [int]
 *   \item {\bf TSM_PARAM_END_LEVEL}: TV end pyramid level (def=highest-res)
 *         [int]
 *   \item {\bf TSM_PARAM_TEXEL_SCALE}: scale for LOD range calc (def=1.0)
 *          [double]
 *   \item {\bf TSM_PARAM_ELEV_SCALE}: scale for elevation vs. lat/long
 *          (def=1.0) [double]
 *   \item {\bf TSM_PARAM_WORLD_SCALE}: scaling factor for the entire world
 *          (def=1.0) [double]
 *   \item {\bf TSM_PARAM_TREE_TYPE}: there are a number of ways to
 *          generate the VRML tree files. Currently you can choose between
 *          TSM_TREE_TYPE_ANCHOR (default), TSM_TREE_TYPE_LOD, or 
 *	    TSM_TREE_TYPE_QUADLOD. [int]
 *   \item {\bf TSM_PARAM_FLAT}: creates a flat world model (in lat-long
 *          coordinates), flat UTM coordinates, or a globe (in geocentric
 *          coordinates) (TSM_FLAT_VTILES, TSM_UTM_VTILES, or
 *          TSM_GEOCNTRC_VTILES) [int]
 *   \item {\bf TSM_PARAM_GEOTILES}: generates geotiles files for the tree
 *          hierarchy if this value is set to TRUE (default), otherwise the
 *          tree files link directly to the vtile files. (TRUE or FALSE) [int]
 *   \item {\bf TSM_PARAM_NORMALS}: generates per vertex normals for the vtiles
 *          if this value is set to TRUE (default), otherwise the
 *          vtiles will look flat shaded (not gouraud). (TRUE or FALSE) [int]
 *   \item {\bf TSM_PARAM_FAST_BBOXES}: when set to TRUE we use a faster
 *         bounding box calculation routine for tree files that is approximate
 *         but conservative. By default, we do a much slower check of every
 *         single vertex in a tile to compute the bounding box. (TRUE or FALSE)
 *         [int]
 *   \item {\bf TSM_PARAM_VRML_URL}: defines the absolute root URL for VRML 
 *          files. If you supply a trailing '/', then it will be stripped.
 *          [char *]
 *   \item {\bf TSM_PARAM_INCLUDE_VTILES}: includes the vtiles directly in the 
 *		tree file.
 *   \item {\bf TSM_PARAM_ROOT_LEVEL}: specifies the root level number for a
 *          tree hierarchy. The p0p0.wrl tree file at that level is defined
 *          as the entry point for viewing the hierarchy. The default value
 *          is 2, i.e. trees/2/p0p0.wrl is the default entry point.
 *          [int]
 *   \item {\bf TSM_PARAM_FULL_DPSS_READ}: this parameter is currently
 *          only used by the tsmUpdateDatabaseFromServer call. When set 
 *          to TRUE, it will perform a full re-read of any DPSS server.
 *          Otherwise, we only update information for datasets that have
 *          a loaded date that is later than the last time the server
 *          was read.
 *   \item {\bf TSM_PARAM_WMS}: a Web Mapping Server structure
 *   \end{itemize}
 *
 * @param    params a TsmConnectionParams structure to update
 * @param    param_code one of TSM_PARAM_SETID, TSM_PARAM_SERVERS,
 *           TSM_PARAM_FORMAT, etc.
 * @param    param_value the value to set for the specified field, e.g.
 *           a set id, a new server, or an image format. The type of
 *           this value is dependent upon param_code.
 *
 * @return   TRUE if all went just peachy, FALSE if it went pear shaped
 *
 * @author   Martin Reddy
 * @version  6 February 1998
 *
 * @see      tsmConnect, tsmInitParam, tsmFreeParams, tsmPrintParams
 *
 */

void
tsmSetParam( TsmConnectionParams params, TsmParamCode param_code, ... )
{
  const char *funcName = "tsmSetParam";
  int        length;
  va_list    args;

  if ( params == NULL ) return;

  va_start( args, param_code );

  switch ( param_code ) {
  case TSM_PARAM_NONE:
    break;
  case TSM_PARAM_SETID:
    tsmSetParamFlag( params, TSM_PARAM_SETID );
    params->set_id = va_arg( args, int );
    break;
  case TSM_PARAM_SERVERS:
    tsmSetParamFlag( params, TSM_PARAM_SERVERS );
    params->numServers++;
    params->server = (char **) tsmMemManagerRealloc( params->server, 
       params->numServers * sizeof(char *) );

    if ( params->server == NULL ) {
      tsmError( funcName, "Couldn't grow server array" );
      params->numServers = 0;
    } else
      params->server[params->numServers-1] =
	tsmMemStrdup( va_arg( args, char * ) );

    break;
  case TSM_PARAM_FORMAT:
    tsmSetParamFlag( params, TSM_PARAM_FORMAT );
    params->format = va_arg( args, int );
    break;
  case TSM_PARAM_TIFF:
    tsmSetParamFlag( params, TSM_PARAM_TIFF );
    if ( va_arg( args, int ) == TRUE )
      params->tile_format = TSM_TIFF_TILES;
    else
      params->tile_format = TSM_RAW_TILES;
    break;
  case TSM_PARAM_FALLBACK:
    tsmSetParamFlag( params, TSM_PARAM_FALLBACK );
    params->fallback = va_arg( args, int );
    break;
  case TSM_PARAM_VALIDATE_TSPEC:
    tsmSetParamFlag( params, TSM_PARAM_VALIDATE_TSPEC );
    params->validate_tspec = va_arg( args, int );
    break;
  case TSM_PARAM_DPSS_TSPEC:
    tsmSetParamFlag( params, TSM_PARAM_DPSS_TSPEC );
    params->dpss_tspec = va_arg( args, int );
    break;
  case TSM_PARAM_NEW_TSPEC:
    tsmSetParamFlag( params, TSM_PARAM_NEW_TSPEC );
    params->new_tspec = va_arg( args, int );
    break;
  case TSM_PARAM_BLOCKING_READ:
    tsmSetParamFlag( params, TSM_PARAM_BLOCKING_READ );
    params->blocking_read = va_arg( args, int );
    break;
  case TSM_PARAM_REQUEST_CHECK:
    tsmSetParamFlag( params, TSM_PARAM_REQUEST_CHECK );
    params->request_check = va_arg( args, int );
    break;
  case TSM_PARAM_VRML_DIR:       
    tsmSetParamFlag(params, TSM_PARAM_VRML_DIR);
    if ( params->vrml_dir ) tsmMemFree( params->vrml_dir );
    params->vrml_dir = tsmMemStrdup( va_arg( args, char * ) );

    /* Strip trailing '/' if given */

    if ( params->vrml_dir != NULL ) {
      length = strlen( params->vrml_dir );
      if ( length > 0 && params->vrml_dir[length-1] == '/' )
	params->vrml_dir[length-1] = '\0';
    }

    break;
  case TSM_PARAM_TEX_URNS:       
    tsmSetParamFlag(params, TSM_PARAM_TEX_URNS);
    params->nTexUrns++;
    params->texUrns = (char **) tsmMemManagerRealloc( params->texUrns, 
       params->nTexUrns * sizeof(char *) );

    if ( params->texUrns == NULL ) {
      tsmError( funcName, "Couldn't grow texUrn array" );
      params->nTexUrns = 0;
    } else
      params->texUrns[params->nTexUrns-1] =
         tsmMemStrdup( va_arg( args, char * ) );

    break;
  case TSM_PARAM_LCS:            
    tsmSetParamFlag(params, TSM_PARAM_LCS);
    params->lcs = (double *) tsmMemManagerRealloc( params->lcs, 
       (params->num_lcs+1) * sizeof(double) );

    if ( params->lcs == NULL ) {
      tsmError( funcName, "Couldn't grow LCS array" );
      params->num_lcs = 0;
    } else {
      int j, k;
      double value;

      value = va_arg( args, double );

      /* insert sort new LCS val in *descending* order */
      for (j = 0; j < params->num_lcs; j++)
	if ( value > params->lcs[j])
	  break;
      for (k = params->num_lcs; k > j; k--) 
	params->lcs[k] = params->lcs[k-1];
      params->lcs[j] = value;
      params->num_lcs++;
    }

    break;
  case TSM_PARAM_NUM_POLYS:      
    tsmSetParamFlag(params, TSM_PARAM_NUM_POLYS);
    params->num_polys = va_arg( args, int );
    break;
  case TSM_PARAM_NUM_LODS:       
    tsmSetParamFlag(params, TSM_PARAM_NUM_LODS);
    params->num_lods = va_arg( args, int );
    break;
  case TSM_PARAM_TREE_SKIP:      
    tsmSetParamFlag(params, TSM_PARAM_TREE_SKIP);
    params->tree_skip = va_arg( args, int );
    break;
  case TSM_PARAM_GAUSSIAN:       
    tsmSetParamFlag(params, TSM_PARAM_GAUSSIAN);
    params->gaussian = va_arg( args, int );
    break;
  case TSM_PARAM_COLLISION:      
    tsmSetParamFlag(params, TSM_PARAM_COLLISION);
    params->collision = va_arg( args, int );
    break;
  case TSM_PARAM_TEST_VTILES:    
    tsmSetParamFlag(params, TSM_PARAM_TEST_VTILES);
    params->test_vtiles = va_arg( args, int );
    break;
  case TSM_PARAM_START_LEVEL:    
    tsmSetParamFlag(params, TSM_PARAM_START_LEVEL);
    params->start_level = va_arg( args, int );
    break;
  case TSM_PARAM_END_LEVEL:      
    tsmSetParamFlag(params, TSM_PARAM_END_LEVEL);
    params->end_level = va_arg( args, int );
    break;
  case TSM_PARAM_TEXEL_SCALE:    
    tsmSetParamFlag(params, TSM_PARAM_TEXEL_SCALE);
    params->texel_scale = va_arg( args, double );
    break;
  case TSM_PARAM_ELEV_SCALE:    
    tsmSetParamFlag(params, TSM_PARAM_ELEV_SCALE);
    params->elev_scale = va_arg( args, double );
    break;
  case TSM_PARAM_WORLD_SCALE:    
    tsmSetParamFlag(params, TSM_PARAM_WORLD_SCALE);
    params->world_scale = va_arg( args, double );
    break;
  case TSM_PARAM_FLAT:    
    tsmSetParamFlag(params, TSM_PARAM_FLAT);
    params->flat = va_arg( args, int );
    break;
  case TSM_PARAM_TREE_TYPE:    
    tsmSetParamFlag(params, TSM_PARAM_TREE_TYPE);
    params->tree_type = va_arg( args, int );
    break;
  case TSM_PARAM_GEOTILES:
    tsmSetParamFlag(params, TSM_PARAM_GEOTILES);
    params->geotiles = va_arg( args, int );
    break;
  case TSM_PARAM_NORMALS:
    tsmSetParamFlag(params, TSM_PARAM_NORMALS);
    params->normals = va_arg( args, int );
    break;
  case TSM_PARAM_MODELS:
    tsmSetParamFlag(params, TSM_PARAM_MODELS);
    params->vrml_models = va_arg( args, int );
    break;
  case TSM_PARAM_SOLID:
    tsmSetParamFlag(params, TSM_PARAM_SOLID);
    params->solid = va_arg( args, int );
    break;
  case TSM_PARAM_TEXTURES:
    tsmSetParamFlag(params, TSM_PARAM_TEXTURES);
    params->vrml_textures = va_arg( args, int );
    break;
  case TSM_PARAM_FAST_BBOXES:
    tsmSetParamFlag(params, TSM_PARAM_FAST_BBOXES);
    params->fast_bbox = va_arg( args, int );
    break;
  case TSM_PARAM_VRML_URL:
    tsmSetParamFlag(params, TSM_PARAM_VRML_URL);
    if ( params->vrml_url ) tsmMemFree( params->vrml_url );

    params->vrml_url = tsmMemStrdup( va_arg( args, char * ) );

    /* Strip trailing '/' if given */

    if ( params->vrml_url != NULL ) {
      length = strlen( params->vrml_url );
      if ( length > 0 && params->vrml_url[length-1] == '/' )
	params->vrml_url[length-1] = '\0';
    }

    break;
  case TSM_PARAM_ROOT_LEVEL:
    tsmSetParamFlag(params, TSM_PARAM_ROOT_LEVEL);
    params->root_level = va_arg( args, int );
    break;
  case TSM_PARAM_INCLUDE_VTILES:
    tsmSetParamFlag(params, TSM_PARAM_INCLUDE_VTILES);
    params->include_vtiles = va_arg( args, int );
    break;
  case TSM_PARAM_FULL_DPSS_READ:
    tsmSetParamFlag(params, TSM_PARAM_FULL_DPSS_READ);
    params->full_dpss_read = va_arg( args, int );
    break;
  case TSM_PARAM_READ_MULTI:
    tsmSetParamFlag(params, TSM_PARAM_READ_MULTI);
    params->read_multi = va_arg( args, int );
    break;
  case TSM_PARAM_WRITE_MULTI:
    tsmSetParamFlag(params, TSM_PARAM_WRITE_MULTI);
    params->write_multi = va_arg( args, int );
    break;
  case TSM_PARAM_WMS:
    tsmSetParamFlag(params, TSM_PARAM_WMS);
    params->wms = (tsmWMS *) va_arg( args, char * );
    break;
  default:
    tsmError( funcName, "Unsupported parameter code: %d", param_code );
  }

  va_end( args );
}

/*
 * Synopsis:
 *   void tsmFreeParams( TsmConnectionParams params )
 *
 * Doc++ Description: *//** 
 *   Frees a connection params structure. 
 * 
 *   The connection params structure is an extensible means to provide
 *   optional hints to tsmConnect. These are used to specify a continuum
 *   from the specific dataset you want to open to general properties 
 *   about a dataset that you are looking for.
 *
 *   You should call this function once you no longer need the connection
 *   params structure, i.e. any time after you have opened all connections
 *   that required this params structure.
 *
 * @param    params the TsmConnectionParams to deallocate.
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  6 February 1998
 *
 * @see      tsmConnect, tsmInitParam, tsmSetParam, tsmPrintParams
 *
 */

void
tsmFreeParams( TsmConnectionParams params )
{
  int i;

  if ( params == NULL ) return;

  if ( params->numServers > 0 ) {
    for ( i = 0; i < params->numServers; ++i )
      tsmMemFree( params->server[i] );
    tsmMemFree( params->server );
  }

  if ( params->nTexUrns > 0 ) {
    for ( i = 0; i < params->nTexUrns; ++i )
      tsmMemFree( params->texUrns[i] );
    tsmMemFree( params->texUrns );
  }

  if ( params->lcs != NULL )
    tsmMemFree( params->lcs );

  if ( params->vrml_dir != NULL )
    tsmMemFree( params->vrml_dir );

  if ( params->vrml_url != NULL )
    tsmMemFree( params->vrml_url );

  tsmMemFree( params );
}

/*
 * Synopsis:
 *   TsmConnectionParams tsmCopyParams( TsmConnectionParams params )
 *
 * Doc++ Description: *//** 
 *   Copies a connection params structure. 
 * 
 *   The connection params structure is an extensible means to provide
 *   optional hints to tsmConnect. These are used to specify a continuum
 *   from the specific dataset you want to open to general properties 
 *   about a dataset that you are looking for.
 *
 *   This function is used to do a (deep) copy of an already existing
 *   params structure. It will produce an identical copy of the structure
 *   with all strings and arrays replicated also.
 *
 *   If you pass in NULL, then the function will return NULL.
 *
 * @param    params the TsmConnectionParams to copy.
 *
 * @return   the new TsmConnectionParams structure
 *
 * @author   Martin Reddy
 * @version  23 November 1998
 *
 * @see      tsmConnect, tsmInitParam, tsmSetParam, tsmFreeParams,
 *           tsmPrintParams
 *
 */

TsmConnectionParams
tsmCopyParams( TsmConnectionParams params )
{
  TsmConnectionParams new_params;
  int i;

  if ( params == NULL ) return NULL;

  /* Allocate a new TsmConnectionParams and do a bit copy */

  tsmInitParams( &new_params );

  /* Save allocated fields. */
  {
    char *vrml_dir = new_params->vrml_dir;
    char *vrml_url = new_params->vrml_url;

    /* Copy all fields. */
    *new_params = *params;

    /* Restore allocated fields. */
    new_params->vrml_dir = vrml_dir;
    new_params->vrml_url = vrml_url;
  }

  /* Now make copies of all dynamic memory structures */

  if ( params->numServers > 0 )
    for ( i = 0; i < params->numServers; ++i )
      tsmSetParam( new_params, TSM_PARAM_SERVERS, params->server[i] );

  if ( params->nTexUrns > 0 )
    for ( i = 0; i < params->nTexUrns; ++i )
      tsmSetParam( new_params, TSM_PARAM_TEX_URNS, params->texUrns[i] );

  if ( params->lcs != NULL )
    /* Nothing done for this just yet - not used anyway */;

  if ( params->vrml_dir != NULL )
    tsmSetParam( new_params, TSM_PARAM_VRML_DIR, params->vrml_dir );

  if ( params->vrml_url != NULL )
    tsmSetParam( new_params, TSM_PARAM_VRML_URL, params->vrml_url );

  return new_params;
}

/*
 * Synopsis:
 *   int tsmPrintParams( TsmConnectionParams params )
 *
 * Doc++ Description: *//** 
 *   Displays a connection params settings.
 * 
 *   The connection params structure is an extensible means to provide
 *   optional hints to tsmConnect. These are used to specify a continuum
 *   from the specific dataset you want to open to general properties 
 *   about a dataset that you are looking for.
 *
 *   This function lets you easily see which hints have been specified
 *   so far, and what their values are. If you have not specified a
 *   particular hint, then it will be displayed as <not set>.
 *
 *   You can direct the output of this function to any stream, e.g.
 *   a file on disk, or stdout, stderr, etc.
 *
 * @param    params the TsmConnectionParams structure to display
 * @param    handle the stream to write the description out to
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  6 February 1998
 *
 * @see      tsmConnect, tsmInitParams, tsmSetParam, tsmFreeParams
 *
 */

void
tsmPrintParams( TsmConnectionParams params, FILE *handle )
{
  /*static const char funcName[] = "tsmPrintParams";*/
  const char *not_set = "<not set>\n";
  int        i;

  if ( params == NULL || handle == NULL ) return;

  /* output the format parameter setting */
  
  fprintf( handle, "image format : " );
  if ( tsmIsParamSet( params, TSM_PARAM_FORMAT ) ) {
    switch ( params->format ) {
    case TSM_LRBT_FORMAT:
      fprintf( handle, "lrbt-raster\n" );
      break;
    case TSM_JPEG_FORMAT:
      fprintf( handle, "jpeg-compressed\n" );
      break;
    case TSM_LZW_FORMAT:
      fprintf( handle, "lzw-compressed\n" );
      break;
    case TSM_MIPMAP_FORMAT:
      fprintf( handle, "mip-mapped\n" );
      break;
    default:
      fprintf( handle, "none\n" );
    }
  } else
    fprintf( handle, not_set );
	
  /* output the tile format parameter setting */

  fprintf( handle, "tile format : " );
  if ( tsmIsParamSet( params, TSM_PARAM_TIFF ) ) 
    fprintf( handle, "%s\n", ( params->tile_format == TSM_TIFF_TILES ) ?
	     "tiff" : "tile" );
  else
    fprintf( handle, not_set );

  /* output the validate tspec parameter setting */

  fprintf( handle, "validate tspec : " );
  if ( tsmIsParamSet( params, TSM_PARAM_VALIDATE_TSPEC ) ) 
    fprintf( handle, "%s\n", ( params->validate_tspec ) ? "yes" : "no" );
  else
    fprintf( handle, not_set );

  /* output the fallback parameter setting */

  fprintf( handle, "fallback : " );
  if ( tsmIsParamSet( params, TSM_PARAM_FALLBACK ) ) 
    fprintf( handle, "%s\n", ( params->fallback ) ? "yes" : "no" );
  else
    fprintf( handle, not_set );

  /* output the DPSS tspec parameter setting */

  fprintf( handle, "DPSS tspec : " );
  if ( tsmIsParamSet( params, TSM_PARAM_DPSS_TSPEC ) ) 
    fprintf( handle, "%s\n", ( params->dpss_tspec ) ? "dpss" : "metadata" );
  else
    fprintf( handle, not_set );

  /* output the set id parameter setting */

  fprintf( handle, "set-id : " );
  if ( tsmIsParamSet( params, TSM_PARAM_SETID ) ) 
    fprintf( handle, "%d\n", params->set_id );
  else
    fprintf( handle, not_set );

  /* output the server list parameter setting */

  fprintf( handle, "servers:" );
  if ( tsmIsParamSet( params, TSM_PARAM_SERVERS ) ) {
    for ( i = 0; i < params->numServers; ++i )
      fprintf( handle, " %s", params->server[i] );
    fprintf( handle, "\n" );
  } else
    fprintf( handle, not_set );

  /* output the blocking read setting */

  fprintf( handle, "blocking reads : " );
  if ( tsmIsParamSet( params, TSM_PARAM_BLOCKING_READ ) ) 
    fprintf( handle, "%s\n", ( params->set_id ) ? "yes" : "no" );
  else
    fprintf( handle, not_set );

  fprintf(handle, "full DPSS server reads: ");
  if ( tsmIsParamSet( params, TSM_PARAM_FULL_DPSS_READ ) ) 
    fprintf( handle, "%s\n", ( params->full_dpss_read ) ? "yes" : "no" );
  else
    fprintf( handle, not_set );

  fprintf(handle, "TerraVRML parameters:\n");
  fprintf(handle, "  VRML directory: ");
  if ( tsmIsParamSet(params, TSM_PARAM_VRML_DIR)) {
    fprintf(handle, "%s\n", params->vrml_dir);
  }
  else
    fprintf(handle, not_set);

  fprintf(handle, "  Terrain Texture URN's: ");
  if ( tsmIsParamSet(params, TSM_PARAM_TEX_URNS)) {
    for ( i = 0; i < params->nTexUrns; ++i )
      fprintf( handle, " %s", params->texUrns[i] );
    fprintf( handle, "\n" );
  }
  else
    fprintf(handle, not_set);

  fprintf(handle, "  Local Coordinate Systems: ");
  if (tsmIsParamSet(params, TSM_PARAM_LCS)) {
    for ( i = 0; i < params->num_lcs; ++i )
      fprintf( handle, " %1.1f", params->lcs[i] );
    fprintf( handle, "\n" );
  }
  else
    fprintf(handle, not_set);

  fprintf(handle, "  polys per tile side: ");
  if (tsmIsParamSet(params, TSM_PARAM_NUM_POLYS))
    fprintf(handle, "%d\n", params->num_polys);
  else
    fprintf(handle, not_set);

  fprintf(handle, "  LOD's per tree file: ");
  if (tsmIsParamSet(params, TSM_PARAM_NUM_LODS))
    fprintf(handle, "%d\n", params->num_lods);
  else
    fprintf(handle, not_set);

  fprintf(handle, "  Levels skipped between tree files: ");
  if (tsmIsParamSet(params, TSM_PARAM_TREE_SKIP))
    fprintf(handle, "%d\n", params->tree_skip);
  else
    fprintf(handle, not_set);

  fprintf(handle, "  gaussian interpolation: ");
  if (tsmIsParamSet(params, TSM_PARAM_GAUSSIAN)) {
    if (params->gaussian) 
      fprintf(handle, "on\n");
    else
      fprintf(handle, "off\n");
  }
  else
    fprintf(handle, not_set);

  fprintf(handle, "  collision detection: ");
  if (tsmIsParamSet(params, TSM_PARAM_COLLISION))
    if (params->collision) 
      fprintf(handle, "on\n");
    else
      fprintf(handle, "off\n");
  else
    fprintf(handle, not_set);

  fprintf(handle, "  produce test vtiles: ");
  if (tsmIsParamSet(params, TSM_PARAM_TEST_VTILES))
    if (params->test_vtiles) 
      fprintf(handle, "on\n");
    else
      fprintf(handle, "off\n");
  else
    fprintf(handle, not_set);

  fprintf(handle, "  start level: ");
  if (tsmIsParamSet(params, TSM_PARAM_START_LEVEL))
    fprintf(handle, "%d\n", params->start_level);
  else
    fprintf(handle, not_set);

  fprintf(handle, "  end level: ");
  if (tsmIsParamSet(params, TSM_PARAM_END_LEVEL))
    fprintf(handle, "%d\n", params->end_level);
  else
    fprintf(handle, not_set);

  fprintf(handle, "  include vtiles in tree: ");
  if (tsmIsParamSet(params, TSM_PARAM_INCLUDE_VTILES))
    fprintf(handle, "%s\n", ( params->include_vtiles ) ? "on" : "off" );

  fprintf(handle, "  texel scale: ");
  if (tsmIsParamSet(params, TSM_PARAM_TEXEL_SCALE))
    fprintf(handle, "%1.1f\n", params->texel_scale);
  else
    fprintf(handle, not_set);

  fprintf(handle, "  elevation scale: ");
  if (tsmIsParamSet(params, TSM_PARAM_ELEV_SCALE))
    fprintf(handle, "%1.1f\n", params->elev_scale);
  else
    fprintf(handle, not_set);

  fprintf(handle, "  world scale: ");
  if (tsmIsParamSet(params, TSM_PARAM_WORLD_SCALE))
    fprintf(handle, "%1.1f\n", params->world_scale);
  else
    fprintf(handle, not_set);

  fprintf(handle, "  tree files: ");
  if (tsmIsParamSet(params, TSM_PARAM_TREE_TYPE)) {
    switch (params->tree_type) {
      case TSM_TREE_TYPE_ANCHOR:
	fprintf(handle, "Anchor");
	break;
      case TSM_TREE_TYPE_QUADLOD:
	fprintf(handle, "QuadLOD");
	break;
      case TSM_TREE_TYPE_LOD:
	fprintf(handle, "LOD");
	break;
      default:
	fprintf(handle, "Unknown");
	break;
    }
  }
  else
    fprintf(handle, not_set);

  fprintf(handle, "  geotiles: ");
  if (tsmIsParamSet(params, TSM_PARAM_GEOTILES))
    fprintf(handle, "%s\n", ( params->geotiles ) ? "on" : "off" );
  else
    fprintf(handle, not_set);

  fprintf(handle, "  normals: ");
  if (tsmIsParamSet(params, TSM_PARAM_NORMALS))
    fprintf(handle, "%s\n", ( params->normals ) ? "on" : "off" );
  else
    fprintf(handle, not_set);

  fprintf(handle, "  models: ");
  if (tsmIsParamSet(params, TSM_PARAM_MODELS))
    fprintf(handle, "%s\n", ( params->vrml_models ) ? "on" : "off" );
  else
    fprintf(handle, not_set);

  fprintf(handle, "  onesided: ");
  if (tsmIsParamSet(params, TSM_PARAM_SOLID))
    fprintf(handle, "%s\n", ( params->solid ) ? "on" : "off" );
  else
    fprintf(handle, not_set);

  fprintf(handle, "  textures: ");
  if (tsmIsParamSet(params, TSM_PARAM_TEXTURES))
    fprintf(handle, "%s\n", ( params->vrml_textures ) ? "on" : "off" );
  else
    fprintf(handle, not_set);

  fprintf(handle, "  fast bboxes: ");
  if (tsmIsParamSet(params, TSM_PARAM_FAST_BBOXES))
    fprintf(handle, "%s\n", ( params->fast_bbox ) ? "on" : "off" );

  fprintf(handle, "  vrml_url: ");
  if (tsmIsParamSet(params, TSM_PARAM_VRML_URL))
    fprintf(handle, "%s\n", params->vrml_url);
  else
    fprintf(handle, not_set);

  fprintf(handle, "  root level: ");
  if (tsmIsParamSet(params, TSM_PARAM_ROOT_LEVEL))
    fprintf(handle, "%d\n", params->root_level );
  else
    fprintf(handle, not_set);

  fprintf(handle, "  WMS provided: %s", (tsmIsParamSet(params, TSM_PARAM_WMS))
	  ? "yes" : "no" );

}
