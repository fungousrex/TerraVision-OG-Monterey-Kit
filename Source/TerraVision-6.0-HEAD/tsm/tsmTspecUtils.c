/*
 * Name: tsmTspecUtils.c
 *
 * Description: 
 *   Routines to check strings in tspec structs and to print tspec structs.
 *
 * Function List:
 *
 * Revision History:
 *
 *   "$Id: tsmTspecUtils.c,v 1.14 2000/08/28 22:16:20 reddy Exp $"
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

#include "tsmInternal.h"

/************************************************************************/
void 
fprint_Purl(FILE *fp, Purl *p_p)
{
    /*static char const funcName[] = "fprint_Purl()";*/

    fprintf(fp, "Purl:\n");
    fprintf(fp, "originalUrl: %s\n", p_p->originalUrl);
    fprintf(fp, "url: %s\n", p_p->url);
    fprintf(fp, "protocol: %d\n", p_p->protocol);
    fprintf(fp, "protocolString: %s\n", p_p->protocolString);
    fprintf(fp, "hostName: %s\n", p_p->hostName);
    fprintf(fp, "uri: %s\n", p_p->uri);
    return;
}
/* end fprint_Purl */


/************************************************************************/
void 
fprint_IdentityTspec(FILE *fp, IdentityTspec *it_p)
{
    /* static char const funcName[] = "fprint_IdentityTspec()";*/
    unsigned int i;

    fprintf(fp, "IdentityTspec:\n");
    fprintf(fp, "\tname: %s\n", it_p->name);
    fprintf(fp, "\tsetId: %d\n", it_p->setId);

    fprintf (fp, "checksum-lrbt");
    for ( i = 0; i < it_p->checkSumLrbt.len; i++ ) {
      fprintf( fp, " %d", it_p->checkSumLrbt.val[i] );
    }
    fprintf( fp, "\n" );

    fprintf (fp, "checksum-jpeg");
    for ( i = 0; i < it_p->checkSumJpeg.len; i++ ) {
      fprintf( fp, " %d", it_p->checkSumJpeg.val[i] );
    }
    fprintf( fp, "\n" );

    fprintf(fp, "\ttype: %d\n", it_p->type);
    fprintf(fp, "\tsubtype: %d\n", it_p->subtype);
    for (i = 0; i < it_p->sources.len; ++i) {
      fprintf(fp, "\tsources[%d]: %s\n", i, it_p->sources.val[i]);
    }
    for (i = 0; i < it_p->operations.len; ++i) {
      fprintf(fp, "\toperations[%d]: %d\n", i, it_p->operations.val[i]);
    }
    fprintf(fp, "\toperationDate: %s\n", it_p->operationDate);
    fprintf(fp, "\toperationDoneBy: %s\n", it_p->operationDoneBy);
    return;
}
/* end fprint_IdentityTspec */


/************************************************************************/
void 
fprint_TileTspec(FILE *fp, TileTspec *tt_p)
{
    /* static char const funcName[] = "fprint_TileTspec()";*/
    int i;

    fprintf(fp, "TileTspec:\n");
    fprintf(fp, "\tnumPixelsX: %d\n", tt_p->numPixelsX);
    fprintf(fp, "\tnumPixelsY: %d\n", tt_p->numPixelsY);
    for (i = 0; i < 2; ++i)
    {
	fprintf(fp, "\toverLap[%d]: %d\n", i, tt_p->overLap[i]);
    }
    for (i = 0; i < 2; ++i)
    {
	fprintf(fp, "\ttotalPixels[%d]: %d\n", i, tt_p->totalPixels[i]);
    }
    fprintf(fp, "\tnumComponentsPixel: %d\n", tt_p->numComponentsPixel);
    fprintf(fp, "\tnumBytesPerComponent: %d\n", tt_p->numBytesPerComponent);
    fprintf(fp, "\tcomponentNumberType: %d\n", tt_p->componentNumberType);
    fprintf(fp, "\tstorageFormat: %d\n", tt_p->storageFormat);
    return;
}
/* end fprint_TileTspec */


