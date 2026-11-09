/*
 * Name: tsmGeoCoordBuffer.c
 *
 * Description: 
 *   Routines to create a list of coordinates for the GeoVRML export
 *   capabilities of tsmApi. This list of coordinates has the ability
 *   to add and retrieve entries as well as to grow more array entries.
 *
 * Function List:
 *
 * Revision History:
 *   Created Jul 2000 by Yong Tze Chi <chi@ai.sri.com>
 *
 *   $Id: tsmGeoCoordBuffer.c,v 1.2.6.1 2002/03/14 05:22:56 reddy Exp $
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

#include <tsm/tsm.h>
#include <string.h>
#include "tsmTrace.h"
#include "tsmGeoVRML.h"
#include "tsmGeoCoordBuffer.h"
#include "tsmStringBuffer.h"


GeoCoordBuffer *GeoCoordBuffer_new() {
  static const char *funcName = "GeoCoordBuffer_new";
  GeoCoordBuffer *this;

  if ((this = tsmMemMalloc(sizeof(GeoCoordBuffer))) == NULL) {
    tsmError(funcName, "Couldn't allocate memory for GeoCoordBuffer structure");
    return NULL;
  }

  this->point = NULL;
  this->capacity = 0;
  this->size = 0;

  if (GeoCoordBuffer_grow(this)==FALSE) {
    tsmMemFree(this);
    return NULL;
  }

  return this;
}

void GeoCoordBuffer_destroy(GeoCoordBuffer **this) {
  /* static const char *funcName = "GeoCoordBuffer_destroy";*/

  if (this != NULL && *this != NULL) {
    if ((*this)->point != NULL) tsmMemFree((*this)->point);
    tsmMemFree(*this);
    *this = NULL;
  }
}

int GeoCoordBuffer_grow(GeoCoordBuffer *this) {
  static const char *funcName = "GeoCoordBuffer_grow";
  double *newBuffer;
  int i;

  CHECK_NOT_NULL(this, FALSE);

  this->capacity = (this->capacity <= 0) ? GeoCoordBuffer_initialSize : this->capacity * GeoCoordBuffer_sizeIncreaseFactor;

  if ((newBuffer = tsmMemMalloc(sizeof(double) * this->capacity)) == NULL) {
    tsmError(funcName, "Couldn't allocate memory for point array");
    return FALSE;
  }

  if (this->point != NULL) {
    for (i=0; i<this->size; i++) {
      newBuffer[i] = this->point[i];
    }
    tsmMemFree(this->point);
  }
  this->point = newBuffer;

  return TRUE;
}

int GeoCoordBuffer_insert(GeoCoordBuffer *this, double u, double v, double elevation) {
  static const char *funcName = "GeoCoordBuffer_insert";

  CHECK_NOT_NULL(this, FALSE);

  if (this->size+3 > this->capacity) GeoCoordBuffer_grow(this);

  this->point[this->size++] = v;
  this->point[this->size++] = u;
  this->point[this->size++] = elevation;

  return TRUE;
}

MFString *GeoCoordBuffer_getMFString(GeoCoordBuffer *this, vsgWorld *wrl) {
  static const char *funcName = "GeoCoordBuffer_getMFString";
  MFString *mfstring;
  char buffer[512];
  long i;
  StringBuffer *stringBuffer;

  CHECK_NOT_NULL(this, NULL);

  if ((stringBuffer = StringBuffer_new()) == NULL) return NULL;

  mfstring = vsgStartMFString(wrl);

  for (i=0; i<this->size; i++) {
    if (i<this->size-1) {
      if (i%6==5) sprintf(buffer, "%f,\n", this->point[i]);
      else if (i%3==2) sprintf(buffer, "%f, ", this->point[i]);
      else sprintf(buffer, "%f ", this->point[i]);
    }
    else sprintf(buffer, "%f", this->point[i]);
    if (strlen(buffer) + stringBuffer->size > GeoCoordBuffer_maxStringSize) {
      vsgAppendSFString(wrl, mfstring, vsgStrdup(wrl, stringBuffer->buffer));
      StringBuffer_destroy(&stringBuffer);
      if ((stringBuffer = StringBuffer_new()) == NULL) {
        return NULL;
      }
    }
    StringBuffer_append(stringBuffer, buffer);
  }

  vsgAppendSFString(wrl, mfstring, vsgStrdup(wrl, stringBuffer->buffer));
  StringBuffer_destroy(&stringBuffer);

  vsgEndMFString(wrl, mfstring);
  return mfstring;
}

