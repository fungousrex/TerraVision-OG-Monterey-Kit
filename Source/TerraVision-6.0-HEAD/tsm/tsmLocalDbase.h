/*
 * Name: tsmLocalDbase.h
 *
 * Description:
 *   Contains all of the data structure definitions for the local  
 *   Pyramid database.
 *
 * Structure List:
 *   NameUrlIds
 *   Pyramids
 *
 * Dependencies:
 *
 * Revision History: 
 *   February 1998 - Initial version by Martin Reddy
 *
 *   $Id: tsmLocalDbase.h,v 1.6 2000/08/28 22:15:55 reddy Exp $
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

#ifndef TSMLOCALDBASE_H
#define TSMLOCALDBASE_H

#include "TsTileSpecDecs.h"

/*
 * Description:
 *   Holds an array of dataset name/URL/SetId tuples.
 *
 *   The TsmLocalDbase structure is used to store all of the information
 *   contained in a local TSM database,
 *   e.g. the .../TileSets/Pyramids/PyramidUrls file. 
 *
 * @param numItems (int) = number of items in the other arrays
 * @param names (char **) = array of strings containing dataset names
 * @param urls  (urls **) = array of strings containing URLs for the dataset
 * @param ids   (int *) = array of ints containing the set id for the dataset
 *                        (if on DPSS).
 *
 * @see 
 *
 */

struct TsmLocalDbase {
  int  numItems;
  char const **names;
  char const **urls;
  int  *ids;
};

typedef struct TsmLocalDbase TsmLocalDbase;

typedef struct TsmLocalDbase NameUrlIds;

/*
 * Doc++ Description: *//** 
 *   Holds an array of Pyramid structures.
 *
 *   The Pyramids structure can be used to contain an arbitrary number
 *   of pointers for Pyramid tspec structures.
 *
 * @param len (int) = number of *Pyramid pointers
 * @param val (Pyramid **) = array of *Pyramid pointers
 *
 * @see Pyramid
 *
 */

struct Pyramids {
  int     len;
  Pyramid **val;
};

typedef struct Pyramids Pyramids;


#endif
