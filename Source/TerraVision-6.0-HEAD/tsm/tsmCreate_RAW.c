/*
 * Name: tsmCreate_RAW.c
 *
 * Description: 
 *   This file contains functions for generating tiled terrain data
 *   from raw input data files. This is done in an extensible manner
 *   that allows new file format readers to be easily plugged in.
 *   Checkout out the tsmReadRows*.[ch] files for these readers.
 *
 * Function List:
 *
 * Revision History:
 * 
 *   $Id: tsmCreate_RAW.c,v 1.26 2000/11/06 23:50:27 leei Exp $
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

#include "tsmInternal.h"

#define DEF_DOWNSAMPLE_IMAGE(name,T)					\
static void								\
name (void const * const *inv, int in_w, int in_h, int n, int flags,	\
      void * const *outv)						\
{									\
  T const * const * const in = (T const * const *) inv;			\
  T * const * const out = (T * const *) outv;				\
  int i, j, k;								\
									\
  for (j = 0; j < in_h-1; j += 2) {					\
    T const * in_r0 = (T const *) in[j];				\
    T const * in_r1 = (T const *) in[j+1];				\
    T * out_r = out[j/2];						\
    for (i = 0; i < in_w-1; i += 2) {					\
      for (k = 0; k < n; ++k)						\
	out_r[k] = (in_r0[k]+in_r0[k+n]+in_r1[k]+in_r1[k+n] + 2)/4;	\
      in_r0 += 2*n;							\
      in_r1 += 2*n;							\
      out_r += n;							\
    }									\
    if (i < in_w) {							\
      for (k = 0; k < n; ++k)						\
	out_r[k] = (in_r0[k]+in_r1[k] + 1)/2;				\
    }									\
  }									\
  if (j < in_h) {							\
    T const * in_r0 = (T const *) in[j];				\
    T * out_r = out[j/2];						\
    for (i = 0; i < in_w-1; i += 2) {					\
      for (k = 0; k < n; ++k)						\
	out_r[k] = (in_r0[k]+in_r0[k+n] + 1)/2;				\
      in_r0 += 2*n;							\
      out_r += n;							\
    }									\
    if (i < in_w) {							\
      for (k = 0; k < n; ++k)						\
	out_r[k] = in_r0[k];						\
    }									\
  }									\
}

DEF_DOWNSAMPLE_IMAGE(downsample_image_i8,     PRInt8)
DEF_DOWNSAMPLE_IMAGE(downsample_image_u8,     PRUint8)
DEF_DOWNSAMPLE_IMAGE(downsample_image_i16,    PRInt16)
DEF_DOWNSAMPLE_IMAGE(downsample_image_u16,    PRUint16)
DEF_DOWNSAMPLE_IMAGE(downsample_image_i32,    PRInt32)
DEF_DOWNSAMPLE_IMAGE(downsample_image_u32,    PRUint32)
DEF_DOWNSAMPLE_IMAGE(downsample_image_float,  float)
DEF_DOWNSAMPLE_IMAGE(downsample_image_double, double)

#if 0
static void
downsample_image_u32 (void const * const *inv,
		      int in_w, int in_h, int n, int flags,
		      void * const *outv)
{
  typedef PRUint32 T;
  T const * const * const in = (T const * const *) inv;
  T * const * const out = (T * const *) outv;
  int i, j, k;

  for (j = 0; j < in_h-1; j += 2) {
    T const * in_r0 = (T const *) in[j];
    T const * in_r1 = (T const *) in[j+1];
    T * out_r = out[j/2];
    for (i = 0; i < in_w-1; i += 2) {
      for (k = 0; k < n; ++k) {
	out_r[k] = (in_r0[k]+in_r0[k+n]+in_r1[k]+in_r1[k+n] + 2)/4;
	if (flags&TSM_DOWNSAMPLE_DEM && out_r[k] > 10000) 
	  tsmError (funcName, "[%d,%d] %d,%d; %d,%d => %d",
		    i, j, in_r0[k], in_r0[k+n], in_r1[k], in_r1[k+n],
		    out_r[k]);
      }
      in_r0 += 2*n;
      in_r1 += 2*n;
      out_r += n;
    }
    if (i < in_w) {
      for (k = 0; k < n; ++k) {
	out_r[k] = (in_r0[k]+in_r1[k] + 1)/2;
	if (flags&TSM_DOWNSAMPLE_DEM && out_r[k] > 10000)
	  tsmError (funcName, "[%d,%d] %d; %d => %d",
		    i, j, in_r0[k], in_r1[k], out_r[k]);
      }
    }
  }
  if (j < in_h) {
    T const * in_r0 = (T const *) in[j];
    T * out_r = out[j/2];
    for (i = 0; i < in_w-1; i += 2) {
      for (k = 0; k < n; ++k) {
	out_r[k] = (in_r0[k]+in_r0[k+n] + 1)/2;
	if (flags&TSM_DOWNSAMPLE_DEM && out_r[k] > 10000)
	  tsmError (funcName, "[%d,%d] %d,%d => %d",
		    i, j, in_r0[k], in_r0[k+n], out_r[k]);
      }
      in_r0 += 2*n;
      out_r += n;
    }
    if (i < in_w) {
      for (k = 0; k < n; ++k) {
	out_r[k] = in_r0[k];
	if (flags&TSM_DOWNSAMPLE_DEM && out_r[k] > 10000)
	  tsmError (funcName, "[%d,%d] %d => %d",
		    i, j, in_r0[k], out_r[k]);
      }
    }
  }
}
#endif

#define DEF_DOWNSAMPLE_POINT(name,T)					\
static void								\
name (void const * const *inv, int in_w, int in_h, int ncomps, int flags,\
      void * const *outv)						\
{									\
  T const * const * const in = (T const * const *) inv;			\
  T * const * const out = (T * const *) outv;				\
  int i, j, k;								\
									\
  for (j = 0; j < in_h; j += 2) {					\
    T const * in_r = in[j];						\
    T * out_r = out[j/2];						\
    for (i = 0; i < in_w; i += 2) {					\
      for (k = 0; k < ncomps; ++k)					\
	out_r[k] = in_r[k];						\
      in_r += 2*ncomps;							\
      out_r += ncomps;							\
    }									\
  }									\
}

DEF_DOWNSAMPLE_POINT(downsample_point_i8,     PRInt8)
DEF_DOWNSAMPLE_POINT(downsample_point_u8,     PRUint8)
DEF_DOWNSAMPLE_POINT(downsample_point_i16,    PRInt16)
DEF_DOWNSAMPLE_POINT(downsample_point_u16,    PRUint16)
DEF_DOWNSAMPLE_POINT(downsample_point_i32,    PRInt32)
DEF_DOWNSAMPLE_POINT(downsample_point_u32,    PRUint32)
DEF_DOWNSAMPLE_POINT(downsample_point_float,  float)
DEF_DOWNSAMPLE_POINT(downsample_point_double, double)

#define CONV_POINT(g,k,off0,off1,off2,off3)			\
  (+ g[0][0]*in_r0[(k)+(off0)] + g[0][1]*in_r0[(k)+(off1)]	\
   + g[0][2]*in_r0[(k)+(off2)] + g[0][3]*in_r0[(k)+(off3)]	\
   + g[1][0]*in_r1[(k)+(off0)] + g[1][1]*in_r1[(k)+(off1)]	\
   + g[1][2]*in_r1[(k)+(off2)] + g[1][3]*in_r1[(k)+(off3)]	\
   + g[2][0]*in_r2[(k)+(off0)] + g[2][1]*in_r2[(k)+(off1)]	\
   + g[2][2]*in_r2[(k)+(off2)] + g[2][3]*in_r2[(k)+(off3)]	\
   + g[3][0]*in_r3[(k)+(off0)] + g[3][1]*in_r3[(k)+(off1)]	\
   + g[3][2]*in_r3[(k)+(off2)] + g[3][3]*in_r3[(k)+(off3)])

#define DEF_DOWNSAMPLE_GAUSS(name,T)					\
static void								\
name (void const * const *inv, int in_w, int in_h, int n, int flags,	\
      void * const *outv)						\
{									\
  float const g[4][4] = {						\
    {.01808237202, .04915298332, .04915298332, .01808237202},		\
    {.04915298332, .1336116614 , .1336116614 , .04915298332},		\
    {.04915298332, .1336116614 , .1336116614 , .04915298332},		\
    {.01808237202, .04915298332, .04915298332, .01808237202}};		\
  T const * const * const in = (T const * const *) inv;			\
  T * const * const out = (T * const *) outv;				\
  int i, j, k;								\
  int max_i = (flags & TSM_DOWNSAMPLE_RIGHT_BORDER) ? in_w : in_w - 2;	\
									\
  for (j = 0; j < in_h; j += 2) {					\
    T const * in_r0 = (T const *) in[j-1];				\
    T const * in_r1 = (T const *) in[j];				\
    T const * in_r2 = (T const *) in[j+1];				\
    T const * in_r3 = (T const *) in[j+2];				\
    T * out_r = out[j/2];						\
    if (j == 0 && ! (flags & TSM_DOWNSAMPLE_TOP_BORDER))		\
      in_r0 = in_r1;							\
    if (j >= in_h-1 && ! (flags & TSM_DOWNSAMPLE_BOTTOM_BORDER))	\
      in_r2 = in_r1;							\
    if (j >= in_h-2 && ! (flags & TSM_DOWNSAMPLE_BOTTOM_BORDER))	\
      in_r3 = in_r2;							\
    i = 0;								\
    if (! (flags & TSM_DOWNSAMPLE_LEFT_BORDER)) {			\
      for (k = 0; k < n; ++k)						\
	out_r[0] = CONV_POINT (g, k, 0, 0, n, 2*n);			\
      in_r0 += 2*n; in_r1 += 2*n; in_r2 += 2*n; in_r3 += 2*n;		\
      out_r += n;							\
      i += 2;								\
    }									\
    for ( ; i < max_i; i += 2) {					\
      for (k = 0; k < n; ++k)						\
	out_r[0] = CONV_POINT (g, k, -n, 0, n, 2*n);			\
      in_r0 += 2*n; in_r1 += 2*n; in_r2 += 2*n; in_r3 += 2*n;		\
      out_r += n;							\
    }									\
    if (! (flags & TSM_DOWNSAMPLE_RIGHT_BORDER)) {			\
      int off = (i < in_h-1 ? n : 0);					\
      for (k = 0; k < n; ++k)						\
	out_r[0] = CONV_POINT (g, k, -n, 0, off, off);			\
    }									\
  }									\
}

DEF_DOWNSAMPLE_GAUSS(downsample_gauss_i8,     PRInt8)
DEF_DOWNSAMPLE_GAUSS(downsample_gauss_u8,     PRUint8)
DEF_DOWNSAMPLE_GAUSS(downsample_gauss_i16,    PRInt16)
DEF_DOWNSAMPLE_GAUSS(downsample_gauss_u16,    PRUint16)
DEF_DOWNSAMPLE_GAUSS(downsample_gauss_i32,    PRInt32)
DEF_DOWNSAMPLE_GAUSS(downsample_gauss_u32,    PRUint32)
DEF_DOWNSAMPLE_GAUSS(downsample_gauss_float,  float)
DEF_DOWNSAMPLE_GAUSS(downsample_gauss_double, double)


/*
 * Synopsis:
 *   void tsmDownSample_image(void const **in, 
 *                            int in_w, int in_h,
 *                            int bytes_per_comp, int comps_per_pixel,
 *                            int flags,
 *                            void **out)
 *
 * Doc++ Description: *//** 
 *   Downsamples an image indexed by in[j][i].
 *
 *   The input is of size width by height and each pixel consists of
 *   comps_per_pixel components (e.g. RGB is three) each of which is
 *   bytes_per_comp bytes long.  The output image is assumed to be scaled down
 *   by 1/2 from the input.
 *
 *   The flags describe the input data format.
 *   \begin{itemize}
 *     \item {\bf TSM_DOWNSAMPLE_TOP_BORDER}: The input image has a border on
 *           the top edge (i.e. in[-1][0] is valid).
 *     \item {\bf TSM_DOWNSAMPLE_BOTTOM_BORDER}: The input image has a border
 *           on the bottom edge (i.e. in[in_h][0] is valid).
 *     \item {\bf TSM_DOWNSAMPLE_LEFT_BORDER}: The input image has a border on
 *           the left edge (i.e. in[0][-1] is valid).
 *     \item {\bf TSM_DOWNSAMPLE_RIGHT_BORDER}: The input image has a border
             on the right edge (i.e. in[0][in_w] is valid).
 *     \item {\bf TSM_DOWNSAMPLE_FLOAT}: The input image contains floating
 *           point values.
 *     \item {\bf TSM_DOWNSAMPLE_SIGNED}: The input image contains signed
 *           values (not checked if float).
 *     \item {\bf TSM_DOWNSAMPLE_DEM}: The input image is a digital elevation
 *           map (this forces point downsampling).
 *     \item {\bf TSM_DOWNSAMPLE_GAUSSIAN}: The downsampling should be done
 *           with a Gaussian filter instead of point-by-point.
 *   \end{itemize}
 *
 *   N.B. When gaussian downsampling is requested (via the
 *   TSM_DOWNSAMPLE_GAUSSIAN bit in flags) then the image is assumed to have a
 *   1-pixel border on those sides not flagged as boundaries (e.g. by
 *   TSM_DOWNSAMPLE_LEFT_BORDER).
 *
 * @param    in              The input 2D image
 * @param    in_w            The width of the image in.
 * @param    in_h            The height of the image in.
 * @param    bytes_per_comp  The number of bytes in each pixel component.
 * @param    comps_per_pixel The number of components in each pixel.
 * @param    flags           A set of flags describing the input data format.
 * @param    out             The output 2D image (assumed to be of size in_w/2
 *                           by in_h/2)
 *
 * @return   Nothing (void)
 *
 * @author   Lee Iverson
 * @version  September 24, 1999
 *
 * @see      tsmDownSample_block
 *
 */

