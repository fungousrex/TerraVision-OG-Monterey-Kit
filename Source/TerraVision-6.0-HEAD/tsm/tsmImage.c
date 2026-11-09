/*
 * Name: tsmImage.c
 *
 * Description: 
 *   Contains user-level routines for writing TerraVision tiles
 *   (as returned by tsmReadTile) out to an image file on disk
 *   in a number of popular image formats.
 *
 * Function List:
 *   int tsmSaveImage( TsmConnection, uchar *, char *, TsmImageType )
 *
 *   int tsmSaveImagePPM( char *, uchar *, int, int )
 *
 * Revision History:
 *   Created 6 Feb 1998 by Martin Reddy <reddy@ai.sri.com>
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

#include "tsmInternal.h"
#include "tsmConnectionInt.h"

#undef DrawText /* Protect against Windows DrawText->DrawTextA */
#include <gif_lib.h>
#include <jpeg/jpeglib.h>
#include <png.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#define PBM     1
#define PGM     2
#define PPM     3
#define PBMRAW  4
#define PGMRAW  5
#define PPMRAW  6

/*
 * Synopsis:
 *   int tsmSaveImagePPM( char *, uchar *, int, int, int )
 *
 * Description:
 *      Takes a tileBuffer and a filename and writes the data out to the
 *      file in a binary PPM format. We also provide the width and height
 *      of the tile buffer (pixels).
 *
 * Returns: TRUE if save went okay.
 *
 * Author: Martin Reddy
 * 
 * Date: 6 February 1998
 *
 */

int
tsmSaveImagePPM( char *filename, uchar *data, int w, int h, int flip )
{
  static const char *funcName = "tsmSaveImagePPM";
  FILE              *handle;
  int               i;

  if ( ( handle = fopen( filename, "wb" ) ) == NULL ) {
    tsmError( funcName, "Couldn't write PPM file to %s", filename );
    return FALSE;
  }

  fprintf( handle, "P6\n%d %d\n255\n", w, h );
  if ( flip ) {
    for ( i = h - 1; i >= 0; i-- )
      fwrite( &data[i*w*3], 3, w, handle );
  } else {
    for ( i = 0; i < h; i++ )
      fwrite( &data[i*w*3], 3, w, handle );
  }
  

  fclose( handle );
  return TRUE;
}

/*
 * Synopsis:
 *   int tsmSaveImagePGM( char *, uchar *, int, int, int )
 *
 * Description:
 *      Takes a tileBuffer and a filename and writes the data out to the
 *      file in a binary PGM format. We also provide the width and height
 *      of the tile buffer (pixels).
 *
 * Returns: TRUE if save went okay.
 *
 * Author: Martin Reddy
 * 
 * Date: 30 March 1998
 *
 */

int
tsmSaveImagePGM( char *filename, uchar *data, int w, int h, int flip )
{
  static const char *funcName = "tsmSaveImagePGM";
  FILE              *handle;
  int               i;

  if ( ( handle = fopen( filename, "wb" ) ) == NULL ) {
    tsmError( funcName, "Couldn't write PGM file to %s", filename );
    return FALSE;
  }

  fprintf( handle, "P5\n%d %d\n255\n", w, h );
  if ( flip ) {
    for ( i = h - 1; i >= 0; i-- )
      fwrite( &data[i*w], 1, w, handle );
  } else {
    for ( i = 0; i < h; i++ )
      fwrite( &data[i*w], 1, w, handle );
  }

  fclose( handle );
  return TRUE;
}

/*
 * Synopsis:
 *   int tsmSaveImageGIF( char *filename, unsigned char *image, int, int )
 *
 * Description:
 *      Takes an RGB image buffer, its dimensions, and a filename and
 *      then writes the data out to the file in GIF format. This is
 *      performed using the GNU ungif library. This library generates
 *      uncompressed GIF images, so we don't need to worry about any
 *      licensing issues with the LZW compression.
 *
 * Returns: TRUE if save went okay.
 *
 * Author: Martin Reddy
 * 
 * Date: 11 March 1999
 *
 */

