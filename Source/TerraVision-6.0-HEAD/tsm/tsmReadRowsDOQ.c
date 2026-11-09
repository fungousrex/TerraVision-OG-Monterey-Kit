/*
 * Name:  tsmReadRowsDOQ.c
 *
 * Description: 
 *   Import functions for the USGS DOQ file format, used for 1-meter
 *   black/white satellite photography of the US.
 *
 * Function List:
 *
 * Revision History:
 *   Created Jul 2000 by Yong Tze Chi <chi@ai.sri.com>
 *
 *   $Id: tsmReadRowsDOQ.c,v 1.3.4.1 2002/03/14 05:22:58 reddy Exp $
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
#include "tsmReadRowsDOQ.h"



TsmReadRowsDOQ *tsmReadRowsDOQ_new(const char *filename) {
  static const char *funcName = "tsmReadRowsDOQ_new";
  TsmReadRowsDOQ *this;

  if ((this = tsmMemMalloc(sizeof(TsmReadRowsDOQ))) == NULL) {
    tsmError(funcName, "Couldn't create TsmReadRowsDOQ instance.");
    return NULL;
  }

  if ((this->fin = fopen(filename, "rb")) == NULL) {
    tsmError(funcName, "Couldn't open file %s", filename);
    tsmReadRowsDOQ_destroy(&this);
    return NULL;
  }

  if (!tsmReadRowsDOQ_readHeader(this)) {
    tsmError(funcName, "Couldn't read DOQ file header.");
    tsmReadRowsDOQ_destroy(&this);
    return NULL;
  }

  /* Prepare scanline buffer */
  this->compsPerPixel = this->jinfo.num_components;
  this->bytesPerComp = 1;

  jpeg_start_decompress(&this->jinfo);

  return this;
}

void tsmReadRowsDOQ_destroy(TsmReadRowsDOQ **this) {
  if (this != NULL && *this != NULL) {
    jpeg_finish_decompress(&(*this)->jinfo);
    jpeg_destroy_decompress(&(*this)->jinfo);
    tsmMemFree(*this);
    *this = NULL;
  }
}

static int tsmReadRowsDOQ_readHeader(TsmReadRowsDOQ *this) {
  static const char *funcName = "tsmReadRowsDOQ_readHeader";

  CHECK_NOT_NULL(this, FALSE);

  this->jinfo.err = jpeg_std_error(&this->jerr);
  jpeg_create_decompress(&this->jinfo);

  jpeg_stdio_src(&this->jinfo, this->fin);

  jpeg_read_header(&this->jinfo, TRUE);

  this->imageWidth = this->jinfo.image_width;
  this->imageHeight = this->jinfo.image_height;

  return TRUE;
}

int tsmReadRowsDOQ_read(void *buffer, int startRow, int numberOfRows, int numberOfBytes, void *closure) {
  static const char *funcName = "tsmReadRowsDOQ_read";
  TsmReadRowsDOQ *this = (TsmReadRowsDOQ *) closure;
  int scanlines;
  int n;
  unsigned char *p = (unsigned char *) buffer;

  CHECK_NOT_NULL(this, FALSE);
  CHECK_NOT_NULL(buffer, FALSE);

  for (scanlines = 0; scanlines<numberOfRows;
       scanlines += (n=jpeg_read_scanlines(&this->jinfo, (JSAMPARRAY) &p, 1)),
         p += this->imageWidth * this->compsPerPixel * n);

  return TRUE;
}

static char *getBaseName(char *buffer, const char *filename) {
  int i;
  int dot;
  int len = strlen(filename);

  dot = len;
  for (i=len-1; i>=0; i--) {
    if (filename[i]=='.') {
      dot = i;
      break;
    }
  }

  strncpy(buffer, filename, dot);

  return buffer;
}

static FILE *tsmReadRowsDOQ_getInfoFile(char *filename) {
  static const char *funcName = "tsmReadRowsDOQ_getInfoFile";
  char buffer[256];

  if (getBaseName(buffer, filename) == NULL) {
    tsmError(funcName, "Failed to open info file for DOQ image.");
    return NULL;
  }
  strcpy(buffer + strlen(buffer), ".hd");

  return fopen(buffer, "rb");
}

static int tsmReadRowsDOQ_parseInfoFile(char *filename, TsmTileParams *tile, TsmTspecParams *tspec) {
  static const char *funcName = "tsmReadRowsDOQ_parseInfoFile";
  FILE *fparams;
  char buffer[512];
  int count;
  int temp;

  if ((fparams = tsmReadRowsDOQ_getInfoFile(filename)) == NULL) {
    return FALSE;
  }

  count = 0;
  while (!feof(fparams)) {
    while (!feof(fparams) && ((temp=getc(fparams))=='\r' || temp=='\n'));
    ungetc(temp, fparams);
    if (fgets(buffer, 512, fparams)==NULL) {
      break;
    }

    if (strncmp(buffer, DOQ_TRAILER_SENTINEL, sizeof(DOQ_HEADER_SENTINEL)-1)==0) break;
    else if (strncmp(buffer, DOQ_QUADRANGLE_NAME, sizeof(DOQ_QUADRANGLE_NAME)-1)==0) {
      int start, n, len;
      int i;
      len = strlen(buffer);
      start = -1;
      for (i=sizeof(DOQ_QUADRANGLE_NAME)-1; i<len && start==-1; i++)
        if (buffer[i] == '"') start = i+1;
      if (start==-1) {
        tsmError(funcName, "Failed to parse info file.");
        fclose(fparams);
        return FALSE;
      }
      n = -1;
      for (i=start; i<len && n==-1; i++) if (buffer[i] == '"') n = i-start;
      if (n==-1) {
        tsmError(funcName, "Failed to parse info file.");
        fclose(fparams);
        return FALSE;
      }
      if ((tspec->geoName = tsmMemMalloc(n+1)) == NULL) {
        tsmError(funcName, "Failed to parse info file.");
        fclose(fparams);
        return FALSE;
      }
      strncpy(tspec->geoName, buffer+start, n);
      tspec->geoName[n] = 0;

      count++;
    }
    else if (strncmp(buffer, DOQ_COORDINATE_ZONE, sizeof(DOQ_COORDINATE_ZONE)-1) == 0) {
      sscanf(buffer+sizeof(DOQ_COORDINATE_ZONE), "%d", &tspec->zone);
      count++;
    }
    else if (strncmp(buffer, DOQ_SW_QUAD_CORNER_XY, sizeof(DOQ_SW_QUAD_CORNER_XY)-1) == 0) {
      sscanf(buffer+sizeof(DOQ_SW_QUAD_CORNER_XY), "%lf %lf", &tspec->ll_e, &tspec->ll_n);
      count++;
    }

    if (count==3) break;
  }

  tspec->coord_sys = TSM_UTM_COORDSYS;

  return TRUE;
}

int tsmParseCreateParams_DOQ(char *image_name, TsmTileParams *tile, TsmTspecParams *tspec) {
  static const char *funcName = "tsmParseCreateParams_DOQ";
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

  if (!tsmReadRowsDOQ_parseInfoFile(image_name, tile, tspec)) {
    tsmError(funcName, "Error encountered when trying to read info file for DOQ image.");
    return FALSE;
  }

  return TRUE;
}
