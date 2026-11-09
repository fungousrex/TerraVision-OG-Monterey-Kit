/*
 * nanohttp.c: minimalist HTTP GET implementation to fetch external subsets.
 *             focuses on size, streamability, reentrancy and portability
 *
 * This is clearly not a general purpose HTTP implementation
 * If you look for one, check:
 *         http://www.w3.org/Library/
 *
 * See Copyright for the status of this software.
 *
 * Daniel.Veillard@w3.org
 */
 
/* TODO add compression support, Send the Accept- , and decompress on the
        fly with ZLIB if found at compile-time */

#ifdef WIN32
#define INCLUDE_WINSOCK
#include "win32config.h"
#else
#include "config.h"
#endif

#include <libxml/xmlversion.h>
#include <libxml/xmlerror.h>

#ifdef LIBXML_HTTP_ENABLED
#include <stdio.h>
#include <string.h>


#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_NSPR_H
# include <prerror.h>
# include <prnetdb.h>
# include <prsystem.h>

  typedef PRFileDesc *filedesc_t;
# define Read PR_Read
# define Write PR_Write
# define STDIN PR_STDIN
# define STDOUT PR_STDOUT
# define INVALID_FD(fd) (! (fd))

#else

  typedef int filedesc_t;
# define Read(x,y,z) recv(x,y,z,0)
# define Write(x,y,z) send(x,y,z,0)
# define STDIN (0)
# define STDOUT (1)
# define INVALID_FD(fd) ((fd) < 0)

# ifdef HAVE_UNISTD_H
# include <unistd.h>
# endif
# ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
# endif
# ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
# endif
# ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
# endif
# ifdef HAVE_NETDB_H
# include <netdb.h>
# endif
# ifdef HAVE_FCNTL_H
# include <fcntl.h> 
# endif
# ifdef HAVE_ERRNO_H
# include <errno.h>
# endif
# ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
# endif
# ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>
# endif
# ifdef HAVE_STRINGS_H
# include <strings.h>
# endif
# ifdef SUPPORT_IP6
# include <resolv.h>
# endif

# ifdef VMS
# include <stropts>
# define SOCKLEN_T unsigned int
# define SOCKET int
# endif
#endif

#include <libxml/xmlmemory.h>
#include <libxml/parser.h> /* for xmlStr(n)casecmp() */
#include <libxml/nanohttp.h>

/**
 * A couple portability macros
 */
#ifndef _WINSOCKAPI_
#define closesocket(s) close(s)
#define SOCKET int
#endif

#ifdef STANDALONE
#define DEBUG_HTTP
#define xmlStrncasecmp(a, b, n) strncasecmp((char *)a, (char *)b, n)
#define xmlStrcasecmpi(a, b) strcasecmp((char *)a, (char *)b)
#endif

#define XML_NANO_HTTP_MAX_REDIR	10

#define XML_NANO_HTTP_CHUNK	4096

#define XML_NANO_HTTP_CLOSED	0
#define XML_NANO_HTTP_WRITE	1
#define XML_NANO_HTTP_READ	2
#define XML_NANO_HTTP_NONE	4

typedef struct xmlNanoHTTPCtxt {
    char *protocol;	/* the protocol name */
    char *hostname;	/* the host name */
    int port;		/* the port */
    char *path;		/* the path within the URL */
    filedesc_t fd;      /* the file descriptor for the socket */
    int state;		/* WRITE / READ / CLOSED */
    char *out;		/* buffer sent (zero terminated) */
    char *outptr;	/* index within the buffer sent */
    char *in;		/* the receiving buffer */
    char *content;	/* the start of the content */
    char *inptr;	/* the next byte to read from network */
    char *inrptr;	/* the next byte to give back to the client */
    int inlen;		/* len of the input buffer */
    int last;		/* return code for last operation */
    int returnValue;	/* the protocol return value */
    char *contentType;	/* the MIME type for the input */
    char *location;	/* the new URL in case of redirect */
    int contentLength;  /* value of the Content-Length header (XXX reddy) */
    char *authHeader;	/* contents of {WWW,Proxy}-Authenticate header */
} xmlNanoHTTPCtxt, *xmlNanoHTTPCtxtPtr;

static int initialized = 0;
static char *proxy = NULL;	 /* the proxy name if any */
static int proxyPort;	/* the proxy port if any */
static unsigned int timeout = 60;/* the select() timeout in seconds */

#ifndef HAVE_NSPR_H
/**
 * A portability function
 */
int socket_errno(void) {
#ifdef _WINSOCKAPI_
    return(WSAGetLastError());
#else
    return(errno);
#endif
}
#endif

/**
 * xmlNanoHTTPInit:
 *
 * Initialize the HTTP protocol layer.
 * Currently it just checks for proxy informations
 */

void
xmlNanoHTTPInit(void) {
    const char *env;
#ifdef _WINSOCKAPI_
    WSADATA wsaData;    
#endif

    if (initialized)
	return;

#ifdef _WINSOCKAPI_
    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0)
	return;
#endif

    if (proxy == NULL) {
	proxyPort = 80;
	env = getenv("no_proxy");
	if (env != NULL)
	    goto done;
	env = getenv("http_proxy");
	if (env != NULL) {
	    xmlNanoHTTPScanProxy(env);
	    goto done;
	}
	env = getenv("HTTP_PROXY");
	if (env != NULL) {
	    xmlNanoHTTPScanProxy(env);
	    goto done;
	}
    }
done:
    initialized = 1;
}

/**
 * xmlNanoHTTPClenup:
 *
 * Cleanup the HTTP protocol layer.
 */

void
xmlNanoHTTPCleanup(void) {
    if (proxy != NULL)
	xmlFree(proxy);
#ifdef _WINSOCKAPI_
    if (initialized)
	WSACleanup();
#endif
    initialized = 0;
    return;
}

/**
 * xmlNanoHTTPTimeout:
 * @delay:  the delay in seconds
 *
 * Set the HTTP timeout, (default is 60secs).  0 means immediate
 * return, while -1 infinite.
 */

