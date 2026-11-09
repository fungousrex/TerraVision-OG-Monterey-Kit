/*
 * Name: tsmJPEG.c
 *
 * Description: 
 *   Routines to JPEG compress and decompress TV tiles
 *
 * Function List:
 *   int tsmJPEGCompressTile( unsigned char *tileBuffer, unsigned char
 *                            *jpegBuffer, TileTspec *tile, int quality_level )
 *
 *   int tsmJPEGDecompressTile( unsigned char *jpegBuffer, unsigned char 
 *                              *tileBuffer, TileTspec *tile )
 *
 * Revision History:
 *
 *   $Id: tsmJPEG.c,v 1.10.6.2 2001/06/01 22:36:05 reddy Exp $
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
#include <jpeg/jpeglib.h>


/* Some NULL functions to use as callbacks for the destination */
/* data manager in the tsmJPEGCompressTile() function.         */

void null_init_destination( j_compress_ptr cinfo ) {}
boolean null_empty_output_buffer( j_compress_ptr cinfo ) {return TRUE;}
void null_term_destination( j_compress_ptr cinfo ) {}

/*
 * Synopsis:
 *   int tsmJPEGCompressTile( unsigned char *tileBuffer, unsigned char
 *                            *jpegBuffer, TileTspec *tile, int quality_level )
 *
 * Doc++ Description: *//** 
 *   JPEG compress a raw tile in memory.
 *
 *   This function will accept two pointers to tile buffers that have
 *   already been allocated (e.g. with tsmMakeTileBuffer). The tileBuffer
 *   contains a raw bitmap, whilst the jpegBuffer will be filled by this
 *   function and will contain the JPEG compressed version of the raw 
 *   data. The function returns the size of the JPEG compressed data that
 *   was written to jpegBuffer (in bytes).
 *
 *   A few things to note about this function:
 *   \begin{itemize}
 *   \item The raw data will undergo an implicit colour space conversion into
 *         the YCbCr space. This is required for compatibility with the
 *         JFIF de-facto standard.
 *   \item The origin of the JPEG compressed data will be the top-left corner
 *         of the tile, rather than the bottom-left corner as used by TV.
 *         Again this is required for conformance with JFIF.
 *   \item The JPEG compressed data is not a complete JFIF stream, i.e. it
 *         does not have the M_SOI and M_APP0 markers at the start.
 *   \end{itemize}
 *
 *   N.B. this function outputs exactly the same JPEG files (size, data,
 *   tables, etc.) as the libtiff library routines. However this routine is 
 *   slightly more efficient because it compresses the data in reverse
 *   order for you so you don't have to vertically flip the input buffer 
 *   before compressing.
 *
 * @param    tileBuffer the tile buffer of raw data to read from
 * @param    jpegBuffer the tile buffer to be filled with JPEG data
 * @param    tile the tile tspec giving the image dimensions, etc.
 * @param    quality_level the quality level for the compression. This can
 *           take a value from 0 (most compression) to 100 (least compression).
 *           A good default value is 75.
 *
 * @return   the number of bytes written to the jpegBuffer, 0 = failure
 *
 * @author   Martin Reddy
 * @version  6 November 1997
 *
 * @see      tsmJPEGDecompressTile, tsmMakeTileBuffer,
 *           tsmSetJPEGQuality, TileTspec
 *
 */

int
tsmJPEGCompressTile( unsigned char *tileBuffer, unsigned char *jpegBuffer,
		     TileTspec *tile, int quality_level )
{
  return tsmJPEGCompress( tileBuffer, jpegBuffer, tile->totalPixels[0],
			  tile->totalPixels[1], tile->numComponentsPixel,
			  tile->numBytesPerComponent, quality_level );
}

