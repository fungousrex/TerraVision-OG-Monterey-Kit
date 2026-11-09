/* -*- tab-width: 2; -*-

  Name: Features.c

  Description: This file handles the placement and drawing of the 
     building models, in two and three d.

  Revision History:
      $Id: Features.c,v 2.40.2.15 2002/10/31 05:26:29 reddy Exp $

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

#include "TerraVisionInc.h"
#include "VRML.h"
#include "GeoVRML.h"

#include <stdio.h>
#include <GL/gl.h>

#include <vrml2/vrml2.h>
#include <vrml2/c_nodes.h>

#include <tsm/tsm.h>

#include "Features.h"
#include "ThreeDCoreI.h"

/* the global state for this module is the list of all loaded features */
/* feature_list is simply a list of FeatureItem structures.            */

int    last_id = 0;
list_t *feature_list = NULL;
list_t *texture_list = NULL;

/* here are the type definitions that are local to this module */

typedef int (TexFunc_t) (vsgWorld *, char *);

/* list the prototypes for this module's private functions */

static void FeatureAbsUrl( vsgWorld *world, char const *url, char absurl[] );
static int FeatureUnloadTextures( tvBase *, vsgWorld *world );
static int FeatureTexturesTraverse( tvBase *, vsgWorld *, MFNode *,
				    TexFunc_t );
static vsgWorld *FeatureLoadInline( char const *url );
static void FeatureSetHyperlink_r( tvBase *, vsgWorld *, MFNode *, int *,
				   int, int * );


/*
 * Synopsis:
 *   void FeatureInit( void )
 *
 * Doc++ Description: *//**
 *   This function performs all of the required initialisations for
 *   the VRML feature support. This routine must be the first function
 *   that you call in this module. Bad things will ensue if you do not
 *   do what you are told!
 *
 * @return    void
 *
 * @author    Martin Reddy
 * 
 * @version   9 April, 1999
 *
 */

void
FeatureInit( void )
{
  vsgcSetup();
  InitVRML();

  feature_list = list_create( TRUE );
  texture_list = list_create( TRUE );
}

/*
 * Synopsis:
 *   int FeatureAddToWidget( tvBase *threeD, int id )
 *
 * Doc++ Description: *//**
 *   Add an ID to the tvBase's list of IDs. This list is used to
 *   specify which subset of all loaded features a particular tvBase
 *   should display. If the specified ID already exists in the widget's
 *   ID list, then no operation is performed.
 *
 *   The function will return a value of TRUE if the ID was added to
 *   the widget's list, or if it was already present. A FALSE result
 *   means that the list does not contain the ID upon completion of
 *   this function.
 *
 *   If an invalid ID is passed (<0) then no operation is performed
 *   and a value of FALSE is returned.
 *
 * @param threed  handle to rendering state.
 * @param id      ID to be added.
 *
 * @return    TRUE if all went well, FALSE if ID not added to list
 *
 * @author    Martin Reddy
 * 
 * @version   9 April, 1999
 *
 */

int
FeatureAddToWidget( tvBase *threeD, int id )
{
  list_iter_t     i;
  FeatureInstance *item;
  list_t          *list;

  /* do we have a valid ID? */

  if ( id < 0 || threeD == NULL ) return FALSE;

  /* check to see if ID already exists in the widget's ID list */

  list = threeD->fselIDs;

  for ( i = list_first( list ); ! list_done( i ); i = list_next( i ) ) {
    item = (FeatureInstance *) list_value( i );
    if ( item->id == id )
      return TRUE;
  }

  /* we have a go... so create a new instance and add it to the list */

  item = (FeatureInstance *) tsmMemCalloc( 1, sizeof(FeatureInstance) );
  if ( item == NULL ) return FALSE;

  item->id       = id;
  item->list     = 0;
  item->threeD   = threeD;
  
  list_push( list, item );

  return TRUE;
}

/*
 * Synopsis:
 *   int FeatureRemoveFromWidget( tvBase *threeD, int id )
 *
 * Doc++ Description: *//**
 *   Removes an ID from the tvBase's list of IDs. This list is
 *   used to specify which subset of all loaded features a particular
 *   tvBase should display. If the widget list does not contain
 *   the ID, then no operation is performed.
 *
 *   If an invalid ID is passed (<0) then no operation is performed
 *   and a value of FALSE is returned.
 *
 * @param threeD  handle to rendering state.
 * @param id      ID to be removed.
 *
 * @return   TRUE if ID was removed from list, FALSE if it didn't exist
 *
 * @author   Martin Reddy
 * 
 * @version  9 April, 1999
 *
 */

int
FeatureRemoveFromWidget( tvBase *threeD, int id )
{
  list_iter_t     i;
  FeatureInstance *item;
  list_t          *list;

  /* do we have a valid ID? */

  if ( id < 0 || threeD == NULL ) return FALSE;

  /* check to see if ID already exists in widget's ID list, and remove it */

  list = threeD->fselIDs;

  for ( i = list_first( list ); ! list_done( i ); i = list_next( i ) ) {
    item = (FeatureInstance *) list_value( i );
    if ( item->id == id ) {
      if ( item->list ) glDeleteLists( item->list, 1 );
      tsmMemFree( item );
      list_delete( i );
      return TRUE;
    }
  }

  return FALSE;
}

/*
 * Synopsis:
 *   void FeatureFreeInstances( tvBase *threeD )
 *
 * Description:
 *   This function will free up the list of FeatureInstances that
 *   is stored for a tvBase. This involves destroying all of
 *   OpenGL display lists that were created for displaying features.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 13 April, 1999
 *
 */

void
FeatureFreeInstances( tvBase *threeD )
{
  list_iter_t     i;
  FeatureInstance *item;

  /* do we have a valid ID? */

  if ( threeD == NULL || threeD->fselIDs == NULL ) return;

  /* destroy each entry in the list */

  for ( i = list_first( threeD->fselIDs ); ! list_done( i );  ) {
    item = (FeatureInstance *) list_value( i );
    if ( item ) {
      if ( item->list ) glDeleteLists( item->list, 1 );
      tsmMemFree( item );
    }
    i = list_delete( i );
  }

  /* destroy the list itself */

  list_destroy( threeD->fselIDs );
  threeD->fselIDs = NULL;

  return;
}

/*
 * Synopsis:
 *   FeatureItem *FeatureGetItem( int id )
 *
 * Doc++ Description:  *//**
 *   Search through the list of all loaded features looking for one
 *   with the specified ID. If one is found, then we return a pointer
 *   to it's FeatureItem structure. Otherwise, we return NULL.
 *
 * @param  id  ID of features to be retreived.
 *
 * @return   A valid FeatureItem, or NULL if ID not matched.
 *
 * @author   Martin Reddy
 * 
 * @version  9 April, 1999
 *
 * @see      FeatureItem
 *
 */

FeatureItem *
FeatureGetItem( int id )
{
  list_iter_t i;
  FeatureItem *item;

  /* search through the feature list looking for a feature with the */
  /* specified ID. We can assume that features are sorted on ID, so */
  /* if we reach a feature ID > id, then we can short-circuit out.  */

  for ( i = list_first( feature_list );	! list_done( i ); i = list_next( i ) ){
    item = (FeatureItem *) list_value( i );
    if ( item->id == id ) return item;
    if ( item->id > id ) return NULL;
  }

  return NULL;
}

