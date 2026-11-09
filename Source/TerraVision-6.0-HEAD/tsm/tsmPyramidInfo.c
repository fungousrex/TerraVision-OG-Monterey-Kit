/*
 * Name: tsmPyramidInfo_dpss.c
 *
 * Description: 
 *   Contains routines for compiling complete server information about
 *   datasets.
 *
 * Function List:
 *   PyramidInfo *tsmGetPyramidInfo( TsmHandle *, const char * );
 *   PyramidInfos *tsmGetPyramidInfos( TsmHandle *, int, char ** );
 *   PyramidInfos *tsmGetAllPyramidInfos( TsmHandle * );
 *
 *   PyramidInfo *tsmFreePyramidInfo( PyramidInfo *info );
 *   PyramidInfo *tsmMakePyramidInfo( void );
 *   ServerGroup *tsmFreeServerGroupContents( ServerGroup *group );
 *
 *   int tsmSetPyramidInfoName( PyramidInfo *info, char *name );
 *   int tsmSetPyramidInfoPyramid( PyramidInfo *info, Pyramid *pyramid );
 *   int tsmSetPyramidInfoSetId( PyramidInfo *info, int set_id,
 *                               int server_group );
 *   int tsmSetPyramidInfoMaster( PyramidInfo *info, char *master,
 *                                int server_group );
 *   int tsmAddPyramidInfoServer( PyramidInfo *info, char *server,
 *                                int server_group );
 *   int tsmAddPyramidInfoMetaData( PyramidInfo *info, char *meta,
 *                                  int server_group );
 *
 *   void tsmPrintPyramidInfo (FILE *stream, PyramidInfo *pyramidInfo);
 *   void tsmPrintPyramidInfos (FILE *stream, PyramidInfos *pyramidInfos);
 *   void tsmPrintServerGroup (FILE *stream, ServerGroup *serverGroup,
 *                             char *leadString);
 *
 *   int tsmSetServerGroupId( ServerGroup *group, int set_id );
 *   int tsmSetServerGroupMaster( ServerGroup *group, char *master );
 *   int tsmAddServerGroupServer( ServerGroup *group, char *server );
 *   int tsmAddServerGroupMetaData( ServerGroup *group, char *meta_data );
 *   int tsmGrowPyramidInfoServerArray( PyramidInfo *info, int new_size );
 *
 * Revision History:
 *   Created 12 Jan 1998 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmPyramidInfo.c,v 1.9 2000/08/28 22:16:00 reddy Exp $
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

/*
 * Synopsis:
 *   int tsmSetPyramidInfoName( PyramidInfo *info, char *name )
 *
 * Doc++ Description: *//** 
 *    Copy the name into the PyramidInfo structure.
 *
 *    The PyramidInfo structure contains all of the information required
 *    to describe a single instance of a pyramid, be it on local disk or
 *    on a DPSS. This is a convenience function to allow you to fill in
 *    the name field of the PyramidInfo structure. This is used to provide
 *    the name of the Pyramid, e.g. ntc-1-utm-2scene.128.oi.
 *
 * @param    info  the PyramidInfo structure to update
 * @param    name  the name of the Pyramid to strdup into the PyramidInfo
 *
 * @return   TRUE if all went well, FALSE if failed (NULL pointer passed)
 *
 * @author   Martin Reddy
 * @version  31 October 1997
 *
 * @see      tsmSetPyramidInfoPyramid, PyramidInfo
 *
 */

int
tsmSetPyramidInfoName( PyramidInfo *info, char *name )
{
  const char *funcName = "tsmSetPyramidInfoName";

  if ( info == NULL ) {
    tsmError( funcName, "NULL pointer passed for PyramidInfo" );
    return FALSE;
  }

  info->name = tsmMemManagerStrdup( name );

  return TRUE;
}

/*
 * Synopsis:
 *    int tsmSetPyramidInfoPyramid( PyramidInfo *info, Pyramid *pyramid )
 *
 * Doc++ Description: *//** 
 *    Add a Pyramid into the PyramidInfo structure.
 *
 *    The PyramidInfo structure contains all of the information required
 *    to describe a single instance of a pyramid, be it on local disk or
 *    on a DPSS.
 *
 *    This is a convenience function to allow you to fill in
 *    the pyramid field of the PyramidInfo structure. This is used to 
 *    provide a pointer to an already allocated Pyramid structure (the
 *    pointer is simply put into the pyramid field as-is, the Pyramid
 *    structure is not copied in any way).
 *
 * @param    info  the PyramidInfo structure to update
 * @param    name  the Pyramid structure to link into the PyramidInfo
 *
 * @return   TRUE if all went well, FALSE if failed (NULL pointer passed)
 *
 * @author   Martin Reddy
 * @version  31 October 1997
 *
 * @see      tsmSetPyramidInfoName, PyramidInfo
 *
 */

