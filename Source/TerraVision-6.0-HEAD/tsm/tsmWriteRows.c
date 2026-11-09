/*
 * Name: tsmWriteRows.c
 *
 * Description: 
 *   Defines the callback function to be used with
 *   tsmDownSample_inmemory for writing out down-sampled tiles into
 *   the TerraVision dataset. Also support debugging mode that writes
 *   out tiles into individual PPM files.
 *
 *   tsmWriteRows_write is the function to be hooked up to
 *   tsmDownSample_inmemory.  Since all functions in this file,
 *   including tsmWriteRows_write is relying on an "object" (or
 *   closure) for information, we should create the object using the
 *   tsmWriteRows_new function first. The returned reference to the
 *   TsmWriteRows instance can then be passed around. For
 *   deinitialization, we should call tsmWriteRows_destroy.
 *
 *   Since we are supporting file format such as JPEG, GIF, and PBM,
 *   which do not store the image in LRBT order, and since the
 *   tsmWriteTile is expecting an LRBT format, we have to do some
 *   processing on each tile if the input format is non-LRBT. The lrbt
 *   field in the TsmWriteRows struct is used to indicate whether
 *   the input format is lrbt, and it's set to FALSE by
 *   default. Should the input format be in lrbt order, you should set
 *   this field to TRUE after getting the instance back from
 *   tsmWriteRows_new.
 *
 * Function List:
 *
 * Revision History:
 *   Created 8 Aug 2000 by Yong Tze Chi <chi@ai.sri.com>
 *
 *   $Id: tsmWriteRows.c,v 1.20.2.4 2002/07/18 18:29:49 reddy Exp $
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

#include <limits.h>
#include "tsmWriteRows.h"
#include "tsmInternal.h"
#include <prnetdb.h>

/* #define DEBUG */


/* +-------------------------------------------------------------------------------------------+ *
 * | Name: tsmWriteRows_new                                                                    | *
 * | Description:                                                                              | *
 * |   Creates an instance of TsmWriteRows and initializes it.                                 | *
 * | Parameters:                                                                               | *
 * |   TsmConnection connection       The connection parameters used for initialization.       | *
 * | Return value:                                                                             | *
 * |   Pointer to the initialized TsmWriteRows instance.                                       | *
 * +-------------------------------------------------------------------------------------------+ */