/************************************************************************/
void 
fprint_CoordTspec(FILE *fp, CoordTspec *ct_p)
{
    /* static char const funcName[] = "fprint_CoordTspec()";*/
    int i;

    fprintf(fp, "CoordTspec:\n");
    fprintf(fp, "\tsystemType: %d\n", ct_p->systemType);
    fprintf(fp, "\tspheroidName: %d\n", ct_p->spheroidName);
    fprintf(fp, "\tequatorialRadius: %f\n", ct_p->equatorialRadius);
    fprintf(fp, "\tpolarRadius: %f\n", ct_p->polarRadius);
    for (i = 0; i < 16; ++i) {
	fprintf(fp, "\tlvcstoGeo[%d]: %f\n", i, ct_p->lvcstoGeo[i]);
    }
    for (i = 0; i < 16; ++i) {
	fprintf(fp, "\tgeotoLVCS[%d]: %f\n", i, ct_p->geotoLVCS[i]);
    }
    fprintf(fp, "\tlcsType: %d\n", ct_p->lcsType);
    fprintf(fp, "\toriginLat: %f\n", ct_p->originLat);
    fprintf(fp, "\toriginLong: %f\n", ct_p->originLong);
    fprintf(fp, "\tlocalUnitsPerMeter: %f\n", ct_p->localUnitsPerMeter);
    for (i = 0; i < ct_p->componentNames.len; ++i) {
	fprintf(fp, "\tcomponentNames.componentNames[%d]: %d\n",
		i, ct_p->componentNames.val[i]);
    }
    fprintf(fp, "\tcomponentUnits: %d\n", ct_p->componentUnits);
    return;
}
/* end fprint_CoordTspec */


/************************************************************************/
void 
fprint_PixelTspec(FILE *fp, PixelTspec *pt_p)
{
    /*static char const funcName[]= "fprint_PixelTspec()";*/
    int i;

    fprintf(fp, "PixelTspec:\n");
    for (i = 0; i < pt_p->componentNames.len; ++i)
    {
	fprintf(fp, "componentNames[%d]: %d\n",
		i, pt_p->componentNames.val[i]);
    }
    fprintf(fp, "minVal: %f\n", pt_p->minVal);
    fprintf(fp, "maxVal: %f\n", pt_p->maxVal);
    fprintf(fp, "scalingType: %d\n", pt_p->scalingType);
    fprintf(fp, "scale: %f\n", pt_p->scale);
    fprintf(fp, "offset: %f\n", pt_p->offset);
    return;
}
/* end fprint_PixelTspec */


/************************************************************************/
void 
fprint_ImageTransformationTspec(FILE *fp, ImageTransformationTspec *itt_p)
{
    /*static char const funcName[]= "fprint_ImageTransformationTspec()";*/
    int i;

    fprintf(fp, "ImageTransformationTspec:\n");
    for (i = 0; i < 9; ++i)
    {
	fprintf(fp, "uvtoIXIY[%d]: %f\n", i, itt_p->uvtoIXIY[i]);
    }
    for (i = 0; i < 9; ++i)
    {
	fprintf(fp, "ixiytoUV[%d]: %f\n", i, itt_p->ixiytoUV[i]);
    }
    return;
}
/* end fprint_ImageTransformationTspec */


/************************************************************************/
void 
fprint_ProjectionTspec(FILE *fp, ProjectionTspec *pt_p)
{
    /*static char const funcName[]= "fprint_ProjectionTspec()";*/
    unsigned int i;

    fprintf(fp, "projectionType: %d\n", pt_p->projectionType);
    for (i = 0; i < 16; ++i)
    {
	fprintf(fp, "\tlcsToUvProjectiveMatrix[%d]: %f\n", i,
		pt_p->lcsToUvProjectiveMatrix[i]);
    }
    for (i = 0; i < 16; ++i)
    {
	fprintf(fp, "\tuvaToLcsMatrix[%d]: %f\n", i, pt_p->uvaToLcsMatrix[i]);
    }
    fprintf(fp, "inverseFocalLength: %f\n", pt_p->inverseFocalLength);
    for (i = 0; i < 2; ++i)
    {
	fprintf(fp, "\tpiercingPoint[%d]: %f\n", i, pt_p->piercingPoint[i]);
    }
    fprintf(fp, "numCoeffs: %d\n", pt_p->numCoeffs);
    for (i = 0; i < 6; ++i)
    {
	fprintf(fp, "\tcoeffs[%d]: %f\n", i, pt_p->coeffs[i]);
    }
    fprintf(fp, "utmZoneNumber: %d\n", pt_p->utmZoneNumber);
    fprintf(fp, "centralMeridian: %f\n", pt_p->centralMeridian);
    fprintf(fp, "k0: %f\n", pt_p->k0);
    fprintf(fp, "geoidSeparation: %f\n", pt_p->geoidSeparation);
    return;
}
/* end fprint_ProjectionTspec */


