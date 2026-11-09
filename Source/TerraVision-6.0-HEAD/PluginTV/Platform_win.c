/*****************************************************************************
 * Name:
 *    	Platform_win.c
 *
 * Description:
 *	MS-Windows specific routines
 *
 * Function List:
 *
 * Dependencies:
 *
 *  ----- BEGIN LICENSE BLOCK -----
 *  Version: MPL 1.1
 * 
 *  The contents of this file are subject to the Mozilla Public License Version
 *  1.1 (the "License"); you may not use this file except in compliance with
 *  the License. You may obtain a copy of the License at
 *  http://www.mozilla.org/MPL/
 * 
 *  Software distributed under the License is distributed on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 *  for the specific language governing rights and limitations under the
 *  License.
 * 
 *  The Original Code is the TerraVision 3D Terrain Visualization System.
 * 
 *  The Initial Developer of the Original Code is SRI International.
 *
 *  Portions created by the Initial Developer are Copyright (C) 1992-2002
 *  the Initial Developer. All Rights Reserved.
 * 
 *  Contributor(s): Yvan Leclerc, Lee Iverson, Martin Reddy, Nat Bletter. 
 *  ----- END LICENSE BLOCK -----
 *
 *****************************************************************************/

#include "TerraVisionInc.h"
#include "ThreeDCore.h"
#include "ThreeDCoreI.h"
#include "ThreeDWidgetI.h"
#include "ThreeDWidget.h"

#include <tvutils/tvutils.h>

#include <glt/glm.h>

#include <windowsx.h>

#include <pthr/list.h>
#include <pthr/trace.h>

#include <prinrval.h>

UINT  initial_timer = 0;

LRESULT CALLBACK 
WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
static void ThreeDWidgetAddTimerHandler (void **loc, 
					 void *closure, unsigned int msecs, 
					 void (*f) (void *closure),
					 void *f_closure);
static void ThreeDWidgetDelTimerHandler (void **loc, void *closure);


int
PlatformSetupGraphics( ThreeDWidget *pluginWidget )
{
  const char     *funcName = "PlatformSetupGraphics";
  PlatformWidget *platform = &pluginWidget->platform;
  WNDCLASS       wc;

  /* get the settings from the command line */

  platform->hWnd = (HWND) tvuGetInt( TVU_NS_WIN_ID );
  pluginWidget->width = tvuGetInt( TVU_WIDTH );
  pluginWidget->height = tvuGetInt( TVU_HEIGHT );

  /* get this module's instance */

  platform->hInst = GetModuleHandle( NULL );

  /* enforce a default window size if not defined */

  if ( pluginWidget->width  == 0 ) pluginWidget->width = 450;
  if ( pluginWidget->height == 0 ) pluginWidget->height = 450;

  /* are we running as a plugin or a standalone? */

  if ( platform->hWnd == NULL ) {

    TSMTRACE(( INIT_MASK|1, funcName, "Creating dummy window context" ));

    pluginWidget->is_plugin = FALSE;
    pluginWidget->x         = 0;
    pluginWidget->y         = 0;

    /*  register window class */

    wc.style         = CS_OWNDC;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = platform->hInst;
    wc.hIcon         = LoadIcon( NULL, IDI_APPLICATION );
    wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = (HBRUSH) GetStockObject( BLACK_BRUSH );
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = "TerraVision";
    RegisterClass( &wc );

    /* create main window */

    platform->hWnd =
      CreateWindow( "TerraVision", "TerraVision Plugin", WS_CAPTION |
		    WS_SIZEBOX | WS_SYSMENU | WS_VISIBLE |
		    WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
		    0, 0, pluginWidget->width, pluginWidget->height,
		    NULL, NULL, platform->hInst, NULL );

  } else {

    /* use the HWND handle given to us from Netscape, but create   */
    /* a child canvas off of this so that we can have full control */
    /* over the event loop and all the window properties.          */

    HWND   parenthWnd = platform->hWnd;
    DWORD  style = WS_CHILD | WS_VISIBLE;

    pluginWidget->is_plugin = TRUE;
    pluginWidget->x         = 0;
    pluginWidget->y         = 0;

    /* get the window handle as an integer from the command line */

    wc.style         = CS_VREDRAW | CS_DBLCLKS | CS_OWNDC;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = platform->hInst;
    wc.hIcon         = LoadIcon( NULL, IDI_APPLICATION );
    wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = (HBRUSH) GetStockObject( BLACK_BRUSH );
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = "TerraVision";

    RegisterClass( &wc );

    /* now allocate the window */

    platform->hWnd =
      CreateWindowEx( 0,                    /* Extended Style */
		      "TerraVision",        /* Registered Class name */
		      "",                   /* Window Name */
		      style,                /* Window Style */
		      0,                    /* horz pos of window */
		      0,                    /* vert pos of window */
		      pluginWidget->width,  /* window width */
		      pluginWidget->height, /* window height */
		      (HWND) parenthWnd,    /* handle to parent window */
		      NULL,                 /* handle to child window */
		      platform->hInst,      /* handle to app instance */
		      NULL                  /* window creation data */
		      );

  }

  return TRUE;
}

