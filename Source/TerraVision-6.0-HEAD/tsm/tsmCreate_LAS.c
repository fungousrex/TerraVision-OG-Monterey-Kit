/*
 * Name: tsmCreate_LAS.c
 *
 * Description: 
 *    This file contains functions for generating tiled terrain data
 *    from input DEM and OI images that are in the LAS format used
 *    by the USGS Eros Data Center (EDC). These functions are accessed
 *    by the user level functions in tsmCreate.c
 *
 * Function List:
 *    tsmImg2Tile() - create a level of OI image tiles 
 *    tsmDem2Tile() - create a level of DEM tiles
 *    tsmDownSample() - Gaussian filter and down sample an image
 *
 * Revision History:
 * 
 *   $Id: tsmCreate_LAS.c,v 1.15 2000/08/28 22:15:34 reddy Exp $
 *
 * License:
 *   The contents of this file are subject to the Open Source Apache
 *   Software License Version 1.1 (the "License"); you may not use
 *   this file except in compliance with the License. You may obtain a
 *   copy of the License at http://www.tsmapi/license.shtml.
 *
 *   Software distributed under the License is distributed on an "AS IS"
 *   basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 *   License for the specific language governing rights and limitations
 *   under the License.
 *
 *   Portions are Copyright (c) SRI International, 1998-2000.
 *
 */

#include "tsmLAS_IO.h"
#include "tsmInternal.h"

/* Define "constants" */

#define TSM_EZ_BYTE      1
#define TSM_EZ_SHORT     2
#define TSM_EZ_LONG      3
#define TSM_EZ_FLOAT     4
#define TSM_EZ_DOUBLE    5
#define TSM_EZ_COMPLEX   6
#define TSM_EZ_DCOMPLEX  7

/* valid data types */

#define TSM_EZ_EBYTE   1
#define TSM_EZ_EWORD   2
#define TSM_EZ_ELONG   3
#define TSM_EZ_EREAL   4
#define TSM_EZ_EDOUBLE 5
#define TSM_EZ_ECHAR   6
#define TSM_EZ_EUWORD  7
#define TSM_EZ_EULONG  8
#define TSM_EZ_NDTYPES 8

extern int errno;

/***************** Close down processing & exit *****************************/

void
tsmLAS_Finish( FILE * in_ptr, FILE *out_ptr, char *out_img_name, long out_nl,
	       long out_ns, long out_nb, long dtype, double out_min,
	       double out_max )
{
  char text[81];
  char command[161];
  char type[80];
  char prtfile[512];
  
  fclose (in_ptr);
  fclose (out_ptr);

  /* Place ddr and minmax information in a text
     file that mimics a dspddr print file
     ---------------------------------------------*/

  strcpy( prtfile, out_img_name );
  tsmAddFilenameExtension( prtfile, ".prt" );
  
  if (dtype == 1)
    strcpy (type, "BYTE");
  else if (dtype == 2)
    strcpy (type, "INTEGER*2");
  else
    strcpy (type, "UNKNOWN");

  sprintf(text, "NL:%ld NS:%ld NB:%ld DTYPE:%s", out_nl, out_ns, out_nb, type); 
  sprintf(command, "echo %s > %s", text, prtfile);
  system (command);

  sprintf(text, "MINIMUM:%f", out_min); 
  sprintf(command, "echo %s >> %s", text, prtfile);
  system (command);

  sprintf(text, "MAXIMUM:%f", out_max); 
  sprintf(command, "echo %s >> %s", text, prtfile);
  system (command);
}

/***************** Allocate buffer space ***********************************/

unsigned char *
tsmLAS_InitBuf( long length, long dtype )
{
  const char *funcName = "tsmLAS_InitBuf";
  long size;
  unsigned char *buffer;

  if (dtype == TSM_EZ_SHORT)
    size = length * sizeof(short);
  else if (dtype == TSM_EZ_LONG)
    size = length * sizeof(long);
  else if (dtype == TSM_EZ_FLOAT)
    size = length * sizeof(float);
  else if (dtype == TSM_EZ_DOUBLE)
    size = length * sizeof(double);
  else if (dtype == TSM_EZ_COMPLEX)
    size = length * sizeof(float) * 2;
  else if (dtype == TSM_EZ_DCOMPLEX)
    size = length * sizeof(double) * 2;
  else
    size = length; /* dtype == TSM_EZ_BYTE */

  if ( ( buffer = (unsigned char *) malloc(size) ) == NULL ) {
    tsmError( funcName, "Error %d allocating memory", errno );
    return NULL;
  }

  return buffer;
}

/***************** Get an image line ***************************************/

int
tsmLAS_GetLine( FILE *in_ptr, unsigned char *buf, long ns )
{
  const char *funcName = "tsmLAS_GetLine";

  if ( fread( buf, 1, ns, in_ptr ) != ns ) {
    tsmError( funcName, "Error %d reading image", errno );
    return FALSE;
  }  
  return TRUE;
}

/***************** Get a DEM line    ***************************************/

int
tsmLAS_GetDemLine( FILE *in_ptr, short *buf, long ns )
{
  const char *funcName = "tsmLAS_GetDemLine";

  if ( fread(buf, sizeof(short), ns, in_ptr) != ns ) {
    tsmError( funcName, "Error %d reading image", errno );
    return FALSE;
  }  
  return TRUE;
}

/***************** Put an image line ***************************************/

int
tsmLAS_PutLine( FILE *out_ptr, unsigned char *buf, long ns )
{
  const char *funcName = "tsmLAS_PutLine";

  if ( fwrite (buf, 1, ns, out_ptr) != ns ) {
    tsmError( funcName, "Error %d writing image", errno );
    return FALSE;
  }
  return TRUE;
}

