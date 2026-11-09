/*
 * Name: tsmDatabase_dpss.c
 *
 * Description:
 *   Contains routines for managing the dataset database file from
 *   data located on a remote http server.
 *
 *   N.B. the format used for the dataset database file is described at:
 *   http://www.ai.sri.com/~magic/tsmApi/dataset-db.html
 *
 * Function List:
 *    NameUrlIds *tsmReadServer_dpss( Purl const *purl )
 *
 * Dependencies:
 *
 * Revision History:
 *    November 1999 - Initial version by Martin Reddy
 *
 *    $Id: tsmDatabase_dpss.c,v 1.4 2000/09/23 01:03:27 leei Exp $
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
#include "tsmConnect_dpss.h"

#ifdef DPSS

/*
 * Synopsis:
 *   int NameUrlIds *tsmReadServer_dpss( Purl const *url )
 *
 * Description:
 *   Read all TerraVision datasets on a DPSS.
 *
 *   This function accepts the name of a DPSS server. It will then
 *   scan through that directory looking for all datasets of type
 *   Pyramid. It will then return a NameUrlIds array with all such
 *   entries. N.B. this array could be zero length if no such entries
 *   exist in the directory.
 *
 *   Currently, this is hacked together using the old tsmApi interface
 *   and the TsmHandle structure.
 *
 * Returns: A list of available datasets, or NULL if couldn't read directory.
 *
 * Author: Martin Reddy
 *
 * Date: 3 November 1999
 *
 */

NameUrlIds *
tsmReadServer_dpss( Purl const *purl )
{
  const char  *funcName = "tsmReadServer_dpss";
  TsmHandle   *tsmh;
  NameUrlIds  *nameUrlIds;

  /* open the Tile Set Manager server */

  if ( ( tsmh = tsmOpen( purl->url ) ) == NULL ) {
    tsmError( funcName, "couldn't connect to %s", purl->url );
    return NULL;
  }

  /* Load the pyramid database list */

  if ( tsmLoadPyramidDatabase( tsmh, "" ) == FALSE ) {
    tsmError( funcName, "couldn't open the pyramid database for server" );
    tsmClose( tsmh );
    return NULL;
  }

  /* grab the PyramidUrls list from the handle */

  nameUrlIds = tsmh->pyramidUrls;

  /* free all of the tsmh memory - except for the tsmh->pyramidsUrls */

  tsmh->pyramidUrls = NULL;
  
  tsmClose( tsmh );

  /* return the PyramidUrls list that we stole from tsmh */

  return nameUrlIds;
}

/*
 * Synopsis:
 *    int tsmCompareDPSSDates( char const *, PRTime )
 *
 * Doc++ Description: *//** 
 *    Compares the DPSS loaded date string and the last contacted time.
 *
 *    This function is used to work out if a DPSS set is older or newer
 *    than the date that the server was last looked at. The DPSS loaded
 *    date is given as a string in the format YYYYDDMMHHMMSS, then
 *    server contact time is given as a time_t value. We therefore 
 *    convert the time_t value into a string in the same format as the
 *    DPSS loaded date and then just do a string compare.
 *    
 *
 * @param    loaded_date a date string in the format YYYYDDMMHHMMSS
 * @param    last_contact a date value of type PRTime
 *
 * @return   < 0 if loaded date is older than the last contacted date
 *           = 0 if both dates are exactly the same
 *           > 0 if loaded date is newer than the last contacted date
 *
 * @author   Martin Reddy
 * @version  7 July 1999
 *
 * @see      tsmUpdateDatabaseFromServerCB
 *
 */

int
tsmCompareDPSSDates( char const *loaded_date, PRTime last_contact )
{
  PRExplodedTime t;
  char last_date[64];

  PR_ExplodeTime( last_contact, PR_GMTParameters, &t );

  /* cftime( last_date, "%Y%d%m%H%M%S", last_contact ); */

  sprintf( last_date, "%4d%2d%2d%2d%2d%2d", (int) t.tm_year,
	   (int) t.tm_mday, (int) t.tm_month + 1, (int) t.tm_hour,
	   (int) t.tm_min, (int) t.tm_sec );
	   
  return strcmp( loaded_date, last_date );
}

#else

int
tsmCompareDPSSDates( char const *loaded_date, PRTime last_contact )
{
  /* > 0 if loaded date is newer than the last contacted date */
  return 1;
}

#endif

