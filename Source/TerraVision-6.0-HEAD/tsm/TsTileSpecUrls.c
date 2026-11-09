/*
 * Name: TsTileSpecUrls.c
 *
 * Description: 
 *
 * Function List:
 *
 * Revision History:
 *
 *   $Id: TsTileSpecUrls.c,v 1.12 2000/08/28 22:15:13 reddy Exp $
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

static const char* pyramidUrlsFileName = "PyramidUrls";

void
tspFreeNameUrlPairs (NameUrlPairs *nameUrlPairs)
{
  tsmMemManagerFree (nameUrlPairs->names.val);
  tsmMemManagerFree (nameUrlPairs->urls.val);
  tsmMemManagerFree (nameUrlPairs);
}

int
tspReadNameUrlPairs (const char *pathName, const char *fileName, NameUrlPairs *nameUrlPairs)
{
  static char * funcName = "tspReadNameUrlPairs()";
  FILE *fp;
  char buffer1[1024], buffer2[1024];
  u_int i;
  int numItems;

  sprintf (buffer1,"%s/%s", pathName, fileName);
  fp = fopen (buffer1,"rb");
  if (!fp) {
    tspError (buffer1, "While reading name/url pairs: couldn't open file!");
    return -1;
  }
  
  numItems = fscanf (fp, "%d\n", &nameUrlPairs->names.len);
  nameUrlPairs->urls.len = nameUrlPairs->names.len;
  if (numItems != 1)
    tspErrorAndClose (pathName, "While reading name/url pairs: invalid number of pairs!", fp);
  
  nameUrlPairs->names.val = (char **)
     tsmMemManagerCalloc (nameUrlPairs->names.len, sizeof (char *));
  if(!nameUrlPairs->names.val) {
    tsmError (funcName, "Unable to allocate space for nameUrlPairs->names.val");
    return -1;
  }
  nameUrlPairs->urls.val = (char **)
     tsmMemManagerCalloc (nameUrlPairs->names.len, sizeof (char *));
  if(!nameUrlPairs->urls.val) {
    tsmError (funcName, "Unable to allocate space for nameUrlPairs->urls.val");
    return -1;
  }

  for (i = 0; i < nameUrlPairs->names.len; ++i) {
    numItems = fscanf (fp, "\"%[^\"]\" \"%[^\"]\"\n", buffer1, buffer2);
    nameUrlPairs->names.val[i] = (char *)
       tsmMemManagerCalloc (strlen(buffer1)+1, sizeof(char));
    if(!nameUrlPairs->names.val[i]) {
      tsmError (funcName, "Unable to allocate space for nameUrlPairs->names.val[%d]", i);
      return -1;
    }
    nameUrlPairs->urls.val[i] = (char *)
       tsmMemManagerCalloc (strlen(buffer2)+1, sizeof(char));
    if(!nameUrlPairs->urls.val[i]) {
      tsmError (funcName, "Unable to allocate space for nameUrlPairs->urls.val[%d]", i);
      return -1;
    }

    strcpy (nameUrlPairs->names.val[i], buffer1);
    strcpy (nameUrlPairs->urls.val[i], buffer2);
    if (numItems != 2)
      tspErrorAndClose (pathName, "While reading name/url pairs: invalid name or url!", fp);
  }
  return TRUE;
}

int
tspReadPyramidUrls (const char *pathName, NameUrlPairs *pyramidUrls)
{
  return (tspReadNameUrlPairs (pathName, pyramidUrlsFileName, pyramidUrls));
}

int
tspMatchName (const char *name, NameUrlPairs *nameUrlPairs)
{
  return tspStringIndexFromSet (name, 0, nameUrlPairs->names.len-1,
				(const char **) nameUrlPairs->names.val, -1);
}

char *
tspNameToUrl (const char *name, NameUrlPairs *nameUrlPairs)
{
  return nameUrlPairs->urls.val[tspMatchName (name, nameUrlPairs)];
}

int
tspUrlToPathName (const char *url, char *pathName)
{
  char machineBuffer[1024];
  int numItems;

  numItems = sscanf (url, "http://%[^/]%s", machineBuffer, pathName);
  if (numItems == 2)
    return TRUE;
  
  numItems = sscanf (url, "file://%[^/]%s", machineBuffer, pathName);
  if (numItems == 2)
    return TRUE;
  
  numItems = sscanf (url, "ftp://%[^/]%s", machineBuffer, pathName);
  if (numItems == 2)
    return TRUE;
  else
    return FALSE;
}

void
tspSaveNameUrlPairs (const char *pathName, const char *fileName, NameUrlPairs *nameUrlPairs)
{
  u_int i;
  FILE *fp;
  fp = tspOpenForWrite (pathName, fileName);

  fprintf (fp, "%d\n", nameUrlPairs->names.len);
  for (i = 0; i < nameUrlPairs->names.len; ++i) {
    fprintf (fp, "\"%s\" \"%s\"\n",
	     nameUrlPairs->names.val[i], nameUrlPairs->urls.val[i]);
  }
  tspCloseForWrite (fp);
}

void
tspSavePyramidUrls (const char *pathName, NameUrlPairs *pyramidUrls)
{
  tspSaveNameUrlPairs (pathName, pyramidUrlsFileName, pyramidUrls);
}

