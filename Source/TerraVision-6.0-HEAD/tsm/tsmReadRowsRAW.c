/*
 * Name: tsmReadRowsRAW.c
 *
 * Description: 
 *   Import functions for supporting raw raster imagery/elevation data.
 *
 * Function List:
 *
 * Revision History:
 *   Created Jul 2000 by Yong Tze Chi <chi@ai.sri.com>
 *
 *   $Id: tsmReadRowsRAW.c,v 1.4.4.1 2002/03/18 01:13:06 reddy Exp $
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

#include "tsmReadRowsRAW.h"
#include "tsmInternal.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>

TsmReadRowsRAW *
tsmReadRowsRAW_new(char const *filename, int width, int height,
		   int compsPerPixel, int bytesPerComp)
{
  static const char *funcName = "tsmReadRowsRAW_new";
  TsmReadRowsRAW    *this;
  struct stat       stat_buffer;

  if ((this = tsmMemMalloc(sizeof(TsmReadRowsRAW))) == NULL) {
    tsmError(funcName, "Couldn't allocate TsmReadRowsRAW instance.");
    return NULL;
  }

  if ((this->fin = fopen(filename, "rb")) == NULL) {
    tsmError(funcName, "Failed to open file named %s.", filename);
    tsmReadRowsRAW_destroy(&this);
    return NULL;
  }

  this->imageWidth = width;
  this->imageHeight = height;
  this->compsPerPixel = compsPerPixel;
  this->bytesPerComp = bytesPerComp;
  this->bytesPerPixel = compsPerPixel * bytesPerComp;

  /* check that the file has the appropriate number of bytes */

  if ( stat( filename, &stat_buffer ) == 0 ) {
    double pixels = ((float) stat_buffer.st_size) /
      ((float) this->bytesPerPixel);
    if ( pixels - floor( pixels ) > 0.001 )
      tsmError( funcName, "File is not a multiple of %d bytes",
		this->bytesPerPixel );
  }

  return this;
}

void
tsmReadRowsRAW_destroy(TsmReadRowsRAW **this)
{
  if (this != NULL && *this!=NULL) {
    if ((*this)->fin != NULL) fclose((*this)->fin);
    tsmMemFree(*this);
    *this = NULL;
  }
}

int
tsmReadRowsRAW_read(void *buffer, int startRow, int numberOfRows,
		    int numberOfBytes, void *closure)
{
  static const char *funcName = "tsmReadRowsRAW_read";
  TsmReadRowsRAW *this = (TsmReadRowsRAW *) closure;
  long offset;

  offset = startRow * this->imageWidth * this->bytesPerPixel;

  if (ftell(this->fin) != offset) fseek(this->fin, offset, SEEK_SET);

  if (fread(buffer, sizeof(char) * this->bytesPerPixel, numberOfRows * this->imageWidth, this->fin)<=0) {
    tsmError(funcName, "Error occured while trying to read from RAW file.");
    return FALSE;
  }

  return TRUE;
}
