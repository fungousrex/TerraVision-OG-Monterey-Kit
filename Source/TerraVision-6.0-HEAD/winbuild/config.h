/*
  COPYRIGHT 2000, SRI INTERNATIONAL.
  ALL RIGHTS RESERVED.

  THIS SOFTWARE IS A PROTOTYPE.  IT IS PROVIDED ON AN "AS IS" BASIS.  DEFECTS
  AND ANOMALIES ARE PRESUMED TO EXIST.  THE SOFTWARE IS PROVIDED FREE OF
  CHARGE AT THE REQUEST OF THE UNITED STATES GOVERNMENT.  THE RECIPIENT
  ASSUMES ALL RISKS, KNOWN AND UNKNOWN, OF USING THE SOFTWARE.  SRI
  INTERNATIONAL EXPRESSLY DISCLAIMS, AND THE RECIPIENT WAIVES, ANY AND ALL
  WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, AS WELL AS ANY
  WARRANTIES REGARDING TITLE OR INFRINGEMENT OF THE INTELLECTUAL PROPERTY
  RIGHTS OF THIRD PARTIES.  THE RECIPIENT, BY ACCEPTING AND USING THE ENCLOSED
  SOFTWARE, AGREES TO HOLD HARMLESS SRI INTERNATIONAL FROM ANY LOSS, COSTS, OR
  DAMAGES INCURRED BY THE RECIPIENT, ITS AFFILIATES, PARENTS, OR SUBSIDIARIES,
  AND THEIR RESPECTIVE EMPLOYEES, OFFICERS AND AGENTS, ARISING OUT OF OR
  RELATING TO THEIR USE OF THE SOFTWARE.
*/

#ifndef __config_h__
#define __config_h__

#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

// hack to remove warning C4005: 'MSVC' : macro redefinition 
#ifdef MSVC
# undef MSVC
#endif // MSVC

#define MSVC

// hack to remove warning C4005: 'HAVE_CONFIG_H' : macro redefinition 
#ifdef HAVE_CONFIG_H
# undef HAVE_CONFIG_H
#endif // HAVE_CONFIG_H

// TerraVision defines
#define HAVE_SQL_H 
#define HAVE_ERRNO_H
#define HAVE_NSPR_H

// GeoWeb support
//#define HAVE_LIBGEO
#undef HAVE_LIBGEO

#ifndef TRACING
#define TRACING
#endif
#ifndef USE_HTTP
#define USE_HTTP
#endif
#ifndef HAVE_INLINE
#define HAVE_INLINE
#endif
#ifndef JPEG_SUPPORT
#define JPEG_SUPPORT
#endif

// NSPR defines
#define NO_NSPR_10_SUPPORT
#define XP_PC
#define WIN95
#define _PR_GLOBAL_THREADS_ONLY
#define _X86_
#define _NSPR_BUILD_

#ifndef strcasecmp
# define strcasecmp stricmp  // This is how it is defined in MSVC++ 5.0. 
#endif // strcasecmp

#ifndef strncasecmp
# define strncasecmp strnicmp  // This is how it is defined in MSVC++ 5.0. 
#endif // strncasecmp

#ifndef alloca
# define alloca _alloca
#endif // alloca

#define inline __inline

#endif // __config_h__
