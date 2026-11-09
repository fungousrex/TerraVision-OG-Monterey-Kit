/*
  Filename: Platform.h

  Description: Contains platform-specific setup for the tvutils library,
  including any system-dependent headers and creating some standard 
  defines for us to detect which system we are under. Specifically:

    TVU_WINDOWS is only set if compiling under a Windows machine
    TVU_UNIX    is only set if compiling under a Unix machine

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

#ifndef TVU_PLATFORM_H
#define TVU_PLATFORM_H

#if defined( _WIN32_ ) || defined( WIN32 ) || defined( __WIN32__ )

#define TVU_WINDOWS
#undef  TVU_UNIX

#include <windows.h>

#else

#undef  TVU_WINDOWS
#define TVU_UNIX

#endif

#endif