void
EmulateMMB( ThreeDWidget *pluginWidget, int lmb, int *mmb, int rmb, int x,
	    int y )
{
  static int prev_mmb = FALSE;

  /* emulate the middle mouse button by pressing the left and right */

  if ( lmb && rmb && ! prev_mmb ) {
    ThreeDWidgetBtnUp( pluginWidget, x, y );
    ThreeDWidgetBtn2Down( pluginWidget, x, y );
    *mmb = TRUE;
  }

  /* release the middle mouse button when left and right released */

  if ( lmb == FALSE && rmb == FALSE && prev_mmb ) {
    ThreeDWidgetBtn2Up( pluginWidget, x, y );
    *mmb = FALSE;
  }

  prev_mmb = *mmb;
}

LRESULT CALLBACK
WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
  const char     *funcName = "WndProc";
  tvBase         *threeDCore = pluginWidget->threeDCore;
  PlatformWidget *platform = &pluginWidget->platform;
  static int     x = 0, y = 0;
  static int     lmb = FALSE, mmb = FALSE, rmb = FALSE;
  int            lx, ly, winsize[2], num;
  
  /* these delta are used for the cursor key movements */

  double pitch_delta = 2.0 * M_PI / 180.0;
  double yaw_delta   = 2.0 * M_PI / 180.0;
  double slide_delta = 50.0;
  double zoom_delta  = 50.0;

  /* We may create subwindows *during* the initialization process.
     Please don't do anything at all here. -- leei */
  if (pluginWidget->platform.initializing)
    return DefWindowProc (hWnd, message, wParam, lParam);

  /* check to see if we have called the initialisation routine */

  if ( pluginWidget->platform.initCB && initial_timer) {
    KillTimer( pluginWidget->platform.hWnd, initial_timer );
    (*pluginWidget->platform.initCB)();
    pluginWidget->platform.initCB = NULL;
    /* ThreeDWidgetInitCallback( pluginWidget ); */
  }
  
  /* get the previous and current mouse coordinates */
  
  lx = x;
  ly = y;

  x = GET_X_LPARAM( lParam );
  y = GET_Y_LPARAM( lParam );
  
  /* now deal with the message that we have pending */

  switch ( message ) {

  case WM_CREATE:
    return 0;

  case WM_CLOSE:
    PostQuitMessage( 0 );
    return 0;

	case WM_ERASEBKGND:
		return FALSE;

  case WM_DESTROY:
    return 0;

  case WM_LBUTTONDOWN:
    lmb = TRUE;
    ThreeDWidgetBtn1Down( pluginWidget, x, y );
    EmulateMMB( pluginWidget, lmb, &mmb, rmb, x, y );
    SetFocus( pluginWidget->platform.hWnd );  /* grab window focus */
    return 0;

  case WM_LBUTTONUP:
    lmb = FALSE;
    ThreeDWidgetBtnUp( pluginWidget, x, y );
    EmulateMMB( pluginWidget, lmb, &mmb, rmb, x, y );
    return 0;

  case WM_MBUTTONDOWN:
    mmb = TRUE;
    ThreeDWidgetBtn2Down( pluginWidget, x, y );
    SetFocus( pluginWidget->platform.hWnd );  /* grab window focus */
    return 0;

  case WM_MBUTTONUP:
    mmb = FALSE;
    ThreeDWidgetBtn2Up( pluginWidget, x, y );
    return 0;

  case WM_RBUTTONDOWN:
    rmb = TRUE;
    ThreeDWidgetBtn3Down( pluginWidget, x, y );
    EmulateMMB( pluginWidget, lmb, &mmb, rmb, x, y );
    SetFocus( pluginWidget->platform.hWnd );  /* grab window focus */
    return 0;

  case WM_RBUTTONUP:
    rmb = FALSE;
    ThreeDWidgetBtnUp( pluginWidget, x, y );
    EmulateMMB( pluginWidget, lmb, &mmb, rmb, x, y );
    return 0;

  case WM_MOUSEMOVE:
    ThreeDWidgetMotion( pluginWidget, x, y );
    return 0;

  case WM_SIZE:
    if ( threeDCore ) {
      winsize[0] = x;  winsize[1] = y;
      tvSetIntegerv( threeDCore, TV_SIZE, winsize );
      tvBasePostRedraw( threeDCore, FALSE );
    } else
      PostMessage( hWnd, message, wParam, lParam );
    return 0;

  case WM_PAINT:
    if ( threeDCore ) tvBasePostRedraw( threeDCore, FALSE );
    ValidateRect( platform->hWnd, NULL );
    return 0;

  case WM_KEYUP:

    switch( wParam ) {
      /* Record whether the shift key is currently depressed */

    case VK_SHIFT:
      TSMTRACE ((THREED_MASK|1, funcName, "Release SHIFT"));
      pluginWidget->shiftDown = FALSE;
      break;

      /* Record whether the ctrl key is currently depressed */

    case VK_CONTROL:
      TSMTRACE ((THREED_MASK|1, funcName, "Release CTRL"));
      pluginWidget->ctrlDown = FALSE;
      break;

      /* Record whether the alt key is currently depressed */

    case VK_MENU:
      TSMTRACE ((THREED_MASK|1, funcName, "Release ALT"));
      pluginWidget->altDown = FALSE;
      break;

    }

    /* can't seem to detect ALT being down, so emulate with SHIFT+CTRL */

    if ( pluginWidget->ctrlDown == FALSE && pluginWidget->ctrlDown == FALSE )
      pluginWidget->altDown = FALSE;

    return 0;

  case WM_KEYDOWN:
 
    if ( threeDCore ) tvBasePostRedraw( threeDCore, FALSE );

    switch ( wParam ) {

      /* SPACE = stop + refresh window (simulate a MMB click) */

    case VK_SPACE:
      lmb = rmb = mmb = FALSE;
      ThreeDWidgetBtn2Down( pluginWidget, lx, ly );
      ThreeDWidgetBtn2Up( pluginWidget, lx, ly );
      return 0;

      /* ESC = quit (only in standalone version) */

    case VK_ESCAPE:
      if ( ! pluginWidget->is_plugin )
      PostQuitMessage( 0 );
      return 0;

      /* BACKSPACE = unload models/datasets */

    case VK_BACK:
      if ( ( num = tvNumDatasets( threeDCore ) ) > 0 ) {
        TsGeoPyramid *geo = tvGetNthDataset( threeDCore, num-1 );
        if ( geo ) tvRemoveDataset( threeDCore, geo );
      }
      break;
      
      /* 2 = go to 2-D mode */

    case '2':
      tvSetBoolean( threeDCore, TV_TWOD_CONSTRAIN, TRUE );
      tvBaseUpdateView ( threeDCore, TRUE );
      break;

      /* 3 = go to 3-D mode */

    case '3':
      tvSetBoolean( threeDCore, TV_TWOD_CONSTRAIN, FALSE );
      tvBaseUpdateView ( threeDCore, FALSE );
      break;

    /* O = reset to origin */

    case 'O':
    case 'o':
      lmb = rmb = mmb = FALSE;
      ThreeDWidgetBtn2Down( pluginWidget, lx, ly );
      ThreeDWidgetBtn2Up( pluginWidget, lx, ly );
      tvBaseInitView( threeDCore, tvBaseTop(threeDCore) );
      break;

    /* D = toggle dataset outlines */

    case 'D':
    case 'd':
      tvSetBoolean( threeDCore, TV_SHOW_DATASETS, 
         ! tvGetBoolean( threeDCore, TV_SHOW_DATASETS ) );
      break;

    /* W = toggle wireframe mode */

    case 'W':
    case 'w':
      tvSetBoolean( threeDCore, TV_SHOW_WIREFRAME, 
         ! tvGetBoolean( threeDCore, TV_SHOW_WIREFRAME ) );
      break;

    /* H = toggle the HUD */

    case 'H':
    case 'h':
      tvSetBoolean( threeDCore, TV_SHOW_HEADS_UP, 
		    ! tvGetBoolean( threeDCore, TV_SHOW_HEADS_UP ) );
      tvBasePostRedraw( threeDCore, FALSE );
      break;

      /* L = toggle the SRI Logo */

    case 'L':
    case 'l':
      tvSetBoolean( threeDCore, TV_SHOW_LOGO, 
		    ! tvGetBoolean( threeDCore, TV_SHOW_LOGO ) );
      tvBasePostRedraw( threeDCore, FALSE );
      break;

      /* - = toggle z-buffer hack */

    case '-':
      tvSetBoolean( threeDCore, TV_USE_ZBUFFER_HACK,
		    ! tvGetBoolean( threeDCore, TV_USE_ZBUFFER_HACK ) );
      tvBaseUpdateView( threeDCore, FALSE );
      break;

      /* Left Arrow = rotate or slide to the left */

    case VK_LEFT:
      if ( pluginWidget->shiftDown )
	tvMoveViewpoint( threeDCore, 0, -slide_delta, 0, FALSE );
      else
	tvRotate( threeDCore, yaw_delta, 0, 0, FALSE );
      break;

      /* Right Arrow = rotate or slide to the right */
      
    case VK_RIGHT:
      if ( pluginWidget->shiftDown )
	tvMoveViewpoint( threeDCore, 0, slide_delta, 0, FALSE );
      else
	tvRotate( threeDCore, -yaw_delta, 0, 0, FALSE );
      break;

      /* Up Arrow = increase pitch, slide, or zoom */

    case VK_UP:
      if ( pluginWidget->shiftDown )
	tvMoveViewpoint( threeDCore, 0, 0, -slide_delta, FALSE );
      else if ( pluginWidget->ctrlDown )
	tvMoveViewpoint( threeDCore, zoom_delta, 0, 0, FALSE );
      else
	tvRotate( threeDCore, 0, -pitch_delta, 0, FALSE );
      break;

      /* Down Arrow = decrease pitch, slide, or zoom */

    case VK_DOWN:
      if ( pluginWidget->shiftDown )
	tvMoveViewpoint( threeDCore, 0, 0, slide_delta, FALSE );
      else if ( pluginWidget->ctrlDown )
	tvMoveViewpoint( threeDCore, -zoom_delta, 0, 0, FALSE );
      else
	tvRotate( threeDCore, 0, pitch_delta, 0, FALSE );
      break;

      /* PgUp = fly to next viewpoint in bookmark list */
      
    case VK_PRIOR:
      ViewpointCycleUp( pluginWidget );
      break;
      
      /* PgUp = fly to previous viewpoint in bookmark list */
      
    case VK_NEXT:
      ViewpointCycleDown( pluginWidget );
      break;
      
      /* Record whether the shift key is currently depressed */

    case VK_SHIFT:
      TSMTRACE ((THREED_MASK|1, funcName, "Press SHIFT"));
      pluginWidget->shiftDown = TRUE;
      break;

      /* Record whether the ctrl key is currently depressed */

    case VK_CONTROL:
      TSMTRACE ((THREED_MASK|1, funcName, "Press CTRL"));
      pluginWidget->ctrlDown = TRUE;
      break;

      /* Record whether the alt key is currently depressed */

    case VK_MENU:
      TSMTRACE ((THREED_MASK|1, funcName, "Press ALT"));
      pluginWidget->altDown = TRUE;
      break;

    }

    /* can't seem to detect ALT being down, so emulate with SHIFT+CTRL */

    pluginWidget->altDown = pluginWidget->ctrlDown && pluginWidget->ctrlDown;

    return 0;

  default:
    /* printf( "0x%x ", message ); fflush( stdout ); */
    return DefWindowProc( hWnd, message, wParam, lParam );
  }
}

