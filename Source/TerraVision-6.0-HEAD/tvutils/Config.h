/*
  Filename: Config.h

  Description: Contains routines for managing the configuration of the
  TerraVision system, including locating and parsing the config file.

  ----- BEGIN LICENSE BLOCK -----
  Version: MPL 1.1
 
  The contents of this file are subject to the Mozilla Public License Version
  1.1 (the "License"); you may not use this file except in compliance with
  the License. You may obtain a copy of the License at
  http://www.mozilla.org/MPL/
 
  Software distributed under the License is distributed on an "AS IS" basis,
  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
  for the specific language governing rights and limitations under the
  License.
 
  The Original Code is the TerraVision 3D Terrain Visualization System.
 
  The Initial Developer of the Original Code is SRI International.

  Portions created by the Initial Developer are Copyright (C) 1992-2002
  the Initial Developer. All Rights Reserved.
 
  Contributor(s): Yvan Leclerc, Lee Iverson, Martin Reddy, Nat Bletter. 
  ----- END LICENSE BLOCK -----

*/

#ifndef TVU_CONFIG_H
#define TVU_CONFIG_H

/* return the directory that TerraVision is installed in */
char *tvuGetInstallDir( void );

/* return the directory that the user keeps config files in */
char *tvuGetUserDir( void );

/* return the full filename to the TerraVision config file */
char *tvuGetConfigFilename( void );

/* return the full filename to the TerraVision local database file */
char *tvuGetDatabaseFilename( void );

/* return the full filename to the TerraVision viewpoint bookmark file */
char *tvuGetViewpointsFilename( void );

/* return the default directory for storing/loading path files */
char *tvuGetPathUrl( char *filename );

/* return the location of the TerraVision.tcl file */
char *tvuGetTclFilename( void );

/* return the location of the .geo schema file */
char *tvuGetSchemaUrl( void );

/* return the location of the .geo profile file */
char *tvuGetProfileUrl( void );

/* frees all of the memory for the config.xml file */
void tvuFreeConfig( void );

/* do a case-independent string compare, return TRUE or FALSE */
int tvuStrCmp( char const *str1, char const *str2 );
int tvuSubStr( char const *str1, char const *str2 );


#endif