/*
 * Synopsis:
 *   int FeatureParseMetadata( FeatureItem *item )
 *
 * Description:
 *   Takes a VRML feature file that has been loaded and parsed into
 *   the item->wrl variable. It then searches through this VRML file
 *   for a Metadata node: one that has been DEF named "METADATA" and
 *   has an exposedField called info. It then reads the info MFString
 *   to extract all of the information that we need for rendering.
 *   Currently this includes:
 *
 *     lcs-offset: x y z
 *        = the (x,y,z) offset of the feature from the dataset's origin
 *          
 *     utm-location: n e elev zone
 *        = define the UTM origin for this world
 *
 *     latlong-location: lat long elev
 *        = define the lat/long origin for this world
 *
 *     y-axis-up: [yes|no]
 *        = is the Y axis defined as up (yes) or the Z axis (no)
 *
 *     geoname: <geoset-name>
 *        = same as the geoname tspec field for OI/DEM to associate with
 *
 *     author: <name>
 *        = the author of this feature file
 *
 *     copyright: <string>
 *        = the copyright information for this feature
 *
 *     description: <string>
 *        = a textual description of what the feature file contains
 *
 * Returns: TRUE if a METADATA node was found, FALSE if not found
 *
 * Author: Martin Reddy
 * 
 * Date: 9 April, 1999
 *
 */

int
FeatureParseMetadata( FeatureItem *item )
{
  /*static char const funcName[] = "FeatureParseMetadata";*/
  MFString        *mfstring;
  char            buffer[1024];

  if ( item == NULL || item->wrl == NULL ) return FALSE;

  item->lcs_offset[0]    = 0.0;
  item->lcs_offset[1]    = 0.0;
  item->lcs_offset[2]    = 0.0;
  item->gcc_origin[0]    = 0.0;
  item->gcc_origin[1]    = 0.0;
  item->gcc_origin[2]    = 0.0;
  item->geoorigin_parsed = FALSE;
  item->y_up             = TRUE;
  item->which_origin     = FEATURE_ORIGIN_GEOVRML;

  /* get the info exposedField of the node DEF-named METADATA */

  if ( ( mfstring = tsmGetMetadataNode( item->wrl ) ) == NULL ) {
    item->name = tsmMemStrdup( "Unknown Name" );
    return TRUE;
  }

  /* is the "lcs-offset" token defined? */

  if ( tsmParseMetadataInfo( mfstring, "lcs-offset", buffer ) ) {
    sscanf( buffer, "%lf%lf%lf", &item->lcs_offset[0], &item->lcs_offset[1],
	    &item->lcs_offset[2] );
    item->which_origin = FEATURE_ORIGIN_LCS;
  }

  /* is the "utm-location" token defined? */

  if ( tsmParseMetadataInfo( mfstring, "utm-location", buffer ) ) {
    double n, e, lat, lon, elev;
    int    zone;

    sscanf( buffer, "%lf%lf%lf%d", &n, &e, &elev, &zone );

    tsmUTMToLatLong( zone, e, n, &lat, &lon );
    TsGDCtoGCC( lon, lat, elev, item->gcc_origin );

    item->which_origin = FEATURE_ORIGIN_GCC;
  }

  /* is the "latlong-location" token defined? */

  if ( tsmParseMetadataInfo( mfstring, "latlong-location", buffer ) ) {
    double lat, lon, elev;

    sscanf( buffer, "%lf%lf%lf", &lat, &lon, &elev );

    TsGDCtoGCC( lon, lat, elev, item->gcc_origin );

    item->which_origin = FEATURE_ORIGIN_GCC;
  }

  /* is the "y-axis-up" token defined? */

  if ( tsmParseMetadataInfo( mfstring, "y-axis-up", buffer ) )
    item->y_up = ( strcasecmp( buffer, "yes" ) == 0 );

  /* is the "geoname" token defined? */
  
  if ( tsmParseMetadataInfo( mfstring, "geoname", buffer ) )
    item->geoset = tsmMemStrdup( buffer );

  /* is the "author" token defined? */
  
  if ( tsmParseMetadataInfo( mfstring, "author", buffer ) )
    item->author = tsmMemStrdup( buffer );

  /* is the "copyright" token defined? */
  
  if ( tsmParseMetadataInfo( mfstring, "copyright", buffer ) )
    item->copyright = tsmMemStrdup( buffer );

  /* is the "description" token defined? */
  /* XXX - we should do a check for a buffer overflow! */
  
  if ( tsmParseMetadataInfo( mfstring, "description", buffer ) )
    item->description = tsmMemStrdup( buffer );

  return TRUE;
}

/*
 * Synopsis:
 *   static void FeatureAbsUrl( vsgWorld *world, char *url, char absurl[] )
 *
 * Description:
 *   This function creates an absolute URL for a location that is
 *   associated with the specified VRML world. The result is copied
 *   into the absurl buffer - there must be enough room here. 
 *
 *   If the specified URL is already absolute (i.e. begins with "/"
 *   or is a file/http URL) then url is copied as-is into absurl.
 *   Otherwise, we prepend the directory URL for the specified world.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 12 April, 1999
 *
 */

static void
FeatureAbsUrl( vsgWorld *world, char const *url, char absurl[] )
{
  char *sep;

  /* do a quick sanity check */

  if ( world == NULL || url == NULL ) {
    strcpy( absurl, "" );
    return;
  }

  /* make the URL absolute by prepending the world directory if needed */

  if ( url[0] != '/' && strchr( url, ':' ) == NULL ) {
    strcpy( absurl, vsgWorldFilename(world) );
    if ( ( sep = strrchr( absurl, '/' ) ) == NULL ) sep = absurl;
    *sep = '\0';
    if ( sep == absurl || sep[-1] != '/' ) strcat( absurl, "/" );
    strcat( absurl, url );
  } else 
    strcpy( absurl, url );

}

/*
 * Synopsis:
 *   int FeatureTexLoad( vsgWorld *world, char *url )
 *
 * Description:
 *   Loads a texture map image and adds it to the global feature texture
 *   map list. If this operation succeeds, then the function will return
 *   TRUE and you can get the appropriate FeatureTexture structure by
 *   using FeatureTexUse().
 *
 *   If the image URL already exists in the texture list, then we increment
 *   its reference count and return TRUE.
 *
 * Returns: TRUE if the the texture is in memory, FALSE if operation failed
 *
 * Author: Martin Reddy
 * 
 * Date: 12 April, 1999
 *
 */

int
FeatureTexLoad( vsgWorld *world, char *url )
{
  const char     *funcName = "FeatureTexLoad";
  list_iter_t    i;
  FeatureTexture *item;
  Image_t        *img;
  char           absurl[512];
  unsigned char  *new_bitmap;
  int            new_width, new_height;

  if ( url == NULL ) return FALSE;

  /* make the URL absolute by prepending the world directory if needed */

  FeatureAbsUrl( world, url, absurl );

  /* search the texture list to see if this URL has already been loaded */

  for ( i = list_first( texture_list );
	! list_done( i );
	i = list_next( i ) ){
    item = (FeatureTexture *) list_value( i );
    if ( item && strcmp( item->url, absurl ) == 0 ) {
      item->refcount++;
      return TRUE;
    }
  }

  /* otherwise, we need to create a new entry for this new texture URL */

  item = (FeatureTexture *) tsmMemCalloc( 1, sizeof(FeatureTexture) );
  if ( item == NULL ) return FALSE;

  item->url      = tsmMemStrdup( absurl );
  item->tex_id   = 0;
  item->refcount = 1;

  /* load the texture map image into memory */

  if ( ( item->image = tsmReadImage( absurl ) ) == NULL ) {
    tsmError( funcName, "Could not read texture: %s\n", absurl );
    if ( item->url ) tsmMemFree( item->url );
    tsmMemFree( item );
    return FALSE;
  }

  img = item->image;

  /* flip the image for use with OpenGL texture mapping functions */

  gltInvertImage( img->bitmap, img->width, img->height, img->components );

  /* make sure that the width and height are powers of two */

  new_width  = gltTextureSize( img->width );
  new_height = gltTextureSize( img->height );

  if ( new_width != img->width || new_height != img->height ) {
    int new_bytes = new_width * new_height * img->components;

    if ( ( new_bitmap = (unsigned char *) tsmMemMalloc( new_bytes ) ) ) {

      /* resize the image so that width and height are powers of two */

      gltScaleImage( img->components, img->width, img->height, img->width *
		     img->components, img->bitmap, new_width, new_height,
		     new_width * img->components, new_bitmap );

      /* update the Image_t structure with the new image data */
      
      tsmMemFree( img->bitmap );

      img->bitmap = new_bitmap;
      img->width  = new_width;
      img->height = new_height;
    }
  }

  /* add the new FeatureTexture to the texture list */

  list_push( texture_list, item );
  return TRUE;
}

