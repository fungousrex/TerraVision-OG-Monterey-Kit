/* -*- tab-width: 2; -*-

  Name: GeoVRML.c
 
  Description: This file contains various utility functions for
     supporting GeoVRML nodes

  Revision History:
      $Id: GeoVRML.c,v 2.8.2.6 2002/10/31 05:26:30 reddy Exp $

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vrml2/vrml2.h>
#include <vrml2/c_nodes.h>
#include <vrml2/c_nodesI.h>
#include <vrml2/quat.h>
#include <se_conv.h>
#include "Trace.h"
#include "GeoVRML.h"
#include "TsCoords.h"
#include "ThreeDCoreI.h" /* need this for tvBasePointAltitude() */

static void   gvParseGeoSystem( GeoVRML gv, MFString * );
static double gvParserNextValue( GeoVRML gv );
static char   gvParserPeekNextChar( GeoVRML gv );

typedef enum {
  GV_NONE,
  GV_GDC,
  GV_GCC,
  GV_UTM
} GeoVRMLCS;

struct GeoVRMLStruct {
  double    origin[3];          /* the GCC origin for a node */
  vsgWorld  *wrl;               /* context used by vrml2 library */

  MFString  input;              /* input string to parse for coords */
  SFString  sfbuffer[1];        /* temp space for faking an MFString */
  int       pos;                /* parse position in string */
  int       num;                /* string number in array being parsed */

  int       utm_zone;           /* zone number for UTM coord system */
  int       utm_north_hemi;     /* is UTM coord in northern hemisphere? */
  SRM_SRF_3D_ENUM              coord_sys;   /* coord system in effect */
  SE_CONVERT_COORD_SYSTEM_PAIR conv_params; /* SEDRIS conversion params */

  tvBase    *threeD;            /* hook to the viewer's tvBase structure */
};

GeoVRML
gvNew( vsgWorld *wrl )
{
  GeoVRML gv;

  if ( ( gv = (GeoVRML) calloc( sizeof(struct GeoVRMLStruct), 1 ) ) == NULL ) 
    return NULL;

  gv->origin[0]      = 0.0;
  gv->origin[1]      = 0.0;
  gv->origin[2]      = 0.0;
  gv->wrl            = wrl;
  gv->coord_sys      = SRM_GD_3D_SRF;
  gv->utm_zone       = 0;
  gv->utm_north_hemi = 1;
  gv->pos            = 0;
  gv->num            = 0;
  gv->threeD         = NULL;
  
  return gv;
}

void
gvFree( GeoVRML gv )
{
  if (gv) {
    if (gv->conv_params) 
      SE_FreeCoordConversionConstants (gv->conv_params);
    free( gv );
  }
}

char const *
gvGetVersion( void )
{
  return "1.0";
}

void
gvSetOrigin3fv( GeoVRML gv, double origin[3] )
{
  gv->origin[0] = origin[0];
  gv->origin[1] = origin[1];
  gv->origin[2] = origin[2];
}

void
gvGetOrigin( GeoVRML gv, SFNode node, double gcc[3] )
{
  vsgEventOutDesc evout;
  symbol_t        name;
  SFNode          geoOrigin;
  MFString        *geoSystem = NULL;
  SFString        geoCoords = NULL;

  if ( node == NULL || gv == NULL ) return;

  /* get a reference to the GeoOrigin node for this GeoVRML node */

  if ( ( name = vsgIntern( gv->wrl, "geoOrigin" ) ) == NULL ) return;
  evout = vsgGetFunc( node, name );
  if ( ( geoOrigin = vsgGetSFNode( gv->wrl, node, evout ) ) == NULL ) return;

  /* now get the geoSystem MFString from the GeoOrigin node */

  if ( ( name = vsgIntern( gv->wrl, "geoSystem" ) ) != NULL ) {
    evout = vsgGetFunc( geoOrigin, name );
    geoSystem = vsgGetMFString( gv->wrl, geoOrigin, evout );
  }
  
  /* and then the geoCoords SFString from the GeoOrigin node */

  if ( ( name = vsgIntern( gv->wrl, "geoCoords" ) ) != NULL ) {
    evout = vsgGetFunc( geoOrigin, name );
    geoCoords = vsgGetSFString( gv->wrl, geoOrigin, evout );
  }

  if ( geoSystem == NULL || geoSystem->mf.len == 0 || geoCoords == NULL )
    return;

  /* parse out the GCC coordinate for the origin */

  gvParserSFInit( gv, geoCoords, geoSystem );
  gvParserNextGCC( gv, gcc );
}

