#include <stdio.h>
#include "../../libxml/nanohttp.h"

#define URL "http://www.ai.sri.com/~reddy/tmp.html"

int
main( int argc, char ** argv )
{
  char *content = "";

  xmlNanoHTTPFetch( URL, "download.html", &content );

  printf( "content-type: %s\n", content );

  return 0;
}