TsmWriteRows *tsmWriteRows_new(TsmConnection connection, TsmTileParams *tile) {
  static const char *funcName = "tsmWriteRows_new";
  TsmWriteRows *this;
  int levels;
  int i;

  CHECK_NOT_NULL(connection, NULL);

  if ((this = tsmMemMalloc(sizeof(TsmWriteRows))) == NULL) {
    tsmError(funcName, "Couldn't create TsmWriteRows instance.");
    return NULL;
  }

  this->connection = connection;
  if ((this->pyramid = (Pyramid *) tsmGet(this->connection, TSM_DATASET_TSPEC)) == NULL) {
    tsmError(funcName, "Couldn't retrieve dataset's tspec.");
    tsmWriteRows_destroy(&this);
    return NULL;
  }

  this->tileSize[0] = this->pyramid->tileTspec.numPixelsX;
  this->tileSize[1] = this->pyramid->tileTspec.numPixelsY;
  this->overlap[0] = this->pyramid->tileTspec.totalPixels[0] - this->tileSize[0];
  this->overlap[1] = this->pyramid->tileTspec.totalPixels[0] - this->tileSize[1];
  this->tileTotalPixel[0] = this->tileSize[0] + this->overlap[0];
  this->tileTotalPixel[1] = this->tileSize[1] + this->overlap[1];
  this->compsPerPixel = tile->comps;
  this->bytesPerComp = tile->bytespercomp;
  this->bytesPerPixel = this->compsPerPixel * this->bytesPerComp;
  this->outBytesPerComp = tile->out_bytespercomp;
  this->levels = levels = this->pyramid->maxLevel - this->pyramid->minLevel + 1;
  this->lrbt = FALSE;
  this->resample_error = 0;

  if ((this->buffer = tsmMemMalloc(sizeof(unsigned char*) * levels)) == NULL) {
    tsmError(funcName, "Couldn't allocate memory for buffer.");
    tsmWriteRows_destroy(&this);
    return NULL;
  }
  for (i=0; i<levels; i++) {
    SimpleTileSet *tileset =
      &this->pyramid->tspecLevel.val[this->pyramid->minLevel+i];
    int tmpsize = sizeof(unsigned char) * tileset->numValidPixel[0]
      * this->tileTotalPixel[1] * this->bytesPerPixel;
    
    this->buffer[i] = tsmMemMalloc( tmpsize );

    if ( this->buffer[i] == NULL) {
      tsmError(funcName, "Couldn't allocate memory for image buffer.");
      tsmWriteRows_destroy(&this);
      return NULL;
    }
  }

  /* Allocate tile buffer */
  if ((this->tileBuffer = tsmMemMalloc(sizeof(char) * this->tileTotalPixel[0] * this->tileTotalPixel[1] * this->bytesPerPixel)) == NULL) {
    tsmError(funcName, "Couldn't allocate memory for tile buffer.");
    tsmWriteRows_destroy(&this);
    return NULL;
  }

  /* Allocate conversion buffer */
  if ((this->convBuffer = tsmMemMalloc(sizeof(char) * this->tileTotalPixel[0] * this->tileTotalPixel[1]
       * this->compsPerPixel * this->outBytesPerComp)) == NULL) {
    tsmError(funcName, "Couldn't allocate memory for tile buffer.");
    tsmWriteRows_destroy(&this);
    return NULL;
  }

  return this;
}

/* +-------------------------------------------------------------------------------------------+ *
 * | Name: tsmWriteRows_destroy                                                                | *
 * | Description:                                                                              | *
 * |   Free up the memory allocated during initialization, and set the pointer passed in to    | *
 * |   NULL.                                                                                   | *
 * | Parameters:                                                                               | *
 * |   TsmWriteRows **this     Reference to pointer of TsmWriteRows instance.                  | *
 * | Return value:                                                                             | *
 * |   None.                                                                                   | *
 * +-------------------------------------------------------------------------------------------+ */
void tsmWriteRows_destroy(TsmWriteRows **this) {
  int i;
  if (this != NULL && *this != NULL) {
    if ( (*this)->resample_error > 0 )
      fprintf( stderr, "WARNING: %d overflows occured during the "
               "resampling of data values\n", (*this)->resample_error );

    if ((*this)->buffer != NULL) {
      for (i=0; i<(*this)->levels; i++) {
        if ((*this)->buffer[i] != NULL) {
          tsmMemFree((*this)->buffer[i]);
        }
      }
      tsmMemFree((*this)->buffer);
    }
    if ((*this)->tileBuffer != NULL) tsmMemFree((*this)->tileBuffer);
    if ((*this)->convBuffer != NULL) tsmMemFree((*this)->convBuffer);
    tsmMemFree(*this);
    *this = NULL;
  }
}


