/*
 * Name: tsmPackTspecs.c
 *
 * Description: 
 *   conversion routines for the tspec datatype (to and from byte arrays) used
 *   for network communication, etc. with the DSM
 *   karan bhatia (7/19/95)
 *
 * Function List:
 *
 * Revision History:
 *
 *   "$Id: tsmPackTspecs.c,v 1.13 2000/08/28 22:15:57 reddy Exp $"
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

#include <rpc/rpc.h>

#ifdef sgi
#include <bstring.h>
#endif

#ifdef NEW_OFFSETARRAY
#define pyramidOffset(p) ((void) 0)
#define pyramidUnoffset(p) ((void) 0)
#else
static void pyramidOffset (Pyramid *p);
static void pyramidUnoffset (Pyramid *p);
#endif

int tsmCheckString(char **str_p);

/************************************************************************/
char *pyramidPack(Pyramid *p_p, int *len)
{
  static const char *funcName = "pyramidPack()";
  XDR x;
  char *p;
  int size;

  if (check_Pyramid_for_strings(p_p) < 0) {
    tsmError (funcName, "error formatting tspec information database for storage");
    return (NULL);
  } 

  if ((size = xdr_sizeof(xdr_Pyramid, (void *)p_p)) == 0) {
    fprintf(stderr, "Error in xdr_sizeof\n");
    *len = 0;
    return NULL;
  }
    
  p = (char *) tsmMemManagerMalloc (size * sizeof(char));
 
  xdrmem_create(&x, p, size, XDR_ENCODE);
    
  pyramidUnoffset (p_p);
  if (!xdr_Pyramid(&x, p_p)) {
    fprintf(stderr, "Error in xdr_Pyramid");
    *len = 0;
    return NULL;
  }
  pyramidOffset (p_p);
 
  *len = size;
  return p;
}
/* end store_Pyramid */

Pyramid *pyramidUnpack(char *p, int len)
{
  static const char *funcName = "pyramidUnpack()";
  XDR x;
  Pyramid *v = (Pyramid *) tsmMemManagerMalloc (sizeof (Pyramid));

  memset ((char *)v, 0, sizeof(Pyramid));

  xdrmem_create(&x, p, len, XDR_DECODE);
    
  if (!xdr_Pyramid(&x, v)) {
    fprintf(stderr, "Error in xdr_Pyramid");
    return NULL;
  }
  pyramidOffset (v);
 
  return v;
}


/******************************************************/

int
nw_TileSetTspec (int outfd, Pyramid *top_tspec)
{
  static const char *funcName = "nw_TileSetTspec()";
  XDR xdrs;
  char *xdr_buffer;
  u_int pos;
 
  if (check_Pyramid_for_strings(top_tspec) < 0) {
    tsmError (funcName, "Error checking for strings.", funcName);
    return(0);
  }
    
  if ((xdr_buffer = (char *) tsmMemManagerCalloc (100, sizeof(Pyramid))) == 0)
  {
    perror ("nw_TileSetTspec - calloc");
    return 0;
  }
    
  memset (&xdrs, 0, sizeof (xdrs));
  xdrmem_create (&xdrs, xdr_buffer, sizeof(Pyramid)*100, XDR_ENCODE);
    
  pyramidUnoffset (top_tspec);
  if (xdr_Pyramid (&xdrs, top_tspec) == 0)
  {
    fprintf (stderr, "nw_TileSetTspec - XDR encoding failed\n");
    return 0;
  }
  pyramidOffset (top_tspec);
 
  pos = xdr_getpos (&xdrs);
  TSMTRACE ((TSMMASK|4, funcName, "will write %d bytes.\n"
	     "('%d' takes %d bytes to store on this machine).", 
	     pos, pos, (int) sizeof (pos)));
  pos = htonl (pos);
  if (writen (outfd, (char *)&pos, (int) sizeof (pos)) < (int) sizeof (pos))
  {
    perror ("nw_TileSetTspec - write (size)");
    return 0;
  }
 
  TSMTRACE ((TSMMASK|4, funcName, "before write."));
  pos = ntohl (pos);
  if (writen (outfd, xdr_buffer, pos) < pos)
  {
    perror ("nw_TileSetTspec - write (buffer)");
    return 0;
  }
  TSMTRACE ((TSMMASK|4, funcName, "after write."));
  xdr_destroy (&xdrs);
  tsmMemManagerFree (xdr_buffer);
  
  return 1;
}
 
