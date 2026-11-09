/* -*- tab-width: 2; -*-

  Name:  Features.h

  Description: Header file for drawing building models

  Revision History:
      $Id: Features.h,v 2.25.2.7 2002/10/31 05:26:30 reddy Exp $

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

#ifndef FEATURES_H
#define FEATURES_H

#ifdef WIN32
# define  WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

#include <GL/gl.h>

#include <tsm/tsm.h>
#include <tsm/tsmImage.h>
#include <vrml2/vrml2.h>

#include "ThreeDCore.h"

#define MAX_FEATURES   100       /* max number of feature files supported */

/* the following defines are used for the which_offset field if the */
/* FeatureItem structure. They define whether the VRML file has a   */
/* Metadata node that specifies an (x,y,z) offset from a dataset's  */
/* LVCS or an absolute GCC coordinate origin, or if no origin is    */
/* defined then we assume a GeoVRML world which will define all of  */
/* the georefencing stuff for us.                                   */

#define FEATURE_ORIGIN_GEOVRML  0    /* use a GeoOrigin node, if specified */
#define FEATURE_ORIGIN_LCS      1    /* (x,y,z) offset from terrain */
#define FEATURE_ORIGIN_GCC      2    /* absolute geocentric location */

/* FeatureItem describes a single instance of a feature file that has */
/* been loaded into memory. Feature.c contains a global, master list  */
/* of all VRML feature files using this structure.                    */

/* Doc++ Documentation: *//**
 *    FeatureItem describes a single instance of a feature file that has
 *     been loaded into memory. 
 * <pre>
 *  struct {
 * char     *name;            * the name of this model (from database) *
 * vsgWorld *wrl;             * the VRML scene graph for this feature *
 * double   lcs_offset[3];    * feature's offset from the dataset's LCS *
 * double   gcc_origin[3];    * geocentric coordinate for feature's origin *
 * char     *url;             * URL for this VRML scene *
 * char     *geoset;          * name of the geoset this model applies to *
 * char     *author;          * author of the feature file *
 * char     *copyright;       * copyright information for feature *
 * char     *description;     * description of what the feature is *
 * int      y_up;             * is Y defined as up for the VRML LCS? *
 * int      refcount;         * reference count for this feature *
 * int      textures;         * number of textures loaded for this model *
 * int      id;               * a unique ID number for this feature *
 * int      loaded;           * true when model us fully loaded into memory *
 * int      which_origin;     * 0 = geovrml, 1 = lcs_offset, 2 = gcc *
 * ThreeDCore *closure;       * closure passed into FeatureLoadCB *
 * }
 * </pre>
 */
struct FeatureItem {
  char     *name;            /* the name of this model (from database) */
  vsgWorld *wrl;             /* the VRML scene graph for this feature */
  double   lcs_offset[3];    /* feature's offset from the dataset's LCS */
  double   gcc_origin[3];    /* geocentric coordinate for feature's origin */
  char     *url;             /* URL for this VRML scene */
  char     *geoset;          /* name of the geoset this model applies to */
  char     *author;          /* author of the feature file */
  char     *copyright;       /* copyright information for feature */
  char     *description;     /* description of what the feature is */
  int      y_up;             /* is Y defined as up for the VRML LCS? */
  int      refcount;         /* reference count for this feature */
  int      textures;         /* number of textures loaded for this model */
  int      anchors;          /* count of the number of Anchor nodes */
  int      inlines;          /* count of the number of Inline nodes */
  int      id;               /* a unique ID number for this feature */
  int      loaded;           /* true when model us fully loaded into memory */
  int      geoorigin_parsed; /* TRUE if a GeoOrigin node has been parsed */
  int      which_origin;     /* 0 = geovrml, 1 = lcs_offset, 2 = gcc */
  long     filesize;         /* filesize in bytes of root VRML scene */
  tvBase   *closure;       /* closure passed into FeatureLoadCB */
};
typedef struct FeatureItem FeatureItem;

/* a list of FeatureInstance structures is stored in the tvBase */
/* This defines the subset of all loaded features to be displayed and */
/* contains the widget-specific state in order to render this feature */

typedef struct {
  int      id;       /* the ID of the FeatureItem entry */
  int      list;     /* an OpenGL display list for the scene */
  tvBase * threeD;   /* used to pass tvBase into render */
} FeatureInstance;

/* FeatureTexture contains a single instance of a texture image that has */
/* been loaded into memory. Feature.c maintains a master list of these.  */

typedef struct {
  char    *url;                  /* URL for this texture map */
  GLuint  tex_id;                /* texture ID for OpenGL */
  Image_t *image;                /* image bitmap and dimensions */
  int     refcount;              /* reference count */
} FeatureTexture;

#ifdef __cplusplus
extern "C" {
#endif

/* the init function *must* be the first function called in this module */

void FeatureInit( void );

/* this function frees all memory and OpenGL lists used for features */

void FeatureFreeInstances( tvBase *threeD );


/* render all of the features associated with a tvBase */

int FeatureDisplay( tvBase *, TsGeoPyramid * );


/* routine to get the scene graph of an Inline node's url */

vsgWorld *FeatureGetInline( vsgWorld *world, char const *url );


/* get a FeatureItem structure if you have the feature ID */

FeatureItem *FeatureGetItem( int id );

/* low-level loading and unloading functions */

FeatureItem * FeatureAlloc (char const *url, int add_to_list);

void FeatureFree( FeatureItem *item );

FeatureItem * FeatureQuery( char const *url );

FeatureItem * FeatureLoad (tvBase *threeD, FeatureItem *item);
FeatureItem * FeatureParse (tvBase *threeD, const char *vrml, const char *url);

FeatureItem * FeatureRefCountLoad( tvBase *threeD, char *url );
FeatureItem * FeatureRefCountParse( tvBase *threeD, char const *buffer,
																		char const *url );

int FeatureUnload( tvBase *threeD, char *url );

void FeatureUnloadAll( tvBase *threeD );

int FeatureTexLoad( vsgWorld *world, char *url );

int FeatureTexUnload( vsgWorld *world, char *url );

FeatureTexture *FeatureTexUse( vsgWorld *world, char *url );

int FeatureParseMetadata( FeatureItem *item );
int FeatureAddToWidget( tvBase *threeD, int id );
int FeatureRemoveFromWidget( tvBase *threeD, int id );
int FeatureLoadTextures( tvBase *, vsgWorld *world );
int FeatureInlineTraverse (tvBase *, FeatureItem *, vsgWorld *, MFNode *,
			   int, int);

/* find a feature given its URL */

FeatureItem *GetFeatureFromUrl( char const *url );

/* find a numbered Anchor node and set the tvBase's hyperlink state */

void FeatureSetHyperlink( tvBase *threeD, int fid, int anchor );


#ifdef __cplusplus
}
#endif

#endif

