/*
 * Name: tsmReadRowsPBM.c
 *
 * Description: 
 *   Import functions for supporting the PBM image formats (PPM and PGM)
 *
 * Function List:
 *
 * Revision History:
 *   Created Jul 2000 by Yong Tze Chi <chi@ai.sri.com>
 *
 *   $Id: tsmReadRowsPBM.c,v 1.4 2000/10/14 00:53:58 reddy Exp $
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

#include <stdio.h>
#include "tsmInternal.h"
#include "tsmReadRowsPBM.h"

#define PBM_HEADER_UNSUPPORTED 0
#define PBM_HEADER_PGM         1
#define PBM_HEADER_PPM         1

static int tsmReadRowsPBM_initRowOffsets(TsmReadRowsPBM *this);
static int tsmReadRowsPBM_skipWhiteSpaces(TsmReadRowsPBM *this);
static int tsmReadRowsPBM_readHeader(TsmReadRowsPBM *this);
static int tsmReadRowsPBM_readAll(TsmReadRowsPBM *this);


TsmReadRowsPBM *tsmReadRowsPBM_new(const char *filename) {
  static const char* funcName = "tsmReadRowsPBM_new";
  TsmReadRowsPBM *this;

  if ((this = tsmMemMalloc(sizeof(TsmReadRowsPBM))) == NULL) {
    tsmError(funcName, "Couldn't create TsmReadRowsPBM instance.");
    return NULL;
  }

  if ((this->fin = fopen(filename, "rb")) == NULL) {
    tsmError(funcName, "Couldn't open file %s.", filename);
    tsmReadRowsPBM_destroy(&this);
    return NULL;
  }

  if (!tsmReadRowsPBM_readHeader(this)) {
    tsmError(funcName, "Couldn't read PBM file header.");
    tsmReadRowsPBM_destroy(&this);
    return NULL;
  }

  if (!tsmReadRowsPBM_initRowOffsets(this)) {
    tsmReadRowsPBM_destroy(&this);
    return NULL;
  }

  return this;
}

void tsmReadRowsPBM_destroy(TsmReadRowsPBM **this) {
  /* static const char *funcName = "tsmReadRowsPBM_destroy"; */
  if (this != NULL && *this != NULL) {
    if ((*this)->rowOffsets != NULL) tsmMemFree((*this)->rowOffsets);
    tsmMemFree(*this);
    *this = NULL;
  }
}

static int tsmReadRowsPBM_initRowOffsets(TsmReadRowsPBM *this) {
  static const char *funcName = "tsmReadRowsPBM_initRowOffsets";
  int value[3];
  int j, i;

  CHECK_NOT_NULL(this, FALSE);

  if (fseek(this->fin, this->dataOffset, SEEK_SET)==-1) {
    tsmError(funcName, "Couldn't seek to position %wd.", this->dataOffset);
    return FALSE;
  }

  if ((this->rowOffsets = tsmMemMalloc(sizeof(long) * this->imageHeight)) == NULL) {
    tsmError(funcName, "Couldn't allocate memory for row offsets.");
    return FALSE;
  }

  if (this->fileType == 2) {
    for (j=0; j<this->imageHeight; j++) {
      this->rowOffsets[j] = ftell(this->fin);
      for (i=0; i<this->imageWidth; i++) {
        if (fscanf(this->fin, "%d", &value[0]) != 1) {
          tsmError(funcName, "Premature end of file.");
          return FALSE;
        }
      }
    }
  }
  else if (this->fileType == 3) {
    for (j=0; j<this->imageHeight; j++) {
      this->rowOffsets[j] = ftell(this->fin);
      for (i=0; i<this->imageWidth; i++) {
        if (fscanf(this->fin, "%d %d %d", &value[0], &value[1], &value[2]) != 3) {
          tsmError(funcName, "Premature end of file.");
          return FALSE;
        }
      }
    }
  }
  else if (this->fileType == 5) {
    for (j=0; j<this->imageHeight; j++) {
      this->rowOffsets[j] = j * (long) sizeof(unsigned char) * this->imageWidth + this->dataOffset;
    }
  }
  else if (this->fileType == 6) {
    for (j=0; j<this->imageHeight; j++) {
      this->rowOffsets[j] = j * 3L * sizeof(unsigned char) * this->imageWidth + this->dataOffset;
    }
  }
  else return FALSE;

  return TRUE;
}

