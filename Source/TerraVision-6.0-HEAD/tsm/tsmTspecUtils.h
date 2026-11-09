/*
 * Name: tsmTspecUtils.h (from LBL)
 *
 * Description: 
 *
 * Function List:
 *
 * Revision History:
 *   Created Jul 2000 by Yong Tze Chi <chi@ai.sri.com>
 *
 *   $Id: tsmTspecUtils.h,v 1.5 2000/08/28 22:16:21 reddy Exp $
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

#ifndef TSPEC_UTILS_H
#define TSPEC_UTILS_H

#ifndef TSPEC_BLOCK_KEY      /* defined in latest DPSS inc tspec/tload.h */
#define TSPEC_BLOCK_KEY -999 /* special block_id code for tspec block */
#endif

/*
 * Functions to print contents of data structures:
 */
void fprint_Purl(FILE *, Purl *);
void fprint_IdentityTspec(FILE *, IdentityTspec *);
void fprint_TileTspec(FILE *, TileTspec *);
void fprint_CoordTspec(FILE *, CoordTspec *);
void fprint_PixelTspec(FILE *, PixelTspec *);
void fprint_ImageTransformationTspec(FILE *, ImageTransformationTspec *);
void fprint_ProjectionTspec(FILE *, ProjectionTspec *);
void fprint_SimpleTileSet(FILE *, SimpleTileSet *);
void fprint_Pyramid(FILE *, Pyramid *);
void fprint_NameUrlPairs(FILE *, NameUrlPairs *);

/*
 * Check for strings in tspec data structures.  All functions return 0 on
 * success and -1 on error.  Note that "success" does not imply anything about
 * whether NULL strings were found.  
 */
int check_Purl_for_strings(Purl *);
int check_IdentityTspec_for_strings(IdentityTspec *);
int check_TileTspec_for_strings(TileTspec *);
int check_CoordTspec_for_strings(CoordTspec *);
int check_PixelTspec_for_strings(PixelTspec *);
int check_ImageTransformationTspec_for_strings(ImageTransformationTspec *);
int check_ProjectionTspec_for_strings(ProjectionTspec *);
int check_SimpleTileSet_for_strings(SimpleTileSet *);
int check_Pyramid_for_strings(Pyramid *);
int check_NameUrlPairs_for_strings(NameUrlPairs *);


#endif /* TSPEC_UTILS_H */

/* bltierney@lbl.gov   http://george.lbl.gov/~tierney     tel: 510-486-7381 */ 
/* Brian L. Tierney,   Lawrence Berkeley National Laboratory (LBNL)         */
/* 1 Cyclotron Rd.  MS: 50B-2239,  Berkeley, CA  94720    fax: 510-486-6363 */ 


