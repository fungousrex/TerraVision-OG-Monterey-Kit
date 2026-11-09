
#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <stdio.h>
#include <prio.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* if part of DPSS lib, define this, otherwise comment it out */
/*#define DPSS */


#define NL_FILE 0
#define NL_SYSLOG 1
#define NL_HOST 2

#ifndef  DPSS_NETLOGGER_PORT
#define DPSS_NETLOGGER_PORT 14830
#endif

typedef struct _loginfo {
   int method;
   FILE *log_fp;
   PRFileDesc *log_socket;
   char *pname;
   char *hname;
   char *level;
} NLhandle;

/* NetLogger Routines */
NLhandle *NetLoggerOpen(int method, char *pname, char *filename, char *hostname, int port);
int NetLoggerWriteString(NLhandle * lp, char *keyword, char *data);
int NetLoggerWrite(NLhandle *lp, char *keyword, char *fmt,...);
int NetLoggerClose(NLhandle *lp);

/* special write calls */

/* user passes in timing info */
int NetLoggerGTWrite(NLhandle *lp, long sec, long usec, char *keyword, char *fmt,...);
int NetLoggerGTWriteString(NLhandle * lp, long sec, long usec, char *keyword, char *data);

/* set default level */
int NetLoggerSetLevel(NLhandle *lp, char *level);

/* internal NetLogger routines */
FILE  *LogOpen(char *logfile_name);
int   SysLogOpen(char *prog_name, int syslog_facility);
PRFileDesc *NetLogOpen(char *hostname, int port);

int   LogClose(FILE * fp);
int   SysLogClose(void);
int   NetLogClose(PRFileDesc *socket);
 
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _LOGGING_H_ */