void
xmlNanoHTTPTimeout(int delay) {
    timeout = (unsigned int) delay;
}

/**
 * xmlNanoHTTPScanURL:
 * @ctxt:  an HTTP context
 * @URL:  The URL used to initialize the context
 *
 * (Re)Initialize an HTTP context by parsing the URL and finding
 * the protocol host port and path it indicates.
 */

static void
xmlNanoHTTPScanURL(xmlNanoHTTPCtxtPtr ctxt, const char *URL) {
    const char *cur = URL;
    char buf[4096];
    int index = 0;
    int port = 0;

    if (ctxt->protocol != NULL) { 
        xmlFree(ctxt->protocol);
	ctxt->protocol = NULL;
    }
    if (ctxt->hostname != NULL) { 
        xmlFree(ctxt->hostname);
	ctxt->hostname = NULL;
    }
    if (ctxt->path != NULL) { 
        xmlFree(ctxt->path);
	ctxt->path = NULL;
    }
    if (URL == NULL) return;
    buf[index] = 0;
    while (*cur != 0) {
        if ((cur[0] == ':') && (cur[1] == '/') && (cur[2] == '/')) {
	    buf[index] = 0;
	    ctxt->protocol = xmlMemStrdup(buf);
	    index = 0;
            cur += 3;
	    break;
	}
	buf[index++] = *cur++;
    }
    if (*cur == 0) return;

    buf[index] = 0;
    while (1) {
        if (cur[0] == ':') {
	    buf[index] = 0;
	    ctxt->hostname = xmlMemStrdup(buf);
	    index = 0;
	    cur += 1;
	    while ((*cur >= '0') && (*cur <= '9')) {
	        port *= 10;
		port += *cur - '0';
		cur++;
	    }
	    if (port != 0) ctxt->port = port;
	    while ((cur[0] != '/') && (*cur != 0)) 
	        cur++;
	    break;
	}
        if ((*cur == '/') || (*cur == 0)) {
	    buf[index] = 0;
	    ctxt->hostname = xmlMemStrdup(buf);
	    index = 0;
	    break;
	}
	buf[index++] = *cur++;
    }
    if (*cur == 0) 
        ctxt->path = xmlMemStrdup("/");
    else {
        index = 0;
        buf[index] = 0;
	while (*cur != 0)
	    buf[index++] = *cur++;
	buf[index] = 0;
	ctxt->path = xmlMemStrdup(buf);
    }	
}

/**
 * xmlNanoHTTPScanProxy:
 * @URL:  The proxy URL used to initialize the proxy context
 *
 * (Re)Initialize the HTTP Proxy context by parsing the URL and finding
 * the protocol host port it indicates.
 * Should be like http://myproxy/ or http://myproxy:3128/
 * A NULL URL cleans up proxy informations.
 */

void
xmlNanoHTTPScanProxy(const char *URL) {
    const char *cur = URL;
    char buf[4096];
    int index = 0;
    int port = 0;

    if (proxy != NULL) { 
        xmlFree(proxy);
	proxy = NULL;
    }
    if (proxyPort != 0) { 
	proxyPort = 0;
    }
#ifdef DEBUG_HTTP
    if (URL == NULL)
	xmlGenericError(xmlGenericErrorContext,
		"Removing HTTP proxy info\n");
    else
	xmlGenericError(xmlGenericErrorContext,
		"Using HTTP proxy %s\n", URL);
#endif
    if (URL == NULL) return;
    buf[index] = 0;
    while (*cur != 0) {
        if ((cur[0] == ':') && (cur[1] == '/') && (cur[2] == '/')) {
	    buf[index] = 0;
	    index = 0;
            cur += 3;
	    break;
	}
	buf[index++] = *cur++;
    }
    if (*cur == 0) return;

    buf[index] = 0;
    while (1) {
        if (cur[0] == ':') {
	    buf[index] = 0;
	    proxy = xmlMemStrdup(buf);
	    index = 0;
	    cur += 1;
	    while ((*cur >= '0') && (*cur <= '9')) {
	        port *= 10;
		port += *cur - '0';
		cur++;
	    }
	    if (port != 0) proxyPort = port;
	    while ((cur[0] != '/') && (*cur != 0)) 
	        cur++;
	    break;
	}
        if ((*cur == '/') || (*cur == 0)) {
	    buf[index] = 0;
	    proxy = xmlMemStrdup(buf);
	    index = 0;
	    break;
	}
	buf[index++] = *cur++;
    }
}

/**
 * xmlNanoHTTPNewCtxt:
 * @URL:  The URL used to initialize the context
 *
 * Allocate and initialize a new HTTP context.
 *
 * Returns an HTTP context or NULL in case of error.
 */

static xmlNanoHTTPCtxtPtr
xmlNanoHTTPNewCtxt(const char *URL) {
    xmlNanoHTTPCtxtPtr ret;

    ret = (xmlNanoHTTPCtxtPtr) xmlMalloc(sizeof(xmlNanoHTTPCtxt));
    if (ret == NULL) return(NULL);

    memset(ret, 0, sizeof(xmlNanoHTTPCtxt));
    ret->port = 80;
    ret->returnValue = 0;
    ret->contentLength = 0;

    xmlNanoHTTPScanURL(ret, URL);

    return(ret);
}

/**
 * xmlNanoHTTPFreeCtxt:
 * @ctxt:  an HTTP context
 *
 * Frees the context after closing the connection.
 */

static void
xmlNanoHTTPFreeCtxt(xmlNanoHTTPCtxtPtr ctxt) {
    if (ctxt == NULL) return;
    if (ctxt->hostname != NULL) xmlFree(ctxt->hostname);
    if (ctxt->protocol != NULL) xmlFree(ctxt->protocol);
    if (ctxt->path != NULL) xmlFree(ctxt->path);
    if (ctxt->out != NULL) xmlFree(ctxt->out);
    if (ctxt->in != NULL) xmlFree(ctxt->in);
    if (ctxt->contentType != NULL) xmlFree(ctxt->contentType);
    if (ctxt->location != NULL) xmlFree(ctxt->location);
    if (ctxt->authHeader != NULL) xmlFree(ctxt->authHeader);
    ctxt->state = XML_NANO_HTTP_NONE;
#ifdef HAVE_NSPR_H
    if (ctxt->fd) PR_Close (ctxt->fd);
    ctxt->fd = NULL;
#else
    if (ctxt->fd >= 0) closesocket(ctxt->fd);
    ctxt->fd = -1;
#endif
    xmlFree(ctxt);
}

