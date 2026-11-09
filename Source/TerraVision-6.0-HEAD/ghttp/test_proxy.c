#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "ghttp.h"

int main(int argc, char **argv)
{

  ghttp_request *l_req = NULL;

  l_req = ghttp_request_new();
  ghttp_set_uri(l_req, "ftp://getmeouttahere/pub/test.txt");
  ghttp_set_proxy(l_req, "http://get:3128/");
  ghttp_prepare(l_req);
  if (ghttp_process(l_req) == ghttp_error) {
    printf("request failed.\n");
  } else {
    printf("%s", ghttp_get_body(l_req));
  }
}


