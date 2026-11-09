/*
  tsmDatabase.c
 
  Implements Tcl commands for accessing and manipulating tsm Databases.
 

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
 
  Contributor(s): Rick Brownrigg, Martin Reddy, Lee Iverson, Yvan Leclerc.
  ----- END LICENSE BLOCK -----

*/

#include <stdio.h>
#include <time.h>
#include <tcl.h>

#include <tsm/tsm.h>
#include <TerraVision/Trace.h>

/* module globals... */
static TsmDatabase* theDatabase = NULL;    /* handle to the active database */


/* forward decls... */
int newCommand(Tcl_Interp*, int objc, Tcl_Obj *const objv[]);
int openCommand(Tcl_Interp*, int objc, Tcl_Obj *const objv[]);
int writeCommand(Tcl_Interp*, int objc, Tcl_Obj *const objv[]);
int geosetCommand(Tcl_Interp*, int objc, Tcl_Obj *const objv[]);
int serverCommand(Tcl_Interp*, int objc, Tcl_Obj *const objv[]);

static int findServer(Tcl_Obj*);

/*
 * t s m D a t a b a s e C o m m a n d ( )
 *
 * Main entry point for processing Tcl "tsmDatabase" commands.
 *
 */
int
tsmDatabaseCommand(ClientData clientData, Tcl_Interp *interp, int objc,
    Tcl_Obj *const objv[])
{
    char* cmd;

    if (objc < 2) return TCL_ERROR;

    cmd = Tcl_GetStringFromObj(objv[1], NULL);

    if (!strcmp(cmd, "open")) {
        return openCommand(interp, objc, objv);
    }

    else if (!strcmp(cmd, "write")) {
        return writeCommand(interp, objc, objv);
    }

    else if (!strcmp(cmd, "geoset")) {
        return geosetCommand(interp, objc, objv);
    }

    else if (!strcmp(cmd, "server")) {
        return serverCommand(interp, objc, objv);
    }
    
    else if (!strcmp(cmd, "new")) {
        return newCommand(interp, objc, objv);
    }

    else if (!strcmp(cmd, "exist")) {
      Tcl_SetObjResult(interp, Tcl_NewIntObj( (theDatabase != NULL) ) );
      return TCL_OK;

    }

    return TCL_ERROR;
}

/*
 * o p e n C o m m a n d ( )
 *
 * Process an "open" subcommand.
 *
 */
int
openCommand(Tcl_Interp* interp, int objc, Tcl_Obj *const objv[])
{
    const char *funcName = "openCommand";
    char       *databasePath;

    if (objc != 3) {
        Tcl_Obj* errReturn = Tcl_NewStringObj("usage is: ", -1);
        Tcl_AppendToObj(errReturn, Tcl_GetStringFromObj(objv[0], NULL), -1);
        Tcl_AppendToObj(errReturn, " ", -1);
        Tcl_AppendToObj(errReturn, Tcl_GetStringFromObj(objv[1], NULL), -1);
        Tcl_AppendToObj(errReturn, " <databasePathname>", -1);
        Tcl_SetObjResult(interp, errReturn);
        return TCL_ERROR;
    }

    /* extract the pathname to the database... */
    databasePath = Tcl_GetStringFromObj(objv[2], NULL);
    TSMTRACE(( INIT_MASK|1, funcName, "Database is: %s", databasePath ));

    if (theDatabase)
        /* free previous instance... */
        tsmFreeDatabase(theDatabase);

    theDatabase = tsmReadDatabase(databasePath);
    if (theDatabase == NULL) {
        Tcl_Obj* errReturn = Tcl_NewStringObj("Failed to open/read database: ", -1);
        Tcl_AppendToObj(errReturn, databasePath, -1);
        Tcl_SetObjResult(interp, errReturn);
        return TCL_ERROR;
    }

    return TCL_OK;
}

/*
 * n e w C o m m a n d ( )
 *
 * Process an "new" subcommand.
 *
 */