void
gvSetOrigin( GeoVRML gv, SFNode node )
{
  double new_origin[3];
 
  /* need to make sure that origin is (0,0,0) while in gvGetOrigin */
  /* because this function does an applyOrigin() call. This also   */
  /* means that we can't pass gv->origin in directly as this would */
  /* mean that the origin would get updated before applyOrigin().  */

  gv->origin[0] = 0.0;
  gv->origin[1] = 0.0;
  gv->origin[2] = 0.0;

  gvGetOrigin( gv, node, new_origin );
  
  gv->origin[0] = new_origin[0];
  gv->origin[1] = new_origin[1];
  gv->origin[2] = new_origin[2];
}

char *
gvGetMetadataItem( GeoVRML gv, SFNode node, char const *token )
{
  vsgEventOutDesc evout;
  symbol_t        name;
  MFString        *summary;
  int             i;

  if ( node == NULL || gv == NULL ) return NULL;

  /* get a reference to the summary field from the GeoMetadata node */

  if ( ( name = vsgIntern( gv->wrl, "summary" ) ) == NULL )
    return NULL;
  evout = vsgGetFunc( node, name );
  if ( ( summary = vsgGetMFString( gv->wrl, node, evout ) ) == NULL )
    return NULL;

  /* now look through the summary array for the token string */

  for ( i = 0; i < summary->mf.len; i += 2 ) {
    if ( strcmp( token, summary->data[i] ) == 0 ) {
      if ( ++i < summary->mf.len )
	return strdup( summary->data[i] );
    }
  }

  /* if no match found, then return NULL */
  
  return NULL;
}


static void
gvApplyOrigin( GeoVRML gv, double gcc[3] )
{
  if ( gv != NULL ) {
    gcc[0] -= gv->origin[0];
    gcc[1] -= gv->origin[1];
    gcc[2] -= gv->origin[2];
  }
}

static void
gvRetractOrigin( GeoVRML gv, double gcc[3] )
{
  if ( gv != NULL ) {
    gcc[0] += gv->origin[0];
    gcc[1] += gv->origin[1];
    gcc[2] += gv->origin[2];
  }
}

static void
gvParseGeoSystem( GeoVRML gv, MFString *geo_system )
{
  SE_SRF_PARAMETERS in_sys;
  SE_SRF_PARAMETERS out_sys;
  int i;

  in_sys.is_2D = FALSE;
  in_sys.u.params_3d.spatial_reference_frame  = SRM_GD_3D_SRF;
  in_sys.u.params_3d.u.gd_parameters.horizontal_datum = SE_W84_HDATUM; 
  in_sys.u.params_3d.u.gd_parameters.vertical_datum   = SE_WGS84E_VDATUM;
  in_sys.u.params_3d.u.gd_parameters.elevation_units  = SRM_METERS;

  /* parse out the coordinate system information from the geoSystem strings */

  for ( i = 0; i < geo_system->mf.len; i++ ) {
    char *string = geo_system->data[i];

    /* check for a coordinate system definition */

    if ( strcmp( string, "GDC" ) == 0 )
      gv->coord_sys = SRM_GD_3D_SRF;
    else if ( strcmp( string, "GCC" ) == 0 )
      gv->coord_sys = SRM_GC_3D_SRF;
    else if ( strcmp( string, "UTM" ) == 0 )
      gv->coord_sys = SRM_UTM_3D_SRF;

    /* check for an ellipsoid definition */

    /* check for a UTM zone number */

    else if ( string[0] == 'Z' ) {
      gv->utm_zone = atoi( &string[1] );
      if ( strchr( string, 'S' ) )
	gv->utm_north_hemi = 0;
    }
      
  }

  /* setup the appropriate SEDRIS conversion structure */

  in_sys.is_2D = FALSE;
  in_sys.u.params_3d.spatial_reference_frame = gv->coord_sys;

  switch ( gv->coord_sys ) {
  case SRM_GD_3D_SRF:
    in_sys.u.params_3d.u.gd_parameters.horizontal_datum = SE_W84_HDATUM; 
    in_sys.u.params_3d.u.gd_parameters.vertical_datum   = SE_WGS84E_VDATUM;
    in_sys.u.params_3d.u.gd_parameters.elevation_units  = SRM_METERS;
    break;
  case SRM_GC_3D_SRF:
    break;
  case SRM_UTM_3D_SRF:
    in_sys.u.params_3d.u.utm_parameters.horizontal_datum = SE_W84_HDATUM; 
    in_sys.u.params_3d.u.utm_parameters.vertical_datum   = SE_WGS84E_VDATUM;
    in_sys.u.params_3d.u.utm_parameters.x_offset         = 0.0;
    in_sys.u.params_3d.u.utm_parameters.y_offset         = 0.0;
    in_sys.u.params_3d.u.utm_parameters.z_units          = SRM_METERS;
    in_sys.u.params_3d.u.utm_parameters.zone             = gv->utm_zone;
    break;
  }

  out_sys.is_2D = FALSE;
  out_sys.u.params_3d.spatial_reference_frame = SRM_GC_3D_SRF;

  SE_CreateCoordConversionConstants ( &in_sys, &out_sys, &gv->conv_params );
}