void
tsmDownSample_image (void const * const *in, 
		     int in_w, int in_h,
		     int bytes_per_comp, int comps_per_pixel,
		     int flags,
		     void * const *out)
{
  static char const funcName[] = "tsmDownSample_image";

  void (*ds_f) (void const * const *, int, int, int, int, void * const *) = 0;
  int is_dem    = (flags & TSM_DOWNSAMPLE_DEM);
  int is_signed = (flags & TSM_DOWNSAMPLE_SIGNED);
  int is_float  = (flags & TSM_DOWNSAMPLE_FLOAT);
  int is_gauss  = (flags & TSM_DOWNSAMPLE_GAUSSIAN);

  TSMTRACE ((TSMMASK|3, funcName,
	     "DS image %p %dx%d => %p: %dx%d B/pxl %s%s%s%s %s%s%s%s\n",
	     in[0], in_w, in_h, out[0], comps_per_pixel, bytes_per_comp,
	     is_dem ? "DEM" : "OI",
	     is_signed ? " SGN" : "",
	     is_float ? " FLT" : "",
	     is_gauss ? " GAUSS" : "",
	     (flags & TSM_DOWNSAMPLE_TOP_BORDER) ? "T" : "",
	     (flags & TSM_DOWNSAMPLE_BOTTOM_BORDER) ? "B" : "",
	     (flags & TSM_DOWNSAMPLE_LEFT_BORDER) ? "L" : "",
	     (flags & TSM_DOWNSAMPLE_RIGHT_BORDER) ? "R" : ""));

  if (is_dem) {
    if (is_gauss) {
      fprintf (stderr, "Don't gaussian downsample DEMs\n");
      exit (1);
    }

    if (bytes_per_comp == sizeof (signed char))
      ds_f = (is_signed ? downsample_point_i8 : downsample_point_u8);
    else if (bytes_per_comp == sizeof (short))
      ds_f = (is_signed ? downsample_point_i16 : downsample_point_u16);
    else if (is_float && bytes_per_comp == sizeof (float))
      ds_f = downsample_point_float;
    else if (bytes_per_comp == sizeof (int))
      ds_f = (is_signed ? downsample_point_i32 : downsample_point_u32);
    else if (is_float && bytes_per_comp == sizeof (double))
      ds_f = downsample_point_double;
  } else {
    if (bytes_per_comp == sizeof (signed char))
      ds_f = (is_gauss 
	      ? (is_signed ? downsample_gauss_i8 : downsample_gauss_u8)
	      : (is_signed ? downsample_image_i8 : downsample_image_u8));
    else if (bytes_per_comp == sizeof (short))
      ds_f = (is_gauss 
	      ? (is_signed ? downsample_gauss_i16 : downsample_gauss_u16)
	      : (is_signed ? downsample_image_i16 : downsample_image_u16));
    else if (is_float && bytes_per_comp == sizeof (float))
      ds_f = (is_gauss ? downsample_gauss_float : downsample_image_float);
    else if (bytes_per_comp == sizeof (int))
      ds_f = (is_gauss 
	      ? (is_signed ? downsample_gauss_i32 : downsample_gauss_u32)
	      : (is_signed ? downsample_image_i32 : downsample_image_u32));
    else if (is_float && bytes_per_comp == sizeof (double))
      ds_f = (is_gauss ? downsample_gauss_double : downsample_image_double);
  }

  if (! ds_f) {
    fprintf (stderr, "Can't downsample %d byte component %s images\n",
	     bytes_per_comp, (flags & TSM_DOWNSAMPLE_DEM) ? "DEM" : "OI");
    exit (1);
  }

  (*ds_f) (in, in_w, in_h, comps_per_pixel, flags, out);
}