/************************************************************************/
void 
fprint_SimpleTileSet(FILE *fp, SimpleTileSet *sts_p)
{
    /*static char const funcName[]= "fprint_SimpleTileSet()";*/
    unsigned int i;

    fprintf(fp, "SimpleTileSet:\n");
    for (i = 0; i < 2; ++i)
    {
	fprintf(fp, "\tminTile[%d]: %d\n", i, sts_p->minTile[i]);
    }
    for (i = 0; i < 2; ++i)
    {
	fprintf(fp, "\tmaxTile[%d]: %d\n", i, sts_p->maxTile[i]);
    }
    for (i = 0; i < 2; ++i)
    {
	fprintf(fp, "\tnumTile[%d]: %d\n", i, sts_p->numTile[i]);
    }
    fprintf(fp, "\ttileMapStatus: %d\n", sts_p->tileMapStatus);
    for (i = 0; i < 2; ++i)
    {
	fprintf(fp, "\tnumPixels[%d]: %d\n", i, sts_p->numPixels[i]);
    }
    for (i = 0; i < 2; ++i)
    {
	fprintf(fp, "\tminValidPixel[%d]: %d\n", i, sts_p->minValidPixel[i]);
    }
    for (i = 0; i < 2; ++i)
    {
	fprintf(fp, "\tmaxValidPixel[%d]: %d\n", i, sts_p->maxValidPixel[i]);
    }
    for (i = 0; i < 2; ++i)
    {
	fprintf(fp, "\tnumValidPixel[%d]: %d\n", i, sts_p->numValidPixel[i]);
    }
    fprintf(fp, "imageFileName: %s\n", sts_p->imageFileName);
    fprintf(fp, "description: %s\n", sts_p->description);
    fprint_ImageTransformationTspec(fp, &(sts_p->imageTransformationTspec));
    fprint_IdentityTspec(fp, &(sts_p->identityTspec));
    fprint_TileTspec(fp, &(sts_p->tileTspec));
    fprint_PixelTspec(fp, &(sts_p->pixelTspec));
    fprint_ProjectionTspec(fp, &(sts_p->projectionTspec));
    fprint_CoordTspec(fp, &(sts_p->coordTspec));
    return;
}
/* end fprint_SimpleTileSet */


/************************************************************************/
void 
fprint_Pyramid(FILE *fp, Pyramid *p_p)
{
    /*static char const funcName[]= "fprint_Pyramid()";*/
    unsigned int i;

    fprintf(fp, "Pyramid:\n");
    fprintf(fp, "\ttype: %d\n", p_p->type);
    fprintf(fp, "\tpathName: %s\n", p_p->pathName);
    fprint_Purl(fp, p_p->tspecBasePurl);
    fprint_Purl(fp, p_p->tileBasePurl);
    fprintf(fp, "\tinvResScaleFactor: %f\n", p_p->invResScaleFactor);
    fprintf(fp, "\tminLevel: %d\n", p_p->minLevel);
    fprintf(fp, "\tminResolution: %f\n", p_p->minResolution);
    fprintf(fp, "\tmaxLevel: %d\n", p_p->maxLevel);
    fprintf(fp, "\tmaxResolution: %f\n", p_p->maxResolution);
    fprintf(fp, "\timageHierarchyMinValidLevel: %d\n",
	    p_p->imageHierarchyMinValidLevel);
    fprintf(fp, "\timageHierarchyMaxValidLevel: %d\n",
	    p_p->imageHierarchyMaxValidLevel);
    fprintf(fp, "\timageHierarchyFileName: %s\n", p_p->imageHierarchyFileName);
    fprintf(fp, "description: %s\n", p_p->description);
    for (i = p_p->minLevel; i <= p_p->maxLevel; ++i) {
      fprintf(fp, "\ttspecLevel[%d]: ", i);
      fprint_SimpleTileSet(fp, &(p_p->tspecLevel.val[i]));
    }
    fprint_IdentityTspec(fp, &(p_p->identityTspec));
    fprint_TileTspec(fp, &(p_p->tileTspec));
    fprint_PixelTspec(fp, &(p_p->pixelTspec));
    fprint_ProjectionTspec(fp, &(p_p->projectionTspec));
    fprint_CoordTspec(fp, &(p_p->coordTspec));
    fprintf(fp, "\ttileSize: %d\n", p_p->tileSize);
    return;
}
/* end fprint_Pyramid */