/*
 * Synopsis:
 *   int FeatureTexUnload( vsgWorld *world, char *url )
 *
 * Description:
 *   Searches throught the list of loaded texture images looking for
 *   one for the specified URL. If this is found, then its reference
 *   count is decreased. If the reference count for a texture reaches
 *   zero, then it is unloaded from memory and removed from the global
 *   list of textures. 
 *
 * Returns: TRUE if the texture was found in list and removed, FALSE if not
 *
 * Author: Martin Reddy
 * 
 * Date: 12 April, 1999
 *
 */

int
FeatureTexUnload( vsgWorld *world, char *url )
{
  list_iter_t    i;
  FeatureTexture *item;
  char           absurl[512];

  /* make the URL absolute by prepending the world directory if needed */

  FeatureAbsUrl( world, url, absurl );

  /* search the list to see if this texture has already been loaded */

  for ( i = list_first( texture_list );
	! list_done( i );
	i = list_next( i ) ){
    item = (FeatureTexture *) list_value( i );
    if ( item && strcmp( item->url, absurl ) == 0 ) {

      /* if there are no more references to this feature, */
      /* free it and remove it from the list of features. */

      if ( --item->refcount <= 0 ) {
	if ( item->url ) tsmMemFree( item->url );
	if ( item->tex_id ) gltDeleteTextures( 1, &item->tex_id );
	tsmFreeImage( item->image, TRUE );
	tsmMemFree( item );
	list_delete( i );
      }

      return TRUE;
    }
  }

  return FALSE;
}

/*
 * Synopsis:
 *   FeatureTexture *FeatureTexUse( vsgWorld *world, char *url )
 *
 * Description:
 *   Returns the FeatureTexture structure for a texture map image that
 *   has already been loaded into the global texture list using
 *   FeatureTexLoad(). If the texture URL does not exist in the list,
 *   then we return NULL;
 *
 * Returns: A valid FeatureTexture, of NULL
 *
 * Author: Martin Reddy
 * 
 * Date: 12 April, 1999
 *
 */

FeatureTexture *
FeatureTexUse( vsgWorld *world, char *url )
{
  list_iter_t    i;
  FeatureTexture *item;
  char           absurl[512];

  if ( url == NULL ) return NULL;

  /* make the URL absolute by prepending the world directory if needed */

  FeatureAbsUrl( world, url, absurl );

  /* search the texture list to see if this URL has already been loaded */

  for ( i = list_first( texture_list );
	! list_done( i );
	i = list_next( i ) ){
    item = (FeatureTexture *) list_value( i );
    if ( item && strcmp( item->url, absurl ) == 0 )
      return item;
  }

  return NULL;
}

/*
 * Synopsis:
 *   int FeatureTexturesTraverse( tvBase *,
 *                                vsgWorld *world, MFNode *mfnode,
 *                                TexFunc_t action )
 *
 * Description:
 *   This function will traverse through an entire VRML scene graph
 *   looking for an ImageTexture node. If it finds one, then it will
 *   call the specified callback function with the url of that texture.
 *
 *   url is an MFString in ImageTexture, therefore if the first url
 *   string cannot be found/loaded, then we progress on to the second
 *   url and try this. This continues until we find a valid texture
 *   map to use. This is in line with the VRML97 specification.
 *
 *   This function returns the number of invocations of the action
 *   callback function which returned a result of TRUE. For example,
 *   if action = FeatureTexLoad, then the result of this function is
 *   the number of textures that were loaded for this model.
 *
 * Returns: The number of successful invocations of the action callback
 *
 * Author: Martin Reddy
 * 
 * Date: 14 April, 1999
 *
 */

static int
FeatureTexturesTraverse( tvBase *threeD,
			 vsgWorld *world, MFNode *mfnode,
			 TexFunc_t action )
{
  /*static const char funcName[] = "FeatureTexturesTraverse";*/
  int        i, j, count = 0;
  SFNode     node, app, texture;
  MFString   *url;
  char const *name;
  char const *intern;
  vsgEventOutDesc event_out;
  MFNode     *children;

  if ( world == NULL || mfnode == NULL ) return 0;

  for ( i = 0; i < mfnode->mf.len; i++ ) {
    node = mfnode->data[i];
    name = node->cl->name;

    /* what to do with this node? */

    if ( strcmp( name, vsgCollisionClass->name ) == 0 ||
	 strcmp( name, vsgGroupClass->name ) == 0 ||
	 strcmp( name, vsgTransformClass->name ) == 0 ||
	 strcmp( name, vsgBillboardClass->name ) == 0 ||
	 strcmp( name, vsgAnchorClass->name ) == 0 ||
	 strcmp( name, "GeoLocation" ) == 0 ) {

      /* we have a group node with a children exposedField - traverse it */

      intern = vsgIntern( world, "children" );
      event_out = vsgGetFunc( node, intern );
      if ( event_out.f.MFString == NULL ) continue;
      if ( ( children = vsgGetMFNode( world, node, event_out ) ) == NULL )
	continue;

      count += FeatureTexturesTraverse( threeD, world, children, action );

    } else if ( strcmp( name, vsgShapeClass->name ) == 0 ) {

      /* we have a Shape node - get its Appearance node */

      intern = vsgIntern( world, "appearance" );
      event_out = vsgGetFunc( node, intern );
      if ( event_out.f.SFNode == NULL ) continue;
      if ( ( app = vsgGetSFNode( world, node, event_out ) ) == NULL )
	continue;

      /* now get the ImageTexture node of the Appearance node */

      intern = vsgIntern( world, "texture" );
      event_out = vsgGetFunc( app, intern );
      if ( event_out.f.SFNode == NULL ) continue;
      if ( ( texture = vsgGetSFNode( world, app, event_out ) ) == NULL )
	continue;

      if ( strcmp( texture->cl->name, vsgImageTextureClass->name ) != 0 )
	continue;

      /* now get the URL field of the image texture */

      intern = vsgIntern( world, "url" );
      event_out = vsgGetFunc( texture, intern );
      if ( event_out.f.MFString == NULL ) continue;
      if ( ( url = vsgGetMFString( world, texture, event_out ) ) == NULL )
	continue;

      /* go through each url until we find a texture we can load */

      for ( j = 0; j < url->mf.len; j++ ) {

	/* update the busy dialog with the current texture location */

	tvBaseBusy ( threeD, "Loading texture image from URL:\n%s",
			 url->data[j]);

	/* try to load/unload this texture image */

	if ( (*action)( world, url->data[j] ) ) {
	  count++;
	  break;
	}
      }

    }
  }

  return count;
}