/*
 * Synopsis:
 *   void tsmDownSample_block (void const *in, 
 *                             int width, int height, int span,
 *                             int bytes_per_comp, int comps_per_pixel,
 *                             int flags,
 *                             void *out, int out_span)
 *
 * Doc++ Description: *//** 
 *   Downsamples an image block.
 *
 *   The input is of size in_w by in_h represented as a contiguous block of
 *   memory with in_span pixels per row.  Each pixel consists of
 *   comps_per_pixel components (e.g. RGB is three) each of which is 
 *   bytes_per_comp bytes long.  The output image is assumed to be scaled down
 *   by 1/2 from the input.
 *
 *   N.B. When gaussian downsampling is requested (via the
 *   TSM_DOWNSAMPLE_GAUSSIAN bit in flags) then the image is assumed to have a
 *   1-pixel border on those sides not flagged as boundaries (e.g. by
 *   TSM_DOWNSAMPLE_LEFT_BORDER).
 *
 * @param    in              The input image as a block
 * @param    in_w            The width of the image in.
 * @param    in_h            The height of the image in.
 * @param    in_span         The row-to-row span of the image block (in pixels)
 * @param    bytes_per_comp  The number of bytes in each pixel component.
 * @param    comps_per_pixel The number of components in each pixel.
 * @param    flags           A set of flags describing the input data format.
 * @param    out             The output 2D image (assumed to be of size in_w/2
 *                           by in_h/2)
 * @param    out_span        The row-to-row span of the output block
 *
 * @return   Nothing (void)
 *
 * @author   Lee Iverson
 * @version  September 24, 1999
 *
 * @see      tsmDownSample_image
 *
 */

