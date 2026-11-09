/*
 * Name: tsmTrace.c
 *
 * Description: 
 *   Tracing utility functions.
 *
 *   Calling tsmSetTraceLevel(0) turns off all tracing.
 *
 *   tsmSetTraceLevel(1) typically causes the high-level tsm functions
 *   to print a summary of the arguments and a summary of the results.
 *   Greater values generate greater levels of detail.  Internal
 *   functions typically require a trace level of 3 or greater to show
 *   any output.
 *
 * Function List:
 *
 * Revision History:
 *
 *   $Id: tsmTrace.c,v 1.18 2000/08/28 22:16:19 reddy Exp $
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
#include "RELEASE"

char *tsmApiReleaseString = 0;
char *tsmApiReleaseDateString = 0;

/* ================================================================
 * Functions to create the release and date strings used by
 * tsmSetTraceLevel.
 * ================================================================ */

/*
 * Synopsis:
 *   char *tsmCvsToString( char *cvsString )
 *
 * Doc++ Description: *//** 
 *   Return a printable version of a CVS string.
 *
 *   This function will return a new string with the CVS keyword and
 *   $'s removed. For example, if you pass in the string,
 *   "$Revision: 1.18 $", then this function will return a new 
 *   string "1.14". N.B. you should free this string when you are
 *   finished with it.
 *
 * @param    cvsString a CVS format string
 *
 * @return   the CVS string with the CVS keyword and dollars removed
 *
 * @author   
 * @version  
 *
 * @see      
 * */

char *
tsmCvsToString( char *cvsString )
{
  char *newString = 0;
  int cvsKeyLen = 0;

  cvsKeyLen = strcspn (cvsString, ":");
  newString = tsmMemManagerStrdup (cvsString);
  sprintf (newString, "%s", &cvsString[cvsKeyLen+2]);
  newString[strlen(cvsString)-cvsKeyLen-4] = 0;
  return (newString);
}

/* ================================================================
 * Returns the tsmApi release string with CVS stuff removed.
 * ================================================================ */

char *
tsmApiReleaseOnly (void)
{
  if (!tsmApiReleaseString) {
    tsmApiReleaseString = tsmCvsToString (tsmApiRelease);
  }
  return (tsmApiReleaseString);
}

/* ================================================================
 * Returns the tsmApi release date string with CVS stuff removed.
 * ================================================================ */

char *
tsmApiReleaseDateOnly (void)
{
  if (!tsmApiReleaseDateString) {
    tsmApiReleaseDateString = tsmCvsToString (tsmApiReleaseDate);
  }
  return (tsmApiReleaseDateString);
}

void
tsmPrintReleaseInfo (FILE *stream)
{
  fprintf (stream, 
	   "tsmApi Release %s, %s.\n",
	   tsmApiReleaseOnly(),
	   tsmApiReleaseDateOnly());

#ifdef DPSS
  {
    const char *dpssVersion = issGetInternalVersInfo();

    fprintf (stream,"DPSS Release %s.\n", dpssVersion);
    /*
    int dpssVersion = 0;
    int dpssRevision = 0;

    issGetLibVers (&dpssVersion, &dpssRevision);
    fprintf (stream,
	     "DPSS Release %d.%d.\n",
	     dpssVersion, dpssRevision);
    */
  }
#endif
}

#undef tsmError
void
tsmError (char const *prefix, char const *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  pthr_verror1 (prefix, fmt, ap);
  va_end (ap);
}

#undef tsmStatus
void
tsmStatus (char const *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  pthr_vreport (fmt, ap);
  va_end (ap);
}
