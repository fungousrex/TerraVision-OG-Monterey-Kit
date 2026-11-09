/*
 * Name: tsmStringBuffer.c
 *
 * Description: 
 *   Functions for managing a string buffer where we can add strings and
 *   grow the array to support more strings.
 *
 * Function List:
 *
 * Revision History:
 *   Created Jul 2000 by Yong Tze Chi <chi@ai.sri.com>
 *
 *   $Id: tsmStringBuffer.c,v 1.2.6.1 2002/03/14 05:22:58 reddy Exp $
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
#include "tsmGeoVRML.h"
#include "tsmStringBuffer.h"
#include "tsmTrace.h"

StringBuffer *StringBuffer_new() {
  static const char *funcName = "StringBuffer_new";
  StringBuffer *this;

  if ((this = tsmMemMalloc(sizeof(StringBuffer))) == NULL) {
    tsmError(funcName, "Couldn't allocate memory for StringBuffer structure");
    return NULL;
  }

  this->buffer = NULL;
  this->capacity = 0;
  this->size = 0;

  if (StringBuffer_grow(this) == FALSE) {
    tsmMemFree(this);
    return NULL;
  }
  return this;
}

void StringBuffer_destroy(StringBuffer **this) {
  /* static const char *funcName = "StringBuffer_destroy";*/

  if (this!=NULL && *this!=NULL) {
    if ((*this)->buffer != NULL) {
      tsmMemFree((*this)->buffer);
    }
    tsmMemFree(*this);
    *this = NULL;
  }
}

int StringBuffer_grow(StringBuffer *this) {
  static const char *funcName = "StringBuffer_grow";
  char *newBuffer;

  CHECK_NOT_NULL(this, FALSE);

  this->capacity = (this->capacity==0) ? StringBuffer_initialSize : this->capacity * StringBuffer_sizeIncreaseFactor;

  if ((newBuffer = tsmMemMalloc(sizeof(char) * this->capacity)) == NULL) {
    tsmError(funcName, "Couldn't allocate memory for string buffer");
    return FALSE;
  }

  if (this->buffer != NULL) {
    memcpy(newBuffer, this->buffer, this->size);
    tsmMemFree(this->buffer);
  }

  this->buffer = newBuffer;

  return TRUE;
}

int StringBuffer_append(StringBuffer *this, const char *str) {
  static const char *funcName = "StringBuffer_append";
  int len;

  CHECK_NOT_NULL(this, FALSE);
  CHECK_NOT_NULL(str, FALSE);

  len = strlen(str);
  if (this->size + len > this->capacity && !StringBuffer_grow(this)) {
    return FALSE;
  }

  memcpy(this->buffer+this->size, str, len);
  this->size += len;
  this->buffer[this->size] = 0;

  return TRUE;
}