/*
 * Doc++ Description: *//**
 *   Counts all of the texture, anchor, and inline nodes in a loaded
 *   VRML file. Thes results are appended to the appropriate fields
 *   in the passed FeatureItem structure. This routine will not 
 *   recurse into Inline files or load any further resources.
 *
 * @param world the vsgWorld reference for the loaded VRML file
 * @param mfnode the current scene sub-graph to traverse over
 * @param item the FeatureItem to be filled
 *
 * @return  void
 *
 * @see     FeatureItem
 *
 */
static void
FeatureCountNodes( vsgWorld *world, MFNode *mfnode, FeatureItem *item )
{
  SFNode     node, app, texture;
  char const *name;
  char const *intern;
  vsgEventOutDesc event_out;
  MFNode     *children;
  int        i;

  if ( world == NULL || mfnode == NULL ) return;

  for ( i = 0; i < mfnode->mf.len; i++ ) {
    node = mfnode->data[i];
    name = node->cl->name;

    /* what to do with this node? */

    if ( strcmp( name, vsgInlineClass->name ) == 0 ) item->inlines++;
    if ( strcmp( name, vsgAnchorClass->name ) == 0 ) item->anchors++;

    if ( strcmp( name, vsgShapeClass->name ) == 0 ) {
      /* we have a Shape node - get its Appearance node */
      intern = vsgIntern( world, "appearance" );
      event_out = vsgGetFunc( node, intern );
      if ( event_out.f.SFNode == NULL ) continue;
      if ( ( app = vsgGetSFNode( world, node, event_out ) ) == NULL )
	continue;
      /* is there an ImageTexture node of the Appearance node */
      intern = vsgIntern( world, "texture" );
      event_out = vsgGetFunc( app, intern );
      if ( event_out.f.SFNode == NULL ) continue;
      if ( ( texture = vsgGetSFNode( world, app, event_out ) ) != NULL )
	if ( strcmp( texture->cl->name, vsgImageTextureClass->name ) == 0 )
	  item->textures++;    /* we have found a texture! */
    }

    /* do we recurse? */

    if ( strcmp( name, vsgCollisionClass->name ) == 0 ||
	 strcmp( name, vsgGroupClass->name ) == 0 ||
	 strcmp( name, vsgTransformClass->name ) == 0 ||
	 strcmp( name, vsgBillboardClass->name ) == 0 ||
	 strcmp( name, vsgAnchorClass->name ) == 0 ||
	 strcmp( name, "GeoLocation" ) == 0 ) {

      /* we have a group node with a children exposedField - traverse it */

      intern = vsgIntern( world, "children" );
      event_out = vsgGetFunc( node, intern );
      if ( event_out.f.MFString == NULL ) continue;
      if ( ( children = vsgGetMFNode( world, node, event_out ) ) == NULL )
	continue;

      FeatureCountNodes( world, children, item );
    }
  }
}

/*
 * Doc++ Description: *//**
 *   Returns an initialized FeatureItem struct, and pushes it on the 
 *   internal list of features.
 *
 * (ISSUE:  should this really be part of the public interface?  Called one in
 * .../XmTV/FeatureFE.c  --  refactor?)
 *
 * @param url  url to be associated with the return FeatureItem.
 *
 * @return  Pointer to a newly allocated and initialized FeatureItem.
 *
 * @see     FeatureItem
 *
 */
FeatureItem *
FeatureAlloc (char const *url, int add_to_list)
{
  FeatureItem *item;

  if ( ( item = (FeatureItem *) tsmMemCalloc(1, sizeof(FeatureItem)) ) == NULL)
    return NULL;

  item->id       = last_id++;
  item->url      = tsmMemStrdup( url );
  item->refcount = 1;
  item->wrl      = NULL;
  item->loaded   = FALSE;

  if ( add_to_list )
    list_push( feature_list, item );

  return item;
}

/*
 * Doc++ Description: *//**
 *   Frees a FeatureItem structure. 
 *
 * @param item the FeatureItem to be freed
 *
 * @return  void
 *
 * @see     FeatureItem
 *
 */
void
FeatureFree( FeatureItem *item )
{
  if ( item ) {
    if ( item->url ) tsmMemFree( item->url );
    if ( item->name ) tsmMemFree( item->name );
    if ( item->geoset ) tsmMemFree( item->geoset );
    if ( item->author ) tsmMemFree( item->author );
    if ( item->copyright ) tsmMemFree( item->copyright );
    if ( item->description ) tsmMemFree( item->description );
    if ( item->wrl ) vsgDestroyWorld( item->wrl );
    tsmMemFree( item );
  }
}

/* create a unique hash if URL is not set - needs to be freed by client */

char *
FeatureURLHash( char const *url )
{
  static int  id = 0;
  char        hash[32];
	
	if ( url ) return strdup( url );

	sprintf( hash, "internal_model_%d", ++id );

	return strdup( hash );
}

/* complete the loading of all .wrl resources and register the model */

static FeatureItem *
FeatureRegister (tvBase *threeD, FeatureItem *item)
{
  static char const funcName[] = "FeatureRegister";

  /* parse the contents of the file's METADATA node */

  if ( FeatureParseMetadata( item ) == FALSE ) {
    tsmError( funcName, "No valid METADATA node for %s", item->url );
  }

  /* expand all of the Inline nodes for this feature at load time */
  
  FeatureInlineTraverse( threeD, item, item->wrl, vsgWorldNodes( item->wrl ),
			 TRUE, threeD->f_loadtextures );

  /* pre-load all of the textures for this model */

  if ( threeD->f_loadtextures )
    item->textures = FeatureLoadTextures( threeD, item->wrl );

  /* refresh the screen */

  tvBasePostRedraw( threeD, FALSE );
  tvBaseConsole(threeD, "Loaded feature successfully - %s", item->url );

  /* signal that we have finished loading and exit this thread */

  item->loaded = TRUE;
  item->closure = NULL;

  return item;
}


/*
 * Doc++ Description: *//**
 *   Loads features into the given FeatureItem struct.
 *
 * @param threeD   handle to rendering state.
 * @param item     feature item to load.
 *
 * @return   Returns to given FeatureItem upon success.  Returns NULL on 
 *           failure.
 *
 * @see   FeatureItem
 *
 */
FeatureItem *
FeatureLoad (tvBase *threeD, FeatureItem *item)
{
  static char const funcName[] = "FeatureLoad";
  FILE               *stream;

  if ( threeD == NULL || item == NULL )
    return NULL;

  tvBaseConsole(threeD, "Loading feature - %s", item->url );

  if ( ( stream = tsmOpenStream( item->url, "r" ) ) == NULL ) {
    list_remove( feature_list, item );
    tvBaseBusy (threeD, "Could not open feature file URL:\n%s", item->url );
    tvBaseConsole(threeD, "Failed to open feature - %s", item->url );
    return NULL;
  }
  
  item->wrl = vsgNewWorld( 0 );

  /*vsgTracing = 1;*/
  if ( vsgParseStream( item->wrl, stream, tsmMemStrdup( item->url ) ) != 0 ) {
    tvBaseBusy (threeD, "Error parsing VRML file at:\n%s", item->url );
    tvBaseConsole(threeD, "Failed to parse feature - %s", item->url );
    list_remove( feature_list, item );
    return NULL;
  }

  item->filesize = ftell( stream );
  tsmCloseStream( stream );

  return FeatureRegister( threeD, item );
}

