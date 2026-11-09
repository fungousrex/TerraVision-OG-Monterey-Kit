/* ================================================================

   Function tsmIsLocalHost (see below for definition).

   tsmIsLocalHost uses functions:

   char *getrealhost(char *);
   char *getfulllocalhostname (void);

   from the Harvest 1.1beta distribution.  Below is the Copyright
   notice from Harvest file host.c.

   ================================================================ */

/*
 *  host.c - Retrieves full DNS name of the current host
 *
 *  Darren Hardy, hardy@cs.colorado.edu, April 1994
 *
 *  ----------------------------------------------------------------------
 *  Copyright (c) 1994, 1995.  All rights reserved.
 *  
 *          Mic Bowman of Transarc Corporation.
 *          Peter Danzig of the University of Southern California.
 *          Darren R. Hardy of the University of Colorado at Boulder.
 *          Udi Manber of the University of Arizona.
 *          Michael F. Schwartz of the University of Colorado at Boulder. 
 *  
 *  This copyright notice applies to all code in Harvest other than
 *  subsystems developed elsewhere, which contain other copyright notices
 *  in their source text.
 *  
 *  The Harvest software was developed by the Internet Research Task
 *  Force Research Group on Resource Discovery (IRTF-RD).  The Harvest
 *  software may be used for academic, research, government, and internal
 *  business purposes without charge.  If you wish to sell or distribute
 *  the Harvest software to commercial clients or partners, you must
 *  license the software.  See
 *  http://harvest.cs.colorado.edu/harvest/copyright,licensing.html#licensing.
 *  
 *  The Harvest software is provided ``as is'', without express or
 *  implied warranty, and with no support nor obligation to assist in its
 *  use, correction, modification or enhancement.  We assume no liability
 *  with respect to the infringement of copyrights, trade secrets, or any
 *  patents, and are not responsible for consequential damages.  Proper
 *  use of the Harvest software is entirely the responsibility of the user.
 *  
 *  For those who are using Harvest for non-commercial purposes, you may
 *  make derivative works, subject to the following constraints:
 *  
 *  - You must include the above copyright notice and these accompanying 
 *    paragraphs in all forms of derivative works, and any documentation 
 *    and other materials related to such distribution and use acknowledge 
 *    that the software was developed at the above institutions.
 *  
 *  - You must notify IRTF-RD regarding your distribution of the 
 *    derivative work.
 *  
 *  - You must clearly notify users that your are distributing a modified 
 *    version and not the original Harvest software.
 *  
 *  - Any derivative product is also subject to the restrictions of the 
 *    copyright, including distribution and use limitations.
 */

#include "tsmInternal.h"

#include <prnetdb.h>
#include <prsystem.h>

/* Functions grabbed from Harvest 1.1beta */

char *getrealhost(char *);
char *getfulllocalhostname (void);

/*
 *  getrealhost() - Returns the real fully qualified name of the given
 *  host or IP number, or NULL on error.  
 */
char *getrealhost(char *s)
{
  char *q;
  PRHostEnt host;
  int is_octet = 1, ndots = 0;
  char netbuf[PR_NETDB_BUF_SIZE];
  PRStatus status;

  if (s == NULL || *s == '\0')
    return(NULL);

  for (q = s; *q; q++) {
    if (*q == '.') {
      ndots++;
      continue;
    } else if (!isdigit(*q)) {      /* [^0-9] is a name */
      is_octet = 0;
      break;
    }
  }

  if (ndots != 3)
    is_octet = 0;

  if (is_octet) {
    PRNetAddr addr;
    status = PR_StringToNetAddr (s, &addr);
    if (status == PR_SUCCESS)
      status = PR_GetHostByAddr(&addr, netbuf, PR_NETDB_BUF_SIZE, &host);
  } else {
    status = PR_GetHostByName(s, netbuf, PR_NETDB_BUF_SIZE, &host);
  }
  return(status != PR_FAILURE ? tsmMemManagerStrdup(host.h_name) : NULL);
}


/*
 *  getfullhostname() - Returns the fully qualified name of the current
 *  host, or NULL on error.  Pointer is only valid until the next call
 *  to the gethost*() functions.
 */
char *getfulllocalhostname (void)
{
  PRHostEnt host;
  char buf[64];
  char netbuf[PR_NETDB_BUF_SIZE];

  if (PR_GetSystemInfo (PR_SI_HOSTNAME, buf, 64) == PR_FAILURE)
    return NULL;
  if (PR_GetHostByName(buf, netbuf, PR_NETDB_BUF_SIZE, &host) == PR_FAILURE)
    return NULL;
  return tsmMemStrdup (host.h_name);
}

/* ================================================================
   tsmIsLocalHost returns a value of 0, 1, or 2 depending upon the
   exactness of the match between two host names.

   It does this by comparing a full version of the input hostName
   against the full version of the local host's name (case insensitive).

   The function will return a value of 2 if the domain names of the
   two hosts are the same. A value of 1 is returned if the two
   hostnames match exactly. And a value of 0 (FALSE) is returned if
   neither of these cases occur.
   ================================================================ */

int
tsmIsLocalHost (char *hostName )
{
  const char *funcName = "tsmIsLocalHost";
  char *fullLocalHostName, *fullHostName;
  char *localDomain, *hostDomain;
  int  isLocalHost = FALSE;

  /* Get the full name for hostName. Note that this is a strdup or NULL, */
  /* so it has to be freed if its not NULL. */

  fullHostName = getrealhost (hostName);

  if (fullHostName) { /* Got a real host name. */

    /* Get the local host's full name.  Note that this name is an
       ephemeral that goes away upon calling any other "host"
       functions.  So this MUST be called AFTER getrealhost. */

    fullLocalHostName = getfulllocalhostname ();

    localDomain = strchr( fullLocalHostName, '.' );
    hostDomain  = strchr( fullHostName, '.' );

    /* If we get both the full input and local names, do a caseless
       comparison and convert to 0 (FALSE), 1, or 2. */

    TSMTRACE ((TSMMASK|3, funcName,
	       "Input host name = '%s'\n"
	       "Test input name = '%s'\n"
	       "Test local name = '%s'\n",
	       hostName, fullHostName, fullLocalHostName));

    if ( localDomain != NULL && hostDomain != NULL ) {
      if ( strcasecmp( localDomain, hostDomain ) == 0 ) {
	isLocalHost = 2;
	if ( strcasecmp( fullLocalHostName, fullHostName ) == 0 )
	  isLocalHost = 1;
      }
    } else  /* No full local host name. This should never happen!! */
      tsmError( funcName, "domain name not defined!" );

    tsmMemManagerFree (fullHostName);
  }
  else /* No full input host name, so the comparison is necessarily FALSE. */
    TSMTRACE ((TSMMASK|2, funcName, "Full input host name not defined."));

  TSMTRACE ((TSMMASK|3, funcName, "tsmIsLocalHost (%s) = %d", hostName, isLocalHost));
  return (isLocalHost);
}