int
tsmSaveImageGIF( char *filename, unsigned char *image, int width, int height )
{
  const char     *funcName = "tsmWriteImageGIF";
  int            ColorMapSize = 256;
  GifFileType    *GifFile;
  ColorMapObject *OutputColorMap;
  GifByteType    *RedBuffer, *GreenBuffer, *BlueBuffer;
  GifByteType    *OutputBuffer, *p;
  unsigned char  *buffer;
  int            i, pixels = width * height, quant;

  /* Allocate separate buffers to store the R, G, and B components */

  RedBuffer   = (GifByteType *) tsmMemMalloc( pixels );
  GreenBuffer = (GifByteType *) tsmMemMalloc( pixels );
  BlueBuffer  = (GifByteType *) tsmMemMalloc( pixels );

  if ( RedBuffer == NULL || GreenBuffer == NULL || BlueBuffer == NULL ) {
    tsmError( funcName, "Could not allocate separate RGB buffers" );
    return FALSE;
  }

  /* Fill these separate RGB buffers from the RGBRGBRGB... image */
  /* We need these separated to do the 24 -> 8-bit conversion. */

  for ( buffer = image, i = 0; i < pixels; i++, buffer += 3 ) {
    RedBuffer[i]   = buffer[0];
    GreenBuffer[i] = buffer[1];
    BlueBuffer[i]  = buffer[2];
  }

  /* Allocate a colour map object and an output buffer for the quantization */

  OutputColorMap = MakeMapObject( ColorMapSize, NULL );
  OutputBuffer = (GifByteType *) tsmMemMalloc( pixels * sizeof(GifByteType) );

  if ( OutputColorMap == NULL || OutputBuffer == NULL ) {
    tsmError( funcName, "Could not allocate output buffer" );
    return FALSE;
  }

  /* Quantize the 24-bit RGB buffer to a palette-based 8-bit image */

  quant = QuantizeBuffer( width, height, &ColorMapSize, RedBuffer, GreenBuffer,
			  BlueBuffer, OutputBuffer, OutputColorMap->Colors );

  tsmMemFree( RedBuffer );
  tsmMemFree( GreenBuffer );
  tsmMemFree( BlueBuffer );

  if ( quant == GIF_ERROR) {
    tsmError( funcName, "Failed to quantize 24-bit to 8-bit image" );
    tsmMemFree( OutputBuffer );
    return FALSE;
  }

  /* Open the GIF image for output */

  if ( ( GifFile = EGifOpenFileName( filename, FALSE ) ) == NULL ) {
    tsmError( funcName, "Could not write to the file: %s", filename );
    tsmMemFree( OutputBuffer );
    return FALSE;
  }

  /* Write out the screen and image description elements */

  if ( ! EGifPutScreenDesc( GifFile, width, height, 8, 0, OutputColorMap ) ) {
    tsmError( funcName, "Could not write screen descrtiption: %s", filename );
    tsmMemFree( OutputBuffer );
    return FALSE;
  }

  if ( ! EGifPutImageDesc( GifFile, 0, 0, width, height, FALSE, NULL) ) {
    tsmError( funcName, "Could not write image description: %s", filename );
    tsmMemFree( OutputBuffer );
    return FALSE;
  }

  /* Write the output buffer to the file */

  for ( i = 0, p = OutputBuffer; i < height; i++, p += width )
    EGifPutLine( GifFile, p, width );

  /* We're all done - clean up and say bye-bye */
      
  tsmMemFree( OutputBuffer );
  EGifCloseFile( GifFile );
  return TRUE;
}

/* support this old tsmApi function using the new GIF writer */
void
rgb_image_to_gif_file( char *filename, unsigned char *image24, int nx, int ny )
{
  tsmSaveImageGIF( filename, image24, nx, ny );
}

/*
 * Synopsis:
 *   int tsmSaveImageJFIF( char *, uchar *, int, int )
 *
 * Description:
 *      Takes a tileBuffer and a filename and writes the data out to the
 *      file in JPEG/JFIF format. We also provide the width and height
 *      of the tile buffer (pixels).
 *
 * Externals: None
 *
 * Returns: TRUE if save went okay.
 *
 * Author: Martin Reddy
 * 
 * Date: 6 February 1998
 *
 */

#define JFIF_MAGIC_SIZE  20  /* the byte size of the JFIF file header */

