/* File    : tcp.c
   Authors : Adam Cheyer, Didier Guzzoni
   Purpose : Contains C version of TCP library for the Open Agent Architecture
	     Works for both Microsoft Windows (1st half) and UNIX (2nd half)
   Updated : 7/23/97
 
   Copyright (C) 1997 SRI International.  All rights reserved.
 
   "Open Agent Architecture" and "OAA" are trademarks of SRI International.
*/


#ifndef _TCP_H_INCLUDED
#define _TCP_H_INCLUDED

#include <prio.h>

/****************************************************************************
 * Definitions for NSPR
 ****************************************************************************/

#define INVALID_OAASOCKET NULL
typedef PRFileDesc *OAASOCKET;

extern OAASOCKET tcpConnect(char *hostName, int port);
extern int tcpSelect(OAASOCKET socket, double timeout);
extern void tcpShutdown(OAASOCKET *socket);

/* ---------------------------------------------------------------------------*/

#endif
