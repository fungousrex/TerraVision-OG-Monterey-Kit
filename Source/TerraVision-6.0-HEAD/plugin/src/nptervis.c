/*
 * Filename: nptervis.c
 *
 * Purpose:
 *   This is the primary file that implements the TerraVision
 *   Netscape plugin. This plugin uses LiveConnect to enable
 *   it to be controlled by Java and JavaScript in an HTML
 *   page, and also to enable it to talk to Java.
 *          
 *   This version of the TerraVision plugin uses the "old"
 *   Netscape API. This should work on all Netscape 3.x and 
 *   4.x browsers. The Netscape Plugin SDK can be found from:
 *
 *     home.netscape.com/comprod/development_partners/plugin_api/
 *
 *   Some useful URLs for developing Netscape plugins include:
 *
 *     devedge.netscape.com/support/faqs/plugins/
 *     developer.netscape.com/docs/manuals/communicator/plugin/contents.html
 *     home.netscape.com/eng/mozilla/3.0/handbook/plugins/pjava.htm
 *
 * Author:
 *   Martin Reddy, SRI International.
 *
 * Revision:
 *   $Id: nptervis.c,v 1.9 2000/08/07 22:11:08 reddy Exp $
 *
 * (c) Copyright 1993-1999. SRI International. All rights reserved.
 *
 */

#include "nptervis_all.h"

/*
 * Synopsis:
 *   NPError NPP_Initialize( void )
 *
 * Doc++ Description: *//** 
 *   Provides global initialization for a plug-in. 
 *
 *   This function is called once when a plug-in is loaded, before the
 *   first instance is created. You should allocate any memory or
 *   resources shared by all instances of your plug-in at this
 *   time. After the last instance has been deleted, NPP_Shutdown will
 *   be called, where you can release any memory or resources
 *   allocated by NPP_Initialize.
 *   
 * @return   NPERR_NO_ERROR if everything went well
 *
 * @author   Martin Reddy
 * @version  27 October 1999
 *
 * @see      NPP_Shutdown
 *
 */

NPError
NPP_Initialize( void )
{
  return NPERR_NO_ERROR;
}

/*
 * Synopsis:
 *   jref NPP_GetJavaClass( void )
 *
 * Doc++ Description: *//** 
 *   Return the Java class associated with the plugin.
 *
 *   NPP_GetJavaClass is called during initialization to ask your
 *   plugin what its associated Java class is. If you don't have one,
 *   just return NULL. Otherwise, use the javah-generated "use_"
 *   function to both initialize your class and return it. If you
 *   can't find your class, an error will be signalled by "use_" and
 *   will cause the Navigator to complain to the user.
 *
 *   The Java class is used to enable communication between the 
 *   plugin and Java. Also, through LiveConnect, it enables JavaScript
 *   to talk to the plugin.
 *
 * @return   Object pointer to the Java class, or NULL.
 *
 * @author   Martin Reddy
 * @version  27 October 1999
 *
 */

jref
NPP_GetJavaClass( void )
{
  struct java_lang_Class *myClass;
  JRIEnv                 *env;

  /* check to see if Java is enabled first */

  if ( ( env = NPN_GetJavaEnv() ) == NULL)
    return NULL;

  /* initialize and get the object pointer for our Java class */
  
  myClass = use_nptervis(env);
  use_netscape_plugin_Plugin( env );
  
  return (jref) myClass;
}

/*
 * Synopsis:
 *   void NPP_Shutdown( void )
 *
 * Doc++ Description: *//** 
 *   Provides global deinitialization for a plug-in. 
 * 
 *   This function is called once after the last instance of your
 *   plug-in is destroyed. Use this function to release any memory or
 *   resources shared across all instances of your plug-in. You should
 *   be a good citizen and declare that you're not using your java
 *   class any more. This allows java to unload it, freeing up memory.
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  27 October 1999
 *
 * @see      NPP_Initialize
 *
 */

void
NPP_Shutdown( void )
{
  JRIEnv *env = NPN_GetJavaEnv();

  if (env) {
    unuse_nptervis(env);
    unuse_netscape_plugin_Plugin( env );
  }
}