/**
 * xmlNanoHTTPSend:
 * @ctxt:  an HTTP context
 *
 * Send the input needed to initiate the processing on the server side
 */

static void
xmlNanoHTTPSend(xmlNanoHTTPCtxtPtr ctxt) {
    if (ctxt->state & XML_NANO_HTTP_WRITE) {
        int total_sent = 0;
        while (total_sent <strlen(ctxt->outptr)) {
            int nsent = Write(ctxt->fd, ctxt->outptr+total_sent,
			      strlen(ctxt->outptr)-total_sent);
            if (nsent>0)
                total_sent += nsent;
	}

        ctxt->last = total_sent;
    }
}

/**
 * xmlNanoHTTPRecv:
 * @ctxt:  an HTTP context
 *
 * Read information coming from the HTTP connection.
 * This is a blocking call (but it blocks in select(), not read()).
 *
 * Returns the number of byte read or -1 in case of error.
 */

static int
xmlNanoHTTPRecv(xmlNanoHTTPCtxtPtr ctxt) {
    while (ctxt->state & XML_NANO_HTTP_READ) {
	if (ctxt->in == NULL) {
	    ctxt->in = (char *) xmlMalloc(65000 * sizeof(char));
	    if (ctxt->in == NULL) {
	        ctxt->last = -1;
		return(-1);
	    }
	    ctxt->inlen = 65000;
	    ctxt->inptr = ctxt->content = ctxt->inrptr = ctxt->in;
	}
	if (ctxt->inrptr > ctxt->in + XML_NANO_HTTP_CHUNK) {
	    int delta = ctxt->inrptr - ctxt->in;
	    int len = ctxt->inptr - ctxt->inrptr;
	    
	    memmove(ctxt->in, ctxt->inrptr, len);
	    ctxt->inrptr -= delta;
	    ctxt->content -= delta;
	    ctxt->inptr -= delta;
	}
        if ((ctxt->in + ctxt->inlen) < (ctxt->inptr + XML_NANO_HTTP_CHUNK)) {
	    int d_inptr = ctxt->inptr - ctxt->in;
	    int d_content = ctxt->content - ctxt->in;
	    int d_inrptr = ctxt->inrptr - ctxt->in;

	    ctxt->inlen *= 2;
            ctxt->in = (char *) xmlRealloc(ctxt->in, ctxt->inlen);
	    if (ctxt->in == NULL) {
	        ctxt->last = -1;
		return(-1);
	    }
            ctxt->inptr = ctxt->in + d_inptr;
            ctxt->content = ctxt->in + d_content;
            ctxt->inrptr = ctxt->in + d_inrptr;
	}
	ctxt->last = Read(ctxt->fd, ctxt->inptr, XML_NANO_HTTP_CHUNK);
	if (ctxt->last > 0) {
	    ctxt->inptr += ctxt->last;
	    return(ctxt->last);
	}
	if (ctxt->last == 0) {
	    return(0);
	}
	if (ctxt->last == -1) {
#ifdef HAVE_NSPR_H
	    switch (PR_GetError ()) {
	    case PR_IN_PROGRESS_ERROR:
	    case PR_WOULD_BLOCK_ERROR:
	      break;
	    default:
	      return(0);
	    }
#else
	    switch (socket_errno()) {
		case EINPROGRESS:
		case EWOULDBLOCK:
#if defined(EAGAIN) && EAGAIN != EWOULDBLOCK
		case EAGAIN:
#endif
		    break;
		default:
		    return(0);
	    }
#endif
	}
#ifdef HAVE_NSPR_H
	{
	  PRPollDesc pds[1];
	  PRIntervalTime timeout = PR_SecondsToInterval(10);
	  pds[0].fd = ctxt->fd;
	  pds[0].in_flags = PR_POLL_READ;
	  if (PR_Poll (pds, 1, timeout) <= 0)
	    return(0);
	}
#else
	{
	  struct timeval tv;
	  fd_set rfd;

	  tv.tv_sec=timeout;
	  tv.tv_usec=0;
	  FD_ZERO(&rfd);
	  FD_SET(ctxt->fd, &rfd);
	
	  if(select(ctxt->fd+1, &rfd, NULL, NULL, &tv)<1)
	    return(0);
	}
#endif
    }
    return(0);
}

/**
 * xmlNanoHTTPReadLine:
 * @ctxt:  an HTTP context
 *
 * Read one line in the HTTP server output, usually for extracting
 * the HTTP protocol informations from the answer header.
 *
 * Returns a newly allocated string with a copy of the line, or NULL
 *         which indicate the end of the input.
 */

static char *
xmlNanoHTTPReadLine(xmlNanoHTTPCtxtPtr ctxt) {
    char buf[4096];
    char *bp = buf;
    
    while (bp - buf < 4095) {
	if (ctxt->inrptr == ctxt->inptr) {
	    if (xmlNanoHTTPRecv(ctxt) == 0) {
		if (bp == buf)
		    return(NULL);
		else
		    *bp = 0;
		return(xmlMemStrdup(buf));
	    }
	}
	*bp = *ctxt->inrptr++;
	if (*bp == '\n') {
	    *bp = 0;
	    return(xmlMemStrdup(buf));
	}
	if (*bp != '\r')
	    bp++;
    }
    buf[4095] = 0;
    return(xmlMemStrdup(buf));
}


