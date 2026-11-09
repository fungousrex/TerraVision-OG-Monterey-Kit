/*
 * Name: tsmConnectionParamsInt.h
 *
 * Description: 
 *   The internal definitions for the support of the tsmApi's
 *   Connection Parameters facilitity.
 *
 * Function List:
 *
 * Revision History:
 *
 *   $Id: tsmConnectionParamsInt.h,v 1.24.4.3 2001/07/09 19:04:06 leei Exp $
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

#ifndef TSMCONNECTIONPARAMSINT_H
#define TSMCONNECTIONPARAMSINT_H

#include "tsmTile.h"

#define tsmIsParamSet(p,x) (((p)!=NULL) && ((p)->flag & (x)) != 0)
#define tsmSetParamFlag(p,x)   ((void)(((p)!=NULL) && ((p)->flag |= (x))))

struct TsmConnectionParamsStruct {
  TsmParamCode  flag;           /* bit field to say which fields are set */
  int           set_id;         /* set ID of dataset on a DPSS */
  int           numServers;     /* number of servers in server array */
  char          **server;       /* array of server names for dataset on DPSS */
  int           format;         /* TSM_LRBT_FORMAT, TSM_JPEG_FORMAT, ... */
  TsmTileFormat tile_format;    /* for localdisk datasets only */
  int           fallback;       /* keep trying if desired dataset not found? */
  int           validate_tspec; /* try to validate tspec when loaded? */
  int           dpss_tspec;     /* read the tspec from the DPSS by default */
  int           new_tspec;      /* on localdisk, read single-file tspec */
  char          *vrml_dir;      /* directory to write VRML sets to */
  char          **texUrns;      /* list of URLS to get terrain textures from */
  int           nTexUrns;       /* total number of texUrns */
  double        *lcs;           /* list of local coordinate system scales */
  int           num_lcs;        /* number of local coordinate systems */
  int           num_polys;      /* number of polys per side on a VRML tile */
  int           num_lods;       /* no. of LODs to include in one VRML tree */
  int           tree_skip;      /* num_lods-1, or just 1 = lod:anchor ratio */
  int           gaussian;       /* use Gaussian sampling of DEMs? */
  int           collision;      /* include VRML Collision detection node */
  int           test_vtiles;    /* TRUE creates flat, rand-colored vtiles */
  int           start_level;    /* The TV start level for the OI tspec (+) */
  int           end_level;      /* The TV end level for the OI tspec (-) */
  double        texel_scale;    /* value to scale the LOD range calculation */
  int           flat;		/* should world be created flat or in globe? */
  double	elev_scale;	/* elevation scaling */
  double	world_scale;	/* world scaling */
  int           tree_type;      /* the tree file type (anchor, quadlod,etc.) */
  int           geotiles;       /* TRUE if we want to make geotiles */
  int		normals;	/* TRUE for per vertex normal generation */
  int     	solid;		/* one- or two-sided polygons for terrain */
  int           fast_bbox;      /* TRUE when we want to do fast bbox calc */
  char          *vrml_url;      /* root URL used in all VRML files */
  int           blocking_read;  /* do blocking reads for tsmReadTile? */
  int		request_check;  /* check for multiple requests */
  int           root_level;     /* the root level of the tree hierarchy */
  int           include_vtiles; /* include vtiles in tree */
  int           full_dpss_read; /* do a full or quick DPSS server update */
  int           cache_conns;    /* should we cache connections? */

  int           vrml_models;    /* TRUE if we should generate VRML files */
  int           vrml_textures;  /* TRUE if we should generate texture image
				   files */
  int           read_multi;     /* The number of read connections to open */
  int           write_multi;    /* The number of write connections to open */

  tsmWMS        *wms;           /* Web Mapping Server structure */
};

#endif
