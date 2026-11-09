/*
 * Filename: make_geovrml.c
 *
 * Description:
 *     This is a utility program to convert TerraVision datasets to GeoVRML
 *     models that are constructed of GeoLOD and GeoElevationGrid specifically.
 *
 *     This program was written for use with the tsmApi library from
 *     SRI International. For further information about this library,
 *     including downloads, documentation, and other examples, see:
 *
 *         http://www.tsmApi.com/
 *
 * Author:
 *     Yong Tze Chi, <chi@ai.sri.com> - 28 June 2000.
 *
 * Revision Information:
 *
 *     $Id: make_geovrml.c,v 1.3 2000/08/28 23:56:05 reddy Exp $
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
 */

#include <stdio.h>
#include <tsm/tsmGeoVRML.h>
#include <vrml2/vrml2.h>

int
main(int argc, char **argv) {
  TsmGeoVRML *app = TsmGeoVRML_new(argc, argv);

  if (app==NULL) {
    printf("Fail to initialize\n");
    return -1;
  }

  printf("Copyright (c) 2000 SRI International.  All rights reserved.\n\n");

  if (!TsmGeoVRML_writeAllTiles(app)) {
    printf("Failed to produce GeoVRML models.\n");
    TsmGeoVRML_destroy(&app);
    return -1;
  }

  TsmGeoVRML_destroy(&app);
  return 0;
}

/* EOF: make_geovrml.c */