/* +-------------------------------------------------------------------------------------------+ *
 * | Name: tsmWriteRows_write                                                                  | *
 * | Description:                                                                              | *
 * |   This function is called by tsmDownSample_inmemory when it is ready to write out a row   | *
 * |   of tiles. It can handle an lrbt or non-lrbt image format, with or without overlaping    | *
 * |   region requested.                                                                       | *
 * |                                                                                           | *
 * |   For a non-overlapping lrbt image, it simply writes out the input buffer as it comes in. | *
 * |   For an overlapping lrbt image, it store up the input buffer into the cache buffer and   | *
 * |   only writes out the cache buffer once it has filled up the overlapping region.          | *
 * |   For a lrbt image, it splits the input buffer into two parts, pad the first (top) part   | *
 * |   to the bottom of the cache buffer and then writes it out. After that it would store up  | *
 * |   the second (bottom) part of the input buffer into the cache buffer for later use.       | *
 * | Parameters:                                                                               | *
 * |   int level            The level the tiles belong to.                                     | *
 * |   const void *buffer   The input buffer.  This is a numberOfRows x imageWidth buffer.     | *
 * |   int startRow         The starting row number, zero being the first.                     | *
 * |   int numberOfRows     Number of rows contained in the buffer.  Should be equal to        | *
 * |                        tileSize[1] except for the last row of tiles.                      | *
 * |   int numberOfBytes    Number of bytes in the buffer.                                     | *
 * |   void *closure        Pointer to the TsmWriteRows instance.                              | *
 * | Return value:                                                                             | *
 * |   TRUE if everything AOK, FALSE otherwise.                                                | *
 * +-------------------------------------------------------------------------------------------+ */