/*
 * Doc++ Description: *//**
 *   Parses a feature from a buffer into the given FeatureItem struct.
 *
 * @param threeD   handle to rendering state.
 * @param vrml     the buffer containing the VRML97 scene
 * @param url      optional url to actual resource (for hashing)
 *
 * @return   Returns to given FeatureItem upon success.  Returns NULL on 
 *           failure.
 *
 * @see   FeatureItem
 *
 */
FeatureItem *
FeatureParse (tvBase *threeD, const char *vrml, const char *url)
{
  static char const funcName[] = "FeatureParse";
  char        *hash = FeatureURLHash( url );
  FeatureItem *item;

  if ( threeD == NULL || vrml == NULL || hash == NULL )
    return NULL;

  if ( ( item = FeatureAlloc( hash, TRUE ) ) == NULL )
    return NULL;

	free( hash );

  item->wrl = vsgNewWorld( 0 );

  if ( vsgParseString( item->wrl, vrml ) ) {
    tvBaseBusy (threeD, "Error parsing VRML file from buffer" );
    tvBaseConsole(threeD, "Failed to parse feature from buffer" );
    list_remove( feature_list, item );
    return NULL;
  }

  item->filesize = strlen( vrml );

  return FeatureRegister( threeD, item );
}

/*
 * Doc++ Description: *//**
 *   Loads a feature, parses its contents, and creates a FeatureItem struct,
 *   however, does not add this to any viewers and does not recursively
 *   load any inlines or textures. This is meant to be able to parse out
 *   any metadata information to display to the user.
 *
 *   Use FeatureFree() to free the returned structure when done with it.
 *
 * @param url   url of the GeoVRML file to query
 *
 * @return   Returns valid FeatureItem if query was sucessful, else NULL
 *
 * @see   FeatureItem
 *
 */
FeatureItem *
FeatureQuery( char const *url )
{
  /* char const  funcName[] = "FeatureQuery"; */
  FILE        *stream;
  FeatureItem *item;

  if ( url == NULL || ( item = FeatureAlloc( url, FALSE ) ) == NULL )
    return NULL;

  if ( ( stream = tsmOpenStream( url, "r" ) ) == NULL )
    return NULL;
  
  item->wrl = vsgNewWorld( 0 );

  if ( vsgParseStream( item->wrl, stream, tsmMemStrdup( url ) ) != 0 )
    return NULL;

  item->filesize = ftell( stream );
  tsmCloseStream( stream );

  /* parse the contents of the file's METADATA node */

  FeatureParseMetadata( item );

  /* count the number of texture, inline, and anchor nodes */

  FeatureCountNodes( item->wrl, vsgWorldNodes( item->wrl ), item );

  return item;
}

/*
 * Synopsis:
 *   int FeatureLoadTextures( tvBase *threeD, vsgWorld *world )
 *
 * Description:
 *   This function will traverse the specified scene graph and load
 *   all of the texture maps from the ImageTexture nodes into memory.
 *   If a texture map is already in memory, then we simply increment
 *   its reference count.
 *
 * Returns: The total number of textures that were loaded (or in memory).
 *
 * Author: Martin Reddy
 * 
 * Date: 14 April, 1999
 *
 */

int
FeatureLoadTextures( tvBase *threeD, vsgWorld *world )
{
  if ( world == NULL ) return 0;
  return FeatureTexturesTraverse( threeD, world, vsgWorldNodes( world ),
				  FeatureTexLoad );
}

/*
 * Synopsis:
 *   int FeatureUnloadTextures( vsgWorld *world )
 *
 * Description:
 *   This function will traverse the specified scene graph and unloads
 *   all of the texture maps of the ImageTexture nodes from memory.
 *   If a texture map is being used by another scene graph, then we
 *   just decrease its reference count but keep it hanging around.
 *
 * Returns: The total number of textures that were unloaded (or had
 *          their reference count decremented)
 *
 * Author: Martin Reddy
 * 
 * Date: 14 April, 1999
 *
 */

static int
FeatureUnloadTextures( tvBase *threeD, vsgWorld *world )
{
  if ( world == NULL ) return 0;
  return FeatureTexturesTraverse( threeD, world, vsgWorldNodes( world ),
				  FeatureTexUnload );
}

/*
 * Synopsis:
 *   vsgWorld *FeatureLoadInline( char const *url )
 *
 * Description:
 *   This function will load an URL that has been specified by an
 *   Inline node in a VRML file. First we check to see that the 
 *   inlined file has not already been loaded - if it has then
 *   we just return the previously loaded world. Otherwise, we try
 *   to load and parse the specified VRML feature file and then add
 *   this file to the list of loaded feature files. We return the
 *   parsed scene graph iff this succeeds.
 * 
 *   N.B. there is no internal distinction between a root VRML file
 *   or an inlined file - they are all stored in the same list.
 *
 *   You can use FeatureUnload in order to unload an inline file.
 *
 *   We have a custom function to load inlines because there are
 *   some subtle differences between loading the root file and
 *   loading its inlines, e.g. we don't want to close the busy
 *   dialog afterwards, if we fail to load a file then we do
 *   not consider it to be fatal, we don't need to parse any
 *   METADATA nodes again (although we do), loading doesn't
 *   happen in a separate thread, etc.
 *
 * Returns: A valid vsgWorld pointer, or NULL if operation failed
 *
 * Author: Martin Reddy
 * 
 * Date: 19 April, 1999
 *
 */

static vsgWorld *
FeatureLoadInline( char const *url )
{
  static const char *funcName = "FeatureLoadInline";
  FILE              *stream;
  list_iter_t       i;
  FeatureItem       *item;
  vsgWorld          *world;

  /* search the feature file list to see if this URL has already been loaded */

  for ( i = list_first( feature_list );
	! list_done( i );
	i = list_next( i ) ){
    item = (FeatureItem *) list_value( i );
    if ( item && strcmp( item->url, url ) == 0 ) {
      item->refcount++;
      return item->wrl;
    }
  }

  /* okay, this is a new file - let's try and open it */

  if ( ( stream = tsmOpenStream( url, "r" ) ) == NULL ) {
    tsmError( funcName, "Could not find inline: %s", url );
    return NULL;
  }

  /* we have opened the VRML file successfully - let's try and parse it */

  world = vsgNewWorld( 0 );

  if ( vsgParseStream( world, stream, tsmMemStrdup( url ) ) != 0 ) {
    tsmError( funcName, "Error parsing inline file: %s", url );
    tsmCloseStream( stream );
    return NULL;
  }

  tsmCloseStream( stream );

  /* okay, we have successfully loaded the inline world! */

  if ( ! ( item = (FeatureItem *) tsmMemCalloc(1, sizeof(FeatureItem)) ) ) {
    vsgDestroyWorld( world );
    return NULL;
  }

  item->id       = last_id++;
  item->url      = tsmMemStrdup( url );
  item->refcount = 1;
  item->wrl      = world;
  item->loaded   = TRUE;

  /* we don't need to parse any METADATA node, but just to preserve the  */
  /* fact that, internally, Inline-loaded files look no different from   */
  /* root VRML files, we will parse the METADATA node for the inline too */

  FeatureParseMetadata( item );

  /* add this inlined world to our global list of loaded VRML files */

  list_push( feature_list, item );

  return world;
}