void
PlatformShowWindow( ThreeDWidget *pluginWidget )
{
  /* nothing to do here under windows */
}

void
PlatformTimedFunc( ThreeDWidget *pluginWidget, void (*func)(void), int ms )
{
  /* we implement this in the WndProc callback */
  pluginWidget->platform.initCB = func;
  initial_timer = SetTimer( pluginWidget->platform.hWnd, 102, ms, NULL );
}

typedef struct {
  unsigned int singleBuffer:1;
  unsigned int stereo:1;
  unsigned int slow:1;
  int n, len;
  glmContextProperties *props;
} glmContextList;

static int
GatherContexts (glmContextProperties const *props,
		void *closure)
{
  static char const funcName[] = "GatherContexts";
  glmContextList *ctxs = closure;
  int i;

  if (props->overlay || props->underlay || props->colorIndex
      || props->singleBuffer != ctxs->singleBuffer
      || props->stereo != ctxs->stereo
      || props->slow != ctxs->slow)
    return 0;
  if (props->zbufDepth < 16
      || props->colorDepth < 8)
    return 0;

  i = ctxs->n++;
  if (i >= ctxs->len) {
    ctxs->len = ctxs->len*2;
    ctxs->props = realloc (ctxs->props, ctxs->len * sizeof (ctxs->props[0]));
  }

  PTRACE ((GLT_MASK|1, funcName, 
	   "[%d] %#x: %s%s %s%s%s P:%d D:%d S:%d M:%d/%d\n",
	   i, props->id,
	   props->colorIndex ? "IDX" : "RGB",
	   props->overlay ? "/Over" : props->underlay ? "/Under" : "",
	   props->singleBuffer ? "S" : "D",
	   props->stereo ? "/St" : "",
	   props->slow ? " Slow" : "",
	   props->colorDepth,
	   props->zbufDepth,
	   props->stencilDepth,
	   props->multiBufs, props->multiBufSamples));
  ctxs->props[i] = *props;
  return 0;
}