/***************** Put a DEM line ***************************************/

int
tsmLAS_PutDemLine( FILE *out_ptr, short *buf, long ns )
{
  const char *funcName = "tsmLAS_PutDemLine";

  if ( fwrite (buf, sizeof(short), ns, out_ptr) != ns ) {
    tsmError( funcName, "Error %d writing image", errno );
    return FALSE;
  }
  return TRUE;
}

/***************** Open the output image ************************************/

int
tsmLAS_InitOutImg( char *out_name, FILE **out_ptr, long nl, long ns,
		   long nb, long odtype )
{
  const char *funcName = "tsmLAS_InitOutImg";
  char out_img_name[257];
  long otype;

  /* Set otype
     ---------*/
  if (odtype == TSM_EZ_SHORT)
    otype = TSM_EZ_EWORD;
  else if (odtype == TSM_EZ_LONG)
    otype = TSM_EZ_ELONG;
  else if (odtype == TSM_EZ_FLOAT)
    otype = TSM_EZ_EREAL;
  else if (odtype == TSM_EZ_DOUBLE) {
    otype = TSM_EZ_EREAL;
    ns *= 2;
  } else if (odtype == TSM_EZ_COMPLEX) {
    otype = TSM_EZ_EREAL;
    ns *= 2;
  } else if (odtype == TSM_EZ_DCOMPLEX) {
    otype = TSM_EZ_EREAL;
    ns *= 4;
  } else
    otype = TSM_EZ_EBYTE; /* odtype == TSM_EZ_BYTE */
    
  /* Open the output image
     ---------------------*/
  strcpy (out_img_name, out_name);
  if ( strcmp( (char *) (out_name + strlen(out_name) - 4), ".img" ) != 0 )
    strcat (out_img_name, ".img");

  if ( ( *out_ptr =  fopen (out_img_name, "w") ) == NULL ) {
    tsmError(funcName,"Error %d opening output file %s",errno, out_img_name);
    return FALSE;
  }

  return TRUE;
}

/***************** Open the input image ************************************/

int
tsmLAS_InitInImg( char *img_name, FILE **in_ptr,
		  long nl, long ns, long nb, long dtype )
{
  const char *funcName = "tsmLAS_InitInImg";
  char in_img_name[257];

  /* Get input file information
     -----------------------------*/

  strcpy (in_img_name, img_name);
  if ( strcmp( (char *) (img_name + strlen(img_name) - 4), ".img" ) != 0 )
    strcat (in_img_name,".img");

  if ( ( *in_ptr = fopen (in_img_name, "r") ) == NULL ) {
    tsmError( funcName, "Error %d opening input image file %s", 
	      errno, in_img_name);
    return FALSE;
  }

  return TRUE;
}


/*
 * Synopsis:
 *     int tsmCreateLevelTiles_LASRGB( TsmConnection connect,
 *                                     const char *imgfile,
 *                                     TsmTileParams params, int level )
 *
 * Description:
 *     Tile a source LAS image into an ortho-image (OI) mosaic.
 *      
 *     Takes a LAS format raster BSQ image and chops it up into
 *     a collection of 128 x 128 tiles for use by TerraVision. The tiles
 *     can be generated in either raw lrbt-raster format or JPEG compressed
 *     (depending upon the connection's write settings). The image
 *     file must reside on localdisk.
 *
 *     All of the details about the image (e.g. width, height, number of
 *     components per pixel, etc.) should be provided in the params
 *     data structure - refer to the documentation on TsmTileParams for
 *     a list of all the fields that you can set (N.B. you should always
 *     initialise a TsmTileParams structure with tsmInitTileParams).
 *
 *     This function operates on a rectangular block of the image
 *     by specifying the start tile coordinate (col,row) and the
 *     size of the block horizontally and vertically. You can specify
 *     the entire image by giving a coord of (0,0) and block_size <= 0.
 *     If you specify a tile block size that encompasses tiles outwith
 *     the source image, then the block size is clipped appropriately.
 *     E.g. a block_size of 5 will attempt to write 25 128x128 tiles.
 *
 *     N.B. if you are writing tiles to a local disk, then you should
 *     convert the entire image in a oner (as described above) because
 *     the TIFF file can only be written to in a specific order.
 *
 * @param    connect a valid connection handle, as returned by tsmConnect
 * @param    image_name the (localdisk) name of the input image.
 * @param    params a TsmTileParams structure defining all about the tileset
 * @param    level the level of the pyramid to be created
 *
 * @return   the number of tiles written (i.e. 0 means total failure)
 *
 * @author   Martin Reddy, based on code by Brian Davis
 * @version  16 July 1997
 *
 * @see      tsmCreateLevelTiles_LASELEV, tsmCreateLevelTiles
 *
 */