/************************************************************************/
void 
fprint_NameUrlPairs(FILE *fp, NameUrlPairs *pu_p)
{
    /*static char const funcName[]= "fprint_NameUrlPairs()";*/
    unsigned int i;

    fprintf(fp, "NameUrlPairs:\n");
    for (i = 0; i < pu_p->names.len; ++i)
    {
	fprintf(fp, "\tnames[%d]: %s\n", i, pu_p->names.val[i]);
    }
    for (i = 0; i < pu_p->urls.len; ++i)
    {
	fprintf(fp, "\turls[%d]: %s\n", i, pu_p->urls.val[i]);
    }
    return;
}
/* end fprint_NameUrlPairs */


/************************************************************************/
int 
check_Purl_for_strings(Purl *p_p)
{
    const char *funcName = "check_Purl_for_strings";

    if (tsmCheckString(&(p_p->originalUrl)) < 0) {
      tsmError (funcName, "error checking \"originalUrl\" field");
      return(-1);
    }
    if (tsmCheckString(&(p_p->url)) < 0) {
      tsmError (funcName, "error checking \"url\" field");
      return(-1);
    }
    if (tsmCheckString(&(p_p->protocolString)) < 0) {
      tsmError (funcName, "error checking \"protocolString\" field");
      return(-1);
    }
    if (tsmCheckString(&(p_p->hostName)) < 0) {
      tsmError (funcName, "error checking \"hostName\" field");
      return(-1);
    }
    if (tsmCheckString(&(p_p->uri)) < 0) {
      tsmError (funcName, "error checking \"uri\" field");
      return(-1);
    }

    if (tsmCheckString(&(p_p->port)) < 0) {
      tsmError (funcName, "error checking \"port\" field");
      return(-1);
    }
     
    return(0);
}
/* end check_Purl_for_strings */


/************************************************************************/
int 
check_IdentityTspec_for_strings(IdentityTspec *it_p)
{
  const char *funcName = "check_IdentityTspec_for_strings";
  int i;

    if (tsmCheckString(&(it_p->name)) < 0) {
      tsmError (funcName, "error checking \"name\" field");
      return(-1);
    }
    for (i = 0; i < it_p->sources.len; ++i)
    {
	if (tsmCheckString(&(it_p->sources.val[i])) < 0) {
	  tsmError (funcName, "error checking \"sources\" array, element %d", i);
	  return(-1);
	}
    }
    if (tsmCheckString(&(it_p->operationDate)) < 0) {
      tsmError (funcName, "error checking \"operationDate\" field");
      return(-1);
    }
    if (tsmCheckString(&(it_p->operationDoneBy)) < 0) {
      tsmError (funcName, "error checking \"operationDoneBy\" field");
      return(-1);
    }
    return(0);
}
/* end check_IdentityTspec_for_strings */


/************************************************************************/
int 
check_TileTspec_for_strings(TileTspec *tt_p)
{
    /*static char const funcName[]= "check_TileTspec_for_strings()";*/

    /* Nothing to check */
    return(0);
}
/* end check_TileTspec_for_strings */


/************************************************************************/
int 
check_CoordTspec_for_strings(CoordTspec *ct_p)
{
    /*static char const funcName[]= "check_CoordTspec_for_strings()";*/

    /* Nothing to check */
    return(0);
}
/* end check_CoordTspec_for_strings */


/************************************************************************/
int 
check_PixelTspec_for_strings(PixelTspec *pt_p)
{
    /*static char const funcName[]= "check_PixelTspec_for_strings()";*/

    /* Nothing to check */
    return(0);
}
/* end check_PixelTspec_for_strings */


/************************************************************************/
int 
check_ImageTransformationTspec_for_strings(ImageTransformationTspec *itt_p)
{
    /*static char const funcName[]= "check_ImageTransformTspec_for_strings()";*/

    /* Nothing to check */
    return(0);
}
/* end check_ImageTransformTspec_for_strings */


/************************************************************************/
int 
check_ProjectionTspec_for_strings(ProjectionTspec *pt_p)
{
    /*static char const funcName[]= "check_ProjectionTspec_for_strings()";*/

    /* Nothing to check */
    return(0);
}
/* end check_ProjectionTspec_for_strings */