int
tsmSaveImageJFIF( TsmConnection connect, char *filename, uchar *data, int w,
		  int h )
{
  static const char *funcName = "tsmSaveImageJFIF";
  static const int  offset = JFIF_MAGIC_SIZE - 2; /* +header, -M_SOI marker */
  TileTspec         *tile = &connect->tspec->tileTspec;
  FILE              *handle;
  uchar             *jpegBuffer;
  int               length;

  /* store the JFIF header section in a static array for quick access */

  static unsigned char jfif_header[JFIF_MAGIC_SIZE] = {
    0xff, 0xD8,                  /* M_SOI marker */
    0xFF, 0xE0, 0x00, 0x10,      /* M_APP0 marker and length */
    'J', 'F', 'I', 'F', '\0',    /* JFIF identifier */
    0x01, 0x02,                  /* JFIF version = 1.02 */
    0x00,                        /* units = none */
    0x00, 0x01, 0x00, 0x01,      /* x and y densities = 1 */
    0x00, 0x00                   /* no thumbnail image */
  };

  /* allocate a large enough buffer to compress the tile into */

  length = connect->tileBufferSize + offset;

  if ( ( jpegBuffer = tsmMemManagerMalloc( length ) ) == NULL ) {
    tsmError( funcName, "Couldn't allocate a JPEG decompress buffer" );
    return FALSE;
  }

  /* Compress the tile into the jpeg buffer (leaving room for header) */

  length = tsmJPEGCompressTile( data, jpegBuffer + offset, tile,
				connect->jpeg_quality );

  if ( length == 0 ) {
    tsmError( funcName, "Couldn't JPEG compress the given tile" );
    tsmMemManagerFree( jpegBuffer );
    return FALSE;
  }

  /* Write the JFIF APP header to the start of the jpeg buffer */

  memcpy( jpegBuffer, jfif_header, JFIF_MAGIC_SIZE );

  /* And then write out the JPEG/JFIF file to the specified file on disk */

  if ( ( handle = fopen( filename, "wb" ) ) == NULL ) {
    tsmError( funcName, "Couldn't write JPEG/JFIF file to %s", filename );
    tsmMemManagerFree( jpegBuffer );
    return FALSE;
  }

  fwrite( jpegBuffer, 1, length + offset, handle );
  fclose( handle );
  tsmMemManagerFree( jpegBuffer );
  return TRUE;
}

/*
 * Synopsis:
 *   int tsmSaveImage( TsmConnection connect, uchar *tileBuffer,
 *                     char *filename, TsmImageType type )
 *
 * Doc++ Description: *//** 
 *   This function will write out a single TerraVision tile (e.g. as
 *   returned by tsmReadTile) to an image file on disk. You need to
 *   pass the connection handle that was used to read the tile from
 *   (so that the information such as the dimensions of the tile may
 *   by gleaned), a localhost filename (or file: URL) to write the
 *   image file to, and an image format. Currently supported image
 *   formats are:
 *
 * \begin{itemize}
 * \item {\bf TSM_PPM}: binary Portable BitMap format file (no compression).
 * \item {\bf TSM_GIF}: Graphic Interchange Format file (lossless compression).
 * \item {\bf TSM_JFIF}: JPEG File Interchange Format file (lossy compression).
 * \end{itemize}
 *
 *   A filename extension will be added to your filename automatically (if
 *   not already supplied), i.e. one of ppm, gif, or jpg, respectively.
 *
 *   Note that when you choose TSM_PPM, the output image will be in 
 *   ppm format if the image data is RGB, or pgm format if grayscale.
 *
 *   You can use the tsmSet(c,TSM_JPEG_QUALITY,v) function call as normal
 *   to set the jpeg quality level that is used to create the JPEG/JFIF
 *   format files.
 *
 * @param    connect a connection handle, as returned by tsmConnect
 * @param    tileBuffer a TerraVision tile buffer, as returned by tsmReadTile
 * @param    filename the filename/URL for the file to write to on disk
 * @param    type one of TSM_PPM, TSM_GIF, or TSM_JFIF
 *
 * @return   TRUE if image file written successfully, FALSE if error occurred
 *
 * @author   Martin Reddy
 * @version  12 January 1998
 *
 * @see      tsmSet, tsmReadTile
 *
 */


