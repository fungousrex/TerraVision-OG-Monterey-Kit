/* File    : tcp.c
   Authors : Adam Cheyer, Didier Guzzoni
   Purpose : Contains C version of TCP library for the Open Agent Architecture
	     Works for both Microsoft Windows (1st half) and UNIX (2nd half)
   Updated : 7/23/97
 
   Copyright (C) 1997 SRI International.  All rights reserved.
 
   "Open Agent Architecture" and "OAA" are trademarks of SRI International.
*/


/****************************************************************************
 * Definitions for NSPR
 ****************************************************************************/


/****************************************************************************
 * Include files
 ****************************************************************************/
#include <prtypes.h>
#include <prio.h>
#include <prnetdb.h>

#include <stdio.h>

#include "tcp.h"

/****************************************************************************
 * name:    tcpShutdown
 * purpose: Shuts down an open socket
 * inputs:
 *   - int   socket     : Socket to close
 ****************************************************************************/
void tcpShutdown(OAASOCKET *socket) {
   PR_Shutdown (*socket, PR_SHUTDOWN_BOTH);
   PR_Close(*socket);
   *socket = NULL;
}
 
 
/****************************************************************************
 * name:    tcpConnect
 * purpose: Given a host and port, connect to server socket
 * inputs:
 *   - char* hostName   : May be a host ID or IP address
 *   - int   port       : Port to connect to on hostName
 * returns: File descriptor to socket or -1 if failure
 ****************************************************************************/
OAASOCKET tcpConnect(char *hostName, int port)
{
   OAASOCKET s;
   PRStatus ret;
   PRNetAddr saddr;
   PRHostEnt hosts;
   char buf[PR_NETDB_BUF_SIZE];

   if ((ret = PR_GetHostByName(hostName, buf, PR_NETDB_BUF_SIZE, &hosts))
       != PR_SUCCESS) {
     if ((ret = PR_StringToNetAddr (hostName, &saddr)) == PR_SUCCESS)
       ret = PR_GetHostByAddr(&saddr, buf, PR_NETDB_BUF_SIZE, &hosts);
   }
     
   if (ret != PR_SUCCESS) {
     fprintf(stderr, "TCP - Could not resolve host: %s\n", hostName);
     return INVALID_OAASOCKET;
   }
 
   s = PR_NewTCPSocket ();
 
   if (! s) {
     fprintf(stderr, "TCP - Could not create socket.\n");
     return INVALID_OAASOCKET;
   }
 
   PR_EnumerateHostEnt (0, &hosts, port, &saddr);
 
   if (PR_Connect(s, &saddr, PR_INTERVAL_NO_TIMEOUT) != PR_SUCCESS) {
     fprintf(stderr, "TCP - Could not connect to %s #%d\n", hostName,port);
     tcpShutdown(&s);
     return INVALID_OAASOCKET;
   }
 
   return s;
}
 
 
/****************************************************************************
 * name:    tcpSelect
 * purpose: Wait for socket input, or until timeout
 * inputs:
 *   - char*    socket  : socket to wait for
 *   - double   timeout : max wait time in seconds (if 0, blocks)
 * returns: Number of file descriptors read
 ****************************************************************************/
int tcpSelect(OAASOCKET socket, double timeout)
{
 
  PRIntervalTime time = PR_INTERVAL_NO_TIMEOUT;
  PRPollDesc pollDesc[1];
 
  if (timeout > 0)
    time = PR_MicrosecondsToInterval ((PRUint32) (timeout * 1e6));
 
  pollDesc[0].fd = socket;
  pollDesc[0].in_flags = PR_POLL_READ;
 
  return PR_Poll (pollDesc, 1, time);
}
