/*
 * Name: pixmaps.c
 *
 * Description: instantiates the pixmaps that are used for the web icons.
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
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pixmaps.h"

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include "pixmaps/darpa.h"
#include "pixmaps/sri.h"
#include "pixmaps/magic.h"
#include "pixmaps/tv.h"
#include "pixmaps/icon.h"

#include "bitmaps/darpa.xbm"
#include "bitmaps/sri.xbm"
#include "bitmaps/magic.xbm"
#include "bitmaps/tv.xbm"
#include "bitmaps/icon.xbm"

Pixmap web_icon[NUMB_WEB_ICONS] = { 0 };
Pixmap icon_pixmap = 0;

static Pixmap MakeColourPixmap( Widget, int, int, unsigned char * );
static Pixmap MakeBWPixmap( Widget, int, int, char * );
static Pixmap MakePixmap( Widget, int, int, unsigned char *, char * );

/*
 * Synopsis:
 *   static Pixmap MakeColourPixmap( Widget, int, int, unsigned char * )
 *
 * Description:
 *   This function takes a pointer to a colour image bitmap and tries
 *   to create a colour Pixmap structure for it. We use a hardcoded
 *   bitmap format (see pixmaps/ dir) for efficiency - if the display
 *   cannot handle this format, e.g. if it is 8-bit, then this function
 *   will fail.
 *
 * Returns: a valid Pixmap structure of BadPixmap if failed
 *
 * Author: Martin Reddy
 * 
 * Date: 5 April, 1999
 *
 */

static Pixmap
MakeColourPixmap( Widget w, int width, int height, unsigned char *bitmap )
{
  Display   *dpy = XtDisplay( w );
  Window    win = XtWindow( w );
  XImage    *image;
  Pixmap    pixmap;
  XGCValues values;
  GC        gc;
  XWindowAttributes wattr;
  Visual    *visual;
  int       depth = 0;

  /* find out the depth of the parent window and the visual */

  XGetWindowAttributes( dpy, win, &wattr );
  visual = wattr.visual;
  depth = wattr.depth;

  /* we only support colour pixmaps in 24-bit mode */

  if ( depth < 24 ) return BadPixmap;

  /* okay, lets create an XImage structure from the raw bitmap */

  image = XCreateImage( dpy, visual, depth, ZPixmap, 0, (char *) bitmap,
			width, height, 32, width*4 );

  /* again, our data is in a very strict format: 32-bit */

  if ( image->bits_per_pixel != 32 ) {
    XDestroyImage( image );
    return BadPixmap;
  }

  /* we assume BGR byte ordering for the image. This matches the */
  /* default visual for O2 and Exceed, but not for Octane/Onyx2, */
  /* so if we get an RGB visual then we need to swap some bytes. */

  if ( image->red_mask == 0xff0000 ) {
    unsigned char *pixel = bitmap, temp;
    int           i, num_pixels = width * height;

    for ( i = 0; i < num_pixels; i++, pixel += 4 ) {
      temp     = pixel[3];
      pixel[3] = pixel[1];
      pixel[1] = temp;
    }
  }

  /* create a pixmap drawable to hold the image */

  pixmap = XCreatePixmap( dpy, win, width, height, depth );

  /* create a graphics context */

  values.foreground = 1;
  values.background = 0;
  gc = XCreateGC( dpy, pixmap, GCForeground|GCBackground, &values );

  /* render the image into the pixmap */

  XPutImage( dpy, pixmap, gc, image, 0, 0, 0, 0, width, height );

  /* clean up, and we're done. Now, that was easy wasn't it!! */
  /* XXX - actually, cleaning up seems to be bad under IRIX! */
  /* If we try doing the following cleanup, we get Killed!! */

  /* XFreeGC( dpy, gc ); */
  /* XDestroyImage( image );*/

  return pixmap;
}

/*
 * Synopsis:
 *   static Pixmap MakeBWPixmap( Widget, int, int, char * )
 *
 * Description:
 *   This function takes a pointer to an X11 bitmap file (.xbm file)
 *   and creates a black & white Pixmap structure from this. This
 *   should never fail.
 *
 * Returns: A valid Pixmap structure
 *
 * Author: Martin Reddy
 * 
 * Date: 5 April, 1999
 *
 */

static Pixmap
MakeBWPixmap( Widget w, int width, int height, char *bitmap )
{
  Display   *dpy = XtDisplay( w );
  Window    win = XtWindow( w );
  int       screenNum = DefaultScreen( dpy );
  Cardinal  depth;

  /* make sure we use the right depth value */

  XtVaGetValues( w, XtNdepth, &depth, NULL );

  /* create the pixmap from the appropriate bitmap file data */

  return XCreatePixmapFromBitmapData( dpy, win, bitmap, width, height,
				      BlackPixel( dpy, screenNum ),
				      WhitePixel( dpy, screenNum ),
				      depth );
}

/*
 * Synopsis:
 *   static Pixmap MakePixmap( Widget, int, int, uchar *, char * )
 *
 * Description:
 *   This function will take a colour bitmap and a black and white 
 *   bitmap for a single image. It will first try to make a Pixmap
 *   using the colour bitmap. If this fails for any reason, then
 *   it will create a Pixmap using the black and white bitmap.
 *
 *   It is assumed that the colour and B&W bitmaps are the same dimension.
 *
 * Returns: a valid Pixmap
 *
 * Author: Martin Reddy
 * 
 * Date: 5 April, 1999
 *
 */

static Pixmap
MakePixmap( Widget w, int width, int height, unsigned char *colour_data,
	    char *bw_data )
{
  Pixmap pixmap;

  /* Try to make a colour pixmap first */

  pixmap = MakeColourPixmap( w, width, height, colour_data );

  /* if this failed, then just use a black and white bitmap version */

  if ( pixmap <= BadPixmap )
    pixmap = MakeBWPixmap( w, width, height, bw_data );
    
  return pixmap;
}

/*
 * Synopsis:
 *   void MakeWebIconPixmaps( Widget wid )
 *
 * Description:
 *   This function creates Pixmaps for all of the web icons. The web 
 *   icons are those buttons at the bottom of the viewer window. We
 *   try to build these in colour, but if that's not possible then we
 *   fallback to black and white representations. 
 *
 *   Upon successful completion, the array web_icon will be filled 
 *   with NUMB_WEB_ICONS valid Pixmap structures.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 5 April, 1999
 *
 */

void
MakeWebIconPixmaps( Widget wid )
{
  if ( web_icon[0] != 0 ) return;

  web_icon[DARPA_PIXMAP] = MakePixmap( wid, width_darpa, height_darpa,
				       bitmap_darpa, darpa_bw_bits);
  web_icon[SRI_PIXMAP]   = MakePixmap( wid, width_sri, height_sri, bitmap_sri,
				       sri_bw_bits );
  web_icon[MAGIC_PIXMAP] = MakePixmap( wid, width_magic, height_magic,
				       bitmap_magic, magic_bw_bits);
  web_icon[TV_PIXMAP]    = MakePixmap( wid, width_tv, height_tv, bitmap_tv,
				       tv_bw_bits );
}

/*
 * Synopsis:
 *   Pixmap MakeIconifiedPixmap( Widget wid )
 *
 * Description:
 *   This function will create a Pixmap image which could be used
 *   for the icon when the application is iconified. 
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 5 April, 1999
 *
 */

Pixmap
MakeIconifiedPixmap( Widget wid )
{
  if ( ! icon_pixmap ) 
    icon_pixmap = MakePixmap( wid, width_icon, height_icon, bitmap_icon,
			      icon_bw_bits );

  return icon_pixmap;
}