int tsmWriteRows_write(int level, const void *buffer, int startRow, int numberOfRows, int numberOfBytes, void *closure) {
  static const char *funcName = "tsmWriteRows_write";
  static int progress = 0;
  static int total = -1;
  TsmWriteRows *this = (TsmWriteRows *) closure;
  int ty;
  int size;
  int offset;
  SimpleTileSet *tileset;
  int h1, h2;

  CHECK_NOT_NULL(this, FALSE);

  if (level < this->pyramid->minLevel || level > this->pyramid->maxLevel) {
    char buf[80];
    sprintf(buf, "Invalid level %d (min = %d max = %d)", level,
	    this->pyramid->minLevel, this->pyramid->maxLevel);
    return FALSE;
  }

#if 0
  if (this->bytesPerComp == 4 && this->compsPerPixel == 1) {
    PRInt32 const *px = (PRInt32*)buffer;
    PRInt32 const *pend = (PRInt32*) &((char*)buffer)[numberOfRows*numberOfBytes];
    for (; px < pend; ++px) {
      if (px[0] > 10000) {
	int off = px - (PRInt32*)buffer;
	int pxPerRow = (numberOfBytes/4);
	tsmError (funcName, "L%d: out[%d,%d] = %d!",
		  level, off%pxPerRow, off/pxPerRow+startRow,
		  px[0]);
      }
    }
  }
#endif

  if (total==-1) {
    int i;
    for (i=this->pyramid->minLevel; i<=this->pyramid->maxLevel; i++) {
      total += this->pyramid->tspecLevel.val[i].numValidPixel[1];
    }
    progress = 0;
  }

  tileset = &this->pyramid->tspecLevel.val[this->pyramid->minLevel+level];

  progress += numberOfRows;
  if (progress < total) {
    printf("Generating data set... %6.2f%%\r", progress*100.0/total);
  }
  else {
    printf("Generating data set... done.        \n");
  }
  fflush(stdout);

  ty = startRow / this->tileSize[1];

  /* If we are dealing with a non lrbt format, we need to split the entire rows
     of data into two: the first part padded to the previous rows, and the second
     part stored in buffer to be written out later.  Here we are figuring out the
     height of each part */
  h1 = (tileset->numValidPixel[1] % this->tileSize[1]);
  if (h1==0) h1 = this->tileSize[1];
  h2 = this->tileSize[1] - h1 + this->overlap[1];

  /* Check whether there is an overlap region needed */
  if (this->overlap[1] > 0) {
    /* Yes, overlap region required */

    if (ty==0) {
      if (ty==tileset->maxTile[1]) {
        /* If this is the first and last row of tiles, just write out the tiles and we are done. */
        return tsmWriteRows_writeTiles(this, buffer, startRow, numberOfRows, level);
      }
      else if (!this->lrbt) {
        /* If this is not an LRBT image, we need to pad black region for the first row of tiles.
           The tsmWriteRows_writeTiles function will do that for us, and so just make the call. */
        tsmWriteRows_writeTiles(this, buffer, startRow, h1, level);

        /* Then, save the second part for later use */
        size = h2 * tileset->numValidPixel[0] * this->bytesPerPixel;
        offset = (h1-this->overlap[1]) * tileset->numValidPixel[0] * this->bytesPerPixel;
        memcpy(this->buffer[level], ((char*)buffer)+offset, size);

        return TRUE;
      }
      else {
        /* If this is the first row of tiles, but not the last row, just copy the input
           into the cache buffer and return TRUE, since we can't write out anything until
           we have obtained the first this->overlap[1] lines of data from the next row of tiles. */
        size = numberOfRows * tileset->numValidPixel[0] * this->bytesPerPixel;
        memcpy(this->buffer[level], buffer, size);
        return TRUE;
      }
    }

    if (!this->lrbt) {
      /* If this is a non-lrbt image, we need to pad the first part into the bottom of the cache buffer
         and write the cache buffer out. */
      size = h1 * tileset->numValidPixel[0] * this->bytesPerPixel;
      offset = h2 * tileset->numValidPixel[0] * this->bytesPerPixel;
      memcpy(this->buffer[level]+offset, buffer, size);

      tsmWriteRows_writeTiles(this, (const uchar *)this->buffer[level],
			      startRow, this->tileTotalPixel[1], level);

      /* Also, copy the second part into the beginning of the cache buffer, if this is not the
         last rows of data. */
      if (ty<tileset->maxTile[1]) {
        size = h2 * tileset->numValidPixel[0] * this->bytesPerPixel;
        offset = (h1-this->overlap[1]) * tileset->numValidPixel[0] * this->bytesPerPixel;
        memcpy(this->buffer[level], ((char*)buffer)+offset, size);
      }
    }
    else {
      /* If this is not the first row of tiles, then copy this->overlap[1] lines
         from the input buffer into the cache buffer, and then set outputBuffer to point
         to the cache buffer so that we can write it out later on. */
      offset = tileset->numValidPixel[0] * this->tileSize[1] * this->bytesPerPixel;
      size = tileset->numValidPixel[0] * this->overlap[1] * this->bytesPerPixel;
      memcpy(this->buffer[level]+offset, buffer, size);
      tsmWriteRows_writeTiles(this, (const uchar *)this->buffer[level],
			      startRow-this->tileSize[1],
			      this->tileTotalPixel[1], level);

      if (ty==tileset->maxTile[1]) {
        /* If this is the last row of tiles, we should also write out
           the input buffer, since there's not going to be another call
           to this function */
        tsmWriteRows_writeTiles(this, buffer, startRow, numberOfRows, level);
      }
      else {
        /* Otherwise, just copy the input buffer into the cache buffer */
        size = numberOfRows * tileset->numValidPixel[0] * this->bytesPerPixel;
        memcpy(this->buffer[level], buffer, size);
      }
    }
  }
  else {
    /* No overlap region, just write out the input buffer and we are done */
    if (this->lrbt) {
      /* It's a lrbt image, so just write out the input buffer */
      tsmWriteRows_writeTiles(this, buffer, startRow, numberOfRows, level);
    }
    else {
      /* Need special care for non-lrbt image */

      if (ty==0) {
        /* If it's the first rows, just write out the first part */
        tsmWriteRows_writeTiles(this, buffer, startRow, h1, level);
      }
      else {
        /* Otherwise, need to pad the first part into the cache buffer first before
           writting out. */
        size = h1 * tileset->numValidPixel[0] * this->bytesPerPixel;
        offset = h2 * tileset->numValidPixel[0] * this->bytesPerPixel;
        memcpy(this->buffer[level]+offset, buffer, size);
        tsmWriteRows_writeTiles(this, (const uchar *)this->buffer[level],
				startRow, this->tileTotalPixel[1], level);
      }

      /* Also, save the second part into the cache buffer, if it's not the last rows of
         data. */
      if (ty<tileset->maxTile[1]) {
        size = h2 * tileset->numValidPixel[0] * this->bytesPerPixel;
        offset = h1 * tileset->numValidPixel[0] * this->bytesPerPixel;
        memcpy(this->buffer[level], ((char*)buffer)+offset, size);
      }
    }
  }

  return TRUE;
}