void
tsmDownSample_block (void const *in, 
		     int in_w, int in_h, int in_span,
		     int bytes_per_comp, int comps_per_pixel,
		     int flags,
		     void *out, int out_span)
{
  int is_gauss = (flags & TSM_DOWNSAMPLE_GAUSSIAN);
  int eff_h = (is_gauss ? in_h+2 : in_h);
  void const **in_dex =((void const **) alloca (eff_h * sizeof (void*)));
  void **out_dex = (void **) alloca (in_h/2 * sizeof (void*));
  int i;
  int bytes_per_pixel = bytes_per_comp * comps_per_pixel;
  int span_bytes;

  /* Initialize in_dex */
  span_bytes = bytes_per_pixel * in_span;
  if (is_gauss) {
    in_dex -= 1;
    if (flags & TSM_DOWNSAMPLE_TOP_BORDER) {
      in_dex[-1] = ((char *) in) - span_bytes;
    } else {
      /* Point to the first line */
      in_dex[-1] = in;
      flags &= ~TSM_DOWNSAMPLE_TOP_BORDER;
    }
  }

  for (i = 0; i < in_h; ++i) {
    in_dex[i] = in;
    in = ((char *) in) + span_bytes;
  }

  if (is_gauss) {
    if (flags & TSM_DOWNSAMPLE_BOTTOM_BORDER) {
      in_dex[in_h] = in;
    } else {
      /* Point to the last line */
      in_dex[in_h] = ((char *) in) - span_bytes;
      flags &= ~TSM_DOWNSAMPLE_BOTTOM_BORDER;
    }
  }

  /* Initialize out_dex */
  span_bytes = bytes_per_pixel * out_span;
  for (i = 0; i < in_h/2; ++i) {
    out_dex[i] = out;
    out = ((char *) out) + span_bytes;
  }

  tsmDownSample_image (in_dex, in_w, in_h, 
		       bytes_per_comp, comps_per_pixel, flags,
		       out_dex);
}