/*
 * Synopsis:
 *   vsgWorld *FeatureGetInline( vsgWorld *world, char const *url )
 *
 * Description:
 *   Search through the list of all loaded features looking for one
 *   with the specified url. If one is found, then we return a pointer
 *   to the vsgWorld that contains that feature's VRML scene graph.
 *
 * Returns: A vsgWorld pointer for the inline file, or NULL if not loaded
 *
 * Author: Martin Reddy
 * 
 * Date: 19 April, 1999
 *
 */

vsgWorld *
FeatureGetInline( vsgWorld *world, char const *url )
{
  list_iter_t i;
  FeatureItem *item;
  char        absurl[512];

  if ( url == NULL ) return NULL;

  FeatureAbsUrl( world, url, absurl );

  /* search through the feature list looking for a feature with the */
  /* specified url. Return it's scene graph if we find it.          */

  for ( i = list_first( feature_list );
	! list_done( i );
	i = list_next( i ) ){
    item = (FeatureItem *) list_value( i );
    if ( strcmp( item->url, absurl ) == 0 && item->wrl ) 
      return item->wrl;
  }

  /* return NULL if the file has not been loaded into memory */

  return NULL;
}

/*
 * Synopsis:
 *   int FeatureInlineTraverse( tvBase *threeD, FeatureItem *item,
 *                              vsgWorld *world, MFNode *mfnode,
 *                              int load_inline, int load_textures )
 *
 * Description:
 *   This function will traverse through an entire VRML scene graph
 *   looking for all Inline nodes. If any are found, then we either
 *   (recursively) load them into memory (load_it==TRUE) or
 *   unload them (load_it==FALSE).
 *
 *   url is an MFString in Inline, therefore if the first url
 *   string cannot be found/loaded, then we progress on to the second
 *   url and try this. This continues until we find a valid inline file.
 *   This is in line with the VRML97 specification.
 *
 *   This function returns the number of invocations of the action
 *   callback function which returned a result of TRUE. For example,
 *   if action = FeatureTexLoad, then the result of this function is
 *   the number of textures that were loaded for this model.
 *
 * Returns: The number of inline files that were loaded (or 0 if
 *          load_it==FALSE)
 *
 * Author: Martin Reddy
 * 
 * Date: 19 April, 1999
 *
 */

int
FeatureInlineTraverse( tvBase *threeD, FeatureItem *item, vsgWorld *world,
		       MFNode *mfnode, int load_it, int load_textures )
{
  /*static const char funcName[] = "FeatureInlineTraverse";*/
  int             i, j, count = 0;
  SFNode          node;
  MFString        *url;
  char const      *name;
  char const      *intern;
  vsgEventOutDesc event_out;
  vsgWorld        *inline_wrl;
  MFNode          *children;
  char            absurl[512];

  if ( world == NULL || mfnode == NULL ) return 0;

  for ( i = 0; i < mfnode->mf.len; i++ ) {
    node = mfnode->data[i];
    name = node->cl->name;

    /* what to do with this node? */

    if ( strncmp( name, "Geo", 3 ) == 0 && item->geoorigin_parsed == FALSE ) {

      /* while we are traversing the scene graph, let's look for */
      /* a GeoVRML GeoOrigin node so that we can get the origin. */

      intern = vsgIntern( world, "geoOrigin" );
      event_out = vsgGetFunc( node, intern );
      if ( event_out.f.SFNode != NULL ) {

	/* parse out the geocentric origin for this GeoVRML world */

	GeoVRML gv = gvNew( world );
	gvGetOrigin( gv, node, item->gcc_origin );
	gvFree( gv );

	item->geoorigin_parsed = TRUE;
      }

    }

    if ( strcmp( name, "GeoMetadata" ) == 0 ) {

      /* do we have a GeoVRML metadata node? If so, parse out its values */

      char *value = NULL;
      GeoVRML gv = gvNew( world );

      if ( ( value = gvGetMetadataItem( gv, node, "title" ) ) != NULL ) {
	if ( item->name ) tsmMemFree( item->name );
	item->name = value;
      }

      if ( ( value = gvGetMetadataItem( gv, node, "description" ) ) != NULL ) {
	if ( item->description ) tsmMemFree( item->description );
	item->description = value;
      }

      if ( ( value = gvGetMetadataItem( gv, node, "copyright" ) ) != NULL ) {
	if ( item->copyright ) tsmMemFree( item->copyright );
	item->copyright = value;
      }

      if ( ( value = gvGetMetadataItem( gv, node, "originator" ) ) != NULL ) {
	if ( item->author ) tsmMemFree( item->author );
	item->author = value;
      }

      gvFree( gv );

    } else if ( strcmp( name, vsgCollisionClass->name ) == 0 ||
		strcmp( name, vsgGroupClass->name ) == 0 ||
		strcmp( name, vsgTransformClass->name ) == 0 ||
		strcmp( name, vsgBillboardClass->name ) == 0 ||
		strcmp( name, vsgAnchorClass->name ) == 0 ||
		strcmp( name, "GeoLocation" ) == 0 ) {

      /* we have a group node with a children exposedField - traverse it */

      intern = vsgIntern( world, "children" );
      event_out = vsgGetFunc( node, intern );
      if ( event_out.f.MFString == NULL ) continue;
      if ( ( children = vsgGetMFNode( world, node, event_out ) ) == NULL )
	continue;

      count += FeatureInlineTraverse( threeD, item, item->wrl, children,
				      load_it, load_textures );

      /* if we have an anchor node, then increment our count of these */

      if ( strcmp( name, vsgAnchorClass->name ) == 0 )
	item->anchors++;

    } else if ( strcmp( name, vsgInlineClass->name ) == 0 ) {

      /* we have an Inline node - get its url field */

      intern = vsgIntern( world, "url" );
      event_out = vsgGetFunc( node, intern );
      if ( event_out.f.MFString == NULL ) continue;
      if ( ( url = vsgGetMFString( world, node, event_out ) ) == NULL )
	continue;

      item->inlines++;

      /* go through each url until we find an inline we can load */

      for ( j = 0; j < url->mf.len; j++ ) {

	/* convert any relative URLs to absolute */

	FeatureAbsUrl( world, url->data[j], absurl );

	/* update the busy dialog with the Inline's location */

	tvBaseBusy (threeD, "Loading Inline file from URL:\n%s", absurl );

	/* have we been told to load or unload the inline file? */

	if ( load_it ) {

	  /* load the new scene (+ textures) and then recurse over it */

	  if ( ( inline_wrl = FeatureLoadInline( absurl ) ) != NULL ) {

	    if ( load_textures )
	      FeatureLoadTextures( threeD, inline_wrl );

	    children = vsgWorldNodes( inline_wrl );
	    count += FeatureInlineTraverse( threeD, item, inline_wrl, children,
					    TRUE, load_textures );
	    count++;
	    break;
	  }
	  
	} else {

	  /* recurse over the scene, if loaded, and then unload it */
	  /* The FeatureUnload routine deals with recursively      */
	  /* unloading inlines and texture maps for us.            */

	  if ( FeatureUnload( NULL, absurl ) >= 0 )
	    break;

	}

      }

    } else if ( strcmp( name, "GeoOrigin" ) == 0 ) {

      /* while we are traversing the scene graph, let's look for */
      /* a GeoVRML GeoOrigin node so that we can get the origin. */

      puts( "Got a GeoOrigin node!" );
    }
  }

  return count;
}