int
tsmJPEGCompress( unsigned char *tileBuffer, unsigned char *jpegBuffer,
		 int width, int height, int comps, int compbytes,
		 int quality_level )
{
  const char                  *funcName = "tsmJPEGCompressTile";
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr       jerr;
  struct jpeg_destination_mgr dest_mgr;
  JSAMPROW                    buffer;
  int                         tile_size, jpeg_tile_size;
  int                         jpeg_data_size, row_size;

  if ( tileBuffer == NULL || jpegBuffer == NULL ) {
    tsmError( funcName, "input and output buffers must be non-NULL!" );
    return FALSE;
  }

  /* First we shall work out the size of the uncompressed tile (bytes) */
  /* The size of the jpeg tile is currently specified to be the same   */
  /* as the raw (uncompressed) tile.                                   */

  tile_size = width * height * comps * compbytes;

  jpeg_tile_size = tile_size;

  /* Initialise the JPEG compression object with default error handling. */

  cinfo.err = jpeg_std_error( &jerr );
  jpeg_create_compress( &cinfo );
 
  /* Initialize the JPEG colour space parameters and the quality setting */
  /* If we have a DEM tile, then we set the colour space to GRAYSCALE    */
  /* because we only have one component per pixel, otherwise, it's RGB   */

  cinfo.input_components = comps;
  if ( comps == 1 )
    cinfo.in_color_space = JCS_GRAYSCALE;
  else
    cinfo.in_color_space = JCS_RGB;

  jpeg_set_defaults( &cinfo );

  /* Set the colour space to store the jpeg data in. This has to be  */
  /* YCbCr for compatibility with the JFIF standard. We also set the */
  /* compression quality and specify that we don't want to force the */
  /* generation of quantization table entires that are limited to    */
  /* 1..255 for JPEG baseline compat (this is what libtiff does also)*/

  jpeg_set_colorspace( &cinfo, JCS_YCbCr );
  jpeg_set_quality( &cinfo, quality_level, FALSE ); 

  /* Now fill in all of the image dimension details. We specify the */
  /* width to include the number of bytes per component so that we  */
  /* could feasible JPEG compress DEM tiles okay.                   */

  cinfo.image_width        = (JDIMENSION) width * compbytes;
  cinfo.image_height       = (JDIMENSION) height;

  /* we don't want any JFIF or Adobe headers - so tell 'em that. */

  cinfo.write_JFIF_header  = FALSE;
  cinfo.write_Adobe_marker = FALSE;

  /* We specify here that we want the compressor to compute optimal */
  /* Huffman coding tables for the image.  This requires a bit more */
  /* computation, but it saves a couple hundred bytes per image.    */
  /* This optimisation is also performed by libtiff.                */

  cinfo.optimize_coding = TRUE;

  /* These sampling factors aren't setup correctly by default, so */
  /* we should reset these to {1,1} here, again just like libtiff */

  cinfo.comp_info[0].h_samp_factor = 1;
  cinfo.comp_info[0].v_samp_factor = 1;

  /* set up the destination manager so that the jpeg_write_scanlines() */
  /* routine will write directly to our jpeg buffer. We set all of the */
  /* callbacks to empty functions because they should never be needed  */
  /* because we should have a buffer big enough to fit the entire image*/

  dest_mgr.next_output_byte    = (JOCTET *) jpegBuffer;
  dest_mgr.free_in_buffer      = (size_t) jpeg_tile_size;
  dest_mgr.init_destination    = null_init_destination;
  dest_mgr.empty_output_buffer = null_empty_output_buffer;
  dest_mgr.term_destination    = null_term_destination;

  cinfo.dest = &dest_mgr;

  /* Start the JPEG compressor... and include all tables */

  jpeg_start_compress( &cinfo, TRUE );

  /* Compress the data into jpegBuffer, starting with the last row    */
  /* and working back to the first row because we also need to switch */
  /* the origin from lrbt to lrtb for JPEG.                           */

  row_size = cinfo.image_width * cinfo.input_components;
  buffer   = (JSAMPROW) tileBuffer + row_size * ( cinfo.image_height - 1 );

  while ( cinfo.next_scanline < cinfo.image_height ) {
    jpeg_write_scanlines( &cinfo, &buffer, 1 );
    buffer -= row_size;
  }

  /* Finish the compressor, then work out how large the written */
  /* JPEG data is, and release all memory for the JPEG Object   */

  jpeg_finish_compress( &cinfo );
  jpeg_data_size = jpeg_tile_size - dest_mgr.free_in_buffer;
  jpeg_destroy_compress( &cinfo );

  return jpeg_data_size;
}

/* Some NULL functions to use as callbacks for the source */
/* data manager in the tsmJPEGDecompressTile() function.  */

void null_init_source( j_decompress_ptr cinfo ) {}
boolean null_fill_input_buffer( j_decompress_ptr cinfo ) {return TRUE;}
boolean null_resync_to_restart( j_decompress_ptr cinfo, int d ) {return TRUE;}
void null_term_source( j_decompress_ptr cinfo ) {}
void null_skip_input_data( j_decompress_ptr cinfo, long num_bytes ) {
  cinfo->src->next_input_byte += num_bytes;
  cinfo->src->bytes_in_buffer -= num_bytes;
}