#define BLOCK (64)

typedef struct {
  int w, h;
  int span, span_h;
  int buf_h;
  int last_y;
  char *buf;
  char **rows;
} LevelBuf;

static void
tsmDownSample_byblocks (char * const *inbuf, int w, int h,
			int bytes_per_comp, int comps_per_pixel,
			int pixel_size, int flags,
			char * const *outbuf)
{
  int x_incr = BLOCK * pixel_size;
  int orig_flags = flags;
  int x, y;
  char const **in_dex = alloca ((h+2) * sizeof (in_dex[0]));
  int out_h = (h+1)/2;
  char **out_dex = alloca (out_h * sizeof (out_dex[0]));
  int i;

  ++in_dex;
  in_dex[-1] = inbuf[0];
  in_dex[h] = inbuf[h-1];
  for (i = 0; i < h; ++i) in_dex[i] = inbuf[i];
  if (flags & TSM_DOWNSAMPLE_GAUSSIAN) {
    if (flags & TSM_DOWNSAMPLE_TOP_BORDER) 
      in_dex[-1] = inbuf[-1];
    if (flags & TSM_DOWNSAMPLE_BOTTOM_BORDER)
      in_dex[h] = inbuf[h];
  }
  for (i = 0; i < out_h; ++i) out_dex[i] = outbuf[i];

  for (x = 0; x < w; x += BLOCK) {
    int block_w = (x+BLOCK <= w ? BLOCK : w-x);
    if (x > 0) flags |= TSM_DOWNSAMPLE_LEFT_BORDER;
    if (x+BLOCK < w) flags |= TSM_DOWNSAMPLE_RIGHT_BORDER;
    else if (! (orig_flags & TSM_DOWNSAMPLE_RIGHT_BORDER))
      flags &= ~TSM_DOWNSAMPLE_RIGHT_BORDER;
    for (y = 0; y < h; y += BLOCK) {
      int block_h = (y+BLOCK <= h ? BLOCK : h-y);
      if (y > 0) flags |= TSM_DOWNSAMPLE_TOP_BORDER;
      if (y+BLOCK < h) flags |= TSM_DOWNSAMPLE_BOTTOM_BORDER;
      else if (! (orig_flags & TSM_DOWNSAMPLE_BOTTOM_BORDER))
	flags &= ~TSM_DOWNSAMPLE_BOTTOM_BORDER;
      tsmDownSample_image ((void const * const *) &in_dex[y], block_w, block_h,
			   bytes_per_comp, comps_per_pixel, flags,
			   (void * const *) &out_dex[y/2]);
    }
    
    /* Increment image pointers. */
    for (i = -1; i <= h; ++i) in_dex[i] += x_incr;
    for (i = 0; i < out_h; ++i) out_dex[i] += x_incr/2;
  }
}