/**
 * xmlNanoHTTPScanAnswer:
 * @ctxt:  an HTTP context
 * @line:  an HTTP header line
 *
 * Try to extract useful informations from the server answer.
 * We currently parse and process:
 *  - The HTTP revision/ return code
 *  - The Content-Type
 *  - The Location for redirrect processing.
 *
 * Returns -1 in case of failure, the file descriptor number otherwise
 */

static void
xmlNanoHTTPScanAnswer(xmlNanoHTTPCtxtPtr ctxt, const char *line) {
    const char *cur = line;

    if (line == NULL) return;

    if (!strncmp(line, "HTTP/", 5)) {
        int version = 0;
	int ret = 0;

	cur += 5;
	while ((*cur >= '0') && (*cur <= '9')) {
	    version *= 10;
	    version += *cur - '0';
	    cur++;
	}
	if (*cur == '.') {
	    cur++;
	    if ((*cur >= '0') && (*cur <= '9')) {
		version *= 10;
		version += *cur - '0';
		cur++;
	    }
	    while ((*cur >= '0') && (*cur <= '9'))
		cur++;
	} else
	    version *= 10;
	if ((*cur != ' ') && (*cur != '\t')) return;
	while ((*cur == ' ') || (*cur == '\t')) cur++;
	if ((*cur < '0') || (*cur > '9')) return;
	while ((*cur >= '0') && (*cur <= '9')) {
	    ret *= 10;
	    ret += *cur - '0';
	    cur++;
	}
	if ((*cur != 0) && (*cur != ' ') && (*cur != '\t')) return;
	ctxt->returnValue = ret;
    } else if (!strncmp(line, "Content-Length:", 15) ||
	       !strncmp(line, "content-length:", 15)) {
        cur += 15;
	while ((*cur == ' ') || (*cur == '\t')) cur++;
	ctxt->contentLength = atoi(cur);
    } else if (!xmlStrncasecmp(BAD_CAST line, BAD_CAST"Content-Type:", 13)) {
        cur += 13;
	while ((*cur == ' ') || (*cur == '\t')) cur++;
	if (ctxt->contentType != NULL)
	    xmlFree(ctxt->contentType);
	ctxt->contentType = xmlMemStrdup(cur);
    } else if (!xmlStrncasecmp(BAD_CAST line, BAD_CAST"ContentType:", 12)) {
        cur += 12;
	if (ctxt->contentType != NULL) return;
	while ((*cur == ' ') || (*cur == '\t')) cur++;
	ctxt->contentType = xmlMemStrdup(cur);
    } else if (!strncmp(line, "content-type:", 13)) {
        cur += 13;
	if (ctxt->contentType != NULL) return;
	while ((*cur == ' ') || (*cur == '\t')) cur++;
	ctxt->contentType = xmlMemStrdup(cur);
    } else if (!strncmp(line, "contenttype:", 12)) {
        cur += 12;
	if (ctxt->contentType != NULL) return;
	while ((*cur == ' ') || (*cur == '\t')) cur++;
	ctxt->contentType = xmlMemStrdup(cur);
    } else if (!xmlStrncasecmp(BAD_CAST line, BAD_CAST"Location:", 9)) {
        cur += 9;
	while ((*cur == ' ') || (*cur == '\t')) cur++;
	if (ctxt->location != NULL)
	    xmlFree(ctxt->location);
	ctxt->location = xmlMemStrdup(cur);
    } else if (!xmlStrncasecmp(BAD_CAST line, BAD_CAST"WWW-Authenticate:", 17)) {
        cur += 17;
	while ((*cur == ' ') || (*cur == '\t')) cur++;
	if (ctxt->authHeader != NULL)
	    xmlFree(ctxt->authHeader);
	ctxt->authHeader = xmlMemStrdup(cur);
    } else if (!xmlStrncasecmp(BAD_CAST line, BAD_CAST"Proxy-Authenticate:", 19)) {
        cur += 19;
	while ((*cur == ' ') || (*cur == '\t')) cur++;
	if (ctxt->authHeader != NULL)
	    xmlFree(ctxt->authHeader);
	ctxt->authHeader = xmlMemStrdup(cur);
    }
}

/**
 * xmlNanoHTTPConnectAttempt:
 * @ia:  an internet adress structure
 * @port:  the port number
 *
 * Attempt a connection to the given IP:port endpoint. It forces
 * non-blocking semantic on the socket, and allow 60 seconds for
 * the host to answer.
 *
 * Returns -1 in case of failure, the file descriptor number otherwise
 */