/*
 * Synopsis:
 *   int tsmJPEGDecompressTile( unsigned char *jpegBuffer, unsigned char
 *                              *tileBuffer, TileTspec *tile )
 *
 * Doc++ Description: *//** 
 *   Decompress a JPEG tile in memory.
 *
 *   This function will take two already allocated tiles (e.g. allocated
 *   with tsmMakeTileBuffer), where jpegBuffer is assumed to contain the
 *   JPEG-compressed data for a tile. The function will then decompress
 *   this image to a raw bitmap, which it will write to tileBuffer.
 *
 *   You also provide a TileTspec structure (e.g. pyramid->tileTspec) to
 *   provide information about the size of both tile buffers (i.e. both
 *   tile buffers are deemed to be the same size).
 *
 *   N.B. This function will write the raw data in reverse row order so that
 *   we automatically convert the data ordering from JPEG's lrtb to TV's
 *   lrbt format - no need to flip the tile afterwards.
 *
 * @param    jpegBuffer the input buffer with the JPEG data in it
 * @param    tileBuffer the tile buffer to write the raw (lrbt) bitmap to
 * @param    tile       the TileTspec giving both tile's dimensions
 *
 * @return   TRUE if all went well, FALSE if an error occurred
 *
 * @author   Martin Reddy
 * @version  7 November 1997
 *
 * @see      tsmJPEGCompressTile, tsmMakeTileBuffer, TileTspec
 *
 */

int
tsmJPEGDecompressTile( unsigned char *jpegBuffer, unsigned char *tileBuffer,
		       TileTspec *tile )
{
  const char                    *funcName = "tsmJPEGDecompressTile";
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr         jerr;
  struct jpeg_source_mgr        src_mgr;
  JSAMPROW                      buffer;
  int                           jpeg_tile_size, row_size;

  if ( tileBuffer == NULL || jpegBuffer == NULL ) {
    tsmError( funcName, "input and output buffers must be non-NULL!" );
    return FALSE;
  }

  jpeg_tile_size = tile->totalPixels[0] * tile->totalPixels[1] *
                   tile->numComponentsPixel * tile->numBytesPerComponent;

  /* Initialise the JPEG decompression object with default error handling. */

  cinfo.err = jpeg_std_error( &jerr );
  jpeg_create_decompress( &cinfo );

  /* set up the data source manager to read from jpegBuffer */

  src_mgr.next_input_byte   = jpegBuffer;
  src_mgr.bytes_in_buffer   = jpeg_tile_size;
  src_mgr.init_source       = null_init_source;
  src_mgr.fill_input_buffer = null_fill_input_buffer;
  src_mgr.skip_input_data   = null_skip_input_data;
  src_mgr.resync_to_restart = null_resync_to_restart;
  src_mgr.term_source       = null_term_source;

  cinfo.src = &src_mgr;

  /* Read the JPEG header and set default decompression parameters */

  jpeg_read_header( &cinfo, TRUE );

  /* Check that the tileBuffer is the right size. Currently we just do  */
  /* this by checking the number of components per pixel because a tile */
  /* is hardcoded to a particular dimension based upon this value.      */

  if ( cinfo.num_components != tile->numComponentsPixel ) {
    tsmError( funcName, "Decompress buffer is wrong size! (%d != %d comps)",
	      cinfo.num_components, tile->numComponentsPixel );
    return FALSE;
  }

  /* Start decompressor */

  jpeg_start_decompress( &cinfo );

  /* Decompress the data into tileBuffer, starting with the last row  */
  /* and working back to the first row because we also need to switch */
  /* the origin from JPEG's lrtb to TV's lrbt                         */

  row_size = cinfo.image_width * cinfo.num_components;
  buffer   = (JSAMPROW) tileBuffer + row_size * ( cinfo.image_height - 1 );

  while ( cinfo.output_scanline < cinfo.output_height ) {
    jpeg_read_scanlines( &cinfo, &buffer, 1 );
    buffer -= row_size;
  }

  /* Finish decompression and release memory. */

  jpeg_finish_decompress( &cinfo );
  jpeg_destroy_decompress( &cinfo );

  return TRUE;
}

