/*
 * Name: tsmImage.h
 *
 * Description: 
 *   Definitions to support the image writing routines of tsmImage.c
 *
 * Function List:
 *
 * Revision History:
 *
 *   $Id: tsmImage.h,v 1.5.6.1 2001/12/17 19:13:45 reddy Exp $
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

#ifndef TSMIMAGE_H
#define TSMIMAGE_H

/* The following enum defines all of the image formats that   */
/* are supported by the tsmSaveImage() function, which writes */
/* out a single TerraVision tile to an image file on disk.    */

typedef enum {
  TSM_PPM,
  TSM_GIF,
  TSM_JFIF
} TsmImageType;

/* This structure is used by the tsmApi's image utilities */
/* in order to represent the data for a bitmapped image.  */

typedef struct {
  unsigned char *bitmap;          /* bitmap data for the image */
  int           width, height;    /* pixel dimensions of the image */
  int           components;       /* 1 = greyscale, 3 = RGB, 4 = RGBA */
} Image_t;


Image_t *tsmReadImage( char const *url );

int tsmWriteImage( Image_t *image, char const *filename, TsmImageType format );

void tsmFreeImage( Image_t *image, int free_bitmap );


#endif