int
tsmSaveImage( TsmConnection connect, uchar *tileBuffer, char *filename,
	      TsmImageType type )
{
  static const char *funcName = "tsmSaveImage";
  Purl              *purl;
  char              imagefile[512];
  int               width, height, comps, x, y, result;
  uchar             *demBuffer = NULL;
  int               *intBuffer;
  float             minval, maxval, scale, offset, elev;

  /* Check the connection structure and enforce its access mode */

  if ( ! tsmCheckConnectMode( funcName, connect, TSM_READ_WRITE, TRUE ) )
    return FALSE;

  /* Check that we have a valid tile buffer */

  if ( tileBuffer == NULL ) {
    tsmError( funcName, "NULL tile buffer parameter passed" );
    return FALSE;
  }
  
  /* Try to parse the filename/URL so that we can support file: params */

  if ( ( purl = tsmParseUrl( filename ) ) == NULL ) {
    tsmError( funcName, "Couldn't parse the filename URL: %s", filename );
    return FALSE;
  }

  /* Only handle file:localhost URLs */

  if ( purl->protocol != tsmLocalFile ) {
    tsmNoProtocolYet( purl->protocol, funcName );
    return FALSE;
  }

  strcpy( imagefile, purl->uri );
  tsmFreePurl( purl );

  /* Work out the tile dimensions */

  width  = connect->tspec->tileTspec.numPixelsX;
  height = connect->tspec->tileTspec.numPixelsY;
  comps  = connect->tspec->tileTspec.numComponentsPixel;

  /* If the tile comes from a DEM dataset, then the pixels */
  /* are 32-bits wide. In order to make these more visual, */
  /* we convert these tiles into a greyscale image by      */
  /* scaling each values into the range 0..255.            */

  if ( connect->dataset_type == TSM_DEM_DATASET ) {
    if ( ( demBuffer = tsmAllocTileBuffer( connect ) ) != NULL ) {
      minval = connect->tspec->pixelTspec.minVal;
      maxval = connect->tspec->pixelTspec.maxVal;
      scale  = connect->tspec->pixelTspec.scale;
      scale  *= 255.0f / ( maxval - minval );
      offset = connect->tspec->pixelTspec.offset;

      intBuffer    = (int *) tileBuffer;

      for ( y = 0; y < height; ++y ) {
	for ( x = 0; x < width; ++x ) {
	  elev = ((*intBuffer) - offset ) * scale;
	  demBuffer[y*width+x] = (uchar) elev;
	  intBuffer++;
	}
	intBuffer++;  /* skip overlap posting at end of each row */
      }

      tileBuffer = demBuffer;
    }
  }

  /* vector off to the appropriate tile save function */

  switch( type ) {
  case TSM_PPM:

    if ( comps == 1 ) {
      tsmAddFilenameExtension( imagefile, ".pgm" );
      result = tsmSaveImagePGM( imagefile, tileBuffer, width, height, TRUE );
    } else {
      tsmAddFilenameExtension( imagefile, ".ppm" );
      result = tsmSaveImagePPM( imagefile, tileBuffer, width, height, TRUE );
    }
    break;

  case TSM_GIF:
    tsmAddFilenameExtension( imagefile, ".gif" );
    tsmSaveImageGIF( imagefile, tileBuffer, width, height );
    result = TRUE;
    break;

  case TSM_JFIF:
    tsmAddFilenameExtension( imagefile, ".jpg" );
    result = tsmSaveImageJFIF( connect, imagefile, tileBuffer, width, height );
    break;

  default:
    tsmError( funcName, "Unknown image format (%d)", (int) type );
    result = FALSE;
  }

  if ( demBuffer != NULL ) tsmFreeTileBuffer( demBuffer );

  return result;
}

/*
 * Synopsis:
 *      int tsmFlipTileVertically(unsigned char *, TileTspec *)
 *
 * Description:
 *      Takes a pointer to a tile's image buffer and flips the image
 *      vertically (i.e. mirrors it). We pass in the TileTspec
 *      pointer for the tile so that we can easily find the width,
 *      height, and componentsPerPixel attributes for the image.
 *      
 * Externals: None
 *
 * Returns: TRUE if operation was succesful, FALSE if it failed.
 *
 * Author: Martin Reddy
 * 
 * Date: 14 July 1997
 *
 */