int
tsmCreateLevelTiles_LASRGB( TsmConnection connect, const char *imgfile,
			    TsmTileParams params, int level )
{
  const char    *funcName = "tsmCreateLevelTiles_LASRGB";
  PRFileDesc    *ifp;
  int           p;
  int           bnd_sz, cnt;
  int           r, c, b, l, ll, s, ss;
  int           extra_lines, non_extra_samps, non_pad_lines;
  int           saved_extra_lines, img_end, curr_tile_row;
  int           pad, sav_pad;
  int           stop_col, stop_row, tiles_written = 0;
  int           cols, rows;
  unsigned char tbuf[128][128*3];
  unsigned char *ibuf;
  extern int    errno;
  long          tot_cnt = 0;

  int           nl = params.height;
  int           ns = params.width;
  int           nb = params.comps;
  int           start_col = params.start_col;
  int           start_row = params.start_row;
  int           block_size = params.block_size;

  /* work out the number of tiles horiztonally and vertically */

  cols = ( ns / 128 ) + ( ns % 128 > 0 );
  rows = ( nl / 128 ) + ( nl % 128 > 0 );
  bnd_sz = nl * ns;

  saved_extra_lines = 128 - ( nl % 128 );
  non_extra_samps   = ns % 128;

  /* Work out the row and column range to operate over */

  if ( block_size > 0 ) {
    stop_col = min( start_col + block_size - 1, cols - 1 );
    stop_row = min( start_row + block_size - 1, rows - 1 );
  } else {
    stop_col = cols - 1;
    stop_row = rows - 1;
  }

  /* We need to flip the row range because the image file is in top-left */
  /* origin format, whilst the row range just now assumes the bot-left   */
  /* orientation used by TerraVision. Beforehand however, we save the    */
  /* value of start_row for use when writing out the TerraVision tiles   */

  curr_tile_row = start_row;

  stop_row = rows - stop_row - 1;
  start_row = rows - start_row - 1;

  /* Get the tspec structure for the dataset we are to create.  */
  /* It is an error if this doesn't exist because this contains */
  /* all of the info such as tile size, overlap pixels, etc.    */

  if ( (Pyramid *) tsmGet( connect, TSM_DATASET_TSPEC ) == NULL){
    tsmError( funcName, "Dataset has no tspec defined yet - cannot continue" );
    return tiles_written;
  }

  /* Allocate a buffer for data from the input image, and open the file */

  if ( ( ibuf = (unsigned char *) tsmMemMalloc( nb * 128 * ns ) ) == NULL ) {
    tsmError( funcName, "malloc error\n" );
    return tiles_written;
  }

  if ( ( ifp = PR_Open ( imgfile, PR_RDONLY, 0 ) ) != 0 ) {
    tsmError( funcName, "Error %d opening image file %s\n", errno, imgfile );
    tsmMemManagerFree( ibuf );
    return tiles_written;
  }

  /* read each tile from the image and output it to the pyramid */

  for ( r = start_row; r >= stop_row; r-- ) {       /* go through each row */

    p = ( rows <= 1 ) ? 100 : 100 - ( r * 100 ) / ( rows - 1 );
    tsmProgressReport( connect, "Creating level %d tiles... (%d%%)", level,p );
    
    for ( b = 0; b < nb; b++ ) {                 /* go through all 3 bands */

      /* pad out the extra lines at the top of the image for row 0 only */

      if ( r == 0 ) {
        for ( l = 0; l < saved_extra_lines; l++ ) {      /* do the padding */
	  memset( ibuf + (b * 128 * ns) + (l * ns), 0, ns );
	}
	extra_lines = saved_extra_lines;
	non_pad_lines = 128 - saved_extra_lines;
	sav_pad = 0;
	pad = extra_lines * ns;
      } else {
	extra_lines = 0;
	non_pad_lines = 128;
	sav_pad = saved_extra_lines * ns;
	pad = 0;
      }

      /* now seek to appropriate part of the image file and then read in */
      /* in the entire row of data (0..cols) all at once.                */

      if ( PR_Seek (ifp, (b * bnd_sz)+(r * 128 * ns)-sav_pad, PR_SEEK_SET) < 0 ) {
	tsmError( funcName, "Error %d seeking for input image read\n", errno );
	tsmMemManagerFree( ibuf );
	PR_Close( ifp );
	return tiles_written;
      }
      for (ll = 0,l = 0; ll < non_pad_lines; ll++,l++) {
	cnt = PR_Read( ifp, ibuf + ((b * 128 * ns) + (l * ns) + pad), ns );
	if ( cnt != ns ) {
	  printf( "Error %d reading input image line %d\n", errno, l+1 );
	  printf( "r=%d b=%d cnt=%d ll=%d extra_lines=%d non_pad_lines=%d\n",
		  r, b, cnt, ll, extra_lines, non_pad_lines );
	  printf( "pad=%d sav_pad=%d\n", pad ,sav_pad );
	  printf( "Total lines read before this attempt=%ld\n", tot_cnt );
	  tsmMemManagerFree( ibuf );
	  PR_Close( ifp );
	  return tiles_written;
	}
	tot_cnt++;
      }
    }

    /* Go through each column of data and output the entire tile in  */
    /* each case. N.B. this loop will automatically pad tiles on the */
    /* right-most column if the image is not a multiple of tile size */

    for (c = start_col; c <= stop_col; c++) {        /* go through each col */

      /* Rearrange order to lrbt format and then write the tile. Will */
      /* pad any pixels beyond the edge of the input image with zeros */

      img_end = ( c == cols-1 ) ? non_extra_samps : 128;
      for (l = 0, ll = 127; l < 128; l++, ll--) {
	for (s = 0, ss = 0; s < img_end; s++, ss+=3) {
	  for (b = 0; b < nb; b++)
	    tbuf[ll][ss + b] = *(ibuf+(b*128*ns)+(l*ns)+(c*128) + s);
	}
	for (s = img_end; s < 128; s++, ss+=3) {
	  for (b = 0; b < nb; b++)
	    tbuf[ll][ss + b] = 0;
	}
      }

      /* write the tile and increment the tile count if successful */

      if ( tsmWriteTile( connect, c, curr_tile_row, level, (uchar *) tbuf ) ) {
	tiles_written++;

	/* calculate min/max and checksum, and merge info into tspec */

	tsmUpdateTileStats( connect, (uchar *) tbuf, level );
      }
    }

    curr_tile_row++;
  }

  /* All done: now wash your hands... */

  tsmMemManagerFree( ibuf );
  PR_Close( ifp );
  tsmProgressReport( connect, "" );
  return tiles_written;
}

