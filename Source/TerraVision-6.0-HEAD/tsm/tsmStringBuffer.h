/*
 * Name: tsmStringBuffer.h
 *
 * Description: 
 *
 * Function List:
 *
 * Revision History:
 *   Created Jul 2000 by Yong Tze Chi <chi@ai.sri.com>
 *
 *   $Id: tsmStringBuffer.h,v 1.2 2000/08/28 22:16:11 reddy Exp $
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

#ifndef _TSMSTRINGBUFFER_H_
#define _TSMSTRINGBUFFER_H_

typedef struct {
  char *buffer;
  int  capacity;
  int size;
} StringBuffer;

#define StringBuffer_initialSize 50
#define StringBuffer_sizeIncreaseFactor 2

StringBuffer *StringBuffer_new();
void StringBuffer_destroy(StringBuffer **this);
int StringBuffer_grow(StringBuffer *this);
int StringBuffer_append(StringBuffer *this, const char *str);


#endif