int
tsmFlipTileVertically( unsigned char *tileBuffer, TileTspec *tiletspec )
{
  const char *funcName = "tsmFlipTileVertically";
  unsigned char *tempBuffer, *src, *dest;
  int width  = tiletspec->numPixelsX;
  int height = tiletspec->numPixelsY;
  int comps  = tiletspec->numComponentsPixel;
  int bufferSize = width * height * comps;
  int bytesPerRow = width * comps;
  int y;

  /* Allocate a buffer to contain a copy of the image data */

  if ( ( tempBuffer = (unsigned char *)
	 tsmMemManagerMalloc( bufferSize ) ) == NULL ) {
    tsmError( funcName, "Couldn't allocate memory for operation" );
    return FALSE;
  }

  memcpy( tempBuffer, tileBuffer, bufferSize );

  /* Now mirror each row from the temporary buffer to the tile buffer */
  /* We try to optimise this by using pointer arithmetic instead of   */
  /* array look-ups (i.e. tileBuffer[]), and using memcopy().         */

  src  = &tempBuffer[bufferSize-bytesPerRow];
  dest = tileBuffer;
  for ( y = 0; y < height; y++ ) {
    memcpy( dest, src, bytesPerRow );
    dest += bytesPerRow;
    src  -= bytesPerRow;
  }

  /* Free the temporary image buffer and return a successful result */

  free( tempBuffer );
  return TRUE;
}

static void
ppm_skip_whitespace (FILE *in)
{
  int c;

  while ((c = getc(in)) != EOF) {
    if (c == '#') {
      while ((c = getc(in)) != EOF)
	if (c == '\n') break;
    } else if (! isspace (c)) {
      ungetc (c, in);
      break;
    }
  }
}

static int
ppm_read_header (FILE *in, int *width, int *height)
{
  char hdr[2];
  int type;

  /* Read two initial bytes for the "magic number" */
  if ( fread (hdr, 1, 2, in) != 2 )
    return -1;

  /* Ensure that this is, in fact a PPM file. */
  if (hdr[0] != 'P' || ! isdigit (hdr[1]) || (type = hdr[1]-'0') > 6)
    return -1;

  /* Read in the image width */
  ppm_skip_whitespace( in );
  if ( fscanf( in, "%d", width ) != 1) return -1;

  /* Read in the image height */
  ppm_skip_whitespace( in );
  if ( fscanf( in, "%d", height ) != 1) return -1;

  return type;
}

static unsigned char *
ppm_read (FILE *in, int *width, int *height, int *pixel_len)
{
  const char    *funcName = "ppm_read";
  unsigned char *data, *p;
  int            type, len, maxval, c, bytes;

  if ( ( type = ppm_read_header( in, width, height ) ) < 0 )
    return NULL;

  len = *width * *height;
  *pixel_len = 3; /* assume RGB by default */

  switch (type) {

  case PGM:
    *pixel_len = 1;
    /* fall through to PPM (ASCII) reading routine */

  case PPM:
    ppm_skip_whitespace( in );
    if ( fscanf( in, "%d", &maxval ) != 1 ) return NULL;
    if ( maxval > 255 ) return NULL;
    bytes = *pixel_len * len;

    if ( ( data = malloc( bytes ) ) == NULL ) return NULL;

    for ( p = data; bytes > 0 ; --bytes, p++ ) {
      if ( fscanf( in, "%d", &c ) == 0 ) {
	free( data );
	return NULL;
      }
      *p = ( c > maxval ) ? maxval : c;
    }
    break;

  case PGMRAW:
    *pixel_len = 1;
    /* fall through to PPM (RAW) reading routine */

  case PPMRAW:
    ppm_skip_whitespace( in );
    if ( fscanf( in, "%d", &maxval ) != 1 ) return NULL;
    if ( maxval > 255 || (c = getc(in) ) == EOF || ! isspace(c) ) return NULL;
    bytes = *pixel_len * len;

    if ( ( data = malloc( bytes ) ) == NULL ) return NULL;

    if ( fread( data, 1, bytes, in ) != bytes ) {
      free( data );
      return NULL;
    }
    break;

  case PBMRAW:
  case PBM:
  default:
    tsmError( funcName, "Unsupported format. PBM files not supported." );
    return NULL;
  }

  return data;
}

static unsigned char *
ppm_read_file (char const *filename, int *width, int *height, int *pixel_len)
{
  FILE *in = fopen (filename, "rb");
  unsigned char *data;

  if (! in) return NULL;
  data = ppm_read (in, width, height, pixel_len);
  fclose (in);

  return data;
}