unsigned long
FindBestVisual (HWND parentWnd, int w, int h, glmContextList const *ctxs)
{
  static char const funcName[] = "FindBestVisual";
  float best_score = 0;
  unsigned long best_visual = 1;
  HMODULE hInst = GetModuleHandle( NULL );
  WNDCLASS       wc;
  int i;

  PTRACE ((GLT_MASK|1, funcName, "%d modes", ctxs->n));
  wc.style         = CS_OWNDC;
  wc.lpfnWndProc   = DefWindowProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = hInst;
  wc.hIcon         = LoadIcon( NULL, IDI_APPLICATION );
  wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
  wc.hbrBackground = (HBRUSH) GetStockObject( BLACK_BRUSH );
  wc.lpszMenuName  = NULL;
  wc.lpszClassName = "TVTimer";
  if (! RegisterClass( &wc )) {
    DWORD err = GetLastError ();
    fprintf (stderr, "RegisterClass failed.  Error %d\n", err);
    return best_visual;
  }
  
  for (i = 0; i < ctxs->n; ++i) {
    char name[16];
    glmContextProperties const *props = &ctxs->props[i];
    glmContext glmCtx;
    HWND hWnd;

    PTRACE ((GLT_MASK|1, funcName, "Testing [%d]", i));
    PTRINC ((GLT_MASK|1, 1));

    sprintf (name, "tvTimer:%d", i);
    hWnd = CreateWindowEx( 0,                    /* Extended Style */
			   "TVTimer",            /* Registered Class name */
			   name,                 /* Window Name */
			   WS_CHILD | WS_VISIBLE,/* Window Style */
			   0,                    /* horz pos of window */
			   0,                    /* vert pos of window */
			   w,                    /* window width */
			   h,                    /* window height */
			   parentWnd,            /* handle to parent window */
			   NULL,                 /* handle to child window */
			   hInst,                /* handle to app instance */
			   NULL                  /* window creation data */
			   );

    if (! hWnd) {
      DWORD err = GetLastError ();
      tsmError (funcName, "CreateWindowEx failed.  Error %d\n", err);
      continue;
    }

    glmCtx = glmWCreateContext (hWnd, props->id);
    if (glmCtx) {
      glmWindow glmw =
	glmWCreateWindow(hWnd, glmCtx, ctxs->singleBuffer, ctxs->stereo);
      int speed, score;

      speed = glmWindowSpeed (w, h, glmw);
      score = speed * (props->colorDepth 
		       + props->zbufDepth 
		       + props->stencilDepth);

      PTRACE ((GLT_MASK|1, funcName, "[%d]: speed %d score %d\n",
	       props->id, speed, score));
      if (score > best_score) {
	best_score = score;
	best_visual = props->id;
      }
    
      glmDestroyWindow (glmw);
      glmDestroyContext (glmCtx);
    } else {
      tsmError (funcName, "Couldn't create glmContext (%p, %d)\n", 
		hWnd, props->id);
    }

    DestroyWindow (hWnd);
    PTRINC ((GLT_MASK|1, -1));
  }

  return best_visual;
}

