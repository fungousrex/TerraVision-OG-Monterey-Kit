
/*
 * logging.c     NetLogger routines
 *
 * creates log messages in ULM (Universal Logger Message) format, as
 *	defined by the IETF Internet-Draft
 *	(see: ftp://ds.internic.net/internet-drafts/draft-abela-ulm-02.txt)
 *
 * The ULM format is as follows:
 *   DATE=YYYYMMDDhhmmss HOST=host PROG=progname LVL=level F1= F2=......
 *
 *    where LVL = "Emergency" / "Alert" / "Error" / "Warning" / "Auth" / "Security"
 *                    / "Usage" / "System" / "Important" / "Debug";
 *
 *	NetLogger by default uses the levels "Usage" for general timing information,
 *		and "System" for system events like TCP retransmissions
 *
 *    and where F1, F2, etc are any number of user defined fields.
 *
 *  ULM recommends that field names are 4 or less characters.
 *
 * NetLogger always adds the following fields:
 *	NL.EVNT=event_name NL.SEC=seconds NL.USEC=microseconds
 *		NL.EVNT: NetLogger event name
 *		NL.SEC/USEC: time from the Unix gettimeofday() call
 *
 * Sample ULM NetLogger message:
 *   DATE=19980401103010 HOST=dpss.lbl.gov PROG=dpss_mast LVL=Usage NL.EVNT=ISS_MASTER_IN
 *		NL.SEC=889644821 NL.USEC=333739 DPSS.SES=1 DPSS.BID=15.98.144.0
 *
 *  for more info see: http://www-didc.lbl.gov/NetLogger
 *
 *    Brian Tierney, LBNL
 *
 *  Modified by Martin Reddy (SRI) as follows:
 *    - added NetLoggerWriteString and netLoggerGTWriteString. This allows me to have
 *      a wrapper vararg function that calls the NetLogger write functions.
 *    - added include for config.h to get tsmApi's DPSS definition
 */

#include "config.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include <prtime.h>
#include <prnetdb.h>
#include <prsystem.h>

#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif

#include <tsm/tsm.h>
#include "logging.h"

#define MAXLINE 255

#ifndef DPSS

/* these routines are defined in the DPSS utils lib, but are also included here
	for standalone usage.  */

char     *pname = NULL;
char     *hname = NULL;

static void my_status(char *,...);
static void err_ret(char *,...);
static void my_perror();
static char *sys_err_str();
#endif

/***********************************************************/

NLhandle *
NetLoggerOpen(int method, char *p_name, char *filename, char *hostname, int port)
{
    NLhandle *lp = (NLhandle *) malloc(sizeof(NLhandle));
    char lhostname[512];
    char netbuf[PR_NETDB_BUF_SIZE];
    PRHostEnt host; 

    /* get the local host name (probably does not include the domain) */
    if (PR_GetSystemInfo (PR_SI_HOSTNAME, lhostname, 512) == PR_FAILURE)
      return 0;

    /* Now get the canonical host name. */
    if ( ( PR_GetHostByName (lhostname, netbuf, PR_NETDB_BUF_SIZE, &host) )
	 == PR_FAILURE )
      return 0;

    switch (method)
    {
    case NL_FILE:
	lp->method = method;
	lp->pname = strdup(p_name);
	lp->hname = strdup(host.h_name);
	lp->level = strdup("Usage");
	lp->log_fp = LogOpen(filename);
	lp->log_socket = NULL;
	break;

    case NL_SYSLOG:
#ifdef HAVE_SYSLOG_H
	lp->method = method;
	lp->pname = strdup(p_name);
	lp->hname = strdup(host.h_name);
	lp->level = strdup("Usage");
	lp->log_fp = NULL;
	lp->log_socket = NULL;
	SysLogOpen(p_name, LOG_LOCAL0);
#else
	err_ret("Error: No syslog NetLogger method.");
	lp = NULL;
#endif
	break;

    case NL_HOST:
	lp->method = method;
	lp->pname = strdup(p_name);
	lp->hname = strdup(host.h_name);
	lp->level = strdup("Usage");
	lp->log_fp = NULL;
	lp->log_socket = NetLogOpen(hostname, port);
	break;

    default:
	err_ret("Error: Unknown NetLogger method.");
	lp = NULL;
	break;
    }

    return lp;
}

