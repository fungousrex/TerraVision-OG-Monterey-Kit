/*
 * Filename: test_inmem.c
 *
 * Description:
 *     This program was written to test the new tiling and down
 *     sampling routines in tsmApi 2.0.
 *
 *     This program was written for use with the tsmApi library from
 *     SRI International. For further information about this library,
 *     including downloads, documentation, and other examples, see:
 *
 *         http://www.tsmApi.com/
 *
 * Author:
 *     Yong Tze Chi, <chi@ai.sri.com> - August 2000.
 *
 * Revision Information:
 *
 *     $Id: test_inmem.c,v 1.3 2000/08/28 23:56:07 reddy Exp $
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
 */

#include <tsm/tsm.h>
#include <tsm/tsmIntProts.h>

static int
nlevels (int size, int tile_size)
{
  int n = 1;
  while (size > tile_size) {
    ++n;
    size = (size+1)/2;
  }
  return n;
}

typedef struct {
  int fd;
} RowClosure;

static int
read_rows (void *vbuf, int row, int n, int nbytes, void *closure)
{
  RowClosure *rowClosure = closure;
  char *buf = vbuf;

  printf ("L0 Read [%d] %d rows %dB/row\n", row, n, nbytes);
  memset (buf, 0x55, n*nbytes);

  return 1;
}

static int
write_rows (int level,
	    void const *vbuf, int row, int n, int nbytes,
	    void *closure)
{
  RowClosure *rowClosure = closure;
  char const *buf = vbuf;

  printf ("L%d Write [%d] %d rows %dB/row\n", level, row, n, nbytes);
  write (rowClosure->fd, buf, n*nbytes);
  return 1;
}

int
main ()
{
  RowClosure rowClosure;
  
  rowClosure.fd = open ("/dev/null", 1);
  tsmDownSample_inmemory (10001, 6000, nlevels (10001, 128), 128,
			  1, 3, 0,
			  read_rows, &rowClosure, write_rows, &rowClosure);

  close (rowClosure.fd);

  return 0;
}
