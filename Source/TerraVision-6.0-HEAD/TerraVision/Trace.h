/* -*- tab-width: 2; -*-

  Revision History:
      $Id: Trace.h,v 2.3.2.5 2002/10/31 05:26:37 reddy Exp $

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

#ifndef _tvTrace_h_
#define _tvTrace_h_

#include <tsm/tsmTrace.h>

#define VIS_MASK     (TSMMASK<<1)
#define TWOD_MASK    (TSMMASK<<2)
#define THREED_MASK  (TSMMASK<<3)
#define TEX_MASK     (TSMMASK<<4)
#define MAP_MASK     (TSMMASK<<5)
#define INIT_MASK    (TSMMASK<<6)
#define OGL_MASK     (TSMMASK<<7)
#define OAA_MASK     (TSMMASK<<8)
#define LOG_MASK     (TSMMASK<<9)
#define CACHE_MASK   (TSMMASK<<10)
#define DOTGEO_MASK  (TSMMASK<<11)

#ifdef TRACE_VIS
# define VISTR(x) printf x
#else
# define VISTR(x)
#endif

#ifdef TRACE_OGL
# define OGLTR(x) printf x
#else
# define OGLTR(x)
#endif

#ifdef TRACE_RPR
# define RPRTR(x) printf x
#else
# define RPRTR(x)
#endif

#endif /* _tvTrace_h_ */