/*
 * Synopsis:
 *  int tsmCreateLevelTiles_LASELEV( TsmConnection connect,
 *                                   const char *imgfile,
 *                                   TsmTileParams params, int level )
 *
 * Description:
 *     Tile source elevation data into an DEM mosaic.
 *      
 *     Takes a LAS format raster BSQ image and chops it up into
 *     a collection of 32 x 32 tiles for use by TerraVision. The tiles
 *     can be generated in either raw lrbt-raster format or JPEG compressed.
 *     (depending upon the connection's write settings). The image
 *     file must reside on localdisk.
 *
 *     All of the details about the image (e.g. width, height, number of
 *     components per pixel, etc.) should be provided in the params
 *     data structure - refer to the documentation on TsmTileParams for
 *     a list of all the fields that you can set (N.B. you should always
 *     initialise a TsmTileParams structure with tsmInitTileParams).
 *
 * @param    connect a valid connection handle, as returned by tsmConnect
 * @param    image_name the (localdisk) name of the input image.
 * @param    params a TsmTileParams structure defining all about the tileset
 * @param    level the level of the pyramid to be created
 *
 * @return   the number of tiles written (i.e. 0 means total failure) 
 *
 * @author   Martin Reddy
 * @version  22 July 1997
 *
 * @see      tsmCreateLevelTiles_LAS, tsmCreateLevelTiles
 *
 */

int
tsmCreateLevelTiles_LASELEV( TsmConnection connect, const char *imgfile,
			     TsmTileParams params, int level )
{
  static const char *funcName = "tsmCreateLevelTiles_LASELEV";
  FILE              *handle;
  int               status;

  if ( ( handle = fopen( imgfile, "r" ) ) == NULL ) {
    tsmError( funcName, "Couldn't open raw image file: %s", imgfile );
    return FALSE;
  }

  status = tsmCreateLevelTiles_generic( connect, level, handle, 0,
					params.width, params.height,
					params.tile_width, params.tile_height,
					params.tile_overlap_x,
					params.tile_overlap_y,
					2 /* 2 bytes = 16-bit DEMs */,
					FALSE, params.sea_level, params.scale,
					params.offset );
  fclose( handle );
  return status;
}

/*
 * Synopsis:
 *  int tsmCreateLevelTiles_LASGREY( TsmConnection connect,
 *                                   const char *imgfile,
 *                                   TsmTileParams params, int level )
 *
 * Description:
 *     Tile source greyscale imagery data into an OI mosaic.
 *      
 *     Takes a LAS format raster BSQ image and chops it up into
 *     a collection of 128 x 128 tiles for use by TerraVision. The tiles
 *     can be generated in either raw lrbt-raster format or JPEG compressed.
 *     (depending upon the connection's write settings). The image
 *     file must reside on localdisk.
 *
 *     All of the details about the image (e.g. width, height, number of
 *     components per pixel, etc.) should be provided in the params
 *     data structure - refer to the documentation on TsmTileParams for
 *     a list of all the fields that you can set (N.B. you should always
 *     initialise a TsmTileParams structure with tsmInitTileParams).
 *
 * @param    connect a valid connection handle, as returned by tsmConnect
 * @param    image_name the (localdisk) name of the input image.
 * @param    params a TsmTileParams structure defining all about the tileset
 * @param    level the level of the pyramid to be created
 *
 * @return   the number of tiles written (i.e. 0 means total failure) 
 *
 * @author   Martin Reddy
 * @version  1 July 1999
 *
 * @see      tsmCreateLevelTiles_LAS, tsmCreateLevelTiles
 *
 */

int
tsmCreateLevelTiles_LASGREY( TsmConnection connect, const char *imgfile,
			     TsmTileParams params, int level )
{
  static const char *funcName = "tsmCreateLevelTiles_LASGREY";
  FILE              *handle;
  int               status;

  if ( ( handle = fopen( imgfile, "r" ) ) == NULL ) {
    tsmError( funcName, "Couldn't open raw image file: %s", imgfile );
    return FALSE;
  }

  status = tsmCreateLevelTiles_generic( connect, level, handle, 0,
					params.width, params.height,
					params.tile_width, params.tile_height,
					0, 0, 1, FALSE,
					params.sea_level, params.scale,
					params.offset );
  fclose( handle );
  return status;
}

/*
 * Synopsis:
 *      void filterd2( unsigned char *, int, float *, float * )
 *      void filterd2_short( short *, int, float *, float * )
 *
 * Description:
 *      Functions to apply 5 point convolution in the x dimension and
 *      downsample results by two. Parameters = input buffer (in), number
 *      of samples output (ns), weight vector (w), results of convolution
 *      and downsampling (out). Both of these functions are used by
 *      tsmDownSample() below, to downsample image and DEM files respectively
 *
 * Externals: None
 *
 * Author: Brian Davis. (minor stylistic changes made by Martin Reddy)
 *
 * Date: 22 July 1997
 *
 */

static void
filterd2( unsigned char *in, int ns, float *w, float *out )
{
  int i, j;

  for ( i = 1, j = 2; i < (ns - 1); j += 2, i++ )
    out[i] = w[0] * in[j-2] + w[1] * in[j-1] + w[2] * in[j] + w[3] *
             in [j+1] + w[4] * in[j+2];

  out[0]    = out[1];
  out[ns-1] = out[ns-2];
}

static void
filterd2_short( short *in, int ns, float *w, float *out )
{
  int i, j;

  for ( i = 1, j = 2; i < (ns - 1); j += 2, i++ ) 
    out[i] = w[0] * in[j-2] + w[1] * in[j-1] + w[2] * in[j] + w[3] *
             in[j+1] + w[4] * in[j+2];

  out[0]    = out[1];
  out[ns-1] = out[ns-2];
}