static unsigned char *
tsm_png_read( FILE *fp, int *width, int *height, int *comps )
{
  const char    *funcName = "tsm_png_read";
  png_structp   png_ptr;
  png_infop     info_ptr;
  png_infop     end_info;
  int           bit_depth, color_type, i;
  unsigned char *bitmap;
  png_bytep     *row_pointers;

  /* initialise the PNG structures */
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr)
    return NULL;

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_read_struct(&png_ptr,(png_infopp)NULL, (png_infopp)NULL);
    return NULL;
  }

  end_info = png_create_info_struct(png_ptr);
  if (!end_info) {
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    return NULL;
  }
 
  /* open the image file */
  png_init_io(png_ptr, fp);

  /* read the image header information */
  png_read_info(png_ptr, info_ptr);

  /* copy these into our return variables */
  *width = png_get_image_width(png_ptr, info_ptr);
  *height = png_get_image_height(png_ptr, info_ptr);
  bit_depth = png_get_bit_depth(png_ptr, info_ptr);
  color_type = png_get_color_type(png_ptr, info_ptr);

  *comps = 3;
  if ( color_type == PNG_COLOR_TYPE_RGB_ALPHA || 
       color_type == PNG_COLOR_TYPE_GRAY_ALPHA )
    *comps = 4;
  
  /* scale < 8-bit images to 8-bit and expand palettes to RGB */
  if (color_type == PNG_COLOR_TYPE_PALETTE && bit_depth <= 8)
    png_set_expand(png_ptr);

  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand(png_ptr);

  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
    png_set_expand(png_ptr);

  /* sample grayscale images to RGB */
  if (color_type == PNG_COLOR_TYPE_GRAY ||
      color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
     png_set_gray_to_rgb(png_ptr);

  /* update png info for above changes */
  png_read_update_info(png_ptr, info_ptr);

  /* allocate bitmap for the image */

  if ( ( bitmap = malloc( *width * *height * *comps ) ) == NULL )
    return NULL;

  /* allocate the row pointers array */

  if ( ( row_pointers = malloc( *height * sizeof(png_bytep) ) ) == NULL )
    return NULL;

  for ( i = 0; i < *height; i++ )
    row_pointers[i] = bitmap + ( *width * *comps * i );

  /* read image here */

  png_read_image( png_ptr, row_pointers );

  free( row_pointers );

  /* clean up */
  png_read_end(png_ptr, end_info);
  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

  return bitmap;
}

static unsigned char *
gif_read( FILE *handle, int *width, int *height, int *comps )
{
  const char        *funcName = "gif_read";
  GifFileType       *gif_info;
  struct SavedImage *gif_image;
  unsigned char     *image, *out_buffer;
  unsigned char     *in_buffer;
  ColorMapObject    *colormap;
  GifColorType      *palette, *rgbcolor;
  int               fd, bytes, n;

  /* take a copy of the file descriptor from the FILE structure */

  fd = dup( fileno (handle) );

  /* read the gif image */

  if ( ( gif_info = DGifOpenFileHandle( fd ) ) == NULL )
    return NULL;

  if ( DGifSlurp( gif_info ) == GIF_ERROR || gif_info->ImageCount < 1 ) {
    DGifCloseFile( gif_info );
    return NULL;
  }

  /* extract the bits we need - take a copy of the bitmap data */

  gif_image = &gif_info->SavedImages[0];
  *width    = gif_image->ImageDesc.Width;
  *height   = gif_image->ImageDesc.Height;
  *comps    = 3; /* this is always the case for GIF */
  bytes     =  *width * *height * *comps;

  /* get a pointer to the image's palette */

  if ( ( colormap = gif_image->ImageDesc.ColorMap ) == NULL ) { /* local */
    if ( ( colormap = gif_info->SColorMap ) == NULL ) { /* global */
      tsmError( funcName, "GIF image does not have a color map!" );
      DGifCloseFile( gif_info );
      return NULL;
    }
  }

  palette = colormap->Colors;

  /* allocate memory for the bitmap */

  if ( ( image = (unsigned char *) tsmMemMalloc( bytes ) ) == NULL ) {
    DGifCloseFile( gif_info );
    return NULL;
  }

  /* convert the palette-based bitmap into 24-bit RGB */

  in_buffer  = (unsigned char *) gif_image->RasterBits;
  out_buffer = image;
  for ( n = 0; n < bytes; n += 3 ) {
    rgbcolor = &palette[*in_buffer++];
    *out_buffer++ = rgbcolor->Red; 
    *out_buffer++ = rgbcolor->Green; 
    *out_buffer++ = rgbcolor->Blue; 
  }
  
  /* close the file, free the GifFileType structure, and exit */
  
  DGifCloseFile( gif_info );
  return image;
}

