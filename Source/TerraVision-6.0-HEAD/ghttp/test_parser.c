#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "http_uri.h"

void
test_uri_parser(char *a_string);

int main(int argc, char **argv)
{

  printf("*** testing parser ***\n");
  test_uri_parser("http://localhost/");
  test_uri_parser("http://odin.appliedtheory.com:1/");
  test_uri_parser("http://foobar/test.html");
  test_uri_parser("http:///");
  test_uri_parser("http://:80");
  test_uri_parser("http:/");
  test_uri_parser("http://foobar:/index.html");
  test_uri_parser("http://test");
  test_uri_parser("http://test:62");
}

void
test_uri_parser(char *a_string) {
  
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