/*
 * Synopsis:
 *     int tsmDownSample_LAS( const char *in_name, const char *out_name,
 *                            TsmTileParams params )
 *
 * Description:
 *   Create a half-sized version of a source image.
 *      
 *   Down samples an image by two (Used to create successive Pyramid mosaics).
 *   The input and output filenames on local disk are supplied.
 *
 *   All of the details about the input image (e.g. width, height, number of
 *   components per pixel, etc.) should be provided in the params
 *   data structure - refer to the documentation on TsmTileParams for
 *   a list of all the fields that you can set (N.B. you should always
 *   initialise a TsmTileParams structure with tsmInitTileParams).
 *
 *   The caller can select whether they want the image to be downsampled
 *   using a gaussian filter (smoother) or a simple point sampling
 *   technique (take each 2nd pixel). These are specified by setting the
 *   filtering field of the params structure to one of TSM_GAUSSIAN_FILTER
 *   or TSM_POINT_SAMPLE.
 *
 * @param    in_name    filename of input image
 * @param    out_name   filename of output downsampled image
 * @param    params a TsmTileParams structure describing the input image
 *
 * @return   TRUE if output image written okay, FALSE if operation failed
 *
 * @author   Martin Reddy, based on code by Brian Davis
 * @version  22 September 1997
 *
 * @see      tsmDownSample, tsmCreateLevelTiles
 *
 */

int
tsmDownSample_LAS( TsmConnection connect, const char *in_name,
		   const char *out_name, TsmTileParams params, int in_level )
{
  const char *funcName = "tsmDownSample_LAS";
  int        data_type;
  double     min, max;
  
  switch ( params.dataset_type ) {
  case TSM_DEM_DATASET:
    data_type = TSM_EZ_SHORT;  /* INTEGER*2 */
    min       = 0.0;
    max       = 32767.0;
    break;
  case TSM_IMAGE_DATASET:
    data_type = TSM_EZ_BYTE;   /* BYTE */
    min       = 0.0;
    max       = 255.0;
    break;
  default:
    tsmError( funcName, "Unknown dataset type (%d)", params.dataset_type );
    return FALSE;
  }

  tsmProgressReport( connect, "Downsampling level %d image...", in_level );

  switch ( params.filtering ) {
  case TSM_GAUSSIAN_FILTER:
    return tsmDownSample_LAS_gaussian( connect, (char *) in_name,
				       (char *) out_name, params.height,
				       params.width, params.comps, data_type,
				       min, max, in_level );
  case TSM_POINT_SAMPLE:
    return tsmDownSample_LAS_psample( connect, (char *) in_name,
				      (char *) out_name, params.height,
				      params.width, params.comps, data_type,
				      min, max, in_level );
  default:
    tsmError( funcName, "invalid filtering mode (%d)", params.filtering );
    return FALSE;
  }
}

/*
 * Synopsis:
 *      int tsmDownSample_LAS_psample( char *, char *, int, int,
 *                                     int, int, double, double )
 *
 * Description:
 *      Down samples an image by two (Used to create a Gaussian Pyramid).
 *      The input and output filenames on local disk are supplied, aint 
 *      with the image dimensions (nl,ns,nb) and data type (1=BYTE, or
 *      2=INTEGER*2: see tsmLAS_IO.h). The valid min/max are specified also.
 *
 *      The result is an image file written to out_name that is half the
 *      size of in_name, having undergone a simple point sampling process
 *      where each second pixel is taken from the input image.
 *
 * Externals: None
 *
 * Returns: TRUE if output image written okay, FALSE if operation failed
 *
 * Author: Martin Reddy, based around code written by Brian Davis
 *
 * Date: 22 September 1997
 *
 */