/***********************************************************/
int
NetLoggerGTWriteString(NLhandle * lp, long sec, long usec, char *keyword,
		       char *data)
{
    char     *func = "NetLoggerWrite";
    int       rval = 1, n, size;
    char      outbuf2[512];
    char      date[MAXLINE];
    PRExplodedTime tv;

/*RLB:  I find the usage here peculiar, where we compute local time, but
        use "secs/usecs" passed in?   I also could not find anyplace where
        this function was used.
 */
    PR_ExplodeTime(PR_Now(), PR_LocalTimeParameters, &tv);
    if (sprintf(date, "%4d%02d%02d%02d%02d%02d", tv.tm_year, tv.tm_month, 
	tv.tm_mday, tv.tm_hour, tv.tm_min, tv.tm_sec) != 14)
	perror("sprintf error on time");

    switch (lp->method)
    {
    case NL_FILE:
	fprintf(lp->log_fp, "DATE=%s HOST=%s PROG=%s LVL=%s NL.EVNT=%s NL.SEC=%lu NL.USEC=%lu %s \n",
		date, lp->hname, lp->pname, lp->level, keyword,
		sec, usec, data);
	break;

#ifdef HAVE_SYSLOG_H
    case NL_SYSLOG:
	syslog(LOG_INFO, "NL.EVNT=%s NL.SEC=%u NL.USEC=%u %s ", keyword, sec, usec,
	       data);
	break;
#endif

    case NL_HOST:

	sprintf(outbuf2, "DATE=%s HOST=%s PROG=%s LVL=%s NL.EVNT=%s NL.SEC=%lu NL.USEC=%lu %s \n",
		date, lp->hname, lp->pname, lp->level, keyword,
		sec, usec, data);

	/* write size, then write message */
	size = PR_htonl(strlen(outbuf2));
	n = PR_Write (lp->log_socket, (char *) &size, sizeof(int));
	if (n != sizeof(int))
	{
	    my_status("%s: Error sending Netlogger message to socket.", func);
	    return -1;
	}
	n = PR_Write (lp->log_socket, outbuf2, strlen(outbuf2));
	if (n != strlen(outbuf2))
	{
	    my_status("%s: Error sending Netlogger message to socket.", func);
	    return -1;
	}
	break;

    default:
	err_ret("Error: Unknown NetLogger method.");
	rval = -1;
	break;
    }

    return rval;
}

/***********************************************************/
int
NetLoggerGTWrite(NLhandle * lp, long sec, long usec, char *keyword,
		 char *fmt,...)
{
  va_list args;
  char    outbuf[256];

  outbuf[0] = '\0';
  va_start(args, fmt);
  vsprintf(outbuf, fmt, args);
  va_end(args);

  return NetLoggerGTWriteString( lp, sec, usec, keyword, outbuf );
}

/***********************************************************/
 /*
  * build and write a NetLogger message.
  * 
  * For example: DATE=19980401103010 HOST=dpss.lbl.gov PROG=dpss_mast LVL=Usage
  * NL.EVNT=ISS_MASTER_IN NL.SEC=889644821 NL.USEC=333739 DPSS.SES=1
  * DPSS.BID=15.98.144.0
  */

int
NetLoggerWriteString(NLhandle * lp, char *keyword, char *data)
{
    char     *func = "NetLoggerWrite";
    int       rval = 1, n, size;
    char      outbuf2[512];
    char      date[MAXLINE];
    long      sec, usec;
    PRTime    now = PR_Now();
    PRExplodedTime tv;
    
    /* get system time */
    PR_ExplodeTime(now, PR_LocalTimeParameters, &tv);

    if (sprintf(date, "%4d%02d%02d%02d%02d%02d", tv.tm_year, tv.tm_month, 
	tv.tm_mday, tv.tm_hour, tv.tm_min, tv.tm_sec) != 14)
	perror("sprintf error on time");

    sec = now / 1e6;
    usec = tv.tm_usec;

    switch (lp->method)
    {
    case NL_FILE:
	fprintf(lp->log_fp, "DATE=%s HOST=%s PROG=%s LVL=%s NL.EVNT=%s NL.SEC=%lu NL.USEC=%lu %s \n",
		date, lp->hname, lp->pname, lp->level, keyword,
	        sec, usec, data);
	break;

#ifdef HAVE_SYSLOG_H
    case NL_SYSLOG:
	/*
	 * syslog already writes data, hostname, progname, etc, so dont send
	 * them
	 */
	syslog(LOG_INFO, "NL.EVNT=%s NL.SEC=%lu NL.USEC=%lu %s ", keyword,
	       sec, usec, data);
	break;
#endif

    case NL_HOST:
	sprintf(outbuf2, "DATE=%s HOST=%s PROG=%s LVL=%s NL.EVNT=%s NL.SEC=%lu NL.USEC=%lu %s \n",
		date, lp->hname, lp->pname, lp->level, keyword,
		sec, usec, data);

	/* write size, then write message */
	size = PR_htonl(strlen(outbuf2));
	n = PR_Write (lp->log_socket, (char *) &size, sizeof(int));
	if (n != sizeof(int))
	{
	    my_status("%s: Error sending Netlogger message to socket.", func);
	    return -1;
	}
	n = PR_Write (lp->log_socket, outbuf2, strlen(outbuf2));
	if (n != strlen(outbuf2))
	{
	    my_status("%s: Error sending Netlogger message to socket.", func);
	    return -1;
	}
	break;

    default:
	err_ret("Error: Unknown NetLogger method.");
	rval = -1;
	break;
    }
    return rval;
}

int
NetLoggerWrite(NLhandle * lp, char *keyword, char *fmt,...)
{
  va_list args;
  char    outbuf[256];

  outbuf[0] = '\0';
  va_start(args, fmt);
  vsprintf(outbuf, fmt, args);
  va_end(args);
  
  return NetLoggerWriteString( lp, keyword, outbuf );
}