static int
tsmDownSample_inmemory_r (LevelBuf *levels, int l, int nlevels, int force,
			  char const *inbuf, int y0, int nrows, 
			  int bytes_per_comp, int comps_per_pixel,
			  int pixel_size, int flags,
			  int (*write_rows) (int level,
					     void const *buf, int w,
					     int row, int n, void *closure),
			  void *closure)
{
  static char const funcName[] = "tsmDownSample_inmemory_r";

  LevelBuf *level0 = &levels[l];
  int span = level0->w * pixel_size;
  int h0_span = level0->span_h;
  char * const *inbufp  = &level0->rows[y0%h0_span];

  if (! (*write_rows) (l, inbufp[0], y0, nrows, span, closure))
    return FALSE;

  if (l+1 < nlevels) {
    LevelBuf *level1 = &levels[l+1];
    int h1_span = level1->span_h;
    int last_y1 = level1->last_y;
    int y1 = y0/2;
    char * const *outbufp = &level1->rows[y1%h1_span];

    if (y0 > 0) flags |= TSM_DOWNSAMPLE_TOP_BORDER;
    if (y0+nrows < level0->h) flags |= TSM_DOWNSAMPLE_BOTTOM_BORDER;

    TSMTRACE ((TSMMASK|1, funcName, 
	       "L%d Downsample [%d-%d] buf[%d] => L%d [%d-%d] buf[%d]\n",
	       l,   y0, y0+nrows-1, inbufp-&level0->rows[0],
	       l+1, y1, y1+(nrows+1)/2-1, outbufp-&level1->rows[0]));
    tsmDownSample_byblocks (inbufp, level0->w, nrows,
			    bytes_per_comp, comps_per_pixel, 
			    pixel_size, flags,
			    outbufp);

    y0 += nrows; y1 = (y0+1)/2;
    if (y0 >= level0->h || y0%h0_span == 0) {
      if (! tsmDownSample_inmemory_r (levels, l+1, nlevels, y0 >= level0->h,
				      outbufp[0], last_y1, y1-last_y1,
				      bytes_per_comp, comps_per_pixel,
				      pixel_size, flags,
				      write_rows, closure))
	return FALSE;
      level1->last_y = y1;
      TSMTRACE ((TSMMASK|1, funcName, "L%d last_y %d\n", l+1, y1));
    }
  }

  return TRUE;
}