int
tsmDownSample_LAS_psample( TsmConnection connect, char *in_name,
			   char *out_name, int in_nl, int in_ns, int in_nb,
			   int data_type, double min, double max, int in_level)
{
  const char *funcName = "tsmDownSample_LAS_psample";
  int  out_nl, out_ns;               /* Number of lines & samples in output */
  int  line,                         /* line counter */
       band,                         /* band index */
       i, j;                         /* Loop counters */

  unsigned char *inbuf, *outbuf;     /* Input and Output buffers */
	
  short *short_buf, *out_short_buf;  /* DEM Input and Output buffers */

  FILE *in_ptr;                      /* input file pointer */
  FILE *out_ptr;                     /* output file pointer */

  /* Initialize processing, images, and buffer space */

  if ( ! tsmLAS_InitInImg(in_name, &in_ptr, in_nl, in_ns, in_nb, data_type) ) {
    tsmError( funcName, "Error initialising input image\n" );
    return FALSE;
  }
  out_nl = in_nl / 2;
  out_ns = in_ns / 2;

  if ( ! tsmLAS_InitOutImg(out_name,&out_ptr,out_nl,out_ns,in_nb,data_type) ) {
    tsmError( funcName, "Error initialising output image\n" );
    return FALSE;
  }

  /* do the point-sampling downsample for the BYTE (OI) data */

  if ( data_type == TSM_EZ_EBYTE ) {
    inbuf  = (unsigned char *) tsmLAS_InitBuf( in_ns, data_type );
    outbuf = (unsigned char *) tsmLAS_InitBuf( out_ns, data_type );

    for ( band = 0; band < in_nb; band++ ) {

      for ( line = 0; line < out_nl; line++ ) {

	/* read in a single line from the source image and create a new */
	/* line that is half the size, by sampling each second pixel    */

	if ( tsmLAS_GetLine( in_ptr, inbuf, in_ns ) == FALSE ) {
	  tsmError( funcName, "Error returned from tsmLAS_GetLine\n" );
	  return FALSE;
	}

	for ( i = j = 0; i < out_ns; i++, j+=2 ) {
	  outbuf[i] = inbuf[j];
	}

	/* write out this downsampled line and skip the next line in the */
	/* source image so that we only sample from each second line.    */

	if ( tsmLAS_PutLine( out_ptr, outbuf, out_ns ) == FALSE ) {
	  tsmError( funcName, "Error returned from tsmLAS_PutLine\n" );
	  return FALSE;
	}

	if ( tsmLAS_GetLine( in_ptr, inbuf, in_ns ) == FALSE ) {
	  tsmError( funcName, "Error returned from tsmLAS_GetLine\n" );
	  return FALSE;
	}

      }
    }

  } else {

    /* do the point-sampling downsample for the INTEGER (DEM) data */

    short_buf     = (short *) tsmLAS_InitBuf( in_ns, data_type );
    out_short_buf = (short *) tsmLAS_InitBuf( out_ns, data_type );

    for ( band = 0; band < in_nb; band++ ) {

      for ( line = 0; line < out_nl; line++ ) {

	/* read in a single line from the source image and create a new */
	/* line that is half the size, by sampling each second pixel    */
	
	if ( tsmLAS_GetDemLine( in_ptr, short_buf, in_ns ) == FALSE ) {
	  tsmError( funcName, "Error returned from tsmLAS_GetDemLine\n" );
	  return FALSE;
	}

	for ( i = j = 0; i < out_ns; i++, j+=2 ) {
	  out_short_buf[i] = short_buf[j];
	}

	/* write out this downsampled line and skip the next line in the */
	/* source image so that we only sample from each second line.    */

	if ( tsmLAS_PutDemLine( out_ptr, out_short_buf, out_ns ) == FALSE ) {
	  tsmError( funcName, "Error returned from tsmLAS_PutDemLine\n" );
	  return FALSE;
	}

	if ( tsmLAS_GetDemLine( in_ptr, short_buf, in_ns ) == FALSE ) {
	  tsmError( funcName, "Error returned from tsmLAS_GetDemLine\n" );
	  return FALSE;
	}
      }
    }
  }

  /* Close down processing and exit */

  tsmLAS_Finish( in_ptr, out_ptr, out_name, out_nl, out_ns, in_nb, data_type,
		 min, max );
  return TRUE;
}

/*
 * Synopsis:
 *      int tsmDownSample_LAS_gaussian( char *, char *, int, int, int, int,
 *                                      double, double )
 *
 * Description:
 *      Down samples an image by two (Used to create a Gaussian Pyramid).
 *      The input and output filenames on local disk are supplied, aint 
 *      with the image dimensions (nl,ns,nb) and data type (1=BYTE, or
 *      2=INTEGER*2: see tsmLAS_IO.h). The valid min/max are specified also.
 *
 *      The result is an image file written to out_name that is half the
 *      size of in_name, having undergone a Guassian filtering process.
 *
 * Externals: None
 *
 * Returns: TRUE if output image written okay, FALSE if operation failed
 *
 * Revision History:
 *
 *  VERSION     DATE    AUTHOR      CODE/CONT 
 *  -------     ----    ------      ---------
 *    1.0       4/92   D. Steinwand
 *    1.1       5/96   B. Davis     SSD/SSB
 *                                  c_minmnax and c_upddr were commented out
 *                                  within finish() of ezio.c.
 *                                  updated init_inimg, init_outimg, get_line,
 *                                  and put_line to accommodate multi-band
 *                                  images, and their calls from main.
 *    1.2       10/96  B. Davis     SSD/SSB
 *                                  multi-band and I*2 were enabled.
 *    1.3       2/97   B. Davis     SSD/SSB
 *                                  eliminated LAS dependencies, C main from
 *                                  unix command line.
 *    1.4       7/97   M. Reddy     Made into a tsmApi function rather than 
 *                                  stand-alone prog. Minor stylistic changes.
 *                                  Corrected error with calc at image edge.
 */

