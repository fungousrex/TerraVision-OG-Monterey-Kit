/*
 * Name: tsmWMS.h
 *
 * Description: 
 *   Definitions for interfacing with OpenGIS Consortium (OGC)
 *   compliant Web Map Servers (WMS).
 *
 * Revision History:
 *   Created 13 Jun 2001 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmWMS.h,v 1.1.2.5 2001/07/24 00:48:05 reddy Exp $
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
 *   Portions are Copyright (c) SRI International, 1998-2001.
 *
 */

#ifndef TSMWMS_H
#define TSMWMS_H

#define TSM_WMS_FORMAT_NONE  0
#define TSM_WMS_FORMAT_LRBT  1
#define TSM_WMS_FORMAT_JPEG  2

typedef struct {
  char   *name;
  char   *title;
  char   *abstract;
  int    srs;
  double uv_bbox[4];
  double ll_bbox[4];
  double resx, resy;
  double scaleHint[2];
} tsmLayer;

struct tsmWMS_ {
  char     *url;
  char     *getmap_url;
  char     *version;
  char     *title;
  char     **format;
  int      numFormats;
  int      mapFormat;    /* lookup index into the format[] array */
  int      mapFormatID;  /* one of TSM_WMS_FORMAT_*, as defined above */   
  tsmLayer *layer;
  int      numLayers;
  char     *error;
};

tsmWMS *tsmWMSCreate( char *url );
void tsmWMSFree( tsmWMS **wms_ptr );

int tsmWMSAddLayer( tsmWMS *wms, tsmLayer *layer );
tsmLayer *tsmWMSFindLayer( tsmWMS *wms, char *name );

int tsmWMSGetCapabilities( tsmWMS *wms );

double tsmWMSVersion( char *version );

int tsmWMSParseSRS( char *srs );

char *tsmWMSGetException( char *buffer );

char *tsmWMSExtractUrl( char *url );
char *tsmWMSExtractParam( char *url, char *param );
char *tsmWMSDatasetUrl( char *url, char *layer_name );


#endif