void
gvParserMFInit( GeoVRML gv, MFString *parse_string, MFString *geo_system )
{
  /* copy the string array into our private storage (not deep copy) */

  memcpy( &gv->input, parse_string, sizeof(MFString) );

  /* parse the geoSystem string array */

  gvParseGeoSystem( gv, geo_system );

  /* initialise the parser state */

  gv->pos = gv->num = 0;
}

void
gvParserSFInit( GeoVRML gv, SFString parse_string, MFString *geo_system )
{
  /* fake the SFString parameter into an MFString node */

  gv->input.mf.len   = 1;
  gv->input.mf.type  = vsgMFString;
  gv->input.mf.flags = 0;
  gv->input.data     = gv->sfbuffer;
  gv->sfbuffer[0]    = parse_string;

  /* parse the geoSystem string array */

  gvParseGeoSystem( gv, geo_system );

  /* initialise the parser state */

  gv->pos = gv->num = 0;
}

static char
gvParserPeekNextChar( GeoVRML gv )
{
  char const  *ptr;
  int         num = gv->num;

  /* have we read past the end of the string? */

  if ( num >= gv->input.mf.len )
    return '\0';

  /* advance over any leading whitespace */

  ptr = gv->input.data[num] + gv->pos;

  while ( *ptr <= ' ' ) {
    if ( *ptr == '\0' ) {
      num++;
      if ( num >= gv->input.mf.len ) return '\0';
      ptr = gv->input.data[num];
    } else
      ptr++;
  }
  
  /* return the next character in the string */

  return *ptr;
}

static double
gvParserNextValue( GeoVRML gv )
{
  char const  *ptr;
  double      value;

  /* have we read past the end of the string? */

  if ( gv->num >= gv->input.mf.len )
    return 0.0;

  /* advance over any leading whitespace */

  ptr = gv->input.data[gv->num] + gv->pos;
  while ( *ptr <= ' ' ) {
    if ( *ptr == '\0' ) {
      gv->pos = 0;
      gv->num++;
      if ( gv->num >= gv->input.mf.len ) return 0.0;
      ptr = gv->input.data[gv->num];
    } else {
      ptr++;
      gv->pos++;
    }
  }
  
  /* convert the current value in the string into a double */

  value = atof( ptr );

  /* advance over the value so we read the next value next time */

  while ( *ptr > ' ' ) {
    ptr++;
    gv->pos++;
  }

  /* return the result */

  return value;
}