int
newCommand(Tcl_Interp* interp, int objc, Tcl_Obj *const objv[])
{
    char* databasePath;

    if (objc != 3) {
        Tcl_Obj* errReturn = Tcl_NewStringObj("usage is: ", -1);
        Tcl_AppendToObj(errReturn, Tcl_GetStringFromObj(objv[0], NULL), -1);
        Tcl_AppendToObj(errReturn, " ", -1);
        Tcl_AppendToObj(errReturn, Tcl_GetStringFromObj(objv[1], NULL), -1);
        Tcl_AppendToObj(errReturn, " <databasePathname>", -1);
        Tcl_SetObjResult(interp, errReturn);
        return TCL_ERROR;
    }

    /* extract the pathname to the database... */
    databasePath = Tcl_GetStringFromObj(objv[2], NULL);

    if (theDatabase) 
        /* free previous instance... */
        tsmFreeDatabase(theDatabase);

    theDatabase = tsmMakeDatabase();
    theDatabase->location = tsmParseUrl(databasePath);

    if (tsmWriteDatabase(theDatabase, NULL) == FALSE) {
        Tcl_Obj* errReturn = Tcl_NewStringObj("Could not create new database\nCheck that filename is valid.", -1);
        Tcl_SetObjResult(interp, errReturn);
        return TCL_ERROR;
    }
        
    return TCL_OK;
}


/*
 * w r i t e C o m m a n d ( )
 *
 * Writes the current database to persistent store.
 *
 */
int
writeCommand(Tcl_Interp* interp, int objc, Tcl_Obj *const objv[])
{
    char* databasePath = NULL;

    if (!theDatabase) {
        Tcl_Obj* errReturn = 
            Tcl_NewStringObj("there is no open database to save", -1);
        Tcl_SetObjResult(interp, errReturn);
        return TCL_ERROR;
    }

    if (objc == 3) {
        databasePath = Tcl_GetStringFromObj(objv[2], NULL);
    }

    if (tsmWriteDatabase(theDatabase, databasePath) == FALSE) {
        Tcl_Obj* errReturn = 
            Tcl_NewStringObj("An error occurred while saving the database file", -1);
        Tcl_SetObjResult(interp, errReturn);
        return TCL_ERROR;
    }
    
    return TCL_OK;
}


/*
 * g e o s e t C o m m a n d ( )
 *
 * Process a "geoset" subcommand.
 *
 */
int
geosetCommand(Tcl_Interp* interp, int objc, Tcl_Obj *const objv[])
{
    Tcl_Obj* returnList;
    Tcl_Obj* elemObj;
    int i;
    char* subCmd;

    if (objc < 3) return TCL_ERROR;

    if ( ! theDatabase) {
      returnList = Tcl_NewObj();
      Tcl_SetObjResult(interp, returnList);
      return TCL_OK;
    }

    subCmd = Tcl_GetStringFromObj(objv[2], NULL);
    if (!strcmp(subCmd, "list")) {
        /* make the list object... */
        returnList = Tcl_NewObj();

        /* populate it... */
        for (i=0; i < theDatabase->numGeoSets; i++) {
            elemObj = Tcl_NewStringObj(theDatabase->geoset[i].name, -1);
            Tcl_ListObjAppendElement(interp, returnList, elemObj);
        }

        /* set return value... */
        Tcl_SetObjResult(interp, returnList);
        return TCL_OK;
    }

    else if (!strcmp(subCmd, "getDatasets")) {
        TsmDataSet* tsmDataset;
        char* geoset;
        Tcl_Obj* imageList;
        Tcl_Obj* demList;
        int geosetIndex = -1;

        if (objc != 4) return TCL_ERROR;

        /* find the geoset in the database record... */
        geoset = Tcl_GetStringFromObj(objv[3], NULL);
        for (i=0; i<theDatabase->numGeoSets; i++) {
            if (!strcmp(theDatabase->geoset[i].name, geoset)) {
                geosetIndex = i;
                break;
            }
        }
        if (geosetIndex < 0) return TCL_ERROR; /* NEED TO REPORT THIS FOR CATCH */

        /* find the ortho-image dataset, and all its "instances"... */
        tsmDataset = 
            tsmGetDataSet(theDatabase, theDatabase->geoset[geosetIndex].oi_name);
        if (!tsmDataset) return TCL_ERROR; /* NEED BETTER REPORTING HERE! */
        
        /* make a list of the instances... */
        imageList = Tcl_NewObj();
        for (i=0; i < tsmDataset->numInstances; i++) {
            elemObj = 
                Tcl_NewStringObj(tsmDataset->instance[i].location->url, -1);
            Tcl_ListObjAppendElement(interp, imageList, elemObj);
        }

        /* find the dem dataset, and all its "instances"... */
        tsmDataset = 
            tsmGetDataSet(theDatabase, theDatabase->geoset[geosetIndex].dem_name);
        if (!tsmDataset) return TCL_ERROR; /* NEED BETTER REPORTING HERE! */
        
        /* want a list of the instances... */
        demList = Tcl_NewObj();
        for (i=0; i < tsmDataset->numInstances; i++) {
            elemObj = 
                Tcl_NewStringObj(tsmDataset->instance[i].location->url, -1);
            Tcl_ListObjAppendElement(interp, demList, elemObj);
        }

        /* set return value... */
        returnList = Tcl_NewObj();
        Tcl_ListObjAppendElement(interp, returnList, imageList);
        Tcl_ListObjAppendElement(interp, returnList, demList);
        Tcl_SetObjResult(interp, returnList);
        return TCL_OK;
    }        

    return TCL_ERROR;
}