/* +-------------------------------------------------------------------------------------------+ *
 * | Name: tsmWriteRows_writeTiles                                                             | *
 * | Description:                                                                              | *
 * |   Split up the buffer passed in (which is assume to be made up of a row of tiles), and    | *
 * |   writes out each individual tile. Takes into consideration of lrbt or non-lrbt image     | *
 * |   format.  For lrbt format, the rows are copied in the same order of the input buffer,    | *
 * |   whereas for non-lrbt format, the rows are copied in the reverse order.  Calls           | *
 * |   tsmWriteRows_writeTile to actually write out a processed tile.                          | *
 * | Parameters:                                                                               | *
 * |   TsmWriteRows *this            The TsmWriteRows instance.                                | *
 * |   const unsigned char *buffer   Input buffer. This is a numberOfRows x imageWidth image.  | *
 * |   int startRow                  Starting row number, counting from the top of the whole   | *
 * |                                 image.                                                    | *
 * |   int numberOfRows              Number of rows in the buffer.                             | *
 * |   int level                     The level the tiles belong to.                            | *
 * | Return value:                                                                             | *
 * |   TRUE if successful, FALSE otherwise.                                                    | *
 * +-------------------------------------------------------------------------------------------+ */
static int tsmWriteRows_writeTiles(TsmWriteRows *this, const unsigned char *buffer, int startRow, int numberOfRows, int level) {
  static const char *funcName = "tsmWriteRows_writeTiles";
  SimpleTileSet *tileset;
  int tx, ty;
  int numPixelsX;
  int j;
  unsigned char *p1;
  const unsigned char *p2;

  CHECK_NOT_NULL(this, FALSE);
  CHECK_NOT_NULL(buffer, FALSE);

  if (level < this->pyramid->minLevel || level > this->pyramid->maxLevel) {
    char buf[80];
    sprintf(buf, "Invalid level %d (min = %d max = %d)", level,
	    this->pyramid->minLevel, this->pyramid->maxLevel);
    return FALSE;
  }

  tileset = &this->pyramid->tspecLevel.val[this->pyramid->minLevel+level];

  ty = startRow / this->tileSize[1];

  for (tx=0; tx<=tileset->maxTile[0]; tx++) {
    numPixelsX = (tx==tileset->maxTile[0])
      ? tileset->numValidPixel[0] - tx * this->tileSize[0]
      : this->tileTotalPixel[0];
    memset(this->tileBuffer, 0, this->tileTotalPixel[0] * this->tileTotalPixel[1] * this->bytesPerPixel);

    if (this->lrbt) {
      for (j=0, p1=this->tileBuffer, p2=buffer+tx*this->tileSize[0]*this->bytesPerPixel;
           j<numberOfRows;
           j++, p1+=this->tileTotalPixel[0]*this->bytesPerPixel, p2+=tileset->numValidPixel[0]*this->bytesPerPixel) {
        memcpy(p1, p2, numPixelsX * this->bytesPerPixel);
      }
      if (!tsmWriteRows_writeTile(this, tx, ty, level, this->tileBuffer)) {
        tsmError(funcName, "Failed to write out tile (%d, %d) of level %d.", tx, ty, level);
        return FALSE;
      }
    }
    else {
      for (j=0, p1=this->tileBuffer+(numberOfRows-1)*this->tileTotalPixel[0]*this->bytesPerPixel,
             p2=buffer+tx*this->tileSize[0]*this->bytesPerPixel;
           j<numberOfRows;
           j++, p1-=this->tileTotalPixel[0]*this->bytesPerPixel,
             p2+=tileset->numValidPixel[0]*this->bytesPerPixel) {

        memcpy(p1, p2, numPixelsX * this->bytesPerPixel);

      }
      if (!tsmWriteRows_writeTile(this, tx, tileset->maxTile[1]-ty, level, this->tileBuffer)) {
        tsmError(funcName, "Failed to write out tile (%d, %d) of level %d.", tx, ty, level);
        return FALSE;
      }
    }
  }

  return TRUE;
}

