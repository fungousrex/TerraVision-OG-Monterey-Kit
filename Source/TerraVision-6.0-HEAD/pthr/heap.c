/* -*- C -*- */

/*
 * This template is included in order to define the fast, inlined heap.
 * Before inlining, the following #define's must be made:
 *
 *    HEAP_PREFIX       => a unique prefix to use for function names.
 *    HEAP_T            => name of overall type.
 *    HEAP_ELEM_T       => type of element on heap.
 *    HEAP_VALUE_T      => type of element value carried.
 *    HEAP_VALUE (elem) => how to extract value from HEAP_ELEM_T.
 *    HEAP_COST_T       => type of element cost.
 *    HEAP_COST (elem)  => how to extract cost from HEAP_ELEM_T.
 *    HEAP_MAXCOST	=> maximum value of type HEAP_COST_T.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "heap.h"

#include <assert.h>

#define New(t) (t*) malloc (sizeof (t))
#define Newv(t,n) (t*) malloc ((n) * sizeof (t))
#define Renewv(t,p,n) (t*) realloc ((p), (n) * sizeof (t))

#define PARENT_IDX(i) (((i)-1)/2)
#define LEFT_IDX(i) (2*(i)+1)
#define RIGHT_IDX(i) (2*(i)+2)

struct heap {
  unsigned int length, next;
  heap_elem_t *elems;
  heap_cmp_t cmp;
  heap_update_t update;
};

heap_t *
heap_create (unsigned int len, heap_cmp_t cmp, heap_update_t upd)
{
  heap_t *h = New (heap_t);

  h->length = len;
  h->next = 0;
  h->cmp = cmp;
  h->update = upd;
  h->elems = Newv (heap_elem_t, len);

  return h;
}

void
heap_destroy (heap_t *h)
{
  free (h->elems);
  h->elems = 0;
  h->length = 0;
  free (h);
}

int
heap_size (heap_t const *h)
{
  return h->next;
}

/*
 * The top of the heap is the value with min cost.
 */
int
heap_is_empty (heap_t const *h)
{
  return h->next == 0;
}

/*
 * This is the primary invariant for the heap.  All operations should preserve
 * this relationship.
 */

static inline int
parent_larger (heap_t const *h, heap_elem_t parent, heap_elem_t child)
{
  return (h->cmp (parent, child) >= 0);
}

/*
 * Move an element and notify element it has been moved
 */
static inline void
heap_update (heap_t *h, int idx, heap_elem_t elem)
{
  h->elems[idx] = elem;
  if (h->update) h->update (elem, idx);
}

/*
 * Add <val,cost> pair at end of heap and bring it up.
 */
static int
heap_insert (heap_t *h, unsigned int idx, heap_elem_t elem)
{
  heap_elem_t *elems = h->elems;
  unsigned int next = idx;

  while (idx > 0) {
    unsigned int parent = PARENT_IDX(idx);

    /* Exit when parent is smaller. */
    if (parent_larger (h, elems[parent], elem))
      break;

    /* Exchange idx and parent. */
    heap_update (h, idx, elems[parent]);
    idx = parent;
  }

  /* Add the point. */
  heap_update (h, idx, elem);

  return next;
}

/*
 * Assume that elem is at idx and is larger than all parents.
 * Move it down the heap until it is smaller than all its children.
 */
static int
heap_adjust (heap_t *h, unsigned int idx, heap_elem_t elem)
{
  heap_elem_t *elems = h->elems;
  unsigned int next = h->next;

  while (1) {
    unsigned int child = LEFT_IDX(idx);
    heap_elem_t c1, c2;

    /* Don't fall off bottom of heap. */
    if (child >= next) break;

    /* Choose larger child. */
    c1 = elems[child];
    c2 = elems[child+1];
    if (child+1 < next && ! parent_larger (h, c1, c2)) {
      ++child;
      c1 = c2;
    }

    /* If we are smaller than children, done. */
    if (parent_larger (h, elem, c1))
      break;
	
    /* Exchange idx and child. */
    heap_update (h, idx, c1);
    idx = child;
  }

  /* Add the point. */
  heap_update (h, idx, elem);
    
  return next;
}

/*
 * Insert <val,cost> beyond the end of the array.
 */

