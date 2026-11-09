/*
 * Filename: nptervis_all.h
 *
 * Purpose:
 *   Contains definitions for the platform independent parts of
 *   the TerraVision Netscape plugin.
 *
 *   N.B. this file is called nptervis_all.h instead of just
 *   nptervis.h to avoid compile conflicts with _gen/nptervis.h
 *
 * Author:
 *   Martin Reddy, SRI International.
 *
 * Revision:
 *   $Id: nptervis_all.h,v 1.8 2000/11/10 18:50:37 leei Exp $
 *
 * (c) Copyright 1993-1999. SRI International. All rights reserved.
 *
 */

#ifndef NPTERVIS_ALL_H
#define NPTERVIS_ALL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "npapi.h"

/* Define IMPLEMENT_nptervis before including nptervis.h to state that */
/* we are implementing the native methods of this plug-in here, and so */
/* need to access it's protected and private memebers.                 */

#define IMPLEMENT_nptervis
#include "_gen/nptervis.h"
#include "_gen/netscape_plugin_Plugin.h"

#ifndef WIN32
#include <prtypes.h>
#include <prproces.h>
#include <prthread.h>
#include <prio.h>
#include <prsystem.h>
#else
#include <windows.h>
#define  strcasecmp stricmp
#endif

/* This is the Cross-Platform PluginInstance structure that is used  */
/* to contain all the state that we record about our plugin instance */

typedef struct _PluginInstance {
  NPWindow*         fWindow;             /* A Netscape window struct */
  uint16            fMode;               /* NP_EMBED or NP_FULL */
  NPP               fNPP;                /* Netscape's instance struct */
  void              *winHandle;          /* fWindow->window */
  int               debug_mode;          /* output debug messages? */
  char              **argv;              /* command line args from embed */
  int               argc;                /* size of argv array from embed */
#ifdef WIN32
  HANDLE            pipe0_in;            /* Win32 handle for pipe0 to app */
  HANDLE            pipe1_out;           /* Win32 handle for pipe1 to app */
  HWND              fhWnd;               /* Win32 window handle */
  WNDPROC           fWinProc;            /* Win32 event callback */
#else
  PRFileDesc        *pipe0_in;           /* input for pipe0 to ext app */
  PRFileDesc        *pipe1_out;          /* output for pipe1 to ext app */
  PRProcessAttr     *pattr;              /* NSPR process attributes */
#endif
} PluginInstance;

/* These are the generic non-platform-specific functions that we use */

void    DisplayJavaMessage( PluginInstance *plugin, char const *msg );
void    AppendMsg( char const *msg );
void    DebugMessage( PluginInstance *plugin, char const *fmt, ... );
void    ShowStatus( PluginInstance *plugin, char const *msg );
int     FileExists( char const *filename );
char    *GetTerraVisionBinaryName( void );

/* There are the platform specific functions */

void    PlatformNew( PluginInstance *plugin );
NPError PlatformDestroy( PluginInstance *plugin );
void    PlatformEventLoop( PluginInstance *plugin, NPWindow *window );

int16   NPP_HandleEvent( NPP instance, void *event );
int16   PlatformHandleEvent( PluginInstance *plugin, void *event );

int     TalkToTerraVision( PluginInstance *plugin, char const *message );
int     RunTerraVision( PluginInstance *plugin, const char *, int, int, int );

char    *GetTerraVisionBinaryName( void );

#endif /* NPTERVIS_ALL_H */
