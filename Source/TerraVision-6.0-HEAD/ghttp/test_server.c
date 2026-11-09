/*
 * test_server.c -- Test server code.
 * Created: Christopher Blizzard <blizzard@appliedtheory.com> 14-Aug-1998
 */

#ifndef __WIN32__

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
  int                l_listen_socket = 0;
  int                l_socket = 0;
  char               l_buf[1024];
  int                l_read = 0;
  int                l_opt = 1;
  char              *l_body = "<html>This is a test.</html>\n";
  char              *l_headers_pre = 
    "HTTP/1.1 200 OK\r\nConnection: close\r\nObscure-Header: test\r\n\ttestagain\r\nContent-Length: ";
  char              *l_headers_post =
    "\r\n\r\n";
  struct sockaddr_in saddr;

  memset(l_buf, 0, 1024);
  memset(&saddr, 0, sizeof(struct sockaddr_in));
  saddr.sin_port = htons(12000);
  saddr.sin_addr.s_addr = INADDR_ANY;
  saddr.sin_family = AF_INET;
  printf("Allocating socket...\n");
  if ((l_listen_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      printf("Failed to allocate socket: %s\n", strerror(errno));
      exit(1);
    }
  if (setsockopt(l_listen_socket, SOL_SOCKET, SO_REUSEADDR,
		 &l_opt, sizeof(l_opt)) < 0)
    {
      printf("Failed to set socket option: %s\n", strerror(errno));
      exit(1);
    }
  printf("Binding...\n");
  if (bind(l_listen_socket,
	   (struct sockaddr *)&saddr,
	   sizeof(struct sockaddr_in)) < 0)
    {
      printf("Failed to bind to socket: %s\n", strerror(errno));
      exit(1);
    }
  printf("Listening...\n");
  if (listen(l_listen_socket, 5) < 0)
    {
      printf("Failed to listen on socket: %s\n", strerror(errno));
      exit(1);
    }
  printf("Waiting for connection.\n");
  while ((l_socket = accept(l_listen_socket, NULL, 0)) > -1)
    {
      printf("Got connection.\n");
      while ((l_read = read(l_socket, l_buf, 1023)) > 0)
	{
	  printf("%s", l_buf);
	  if (strstr(l_buf, "\r\n\r\n"))
	    break;
	  memset(l_buf, 0, 1024);
	}
      memset(l_buf, 0, 1024);
      write(l_socket, l_headers_pre, strlen(l_headers_pre));
      sprintf(l_buf, "%d", strlen(l_body));
      write(l_socket, l_buf, strlen(l_buf));
      write(l_socket, l_headers_post, strlen(l_headers_post));
      write(l_socket, l_body, strlen(l_body));
      close (l_socket);
      memset(l_buf, 0, 1024);
    }
  return 0;
}

#endif
