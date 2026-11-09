/*
 * Name: tsmArray.c
 *
 * Description: 
 *   Array utility functions.  Use these functions to create "arrays" of
 *   things.
 *
 * Function List:
 *
 * Revision History:
 *
 *   $Id: tsmArray.c,v 1.11 2000/08/28 22:15:23 reddy Exp $
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

/* ================================================================
   ================================================================
   Integer arrays.
   ================================================================
   ================================================================ */

/* ================================================================
   tsmMakeIntArray creates an integer array struct with room for at
   least len integers.
   ================================================================ */

IntArray *
tsmMakeIntArray (int len)
{
  static char * funcName = "tsmMakeIntArray()";
  IntArray *intArray;

  intArray = (IntArray *) tsmMemManagerCalloc (1, sizeof (IntArray));
  if(!intArray) {
    tsmError (funcName, "Unable to allocate intArray");
    return NULL;
  }
  intArray->maxLen = max (8, len);
  intArray->len = 0;
  intArray->array = (int *)
     tsmMemManagerCalloc (1, intArray->maxLen * sizeof (int));
  if(!intArray->array) {
    tsmError (funcName, "Unable to allocate intArray->array");
    return NULL;
  }
  return (intArray);
}

void
tsmFreeIntArray (IntArray *intArray)
{
  tsmMemManagerFree (intArray->array);
  tsmMemManagerFree (intArray);
}

/* ================================================================
   tsmMemberIntArray returns the array index j at which array[j] ==
   i.  It returns -1 if there's no such index.
   ================================================================ */

int
tsmMemberIntArray (IntArray *intArray, int i)
{
 int *array, j;

  array = intArray->array;

  /*
   * This was originally i++. Me thinks that was wrong.
   * SL 01/18/96 <lau@ai.sri.com>
   */
  for (j = 0; j < intArray->len; j++) 
  {
      if (i == array[j])
      {
	  return (j);
      }
  }
  return (-1);
}

/* ================================================================
   tsmAppendIntArray will append i after the last entry in the array,
   growing the array if necessary.
   ================================================================ */

int
tsmAppendIntArray (IntArray *intArray, int i)
{
 static char * funcName = "tsmAppendIntArray()";

  if (intArray->len >= intArray->maxLen)
  {
      if(!tsmGrowIntArray (intArray)) {
	tsmError (funcName, "Unable to Grow array!");
	return FALSE;
      }
  }
  intArray->array[intArray->len] = i;
  intArray->len++;
  return TRUE;
}

/* ================================================================
   tsmGrowIntArray reallocs the internal array by a factor of two.
   ================================================================ */

int
tsmGrowIntArray (IntArray *intArray)
{
  static char * funcName = "tsmGrowIntArray()";

  intArray->maxLen = 2 * intArray->maxLen;
  intArray->array = (int *)
     tsmMemManagerRealloc (intArray->array, 
			   intArray->maxLen * sizeof (int));
  if(intArray->array == (int *) -1) {
    tsmError (funcName, "Unable to reallocate space for array!");
    return FALSE;
  }
  return TRUE;
}

/* ================================================================
   ================================================================
   TrItem arrays.
   ================================================================
   ================================================================ */

/* ================================================================
   tsmMakeTrArray creates an integer array struct with room for at
   least len integers.
   ================================================================ */

TrArray *
tsmMakeTrArray (int len)
{
  static char * funcName = "tsmMakeTrArray()";
  TrArray *trArray;

  trArray = (TrArray *) tsmMemManagerCalloc (1, sizeof (TrArray));
  if(!trArray) {
    tsmError (funcName, "Unable to allocate space for trArray");
    return NULL;
  }
  trArray->len = 0;
  trArray->maxLen = max (8, len);
  trArray->array = (TrItem *)
     tsmMemManagerCalloc (trArray->maxLen, sizeof (TrItem));
  if(!trArray->array) {
    tsmError (funcName, "Unable to allocate space for trArray->array");
    return NULL;
  }
  return (trArray);
}

void
tsmFreeTrArray (TrArray *trArray)
{
  tsmMemManagerFree (trArray->array);
  tsmMemManagerFree (trArray);
}

void
tsmAppendTrArray (TrArray *trArray)
{
  if (trArray->len >= trArray->maxLen)
    tsmGrowTrArray (trArray);
  trArray->len++;
}

/* ================================================================
   tsmGrowTrArray reallocs the internal array by a factor of two.
   ================================================================ */

int
tsmGrowTrArray (TrArray *trArray)
{
  trArray->maxLen = 2 * trArray->maxLen;
  trArray->array = (TrItem *) 
     tsmMemManagerRealloc (trArray->array, 
			   trArray->maxLen * sizeof (TrItem));
  if(trArray->array == (TrItem *) -1)
  {
      fprintf(stderr, "ERROR: Unable to realloc in tsmGrowTrArray!\n");
      return FALSE;
  }
  return TRUE;
}