/*
 * Synopsis:
 *   int FeatureUnload( tvBase *threeD, char *url )
 *
 * Doc++ Description:  *//**
 *   Searches through the list of loaded feature files looking for
 *   one for the specified URL. If this is found, then its reference
 *   count is decreased. If the reference count for an object reaches
 *   zero, then it is unloaded from memory and removed from the global
 *   list of features. The ID of the removed feature is returned as
 *   the result of this function. The calling routine must make sure
 *   that this ID is removed from the ThreeDWidget's list of feature
 *   IDs. If the feature URL was not found, then nothing is performed
 *   and the function returns a result of -1.
 *
 *   If you pass NULL for the threeDWidget, then we do not attempt
 *   to refresh the display.
 *
 * @param threeD   handle to rendering state.
 * @param url      lookup key in search
 *
 * @return    The ID of the feature that was unloaded, or -1 if no feature
 *
 * @author    Martin Reddy
 * 
 * @version   9 April, 1999
 *
 */

int
FeatureUnload( tvBase *threeD, char *url )
{
  list_iter_t i;
  FeatureItem *item;
  int         id;
  char        *filename;

  if ( threeD == NULL || url == NULL ) return -1;

  /* search the list to see if this URL has already been loaded */

  for ( i = list_first( feature_list );
	! list_done( i );
	i = list_next( i ) ){
    item = (FeatureItem *) list_value( i );

    if ( item && strcmp( item->url, url ) == 0 ) {
      item->refcount--;
      id = item->id;

      /* if there are no more references to this feature, */
      /* free it and remove it from the list of features. */

      if ( item->refcount <= 0 ) {
	if ( item->wrl != NULL ) {

	  /* unload all of the files that are inlined by this file */

	  FeatureInlineTraverse( threeD, item, item->wrl,
				 vsgWorldNodes( item->wrl ), FALSE, FALSE );

	  /* unload all of the textures for this file */

	  FeatureUnloadTextures( threeD, item->wrl );

	  /* unload the VRML scene graph */

    filename = (char *) vsgWorldFilename( item->wrl );
    if ( filename && strcmp( filename, "<STRING>" ) != 0 )
      tsmMemFree( filename );

    vsgDestroyWorld( item->wrl );

	  /* free any of the strings from the metadata node */

	  if ( item->name ) tsmMemFree( item->name );
	  if ( item->geoset ) tsmMemFree( item->geoset );
	  if ( item->author ) tsmMemFree( item->author );
	  if ( item->description ) tsmMemFree( item->description );
	  if ( item->copyright ) tsmMemFree( item->copyright );

	}
  tvBaseConsole( threeD, "Unloaded feature - %s", item->url );
	if ( item->url != NULL ) tsmMemFree( item->url );
	tsmMemFree( item );
	list_delete( i );

	/* XXX - question: this function is called recursively, so we  */
	/* can delete entries from the list while we are traversing    */
	/* through it - this seems to work okay now but this behaviour */
	/* may be just pathological. Look here again if problems occur */

      }

      /* post a redraw to update the viewer window */

      if ( threeD )
	tvBasePostRedraw( threeD, FALSE );

      return id;
    }
  }

  return -1;
}

/*
 * Synopsis:
 *   int FeatureUnloadAll( tvBase *threeD )
 *
 * Doc++ Description:  *//**
 *   Unloads all models from the specified viewers, freeing the
 *   associated memory if the reference count reaches zero.

 * @param threeD   handle to rendering state.
 *
 * @return    void
 *
 * @author    Martin Reddy
 * 
 * @version   18 December, 2001
 *
 */

void
FeatureUnloadAll( tvBase *threeD )
{
	if ( feature_list == NULL || threeD == NULL )
		return;

	while ( ! list_empty( feature_list ) ) {
		list_iter_t i;
		FeatureItem *item;
		/* do it this way because feature_list is updated */
		/* when we do the actual FeatureUnload() call.    */
		i = list_first( feature_list );
		item = list_value( i );

		if ( item && item->url ) {
			char *url = strdup( item->url );
			FeatureUnload( threeD, url );
			free( url );
		}
	}
}

/*
 * Synopsis:
 *   int FeatureDisplay( tvBase *threeD )
 *
 * Description:
 *   Renders all of the VRML features that have been associated with
 *   the specified tvBase. The widget has a list of IDs that
 *   are used as lookups into the global feature list. We then render
 *   this subset of features. Because each tvBase has its own
 *   list of feature IDs, we can display different subsets of features
 *   in different 3-D viewers.
 *
 * Returns: TRUE
 *
 * Author: Martin Reddy
 * 
 * Date: 9 April, 1999
 *
 */

int
FeatureDisplay( tvBase *threeD, TsGeoPyramid *geo )
{
  list_iter_t     i;
  FeatureInstance *instance;
  FeatureItem     *item;
  list_t          *list = threeD->fselIDs;
  tvViewpoint const *view = &threeD->view;

  /* iterate through each feature ID in the widget's ID list */

  for ( i = list_first( list ); ! list_done( i ); i = list_next( i ) ) {

    /* get the next feature to display. Don't try to display */
    /* it if it is not fully loaded yet though!              */

    if ( ( instance = (FeatureInstance *) list_value( i ) ) == NULL )
      continue;

    item = FeatureGetItem( instance->id );

    if ( item == NULL || item->loaded == FALSE || item->wrl == NULL )
      continue;

    /* don't display in a local coord system if we have no origin */

    if ( geo != NULL && item->which_origin == FEATURE_ORIGIN_GEOVRML )
      continue;

    /* and vice versa */

    if ( geo == NULL && item->which_origin != FEATURE_ORIGIN_GEOVRML )
      continue;

    /* only display this feature if it is for the specified TsGeoPyramid */

    if ( item->geoset == NULL && geo )
      continue;

    if ( geo && strcasecmp( item->geoset, TsGeoName( geo ) ) != 0 )
      continue;

    /* okay, we have a valid VRML scene graph - chuck it to the pipe */

    glPushMatrix(); {

      /* Move the render frame to the origin of the feature */
      /* within the local coordinate system of the dataset. */

      if ( item->which_origin == FEATURE_ORIGIN_GCC ) {
				
				/* model location specified as an absolute geocentric coord */
				/* and set up the correct local orientation for this point  */

				float orient[4];
				
				TsLVCSOrientation( item->gcc_origin, orient );
				tvBaseGLViewpointCustomLVCS( view, item->gcc_origin, orient );
				
      } else if ( item->which_origin == FEATURE_ORIGIN_LCS ) {
				
				/* model location specified as offset from origin of dataset */
				
				tvBaseGLViewpointLVCS( view, geo );
				
				glTranslatef( (float) item->lcs_offset[0],
											(float) item->lcs_offset[1],
											(float) item->lcs_offset[2] );
				
      } else if ( item->which_origin == FEATURE_ORIGIN_GEOVRML ) {
				
				/* we have a GeoVRML GeoOrigin to translate to (no orientation) */
				
				float orient[4] = { 0.0, 0.0, 0.0, 0.0 };
				tvBaseGLViewpointCustomLVCS( view, item->gcc_origin, orient );
      }
			
      /* are we converting from VRML's Y-up coordinate system to */
      /* TV's Z-up? If so then we need to do a rotate at the top */
      
      if ( item->y_up && item->which_origin != FEATURE_ORIGIN_GEOVRML )
				glRotatef( 90, 1, 0, 0 );
			
      /* render the feature */
			
      vsgRender( item->wrl, instance );
      
    }; glPopMatrix();

  }

  return TRUE;
}