/* +-------------------------------------------------------------------------------------------+ *
 * | Name: tsmWriteRows_createTileFile                                                         | *
 * | Description:                                                                              | *
 * |   Generates the output tile filename to be used for the specified tile.  Also return a    | *
 * |   file handle to that file. This function is only used when doing debugging. Normal       | *
 * |   operation of this module should call tsmWriteTile to output a tile.                     | *
 * | Parameters:                                                                               | *
 * |   TsmWriteRows *this     The TsmWriteRows instance.                                       | *
 * |   int tx                                                                                  | *
 * |   int ty                 Tile coordinate.                                                 | *
 * |   int level              Level of the tile.                                               | *
 * |   unsigned char *buffer  Buffer where the generated filename will be stored.              | *
 * | Return value:                                                                             | *
 * |   A file handle ready to accept data. Returns NULL if there's an error creating the file. | *
 * +-------------------------------------------------------------------------------------------+ */
static FILE *tsmWriteRows_createTileFile(TsmWriteRows *this, int tx, int ty, int level, unsigned char *buffer) {
  /* static const char *funcName = "tsmWriteRows_createTileFile"; */

  sprintf((char *)buffer, "p%dp%dp%d.ppm", level, tx, ty);
  return fopen((char *)buffer, "wb");
}

/* +-------------------------------------------------------------------------------------------+ *
 * | Name: tsmWriteRows_writeTile                                                              | *
 * | Description:                                                                              | *
 * |   Output a tile. Calls tsmWriteTile to do the job. In debugging mode, writes out to       | *
 * |   individual .PPM file instead.                                                           | *
 * | Parameters:                                                                               | *
 * |   TsmWriteRows *this             The TsmWriteRows instance.                               | *
 * |   int tx, ty                     Tile coordinate.                                         | *
 * |   int level                      Level of the tile.                                       | *
 * |   const unsigned char *byffer    Data of the tile.                                        | *
 * | Return value:                                                                             | *
 * |   TRUE if everything is OK, or FALSE otherwise.                                           | *
 * +-------------------------------------------------------------------------------------------+ */