int
tsmDownSample_LAS_gaussian( TsmConnection connect, char *in_name,
			    char *out_name, int in_nl, int in_ns, int in_nb,
			    int data_type, double min, double max,
			    int in_level )
{
  const char *funcName = "tsmDownSample_LAS_gaussian";
  int  out_nl, out_ns;               /* Number of lines & samples in output */
  int  line,                         /* line counter */
       band,                         /* band index */
       in_line,                      /* line index */
       i;                            /* Loop counters */

  float weightscale = 0.4;           /* A value for kernel construction */
  float *workbuf;                    /* Working buffer (results of x conv) */
  float w[5];

  unsigned char *inbuf, *outbuf;     /* Input and Output buffers */
	
  short *short_buf, *out_short_buf;  /* DEM Input and Output buffers */

  float *pm2, *pm1, *p0, *p1, *p2,
        *pm2tmp, *pm1tmp;            /* Pointers into workbuf */

  FILE *in_ptr;                        /* input file pointer */
  FILE *out_ptr;                     /* output file pointer */

  /* Initialize processing, images, and buffer space
     -----------------------------------------------*/

  if ( ! tsmLAS_InitInImg(in_name, &in_ptr, in_nl, in_ns, in_nb, data_type) ) {
    tsmError( funcName, "Error returned from tsmLAS_InitInImg" );
    return FALSE;
  }
  out_nl = in_nl / 2;
  out_ns = in_ns / 2;

  if ( ! tsmLAS_InitOutImg(out_name,&out_ptr,out_nl,out_ns,in_nb,data_type) ) {
    tsmError( funcName, "Error returned from tsmLAS_InitOutImg\n" );
    return FALSE;
  }

  if ( data_type == TSM_EZ_EBYTE ) {
    inbuf  = (unsigned char *) tsmLAS_InitBuf( in_ns, data_type );
    outbuf = (unsigned char *) tsmLAS_InitBuf( out_ns, data_type );
  } else if ( data_type == TSM_EZ_EWORD ) {
    short_buf     = (short *) tsmLAS_InitBuf( in_ns, data_type );
    out_short_buf = (short *) tsmLAS_InitBuf( out_ns, data_type );
  }

  workbuf = (float *) tsmLAS_InitBuf( out_ns * 5, TSM_EZ_FLOAT );

  pm2 = workbuf;
  pm1 = pm2 + out_ns;
  p0  = pm1 + out_ns;
  p1  = p0 + out_ns;
  p2  = p1 + out_ns;

  /* Setup convolution kernel and prime image buffers
     ------------------------------------------------*/
  w[2] = weightscale;
  w[3] = w[1] = 0.25;
  w[4] = w[0] = 0.25 - weightscale / 2.0;

  /* Loop thru the output image(s), line-by-line, pixel-by-pixel
     -----------------------------------------------------------*/

  if ( data_type == TSM_EZ_EBYTE ) {
    for ( band = 1; band <= in_nb; band++ ) {

      if ( tsmLAS_GetLine( in_ptr, inbuf, in_ns ) == FALSE )
	return FALSE;
      filterd2( inbuf, out_ns, w, pm2 );

      if ( tsmLAS_GetLine( in_ptr, inbuf, in_ns ) == FALSE )
	return FALSE;
      filterd2( inbuf, out_ns, w, pm1 );

      if ( tsmLAS_GetLine( in_ptr, inbuf, in_ns ) == FALSE )
	return FALSE;
      filterd2( inbuf, out_ns, w, p0 );

      /* p1 is a copy of pm1, and p2 of pm2, for 1st time */
      memcpy( p1, pm1, out_ns * sizeof(float) );
      memcpy( p2, pm2, out_ns * sizeof(float) );

      for ( i = 0; i < out_ns; i++ ) {
	outbuf[i] = w[0] * pm2[i] + w[1] * pm1[i] + w[2] * p0[i] +
	            w[3] * p1[i] + w[4] * p2[i];
      }

      /* no, make the first line a copy of the second, zeros be bad */
      /* Write 1st line */

      if ( tsmLAS_PutLine( out_ptr, outbuf, out_ns ) == FALSE )
	return FALSE;

      for ( line = 0, in_line = 4; line < out_nl - 2; line++, in_line += 2 ) {

	if ( tsmLAS_GetLine( in_ptr, inbuf, in_ns ) == FALSE )
	  return FALSE;
	filterd2( inbuf, out_ns, w, p1 );

	if ( tsmLAS_GetLine( in_ptr, inbuf, in_ns ) == FALSE )
	  return FALSE;
	filterd2( inbuf, out_ns, w, p2 );

	for ( i = 0; i < out_ns; i++ )
	  outbuf[i] = w[0] * pm2[i] + w[1] * pm1[i] + w[2] * p0[i] +
	              w[3] * p1[i] + w[4] * p2[i];

	/* Write data */

	if ( tsmLAS_PutLine( out_ptr, outbuf, out_ns ) == FALSE )
	  return FALSE;

	pm2tmp = pm2;
	pm1tmp = pm1;
	pm2    = p0;
	pm1    = p1;
	p0     = p2;
	p1     = pm2tmp;
	p2     = pm1tmp;
	
      }

      /* last line */

      if ( tsmLAS_PutLine( out_ptr, outbuf, out_ns ) == FALSE )
	return FALSE;

    }

  } else {

    for ( band = 1; band <= in_nb; band++ ) {

      if ( tsmLAS_GetDemLine( in_ptr, short_buf, in_ns ) == FALSE )
	return FALSE;
      filterd2_short( short_buf, out_ns, w, pm2 );

      if ( tsmLAS_GetDemLine( in_ptr, short_buf, in_ns ) == FALSE )
	return FALSE;
      filterd2_short( short_buf, out_ns, w, pm1 );

      if ( tsmLAS_GetDemLine( in_ptr, short_buf, in_ns ) == FALSE )
	return FALSE;
      filterd2_short(short_buf, out_ns, w, p0);

      /* make the first line a copy of the second, not zeros */

      /* p1 is a copy of pm1, and p2 of pm2, for 1st time */
      memcpy( p1, pm1, out_ns * sizeof(float) );
      memcpy( p2, pm2, out_ns * sizeof(float) );

      for ( i = 0; i < out_ns; i++ ) {
	out_short_buf[i] = w[0] * pm2[i] + w[1] * pm1[i] + w[2] * p0[i] +
                           w[3] * p1[i] + w[4] * p2[i];
      }

      if ( tsmLAS_PutDemLine( out_ptr, out_short_buf, out_ns ) == FALSE )
	return FALSE;

      for ( line = 0, in_line = 4; line < out_nl-2; line++, in_line += 2 ) {
	if ( tsmLAS_GetDemLine( in_ptr, short_buf, in_ns ) == FALSE )
	  return FALSE;
	filterd2_short( short_buf, out_ns, w, p1 );

	if ( tsmLAS_GetDemLine( in_ptr, short_buf, in_ns ) == FALSE )
	  return FALSE;
	filterd2_short( short_buf, out_ns, w, p2 );
	
	for ( i = 0; i < out_ns; i++ ) {
	  out_short_buf[i] = w[0] * pm2[i] + w[1] * pm1[i] + w[2] * p0[i] +
	    w[3] * p1[i] + w[4] * p2[i];
	}
	
	if ( tsmLAS_PutDemLine( out_ptr, out_short_buf, out_ns ) == FALSE )
	  return FALSE;

	pm2tmp = pm2;
	pm1tmp = pm1;
	pm2    = p0;
	pm1    = p1;
	p0     = p2;
	p1     = pm2tmp;
	p2     = pm1tmp;
      }
      
      /* wrong, make the last line a copy of the previous */

      if ( tsmLAS_PutDemLine( out_ptr, out_short_buf, out_ns ) == FALSE ) 
	return FALSE;

    }
  }

  /* Close down processing and exit */

  tsmLAS_Finish( in_ptr, out_ptr, out_name, out_nl, out_ns, in_nb, data_type,
		 min, max );
  return TRUE;
}