#ifdef HAVE_NSPR_H
static PRFileDesc *
xmlNanoHTTPConnectAttempt(PRNetAddr *ia, int port)
{
    PRFileDesc *s = PR_NewTCPSocket ();
    PRSocketOptionData sockopt;
    PRStatus status;

    if(! s) {
#ifdef DEBUG_HTTP
	perror("socket");
#endif
	return NULL;
    }
    
    sockopt.option = PR_SockOpt_Nonblocking;
    sockopt.value.non_blocking = PR_FALSE;
    status = PR_SetSocketOption(s, &sockopt);
    
    if(status != PR_SUCCESS) {
#ifdef DEBUG_HTTP
	perror("nonblocking");
#endif
	PR_Close(s);
	return NULL;
    }


    {
      PRIntervalTime timeout = PR_SecondsToInterval(60);

      PR_InitializeNetAddr (PR_IpAddrNull, port, ia);
      if (PR_Connect (s, ia, PR_INTERVAL_NO_TIMEOUT) != PR_SUCCESS
	  && PR_GetError() != PR_IN_PROGRESS_ERROR) {
	PR_Close (s);
	return NULL;
      }	
    }

    return(s);
}
#else /* ! HAVE_NSPR_H */
static int
xmlNanoHTTPConnectAttempt(struct sockaddr *addr, int port)
{
    SOCKET s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    fd_set wfd;
    struct timeval tv;
    int status;
    
    if (s==-1) {
#ifdef DEBUG_HTTP
	perror("socket");
#endif
	return(-1);
    }
    
#ifdef _WINSOCKAPI_
    {
	u_long one = 1;

	status = ioctlsocket(s, FIONBIO, &one) == SOCKET_ERROR ? -1 : 0;
    }
#else /* _WINSOCKAPI_ */
#if defined(VMS)
    {
	int enable = 1;
	status = ioctl(s, FIONBIO, &enable);
    }
#else /* VMS */
    if ((status = fcntl(s, F_GETFL, 0)) != -1) {
#ifdef O_NONBLOCK
	status |= O_NONBLOCK;
#else /* O_NONBLOCK */
#ifdef F_NDELAY
	status |= F_NDELAY;
#endif /* F_NDELAY */
#endif /* !O_NONBLOCK */
	status = fcntl(s, F_SETFL, status);
    }
    if (status < 0) {
#ifdef DEBUG_HTTP
	perror("nonblocking");
#endif
	closesocket(s);
	return(-1);
    }
#endif /* !VMS */
#endif /* !_WINSOCKAPI_ */


    if ((connect(s, addr, sizeof(*addr))==-1)) {
	switch (socket_errno()) {
	    case EINPROGRESS:
	    case EWOULDBLOCK:
		break;
	    default:
		perror("connect");
		closesocket(s);
		return(-1);
	}
    }	
    
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    
    FD_ZERO(&wfd);
    FD_SET(s, &wfd);
    
    switch(select(s+1, NULL, &wfd, NULL, &tv))
    {
	case 0:
	    /* Time out */
	    closesocket(s);
	    return(-1);
	case -1:
	    /* Ermm.. ?? */
#ifdef DEBUG_HTTP
	    perror("select");
#endif
	    closesocket(s);
	    return(-1);
    }

    if ( FD_ISSET(s, &wfd) ) {
	SOCKLEN_T len;
	len = sizeof(status);
	if (getsockopt(s, SOL_SOCKET, SO_ERROR, (char*)&status, &len) < 0 ) {
	    /* Solaris error code */
	    return (-1);
	}
	if ( status ) {
	    closesocket(s);
	    errno = status;
	    return (-1);
	}
    } else {
	/* pbm */
	return (-1);
    }
    
    return(s);
}
#endif /* !HAVE_NSPR_H */

/**
 * xmlNanoHTTPConnectHost:
 * @host:  the host name
 * @port:  the port number
 *
 * Attempt a connection to the given host:port endpoint. It tries
 * the multiple IP provided by the DNS if available.
 *
 * Returns -1 in case of failure, the file descriptor number otherwise
 */

#ifdef HAVE_NSPR_H
static PRFileDesc *
xmlNanoHTTPConnectHost(const char *host, int port)
{
    PRHostEnt h;
    PRNetAddr addr;
    char buf[PR_NETDB_BUF_SIZE];
    int i;
    PRFileDesc *s;
    
    if (PR_GetHostByName(host, buf, PR_NETDB_BUF_SIZE, &h) != PR_SUCCESS)
    {
#ifdef DEBUG_HTTP
	fprintf(stderr,"unable to resolve '%s'.\n", host);
#endif
	return NULL;
    }
    
    for(i = PR_EnumerateHostEnt (0, &h, port, &addr);
	i != 0;
	i = PR_EnumerateHostEnt (i, &h, port, &addr))
    {
	s = xmlNanoHTTPConnectAttempt(&addr, port);
	if (s)
	    return(s);
    }

#ifdef DEBUG_HTTP
    fprintf(stderr, "unable to connect to '%s'.\n", host);
#endif
    return NULL;
}
#else
static int
xmlNanoHTTPConnectHost(const char *host, int port)
{
    struct hostent *h;
    struct sockaddr *addr;
    struct in_addr ia;
    struct sockaddr_in sin;
#ifdef SUPPORT_IP6
    struct in6_addr ia6;
    struct sockaddr_in6 sin6;
#endif
    int i;
    int s;
    
#if defined(SUPPORT_IP6) && defined(RES_USE_INET6)
    if (!(_res.options & RES_INIT))
	res_init();
    _res.options |= RES_USE_INET6;
#endif
    h=gethostbyname(host);
    if (h==NULL)
    {
#ifdef DEBUG_HTTP
	xmlGenericError(xmlGenericErrorContext,"unable to resolve '%s'.\n", host);
#endif
	return(-1);
    }
    
    for(i=0; h->h_addr_list[i]; i++)
    {
	if (h->h_addrtype == AF_INET) {
	    /* A records (IPv4) */
	    memcpy(&ia, h->h_addr_list[i], h->h_length);
	    sin.sin_family = h->h_addrtype;
	    sin.sin_addr   = ia;
	    sin.sin_port   = htons(port);
	    addr = (struct sockaddr *)&sin;
#ifdef SUPPORT_IP6
	} else if (h->h_addrtype == AF_INET6) {
	    /* AAAA records (IPv6) */
	    memcpy(&ia6, h->h_addr_list[i], h->h_length);
	    sin6.sin_family = h->h_addrtype;
	    sin6.sin_addr   = ia6;
	    sin6.sin_port   = htons(port);
	    addr = (struct sockaddr *)&sin6;
#endif
	} else
	    break; /* for */
	
	s = xmlNanoHTTPConnectAttempt(addr, port);
	if (s != -1)
	    return(s);
    }

#ifdef DEBUG_HTTP
    xmlGenericError(xmlGenericErrorContext,
	    "unable to connect to '%s'.\n", host);
#endif
    return(-1);
}
#endif


/**
 * xmlNanoHTTPOpen:
 * @URL:  The URL to load
 * @contentType:  if available the Content-Type information will be
 *                returned at that location
 *
 * This function try to open a connection to the indicated resource
 * via HTTP GET.
 *
 * Returns NULL in case of failure, otherwise a request handler.
 *     The contentType, if provided must be freed by the caller
 */