static int tsmWriteRows_writeTile(TsmWriteRows *this, int tx, int ty, int level, const unsigned char *buffer) {
  static const char *funcName = "tsmWriteRows_writeTile";
  int rv;
#ifdef DEBUG
  FILE *fout;
  unsigned char filename[256];
#endif

  CHECK_NOT_NULL(this, FALSE);
  CHECK_NOT_NULL(buffer, FALSE);

#ifdef DEBUG
  if ((fout = tsmWriteRows_createTileFile(this, tx, ty, level, filename)) == NULL) {
    tsmError(funcName, "Couldn't create file for output.\n");
    return FALSE;
  }

  if (this->compsPerPixel==1)
    fprintf(fout, "P5\n%d %d\n255\n", this->tileTotalPixel[0], this->tileTotalPixel[1]);
  else
    fprintf(fout, "P6\n%d %d\n255\n", this->tileTotalPixel[0], this->tileTotalPixel[1]);

  if (this->bytesPerComp==1) {
    fwrite(buffer, sizeof(unsigned char) * this->bytesPerPixel, this->tileTotalPixel[0] * this->tileTotalPixel[1], fout);
  }
  else {
    int i;
    int size = this->tileTotalPixel[0] * this->tileTotalPixel[1];

    /* printf("Writing out %d bytes per component.\n", this->bytesPerComp); */

    if (this->bytesPerComp==sizeof(short)) {
      short *buff = (short *) buffer;
      for (i=0; i<size; i++) {
        fprintf(fout, "%c", (unsigned char) buff[i]);
        /* fprintf(fout, "%c", (unsigned char) (((float)buff[i]) * (float) UCHAR_MAX / (float) SHRT_MAX)); */
      }
    }
    else if (this->bytesPerComp=sizeof(int)) {
      int *buff = (int *) buffer;
      for (i=0; i<size; i++) {
        fprintf(fout, "%c", (unsigned char) (((float)buff[i]) * (float) UCHAR_MAX / (float) INT_MAX));
      }
    }
    else if (this->bytesPerComp=sizeof(long)) {
      long *buff = (long *) buffer;
      for (i=0; i<size; i++) {
        fprintf(fout, "%c", (unsigned char) (((float)buff[i]) * (float) UCHAR_MAX / (float) LONG_MAX));
      }
    }
  }

  fclose(fout);

  return TRUE;
#else
  if (this->bytesPerComp != this->outBytesPerComp) {
    if (!tsmWriteRows_convertTileFormat(this, buffer)) {
      tsmError(funcName, "Can't convert a %d byte component to a %d byte component.", this->bytesPerComp, this->outBytesPerComp);
      return FALSE;
    }
    rv = tsmWriteTile(this->connection, tx, ty, level, this->convBuffer);
    tsmUpdateTileStats(this->connection, this->convBuffer, level);
  }
  else {
    rv = tsmWriteTile(this->connection, tx, ty, level, (uchar *)buffer);
    tsmUpdateTileStats(this->connection, buffer, level);
  }
  return rv;
#endif
}

static int tsmWriteRows_convertTileFormat(TsmWriteRows *this, const void *inBuffer) {
  /* static const char *funcName = "tsmWriteRows_convertTileFormat"; */
  int i;
  int tileSize;

  tileSize = this->tileTotalPixel[0] * this->tileTotalPixel[1];

  if (this->bytesPerComp==this->outBytesPerComp) {
    memcpy(this->convBuffer, inBuffer, tileSize * this->bytesPerPixel);
    return TRUE;
  }
  else if (this->bytesPerComp == sizeof(char)) {
    if (this->outBytesPerComp == sizeof(short int)) {
      for (i=0; i<tileSize; i++ ) {
        char orig = ((char *) inBuffer)[i];
        ((short int *) this->convBuffer)[i] = PR_htons( (short int) orig );
      }
    }
    else if (this->outBytesPerComp == sizeof(int)) {
      for (i=0; i<tileSize; i++ ) {
        char orig = ((char *) inBuffer)[i];
        ((int *) this->convBuffer)[i] = PR_htonl( (int) orig );
      }
    }
    else return FALSE;
  }
  else if (this->bytesPerComp == sizeof(short int)) {
    if (this->outBytesPerComp == sizeof(char)) {
      for (i=0; i<tileSize; i++) {
        short int orig = PR_ntohs( ((short int *) inBuffer)[i] );
        if ( orig > 0xee ) {
          orig = (short int) 0xee;
          this->resample_error++;
        }
        ((char *) this->convBuffer)[i] = (char) orig;
      }
    }
    else if (this->outBytesPerComp == sizeof(int)) {
      for (i=0; i<tileSize; i++) {
        short int orig = PR_ntohs( ((short int *) inBuffer)[i] );
        ((int *) this->convBuffer)[i] = PR_htonl( (int) orig );
      }
    }
    else return FALSE;
  }
  else if (this->bytesPerComp == sizeof(int)) {
    if (this->outBytesPerComp == sizeof(short int)) {
      for (i=0; i<tileSize; i++) {
        int orig = PR_ntohl( ((int *) inBuffer)[i] );
        if ( orig > 0xeffe ) {
          orig = (int) 0xeffe;
          this->resample_error++;
        }
        ((short int *) this->convBuffer)[i] = PR_htons( (short int) orig );
      }
    }
    else return FALSE;
  }

  return TRUE;
}