static int
tsmDownSample_inmemory_levels (LevelBuf *levels, int nlevels, int tile_size,
			       int bytes_per_comp, int comps_per_pixel,
			       int pixel_size, int flags,
			       int (*read_rows) (void *buf,
						 int row, int n, int nbytes, 
						 void *closure),
			       void *rclosure,
			       int (*write_rows) (int level,
						  void const *buf,
						  int row, int n, int nbytes,
						  void *closure),
			       void *wclosure)
{
  static char const funcName[] = "tsmDownSample_inmemory_levels";

  LevelBuf *level0 = &levels[0];
  LevelBuf *level1 = nlevels == 1 ? NULL : &levels[1];
  int span = level0->w * pixel_size;
  int h0_span = level0->span_h;
  int h1_span = level1 ? level1->span_h : 0;
  int height = level0->h;
  int y0, nrows;
  int last_y1 = 0;
  int orig_flags = flags;

  for (y0 = 0; y0 < height; ) {
    int y1 = y0/2;
    char * const *inbufp  = &level0->rows[y0%h0_span];
    char * const *outbufp;

    nrows = (y0+tile_size > height ? height-y0 : tile_size);
    if (! (*read_rows) (inbufp[0], y0, nrows, span, rclosure))
      return FALSE;
    if (! (*write_rows) (0, inbufp[0], y0, nrows, span, wclosure))
      return FALSE;

#if 0
    if ((flags&TSM_DOWNSAMPLE_DEM)
	&& bytes_per_comp == 4 && comps_per_pixel == 1) {
      PRInt32 const *px = (PRInt32*)inbufp[0];
      PRInt32 const *pend = (PRInt32*) &((char*)inbufp[0])[nrows*span];
      for (; px < pend; ++px) {
	if (px[0] > 10000) {
	  int off = px - (PRInt32*)inbufp[0];
	  int pxPerRow = (span/4);
	  tsmError (funcName, "L0: in[%d,%d] = %d!",
		    off%pxPerRow, off/pxPerRow+y0,
		    px[0]);
	}
      }
    }
#endif

    if ( ! level1 ) {
      y0 += nrows;
      continue;
    }

    outbufp = &level1->rows[y1%h1_span];

    flags = orig_flags;
    if (y0 > 0) flags |= TSM_DOWNSAMPLE_TOP_BORDER;
    if (y0+nrows < height) flags |= TSM_DOWNSAMPLE_BOTTOM_BORDER;

    TSMTRACE ((TSMMASK|1, funcName,
	       "L%d Downsample [%d-%d] buf[%d] => L%d [%d-%d] buf[%d]\n",
	       0, y0, y0+nrows-1, inbufp-&level0->rows[0],
	       1, y1, y1+(nrows+1)/2-1, outbufp-&level1->rows[0]));
    tsmDownSample_byblocks (inbufp, level0->w, nrows,
			    bytes_per_comp, comps_per_pixel, pixel_size,
			    flags, outbufp);

    y0 += nrows; y1 = (y0+1)/2;
    if (y0 >= height || y0%h0_span == 0) {
      if (! tsmDownSample_inmemory_r (levels, 1, nlevels, y0 >= height,
				      outbufp[0], last_y1, y1-last_y1,
				      bytes_per_comp, comps_per_pixel,
				      pixel_size, flags,
				      write_rows, wclosure))
	return FALSE;
      last_y1 = y1;
    }
  }

  return TRUE;
}

