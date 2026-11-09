#include <sys/types.h>
#include <sys/stat.h>
/* #include <sys/mman.h> */  /* not avail under windows */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "http_uri.h"
#include "http_hdrs.h"
#include "http_trans.h"
#include "http_req.h"
#include "http_resp.h"
#include "http_date.h"
#include "http_global.h"

int main(int argc, char **argv)
{
  
  http_uri *l_uri = NULL;
  http_trans_conn *l_conn = NULL;
  http_req *l_req = NULL;
  http_resp *l_resp = NULL;
  int l_rv = 0;
  http_req_type l_type = http_req_type_get;
  int l_fd;
  int i = 0;
  char *l_buf = NULL;
  struct stat l_stat;
  
  memset(&l_stat, 0, sizeof(struct stat));
  if (argc != 4)
    {
      printf("test_post_client [post|put] <url> <file>\n");
      exit(1);
    }
  if (!strcasecmp(argv[1], "post"))
    {
      l_type = http_req_type_post;
    }
  else if (!strcasecmp(argv[1], "put"))
    {
      l_type = http_req_type_put;
    }
  else
    {
      printf("test_post_client [post|put] <url> <file>\n");
      exit(1);
    }
  l_fd = open(argv[3], O_RDONLY);
  if (l_fd < 0)
    {
      printf("Invalid file: %s\n", strerror(errno));
      exit(1);
    }
  if (fstat(l_fd, &l_stat) < 0)
    {
      printf("Failed to stat file: %s\n", strerror(errno));
      exit(1);
    }
  l_uri = http_uri_new();
  if (http_uri_parse(argv[2], l_uri) < 0)
    {
      printf("Invalid URI.\n");
      exit(1);
    }
  printf("*** testing transport ***\n");
  l_conn = http_trans_conn_new();
  l_conn->host = l_uri->host;
  l_conn->port = l_uri->port;
  l_conn->sync = HTTP_TRANS_ASYNC;
  l_conn->io_buf_chunksize = 500;
  if (http_trans_connect(l_conn) < 0)
    {
      if (l_conn->error_type == http_trans_err_type_errno)
	printf("Failed to connect: %s\n", strerror(l_conn->error));
      else if (l_conn->error_type == http_trans_err_type_host)
	printf("Failed to connect: %s\n",
	       http_trans_get_host_error(l_conn->error));
      exit(1);
    }
  else
    printf("*** connected ***\n");
  /* do this a few times */
  printf("*** sending request ***\n");
  l_req = http_req_new();
  l_req->type = l_type;
  l_req->host = l_conn->host;
  l_req->resource = l_uri->resource;
  l_req->body_len = l_stat.st_size;
  if ((l_req->body = mmap(0, l_req->body_len, PROT_READ, MAP_SHARED, l_fd, 0)) == (void *)-1)
    {
      printf("Failed to map file: %s\n", strerror(errno));
      exit(1);
    }
  /* set up some defaults */
  http_req_prepare(l_req);
  /* send request */
  do {
    l_rv = http_req_send(l_req, l_conn);
    if (l_rv == HTTP_TRANS_ERR)
      {
	printf("Failed to send request headers: %s\n", strerror(errno));
	exit(1);
      }
  } while (l_rv == HTTP_TRANS_NOT_DONE);
  printf("*** reading response ***\n");
  l_resp = http_resp_new();
  do {
    l_rv = http_resp_read_headers(l_resp, l_conn);
    if (l_rv == HTTP_TRANS_ERR)
      {
	printf("Failed to read response: %s\n", strerror(errno));
	exit(1);
      }
  } while (l_rv == HTTP_TRANS_NOT_DONE);
  printf("HTTP/%01.1f %d %s\n",
	 l_resp->http_ver,
	 l_resp->status_code,
	     l_resp->reason_phrase);
  for (i=0; i < HTTP_HDRS_MAX; i++)
    {
      if (l_resp->headers->header[i])
	printf("[HDR] %s [VAL] %s\n",
	       l_resp->headers->header[i],
	       l_resp->headers->value[i]);
    }
  do {
    l_rv = http_resp_read_body(l_resp, l_req, l_conn);
    printf("Read body chunk. ( %d of %d )\n", l_conn->io_buf_io_done, l_conn->io_buf_io_left);
    if (l_rv == HTTP_TRANS_ERR)
      {
	printf("Failed to read body: %s\n", strerror(errno));
	exit(1);
      }
  } while (l_rv == HTTP_TRANS_NOT_DONE);
  l_buf = malloc(l_resp->body_len + 1);
  memset(l_buf, 0, l_resp->body_len + 1);
  memcpy(l_buf, l_resp->body, l_resp->body_len);
  printf("[BODY] %d bytes.\n", l_resp->body_len);
  /* printf("%s", l_buf); */
  free(l_buf);
  if (! http_trans_conn_connected (l_conn))
    {
      printf("Connection has been closed.\n");
      exit(1);
    }
  /* http_trans_buf_clean(l_conn); */
  http_resp_destroy(l_resp);
  http_req_destroy(l_req);
  http_trans_conn_destroy(l_conn);
  return 0;
}

void
test_uri_parser(char *a_string)
{
  http_uri *uri = NULL;
  uri = http_uri_new();

  printf("Parsing:\n\t%s\n", a_string);
  if (http_uri_parse(a_string, uri) < 0)
    {
      printf("Failed to parse uri.\n");
      return;
    }
  else
    {
      printf("\thost: %s\n", uri->host);
      printf("\tport: %d\n", uri->port);
      printf("\tresource: %s\n", uri->resource);
    }
  http_uri_destroy(uri);
  return;
}

void
test_date_parser(char *a_string)
{
  time_t l_date;

  printf("%s = ", a_string);
  l_date = http_date_to_time(a_string);
  if (l_date < 0)
    printf("Invalid date\n");
  else
    printf("%s\n", ctime(&l_date));
}