/************************************************************************/
int 
check_SimpleTileSet_for_strings(SimpleTileSet *sts_p)
{
  const char *funcName = "check_SimpleTileSet_for_strings()";

    if (tsmCheckString(&(sts_p->imageFileName)) < 0) {
      tsmError (funcName, "error checking \"imageFileName\" field");
      return(-1);
    }
    if (tsmCheckString(&(sts_p->description)) < 0) {
      tsmError (funcName, "error checking \"description\" field");
      return(-1);
    }
    if (check_ImageTransformationTspec_for_strings(&(sts_p->
imageTransformationTspec)) < 0) {
      tsmError (funcName, "");
      return(-1);
    }
    if (check_IdentityTspec_for_strings(&(sts_p->identityTspec)) < 0) {
      tsmError (funcName, "");
      return(-1);
    }
    if (check_TileTspec_for_strings(&(sts_p->tileTspec)) < 0) {
      tsmError (funcName, "");
      return(-1);
    }
    if (check_PixelTspec_for_strings(&(sts_p->pixelTspec)) < 0) {
      tsmError (funcName, "");
      return(-1);
    }
    if (check_ProjectionTspec_for_strings(&(sts_p->projectionTspec)) < 0) {
      tsmError (funcName, "");
      return(-1);
    }
    if (check_CoordTspec_for_strings(&(sts_p->coordTspec)) < 0) {
      tsmError (funcName, "");
      return(-1);
    }
    return(0);
}
/* end check_SimpleTileSet_for_strings */


/************************************************************************/
int 
check_Pyramid_for_strings(Pyramid *p_p)
{
  const char *funcName = "check_Pyramid_for_strings";
    int i;

    if (tsmCheckString(&(p_p->pathName)) < 0) {
      tsmError (funcName, "error checking \"pathName\" field");
      return(-1);
    }
    if (tsmCheckString(&(p_p->imageHierarchyFileName)) < 0) {
      tsmError (funcName, "error checking \"imageHierarchyFileName\" field");
      return(-1);
    }
    if (tsmCheckString(&(p_p->description)) < 0) {
      tsmError (funcName, "error checking \"description\" field");
      return(-1);
    }
    for (i = 0; i < p_p->tspecLevel.len; ++i)
    {
	if (check_SimpleTileSet_for_strings(&(p_p->tspecLevel.val[i])) < 0) {
	  tsmError (funcName, "error checking \"tspecLevel\" array, element %d", i);
	  return(-1);
	}
    }
    if (check_IdentityTspec_for_strings(&(p_p->identityTspec)) < 0) {
      tsmError (funcName, "");
      return(-1);
    }
    if (check_TileTspec_for_strings(&(p_p->tileTspec)) < 0) {
      tsmError (funcName, "");
      return(-1);
    }
    if (check_PixelTspec_for_strings(&(p_p->pixelTspec)) < 0) {
      tsmError (funcName, "");
      return(-1);
    }
    if (check_ProjectionTspec_for_strings(&(p_p->projectionTspec)) < 0) {
      tsmError (funcName, "");
      return(-1);
    }
    if (check_CoordTspec_for_strings(&(p_p->coordTspec)) < 0) {
      tsmError (funcName, "");
      return(-1);
    }
    return(0);
}
/* end check_Pyramid_for_strings */

/************************************************************************/
int 
check_NameUrlPairs_for_strings(NameUrlPairs *pu_p)
{
  const char *funcName = "check_NameUrlPairs_for_strings";
  int i;

    for (i = 0; i < pu_p->names.len; ++i) {
      if (tsmCheckString(&(pu_p->names.val[i])) < 0) {
	tsmError (funcName, "error checking \"names\" array, element %d", i);
	return(-1);
      }
    }
    for (i = 0; i < pu_p->urls.len; ++i) {
      if (tsmCheckString(&(pu_p->urls.val[i])) < 0) {
	tsmError (funcName, "error checking \"urls\" array, element %d", i);
	return(-1);
      }
    }
    return(0);
}
/* end check_NameUrlPairs_for_strings */



/* bltierney@lbl.gov   http://george.lbl.gov/~tierney     tel: 510-486-7381 */ 
/* Brian L. Tierney,   Lawrence Berkeley National Laboratory (LBNL)         */
/* 1 Cyclotron Rd.  MS: 50B-2239,  Berkeley, CA  94720    fax: 510-486-6363 */ 
