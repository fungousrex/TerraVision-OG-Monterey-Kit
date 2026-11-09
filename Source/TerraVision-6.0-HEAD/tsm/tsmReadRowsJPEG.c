/*
 * Name: tsmReadRowsJPEG.c
 *
 * Description: 
 *   Import functions for supporting the JFIF/JPEG image format.
 *
 * Function List:
 *
 * Revision History:
 *   Created Jul 2000 by Yong Tze Chi <chi@ai.sri.com>
 *
 *   $Id: tsmReadRowsJPEG.c,v 1.7.4.1 2002/03/14 05:22:58 reddy Exp $
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
#include "tsmReadRowsJPEG.h"



TsmReadRowsJPEG *tsmReadRowsJPEG_new(const char *filename) {
  static const char *funcName = "tsmReadRowsJPEG_new";
  TsmReadRowsJPEG *this;

  if ((this = tsmMemMalloc(sizeof(TsmReadRowsJPEG))) == NULL) {
    tsmError(funcName, "Couldn't create TsmReadRowsJPEG instance.");
    return NULL;
  }

  if ((this->fin = fopen(filename, "rb")) == NULL) {
    tsmError(funcName, "Couldn't open file %s", filename);
    tsmReadRowsJPEG_destroy(&this);
    return NULL;
  }

  if (!tsmReadRowsJPEG_readHeader(this)) {
    tsmError(funcName, "Couldn't read JPEG file header.");
    tsmReadRowsJPEG_destroy(&this);
    return NULL;
  }

  /* Prepare scanline buffer */
  this->compsPerPixel = this->jinfo.out_color_components;
  this->bytesPerComp = 1;

  jpeg_start_decompress(&this->jinfo);

  return this;
}

void tsmReadRowsJPEG_destroy(TsmReadRowsJPEG **this) {
  if (this != NULL && *this != NULL) {
    jpeg_finish_decompress(&(*this)->jinfo);
    jpeg_destroy_decompress(&(*this)->jinfo);
    tsmMemFree(*this);
    *this = NULL;
  }
}

static int tsmReadRowsJPEG_readHeader(TsmReadRowsJPEG *this) {
  static const char *funcName = "tsmReadRowsJPEG_readHeader";

  CHECK_NOT_NULL(this, FALSE);

  this->jinfo.err = jpeg_std_error(&this->jerr);
  jpeg_create_decompress(&this->jinfo);

  jpeg_stdio_src(&this->jinfo, this->fin);

  jpeg_read_header(&this->jinfo, TRUE);

  this->imageWidth = this->jinfo.image_width;
  this->imageHeight = this->jinfo.image_height;

  return TRUE;
}

int tsmReadRowsJPEG_read(void *buffer, int startRow, int numberOfRows, int numberOfBytes, void *closure) {
  static const char *funcName = "tsmReadRowsJPEG_read";
  TsmReadRowsJPEG *this = (TsmReadRowsJPEG *) closure;
  int scanlines;
  int n;
  unsigned char *p = (unsigned char *) buffer;

  CHECK_NOT_NULL(this, FALSE);
  CHECK_NOT_NULL(buffer, FALSE);

  for (scanlines = 0; scanlines<numberOfRows;
       scanlines += (n=jpeg_read_scanlines(&this->jinfo, (JSAMPARRAY) &p, 1)),
       p += this->imageWidth * this->jinfo.output_components * n);

  return TRUE;
}

int tsmParseCreateParams_JPEG(char *image_name, TsmTileParams *tile, TsmTspecParams *tspec) {
  static const char *funcName = "tsmParseCreateParams_JPEG";
  struct jpeg_decompress_struct jinfo;
  struct jpeg_error_mgr jerr;
  FILE *fin;

  if ((fin = fopen(image_name, "rb")) == NULL) {
    tsmError(funcName, "Couldn't open file named %s.", image_name);
    return FALSE;
  }

  jinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&jinfo);

  jpeg_stdio_src(&jinfo, fin);

  jpeg_read_header(&jinfo, TRUE);

  tile->width = tspec->width = jinfo.image_width;
  tile->height = tspec->height = jinfo.image_height;
  tile->comps = tspec->comps = jinfo.num_components;
  tile->bytespercomp = 1;

  jpeg_destroy_decompress(&jinfo);
  fclose(fin);

  return TRUE;
}