int
gvParserNextGCC( GeoVRML gv, double gcc[3] )
{
  double      p1, p2, p3;
  SE_COORD coord1, coord2;
  char        symbol;
  
  if ( gv == NULL ) return FALSE;

  /* read the next tuple of coordinate values */

  p1 = gvParserNextValue( gv );
  p2 = gvParserNextValue( gv );

  symbol = gvParserPeekNextChar( gv );
  if ( symbol == '\0' ) return FALSE;  /* end of string */

  if ( gv->threeD == NULL ) symbol = '\0';

  p3 = gvParserNextValue( gv );

  /* printf( "Parsed: %lf %lf %lf\n", p1, p2, p3 ); */

  /* perform the appropriate geographic conversion to get GCC coords */

  coord1.is_2D = FALSE;
  coord1.u.coord_3d.spatial_reference_frame = gv->coord_sys;

  switch ( gv->coord_sys ) {
  case SRM_GD_3D_SRF:
    /* printf( "GDC->GCC\n" ); */
    coord1.u.coord_3d.u.gd_coord.geodetic_latitude  = p1;
    coord1.u.coord_3d.u.gd_coord.geodetic_longitude = p2;

    if ( symbol == '+' )
      coord1.u.coord_3d.u.gd_coord.elevation = 0;
    else
      coord1.u.coord_3d.u.gd_coord.elevation = p3;

    if ( SE_ConvertCoordToGivenSystem( gv->conv_params, &coord1, &coord2 )
	 != SE_COORD_SUCCESS ) {
      return FALSE;
    }

    gcc[0] = coord2.u.coord_3d.u.gc_coord.x;
    gcc[1] = coord2.u.coord_3d.u.gc_coord.y;
    gcc[2] = coord2.u.coord_3d.u.gc_coord.z;

    /* make the elevation DEM-relative if the elevation was prefixed  */
    /* with either a "+" or a "-" symbol. Note that this means having */
    /* to retransform the GDC coordinate again.                       */

    if ( symbol == '+' ) {
      float demelev = -tvBasePointAltitude( gv->threeD, gcc );

      coord1.u.coord_3d.u.gd_coord.elevation = demelev + p3;
      
      /* printf( "demelev = %f, p3 = %f, final = %f\n", demelev, p3,
	      coord1.u.gd_coord.elevation ); */

      if ( SE_ConvertCoordToGivenSystem( gv->conv_params, &coord1, &coord2 )
	   != SE_COORD_SUCCESS ) {
	return FALSE;
      }

      gcc[0] = coord2.u.coord_3d.u.gc_coord.x;
      gcc[1] = coord2.u.coord_3d.u.gc_coord.y;
      gcc[2] = coord2.u.coord_3d.u.gc_coord.z;
    }

    break;
  case SRM_GC_3D_SRF:
    /* printf( "GCC->GCC\n" );*/
    gcc[0] = p1;
    gcc[1] = p2;
    gcc[2] = p3;
    break;
  case SRM_UTM_3D_SRF:
    /* printf( "UTM->GCC\n" );*/
    coord1.u.coord_3d.u.utm_coord.x = p2;  /* easting */
    coord1.u.coord_3d.u.utm_coord.y = p1;  /* northing */
    if ( symbol == '+' )
      coord1.u.coord_3d.u.utm_coord.z = 0;
    else
      coord1.u.coord_3d.u.utm_coord.z = p3;
    if ( gv->utm_north_hemi )
      coord1.u.coord_3d.u.utm_coord.hemisphere = SE_NORTHERN_HEMISPHERE;
    else
      coord1.u.coord_3d.u.utm_coord.hemisphere = SE_SOUTHERN_HEMISPHERE;

    if ( SE_ConvertCoordToGivenSystem( gv->conv_params, &coord1, &coord2 )
	 != SE_COORD_SUCCESS ) {
      return FALSE;
    }

    gcc[0] = coord2.u.coord_3d.u.gc_coord.x;
    gcc[1] = coord2.u.coord_3d.u.gc_coord.y;
    gcc[2] = coord2.u.coord_3d.u.gc_coord.z;

    /* make the elevation DEM-relative if the elevation was prefixed  */
    /* with either a "+" or a "-" symbol. Note that this means having */
    /* to retransform the UTM coordinate again.                       */

    if ( symbol == '+' ) {
      float demelev = -tvBasePointAltitude( gv->threeD, gcc );

      coord1.u.coord_3d.u.utm_coord.z = demelev + p3;

      if ( SE_ConvertCoordToGivenSystem( gv->conv_params, &coord1, &coord2 )
	   != SE_COORD_SUCCESS ) {
	return FALSE;
      }

      gcc[0] = coord2.u.coord_3d.u.gc_coord.x;
      gcc[1] = coord2.u.coord_3d.u.gc_coord.y;
      gcc[2] = coord2.u.coord_3d.u.gc_coord.z;
    }
    break;
  default:
    /* printf( "none\n" ); */
    gcc[0] = gcc[1] = gcc[2] = 0.0;
  }

  /* printf( "GCC: %lf %lf %lf\n", gcc[0], gcc[1], gcc[2] );*/

  gvApplyOrigin( gv, gcc );

  /* printf( "Local: %lf %lf %lf\n", gcc[0], gcc[1], gcc[2] );*/

  return TRUE;
}

void
gvLocalOrientation( GeoVRML gv, double gcc[3], float orient[4] )
{
  /* static char const *funcName = "gvLocalOrientation"; */
  v3f        axis;
  quat       qx, qz, qr;
  float      angle;
  double     gdc[3], geo[3];

  /* reapply the origin and coord frame flip to get the GCC coords */

  geo[0] = gcc[0];
  geo[1] = gcc[1];
  geo[2] = gcc[2];
  gvRetractOrigin( gv, geo );

  /* convert these into lat/long */

  TsGCCtoGDC( geo[0], geo[1], geo[2], gdc );

  /* now do the quaternion math in order to work out the local orientation */

  rotation_quat( 0, 0, 1, (90.0 + gdc[0]), &qz );
  rotation_quat( 1, 0, 0, (180.0 - gdc[1]), &qx );
  add_quat( &qx, &qz, &qr );
  angle = quat_angle( &qr, &axis );

  orient[0] = (float) angle;
  orient[1] = (float) V(axis)[0];
  orient[2] = (float) V(axis)[1];
  orient[3] = (float) V(axis)[2];
}

void
gvSetViewer( GeoVRML gv, tvBase *threeD )
{
  gv->threeD = threeD;
}