/*
 * s e r v e r C o m m a n d ( )
 *
 * Process a "server" subcommand.
 *
 */
int
serverCommand(Tcl_Interp* interp, int objc, Tcl_Obj *const objv[])
{
    const char *funcName = "serverCommand";
    Tcl_Obj* returnList;
    Tcl_Obj* elemObj;
    int i;
    char* subCmd;

    if (objc < 3) return TCL_ERROR;

    /* if no database, then return an error */

    if (!theDatabase) {
        Tcl_Obj* errReturn = 
            Tcl_NewStringObj("there is no database open", -1);
        Tcl_SetObjResult(interp, errReturn);
        return TCL_ERROR;
    }

    subCmd = Tcl_GetStringFromObj(objv[2], NULL);
    
    /*
     * l i s t 
     */
    if (!strcmp(subCmd, "list")) {
        /* make the list object... */
        returnList = Tcl_NewObj();

        /* populate it... */
        for (i=0; i < theDatabase->numServers; i++) {
	  char *string = theDatabase->server[i].location->url;
	  elemObj = Tcl_NewStringObj(string, -1);
	  Tcl_ListObjAppendElement(interp, returnList, elemObj);
        }

        /* set return value... */
        Tcl_SetObjResult(interp, returnList);
        return TCL_OK;
    }

    /*
     * i n f o 
     */
    if (!strcmp(subCmd, "info")) {
        Tcl_Obj* retStr;
        int serverIndex;
        char buff[100];
        time_t contactTime;

        if (objc != 4) return TCL_ERROR;

        serverIndex = findServer(objv[3]);
        if (serverIndex < 0) return TCL_ERROR;
        
        sprintf(buff, "%s", 
            tsmDateString(theDatabase->server[serverIndex].last_contacted));

        retStr = Tcl_NewObj();
        Tcl_SetStringObj(retStr, buff, -1);
        Tcl_SetObjResult(interp, retStr);
        return TCL_OK;
    }

    /*
     * u p 
     */
    if (!strcmp(subCmd, "up")) {
        int serverIndex;
        TsmServer tmp;

        if (objc != 4) return TCL_ERROR;

        serverIndex = findServer(objv[3]);
        if (serverIndex < 0) return TCL_ERROR;
        if (serverIndex == 0) return TCL_OK;

        tmp = theDatabase->server[serverIndex];
        theDatabase->server[serverIndex] = theDatabase->server[serverIndex-1];
        theDatabase->server[serverIndex-1] = tmp;

        return TCL_OK;
    }

    /*
     * d o w n 
     */
    if (!strcmp(subCmd, "down")) {
        int serverIndex;
        TsmServer tmp;

        if (objc != 4) return TCL_ERROR;

        serverIndex = findServer(objv[3]);
        if (serverIndex < 0) return TCL_ERROR;
        if (serverIndex == (theDatabase->numServers-1)) return TCL_OK;

        tmp = theDatabase->server[serverIndex];
        theDatabase->server[serverIndex] = theDatabase->server[serverIndex+1];
        theDatabase->server[serverIndex+1] = tmp;

        return TCL_OK;
    }

    /*
     * a d d 
     */
    if (!strcmp(subCmd, "add")) {
        char* urlStr;
        Purl* purl;
        Tcl_Obj* retVal;

        if (objc != 4) return TCL_ERROR;

        /* check that the argument is a valid Url... */
        urlStr = Tcl_GetStringFromObj(objv[3], NULL);
        if ( !(purl = tsmParseUrl(urlStr))) {
            Tcl_Obj* errReturn = Tcl_NewStringObj("Not a valid URL!", -1);
            Tcl_SetObjResult(interp, errReturn);
            return TCL_ERROR;
        }

        if (urlStr[0] != '/' && urlStr[0] != '~' && !strchr(urlStr,':' )){
            Tcl_Obj* errReturn = 
                Tcl_NewStringObj("Relative URLs are not allowed - make it absolute", -1);
            Tcl_SetObjResult(interp, errReturn);
            tsmFreePurl(purl);
            return TCL_ERROR;
        }

        /* add the new server to the database... */
        if (!tsmAddServerToDatabase(theDatabase, urlStr)) {
            Tcl_Obj* errReturn = 
                Tcl_NewStringObj("tsm-library failed to add server", -1);
            Tcl_SetObjResult(interp, errReturn);
            tsmFreePurl(purl);
            return TCL_ERROR;
        }

        retVal = Tcl_NewStringObj(purl->url, -1);
        Tcl_SetObjResult(interp, retVal);
        tsmFreePurl(purl);
        return TCL_OK;
    }

    /*
     * r e m o v e 
     */
    if (!strcmp(subCmd, "remove")) {
        int serverIndex;

        if (objc != 4) return TCL_ERROR;
        
        serverIndex = findServer(objv[3]);
        if (serverIndex < 0) return TCL_ERROR;

        /* remove the geosets associated with the server, and then remove the
         * server itself.
         */
        tsmRemoveServerSetsFromDatabase(theDatabase, 
            &theDatabase->server[serverIndex], NULL);
        tsmRemoveServerFromDatabase(theDatabase, 
            theDatabase->server[serverIndex].location->url);
        
        /* save these changes back out to the persistent database store... */
        if (!tsmWriteDatabase(theDatabase, NULL)) {
            fprintf(stderr, "UNABLE TO WRITE CHANGES TO DATABASE FILE!n");
            return TCL_ERROR;
        }

        return TCL_OK;
    }

    /*
     * r e a d 
     */
    if (!strcmp(subCmd, "read")) {
        int serverIndex;

        if (objc != 4) return TCL_ERROR;

	TSMTRACE(( TSMMASK|1, funcName, "Reading server %s", objv[3] ));
        serverIndex = findServer(objv[3]);
        
        if (!tsmUpdateDatabaseFromServerCB(theDatabase, 
            &theDatabase->server[serverIndex], NULL, NULL, NULL))
        {
            tvMsgListAdd( "ERROR", NULL, "Error contacting server - may be down", NULL );
        }

        return TCL_OK;
    }

    return TCL_ERROR;
}


/*
 * f i n d S e r v e r ( )
 *
 * Utility function to locate a server in the global database's list of server.
 * Takes a server represented as a Tcl_Obj* argument.  Returns the index into
 * the list.  Returns -1 is not found.
 *
 */
int
findServer(Tcl_Obj* serverObj)
{
    int i;
    char* serverName;
    int serverIndex = -1;

    serverName = Tcl_GetStringFromObj(serverObj, NULL);
    for (i=0; i < theDatabase->numServers; i++) {
        if (!strcmp(serverName, theDatabase->server[i].location->url)) {
            serverIndex = i;
            break;
        }
    }

    return serverIndex;
}

/*
 * g e t D B a s e ( )
 *
 * Returns the database structure managed by this module
 *
 */

TsmDatabase *
getDBase( void )
{
  return theDatabase;
}
