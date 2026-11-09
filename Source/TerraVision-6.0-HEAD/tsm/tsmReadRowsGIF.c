/*
 * Name: tsmReadRowsGIF.c
 *
 * Description: 
 *   Import functions for supporting the GIF image format
 *
 * Function List:
 *
 * Revision History:
 *   Created Jul 2000 by Yong Tze Chi <chi@ai.sri.com>
 *
 *   $Id: tsmReadRowsGIF.c,v 1.5 2000/12/13 16:43:11 leei Exp $
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

#include <libungif/lib/gif_lib.h>
#include "tsmReadRowsGIF.h"
#include "tsmInternal.h"


TsmReadRowsGIF *tsmReadRowsGIF_new(const char *filename) {
  static const char *funcName = "tsmReadRowsGIF_new";
  TsmReadRowsGIF *this;

  if ((this = tsmMemMalloc(sizeof(TsmReadRowsGIF))) == NULL) {
    tsmError(funcName, "Couldn't allocate memory for TsmReadRowsGIF instance.");
    return NULL;
  }

  if ((this->gifFile = DGifOpenFileName(filename)) == NULL) {
    tsmError(funcName, "Couldn't open GIF file named %s", filename);
    tsmReadRowsGIF_destroy(&this);
    return NULL;
  }

  if (!tsmReadRowsGIF_readHeader(this)) {
    tsmReadRowsGIF_destroy(&this);
    return NULL;
  }

  this->colorMap = this->gifFile->SavedImages[0].ImageDesc.ColorMap;
  if (this->colorMap == NULL) this->colorMap = this->gifFile->SColorMap;
  if (this->colorMap == NULL) {
    tsmError(funcName, "Color map is missing in the GIF file.");
    tsmReadRowsGIF_destroy(&this);
    return NULL;
  }

  this->compsPerPixel = 3;
  this->bytesPerComp = 1;

  return this;
}

void tsmReadRowsGIF_destroy(TsmReadRowsGIF **this) {
  if (this != NULL && *this != NULL) {
    if ((*this)->gifFile != NULL) {
      DGifCloseFile((*this)->gifFile);
    }
    tsmMemFree(*this);
    *this = NULL;
  }
}

static int tsmReadRowsGIF_readHeader(TsmReadRowsGIF *this) {
  static const char *funcName = "tsmReadRowsGIF_readHeader";
  GifRecordType recordType;
  GifByteType *extension;
  SavedImage  saved;

  CHECK_NOT_NULL(this, FALSE);
  CHECK_NOT_NULL(this->gifFile, FALSE);

  do {
    if (DGifGetRecordType(this->gifFile, &recordType) == GIF_ERROR) {
      tsmError(funcName, "Couldn't determine GIF record type.");
      return FALSE;
    }

    switch (recordType) {
    case IMAGE_DESC_RECORD_TYPE:
      if (DGifGetImageDesc(this->gifFile) == GIF_ERROR) {
        tsmError(funcName, "Couldn't get Image Descriptor block.");
        return FALSE;
      }
      this->imageWidth = this->gifFile->Image.Width;
      this->imageHeight = this->gifFile->Image.Height;
      return TRUE;
    case EXTENSION_RECORD_TYPE:
      if (DGifGetExtension(this->gifFile, &saved.Function, &extension)==GIF_ERROR) {
        tsmError(funcName, "Couldn't read extension block.");
        return FALSE;
      }
      break;
    case TERMINATE_RECORD_TYPE:
      break;
    }
  } while (recordType != TERMINATE_RECORD_TYPE);

  return FALSE;
}

int tsmReadRowsGIF_read(void *buffer, int startRow, int numberOfRows, int numberOfBytes, void *closure) {
  static const char *funcName = "tsmReadRowsGIF_read";
  TsmReadRowsGIF *this = (TsmReadRowsGIF *) closure;
  unsigned char *indexBuffer;
  int size;
  int i;
  long offset;

  CHECK_NOT_NULL(this, FALSE);
  CHECK_NOT_NULL(buffer, FALSE);
  CHECK_NOT_NULL(this->gifFile, FALSE);

  size = numberOfRows * this->gifFile->Image.Width * sizeof(unsigned char);

  if ((indexBuffer = tsmMemMalloc(size)) == NULL) {
    tsmError(funcName, "Couldn't allocate memory to read data from GIF file.");
    return FALSE;
  }

  if (DGifGetLine(this->gifFile, indexBuffer, size) == GIF_ERROR) {
    tsmError(funcName, "Error encountered while reading GIF file.");
    return FALSE;
  }

  for (i=0, offset=0; i<size; i++) {
    if (indexBuffer[i] > this->colorMap->ColorCount) {
      tsmError(funcName, "Invalid color index encountered while reading GIF file.");
      return FALSE;
    }
    ((unsigned char *)buffer)[offset++] = this->colorMap->Colors[indexBuffer[i]].Red;
    ((unsigned char *)buffer)[offset++] = this->colorMap->Colors[indexBuffer[i]].Green;
    ((unsigned char *)buffer)[offset++] = this->colorMap->Colors[indexBuffer[i]].Blue;
  }

  tsmMemFree(indexBuffer);

  return TRUE;
}

int tsmParseCreateParams_GIF(char *image_name, TsmTileParams *tile, TsmTspecParams *tspec ) {
  static const char *funcName = "tsmParseCreateParams_GIF";
  GifFileType *gifFile;
  GifRecordType recordType;
  GifByteType *extension;
  SavedImage  saved;

  if ((gifFile = DGifOpenFileName(image_name)) == NULL) {
    tsmError(funcName, "Couldn't open GIF file named %s", image_name);
    return FALSE;
  }

  do {
    if (DGifGetRecordType(gifFile, &recordType) == GIF_ERROR) {
      tsmError(funcName, "Couldn't determine GIF record type.");
      return FALSE;
    }

    switch (recordType) {
    case IMAGE_DESC_RECORD_TYPE:
      if (DGifGetImageDesc(gifFile) == GIF_ERROR) {
        tsmError(funcName, "Couldn't get Image Descriptor block.");
        return FALSE;
      }
      tile->width = tspec->width = gifFile->Image.Width;
      tile->height = tspec->height = gifFile->Image.Height;
      tspec->comps  = tile->comps = 3;
      tile->bytespercomp = 1;
      DGifCloseFile(gifFile);
      return TRUE;
    case EXTENSION_RECORD_TYPE:
      if (DGifGetExtension(gifFile, &saved.Function, &extension)==GIF_ERROR) {
        tsmError(funcName, "Couldn't read extension block.");
        return FALSE;
      }
      break;
    case TERMINATE_RECORD_TYPE:
      break;
    }
  } while (recordType != TERMINATE_RECORD_TYPE);

  DGifCloseFile(gifFile);
  return FALSE;
}
