#include <tsm/tsm.h>

int
main( int argc, char **argv )
{
  char *url = "http://lomond.ai.sri.com/wms?layer=oh.oi";
  char *name = tsmWMSExtractParam( url, "layer" );
  uchar *buffer;
  FILE *fp;

  tsmWMS *wms = tsmWMSCreate( url );

  Pyramid *tspec = (Pyramid *) tsmReadTspec_wms( url, NULL );

  TsmConnection connect;

  tsmWMSGetCapabilities( wms );

  connect = tsmConnect( url, "r", NULL );

  if ( ! tsmAllocReadTile( connect, 1, 1, 6, &buffer ) ) {
    fprintf( stderr, "Error reading tile from WMS!\n" );
    exit( 1 );
  }

  if ( ( fp = fopen( "tile.ppm", "w" ) ) ) {
    fprintf( fp, "P6\n128 128\n255\n" );
    fwrite( buffer, 3, 128*128, fp );
    fclose( fp );
    fprintf( stderr, "Tile written to tile.ppm\n" );
  }

  tsmDisconnect( connect );

  return 0;
}