int
tsmSetPyramidInfoPyramid( PyramidInfo *info, Pyramid *pyramid )
{
  const char *funcName = "tsmSetPyramidInfoPyramid";

  if ( info == NULL ) {
    tsmError( funcName, "NULL pointer passed for PyramidInfo" );
    return FALSE;
  }

  info->pyramid = pyramid;

  return TRUE;
}

/*
 * Synopsis:
 *    int tsmSetServerGroupId( ServerGroup *group, int set_id )
 *    int tsmSetServerGroupMaster( ServerGroup *group, char *master )
 *    int tsmAddServerGroupServer( ServerGroup *group, char *server )
 *    int tsmAddServerGroupMetaData( ServerGroup *group, char *meta_data )
 *
 * Description:
 *    Routines to add data to a ServerGroup structure. The tsmAddServer...
 *    functions will automatically grow the relevant array and add the
 *    specific information to the end of that array. 
 *
 * Externals: None
 *
 * Returns: All return TRUE if successful, FALSE if an error occurred.
 *
 * Author: Martin Reddy
 *
 * Date: 31 October 1997
 *
 */

int 
tsmSetServerGroupId( ServerGroup *group, int set_id )
{
  const char *funcName = "tsmSetServerGroupId";

  if ( group == NULL ) {
    tsmError( funcName, "NULL pointer passed for ServerGroup" );
    return FALSE;
  }

  group->setId = set_id;

  return TRUE;
}

int 
tsmSetServerGroupMaster( ServerGroup *group, char *master )
{
  const char *funcName = "tsmSetServerGroupMaster";

  if ( group == NULL ) {
    tsmError( funcName, "NULL pointer passed for ServerGroup" );
    return FALSE;
  }

  group->masterHostName = tsmMemManagerStrdup( master );

  return TRUE;
}

int
tsmAddServerGroupServer( ServerGroup *group, char *server )
{
  const char *funcName = "tsmSetServerGroupServer";

  if ( group == NULL ) {
    tsmError( funcName, "NULL pointer passed for ServerGroup" );
    return FALSE;
  }

  group->numServers++;
  group->serverHostNames = (char **)
    tsmMemManagerRealloc( group->serverHostNames, group->numServers *
			  sizeof(char *) );
  
  if ( group->serverHostNames == NULL ) {
    tsmError( funcName, "Couldn't add an entry to the server array" );
    return FALSE;
  }

  group->serverHostNames[group->numServers-1] = tsmMemManagerStrdup( server );

  return TRUE;
}

