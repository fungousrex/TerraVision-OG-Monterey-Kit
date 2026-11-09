/*
 * Name: tsmPackTspecs.h
 *
 * Description: 
 *
 * Function List:
 *
 * Revision History:
 *
 *   $Id: tsmPackTspecs.h,v 1.8 2000/08/28 22:15:58 reddy Exp $
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

#ifndef TSMPACKTSPECS_H
#define TSMPACKTSPECS_H

#include "TsTileSpecDecs.h"

char *pyramidPack( Pyramid *p_p, int *len );
Pyramid *pyramidUnpack( char *p, int len );

int nr_TileSetTspec( int infd, Pyramid *top_tspec );
int nw_TileSetTspec( int outfd, Pyramid *top_tspec );

#endif /* TSMPACKTSPECS_H */