/*
 * Synopsis:
 *   FeatureItem *GetFeatureFromUrl( char const *url )
 *
 * Doc++ Description: *//**
 *   Checks to see if a feature file with the specified URL is
 *   currently loaded in memory. Returns a pointer to the FeatureItem
 *   structure if this is the case, or NULL if the feature URL does
 *   not exist in memory.
 *
 * @param  url  search key
 *
 * @return   pointer to FeatureItem if URL is loaded.
 *
 * @author   Martin Reddy
 * 
 * @version  26 May, 1999
 *
 * @see      FeatureItem
 *
 */

FeatureItem *
GetFeatureFromUrl( char const *url )
{
  list_iter_t i;
  FeatureItem *item;

  for ( i = list_first( feature_list );
	! list_done( i );
	i = list_next( i ) ){
    item = (FeatureItem *) list_value( i );
    if ( item == NULL || item->url == NULL ) continue;
    if ( strcmp( item->url, url ) == 0 ) return item;
  }

  return NULL;
}

FeatureItem *
FeatureRefCountLoad( tvBase *threeD, char *url )
{
  FeatureItem *item;

  if ( threeD == NULL || url == NULL ) return NULL;

  if ( ( item = GetFeatureFromUrl( url ) ) )
    item->refcount++;
  else
    item = FeatureLoad( threeD, FeatureAlloc( url, TRUE ) );

  if ( item )
    FeatureAddToWidget( threeD, item->id );

  return item;
}

FeatureItem *
FeatureRefCountParse( tvBase *threeD, char const *buffer, const char *url )
{
  FeatureItem *item;
  char        *hash = FeatureURLHash( url );

  if ( threeD == NULL || buffer == NULL || hash == NULL )
		return NULL;

  if ( ( item = GetFeatureFromUrl( url ) ) )
    item->refcount++;
  else
    item = FeatureParse( threeD, buffer, hash );

  if ( item )
    FeatureAddToWidget( threeD, item->id );

	free( hash );
  return item;
}

/*
 * Synopsis:
 *   int FeatureSetHyperlink_r( tvBase *, vsgWorld *world, MFNode *mfnode,
 *                              int anchor, int found )
 *
 * Description:
 *   This function will traverse through an entire VRML scene graph
 *   looking for Anchor nodes. It searches for "anchor"'th such node.
 *   When it finds this numbered Anchor node, it will set the hyperlink
 *   state of the tvBase structure from that Anchor node, i.e. the
 *   url and description fields.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 26 October, 2000
 *
 */

static void
FeatureSetHyperlink_r( tvBase *threeD, vsgWorld *world, MFNode *mfnode,
		       int *curr_anchor, int target_anchor, int *found )
{
  /*static const char funcName[] = "FeatureSetHyperlink_r";*/
  int             i, j;
  SFNode          node;
  MFString        *url;
  SFString        desc;
  char const      *name;
  char const      *intern;
  vsgEventOutDesc event_out;
  MFNode          *children;

  if ( world == NULL || mfnode == NULL || *found ) return;
  
  for ( i = 0; i < mfnode->mf.len; i++ ) {
    node = mfnode->data[i];
    name = node->cl->name;

    /* is this node an Anchor node? If so, is it the one we want? */

    if ( strcmp( name, vsgAnchorClass->name ) == 0 ) {
      
      if ( ++(*curr_anchor) == target_anchor ) {

				/* get a copy of the "url" field from the Anchor node */

				if ( threeD->f_pick_url ) {
					free( threeD->f_pick_url );
					threeD->f_pick_url = NULL;
				}

				intern = vsgIntern( world, "url" );
				event_out = vsgGetFunc( node, intern );
				if ( event_out.f.MFString ) {
					if ( ( url = vsgGetMFString( world, node, event_out ) ) ) {
						if ( url->mf.len > 0 )
							threeD->f_pick_url = strdup( url->data[0] );
					}
				}
				
				/* always have something, as we use NULL to mean no hyperlink */

				if ( ! threeD->f_pick_url ) 
					threeD->f_pick_url = tsmMemStrdup( "" );
				
				/* get a copy of the "description" field from the Anchor node */
				
				if ( threeD->f_pick_desc ) {
					free( threeD->f_pick_desc );
					threeD->f_pick_desc = NULL;
				}
				
				intern = vsgIntern( world, "description" );
				event_out = vsgGetFunc( node, intern );
				if ( event_out.f.MFString ) {
					if ( ( desc = vsgGetSFString( world, node, event_out ) ) ) {
						threeD->f_pick_desc = strdup( desc );
					}
				}
				
				*found = TRUE;
				return;
      }
      
    }

    /* if we have an Inline node, recurse through its children */

    if ( strcmp( name, vsgInlineClass->name ) == 0 ) {
      vsgWorld *inline_wrl;
			MFString *il_children;
      intern = vsgIntern( world, "url" );
      event_out = vsgGetFunc( node, intern );
      if ( event_out.f.MFString == NULL ) continue;
      if ( ( il_children = vsgGetMFString ( world, node, event_out ) ) == NULL )
				continue;

      /* find one of the URL strings that we have loaded */
      for ( j = 0; j < il_children->mf.len; j++ ) {
        const char *url = il_children->data[j];
        if ( ! url ) continue;
        inline_wrl = FeatureGetInline( world, url );
        if ( ! inline_wrl ) continue;

        /* now recurse into this Inline node's scene graph */
        FeatureSetHyperlink_r( threeD, inline_wrl, vsgWorldNodes(inline_wrl),
                               curr_anchor, target_anchor, found );
        if ( *found ) return;
        break;
      }
    
    } else if ( strcmp( name, vsgCollisionClass->name ) == 0 ||
								strcmp( name, vsgGroupClass->name ) == 0 ||
								strcmp( name, vsgTransformClass->name ) == 0 ||
								strcmp( name, vsgBillboardClass->name ) == 0 ||
								strcmp( name, vsgAnchorClass->name ) == 0 ||
								strcmp( name, "GeoLocation" ) == 0 ) {
      
      /* we have a group node with a children exposedField - traverse it */
			
      intern = vsgIntern( world, "children" );
      event_out = vsgGetFunc( node, intern );
      if ( event_out.f.MFString == NULL ) continue;
      if ( ( children = vsgGetMFNode( world, node, event_out ) ) == NULL )
				continue;
			
      FeatureSetHyperlink_r( threeD, world, children, curr_anchor,
														 target_anchor, found );
			
      if ( *found ) return;
    }
  }
}

/*
 * Synopsis:
 *   void FeatureSetHyperlink( tvBase *threeD, int fid, int anchor )
 *
 * Description:
 *   This function will traverse through an entire VRML scene graph
 *   looking for a numbered Anchor node. When this particular Anchor
 *   node is found, it sets the tvBase structure's hyperlink state
 *   (url and description) from that Anchor node.
 *
 * @param threeD   the tvBase structure to be modified
 * @param fid      the feature ID for the VRML scene graph to be searched
 * @param anchor   the sequential number of the Anchor node to find (1..n)
 *
 * @return void
 *
 * @author Martin Reddy
 * 
 * @version 26 October, 2000
 *
 */

void
FeatureSetHyperlink( tvBase *threeD, int fid, int anchor )
{
  FeatureItem *item;
  int         found = FALSE, curr_anchor = 0;

  /* check that we have a valid value to work with */

  if ( threeD == NULL || fid < 0 || anchor <= 0 )
    return;

  /* lookup the feature item structure from the ID number */

  if ( ( item = FeatureGetItem( fid ) ) == NULL )
    return;

  /* finally, search through the scene graph for the required anchor node */

  FeatureSetHyperlink_r( threeD, item->wrl, vsgWorldNodes( item->wrl ),
												 &curr_anchor, anchor, &found );
}