static int tsmReadRowsPBM_skipWhiteSpaces(TsmReadRowsPBM *this) {
  static const char *funcName = "tsmReadRowsPBM_skipWhiteSpaces";
  int ch;

  CHECK_NOT_NULL(this, FALSE);

  while ((ch = fgetc(this->fin)) != EOF) {
    if (ch == '#') {
      while ((ch = fgetc(this->fin)) != EOF && ch != '\n');
      if ((ch = fgetc(this->fin)) == EOF) return FALSE;
    }
    if (ch != ' ' && ch != '\t' && ch != '\n') {
      ungetc(ch, this->fin);
      return TRUE;
    }
  }

  return FALSE;
}

static int tsmReadRowsPBM_readHeader(TsmReadRowsPBM *this) {
  static const char *funcName = "tsmReadRowsPBM_readHeader";
  char header[2];

  CHECK_NOT_NULL(this, FALSE);

  if (fread(header, 1, 2, this->fin) != 2) {
    return FALSE;
  }

  if (header[0] != 'P' || !isdigit(header[1]) || (this->fileType = header[1]-'0') > 6) {
    return FALSE;
  }

  switch (this->fileType) {
  case 2:
  case 5:
    this->compsPerPixel = 1;
    this->bytesPerComp = 1;
    break;
  case 3:
  case 6:
    this->compsPerPixel = 3;
    this->bytesPerComp = 1;
    break;
  }

  tsmReadRowsPBM_skipWhiteSpaces(this);
  if (fscanf(this->fin, "%d", &this->imageWidth) != 1) return FALSE;
  
  tsmReadRowsPBM_skipWhiteSpaces(this);
  if (fscanf(this->fin, "%d", &this->imageHeight) != 1) return FALSE;

  tsmReadRowsPBM_skipWhiteSpaces(this);
  if (fscanf(this->fin, "%d", &this->maxValue) != 1) return FALSE;

  tsmReadRowsPBM_skipWhiteSpaces(this);

  this->dataOffset = ftell(this->fin);

  return TRUE;
}

int tsmReadRowsPBM_read(void *buffer, int startRow, int numberOfRows, int numberOfBytes, void *closure) {
  static const char *funcName = "tsmReadRowsPBM_read";
  TsmReadRowsPBM *this = (TsmReadRowsPBM *) closure;
  int value[3];
  int j, i;
  int offset;

  CHECK_NOT_NULL(closure, FALSE);
  CHECK_NOT_NULL(buffer, FALSE);

  if (ftell(this->fin) != this->rowOffsets[startRow]) {
    if (fseek(this->fin, this->rowOffsets[startRow], SEEK_SET)==-1) {
      tsmError(funcName, "Couldn't read rows %d to %d as requested.", startRow, startRow+numberOfRows-1);
      return FALSE;
    }
  }

  if (this->fileType == 2) {
    for (j=0, offset=0; j<numberOfRows; j++) {
      for (i=0; i<this->imageWidth; i++) {
        if (fscanf(this->fin, "%d", &value[0]) != 1) {
          tsmError(funcName, "Error encountered while reading PGM file.");
          return FALSE;
        }
        ((char *) buffer)[offset++] = (char) value[0];
      }
    }
  }
  else if (this->fileType == 3) {
    for (j=0, offset=0; j<numberOfRows; j++) {
      for (i=0; i<this->imageWidth; i++) {
        if (fscanf(this->fin, "%d %d %d", &value[0], &value[1], &value[2]) != 3) {
          tsmError(funcName, "Error encountered while reading PPM file.");
          return FALSE;
        }
        ((char *) buffer)[offset++] = (char) value[0];
        ((char *) buffer)[offset++] = (char) value[1];
        ((char *) buffer)[offset++] = (char) value[2];
      }
    }
  }
  else if (this->fileType == 5) {
    if ( fread(buffer, sizeof(unsigned char), this->imageWidth *
	       numberOfRows, this->fin) != this->imageWidth * numberOfRows) {
      tsmError(funcName, "Error encountered while reading PGM file." );
      return FALSE;
    }
  }
  else if (this->fileType == 6) {
    if (fread(buffer, sizeof(unsigned char) * 3, this->imageWidth *
	      numberOfRows, this->fin) != this->imageWidth * numberOfRows) {
      tsmError(funcName, "Error encountered while reading PPM file.");
      return FALSE;
    }
  }
  else return FALSE;

  return TRUE;
}