/*
 * Synopsis:
 *   NPError NPP_New( NPMIMEType pluginType, NPP instance, uint16 mode,
 *                    int16 argc, char *argn[], char *argv[],
 *                    NPSavedData *saved )
 *
 * Doc++ Description: *//** 
 *   Creates a new instance of a plug-in and returns an error value. 
 * 
 *   NPP_New creates a new instance of your plug-in with MIME type
 *   specified by pluginType. The parameter mode is NP_EMBED if the
 *   instance was created by an EMBED tag, or NP_FULL if the instance
 *   was created by a separate file.  You can allocate any
 *   instance-specific private data in instance->pdata at this
 *   time. The NPP pointer is valid until the instance is destroyed.
 *   
 *
 * @param    pluginType pointer to the MIME type for the new instance
 * @param    instance Netscape's instance-specific data structure
 * @param    mode NP_EMBED (if created by EMBED tag) or NP_FULL
 * @param    argc number of HTML arguments passed from EMBED tag
 * @param    argn array of attribute names passed from EMBED tag
 * @param    argv array of attribute values passed from EMBED tag
 * @param    saved pointer to data saved by NPP_Destroy
 *
 * @return   NPERR_NO_ERROR if all went fine.
 *
 * @author   Martin Reddy
 * @version  27 October 1999
 *
 * @see      NPP_Destroy
 *
 */

NPError 
NPP_New( NPMIMEType pluginType, NPP instance, uint16 mode, int16 argc,
	 char *argn[], char *argv[], NPSavedData *saved )
{
  PluginInstance *plugin;
  int            i;
  
  if ( instance == NULL )
    return NPERR_INVALID_INSTANCE_ERROR;
  
  /* allocate our plugin instance structure to hold all our state */
  
  instance->pdata = NPN_MemAlloc( sizeof(PluginInstance) );
  if ( ( plugin = (PluginInstance *) instance->pdata ) == NULL )
    return NPERR_OUT_OF_MEMORY_ERROR;
  
  /* initialise the fields of our plugin instance (we could use */
  /* the NPSavedData we saved from NPP_Destroy if non-NULL).    */
  
  plugin->fWindow    = NULL;
  plugin->winHandle  = NULL;
  plugin->fMode      = mode;
  plugin->fNPP       = instance;
  plugin->debug_mode = TRUE;
  plugin->pipe0_in   = NULL;
  plugin->pipe1_out  = NULL;
  plugin->argv       = (char **) calloc( (argc + 1) * 2, sizeof(char *) );
  plugin->argc       = 0;

  /* do any platform specific initialization of the plugin structure */

  PlatformNew( plugin );

  /* create a TerraVision command line array from the EMBED arguments */

  for ( i = 0; i < argc; i++ ) {
    char optname[128];

    /* don't include some of the standard EMBED parameters */

    if ( strcasecmp( argn[i], "type" ) == 0 ||
	 strcasecmp( argn[i], "name" ) == 0 ||
	 strcasecmp( argn[i], "width" ) == 0 ||
	 strcasecmp( argn[i], "height" ) == 0 )
      continue;

    sprintf( optname, "-%s", argn[i] );
    plugin->argv[plugin->argc++] = strdup( optname );
    plugin->argv[plugin->argc++] = strdup( argv[i] );
  }

  /* write an initial message out to the Java Console */
    
  if ( NPN_GetJavaEnv() )
    DebugMessage( plugin, "Hello from the TerraVision plugin!" );
  
  return NPERR_NO_ERROR;
}

/*
 * Synopsis:
 *   NPError NPP_Destroy( NPP instance, NPSavedData **save )
 *
 * Doc++ Description: *//** 
 *   Deletes a specific instance of a plug-in. 
 * 
 *   NPP_Destroy is called when a plug-in instance is deleted,
 *   typically because the user has left the page containing the
 *   instance, closed the window, or quit the application. You should
 *   delete any private instance-specific information stored in
 *   instance->pdata. If the instance being deleted is the last
 *   instance created by your plug-in, NPP_Shutdown will subsequently
 *   be called, where you can delete any data allocated in
 *   NPP_Initialize to be shared by all your plug-in's instances. Note
 *   that you should not perform any graphics operations in
 *   NPP_Destroy as the instance's window is no longer guaranteed to
 *   be valid.
 *
 * @param    instance Netscape's instance-specific data structure
 * @param    save pointer to give to NPP_New if we resurrect this instance
 *
 * @return   NPERR_NO_ERROR if all went well.
 *
 * @author   Martin Reddy
 * @version  27 October 1999
 *
 * @see      NPP_New
 *
 */

