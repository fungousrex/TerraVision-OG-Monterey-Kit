/*
 * Name: tsmPyramidInfo_web.c
 *
 * Description: 
 *   Contains routines for compiling complete server information about
 *   datasets. These are the web (http) specific routines and are
 *   private to the tsmApi.
 *
 *   If USE_HTTP is not #defined, then the dummy functions at the end are
 *   defined.  These will print error messages on stderr and return the
 *   appropriate error code.
 *
 * Function List:
 *   PyramidInfo *tsmGetPyramidInfo_web( TsmHandle *, const char *, Purl * )
 *
 * Revision History:
 *   Created 12 Jan 1998 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmPyramidInfo_web.c,v 1.5.4.1 2001/03/15 20:04:20 leei Exp $
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

#ifdef USE_HTTP

PyramidInfo *
tsmGetPyramidInfo_web(TsmHandle *tsmh, const char *pyramidName, Purl *basePurl)
{
  const char *funcName = "tsmGetPyramidInfo_web";
  PyramidInfo *pyramidInfo;
  
  pyramidInfo = (PyramidInfo *) tsmMemManagerCalloc (1, sizeof (PyramidInfo));
  if(!pyramidInfo) {
    tsmError (funcName, "Unable to allocate space for pyramidInfo.");
    return NULL;
  }
  
  pyramidInfo->name = tsmMemManagerStrdup (pyramidName);
  pyramidInfo->numServerGroups = 1;
  pyramidInfo->serverGroup = (ServerGroup *)
     tsmMemManagerCalloc (1, sizeof (ServerGroup));
  if(!pyramidInfo->serverGroup) {
    tsmError (funcName, "Unable to allocate space for ServerGroup.");
    return NULL;
  }
  pyramidInfo->serverGroup->setId = tsmNameToId (pyramidName, tsmh->pyramidUrls);
  pyramidInfo->serverGroup->numServers = 1;
  pyramidInfo->serverGroup->masterHostName = basePurl->hostName;
  pyramidInfo->serverGroup->serverHostNames = (char **) 
     tsmMemManagerCalloc (1, sizeof (char *));
  if(!pyramidInfo->serverGroup->serverHostNames) {
    tsmError (funcName, "Unable to allocate space for server hostnames.");
    return NULL;
  }
  pyramidInfo->serverGroup->serverHostNames[0] = basePurl->hostName;
  
  return (pyramidInfo);
}

#else

PyramidInfo *
tsmGetPyramidInfo_web(TsmHandle *tsmh, const char *pyramidName, Purl *basePurl)
{
  tsmNotCompiledIn( basePurl->protocol, "tsmGetPyramidInfo_web" );
  return FALSE;
}

#endif
