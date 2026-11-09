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

char *date_1 = "Sun, 06 Nov 1994 08:49:37 GMT";
char *date_2 = "Sunday, 06-Nov-94 08:49:37 GMT";
char *date_3 = "Sun Nov  6 08:49:37 1994";
char *date_4 = "Sunday, 06-Nov-03 08:49:37 GMT";

void
test_uri_parser(char *a_string);

void
test_date_parser(char *a_string);

int main(int argc, char **argv)
{

  char *l_buf = NULL;
  /* http_hdr_list *l_hdrs = NULL; */
  http_uri *l_uri = NULL;
  http_trans_conn *l_conn = NULL;
  http_req *l_req = NULL;
  http_resp *l_resp = NULL;
  int i = 0;
  int j = 0;
  int l_rv = 0;

  if (argc != 2)
    {
      printf("test <url>\n");
      exit(1);
    }
#if 0
  printf("*** testing parser ***\n");
  test_uri_parser("http://localhost/");
  test_uri_parser("http://odin.appliedtheory.com:1/");
  test_uri_parser("http://foobar/test.html");
  test_uri_parser("http:///");
  test_uri_parser("http://:80");
  test_uri_parser("http:/");
  test_uri_parser("http://foobar:/index.html");
  printf("*** testing hdrs ***\n");
  l_hdrs = http_hdr_list_new();
    http_hdr_set_value(l_hdrs,
		       http_hdr_Host,
		       "odin.appliedtheory.com");
    http_hdr_set_value(l_hdrs,
		       http_hdr_User_Agent,
		       "gnome-http-test-stub/1.0");
    http_hdr_set_value(l_hdrs,
		       http_hdr_Host,
		       "foobar.appliedtheory.com");
    http_hdr_clear_value(l_hdrs,
			 http_hdr_Host);
    http_hdr_set_value(l_hdrs,
    http_hdr_Host,
    "somewhere.appliedtheory.com");
    http_hdr_set_value(l_hdrs,
    http_hdr_User_Agent,
    "another-test-stub/2.0");
    /* this should overwrite the above setting */
  http_hdr_set_value(l_hdrs,
		     "user-agent",
		     "hi-mom/3.0");
  /* this should clear it */
  http_hdr_clear_value(l_hdrs,
		       http_hdr_User_Agent);
  /* use the nts code */
  http_hdr_set_value_no_nts(l_hdrs,
			    "user-agent-foo-bar",
			    10, /* just get the user-agent */
			    "12345678901234567890",
			    10); /* just get the first part */
  /* add this */
  http_hdr_set_value_no_nts(l_hdrs,
			    "some-random-hdr-ext-with-trailing-text",
			    19,
			    "a-random-value",
			    8);
  /* clear one of them */
  http_hdr_clear_value(l_hdrs,
		       "some-random-hdr-ext");
  http_hdr_list_destroy(l_hdrs);
  printf("*** testing date parsing ***\n");
  test_date_parser(date_1);
  test_date_parser(date_2);
  test_date_parser(date_3);
  test_date_parser(date_4);
  exit(0);
#endif
  l_uri = http_uri_new();
  if (http_uri_parse(argv[1], l_uri) < 0)
    {
      printf("Invalid URI.\n");
      exit(1);
    }
  printf("*** testing transport ***\n");
  l_conn = http_trans_conn_new();
  l_conn->host = l_uri->host;
  l_conn->port = l_uri->port;
  l_conn->sync = HTTP_TRANS_SYNC;
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
  for (j=0; j < 5; j++)
    {
      printf("*** sending request ***\n");
      l_req = http_req_new();
      l_req->type = http_req_type_get;
      l_req->host = l_conn->host;
      l_req->resource = l_uri->resource;
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
      /*printf("%s", l_buf);*/
      free(l_buf);
      if (! http_trans_conn_connected (l_conn))
	{
	  printf("Connection has been closed.\n");
	  exit(1);
	}
      /* http_trans_buf_clean(l_conn); */
      http_resp_destroy(l_resp);
      http_req_destroy(l_req);
    }
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