int
tsmAddServerGroupMetaData( ServerGroup *group, char *meta_data )
{
  const char *funcName = "tsmSetServerGroupMetaData";
  Purl  *purl;

  if ( group == NULL ) {
    tsmError( funcName, "NULL pointer passed for ServerGroup" );
    return FALSE;
  }

  if ( ( purl = tsmParseUrl( meta_data ) ) == NULL ) {
    tsmError( funcName, "Couldn't parse the URL '%s'", meta_data );
    return FALSE;
  }

  group->numMetaData++;
  group->metaData = (Purl **)
    tsmMemManagerRealloc( group->metaData, group->numMetaData * 
			  sizeof(Purl *) );
  
  if ( group->metaData == NULL ) {
    tsmError( funcName, "Couldn't add an entry to the metadata array" );
    return FALSE;
  }

  group->metaData[group->numMetaData-1] = purl;

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmGrowPyramidInfoServerArray( PyramidInfo *info, int new_size )
 *
 * Description:
 *   Takes a PyramidInfo structure and grows its ServerGroup array (if
 *   necessary) in order to accommodate new_size number of ServerGroup
 *   structures. All newly allocated structures at the end of the list
 *   are initialised to contain zeros.
 *
 * Externals: None.
 *
 * Returns: TRUE if all went well, FALSE if we failed 
 *
 * Author: Martin Reddy
 *
 * Date: 31 October 1997
 *
 */

int
tsmGrowPyramidInfoServerArray( PyramidInfo *info, int new_size )
{
  const char *funcName = "tsmGrowPyramidInfoServerArray";

  if ( info == NULL ) {
    tsmError( funcName, "NULL pointer passed for PyramidInfo" );
    return FALSE;
  }

  /* nothing to do if array already big enough for new_size */

  if ( new_size <= info->numServerGroups )
    return TRUE;

  /* try to realloc the array to hold new_size ServerGroup structures */

  info->serverGroup = (ServerGroup *)
    tsmMemManagerRealloc( info->serverGroup, new_size * sizeof(ServerGroup) );

  if ( info->serverGroup == NULL ) {
    tsmError( funcName, "couldn't grow the server array to %d", new_size );
    return FALSE;
  }
  
  /* clear the new ServerGroup structures for all new array entries */

  memset( info->serverGroup + info->numServerGroups, 0,
	  ( new_size - info->numServerGroups ) * sizeof(ServerGroup) );

  /* set the new array size and exit */

  info->numServerGroups = new_size;

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmSetPyramidInfoSetId( PyramidInfo *info, int set_id,
 *                               int server_group )
 *
 * Doc++ Description: *//** 
 *   Sets the set ID for a server group in a PyramidInfo.
 *
 *    The PyramidInfo structure contains all of the information required
 *    to describe a single instance of a pyramid, be it on local disk or
 *    on a DPSS. In particular, a single pyramid can have a number of
 *    different server configurations.
 *
 *    This function lets you define the set ID for any server group of
 *    a Pyramid.
 *
 *    You specify which server group configuration of the pyramid you want
 *    to update by giving the server_group number. Internally, the PyramidInfo
 *    structure holds an array of ServerGroup structures - this function
 *    will automatically grow this server array if you specify a server_group
 *    number larger than the number of ServerGroups currently allocated, e.g.
 *    if you have not previously added any information to the ServerGroup
 *    array (i.e. the array is size 0), and you specify that you want to 
 *    update the contents of server_group 2, then this function will add two
 *    new ServerGroup entries to the array, clear the contents of both to 0,
 *    then add the specified information to the second of these.
 *
 * @param    info   the PyramidInfo structure describing the pyramid
 * @param    set_id the set ID of the server group specifed by server_group
 * @param    server_group the server group number to update (0,1,2,...)
 *
 * @return   TRUE if all went well, FALSE if something spooky happened.
 *
 * @author   Martin Reddy
 * @version  31 October 1997
 *
 * @see      tsmSetPyramidInfoMaster, tsmAddPyramidInfoServer,
 *           tsmAddPyramidInfoMetaData, tsmSetPyramidInfoName,
 *           tsmSetPyramidInfoPyramid, PyramidInfo
 *
 */

int
tsmSetPyramidInfoSetId( PyramidInfo *info, int set_id, int server_group )
{
  if ( tsmGrowPyramidInfoServerArray( info, server_group+1 ) == FALSE )
    return FALSE;

  return tsmSetServerGroupId( &info->serverGroup[server_group], set_id );
}

/*
 * Synopsis:
 *   int tsmSetPyramidInfoMaster( PyramidInfo *info, char *master,
 *                                int server_group )
 *
 * Doc++ Description: *//** 
 *   Sets the master host name for a server group in a PyramidInfo.
 *
 *    The PyramidInfo structure contains all of the information required
 *    to describe a single instance of a pyramid, be it on local disk or
 *    on a DPSS. In particular, a single pyramid can have a number of
 *    different server configurations.
 *
 *    This function lets you define the master host name for any server
 *    group of a Pyramid.
 *
 *    You specify which server group configuration of the pyramid you want
 *    to update by giving the server_group number. Internally, the PyramidInfo
 *    structure holds an array of ServerGroup structures - this function
 *    will automatically grow this server array if you specify a server_group
 *    number larger than the number of ServerGroups currently allocated, e.g.
 *    if you have not previously added any information to the ServerGroup
 *    array (i.e. the array is size 0), and you specify that you want to 
 *    update the contents of server_group 2, then this function will add two
 *    new ServerGroup entries to the array, clear the contents of both to 0,
 *    then add the specified information to the second of these.
 *
 * @param    info   the PyramidInfo structure describing the pyramid
 * @param    master the master host name for the server group specifed
 *                  by server_group
 * @param    server_group the server group number to update (0,1,2,...)
 *
 * @return   TRUE if all went well, FALSE if something spooky happened.
 *
 * @author   Martin Reddy
 * @version  31 October 1997
 *
 * @see      tsmSetPyramidInfoSetId, tsmAddPyramidInfoServer,
 *           tsmAddPyramidInfoMetaData, tsmSetPyramidInfoName,
 *           tsmSetPyramidInfoPyramid, PyramidInfo
 *
 */

int
tsmSetPyramidInfoMaster( PyramidInfo *info, char *master, int server_group )
{
  if ( tsmGrowPyramidInfoServerArray( info, server_group+1 ) == FALSE )
    return FALSE;

  return tsmSetServerGroupMaster( &info->serverGroup[server_group], master );
}

/*
 * Synopsis:
 *   int tsmAddPyramidInfoServer( PyramidInfo *info, char *server,
 *                                int server_group )
 *
 * Doc++ Description: *//** 
 *   Adds a server host name to a server group in a PyramidInfo.
 *
 *    The PyramidInfo structure contains all of the information required
 *    to describe a single instance of a pyramid, be it on local disk or
 *    on a DPSS. In particular, a single pyramid can have a number of
 *    different server configurations.
 *
 *    This function lets you add a server host name to the list of servers
 *    for any server group of a Pyramid. In each case, the server array of
 *    the specific server group will be grown by one for you automatically 
 *    and the new data added.
 *
 *    You specify which server group configuration of the pyramid you want
 *    to update by giving the server_group number. Internally, the PyramidInfo
 *    structure holds an array of ServerGroup structures - this function
 *    will automatically grow this server array if you specify a server_group
 *    number larger than the number of ServerGroups currently allocated, e.g.
 *    if you have not previously added any information to the ServerGroup
 *    array (i.e. the array is size 0), and you specify that you want to 
 *    update the contents of server_group 2, then this function will add two
 *    new ServerGroup entries to the array, clear the contents of both to 0,
 *    then add the specified information to the second of these.
 *
 * @param    info   the PyramidInfo structure describing the pyramid
 * @param    server the server host name to be added to the server group
 *                  specifed by server_group
 * @param    server_group the server group number to update (0,1,2,...)
 *
 * @return   TRUE if all went well, FALSE if something spooky happened.
 *
 * @author   Martin Reddy
 * @version  31 October 1997
 *
 * @see      tsmSetPyramidInfoMaster, tsmAddPyramidInfoSetId,
 *           tsmAddPyramidInfoMetaData, tsmSetPyramidInfoName,
 *           tsmSetPyramidInfoPyramid, PyramidInfo
 *
 */

int
tsmAddPyramidInfoServer( PyramidInfo *info, char *server, int server_group )
{
  if ( tsmGrowPyramidInfoServerArray( info, server_group+1 ) == FALSE )
    return FALSE;

  return tsmAddServerGroupServer( &info->serverGroup[server_group], server );
}

/*
 * Synopsis:
 *   int tsmAddPyramidInfoMetaData( PyramidInfo *info, char *meta,
 *                                  int server_group )
 *
 * Doc++ Description: *//** 
 *   Adds a metadata location to a server group in a PyramidInfo.
 *
 *    The PyramidInfo structure contains all of the information required
 *    to describe a single instance of a pyramid, be it on local disk or
 *    on a DPSS. In particular, a single pyramid can have a number of
 *    different server configurations.
 *
 *    This function lets you add a metadata location (URL) to the list of
 *    metadata locations for any server group of a Pyramid. Metadata means
 *    tspec data in terms of the TerraVision application. In each case,
 *    the metadata array of the specific server group will be grown by one
 *    for you automatically and the new data added.
 *
 *    You specify which server group configuration of the pyramid you want
 *    to update by giving the server_group number. Internally, the PyramidInfo
 *    structure holds an array of ServerGroup structures - this function
 *    will automatically grow this server array if you specify a server_group
 *    number larger than the number of ServerGroups currently allocated, e.g.
 *    if you have not previously added any information to the ServerGroup
 *    array (i.e. the array is size 0), and you specify that you want to 
 *    update the contents of server_group 2, then this function will add two
 *    new ServerGroup entries to the array, clear the contents of both to 0,
 *    then add the specified information to the second of these.
 *
 * @param    info   the PyramidInfo structure describing the pyramid
 * @param    meta   the location of the metadata to be added to the server
 *                  group specifed by server_group
 * @param    server_group the server group number to update (0,1,2,...)
 *
 * @return   TRUE if all went well, FALSE if something spooky happened.
 *
 * @author   Martin Reddy
 * @version  31 October 1997
 *
 * @see      tsmSetPyramidInfoMaster, tsmAddPyramidInfoServer,
 *           tsmAddPyramidInfoSetId, tsmSetPyramidInfoName,
 *           tsmSetPyramidInfoPyramid, PyramidInfo
 *
 */

int
tsmAddPyramidInfoMetaData( PyramidInfo *info, char *meta, int server_group )
{
  if ( tsmGrowPyramidInfoServerArray( info, server_group+1 ) == FALSE )
    return FALSE;

  return tsmAddServerGroupMetaData( &info->serverGroup[server_group], meta );
}

/*
 * Synopsis:
 *      PyramidInfo *tsmMakePyramidInfo( void )
 *
 * Doc++ Description: *//** 
 *      Allocate a PyramidInfo structure.
 *      
 *      Allocate and initialise memory for a PyramidInfo structure (all
 *      fields set to 0/NULL). The PyramidInfo structure holds all of
 *      the information required to describe a particular instantiation
 *      of a Pyramid, e.g. the tspec data, the name, and the 
 *      server group details.
 *
 * @param    None
 *
 * @return   Pointer to allocated PyramidInfo structure, or NULL if failed.
 *
 * @author   Martin Reddy
 * @version  3 November 1997
 *
 * @see      tsmFreePyramidInfo, tsmSetPyramidInfoMaster,
 *           tsmAddPyramidInfoServer, tsmAddPyramidInfoSetId,
 *           tsmSetPyramidInfoName, tsmSetPyramidInfoPyramid, PyramidInfo
 *
 */

PyramidInfo *
tsmMakePyramidInfo( void )
{
  const char  *funcName = "tsmMakePyramidInfo";
  PyramidInfo *info;
  
  info = (PyramidInfo *) tsmMemManagerCalloc (1, sizeof(PyramidInfo) );
  
  if ( info == NULL ) {
    tsmError( funcName, "Error allocating PyramidInfo." );
    return NULL;
  }
  
  return info;
}

/*
 * Synopsis:
 *      ServerGroup * tsmFreeServerGroupContents( ServerGroup *group )
 *
 * Description:
 *      Free the contents of a ServerGroup.
 *
 *      This function takes a pointer to a ServerGroup and frees all of
 *      the dynamic memory that has been allocated by that ServerGroup,
 *      but does not free the memory for the ServerGroup structure itself,
 *      i.e. only the hostnames, and metadata Purls are freed.
 *
 * @param    group the ServerGroup to be freed
 *
 * @return   NULL
 *
 * @author   Martin Reddy
 * @version  3 November 1997
 *
 * @see      ServerGroup
 *
 */

ServerGroup *
tsmFreeServerGroupContents( ServerGroup *group )
{
  int i;

  if ( group != NULL ) {
    if ( group->masterHostName != NULL )
      tsmMemManagerFree( group->masterHostName );

    if ( group->serverHostNames != NULL ) {
      for ( i = 0; i < group->numServers; ++i )
	tsmMemManagerFree( group->serverHostNames[i] );

      tsmMemManagerFree( group->serverHostNames );
    }

    if ( group->metaData != NULL ) {
      for ( i = 0; i < group->numMetaData; ++i )
	tsmFreePurl( group->metaData[i] );

      tsmMemManagerFree( group->metaData );
    }
  }

  return NULL;
}

/*
 * Synopsis:
 *      PyramidInfo *tsmFreePyramidInfo( PyramidInfo *info )
 *
 * Doc++ Description: *//** 
 *   Free a PyramidInfo structure.
 *      
 *   Frees up all memory associated with a PyramidInfo structure, this
 *   includes the memory for the structure itself, the name string, and
 *   all of the memory associated with any allocated ServerGroup structures
 *   (e.g. the hostnames and metadata locations). N.B. The Pyramid tspec
 *   memory is NOT deallocated here - this must be freed separately.
 *
 * @param    info the PyramidInfo structure to free
 *
 * @return   NULL
 *
 * @author   Martin Reddy
 * @version  3 November 1997
 *
 * @see      tsmMakePyramidInfo, PyramidInfo
 *
 */

PyramidInfo *
tsmFreePyramidInfo( PyramidInfo *info )
{
  int i;

  if ( info != NULL ) {

    /* free all of the ServerGroup memory */

    if ( info->serverGroup != NULL ) {
      for ( i = 0; i < info->numServerGroups; ++i ) 
	tsmFreeServerGroupContents( &info->serverGroup[i] );

      tsmMemManagerFree( info->serverGroup );
    }

    /* free the name string */

    if ( info->name != NULL )
      tsmMemManagerFree( info->name );

    /* free the ServerGroup structure itself */
    
    tsmMemManagerFree( info );
  }

  return NULL;
}

/*
 * Synopsis:
 *    void tsmPrintServerGroup (FILE *stream, ServerGroup *serverGroup,
 *                              char *leadString)
 *
 * Doc++ Description: *//** 
 *    Display a ServerGroup structure.
 *
 *   This function can be used to output a textual description of the
 *   contents of a ServerGroup structure. This can be performed to the
 *   screen (stream = stdout or stderr), or to a file. You may also
 *   supply a leading string to display beforehand.
 *
 *   The ServerGroup structure is contained within the PyramidInfo
 *   structure (see tsmDecs.h), which is filled by the
 *   tsmGetPyramidInfo function call. ServerGroup holds the (master) 
 *   host name and server names for a single data server group for a
 *   given Pyramid.  For the DPSS, a server group is the DPSS master
 *   and all of the servers containing the data for the pyramid.
 *   Since there is no notion of master and servers for other protocols,
 *   other protocols will have numServers equal to 1, and the master
 *   and server host names will be the same.
 *
 * @param    stream the stream to write to (e.g. stdout)
 * @param    serverGroup the server group structure to output
 * @param    leadString a string to output before the server group text
 *
 * @return   Nothing (void)
 *
 * @author   
 * @version  
 *
 * @see      tsmGetPyramidInfo, tsmPrintPyramidInfo, ServerGroup
 *
 */

void 
tsmPrintServerGroup (FILE *stream, ServerGroup *serverGroup, char *leadString)
{
  /*static const char funcName[] = "tsmPrintServerGroup()";*/
  int i;

  fprintf (stream, "%sSet Id %d\n%sMaster: %s\n", 
	   leadString, serverGroup->setId,
	   leadString, serverGroup->masterHostName);
  for (i = 0; i < serverGroup->numServers; i++) {
    fprintf (stream, "%sServer %d: %s\n", leadString, i, serverGroup->serverHostNames[i]);
  }
}

/*
 * Synopsis:
 *    void tsmPrintPyramidInfo (FILE *stream, PyramidInfo *pyramidInfo)
 *
 * Doc++ Description: *//** 
 *    Display a PyramidInfo structure.
 *
 *   This function can be used to output a textual description of the
 *   contents of a PyramidInfo structure. This can be performed to the
 *   screen (stream = stdout or stderr), or to a file.
 *
 *   On the DPSS, a given pyramid can have its data distributed across
 *   many groups of master/servers.  The PyramidInfo structure holds all of
 *   the server groups for a given pyramid.  For protocols other than
 *   the DPSS, numServerGroups will be 1. (see tsmDecs.h for the definition
 *   of PyramidInfo.)
 *
 * @param    stream the stream to write to (e.g. stdout)
 * @param    pyramidInfo the PyramidInfo structure to output
 *
 * @return   Nothing (void)
 *
 * @author   
 * @version  
 *
 * @see      tsmGetPyramidInfo, tsmPrintServerGroup, PyramidInfo
 *
 */

void
tsmPrintPyramidInfo (FILE *stream, PyramidInfo *pyramidInfo)
{
  /*static const char funcName[] = "tsmPrintPyramidInfo()";*/
  ServerGroup       *group;
  int i, j;

  for (i = 0; i < pyramidInfo->numServerGroups; i++) {
    group = &pyramidInfo->serverGroup[i];
    fprintf (stream, "Pyramid %s (set id %d):\n", pyramidInfo->name,
	     group->setId);
    fprintf (stream, "  Master: %s\n", group->masterHostName);
    for (j = 0; j < group->numServers; j++)
      fprintf (stream, "  Server %d: %s\n", j, group->serverHostNames[j]);
    for (j = 0; j < group->numMetaData; j++)
      fprintf (stream, "  Metadata %d: %s\n", j, group->metaData[j]->url);
  }
}

/*
 * Synopsis:
 *    void tsmPrintPyramidInfos (FILE *stream, PyramidInfos *pyramidInfos) 
 *
 * Doc++ Description: *//** 
 *    Display the server group info for multiple pyramids.
 *
 *   This function takes a PyramidInfos structure (an array of PyramidInfo's) 
 *   and simply calls the tsmPrintPyramindInfo function for each of these.
 *   That is, it writes the server group information for all of the 
 *   pyramids lists in a PyramidInfos array to a specified stream.
 *
 * @param    stream the stream to write to (e.g. stdout)
 * @param    pyramidInfos a PyramidInfos structure to output
 *
 * @return   Nothing (void)
 *
 * @author   
 * @version  
 *
 * @see      tsmPrintPyramidInfo, tsmGetPyramidInfos, PyramidInfos
 *
 */

void
tsmPrintPyramidInfos (FILE *stream, PyramidInfos *pyramidInfos) 
{
  int i;

  for (i = 0; i < pyramidInfos->len; i++) {
    tsmPrintPyramidInfo (stream, pyramidInfos->val[i]);
  }
}

/*
 * Synopsis:
 *    PyramidInfo *tsmGetPyramidInfo (TsmHandle *tsmh, const char *pyramidName)
 *
 * Doc++ Description: *//** 
 *    Retrieve the server group info for a single pyramid.
 * 
 * On the DPSS, a given pyramid can have its data distributed across
 * many groups of master/servers. The PyramidInfo structure holds all of
 * the server group information for a given pyramid. (N.B. For protocols
 * other than the DPSS, the number of server groups will be 1.)
 *
 * This function returns all of the server information for a particular
 * pyramid. See tsmDecs.h for the definition of the PyramidInfo structure
 * that is returned.
 *
 * @param    tsmh the TSM session handle
 * @param    pyramidName then URL of the pyramid
 *
 * @return   the pyramid information, or NULL if failed
 *
 * @author   
 * @version  
 *
 * @see      tsmGetPyramidInfos, tsmGetAllPyramidInfos, tsmPrintPyramidInfo,
 *           PyramidInfo
 *
 */

PyramidInfo *
tsmGetPyramidInfo (TsmHandle *tsmh, const char *pyramidName)
{
  static const char *funcName = "tsmGetPyramidInfo()";
  PyramidInfo *pyramidInfo;
  Purl *basePurl;
  int j, k;

  if (!pyramidName) pyramidName = "";
  TSMTRINC ((TSMMASK|2, 1));
  TSMTRACE ((TSMMASK|2, funcName, "tsmh = %p, pyramidName = %s", tsmh, pyramidName));

  basePurl = tsmGetPyramidBasePurl (tsmh, pyramidName);
  if (!basePurl) {
    TSMTRACE ((TSMMASK|4, funcName,
	       "Couldn't get basePurl for name '%s'.", pyramidName));
    TSMTRINC ((TSMMASK|2, -1));
    return (NULL);
  }

  switch (basePurl->protocol) {
  case tsmDSM:
  case tsmDPSS:
    pyramidInfo = tsmGetPyramidInfo_dsm (tsmh, pyramidName, basePurl);
    break;
  case tsmLocalFile:
    pyramidInfo = tsmGetPyramidInfo_local (tsmh, pyramidName, basePurl);
    break;
  case tsmHTTP:
    pyramidInfo = tsmGetPyramidInfo_web (tsmh, pyramidName, basePurl);
    break;
  default:
    tsmNoProtocolYet (basePurl->protocol, funcName);
    pyramidInfo = NULL;
  }

  if (pyramidInfo && TSMTRACEP(TSMMASK|2)) {
    TSMTRACE ((TSMMASK|2, funcName, "Set Name = %s",
	       pyramidInfo->name));
    TSMTRACE ((TSMMASK|2, funcName, "Master Name = %s",
	       pyramidInfo->serverGroup->masterHostName));
    TSMTRACE ((TSMMASK|2, funcName, "Number of server groups = %d", 
	       pyramidInfo->numServerGroups));
    for(j = 0; j < pyramidInfo->numServerGroups; j++) {
      TSMTRACE ((TSMMASK|2, funcName, "Server Group %d:", j));
      for (k = 0; k < pyramidInfo->serverGroup[j].numServers; k++) {
	TSMTRACE ((TSMMASK|2, funcName, " - Server %d Name = '%s'",
		   k, pyramidInfo->serverGroup[j].serverHostNames[k]));
      }
    }
  }

  TSMTRACE ((TSMMASK|2, funcName, "pyramidInfo = %p", pyramidInfo));
  TSMTRINC ((TSMMASK|2, -1));
  return (pyramidInfo);
}

/*
 * Synopsis:
 *   PyramidInfos *tsmGetPyramidInfos (TsmHandle *tsmh, int numPyramidNames,
 *                                     char const **pyramidNames)
 *
 * Doc++ Description: *//** 
 *    Retrieve the server group info for mulitple pyramids.
 * 
 * On the DPSS, a given pyramid can have its data distributed across
 * many groups of master/servers. The PyramidInfo structure holds all of
 * the server group information for a given pyramid. (N.B. For protocols
 * other than the DPSS, the number of server groups will be 1.)
 *
 * This function returns all of the server information for a number of
 * different pyramids. You pass in an array of pyramid URLs, and this
 * function will effectively call tsmGetPyramidInfo for each of those
 * URLS. The results are returned as a PyramidInfos structure which is
 * just an array of PyramidInfo structures (see tsmDecs.h).
 *
 * @param    tsmh the TSM session handle
 * @param    numPyramidNames the number of pyramid URLs being passed
 * @param    pyramidNames the array of pyramid numPyramidNames URLs
 *
 * @return   the pyramids' information, or NULL if failed
 *
 * @author   
 * @version  
 *
 * @see      tsmGetPyramidInfo, tsmGetAllPyramidInfos, tsmPrintPyramidInfos,
 *           PyramidInfos
 */

PyramidInfos *
tsmGetPyramidInfos (TsmHandle *tsmh, int numPyramidNames,
		    char const **pyramidNames)
{
  static const char *funcName = "tsmGetPyramidInfos()";
  PyramidInfos *pyramidInfos;
  int i;
  
  TSMTRINC ((TSMMASK|2, 1));
  TSMTRACE ((TSMMASK|2, funcName, 
	     "tsmh = %p, numPyramidNames = %d, pyramidNames = %p",
	     tsmh, numPyramidNames, pyramidNames));

  if (numPyramidNames <= 0) {
    TSMTRINC ((TSMMASK|2, -1));
    TSMTRACE ((TSMMASK|2, funcName, "pyramidInfos = NULL."));
    return (NULL);
  }

  pyramidInfos = (PyramidInfos *)
    tsmMemManagerCalloc (1, sizeof (PyramidInfos));
  if(!pyramidInfos)
    {
      tsmError (funcName, "Unable to allocate pyramidInfos");
      TSMTRINC ((TSMMASK|2, -1));
      return (NULL);
    }

  pyramidInfos->len = numPyramidNames;
  pyramidInfos->val = (PyramidInfo **)
    tsmMemManagerCalloc (numPyramidNames, sizeof (PyramidInfo *));

  if(!pyramidInfos->val) {
    tsmError (funcName, "Unable to allocate pyramidInfos->val.");
    TSMTRINC ((TSMMASK|2, -1));
    return (NULL);
  }

  if (tsmReLoadPyramidDatabase (tsmh, "") == FALSE) {
    tsmError (funcName, "Unable to load PyramidDatabase.");
    TSMTRINC ((TSMMASK|2, -1));
    return (NULL);
  }

  for (i = 0; i < numPyramidNames; i++) {
    pyramidInfos->val[i] = tsmGetPyramidInfo (tsmh, pyramidNames[i]);
    if (!pyramidInfos->val[i]) {
      TSMTRACE ((TSMMASK|3, funcName,
		 "WARNING: Couldn't get pyramidInfo for '%s'.",
		 pyramidNames[i]));
    }
  }

  TSMTRACE ((TSMMASK|2, funcName, "pyramidInfos = %p", pyramidInfos));
  TSMTRINC ((TSMMASK|2, -1));
  return (pyramidInfos);
}

/*
 * Synopsis:
 *      PyramidInfos *tsmGetAllPyramidInfos (TsmHandle *tsmh)
 *
 * Doc++ Description: *//** 
 *    Retrieve the server group info for all pyramids.
 *      
 *   This function is simply a wrapper function for tsmGetPyramidInfos.
 *   It can be used to quickly retrieve all of the server group information
 *   for all of the pyramids that are associated with a particular Tile
 *   Set Manager connection.
 *
 * @param    tsmh - the open TSM session handle
 *
 * @return   the pyramids' server info, or NULL if failed
 *
 * @author   
 * @version  
 *
 * @see      tsmGetPyramidInfo, tsmGetPyramidInfos, tsmPrintPyramidInfos,
 *           PyramidInfos
 *
 */

PyramidInfos *
tsmGetAllPyramidInfos (TsmHandle *tsmh)
{
  static const char *funcName = "tsmGetAllPyramidInfos()";
  PyramidInfos *pyramidInfos;

  TSMTRINC ((TSMMASK|2, 1));
  TSMTRACE ((TSMMASK|2, funcName, "tsmh = %p.", tsmh));

  pyramidInfos = tsmGetPyramidInfos (tsmh, tsmh->pyramidUrls->numItems, 
				     tsmh->pyramidUrls->names);

  if (!pyramidInfos) {
    TSMTRACE ((TSMMASK|2, funcName, "Couldn't get all pyramid infos."));
  }
  else {
    TSMTRACE ((TSMMASK|2, funcName, "pyramidInfos = %p", pyramidInfos));
  }

  TSMTRINC ((TSMMASK|2, -1));
  return (pyramidInfos);
}

