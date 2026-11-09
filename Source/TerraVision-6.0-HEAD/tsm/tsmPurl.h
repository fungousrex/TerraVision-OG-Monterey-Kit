/*
 * Name: tsmPurl.h
 *
 * Description: 
 *   Purl structure definitions.
 *
 *   Header file for the TSM functions that create and manipulate Purl
 *   ("parsed url") structs, and that parse URLs into Purls.
 *
 * Function List:
 *
 * Revision History:
 *
 *   "$Id: tsmPurl.h,v 1.13 2000/08/28 22:15:59 reddy Exp $"
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

#ifndef TSMPURL_H
#define TSMPURL_H

/* ================================================================
   The following are all the protocols that MAY be handled by the TSM,
   depending on some compile-time definitions.
   ================================================================ */

#include "tsmPurlDefns.h"
#include <stdio.h>

extern char *tsmProtocolStrings[tsmNumProtocols+1];

/* External Purl prototypes. */

/* ================================================================
   tsmMakePurl creates an empty Purl struct with
   NULL components and the "unknown protocol".
   ================================================================ */

Purl *tsmMakePurl (void);

/* ================================================================
   tsmCopyPurl makes a complete copy of purl.
   ================================================================ */

Purl *tsmCopyPurl (Purl *);

/* ================================================================
   tsmFreePurl frees the strings in purl and then frees
   purl itself.
   ================================================================ */

void tsmFreePurl (Purl *);

/* ================================================================
   tsmPrintPurl prints the components of purl to fp.
   ================================================================ */

void tsmPrintPurl (FILE *, const Purl *);

/* ================================================================
   tsmParseUrl parses a URL of the form:
 
   protocol://hostname/uri...
 
   where protocol must be one of the protocols supported by the Tile
   Set Manager, such as "x-iss" or "file".  
 
   If the URL does not conform to this pattern, then NULL is returned.
 
   Otherwise, a Purl is returned containing the URL components.

   NOTES:

   o The "/uri..." after hostname is optional.  

   o The "/" preceding "uri" above is always included in the uri slot
   of Purl, even if "uri" is empty or the preceding "/" is
   missing.

   o If the URL begins with "file://localhost", then the protocol is
   specialized from tsmFile to tsmLocalFile.  TODO: Generalize this to
   other hostnames by checking to see if the hostname is actually the
   local host.

   o When the protocol is tsmLocalFile, then uri's beginning with "/~"
   are translated to "/homedir".  For example, the uri for
   "file://localhost/~magic/Pyramids" becomes "/homedir/Pyramids".
   There's probably a better way of doing this, since not all systems
   use "/homedir" for user's directories.

   ================================================================ */

Purl *tsmParseUrl (const char *);

#endif