int
PlatformInitThreeDWidget( ThreeDWidget *threeDWidget )
{
  const char   *funcName = "PlatformInitThreeDWidget";
  tvBase       *threeDCore;
  HWND          hWnd = threeDWidget->platform.hWnd;
  glmContext    glmCtx;
  glmWindow     glmw;
  int           iFormat = 1;

  /* Tell the system that we're busy */
  threeDWidget->platform.initializing = TRUE;

  glmCtx = tvuGetContext( hWnd );
  if (! glmCtx) return FALSE;

  glmw = glmWCreateWindow(hWnd, glmCtx, ! threeDWidget->singleBuffer, FALSE );

  threeDWidget->threeDCore = threeDCore =
    tvBaseCreate (threeDWidget, threeDWidget->title, glmw);

  /* Tell the system that we're done */
  threeDWidget->platform.initializing = FALSE;

  return TRUE;
}

void
ThreeDWidgetInitCallback( ThreeDWidget *threeDWidget )
{
  char const   funcName[] = "ThreeDWidgetInitCallback";
  tvBase       *threeDCore = threeDWidget->threeDCore;
  int          winsize[2];

  TSMTRACE ((THREED_MASK|1, funcName, "threeDWidget=%p", threeDWidget));
  TSMTRINC ((THREED_MASK|1, 1));

  /* Set the size of the window */

  winsize[0] = threeDWidget->width;
  winsize[1] = threeDWidget->height;

  tvSetIntegerv( threeDCore, TV_SIZE, winsize );

  /* Check for a saved origin */
  ThreeDWidgetStartRendering ( threeDWidget );

  /* display an initial loading screen */
  tvDrawLoadScreen( threeDCore );

  TSMTRINC ((THREED_MASK|1, -1));
}