void*
xmlNanoHTTPOpen(const char *URL, char **contentType) {
    xmlNanoHTTPCtxtPtr ctxt;
    char buf[4096];
    char *p;
    int head;
    int nbRedirects = 0;
    char *redirURL = NULL;
    filedesc_t ret;
    
    xmlNanoHTTPInit();
    if (contentType != NULL) *contentType = NULL;

retry:
    if (redirURL == NULL)
	ctxt = xmlNanoHTTPNewCtxt(URL);
    else {
	ctxt = xmlNanoHTTPNewCtxt(redirURL);
	xmlFree(redirURL);
	redirURL = NULL;
    }

    if ((ctxt->protocol == NULL) || (strcmp(ctxt->protocol, "http"))) {
        xmlNanoHTTPFreeCtxt(ctxt);
	if (redirURL != NULL) xmlFree(redirURL);
        return(NULL);
    }
    if (ctxt->hostname == NULL) {
        xmlNanoHTTPFreeCtxt(ctxt);
        return(NULL);
    }
    if (proxy)
	ret = xmlNanoHTTPConnectHost(proxy, proxyPort);
    else
	ret = xmlNanoHTTPConnectHost(ctxt->hostname, ctxt->port);
    if (INVALID_FD (ret)) {
        xmlNanoHTTPFreeCtxt(ctxt);
        return(NULL);
    }
    ctxt->fd = ret;
    if (proxy) {
	if (ctxt->port != 80)
#ifdef HAVE_SNPRINTF
	    snprintf(buf, sizeof(buf),
		     "GET http://%s:%d%s HTTP/1.0\r\nHost: %s\r\n\r\n",
		 ctxt->hostname, ctxt->port, ctxt->path, ctxt->hostname);
#else
	    sprintf(buf, 
		     "GET http://%s:%d%s HTTP/1.0\r\nHost: %s\r\n\r\n",
		 ctxt->hostname, ctxt->port, ctxt->path, ctxt->hostname);
#endif
	else 
#ifdef HAVE_SNPRINTF
	    snprintf(buf, sizeof(buf),"GET http://%s%s HTTP/1.0\r\nHost: %s\r\n\r\n",
		 ctxt->hostname, ctxt->path, ctxt->hostname);
#else
	    sprintf(buf, "GET http://%s%s HTTP/1.0\r\nHost: %s\r\n\r\n",
		 ctxt->hostname, ctxt->path, ctxt->hostname);
#endif
#ifdef DEBUG_HTTP
	if (ctxt->port != 80)
	    printf("-> Proxy GET http://%s:%d%s HTTP/1.0\n-> Host: %s\n\n",
	           ctxt->hostname, ctxt->port, ctxt->path, ctxt->hostname);
	else
	    printf("-> Proxy GET http://%s%s HTTP/1.0\n-> Host: %s\n\n",
	           ctxt->hostname, ctxt->path, ctxt->hostname);
#endif
    } else {
#ifdef HAVE_SNPRINTF
	snprintf(buf, sizeof(buf),"GET %s HTTP/1.0\r\nHost: %s\r\n\r\n",
		 ctxt->path, ctxt->hostname);
#else
	sprintf(buf, "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n",
		 ctxt->path, ctxt->hostname);
#endif
#ifdef DEBUG_HTTP
	printf("-> GET %s HTTP/1.0\n-> Host: %s\n\n",
	       ctxt->path, ctxt->hostname);
#endif
    }
    buf[sizeof(buf) - 1] = 0;
    ctxt->outptr = ctxt->out = xmlMemStrdup(buf);
    ctxt->state = XML_NANO_HTTP_WRITE;
    xmlNanoHTTPSend(ctxt);
    ctxt->state = XML_NANO_HTTP_READ;
    head = 1;

    while ((p = xmlNanoHTTPReadLine(ctxt)) != NULL) {
        if (head && (*p == 0)) {
	    head = 0;
	    ctxt->content = ctxt->inrptr;
	    xmlFree(p);
	    break;
	}
	xmlNanoHTTPScanAnswer(ctxt, p);

#ifdef DEBUG_HTTP
	if (p != NULL) printf("<- %s\n", p);
#endif
        if (p != NULL) xmlFree(p);
    }

    if ((ctxt->location != NULL) && (ctxt->returnValue >= 300) &&
        (ctxt->returnValue < 400)) {
#ifdef DEBUG_HTTP
	printf("\nRedirect to: %s\n", ctxt->location);
#endif
	while (xmlNanoHTTPRecv(ctxt)) ;
        if (nbRedirects < XML_NANO_HTTP_MAX_REDIR) {
	    nbRedirects++;
	    redirURL = xmlMemStrdup(ctxt->location);
	    xmlNanoHTTPFreeCtxt(ctxt);
	    goto retry;
	}
	xmlNanoHTTPFreeCtxt(ctxt);
#ifdef DEBUG_HTTP
	printf("Too many redirrects, aborting ...\n");
#endif
	return(NULL);

    }

    if ((contentType != NULL) && (ctxt->contentType != NULL))
        *contentType = xmlMemStrdup(ctxt->contentType);

#ifdef DEBUG_HTTP
    if (ctxt->contentType != NULL)
	printf("\nCode %d, content-type '%s'\n\n",
	       ctxt->returnValue, ctxt->contentType);
    else
	printf("\nCode %d, no content-type\n\n",
	       ctxt->returnValue);
#endif

    return((void *) ctxt);
}

/**
 * xmlNanoHTTPRead:
 * @ctx:  the HTTP context
 * @dest:  a buffer
 * @len:  the buffer length
 *
 * This function tries to read @len bytes from the existing HTTP connection
 * and saves them in @dest. This is a blocking call.
 *
 * Returns the number of byte read. 0 is an indication of an end of connection.
 *         -1 indicates a parameter error.
 */
int
xmlNanoHTTPRead(void *ctx, void *dest, int len) {
    xmlNanoHTTPCtxtPtr ctxt = (xmlNanoHTTPCtxtPtr) ctx;

    if (ctx == NULL) return(-1);
    if (dest == NULL) return(-1);
    if (len <= 0) return(0);

    while (ctxt->inptr - ctxt->inrptr < len) {
        if (xmlNanoHTTPRecv(ctxt) == 0) break;
    }
    if (ctxt->inptr - ctxt->inrptr < len)
        len = ctxt->inptr - ctxt->inrptr;
    memcpy(dest, ctxt->inrptr, len);
    ctxt->inrptr += len;
    return(len);
}

