/*
 * Name: test_proto.c
 *
 * Library:
 *    This source file is part of the vrml2 library, a library for parsing
 *    and generating VRML97 format files. This library is supplied as part
 *    of the tsmApi distribution, available from:
 *
 *        http://www.tsmApi.com/
 *
 * Revision History:
 *   VRML 2.0 version by Chris Connolly, SRI International
 *   Updated interface by Lee Iverson, SRI International
 *
 *   $Id: test_proto.c,v 1.4 2000/08/28 23:21:42 reddy Exp $
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

#include <stdio.h>

#include "c_nodes.h"

typedef enum {
  VRMLCOORD = 0,
  VRMLCOORDINDEX,
  VRMLTEXURL,
  VRMLTEXCOORD,
  VRMLTEXCOORDINDEX,
  VRMLTEXTRANSFORM,
  UPVECTOR,
  TERRAINTEXURN,
  INFO,
  TITLE,
  TILEX,
  TILEY,
  POLYSPERSIDE,
  NUMFIELDS
} VtileFields;

typedef struct VtileField {
  vsgFieldType fieldType;
  vsgDataType type;
  char *name;
} VtileField;

VtileField vtileFields[] = {
  {vsgTfield, vsgSFNode, "vrmlCoord"},
  {vsgTfield, vsgMFInt32, "vrmlCoordIndex"},
  {vsgTfield, vsgMFString, "vrmlTexUrl"},
  {vsgTfield, vsgSFNode, "vrmlTexCoord"},
  {vsgTfield, vsgMFInt32, "vrmlTexCoordIndex"},
  {vsgTfield, vsgSFNode, "vrmlTexTransform"},
  {vsgTfield, vsgSFVec3f, "upVector"},
  {vsgTfield, vsgMFString, "terrainTexUrn"},
  {vsgTfield, vsgMFString, "info"},
  {vsgTfield, vsgSFString, "title"},
  {vsgTfield, vsgSFInt32, "tileX"},
  {vsgTfield, vsgSFInt32, "tileY"},
  {vsgTfield, vsgSFInt32, "polysPerSide"}
};


int 
main (int argc, char **argv)
{
  extern char *optarg;
  extern int optind;
  int c;
  vsgWorld *wrl, *wrl2;
  int syntax_only = 0;
  int print = 0;
  MFString *mfs;
  char vtileProto[] = "vtileProto";
  int field;
  MFNode *top;
  SFNode vtile;
  char buffer[] = "hello 123";

  while ((c = getopt(argc, argv, "dpst")) != EOF) {
    switch (c) {
    case 'p':
      print = 1;
      break;

    case 's':
      syntax_only = 1;
      break;

#if defined(YYDEBUG) && YYDEBUG
    case 'd': {
      extern int vsgpDebug;
      vsgpDebug = 1;
      break;
    }
    case 't': {
      ++vsgTracing;
      break;
    }
#endif
    }
  }

  if (! syntax_only)
    vsgcSetup ();

  /* vsgTracing = 2; */

  wrl = vsgNewWorld (0);
  vsgStartWorld (wrl);
  vsgSetFiletype(wrl, "#VRML V2.0 utf8");
  top = vsgWorldNodes (wrl);

  vsgStartExternProto (wrl, vtileProto); {
    for (field = 0; field < NUMFIELDS; field++)
      vsgDeclareField (wrl,
		       vtileFields[field].fieldType,
		       vtileFields[field].type,
		       vtileFields[field].name);

    mfs = vsgStartMFString(wrl);
    vsgAppendSFString(wrl, mfs, "../../lib/vtile.wrl");
    vsgDeclareExtern (wrl, vsgEndMFString(wrl, mfs));
  }; vsgEndExternProto (wrl);

  vtile = vsgNewNode (wrl, vtileProto);
  vsgAppendSFNode (wrl, top, vtile);

  mfs = vsgStartMFString (wrl);
  vsgAppendSFString (wrl, mfs, buffer);
  vsgEndMFString (wrl, mfs);
  vsgSet (wrl, vtile, vtileFields[VRMLTEXURL].name, mfs);

  {
    SFNode coord = vsgNewCoordinate (wrl);
    MFVec3f *mfv = vsgStartMFVec3f(wrl);
    vsgAppendSFVec3f (wrl, mfv, vsgMakeSFVec3f (1, 2, 3));
    vsgAppendSFVec3f(wrl, mfv, vsgMakeSFVec3f (3, 4, 5));
    vsgEndMFVec3f (wrl, mfv);

    vsgSet (wrl, coord, vsgNpoint, mfv);
    vsgSet (wrl, vtile, vtileFields[VRMLCOORD].name, coord);
  }

  vsgEndWorld (wrl);

  if (! syntax_only && print)
    vsgPrint (wrl, stdout);


  wrl2 = vsgNewWorld(0);

  {
    SFNode coord = vsgNewCoordinate (wrl2);
    MFVec3f *mfv = vsgStartMFVec3f(wrl2);

    vsgAppendSFVec3f (wrl, mfv, vsgMakeSFVec3f (3, 2, 1));
    vsgAppendSFVec3f(wrl2, mfv, vsgMakeSFVec3f (4, 3, 2));
    vsgEndMFVec3f (wrl2, mfv);

    vsgSet (wrl, coord, vsgNpoint, mfv);
    vsgSet (wrl, vtile, vtileFields[VRMLCOORD].name, coord);
  }

  if (! syntax_only && print)
    vsgPrint (wrl, stdout);

  vsgDestroyWorld (wrl2);

  vsgDestroyWorld (wrl);

  return 0;
}