void
PlatformGraphicsLoop( void *arg )
{
  /* not used under Windows - event loop must be in same thread as creator */
}

void
PlatformCloseDown( ThreeDWidget *pluginDWidget )
{
  PlatformWidget *platform = &pluginWidget->platform;
}

static PRLock *timer_lock = NULL;
static list_t *timer_list = NULL;

typedef struct {
  PRIntervalTime action_time;
  ThreeDWidget   *pluginWidget;
  void           (*f) (void *closure);
  void           *closure;
} TimerClosure;

void
TimerHandler( void )
{
  TimerClosure *tc;
  list_iter_t  i;
  list_t       *del_list = NULL;

  if ( timer_lock == NULL || timer_list == NULL ) return;

  /* pop off all of the timers that have expired onto del_list */

  PR_Lock( timer_lock ); {
    
    for ( i = list_first( timer_list); ! list_done( i ); ) {
      tc = list_value( i );

      if ( (int) ( tc->action_time - PR_IntervalNow() ) <= 0 ) {
	if ( ! del_list ) del_list = list_create( FALSE );
	list_push( del_list, tc );
	i = list_delete( i );
      } else {
	i = list_next( i );
      }
    }

  } PR_Unlock( timer_lock );

  /* go through del_list and call each callback, then destroy the event */

  if ( del_list ) {
    for ( i = list_first( del_list); ! list_done( i ); ) {
      tc = list_value( i );
      (*tc->f) (tc->closure);
      i = list_delete( i );
      free (tc);
    }

    list_destroy( del_list );
  }
}

static void
ThreeDWidgetAddTimerHandler (void **loc, 
			     void *closure, unsigned int msecs, 
			     void (*f) (void *closure), void *f_closure)
{
  PRUint32 ticks_per_msec = PR_TicksPerSecond() / 1000;

  TimerClosure *tc = malloc (sizeof (*tc));
  *loc = tc;
  if ( tc == NULL ) return;

  tc->f            = f;
  tc->closure      = f_closure;
  tc->pluginWidget = (ThreeDWidget *) closure;
  tc->action_time  = PR_IntervalNow() + msecs * ticks_per_msec;

  if ( ! timer_lock )
    timer_lock = PR_NewLock();

  if ( ! timer_list )
    timer_list = list_create( FALSE );

  PR_Lock( timer_lock ); {
    list_push( timer_list, tc );
  } PR_Unlock( timer_lock );
}

static void
ThreeDWidgetDelTimerHandler (void **loc, void *closure)
{
  
  TimerClosure *tc = *loc;

  if (tc) {
    /* Clear the reference location to "take control" of tc */
    *loc = 0;

    PR_Lock( timer_lock ); {
      if ( list_remove( timer_list, tc ) )
	free( tc );
    } PR_Unlock( timer_lock );
  }
}

