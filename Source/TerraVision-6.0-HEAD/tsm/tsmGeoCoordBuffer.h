/*
 * Name: tsmGeoCoordBuffer.h
 *
 * Description: 
 *
 * Function List:
 *
 * Revision History:
 *   Created Jul 2000 by Yong Tze Chi <chi@ai.sri.com>
 *
 *   $Id: tsmGeoCoordBuffer.h,v 1.2 2000/08/28 22:15:48 reddy Exp $
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

#ifndef _TSMGEOCOORDBUFFER_H_
#define _TSMGEOCOORDBUFFER_H_

typedef struct {
  double *point;
  long   capacity;
  long   size;
} GeoCoordBuffer;

#define GeoCoordBuffer_initialSize 20
#define GeoCoordBuffer_sizeIncreaseFactor 2
#define GeoCoordBuffer_maxStringSize 60000

GeoCoordBuffer *GeoCoordBuffer_new();
void GeoCoordBuffer_destroy(GeoCoordBuffer **this);
int GeoCoordBuffer_grow(GeoCoordBuffer *this);
int GeoCoordBuffer_insert(GeoCoordBuffer *this, double u, double v, double elevation);
MFString *GeoCoordBuffer_getMFString(GeoCoordBuffer *this, vsgWorld *wrl);


#endif
