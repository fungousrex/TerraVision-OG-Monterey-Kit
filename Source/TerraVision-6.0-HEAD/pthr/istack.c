/*
 * Name: istack.c
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: istack.c,v 1.7 2000/08/29 00:31:29 reddy Exp $
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "istack.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

/* Decrease the region size by this amount to help with malloc overheads */
#define REGION_FUDGE (8+2*sizeof(void*))

static inline int
log2i (unsigned int x)
{
  unsigned int i;
  for (i = 0; i < 32; ++i)
    if (x <= (1<<i))
      return i;
  return i;
}

static void _istack_alloc_failure (istack *is, int size);

istack *
istack_create (int size)
{
  istack *is = (istack *) malloc (sizeof (*is));
  if (size < 512) size = 512;
  is->region_size = size = (1<<log2i (size)) - REGION_FUDGE;
  is->region = (istack_region *) malloc (size);
  if (! is->region) {
    _istack_alloc_failure (is, size);
    return 0;
  }

  is->region->end = ((char*) is->region) + size;
  is->region->prev = 0;
  is->base = is->next_free = is->region->start;
  is->alignment_mask = 7;
  is->empty_object = 0;
  return is;
}

void
istack_destroy (istack *is)
{
  istack_region *r, *prev;
  for (r = is->region; r; r = prev) {
    prev = r->prev;
    free (r);
  }
  free (is);
}

int
istack_memory_used (istack *is)
{
  istack_region *r;
  int size = 0;
  for (r = is->region; r; r = r->prev) 
    size += (r->end - r->start);
  return size;
}

int
istack_alignment (istack *is, int v)
{
  int old = is->alignment_mask;
  is->alignment_mask = (1<<v)-1;
  return log2i (old+1);
}

static void
_istack_alloc_failure (istack *is, int size)
{
  if (is->alloc_notify)
    (*is->alloc_notify) (is, size, is->alloc_notify_closure);
  else
    exit (1);
}

void
_istack_expand (istack *is, int size)
{
  int partial = istack_object_size (is);
  int region_size = is->region_size;
  istack_region *region = is->region;
  istack_region *new_region;

  /* Either its a small request, or we make a bigger region. */
  if (partial+size >= region_size/2)
    region_size = (4<<log2i (partial+size)) - REGION_FUDGE;

  /* Allocate the new region. */
  {
    char *area = malloc (region_size);
    if (! area) {
      _istack_alloc_failure (is, region_size);
      return;
    }

    new_region = (istack_region *) area;
    new_region->prev = region;
    new_region->end = area + region_size;
  }

  /* Move a partial object over. */
  if (partial)
    memcpy (new_region->start, is->base, partial);

  /* If current region is actually empty, delete it. */
  if (!is->empty_object && is->base == region->start) {
    new_region->prev = region->prev;
    free (region);
  }

  /* Now make the new region current. */
  is->region = new_region;
  is->base = new_region->start;
  is->next_free = is->base + partial;
  is->empty_object = 0;
}

void *
_istack_unwind (istack *is, void *p)
{
  istack_region *r = is->region, *prev;
  istack_region *r_found = 0;

  /* Find the region containing p */
  if (p) {
    for ( ; r; r = prev) {
      if (p >= (void*)r->start && p < (void*)r->end) {
	r_found = r;
	break;
      }
      prev = r->prev;
    }
    if (! r_found)
      return 0;
  }

  /* Must assume this unless we unwound all the way. :-( */
  is->empty_object = !!p;

  /* Free all intervening regions. */
  for (r = is->region; r != r_found; r = prev) {
    prev = r->prev;
    if (! p && ! prev) {
      p = r->start;
      break;
    }
    free (r);
  }

  /* Finally set the region as partial. */
  is->region = r;
  is->base = is->next_free = p;

  return p;
}
