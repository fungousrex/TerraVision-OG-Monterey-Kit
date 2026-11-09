/*
 * Name: tsmArray.h
 *
 * Description: 
 *   header file for the TSM functions that create and manipulate "arrays".
 *
 * Function List:
 *
 * Revision History:
 *
 *   "$Id: tsmArray.h,v 1.8 2000/08/28 22:15:24 reddy Exp $"
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

#ifndef TSMARRAY_H
#define TSMARRAY_H

/* ================================================================
   Integer arrays.
   ================================================================ */

struct IntArray {
  int len;
  int maxLen;
  int *array;
};
typedef struct IntArray IntArray;

IntArray *tsmMakeIntArray (int);
void tsmFreeIntArray (IntArray *);
int tsmMemberIntArray (IntArray *, int);
int tsmAppendIntArray (IntArray *, int);
int tsmGrowIntArray (IntArray *);

/* ================================================================
   TrItem arrays.
   ================================================================ */

/* A TrItem (Tile Request Item) is used by tsmReqTile etc to specify
   a tile request. */

struct TrItem {
  int ix;
  int iy;
  int level;
  int priority;
};

typedef struct TrItem TrItem;

struct TrArray {
  int    len;
  int    maxLen;
  TrItem *array;
};

typedef struct TrArray TrArray;

struct TrLists {
  int         trIndex;
  TrArray     *trArray;
};

typedef struct TrLists TrLists;

TrArray *tsmMakeTrArray (int);
void tsmFreeTrArray (TrArray *);
void tsmAppendTrArray (TrArray *);
int tsmGrowTrArray (TrArray *);

#endif