/**
 * xmlNanoHTTPClose:
 * @ctx:  the HTTP context
 *
 * This function closes an HTTP context, it ends up the connection and
 * free all data related to it.
 */
void
xmlNanoHTTPClose(void *ctx) {
    xmlNanoHTTPCtxtPtr ctxt = (xmlNanoHTTPCtxtPtr) ctx;

    if (ctx == NULL) return;

    xmlNanoHTTPFreeCtxt(ctxt);
}

/**
 * xmlNanoHTTPMethod:
 * @URL:  The URL to load
 * @method:  the HTTP method to use
 * @input:  the input string if any
 * @contentType:  the Content-Type information IN and OUT
 * @headers:  the extra headers
 *
 * This function try to open a connection to the indicated resource
 * via HTTP using the given @method, adding the given extra headers
 * and the input buffer for the request content.
 *
 * Returns NULL in case of failure, otherwise a request handler.
 *     The contentType, if provided must be freed by the caller
 */

#ifndef DEBUG_HTTP
#define DEBUG_HTTP
#endif
void *
xmlNanoHTTPMethod(const char *URL, const char *method, const char *input,
                  char **contentType, const char *headers) {
    xmlNanoHTTPCtxtPtr ctxt;
    char *bp, *p;
    int blen, ilen, ret;
    int head;
    int nbRedirects = 0;
    char *redirURL = NULL;
    filedesc_t fd;

    if (URL == NULL) return(NULL);
    if (method == NULL) method = "GET";
    xmlNanoHTTPInit();

retry:
    if (redirURL == NULL)
	ctxt = xmlNanoHTTPNewCtxt(URL);
    else {
	ctxt = xmlNanoHTTPNewCtxt(redirURL);
	xmlFree(redirURL);
	redirURL = NULL;
    }

    if ((ctxt->protocol == NULL) || (strcmp(ctxt->protocol, "http"))) {
        xmlNanoHTTPFreeCtxt(ctxt);
	if (redirURL != NULL) xmlFree(redirURL);
        return(NULL);
    }
    if (ctxt->hostname == NULL) {
        xmlNanoHTTPFreeCtxt(ctxt);
        return(NULL);
    }
    if (proxy) {
	blen = strlen(ctxt->hostname) * 2 + 16;
	fd = xmlNanoHTTPConnectHost(proxy, proxyPort);
    }
    else {
	blen = strlen(ctxt->hostname);
	fd = xmlNanoHTTPConnectHost(ctxt->hostname, ctxt->port);
    }
    if (INVALID_FD (fd)) {
        xmlNanoHTTPFreeCtxt(ctxt);
        return(NULL);
    }
    ctxt->fd = fd;

    if (input != NULL) {
	ilen = strlen(input);
	blen += ilen + 32;
    }
    else
	ilen = 0;
    if (headers != NULL)
	blen += strlen(headers);
    if (contentType && *contentType)
	blen += strlen(*contentType) + 16;
    blen += strlen(method) + strlen(ctxt->path) + 23;
    bp = xmlMalloc(blen);
    if (proxy) {
	if (ctxt->port != 80) {
	    sprintf(bp, "%s http://%s:%d%s", method, ctxt->hostname,
		 ctxt->port, ctxt->path);
	}
	else
	    sprintf(bp, "%s http://%s%s", method, ctxt->hostname, ctxt->path);
    }
    else
	sprintf(bp, "%s %s", method, ctxt->path);
    p = bp + strlen(bp);
    sprintf(p, " HTTP/1.0\r\nHost: %s\r\n", ctxt->hostname);
    p += strlen(p);
    if (contentType != NULL && *contentType) {
	sprintf(p, "Content-Type: %s\r\n", *contentType);
	p += strlen(p);
    }
    if (headers != NULL) {
	strcpy(p, headers);
	p += strlen(p);
    }
    if (input != NULL)
	sprintf(p, "Content-Length: %d\r\n\r\n%s", ilen, input);
    else
	strcpy(p, "\r\n");
#ifdef DEBUG_HTTP
    xmlGenericError(xmlGenericErrorContext,
	    "-> %s%s", proxy? "(Proxy) " : "", bp);
    if ((blen -= strlen(bp)+1) < 0)
	xmlGenericError(xmlGenericErrorContext,
		"ERROR: overflowed buffer by %d bytes\n", -blen);
#endif
    ctxt->outptr = ctxt->out = bp;
    ctxt->state = XML_NANO_HTTP_WRITE;
    xmlNanoHTTPSend(ctxt);
    ctxt->state = XML_NANO_HTTP_READ;
    head = 1;

    while ((p = xmlNanoHTTPReadLine(ctxt)) != NULL) {
        if (head && (*p == 0)) {
	    head = 0;
	    ctxt->content = ctxt->inrptr;
	    xmlFree(p);
	    break;
	}
	xmlNanoHTTPScanAnswer(ctxt, p);

#ifdef DEBUG_HTTP
	xmlGenericError(xmlGenericErrorContext, "<- %s\n", p);
#endif
        xmlFree(p);
    }

    if ((ctxt->location != NULL) && (ctxt->returnValue >= 300) &&
        (ctxt->returnValue < 400)) {
#ifdef DEBUG_HTTP
	xmlGenericError(xmlGenericErrorContext,
		"\nRedirect to: %s\n", ctxt->location);
#endif
	while (xmlNanoHTTPRecv(ctxt)) ;
        if (nbRedirects < XML_NANO_HTTP_MAX_REDIR) {
	    nbRedirects++;
	    redirURL = xmlMemStrdup(ctxt->location);
	    xmlNanoHTTPFreeCtxt(ctxt);
	    goto retry;
	}
	xmlNanoHTTPFreeCtxt(ctxt);
#ifdef DEBUG_HTTP
	xmlGenericError(xmlGenericErrorContext,
		"Too many redirects, aborting ...\n");
#endif
	return(NULL);

    }

    if (contentType != NULL) {
	if (ctxt->contentType != NULL)
	    *contentType = xmlMemStrdup(ctxt->contentType);
	else
	    *contentType = NULL;
    }

#ifdef DEBUG_HTTP
    if (ctxt->contentType != NULL)
	xmlGenericError(xmlGenericErrorContext,
		"\nCode %d, content-type '%s'\n\n",
	       ctxt->returnValue, ctxt->contentType);
    else
	xmlGenericError(xmlGenericErrorContext,
		"\nCode %d, no content-type\n\n",
	       ctxt->returnValue);
#endif

    return((void *) ctxt);
}

