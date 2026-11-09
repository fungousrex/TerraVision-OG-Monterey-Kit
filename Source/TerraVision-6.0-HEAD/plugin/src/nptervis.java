/*
 * TerraVision LiveConnect Plugin - (c) SRI International 2000.
 *
 * $Id: nptervis.java,v 1.3 2000/01/11 01:22:47 reddy Exp $
 *
 */

import netscape.plugin.Plugin;

class nptervis extends Plugin {

    /*
    ** This native method provides a hook into the TerraVision
    ** plugin's OAA interpreter to control all aspects of the app
    ** The string can be any OAA style command, e.g. "resetview".
    */

    public native void cmd( String msg );

    /*
    ** This method lets us output messages to Netscape's Java
    ** Console from within the TerraVision plugin.
    */

    public void printMsg( String msg ) {
      System.out.println( "npteravis: " + msg );
    }
}