/*
 * Synopsis:
 *  int tsmParseCreateParams_LAS( char *image_name, TsmTileParams *tile,
 *                                TsmTspecParams *tspec )
 *
 * Description:
 *   Tries to parse the .prt file that is associated with the .img
 *   file (the ortho-image data). From this we should be able to
 *   get the image's width and height, the number of bands, the
 *   resolution, and the UTM/LatLong georeferencing info.
 *
 * Author: Martin Reddy
 *
 * Date: 20 March 1998
 *
 */

int
tsmParseCreateParams_LAS( char *image_name, TsmTileParams *tile,
			  TsmTspecParams *tspec )
{
  static const char *funcName = "tsmParseCreateParams_LAS";
  char              prt_name[512];
  char              buffer[512];
  char              dummy[512];
  FILE              *handle;
  int               i, nl, ns, nb;
  double            distx = 1.0, disty = 1.0;

  TSMTRINC(( TSMMASK|2, 1 ));

  strcpy( prt_name, image_name );
  tsmAddFilenameExtension( prt_name, ".prt" );

  if ( ( handle = fopen( prt_name, "r" ) ) == NULL ) {
    tsmError( funcName, "Couldn't open .prt file: %s\n", prt_name );
    TSMTRINC(( TSMMASK|2, -1 ));
    return FALSE;
  }

  while ( feof(handle) == FALSE ) {
    fgets( buffer, 512, handle );

    /* Remove any colons from the string */

    for ( i = 0; i < strlen(buffer); ++i )
      if ( buffer[i] == ':' )
	buffer[i] = ' ';

    /* Read the data dimensions if found */

    if ( strstr( buffer, "NL" ) != NULL ) {
      sscanf( buffer, "%s%d%s%d%s%d", dummy, &nl, dummy, &ns, dummy, &nb );

      TSMTRACE(( TSMMASK|2, funcName, "PRT File: -width %d -height %d -comps %d",
		 ns, nl, nb ));

      tile->width  = tspec->width  = ns;
      tile->height = tspec->height = nl;
      tile->comps  = tspec->comps  = nb;
    }

    /* If we come across the projection info, ensure that it is UTM */

    else if ( strstr( buffer, "PROJ. CODE" ) != NULL ) {
      if ( strstr( buffer, "UTM" ) != NULL ) {
	tspec->coord_sys = TSM_UTM_COORDSYS;
      } else if ( strstr( buffer, "GEOGRAPHIC" ) != NULL ) {
	tspec->coord_sys = TSM_LATLONG_COORDSYS;
      } else {
	tsmError( funcName, "%s does not contain UTM or Lat/Long data.\n", 
		  prt_name );
	TSMTRINC(( TSMMASK|2, -1 ));
	return FALSE;
      } 
    }
    
    /* If we come across the projection distance, then grab it, but */
    /* don't copy into the params structure until we know the units */

    else if ( strstr( buffer, "PROJ. DIST" ) != NULL ) {
      sscanf( buffer, "%s%s%lf%lf", dummy, dummy, &distx, &disty );
    }

    else if ( strstr( buffer, "PROJ. UNITS" ) != NULL ) {
      if ( tspec->coord_sys == TSM_UTM_COORDSYS &&
	   strstr( buffer, "meters" ) == NULL )
	tsmError( funcName, "Warning: UTM data not in units of meters!" );
      else if ( tspec->coord_sys == TSM_LATLONG_COORDSYS &&
		strstr( buffer, "degrees" ) == NULL )
	tsmError( funcName, "Warning: Lat/Long data not in units of degrees!");

      if ( fabs( distx - disty ) > 1.0e-10 )
	tsmError( funcName, "Asymmetric projection size! Using %lf m", distx );

      tspec->start_res = distx;
      TSMTRACE(( TSMMASK|2, funcName, "PRT File: -startres %lf",
		 tspec->start_res ));
    }

    /* If we come across the UTM zone number, then grab it */

    else if ( strstr( buffer, "ZONE CODE" ) != NULL ) {
      sscanf( buffer, "%s%s%d", dummy, dummy, &tspec->zone );
      TSMTRACE(( TSMMASK|2, funcName, "PRT File: -zone %d", tspec->zone ));
    }

    /* If we come across the lower-left UTM coords, then grab them too */

    else if ( strstr( buffer, "LLcorner" ) != NULL ) {
      if ( tspec->coord_sys == TSM_UTM_COORDSYS ) {
	sscanf( buffer, "%s%lf%lf", dummy, &tspec->ll_n, &tspec->ll_e );
	TSMTRACE(( TSMMASK|2, funcName, "PRT File: -ll_e %1.3lf -ll_n %1.3lf",
		   tspec->ll_e, tspec->ll_n ));
      } else if ( tspec->coord_sys == TSM_LATLONG_COORDSYS ) {
	sscanf( buffer, "%s%lf%lf", dummy, &tspec->ll_lat, &tspec->ll_lon );
	TSMTRACE(( TSMMASK|2, funcName, "PRT File: -ll_lat %1.3lf -ll_long %1.3lf",
		   tspec->ll_lat, tspec->ll_lon ));
      }
    }

  }

  fclose( handle );
  TSMTRINC(( TSMMASK|2, -1 ));
  return TRUE;
}
