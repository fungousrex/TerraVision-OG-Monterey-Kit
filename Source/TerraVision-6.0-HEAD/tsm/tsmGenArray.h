/*
 * Name: tsmGenArray.h
 *
 * Description: 
 *   Macros to simplify the management of arrays and offset arrays.
 *
 * Function List:
 *
 * Revision History:
 *
 *   $Id: tsmGenArray.h,v 1.5 2000/08/28 22:15:47 reddy Exp $
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

#ifndef _TSMGENARRAY_H_
#define _TSMGENARRAY_H_

/* ================================================================
   Generic Arrays
   ================================================================ */

#include <rpc/types.h>

#define tsmArray(el_type) struct { u_int len; el_type *val; }

#define tsmArrayMalloc(el_type,arr,l) \
  ((arr).len = (l), \
   (arr).val = (el_type *) tsmMemManagerMalloc((arr).len*sizeof((arr).val[0])))

#define tsmArrayCalloc(el_type,arr,l) \
  ((arr).len = (l), \
   (arr).val = (el_type *) tsmMemManagerCalloc((arr).len, sizeof((arr).val[0])))

#define tsmArrayDup(el_type,dst,src) \
  (tsmArrayMalloc(el_type,dst,(src).len), \
   (dst).val \
    ? (memcpy ((dst).val, (src).val, (dst).len * sizeof((dst).val[0])), \
       (dst).val) \
   : (dst).val)

#define tsmArrayFree(arr) (tsmMemManagerFree((arr).val), (arr).val = 0)

#define declare_xdr_tsmArray(name) \
  bool_t xdr_##name (XDR *xdrs, name *arr);

#define define_xdr_tsmArray(name,el_type) \
  bool_t \
  xdr_##name (XDR *xdrs, name *arr) { \
    return xdr_array (xdrs, (caddr_t *)&arr->val, &arr->len, ~0, \
		      sizeof(el_type), (xdrproc_t) xdr_##el_type); \
  }

/* ================================================================
   Generic Offset Arrays
   ================================================================ */

#define tsmOffsetArray(el_type) struct { int off; u_int len; el_type *val; }

#define tsmOffsetArrayMalloc(el_type,arr,l,o) \
  ((arr).off = (o), \
   (arr).len = (l), \
   (arr).val = (el_type *) \
     tsmMemManagerMalloc((arr).len*sizeof((arr).val[0])),\
   (arr).val ? (arr).val -= (arr).off : (arr).val)

#define tsmOffsetArrayMallocRange(el_type,arr,mn,mx) \
  tsmOffsetArrayMalloc (el_type,arr,(mx)-(mn)+1,mn)

#define tsmOffsetArrayCalloc(el_type,arr,l,o) \
  ((arr).off = (o), \
   (arr).len = (l), \
   (arr).val = (el_type *) \
     tsmMemManagerCalloc((arr).len,sizeof((arr).val[0])),\
   (arr).val ? (arr).val -= (arr).off : (arr).val)

#define tsmOffsetArrayCallocRange(el_type,arr,mn,mx) \
  tsmOffsetArrayCalloc (el_type,arr,(mx)-(mn)+1,mn)

#define tsmOffsetArrayDup(el_type,dst,src) \
  (tsmOffsetArrayMalloc(el_type,dst,(src).len,(src).off), \
   (dst).val \
    ? (memcpy ((dst).val+(dst).off, (src).val+(src).off, \
	       (dst).len * sizeof((dst).val[0])), \
       (dst).val) \
   : (dst).val)

#define tsmOffsetArrayFree(arr) \
  (tsmMemManagerFree((arr).val+(arr).off), \
   (arr).val = 0)

#define declare_xdr_tsmOffsetArray(name) \
  bool_t xdr_##name (XDR *xdrs, name *arr);

#ifdef NEW_OFFSETARRAY

# define define_xdr_tsmOffsetArray(name,el_type) \
  bool_t \
  xdr_##name (XDR *xdrs, name *arr) { \
    bool_t ret = xdr_int (xdrs, &arr->off); \
    if (! ret) return ret;
    if (xdrs->x_op == XDR_ENCODE || xdrs->x_op == XDR_FREE) \
      arr->val += arr->off; \
    ret = xdr_array (xdrs, (caddr_t *)&arr->val, &arr->len, ~0, \
		     sizeof(el_type), (xdrproc_t) xdr_##el_type); \
    if (ret) arr->val -= arr->off; \
    return ret; \
  }

#else /* ! NEW_OFFSETARRAY */

# define define_xdr_tsmOffsetArray(name,el_type) \
  bool_t \
  xdr_##name (XDR *xdrs, name *arr) { \
    if (arr->off != 0) { \
      fprintf (stderr, "Uncorrected offset array %p in XDR", arr); \
      abort (); \
    } \
    if (xdrs->x_op == XDR_DECODE) arr->off = 0; \
    return xdr_array (xdrs, (caddr_t *)&arr->val, &arr->len, ~0, \
		      sizeof(el_type), (xdrproc_t) xdr_##el_type); \
  }

#endif /* NEW_OFFSETARRAY */

#endif /* _TSMGENARRAY_H_ */