static int
tsmParsePBMHeader( FILE *handle, int *type, int *width, int *height,
		   int *maxval, int *data_offset )
{
  char buffer[512];

  /* rewind to the start of the file and then read the PBM header */

  rewind( handle );
  
  /* read the magic number: P5 = binary PGM, P6 = binary PPM */

  fgets( buffer, 512, handle );
  if ( strcmp( buffer, "P6\n" ) == 0 )
    *type = PBM_HEADER_PPM;
  else if ( strcmp( buffer, "P5\n" ) == 0 )
    *type = PBM_HEADER_PGM;
  else {
    *type = PBM_HEADER_UNSUPPORTED;
    return FALSE;
  }

  /* Read over any lines beginning with a '#' symbol */

  while ( ( buffer[0] = fgetc( handle ) ) == '#' ) {
    fgets( buffer, 512, handle );
  }
  ungetc( buffer[0], handle );

  /* Read in the image width, height, and maxval */

  fscanf( handle, "%d%d%d%c", width, height, maxval, buffer );

  *data_offset = ftell( handle );
  return TRUE;
}

int
tsmParseCreateParams_PPM( char *image_name, TsmTileParams *tile,
			  TsmTspecParams *tspec )
{
  static const char *funcName = "tsmParseCreateParams_PPM";
  FILE              *handle;
  int               type, maxval, file_offset;

  /* Open the input and output files */

  if ( ( handle = fopen( image_name, "rb" ) ) == NULL ) {
    tsmError( funcName, "Couldn't open ppm image file: %s", image_name );
    return FALSE;
  }

  /* read the PBM header from the input file and check that it is PPM */

  tsmParsePBMHeader( handle, &type, &tile->width, &tile->height, 
		     &maxval, &file_offset );

  fclose( handle );
  
  if ( type != PBM_HEADER_PPM ) {
    tsmError( funcName, "Input image is not a binary PPM format file" );
    fclose( handle );
    return FALSE;
  }

  tspec->width  = tile->width;
  tspec->height = tile->height;
  tspec->comps  = tile->comps = 3;
  return TRUE;
}

int
tsmParseCreateParams_PGM( char *image_name, TsmTileParams *tile,
			  TsmTspecParams *tspec )
{
  static const char *funcName = "tsmParseCreateParams_PGM";
  FILE              *handle;
  int               type, maxval, file_offset;

  /* Open the input and output files */

  if ( ( handle = fopen( image_name, "rb" ) ) == NULL ) {
    tsmError( funcName, "Couldn't open pgm image file: %s", image_name );
    return FALSE;
  }

  /* read the PBM header from the input file and check that it is PPM */

  tsmParsePBMHeader( handle, &type, &tile->width, &tile->height, 
		     &maxval, &file_offset );
  fclose( handle );
  
  if ( type != PBM_HEADER_PGM ) {
    tsmError( funcName, "Input image is not a binary PGM format file" );
    fclose( handle );
    return FALSE;
  }

  tspec->width  = tile->width;
  tspec->height = tile->height;
  tspec->comps  = tile->comps = 1;
  return TRUE;
}