heap_elem_t *
heap_maybe_resize (heap_t *h, unsigned int nelems)
{
  if (nelems) {
    while (h->next+nelems-1 >= h->length) {
      h->length *= 2;
      h->elems = Renewv (heap_elem_t, h->elems, h->length);
    }
  }

  return h->elems;
}

int
heap_add_nocheck (heap_t *h, heap_elem_t elem)
{
  assert (h->next < h->length);
  return heap_insert (h, h->next++, elem);
}

int
heap_add_noadjust (heap_t *h, heap_elem_t elem)
{
  int idx = h->next;

  heap_maybe_resize (h, 1);
  
  /* Add the point. */
  ++h->next;
  heap_update (h, idx, elem);

  return idx;
}

int
heap_add (heap_t *h, heap_elem_t elem)
{
  heap_maybe_resize (h, 1);

  return heap_add_nocheck (h, elem);
}

/*
 * Remove top element of heap and adjust below.
 */
heap_elem_t
heap_pop (heap_t *h)
{
  heap_elem_t elem = 0;

  if (h->next) {
    heap_elem_t *elems = h->elems;

    elem = elems[0];
    if (h->update) h->update (elem, -1);
    heap_update (h, 0, elems[--h->next]);
    heap_adjust (h, 0, elems[0]);
  }

  return elem;
}

static void
heapify (heap_t *h, int idx)
{
  heap_elem_t *elems = h->elems;
  heap_elem_t elem = elems[idx];
  int cidx = LEFT_IDX (idx);
  int lidx = idx;

  if (cidx < h->next) {
    /* Choose the smallest of elem, left(elem), right(elem).
       The chosen index goes in lidx and the element in elem */
    heap_elem_t child = elems[cidx];
    if (! parent_larger (h, elem, child)) {
      elem = child;
      lidx = cidx;
    }
    if (++cidx < h->next) {
      child = elems[cidx];
      if (! parent_larger (h, elem, child)) {
	elem = child;
	lidx = cidx;
      }
    }

    /* If we need to, swap elem and smallest and heapify */
    if (lidx != idx) {
      child = elems[idx];
      heap_update (h, idx, elem);
      heap_update (h, lidx, child);

      heapify (h, lidx);
    }
  }
}

int
heap_remove_idx (heap_t *h, heap_elem_t elem, int idx)
{
  int next;

  /* Check validity */
  if (idx < 0 || idx >= h->next) return 0;
  if (h->elems[idx] != elem) return 0;

  /* Swap this one with last and heapify */
  if (idx < (next = --h->next)) {
    heap_update (h, idx, h->elems[next]);
    heapify (h, idx);
  }

  return 1;
}

int
heap_recost_idx (heap_t *h, heap_elem_t elem, int idx)
{
  int next = h->next;
  int pidx;

  /* Check validity */
  if (idx < 0 || idx >= next) return 0;
  if (h->elems[idx] != elem) return 0;

  pidx = PARENT_IDX (idx);
  if (idx > 0 && ! parent_larger (h, h->elems[pidx], elem)) {
    heap_update (h, idx, elem);
  } else {
    int c1 = LEFT_IDX (idx);
    int c2 = RIGHT_IDX (idx);
    if ((c1 < next && ! parent_larger (h, elem, h->elems[c1]))
	|| (c2 < next && ! parent_larger (h, elem, h->elems[c2]))) {
      heapify (h, idx);
    }
  }

  return 1;
}

void
heap_check (heap_t *h)
{
  heap_elem_t *elems = h->elems;

  heap_elem_t *elem = elems;
  heap_elem_t *child = elems;
  heap_elem_t *next = &elems[h->next];

  for (; ; ++elem) {
    if (++child >= next) break;
    assert (parent_larger (h, *elem, *child) <= 0);
    if (++child >= next) break;
    assert (parent_larger (h, *elem, *child) <= 0);
  }
}

void
heap_forall (heap_t *h, void (*f)(void *v, void *c), void *closure)
{
  heap_elem_t *elems = h->elems;
  heap_elem_t *elem = elems;
  heap_elem_t *next = &elems[h->next];

  for (; elem < next; ++elem)
    (*f) (*elem, closure);
}