int
nr_TileSetTspec (int infd, Pyramid *top_tspec)
{
  static const char *funcName = "nr_TileSetTspec()";
  XDR xdrs;
  char *xdr_buffer;
  u_int buffer_size;
  int ret;
    
  memset (&xdrs, 0, sizeof (xdrs));
 
  TSMTRACE ((TSMMASK|4, funcName,
	     "OK, I'm reading %d bytes for the buffersize",
	     (int)sizeof(buffer_size)));
 
  if (readn (infd, (char *)&buffer_size, (int)sizeof (buffer_size)) < (int) 
      sizeof (buffer_size))
  {
    perror ("nr_TileSetTspec - read (size)");
    return 0;
  }
  
  buffer_size = ntohl (buffer_size);
 
  TSMTRACE ((TSMMASK|4, funcName,
	     "I'M NOT KIDDING--I'll read %d bytes.", buffer_size));
 
  if ((xdr_buffer = (char *) tsmMemManagerCalloc (1, buffer_size + 1)) == 0)
  {
    perror ("nr_TileSetTspec - calloc");
    return 0;
  }
  xdrmem_create (&xdrs, xdr_buffer, buffer_size, XDR_DECODE);  
 
  if ((ret = readn (infd, xdr_buffer, buffer_size)) < 0)
  {
    perror ("nr_TileSetTspec - read (buffer)");
    return 0;
  }
  else if (ret < buffer_size) {
    tsmError (funcName, "read %d instead of %d bytes.", ret, buffer_size);
    return 0;
  }
 
  TSMTRACE ((TSMMASK|4, funcName, "read %d bytes", ret));
 
  if (xdr_Pyramid (&xdrs, top_tspec) == 0)
  {
    fprintf (stderr, "nw_TileSetTspec - XDR decoding failed\n");
    return 0;
  }
  pyramidOffset (top_tspec);

  xdr_destroy (&xdrs);
  tsmMemManagerFree(xdr_buffer);
 
  return 1;
}

#ifndef NEW_OFFSETARRAY
/* Correct offset arrays assuming that current offsets are zero. */
static void
identityTspecOffset (IdentityTspec *identityTspec, int minLevel)
{
  static const char *funcName = "identityTspecOffset()";

  TSMTRACE ((TSMMASK|2, funcName, "%p minLevel=%d", identityTspec, minLevel));
  if (identityTspec->checkSumLrbt.off != 0) {
    tsmError (funcName, "IdentityTspec %p checkSumLrbt already offset!",
	      identityTspec);
    return;
  }
  identityTspec->checkSumLrbt.off = minLevel;
  identityTspec->checkSumLrbt.val -= minLevel;

  if (identityTspec->checkSumJpeg.off != 0) {
    tsmError (funcName, "IdentityTspec %p checkSumJpeg already offset!",
	      identityTspec);
    return;
  }
  identityTspec->checkSumJpeg.off = minLevel;
  identityTspec->checkSumJpeg.val -= minLevel;
}

static void
pyramidOffset (Pyramid *p)
{
  static const char *funcName = "pyramidOffset()";
  int minLevel = p->minLevel;
  int i;

  TSMTRACE ((TSMMASK|2, funcName, "%p [%d]", p, p->tspecLevel.len));
  TSMTRINC ((TSMMASK|2, 1));

  if (p->tspecLevel.off != 0) {
    tsmError (funcName, "Pyramid %p already offset!", p);
    return;
  }

  /* Index from zero, since this is currently not offset. */
  for (i = 0; i < p->tspecLevel.len; ++i)
    identityTspecOffset (&p->tspecLevel.val[i].identityTspec, minLevel);

  p->tspecLevel.off = minLevel;
  p->tspecLevel.val -= minLevel;

  identityTspecOffset (&p->identityTspec, minLevel);

  TSMTRINC ((TSMMASK|2, -1));
}

/* Remove offsets in offset arrays. */
static void
identityTspecUnoffset (IdentityTspec *identityTspec, int minLevel)
{
  static const char *funcName = "identityTspecUnoffset()";

  TSMTRACE ((TSMMASK|2, funcName, "%p minLevel=%d", identityTspec, minLevel));
  if (identityTspec->checkSumLrbt.off != minLevel) {
    tsmError (funcName, "IdentityTspec %p checkSumLrbt not properly offset!",
	      identityTspec);
    return;
  }
  identityTspec->checkSumLrbt.off = 0;
  identityTspec->checkSumLrbt.val += minLevel;

  if (identityTspec->checkSumJpeg.off != minLevel) {
    tsmError (funcName, "IdentityTspec %p checkSumJpeg not properly offset!",
	      identityTspec);
    return;
  }
  identityTspec->checkSumJpeg.off = 0;
  identityTspec->checkSumJpeg.val += minLevel;
}

static void
pyramidUnoffset (Pyramid *p)
{
  static const char *funcName = "pyramidUnoffset()";
  int minLevel = p->minLevel;
  int i;

  TSMTRACE ((TSMMASK|2, funcName, "%p [%d]", p, p->tspecLevel.len));
  TSMTRINC ((TSMMASK|2, 1));
  
  if (p->tspecLevel.off != minLevel) {
    tsmError (funcName, "Pyramid %p not properly offset!", p);
    return;
  }
  p->tspecLevel.off = 0;
  p->tspecLevel.val += minLevel;

  identityTspecUnoffset (&p->identityTspec, minLevel);

  /* Index from zero, since this is currently not offset. */
  for (i = 0; i < p->tspecLevel.len; ++i)
    identityTspecUnoffset (&p->tspecLevel.val[i].identityTspec, minLevel);

  TSMTRINC ((TSMMASK|2, -1));
}

#endif /* ! NEW_OFFSETARRAY */

/* bltierney@lbl.gov   http://george.lbl.gov/~tierney     tel: 510-486-7381 */ 
/* Brian L. Tierney,   Lawrence Berkeley National Laboratory (LBNL)         */
/* 1 Cyclotron Rd.  MS: 50B-2239,  Berkeley, CA  94720    fax: 510-486-6363 */ 
