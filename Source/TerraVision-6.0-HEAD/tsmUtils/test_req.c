#include "config.h"

#include <prthread.h>

#include <tsm/tsm.h>
#include <tsm/tsmRequestSrv.h>

#include <stdlib.h>

#define APP_VERSION tsmCvsToString( "$Revision: 1.2 $" )

void usage( void )
{
  printf( "test_req - release %s\n", APP_VERSION );
  puts( "\nusage: test_req <pyramid-url> [...]\n" );
  puts( "This utility will time how long it takes to connect to a dataset" );
  puts( "and then how long it takes to read all of the tiles in the dataset" );
  puts( "as a whole, or from a single level of the dataset.\n" );
  puts( " -level <n> = read only this level of tiles from dataset" );
  puts( " -ignore_errors = keeping reading tiles even if error occurs" );
  tsmDescribeArgs( TSM_ARG_INIT | TSM_ARG_READ );
  exit( 0 );
}

TsmConnection conn;
Pyramid const *tspec;
tsmRequestServer *srv;

void requestor (void *v);

int
main (int argc, char *argv[])
{
  TsmConnectionParams params;

  int i;
  PRThread *threads[5];
  
  srandom (15);

  pthr_trace_stream (stdout);
  pthr_trace_html (FALSE);
  pthr_trace_print_pid (TRUE);
  pthr_trace_level (2);
  pthr_trace_mask (TSMMASK);

  if ( argc < 2 ) usage();

  params = tsmParseReadArgs (argc, argv);
  conn = tsmConnect (argv[1], "r", params);  
  tspec = (Pyramid const *) tsmGet (conn, TSM_DATASET_TSPEC );

  srv = tsmReqServerCreate (conn);

  for (i = 0; i < 5; ++i) {
    threads[i] = PR_CreateThread (PR_USER_THREAD, 
				  requestor, (void*)i,
				  PR_PRIORITY_NORMAL, PR_GLOBAL_THREAD,
				  PR_JOINABLE_THREAD, 1<<16);
  }

  for (i = 0; i < 5; ++i)
    PR_JoinThread (threads[i]);
  
  tsmReqServerDestroy (srv);

  return 0;
}

void
requestor (void *v)
{
  static char const funcName[] = "requestor";
  int n = (int) v;
  tsmRequestList *req;
  int res = tspec->maxLevel - n;
  SimpleTileSet const *lev = &tspec->tspecLevel.val[res];
  int i;
  int x = lev->minTile[0];
  int y = lev->minTile[1];

  TSMTRACE ((TSMMASK|1, funcName, "srv %p", srv));
  req = tsmReqServerCreateList (srv);
  TSMTRACE ((TSMMASK|1, funcName, "req %p", req));

  for (i = 0; i < 128; ) {
    tsmRequestAdd (req, x, y, res, (random()>>4)%4);

    /* Move to next tile. */
    ++i;
    if (++x >= lev->maxTile[0]) {
      x = lev->minTile[0];
      if (++y >= lev->maxTile[1]) {
	y = lev->minTile[1];
	if (--res < tspec->minLevel)
	  break;
	lev = &tspec->tspecLevel.val[res];
      }
    }
  }

  tsmRequestSubmit (req);
  sleep (10);

  tsmRequestDestroy (req);
}
