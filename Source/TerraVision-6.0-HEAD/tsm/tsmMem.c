/*
 * Name: tsmMem.c
 *
 * Description:
 *   Contains routines for performing common memory management operations.
 *
 * Function List:
 *
 * Dependencies:
 *
 * Revision History:
 *        $Date: 2000/08/28 22:15:57 $
 *        $Author: reddy $
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

#include "tsmInternal.h"

/*
 * Synopsis:
 *	
 *
 * Description:
 *
 *
 * Externals:
 *
 *
 * Returns:
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *
 */
void * 
tsmMemManagerMalloc(size_t size)
{
 static char * funcName = "tsmMemManagerMalloc()";
 void * buffer;

    buffer = malloc(size);
    if(!buffer) {
      tsmError (funcName, "Error in malloc.");
      return NULL;
    }
    return buffer;
}

/*
 * Synopsis:
 *	
 *
 * Description:
 *
 *
 * Externals:
 *
 *
 * Returns:
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *
 */
void * 
tsmMemManagerRealloc(void * ptr, 
		     size_t size)
{
  static char * funcName = "tsmMemManagerMalloc()";
  void * buffer;

  buffer = realloc(ptr, size);
  
  if(!buffer && size) {
    tsmError (funcName, "Error in realloc'ing %d bytes.", size);
    return (void *) -1;
  }
  return buffer;
}

/*
 * Synopsis:
 *	
 *
 * Description:
 *
 *
 * Externals:
 *
 *
 * Returns:
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *
 */
void * 
tsmMemManagerCalloc(size_t nelem,
		    size_t elsize)
{
 static char * funcName = "tsmMemManagerCalloc()";
 void * buffer;

    buffer = calloc(nelem, elsize);
    if(!buffer) {
      tsmError (funcName, "Error in callocing %d items of size %d.", nelem, elsize);
      return NULL;
    }
    return buffer;
}

/*
 * Synopsis:
 *	
 *
 * Description:
 *
 *
 * Externals:
 *
 *
 * Returns:
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *
 */
int
tsmMemManagerFree(void const * ptr)
{
  /*static const char funcName[] = "tsmMemManagerFree()";*/

  free((void *) ptr);
  return TRUE;
}

/*
 * Synopsis:
 *	
 *
 * Description:
 *
 *
 * Externals:
 *
 *
 * Returns:
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *
 */
char *
tsmMemManagerStrdup(const char * s1)
{
 static char * funcName = "tsmMemManagerStrdup()";
 char * s2;
 int len;

    if(!s1) {
      tsmError (funcName, "NULL passed to strdup, idiot."); 
      return NULL;
    }
    len = strlen(s1) + 1;
    s2 = tsmMemManagerMalloc(len);
    if(!s2) {
      tsmError (funcName, "Unable to dup string of len %d: %s", strlen(s1), s1);
      return NULL;
    }
    memcpy (s2, s1, len);
    return s2;
}