/**
 * xmlNanoHTTPFetch:
 * @URL:  The URL to load
 * @filename:  the filename where the content should be saved
 * @contentType:  if available the Content-Type information will be
 *                returned at that location
 *
 * This function try to fetch the indicated resource via HTTP GET
 * and save it's content in the file.
 *
 * Returns -1 in case of failure, 0 incase of success. The contentType,
 *     if provided must be freed by the caller
 */
int
xmlNanoHTTPFetch(const char *URL, const char *filename, char **contentType) {
    void *ctxt;
    char buf[4096];
    int len;
    filedesc_t fd = STDIN;

    ctxt = xmlNanoHTTPOpen(URL, contentType);
    if (ctxt == NULL) return(-1);

    /* XXX reddy: check for file not found or other http errors */

    if ( xmlNanoHTTPReturnCode( ctxt ) > 299 )
      return (-1);

    if (!strcmp(filename, "-")) 
        ; /* Use value from initialization */
    else {
#ifdef HAVE_NSPR_H
        fd = PR_Open(filename, PR_WRONLY | PR_CREATE_FILE, 0666);
#else
        fd = open(filename, O_CREAT | O_WRONLY);
#endif
	if (INVALID_FD (fd)) {
	    xmlNanoHTTPClose(ctxt);
	    if ((contentType != NULL) && (*contentType != NULL)) {
	        xmlFree(*contentType);
		*contentType = NULL;
	    }
	    return(-1);
	}
    }

    while ((len = xmlNanoHTTPRead(ctxt, buf, sizeof(buf))) > 0) {
	Write(fd, buf, len);
    }

    xmlNanoHTTPClose(ctxt);
#ifdef HAVE_NSPR_H
    PR_Close(fd);
#else
    close(fd);
#endif
    return(0);
}

/**
 * xmlNanoHTTPSave:
 * @ctx:  the HTTP context
 * @filename:  the filename where the content should be saved
 *
 * This function saves the output of the HTTP transaction to a file
 * It closes and free the context at the end
 *
 * Returns -1 in case of failure, 0 incase of success.
 */
int
xmlNanoHTTPSave(void *ctxt, const char *filename) {
    char buf[4096];
    int len;
    filedesc_t fd = STDOUT;
    
    if (ctxt == NULL) return(-1);

    if (!strcmp(filename, "-")) 
        ; /* Use value from initialization */
    else {
#ifdef HAVE_NSPR_H
        fd = PR_Open(filename, PR_WRONLY | PR_CREATE_FILE, 0666);
#else
        fd = open(filename, O_CREAT | O_WRONLY);
#endif
	if (INVALID_FD (fd)) {
	    xmlNanoHTTPClose(ctxt);
	    return(-1);
	}
    }

    while ((len = xmlNanoHTTPRead(ctxt, buf, sizeof(buf))) > 0) {
	Write(fd, buf, len);
    }

    xmlNanoHTTPClose(ctxt);
    return(0);
}

/**
 * xmlNanoHTTPReturnCode:
 * @ctx:  the HTTP context
 *
 * Returns the HTTP return code for the request.
 */
int
xmlNanoHTTPReturnCode(void *ctx) {
    xmlNanoHTTPCtxtPtr ctxt = (xmlNanoHTTPCtxtPtr) ctx;

    if (ctxt == NULL) return(-1);

    return(ctxt->returnValue);
}

/**
 * xmlNanoHTTPContentLength:
 * @ctx:  the HTTP context
 *
 * Returns the value of the Content-Length header for the request.
 */
int
xmlNanoHTTPContentLength(void *ctx) {
    xmlNanoHTTPCtxtPtr ctxt = (xmlNanoHTTPCtxtPtr) ctx;

    if (ctxt == NULL) return(-1);

    return(ctxt->contentLength);
}

/**
 * xmlNanoHTTPAuthHeader:
 * @ctx:  the HTTP context
 *
 * Returns the stashed value of the WWW-Authenticate or Proxy-Authenticate
 * header.
 */
const char *
xmlNanoHTTPAuthHeader(void *ctx) {
    xmlNanoHTTPCtxtPtr ctxt = (xmlNanoHTTPCtxtPtr) ctx;

    if (ctxt == NULL) return(NULL);

    return(ctxt->authHeader);
}

#ifdef STANDALONE
int main(int argc, char **argv) {
    char *contentType = NULL;

    if (argv[1] != NULL) {
	if (argv[2] != NULL) 
	    xmlNanoHTTPFetch(argv[1], argv[2], &contentType);
        else
	    xmlNanoHTTPFetch(argv[1], "-", &contentType);
	if (contentType != NULL) xmlFree(contentType);
    } else {
        xmlGenericError(xmlGenericErrorContext,
		"%s: minimal HTTP GET implementation\n", argv[0]);
        xmlGenericError(xmlGenericErrorContext,
		"\tusage %s [ URL [ filename ] ]\n", argv[0]);
    }
    xmlNanoHTTPCleanup();
    xmlMemoryDump();
    return(0);
}
#endif /* STANDALONE */
#else /* !LIBXML_HTTP_ENABLED */
#ifdef STANDALONE
#include <stdio.h>
int main(int argc, char **argv) {
    xmlGenericError(xmlGenericErrorContext,
	    "%s : HTTP support not compiled in\n", argv[0]);
    return(0);
}
#endif /* STANDALONE */
#endif /* LIBXML_HTTP_ENABLED */