static unsigned char *
jpeg_read( FILE *handle, int *width, int *height, int *comps )
{
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  JDIMENSION    num_scanlines;
  unsigned char *image, *buffer;
  int           bytes, rowsize;

  /* Initialize the JPEG decompression object with default error handling. */
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);

  /* Specify data source for decompression */
  jpeg_stdio_src(&cinfo, handle);

  /* Read file header, set default decompression parameters */
  jpeg_read_header(&cinfo, TRUE);

  /* extract the image dimensions and allocate a bitmap buffer */
  *width  = cinfo.image_width;
  *height = cinfo.image_height;
  *comps  = cinfo.num_components;
  rowsize = *width * *comps;
  bytes   = rowsize * *height;

  if ( ( image = (unsigned char *) tsmMemMalloc( bytes ) ) == NULL ) {
    jpeg_destroy_decompress(&cinfo);
    return NULL;
  }
  buffer = image;

  /* Start decompressor */
  jpeg_start_decompress(&cinfo);

  /* Process data */
  while (cinfo.output_scanline < cinfo.output_height) {
    num_scanlines = jpeg_read_scanlines( &cinfo, (JSAMPARRAY) &buffer, 1 );
    buffer += num_scanlines * rowsize;
  }

  /* Finish decompression and release memory. */
  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  return image;
}

/*
 * Synopsis:
 *   Image_t *tsmReadImage( char *url )
 *
 * Doc++ Description: *//** 
 *   Read an image from localhost or http.
 *
 *   This function will read an image file from a localhost or http
 *   URL. A number of different image formats are supported: GIF,
 *   JPEG, PPM (Raw and ASCII), PGM (Raw and ASCII). The routine looks
 *   into the file in order to work out its format, rather than
 *   checking the filename extension.
 *
 *   The url string can be a localdisk filename, such as "fred.gif",
 *   "/homedir/foo/fred.gif", or "~foo/fred.gif", or it can be a
 *   file://localhost/ or http://hostname/ URL.
 *
 *   On successful completion, the function will return an Image_t
 *   structure with the image data, otherwise NULL is returned.
 *
 *   A note on performance: PPM, PGM, and JPEG images are read
 *   directly from the input stream into the bitmap memory buffer and
 *   so are very fast. GIF images entail an extra memory copy from the
 *   libungif internal buffer into the Image_t buffer.
 *
 * @param    url (char *) = a localdisk filename, or a file/http URL.
 *
 * @return   A valid Image_t structure, or NULL if failed
 *
 * @author   Martin Reddy
 * @version  9 March 1999
 *
 * @see      tsmWriteImage, tsmFreeImage, Image_t
 * */

Image_t *
tsmReadImage( char const *url )
{
  FILE *handle;
  unsigned char header[8];
  Image_t *image;

  if ( ( handle = tsmOpenStream( url, "rb" ) ) == NULL ) return NULL;

  /* read the header bytes and then close and re-open this handle. */
  /* it seems to be safer to do this than trying to do a rewind or */
  /* dup on the file - especially as we might be reading over http */

  fread( header, 8, 1, handle );
  tsmCloseStream( handle );

  if ( ( handle = tsmOpenStream( url, "rb" ) ) == NULL ) return NULL;

  /* Allocate the new Image_t structure */

  if ( ( image = (Image_t *) tsmMemCalloc( 1, sizeof(Image_t) ) ) == NULL ) 
    return NULL;

  /* work out what type of image file we have here */

  if ( header[0] == 'P' && header[1] >= '1' && header[2] <= '6' ) {

    /* PPM format image */

    image->bitmap = ppm_read( handle, &image->width, &image->height,
			      &image->components );

  } else if ( strncmp( (char*)header, "GIF8", 4 ) == 0 ) {

    /* GIF format image */

    image->bitmap = gif_read( handle, &image->width, &image->height,
			      &image->components );

  } else if ( strncmp( (char*)header, "MM", 2 ) == 0 ||
	      strncmp( (char*)header, "II", 2 ) == 0 ) {

    /* TIFF format image */
    printf( "TIFF format not supported yet!\n" );

  } else if ( header[0] == 0xff && header[1] == 0xd8 ) {

    /* JPEF/JFIF format image */

    image->bitmap = jpeg_read( handle, &image->width, &image->height,
			       &image->components );

  } else if ( strncmp( (char*)header+1, "PNG", 3 ) == 0 ) {

    /* PNG format image */

    image->bitmap = tsm_png_read( handle, &image->width, &image->height,
			       &image->components );

  }
  
  tsmCloseStream( handle );

  if ( image->bitmap == NULL ) {
    tsmMemFree( image );
    return NULL;
  }

  return image;
}

