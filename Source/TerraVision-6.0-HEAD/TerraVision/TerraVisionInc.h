/* -*- tab-width: 2; -*-

  Name:  TerraVisionInc.h

  Description: Real bad name for include file

  Revision History:
      $Id: TerraVisionInc.h,v 2.59.2.2 2002/10/31 05:26:32 reddy Exp $

  ----- BEGIN LICENSE BLOCK -----
  Version: MPL 1.1
 
  The contents of this file are subject to the Mozilla Public License Version
  1.1 (the "License"); you may not use this file except in compliance with
  the License. You may obtain a copy of the License at
  http://www.mozilla.org/MPL/
 
  Software distributed under the License is distributed on an "AS IS" basis,
  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
  for the specific language governing rights and limitations under the
  License.
 
  The Original Code is the TerraVision 3D Terrain Visualization System.
 
  The Initial Developer of the Original Code is SRI International.

  Portions created by the Initial Developer are Copyright (C) 1992-2002
  the Initial Developer. All Rights Reserved.
 
  Contributor(s): Yvan Leclerc, Lee Iverson, Martin Reddy, Nat Bletter. 
  ----- END LICENSE BLOCK -----

*/

#ifndef TERRAVISIONINC_H
#define TERRAVISIONINC_H

#ifndef BYTE_DEFINED
typedef unsigned char byte;
#define BYTE_DEFINED
#endif

#ifdef HAVE_CONFIG_H
# ifdef MSVC
#  include "winbuild/config.h"
# else
#  include "config.h"
# endif
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <signal.h>
#include <errno.h>

#ifdef __sgi
#include <malloc.h>
#endif

#include <pthr/list.h>

#include "Trace.h"

#include "../RELEASE"

#ifdef WIN32
# define  WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

#ifndef M_PI
#define M_PI 3.1415926535
#endif

#ifndef sgi
#define trunc(x) ((x>=0) ? floor(x) : floor(x+0.9999999999))
#endif

#define rgb_to_hex(r,g,b) (0xFF000000 + (b << 16) + (g << 8) + r)

#endif /* end ifndef TERRAVISION_INC_H */
