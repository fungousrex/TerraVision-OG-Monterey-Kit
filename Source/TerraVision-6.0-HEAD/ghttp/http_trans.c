/*
 * http_trans.c -- Functions for doing transport related stuff including
 * automatically extending buffers and whatnot.
 * Created: Christopher Blizzard <blizzard@appliedtheory.com>, 5-Aug-1998
 *
 * Copyright (C) 1998 Free Software Foundation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "http_trans.h"
#include "http_global.h"

#if !defined WIN32
#include <unistd.h>
#endif

#ifdef HAVE_NSPR_H
#include <prerror.h>
#include <prsystem.h>
#include <prnetdb.h>

#define INVALID_SOCK (NULL)
#define SOCKET_READ(fd,buf,len) PR_Recv(fd,buf,len,0,PR_INTERVAL_NO_TIMEOUT)
#define SOCKET_WRITE(fd,buf,len) PR_Send(fd,buf,len,0,PR_INTERVAL_NO_TIMEOUT)
#define SOCKET_CLOSE(fd) PR_Close(fd)

struct _ghttp_socket {
  char buf[PR_NETDB_BUF_SIZE];
  PRHostEnt hostent;
  PRHostEnt *hostinfo;
  PRNetAddr saddr;
  PRFileDesc *sock;
};
#else

#if !defined (WIN32)
# include <sys/time.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>
#else
# include <winsock2.h>
#endif

#include <sys/types.h>

# define INVALID_SOCK (-1)
# define SOCKET_READ(fd,buf,len) recv(fd,buf,len,0)
# define SOCKET_WRITE(fd,buf,len) send(fd,buf,len,0)
# ifdef WIN32
#  define SOCKET_CLOSE(fd) closesocket(fd)
# else
#  define SOCKET_CLOSE(fd) close(fd)
# endif

struct _ghttp_socket {
  struct hostent      *hostinfo;
  struct sockaddr_in   saddr;
  int                  sock;
};
#endif

#ifdef GHTTP_DEBUG
int ghttp_debug = 0;
#endif

static int
http_trans_buf_free(http_trans_conn *a_conn);

#ifdef HAVE_NSPR_H
int
http_trans_connect(http_trans_conn *a_conn)
{
  if ((a_conn == NULL) || (a_conn->host == NULL))
    goto ec;
  if (a_conn->socket->hostinfo == NULL)
    {
      http_trans_socket *socket = a_conn->socket;
      int i;

      /* look up the name of the proxy if it's there. */
      if (a_conn->proxy_host)
	{
	  if (PR_GetHostByName (a_conn->proxy_host,
				socket->buf, PR_NETDB_BUF_SIZE,
				&socket->hostent) != PR_SUCCESS)
	    {
	      a_conn->error_type = http_trans_err_type_host;
	      a_conn->error = PR_GetError ();
	      goto ec;
	    }

	  i = PR_EnumerateHostEnt (0, &socket->hostent, a_conn->proxy_port,
				   &socket->saddr);
	}
      else
	{
	  /* look up the name */
	  if (PR_GetHostByName (a_conn->host,
				socket->buf, PR_NETDB_BUF_SIZE,
				&socket->hostent) != PR_SUCCESS)
	    {
	      a_conn->error_type = http_trans_err_type_host;
	      a_conn->error = PR_GetError ();
	      goto ec;
	    }

	  i = PR_EnumerateHostEnt (0, &socket->hostent, a_conn->port,
				   &socket->saddr);
	}

      if (i == 0)
	{
	  a_conn->error_type = http_trans_err_type_host;
	  a_conn->error = PR_GetError ();
	  goto ec;
	}
      socket->hostinfo = &socket->hostent;
    }
  /* set up the socket */
  if ((a_conn->socket->sock = PR_NewTCPSocket ()) == 0)
    {
      a_conn->error_type = http_trans_err_type_errno;
      a_conn->error = PR_GetOSError ();
      goto ec;
    }
  /* set up the socket */
  if (PR_Connect(a_conn->socket->sock,
		 &a_conn->socket->saddr,
		 PR_INTERVAL_NO_TIMEOUT) != PR_SUCCESS)
    {
      PRErrorCode err = PR_GetOSError ();
      if (err != PR_IN_PROGRESS_ERROR)
	{
	  a_conn->error_type = http_trans_err_type_errno;
	  a_conn->error = err;
	  goto ec;
	}
      else
	{
	  PRPollDesc pds[1];
	  pds[0].fd = a_conn->socket->sock;
	  pds[0].in_flags = PR_POLL_WRITE | PR_POLL_EXCEPT;
	  if (PR_Poll (pds, 1, PR_INTERVAL_NO_TIMEOUT) <= 0)
	    {
	      a_conn->error_type = http_trans_err_type_errno;
	      a_conn->error = PR_GetOSError ();
	      goto ec;
	    }
	}
    }
  
  return 0;
 ec:
  return -1;
}
#else
int
http_trans_connect(http_trans_conn *a_conn)
{
  if ((a_conn == NULL) || (a_conn->host == NULL))
    goto ec;
  if (a_conn->socket->hostinfo == NULL)
    {
      /* look up the name of the proxy if it's there. */
      if (a_conn->proxy_host)
	{
	  if ((a_conn->socket->hostinfo = gethostbyname(a_conn->proxy_host)) == NULL)
	    {
	      a_conn->error_type = http_trans_err_type_host;
	      a_conn->error = h_errno;
	      goto ec;
	    }
	}
      else
	{
	  /* look up the name */
	  if ((a_conn->socket->hostinfo = gethostbyname(a_conn->host)) == NULL)
	    {
	      a_conn->error_type = http_trans_err_type_host;
	      a_conn->error = h_errno;
	      goto ec;
	    }
	}
      /* set up the saddr */
      a_conn->socket->saddr.sin_family = AF_INET;
      /* set the proxy port */
      if (a_conn->proxy_host)
	a_conn->socket->saddr.sin_port = htons(a_conn->proxy_port);
      else
	a_conn->socket->saddr.sin_port = htons(a_conn->port);
      /* copy the name info */
      memcpy(&a_conn->socket->saddr.sin_addr.s_addr,
	     a_conn->socket->hostinfo->h_addr_list[0],
	     sizeof(unsigned long));
    }
  /* set up the socket */
  if ((a_conn->socket->sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      a_conn->error_type = http_trans_err_type_errno;
      a_conn->error = errno;
      goto ec;
    }
  /* set up the socket */
  if (connect(a_conn->socket->sock,
	      (struct sockaddr *)&a_conn->socket->saddr,
	      sizeof(struct sockaddr)) < 0)
    {
      a_conn->error_type = http_trans_err_type_errno;
      a_conn->error = errno;
      goto ec;
    }
  
  return 0;
 ec:
  return -1;
}
#endif

http_trans_conn *
http_trans_conn_new(void)
{
  http_trans_conn *l_return = NULL;

  /* allocate a new connection struct */
  l_return = (http_trans_conn *)malloc(sizeof(http_trans_conn));
  memset(l_return, 0, sizeof(http_trans_conn));
  /* default to 80 */
  l_return->port = 80;
  /* default to 1000 bytes at a time */
  l_return->io_buf_chunksize = 1024;
  /* allocate a new trans buffer */
  l_return->io_buf = malloc(l_return->io_buf_chunksize);
  memset(l_return->io_buf, 0, l_return->io_buf_chunksize);
  l_return->io_buf_len = l_return->io_buf_chunksize;
  /* make sure the socket looks like it's closed */
  l_return->socket = malloc (sizeof (*l_return->socket));
  l_return->socket->hostinfo = NULL;
  l_return->socket->sock = INVALID_SOCK;
  return l_return;
}

void
http_trans_conn_destroy(http_trans_conn *a_conn)
{
  /* destroy the connection structure. */
  if (a_conn == NULL)
    return;
  if (a_conn->io_buf)
    free(a_conn->io_buf);
  http_trans_conn_close (a_conn);
  free(a_conn->socket);
  free(a_conn);
  return;
}

#ifdef HAVE_NSPR_H
void
http_trans_conn_close (http_trans_conn *a_conn)
{
  if (a_conn->socket->sock != INVALID_SOCK) {
    SOCKET_CLOSE(a_conn->socket->sock);
    a_conn->socket->sock = INVALID_SOCK;
  }
}
#else
void
http_trans_conn_close (http_trans_conn *a_conn)
{
  if (a_conn->socket->sock != INVALID_SOCK) {
    SOCKET_CLOSE(a_conn->socket->sock);
    a_conn->socket->sock = INVALID_SOCK;
  }
}
#endif

void
http_trans_conn_reset (http_trans_conn *a_conn)
{
  http_trans_conn_close (a_conn);
  a_conn->socket->hostinfo = NULL;
}

int
http_trans_conn_connected (http_trans_conn *a_conn)
{
  return a_conn->socket->sock != INVALID_SOCK;
}

#ifdef HAVE_NSPR_H
const char *
http_trans_get_host_error(int a_herror)
{
  switch (a_herror)
    {
    case PR_DIRECTORY_LOOKUP_ERROR:
      return "Host not found";
    case PR_INSUFFICIENT_RESOURCES_ERROR:
      return "Insufficient memory for host data";
    case PR_OUT_OF_MEMORY_ERROR:
      return "Out of memory";
    case PR_INVALID_ARGUMENT_ERROR:
      return "Invalid argument";
    case PR_BAD_ADDRESS_ERROR:
      return "Invalid network address";
    case PR_ADDRESS_IN_USE_ERROR:
      return "Address already in use";
    case PR_CONNECT_REFUSED_ERROR:
      return "Connection refused";
    case PR_NETWORK_UNREACHABLE_ERROR:
      return "Network is unreachable";
    default:
      return "No error or error not known.";
    }
}
#else
const char *
http_trans_get_host_error(int a_herror)
{
  switch (a_herror)
    {
    case HOST_NOT_FOUND:
      return "Host not found";
    case NO_ADDRESS:
      return "An address is not associated with that host";
    case NO_RECOVERY:
      return "An unrecoverable name server error occured";
    case TRY_AGAIN:
      return "A temporary error occurred on an authoritative name server.  Please try again later.";
    default:
      return "No error or error not known.";
    }
}
#endif

int
http_trans_append_data_to_buf(http_trans_conn *a_conn,
			      char *a_data,
			      int   a_data_len)
{
  if (http_trans_buf_free(a_conn) < a_data_len)
    {
      a_conn->io_buf = realloc(a_conn->io_buf, a_conn->io_buf_len + a_data_len);
      a_conn->io_buf_len += a_data_len;
    }
  memcpy(&a_conn->io_buf[a_conn->io_buf_alloc], a_data, a_data_len);
  a_conn->io_buf_alloc += a_data_len;
  return 1;
}

int
http_trans_read_into_buf(http_trans_conn *a_conn)
{
  int l_read = 0;
  int l_bytes_to_read = 0;

  /* set the length if this is the first time */
  if (a_conn->io_buf_io_left == 0)
    {
      a_conn->io_buf_io_left = a_conn->io_buf_chunksize;
      a_conn->io_buf_io_done = 0;
    }
  /* make sure there's enough space */
  if (http_trans_buf_free(a_conn) < a_conn->io_buf_io_left)
    {
      a_conn->io_buf = realloc(a_conn->io_buf,
			       a_conn->io_buf_len + a_conn->io_buf_io_left);
      a_conn->io_buf_len += a_conn->io_buf_io_left;
    }
  /* check to see how much we should try to read */
  if (a_conn->io_buf_io_left > a_conn->io_buf_chunksize)
    l_bytes_to_read = a_conn->io_buf_chunksize;
  else
    l_bytes_to_read = a_conn->io_buf_io_left;
  if (HTDEBUG) {
    /* read in some data */
    printf ("read (%p, %p, %d)\n", 
	    a_conn->socket->sock,
	    &a_conn->io_buf[a_conn->io_buf_alloc],
	    l_bytes_to_read);
  }
  if ((a_conn->last_read = l_read
       = SOCKET_READ (a_conn->socket->sock,
		      &a_conn->io_buf[a_conn->io_buf_alloc],
		      l_bytes_to_read)) < 0)
    {
      if (HTDEBUG)
	printf ("read () <= %d\n", l_read);
      if (errno == EINTR)
	l_read = 0;
      else
	return HTTP_TRANS_ERR;
    }
  else if (l_read == 0) {
    if (HTDEBUG)
      printf ("read () <= %d\n", l_read);
    return HTTP_TRANS_DONE;
  }
  if (HTDEBUG)
    printf ("read () <= %d\n", l_read);

  /* mark the buffer */
  a_conn->io_buf_io_left -= l_read;
  a_conn->io_buf_io_done += l_read;
  a_conn->io_buf_alloc += l_read;
  /* generate the result */
  if (a_conn->io_buf_io_left == 0)
    return HTTP_TRANS_DONE;
  return HTTP_TRANS_NOT_DONE;
}

int
http_trans_write_buf(http_trans_conn *a_conn)
{
  int l_written = 0;

  if (a_conn->io_buf_io_left == 0)
    {
      a_conn->io_buf_io_left = a_conn->io_buf_alloc;
      a_conn->io_buf_io_done = 0;
    }
  /* write out some data */
  if (HTDEBUG)
    printf ("%s:%d [%d] \"%.*s\"\n",
	    a_conn->host, a_conn->port, a_conn->io_buf_io_left,
	    a_conn->io_buf_io_left,
	    &a_conn->io_buf[a_conn->io_buf_io_done]);

  if ((a_conn->last_read = l_written
       = SOCKET_WRITE (a_conn->socket->sock,
		       &a_conn->io_buf[a_conn->io_buf_io_done],
		       a_conn->io_buf_io_left)) <= 0)
    {
      if (errno == EINTR)
	l_written = 0;
      else
	return HTTP_TRANS_ERR;
    }
  if (l_written == 0)
    return HTTP_TRANS_DONE;
  /* advance the counters */
  a_conn->io_buf_io_left -= l_written;
  a_conn->io_buf_io_done += l_written;
  if (a_conn->io_buf_io_left == 0)
    return HTTP_TRANS_DONE;
  return HTTP_TRANS_NOT_DONE;
}

void
http_trans_buf_reset(http_trans_conn *a_conn)
{
  if (! a_conn->io_buf || a_conn->io_buf_len != a_conn->io_buf_chunksize) {
    if (a_conn->io_buf)
      free(a_conn->io_buf);
    a_conn->io_buf = malloc(a_conn->io_buf_chunksize);
  }

  memset(a_conn->io_buf, 0, a_conn->io_buf_chunksize);
  a_conn->io_buf_len = a_conn->io_buf_chunksize;
  a_conn->io_buf_alloc = 0;
  a_conn->io_buf_io_done = 0;
  a_conn->io_buf_io_left = 0;
}

void
http_trans_buf_clip(http_trans_conn *a_conn, char *a_clip_to)
{
  int l_bytes = 0;
  
  /* get the number of bytes to clip off of the front */
  l_bytes = a_clip_to - a_conn->io_buf;
  if (l_bytes > 0)
    {
      memmove(a_conn->io_buf, a_clip_to, a_conn->io_buf_alloc - l_bytes);
      a_conn->io_buf_alloc -= l_bytes;
    }
  a_conn->io_buf_io_done = 0;
  a_conn->io_buf_io_left = 0;
}

char *
http_trans_buf_has_patt(char *a_buf, int a_len,
			char *a_pat, int a_patlen)
{
  int i = 0;
  for ( ; i <= ( a_len - a_patlen ); i++ )
    {
      if (a_buf[i] == a_pat[0])
	{
	  if (memcmp(&a_buf[i], a_pat, a_patlen) == 0)
	    return &a_buf[i];
	}
    }
  return NULL;
}

/* static functions */

static int
http_trans_buf_free(http_trans_conn *a_conn)
{
  return (a_conn->io_buf_len - a_conn->io_buf_alloc);
}

int
ghttp_socket_connected (http_trans_socket *sock)
{
  return sock->sock != INVALID_SOCK;
}

int
ghttp_socket_read (http_trans_socket *sock, void *buf, size_t nbytes)
{
  if (sock->sock == INVALID_SOCK)
    return -1;
  return SOCKET_READ (sock->sock, buf, nbytes);
}

int
ghttp_socket_write (http_trans_socket *sock, void *buf, size_t nbytes)
{
  if (sock->sock == INVALID_SOCK)
    return -1;
  return SOCKET_WRITE (sock->sock, buf, nbytes);
}