/*
 * Synopsis:
 *   void tsmFreeImage( Image_t *image, int free_bitmap )
 *
 * Doc++ Description: *//** 
 *   Frees an Image_t structure.
 *
 *   This function will free an Image_t structure, as might be returned
 *   by tsmReadImage. You have the choice of whether you wish to free 
 *   the bitmap array of this structure. If you are completely finished
 *   with this structure, then you should set free_bitmap to TRUE. 
 *   However, if you want to free the memory for the Image_t structure
 *   but you want the bitmap data to live on, then you should use a
 *   value of FALSE.
 *
 * @param    image (Image_t *) = pointer to the Image_t structure to free
 * @param    free_bitmap (int) = if TRUE, then free image->bitmap.
 *
 * @return   Nothing
 *
 * @author   Martin Reddy
 * @version  9 March 1999
 *
 * @see      tsmReadImage, tsmWriteImage
 *
 */

void
tsmFreeImage( Image_t *image, int free_bitmap )
{
  if ( image == NULL ) return;

  if ( free_bitmap && image->bitmap ) 
    tsmMemFree( image->bitmap );

  tsmMemFree( image );
}

/*
 * Synopsis:
 *   int tsmWriteImage( Image_t *image, char const *filename, TsmImageType )
 *
 * Doc++ Description: *//** 
 *   Write out an Image_t structure to a file on disk.
 *
 *   This function will save the contents of an Image_t structure out to
 *   a localhost filename or URL. A number of image formats can be
 *   supported by this function, as specified by the format parameter.
 *
 *   Currently, the PPM/PGM (TSM_PPM) and GIF (TSM_GIF) formats are
 *   supported. N.B. the GIF writer generates uncompressed GIF images.
 *   This means we don't need to worry about the licensing issues
 *   surrounding the use LZW compression in GIF.
 *
 *   This function will append an appropriate filename extension to your
 *   filename before saving, e.g. ".gif" for GIF images, ".ppm" for PPM
 *   images, etc.
 *
 * @param    image (Image_t *) = pointer to the Image_t structure
 * @param    filename (char *) = the filename or file://localhost/path URL
 * @param    format (TsmImageType) = currently only TSM_PPM
 *
 * @return   TRUE if the image was written successfully, otherwise FALSE.
 *
 * @author   Martin Reddy
 * @version  9 March 1999
 *
 * @see      tsmReadImage, tsmFreeImage
 * */

int
tsmWriteImage( Image_t *image, char const *filename, TsmImageType format )
{
  const char *funcName = "tsmWriteImage";
  Purl *purl;
  char buffer[512];

  /* Check that we have a localhost filename/url */

  if ( image == NULL || filename == NULL ) return FALSE;

  if ( ( purl = tsmParseUrl( filename ) ) == NULL ) {
    tsmError( funcName, "Invalid filename: %s", filename );
    return FALSE;
  }

  if ( purl->protocol != tsmLocalFile ) {
    tsmError( funcName, "Only supports localhost filenames: %s", filename );
    return FALSE;
  }

  strcpy( buffer, purl->uri );

  /* Save out the image buffer in the requested file format */

  switch ( format ) {
  case TSM_PPM:
    if ( image->components == 3 ) {
      tsmAddFilenameExtension( buffer, ".ppm" );
      tsmSaveImagePPM( buffer, image->bitmap, image->width, image->height, 0 );
    } else {
      tsmAddFilenameExtension( buffer, ".pgm" );
      tsmSaveImagePGM( buffer, image->bitmap, image->width, image->height, 0 );
    }

  case TSM_GIF:
    if ( image->components == 3 ) {
      tsmAddFilenameExtension( buffer, ".gif" );
      tsmSaveImageGIF( buffer, image->bitmap, image->width, image->height );
    } else {
      tsmError( funcName, "Can only write RGB format images to GIF!" );
      return FALSE;
    }

    break;
  default:
    tsmError( funcName, "Specified format not supported yet!" );
    return FALSE;
  }

  return TRUE;
}

