/*
 * Name: tsmReadRowsDEM.h
 *
 * Description: 
 *
 * Function List:
 *
 * Revision History:
 *   Created Jul 2000 by Yong Tze Chi <chi@ai.sri.com>
 *
 *   $Id: tsmReadRowsDEM.h,v 1.5 2000/09/23 01:03:28 leei Exp $
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

#ifndef _TSMREADROWSDEM_H_
#define _TSMREADROWSDEM_H_

#include <stdio.h>

#ifndef MIN
#define MIN( x, y ) ( ((x) < (y)) ? (x) : (y) )
#endif
#ifndef MAX
#define MAX( x, y ) ( ((x) > (y)) ? (x) : (y) )
#endif

#define SW          0
#define NW          1
#define NE          2
#define SE          3

#define NODATA      (short) -9999

typedef struct {
  
  /* the following fields come directly from the DEM file */
  
  char    mapLabel[145];         /* DEM quadrangle name */
  int     DEMlevel;              /* 1 = DEM-1, 2 = DEM-2, 3 = DEM-3 */
  int     elevationPattern;      /* 1 = regular, 2 = future use */
  int     groundSystem;          /* 0 = geographic, 1 = UTM, 2 = state plane */
  int     groundZone;            /* UTM zone or stat plane code */
  double  projectParams[15];     /* parameters for various projections */
  int     planeUnitOfMeasure;    /* 0 = radians, 1 = ft, 2 = m, 3 = arcsec */
  int     elevUnitOfMeasure;     /* 1 = feet, 2 = meters */
  int     polygonSizes;          /* sides of DEM polygon */
  double  groundCoords[4][2];    /* coords of all 4 corners, SW first */
  double  elevBounds[2];         /* min/max elevations for the DEM */
  double  localRotation;         /* counterclockwise angle in radians */
  int     accuracyCode;          /* 0 = unknown, 1 = info in record C */
  double  spatialResolution[3];  /* normally 30,30,1 for 7.5-min DEMs */
  int     profileDimension[2];   /* rows and columns of DEM profiles */
  
  /* the next fields are computed from the previous values */
  
  double  eastMost, westMost;    /* furthest east/west easting value */
  double  southMost, northMost;  /* furthest north/south northing value */
  int     eastMostSample;        /* furthest east sample point */
  int     westMostSample;        /* furthest west sample point */
  int     southMostSample;       /* furthest south sample point */
  int     northMostSample;       /* furthest north sample point */
  int     rows, cols;            /* regular elevation grid dimensions */
  char    shortName[41];         /* short version of map name */
  int     maxElevation;          /* largest integer read */
  int     minElevation;          /* smallest integer read */
  
  /* Finally, we have a number of options for the program */
  
  float   elevscale;             /* vertical exaggeration value */
  float   creaseAngle;           /* GeoElevationGrid crease angle */
  int     colour;                /* output a colour map for the geometry? */
  int     show_info;             /* show info about DEM only */
  int     save_pgm;              /* save as PGM image instead of VRML? */
  int     save_ppm;              /* save as PPM image instead of VRML? */
  int     save_pbm;              /* save as PGM silhouette instead of VRML? */
  int     double_sided;          /* created double-sided polygons? */
  int     stitch_edges;          /* do stitching of edges were not defined? */
  char    *tex_url;              /* the url for a texture for the GeoEG */
  char    *geo_url;              /* URL for GeoElevationGrid EXTERNPROTO */
  char    *format;               /* one of "none", "PPM", "PGM", "PGM (bw)" */
  int     xpolys, ypolys;        /* number of polys on a side */
  int     merge_dem;             /* if TRUE, take geoOrigin for other DEM */
  int     merge_zone;            /* zone number from DEM to merge with */
  int     merge_n, merge_e;      /* northing/easting from DEM to merge with */
  int     merge_elev;            /* elevation from DEM to merge with */
  int     readDataForInfo;       /* read the elevation data before printing info */
  
} DEMInfo;



typedef struct {
  int     imageWidth, imageHeight;
  int     compsPerPixel, bytesPerComp;
  FILE    *fin;
  short   *buffer;
  DEMInfo *demInfo;
} TsmReadRowsDEM;


TsmReadRowsDEM *tsmReadRowsDEM_new(char const *filename);
void tsmReadRowsDEM_destroy(TsmReadRowsDEM **this);
int tsmReadRowsDEM_read(void *buffer, int startRow, int numberOfRows, int numberOfBytes, void *closure);

int tsmParseCreateParams_DEM(char *filename, TsmTileParams *tile, TsmTspecParams *tspec);


#endif