int
tsmDownSample_inmemory (int width, int height, int nlevels, int tile_size,
			int bytes_per_comp, int comps_per_pixel,
			int flags,
			int (*read_rows) (void *buf, 
					  int row, int n, int nbytes,
					  void *closure),
			void *rclosure,
			int (*write_rows) (int level,
					   void const *buf, 
					   int row, int n, int nbytes, 
					   void *closure),
			void *wclosure)
{
  static char const funcName[] = "tsmDownSample_inmemory";

  int pixel_size = bytes_per_comp * comps_per_pixel;

  LevelBuf *levels = alloca (nlevels * sizeof (levels[0]));
  int i, ret;
  int span_h = tile_size * (/*(flags&TSM_DOWNSAMPLE_GAUSSIAN)? 3 :*/ 2);

  for (i = 0; i < nlevels; ++i) {
    int j, span, buf_h;
    char *buf, **rows;

    levels[i].w = width;
    levels[i].h = height;
    levels[i].span = span = pixel_size * width;
    levels[i].span_h = span_h;
    levels[i].buf_h = buf_h = (height > span_h ? span_h : height);
    levels[i].last_y = 0;

    TSMTRACE ((TSMMASK|1, funcName,
	       "L%d size %dx%d buf_h %d\n", i, width, height, buf_h));

    levels[i].buf = buf = malloc (span * (buf_h+1));
    if (! buf) break;

    levels[i].rows = rows = malloc (buf_h * sizeof (rows[0]));
    if (! rows) break;

    for (j = 0; j < buf_h; ++j)
      rows[j] = &buf[j*span];

    width = (width+1)/2;
    height = (height+1)/2;
  }
  /* Failed to allocate enough mem. */
  if (i < nlevels) {
    for ( ; i >= 0; --i)
      free (levels[i].buf);
    return FALSE;
  }

  ret = tsmDownSample_inmemory_levels (levels, nlevels, tile_size,
				       bytes_per_comp, comps_per_pixel, 
				       pixel_size, flags,
				       read_rows, rclosure,
				       write_rows, wclosure);

  for (i = 0; i < nlevels; ++i) {
    free (levels[i].buf);
    free (levels[i].rows);
  }

  return ret;
}
			