/***********************************************************/
int
NetLoggerSetLevel(NLhandle * lp, char *level)
{
    lp->level = strdup(level);
    return 1;
}

/***********************************************************/
int
NetLoggerClose(NLhandle * lp)
{
    int       rval = 1;

    switch (lp->method)
    {
    case NL_FILE:
	LogClose(lp->log_fp);
	break;

#ifdef HAVE_SYSLOG_H
    case NL_SYSLOG:
	SysLogClose();
	break;
#endif

    case NL_HOST:
	NetLogClose(lp->log_socket);
	break;

    default:
	err_ret("Error: Unknown NetLogger method.");
	rval = -1;
	break;
    }

    return rval;
}

/***********************************************************/
FILE     *
LogOpen(char *logfile_name)
{
    FILE     *fp;

    if ((fp = fopen(logfile_name, "w")) == NULL)
    {
	fprintf(stderr, "Error openning log file %s \n", logfile_name);
	return NULL;
    }
    return fp;
}

/***********************************************************/
#ifdef HAVE_SYSLOG_H
int
SysLogOpen(char *prog_name, int syslog_facility)
{
    /* syslog_facility can be LOG_USER, LOG_LOCAL0, etc. See 'man syslog' */

    openlog(prog_name, LOG_PID, syslog_facility);

    /* this call has no error return vals */
    return 1;
}
#endif

/***********************************************************/
PRFileDesc *
NetLogOpen(char *hostname, int port)
{
    PRNetAddr sin;
    PRFileDesc *log_socket;
    char netbuf[PR_NETDB_BUF_SIZE];
    PRHostEnt host; 

    my_status("opening connect to netlog message server host %s, port %d",
	      hostname, port);
    if (hostname == NULL)
    {
	my_status("Error: hostname not set!");
	return NULL;
    }
    /* Now get the canonical host name. */
    if ( ( PR_GetHostByName (hostname, netbuf, PR_NETDB_BUF_SIZE, &host) )
	 == PR_FAILURE ) {
      perror("PR_GetHostByName");
      return NULL;
    }

    sin = *(PRNetAddr *) host.h_addr_list[0];
    if (sin.inet.family != PR_AF_INET)
      return NULL;
    sin.inet.port = PR_htons (port);

    if ((log_socket = PR_NewTCPSocket ()) == NULL)
    {
	perror("PR_NewTCPSocket");
	return NULL;
    }
    if (PR_Connect(log_socket, &sin, PR_INTERVAL_NO_TIMEOUT) == PR_FAILURE)
    {
	perror("PR_Connect");
	PR_Close (log_socket);
	log_socket = NULL;
	return NULL;
    }
    return log_socket;
}

/***********************************************************/
int
LogClose(FILE * fp)
{
    fclose(fp);
    return 1;
}

/***********************************************************/
#ifdef HAVE_SYSLOG_H
int
SysLogClose(void)
{
    closelog();
    return 1;
}
#endif

/***********************************************************/
int
NetLogClose(PRFileDesc *log_socket)
{
    PR_Close (log_socket);
    return 1;
}

/*************************************************************/

/********* routines from utils.c for standalone version of
   the NetLogger Library  */

#ifndef DPSS

/***********************************************************/
static void
my_status(char *fmt,...)	/* use this one with X-windows programs to
				 * avoid name conflict */
{
    va_list   args;



    if (pname != NULL && hname != NULL)
	fprintf(stderr, "%s@%s: ", pname, hname);
    else if (pname != NULL)
	fprintf(stderr, "%s: ", pname);

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fputc('\n', stderr);
    fflush(stderr);

    return;

}

/***********************************************************/

/* err_ret(str, arg1, arg2...): print message and return */
static void
err_ret(char *fmt,...)
{
    va_list   args;

    if (pname != NULL && hname != NULL)
	fprintf(stderr, "%s@%s: ", pname, hname);
    else if (pname != NULL)
	fprintf(stderr, "%s: ", pname);


    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    my_perror();
    fflush(stdout);
    fflush(stderr);

    return;
}

/*************************************************************/
static void
my_perror()
{
#if !defined(__FreeBSD__)
    char     *sys_err_str();
#endif

    fprintf(stderr, " %s\n", sys_err_str());
}

/**************************************************************/
#ifdef HAVE_ERRNO_H
#include <errno.h>
#else
extern int errno;
extern int sys_nerr;
extern char const *sys_errlist[];
#endif

/**************************************************************/
static char *
sys_err_str()
{
    static char msgstr[200];
#if !(defined(MSVC) && defined(WIN32))

    if (errno != 0)
    {
      /* looks like sys_nerr and sys_errlist not defined under Solaris 2.7 */
      /*
      if (errno > 0 && errno < sys_nerr)
	sprintf(msgstr, "(%s)", sys_errlist[errno]);
      else
	sprintf(msgstr, "(errno = %d)", errno);
      */
      sprintf(msgstr, "(errno = %d)", errno);
    } else {
      msgstr[0] = '\0';
    }
#endif

    return (msgstr);
}

#endif
