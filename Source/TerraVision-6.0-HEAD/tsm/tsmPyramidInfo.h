/*
 * Name: tsmPyramidInfo.h
 *
 * Description:
 *   Contains all of the definitions for the PyramidInfo
 *   structure. The PyramidInfo structure contains all of the DPSS server
 *   information for each server group associated with a single set.
 *
 * Structure List:
 *   ServerGroup
 *   PyramidInfo
 *   PyramidInfos
 *
 * Dependencies:
 *
 * Revision History:
 *   February 1998 - Initial version by Martin Reddy
 *
 *   $Id: tsmPyramidInfo.h,v 1.5 2000/08/28 22:16:00 reddy Exp $
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

#ifndef TSMPYRAMIDINFO_H
#define TSMPYRAMIDINFO_H

#include "tsmPurl.h"
#include "TsTileSpecDecs.h"

/*
 * Doc++ Description: *//** 
 *   Holds a host/server names for a data server group.
 *
 *  PyramidServerGroup holds the host name and server names for a single
 *  data server group for a given Pyramid as well as the list of possible
 *  locations for the metadata (i.e. tspecs). For the DPSS, a 
 *  server group is the DPSS master and all of the servers containing
 *  the data for the pyramid.  Since there is no notion of master and
 *  servers For other protocols, other protocols will have numServers
 *  equal to 1, and the master and server host names will be the same.
 *
 * @param setId (int) = the set ID for the server group
 * @param numServers (int) = the number of servers in serverHostNames array
 * @param masterHostName (char *) = string containing the master host name
 * @param serverHostNames (char *) = array of strings with server names
 * @param numMetaData (int) = the number of entries in metaData array
 * @param metaData (Purl *) = array of parsed URLs for all metadata (tspecs).
 *
 * @see PyramidInfo
 *
 */

struct ServerGroup {
  int       setId;
  int       numServers;
  char      *masterHostName;
  char      **serverHostNames;
  int       numMetaData;
  Purl      **metaData;
  tspSymbol imageFormat;
};

typedef struct ServerGroup ServerGroup;

/*
 * Doc++ Description: *//** 
 *   Holds all server group info for a pyramid.
 *
 *  On the DPSS, a given pyramid can have its data distributed across
 *  many groups of master/servers.  PyramidInfo holds all of the server
 *  groups for a given pyramid. For protocols other than the DPSS,
 *  numServerGroups will be 1.
 *
 * @param name (char *) = the name of the pyramid
 * @param pyramid (Pyramid *) = the tspec structure for the pyramid
 * @param numServerGroups (int) = the number of entries in serverGroup array
 * @param serverGroup (ServerGroup *) = array of server groups
 *
 * @see tsmSetPyramidInfoMaster, tsmAddPyramidInfoServer,
 *      tsmAddPyramidInfoMetaData, tsmSetPyramidInfoName,
 *      tsmSetPyramidInfoPyramid, tsmSetPyramidInfoSetId, 
 *      ServerGroup, PyramidInfos, Pyramid
 *
 */

struct PyramidInfo {
  char        *name;
  Pyramid     *pyramid;
  int         numServerGroups;
  ServerGroup *serverGroup;
};

typedef struct PyramidInfo PyramidInfo;

/*
 * Doc++ Description: *//** 
 *   Holds an array of PyramidInfo structures.
 *
 *   The PyramidInfos structure can be used to contain an arbitrary number
 *   of pointers to PyramidInfo structures.
 *
 * @param len (int) = number of *PyramidInfo pointers in val array
 * @param val (Pyramid **) = array of *PyramidInfo pointers
 *
 * @see PyramidInfo
 *
 */

struct PyramidInfos {
  int         len;
  PyramidInfo **val;
};

typedef struct PyramidInfos PyramidInfos;

#endif