NPError 
NPP_Destroy( NPP instance, NPSavedData **save )
{
  PluginInstance *plugin;
  int            i;
  
  if (instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;
  
  if ( ( plugin = (PluginInstance*) instance->pdata ) ) {

    /* tell the external TerraVision application to go away */

    DebugMessage( plugin, "Tell TV to quit" );
    TalkToTerraVision( plugin, "quit" );
    DebugMessage( plugin, "Told TV to quit" );

    /* free up the memory used by the plugin arguments */

    if ( plugin->argv ) {
      for ( i = 0; i < plugin->argc; i++ ) 
	free( plugin->argv[i] );
      free( plugin->argv );
    }

    /* free up the platform specific resources */

    PlatformDestroy( plugin );
    
    /* If desired, we can call NP_MemAlloc to create a NPSavedDate */
    /* structure containing any state information that we want to  */
    /* be restored if this plugin instance is later recreated.     */
    
    NPN_MemFree( plugin );
    instance->pdata = NULL;
  }

  return NPERR_NO_ERROR;
}

int
FileExists( char const *filename )
{
  FILE *handle;

  /* check to see if the file exists */

  if ( ( handle = fopen( filename, "r" ) ) == NULL )
    return FALSE;

  fclose( handle );
  return TRUE;
}

/*
 * Synopsis:
 *   NPError NPP_SetWindow( NPP instance, NPWindow *window )
 *
 * Doc++ Description: *//** 
 *   Sets the window in which a plug-in draws. 
 * 
 *   NPP_SetWindow informs the plug-in instance specified by instance
 *   of the the window denoted by window in which the instance
 *   draws. This NPWindow pointer is valid for the life of the
 *   instance, or until NPP_SetWindow is called again with a different
 *   value. Subsequent calls to NPP_SetWindow for a given instance
 *   typically indicate that the window has been resized.  If either
 *   window or window->window are NULL, the plug-in must not perform
 *   any additional graphics operations on the window and should free
 *   any resources associated with the window.
 *
 * @param    instance Netscape's instance-specific data structure
 * @param    window a generic Netscape window pointer
 *
 * @return   NPERR_NO_ERROR if all went well.
 *
 * @author   Martin Reddy
 * @version  27 October 1999
 *
 */

NPError 
NPP_SetWindow( NPP instance, NPWindow *window )
{
  NPError result = NPERR_NO_ERROR;
  PluginInstance *plugin;
  char           *exename;

  if (instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;
  
  plugin = (PluginInstance *) instance->pdata;
  
  /* Before setting window to point to the new window, you may  */
  /* wish to compare the new window info to the previous window */
  /* (if any) to note window size changes, etc.                 */
  
  plugin->fWindow = window;

  /* Do we already have a window? If so, we need to clean it up */
  
  if ( plugin->winHandle != NULL ) {

    if ( ( window == NULL ) || ( window->window == NULL ) ) {

      /* There is now no window to use. get rid of the old one and exit. */
      
      DebugMessage( plugin, "SetWindow: no window to use anymore!" );

      plugin->winHandle = NULL;
      return NPERR_NO_ERROR;

    } else if ( plugin->winHandle == (void *) window->window ) {

      /* The new window is the same as the old one. Redraw and get out. */
      
      DebugMessage( plugin, "SetWindow: redraw!" );
      return NPERR_NO_ERROR;

    } else {

      /* Clean up the old window, so that we can subclass the new one later. */
      
      DebugMessage( plugin, "SetWindow: new window." );
    }

  } else if ( ( window == NULL ) || ( window->window == NULL ) ) {

    /* Nothing to do: no current window and no new window to use */

    DebugMessage( plugin, "SetWindow: do nothing" );
    return NPERR_NO_ERROR;
  }

  if ( ( exename = GetTerraVisionBinaryName() ) == NULL ) {
    DebugMessage( plugin, "Could not find an external TerraVision binary!" );
    return NPERR_INVALID_INSTANCE_ERROR;
  }

  DebugMessage( plugin, "Running external TerraVision application..." );
  DebugMessage( plugin, "%s", exename );

  plugin->winHandle = (void *) window->window;

  RunTerraVision( plugin, exename, (int) window->window, window->width,
		  window->height );

  PlatformEventLoop( plugin, window );

  DebugMessage( plugin, "Leaving NPP_SetWindow: %d", (int) result );
  return NPERR_NO_ERROR;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++
 * NPP_NewStream:
 * Notifies an instance of a new data stream and returns an error value. 
 * 
 * NPP_NewStream notifies the instance denoted by instance of the creation of
 * a new stream specifed by stream. The NPStream* pointer is valid until the
 * stream is destroyed. The MIME type of the stream is provided by the
 * parameter type. 
 +++++++++++++++++++++++++++++++++++++++++++++++++*/

NPError 
NPP_NewStream( NPP instance, NPMIMEType type, NPStream *stream, 
	       NPBool seekable, uint16 *stype )
{
  PluginInstance *plugin;
  
  if (instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;
  
  plugin = (PluginInstance*) instance->pdata;
  DebugMessage( plugin, "Calling NPP_NewStream" ); 
  
  return NPERR_NO_ERROR;
}


/* PLUGIN DEVELOPERS:
 *	These next 2 functions are directly relevant in a plug-in which
 *	handles the data in a streaming manner. If you want zero bytes
 *	because no buffer space is YET available, return 0. As long as
 *	the stream has not been written to the plugin, Navigator will
 *	continue trying to send bytes.  If the plugin doesn't want them,
 *	just return some large number from NPP_WriteReady(), and
 *	ignore them in NPP_Write().  For a NP_ASFILE stream, they are
 *	still called but can safely be ignored using this strategy.
 */

int32 STREAMBUFSIZE = 0X0FFFFFFF; /* If we are reading from a file in NPAsFile
				   * mode so we can take any size stream in our
				   * write call (since we ignore it) */

/*+++++++++++++++++++++++++++++++++++++++++++++++++
 * NPP_WriteReady:
 * Returns the maximum number of bytes that an instance is prepared to accept
 * from the stream. 
 * 
 * NPP_WriteReady determines the maximum number of bytes that the
 * instance will consume from the stream in a subsequent call NPP_Write. This
 * function allows Netscape to only send as much data to the instance as the
 * instance is capable of handling at a time, allowing more efficient use of
 * resources within both Netscape and the plug-in. 
 +++++++++++++++++++++++++++++++++++++++++++++++++*/

int32 
NPP_WriteReady( NPP instance, NPStream *stream )
{
  PluginInstance *plugin;
  
  if ( instance != NULL ) {
    plugin = (PluginInstance *) instance->pdata;
    DisplayJavaMessage( plugin, "Calling NPP_WriteReady." ); 
  }
  
  /* Number of bytes ready to accept in NPP_Write() */
  return STREAMBUFSIZE;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++
 * NPP_Write:
 * Delivers data from a stream and returns the number of bytes written. 
 * 
 * NPP_Write is called after a call to NPP_NewStream in which the
 * plug-in requested a normal-mode stream, in which the data in the
 * stream is delivered progressively over a series of calls to
 * NPP_WriteReady and NPP_Write. The function delivers a buffer buf of
 * len bytes of data from the stream identified by stream to the
 * instance. The parameter offset is the logical position of buf from
 * the beginning of the data in the stream.
 * 
 * The function returns the number of bytes written (consumed by the
 * instance).  A negative return value causes an error on the stream,
 * which will subsequently be destroyed via a call to
 * NPP_DestroyStream.
 * 
 * Note that a plug-in must consume at least as many bytes as it
 * indicated in the preceeding NPP_WriteReady call. All data consumed
 * must be either processed immediately or copied to memory allocated
 * by the plug-in: the buf parameter is not persistent.
 * +++++++++++++++++++++++++++++++++++++++++++++++++*/

int32 
NPP_Write( NPP instance, NPStream *stream, int32 offset, int32 len,
	   void *buffer )
{
  if ( instance ) {
    PluginInstance *plugin = (PluginInstance*) instance->pdata;
    DisplayJavaMessage( plugin, (char const*) buffer ); 
  }
  
  return len;		/* The number of bytes accepted */
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++
 * NPP_DestroyStream:
 * Indicates the closure and deletion of a stream, and returns an error value. 
 * 
 * The NPP_DestroyStream function is called when the stream identified
 * by stream for the plug-in instance denoted by instance will be
 * destroyed. You should delete any private data allocated in
 * stream->pdata at this time.
 * +++++++++++++++++++++++++++++++++++++++++++++++++*/

NPError 
NPP_DestroyStream( NPP instance, NPStream *stream, NPError reason )
{
  PluginInstance *plugin;
  
  if ( instance == NULL )
    return NPERR_INVALID_INSTANCE_ERROR;
  
  plugin = (PluginInstance *) instance->pdata;
  DisplayJavaMessage( plugin, "Calling NPP_DestroyStream." ); 
  
  return NPERR_NO_ERROR;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++
 * NPP_StreamAsFile:
 * Provides a local file name for the data from a stream. 
 * 
 * NPP_StreamAsFile provides the instance with a full path to a local
 * file, identified by fname, for the stream specified by
 * stream. NPP_StreamAsFile is called as a result of the plug-in
 * requesting mode NP_ASFILEONLY or NP_ASFILE in a previous call to
 * NPP_NewStream. If an error occurs while retrieving the data or
 * writing the file, fname may be NULL.
 * +++++++++++++++++++++++++++++++++++++++++++++++++*/

void 
NPP_StreamAsFile( NPP instance, NPStream *stream, const char *fname )
{
  PluginInstance *plugin;
  
  if ( ! instance ) return;
  
  plugin = (PluginInstance *) instance->pdata;
  DisplayJavaMessage( plugin, "Calling NPP_StreamAsFile." ); 
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++
 * NPP_Print:
 +++++++++++++++++++++++++++++++++++++++++++++++++*/

void 
NPP_Print( NPP instance, NPPrint *printInfo )
{
  
  if( printInfo == NULL )
    return;
  
  if ( instance != NULL ) {
    PluginInstance *plugin = (PluginInstance *) instance->pdata;
    DebugMessage( plugin, "Calling NPP_Print." ); 
    
    if (printInfo->mode == NP_FULL) {
      /*
       * PLUGIN DEVELOPERS:
       *	If your plugin would like to take over
       *	printing completely when it is in full-screen mode,
       *	set printInfo->pluginPrinted to TRUE and print your
       *	plugin as you see fit.  If your plugin wants Netscape
       *	to handle printing in this case, set
       *	printInfo->pluginPrinted to FALSE (the default) and
       *	do nothing.  If you do want to handle printing
       *	yourself, printOne is true if the print button
       *	(as opposed to the print menu) was clicked.
       *	On the Macintosh, platformPrint is a THPrint; on
       *	Windows, platformPrint is a structure
       *	(defined in npapi.h) containing the printer name, port,
       *	etc.
       */
      
      void *platformPrint =
	printInfo->print.fullPrint.platformPrint;
      NPBool printOne =
	printInfo->print.fullPrint.printOne;
      
      /* Do the default*/
      printInfo->print.fullPrint.pluginPrinted = FALSE;
    }
    else {	/* If not fullscreen, we must be embedded */
      /*
       * PLUGIN DEVELOPERS:
       *	If your plugin is embedded, or is full-screen
       *	but you returned false in pluginPrinted above, NPP_Print
       *	will be called with mode == NP_EMBED.  The NPWindow
       *	in the printInfo gives the location and dimensions of
       *	the embedded plugin on the printed page.  On the
       *	Macintosh, platformPrint is the printer port; on
       *	Windows, platformPrint is the handle to the printing
       *	device context.
       */
      
      NPWindow *printWindow =
	&(printInfo->print.embedPrint.window);
      void *platformPrint =
	printInfo->print.embedPrint.platformPrint;
    }
  }
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++
 * NPP_URLNotify:
 * Notifies the instance of the completion of a URL request. 
 * 
 * NPP_URLNotify is called when Netscape completes a NPN_GetURLNotify
 * or NPN_PostURLNotify request, to inform the plug-in that the
 * request, identified by url, has completed for the reason specified
 * by reason. The most common reason code is NPRES_DONE, indicating
 * simply that the request completed normally. Other possible reason
 * codes are NPRES_USER_BREAK, indicating that the request was halted
 * due to a user action (for example, clicking the "Stop" button), and
 * NPRES_NETWORK_ERR, indicating that the request could not be
 * completed (for example, because the URL could not be found). The
 * complete list of reason codes is found in npapi.h.
 * 
 * The parameter notifyData is the same plug-in-private value passed as an
 * argument to the corresponding NPN_GetURLNotify or NPN_PostURLNotify
 * call, and can be used by your plug-in to uniquely identify the request. 
 +++++++++++++++++++++++++++++++++++++++++++++++++*/

void
NPP_URLNotify( NPP instance, const char *url, NPReason reason,
	       void *notifyData )
{
  /* Not used in the TerraVision plugin. */
}

/*****************************************************************************
 * SECTION 4 - Java Native Method Implementations
 ****************************************************************************/

/*** native cmd ***/

extern JRI_PUBLIC_API( void )
native_nptervis_cmd( JRIEnv *env, struct nptervis *self,
		     struct java_lang_String *a )
{
  NPP instance =
    (NPP) netscape_plugin_Plugin_getPeer( env, (netscape_plugin_Plugin*)self );
  
  if ( instance != NULL ) {
    
    PluginInstance *plugin = (PluginInstance *) instance->pdata;
    const char *chars = JRI_GetStringUTFChars( env, a );

    /* just send the command straight through to the external app */

    TalkToTerraVision( plugin, chars );

    /* output the message to the Java Console too. */

    DebugMessage( plugin, "JS: %s", chars );
    
    /* force a refresh of the plugin instance */

    NPP_SetWindow( instance, plugin->fWindow );
  }
}

/*****************************************************************************
 * SECTION 5 - Utility Method Implementations
 ****************************************************************************/

/*+++++++++++++++++++++++++++++++++++++++++++++++++
 * DisplayJavaMessage
 *
 * This function is a utility routine that calls back into Java to print
 * messages to the Java Console.
 +++++++++++++++++++++++++++++++++++++++++++++++++*/

void
DisplayJavaMessage( PluginInstance *plugin, char const *msg )
{
  jref    str, javaPeer;
  JRIEnv  *env = NPN_GetJavaEnv();
  
  /* Did Java fail to initialize? If so, do nothing. */
  
  if ( env == NULL )
    return;
  
  /* Use the JRI (see jri.h) to create a Java string from message */
  
  str = (jref) JRI_NewStringUTF( env, msg, strlen(msg) );
  
  /* Use the NPN_GetJavaPeer operation to get the Java instance that */
  /* corresponds to our plug-in (an instance of the nptervis class): */
  
  javaPeer = NPN_GetJavaPeer( plugin->fNPP );
  
  /* Finally, call the Java method, pringMsg, passing the execution */
  /* environment, the object, and the java string.                  */
  
  nptervis_printMsg( env, javaPeer, str );
}

void
DebugMessage( PluginInstance *plugin, char const *fmt, ... )
{
  char msg[2048];
  va_list arg;

  va_start( arg, fmt );
  vsprintf( msg, fmt, arg );
  va_end( arg );

  if ( plugin->debug_mode )
    DisplayJavaMessage( plugin, msg );
}

void
AppendMsg( char const *msg )
{
  FILE *handle;
  if ( ( handle = fopen( "message.log", "a" ) ) == NULL )
    return;
  fprintf( handle, "%s\n", msg );
  fclose( handle );
}

void
ShowStatus( PluginInstance *plugin, char const *msg )
{
  NPN_Status( plugin->fNPP, msg );
}

