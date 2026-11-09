/*
 * Synopsis: AboutDialog.c
 *	
 * Description: Popup to desribe the MAGIC project and TV
 *
 * Externals:
 *
 * Returns:
 *
 *  ----- BEGIN LICENSE BLOCK -----
 *  Version: MPL 1.1
 * 
 *  The contents of this file are subject to the Mozilla Public License Version
 *  1.1 (the "License"); you may not use this file except in compliance with
 *  the License. You may obtain a copy of the License at
 *  http://www.mozilla.org/MPL/
 * 
 *  Software distributed under the License is distributed on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 *  for the specific language governing rights and limitations under the
 *  License.
 * 
 *  The Original Code is the TerraVision 3D Terrain Visualization System.
 * 
 *  The Initial Developer of the Original Code is SRI International.
 *
 *  Portions created by the Initial Developer are Copyright (C) 1992-2002
 *  the Initial Developer. All Rights Reserved.
 * 
 *  Contributor(s): Yvan Leclerc, Lee Iverson, Martin Reddy, Nat Bletter. 
 *  ----- END LICENSE BLOCK -----
 *
 */

/* RCS INFO */
static char const AboutDialog_rcsid[] = "$Id: AboutDialog.c,v 1.4.2.2 2002/10/31 05:26:45 reddy Exp $";

#include "TerraVisionInc.h"
#include "AboutDialog.h"

#include <stdlib.h>

#include <tsm/tsmImage.h>

#include <glt/glt.h>
#include <glt/glm.h>

#include "X11Include.h"

#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/LabelG.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/SeparatoG.h>

struct AboutDialogWidget
{
    Widget          mainForm;
    Widget          mapWidget;
    Widget          showcaseWidget;
    glmWindow       glmw;
    GLubyte *       usaImage;
    GLuint          usaImageX, usaImageY;
    GLenum          usaImageFmt;
    int             managed;
    char            version[40];
    char            versionDate[40];
    char*           showcaseFile;
};

static void	AboutDialogWidgetInitCallback(Widget, XtPointer, XtPointer);
static void	AboutDialogWidgetExposeCallback(Widget, XtPointer, XtPointer);
static void	AboutDialogWidgetX11Callback(Widget, XtPointer, XtPointer);

/*
 * Synopsis:
 *   
 *
 * Description:
 *      Create Confirm About Dialog Box
 *
 * Externals:
 *
 *
 * Returns: NULL on failure
 *    a pointer to an AboutDialogWidget on success
 *
 * Author:
 *      Stephen Lau
 * 
 * Date: October 19, 1992
 *
 */
AboutDialogWidget *
CreateAboutDialog(Widget parent,
		  char const * version,
		  char const * versionDate)
{
  static char * funcName = "CreateAboutDialog";
  Widget bBoard, separator, rowcol, frame;
  XtCallbackRec callbacks[] = {{NULL, NULL}, {NULL, NULL}};
  AboutDialogWidget * aboutDialog;
  int n = 0; 
  Arg args[11];
  int i, j;
  char buffer[1024];
  XmString string;

  glmContext glmCtx = glmXFindContext (XtDisplay (parent),
				       XtScreen (parent),
				       TRUE, 0);

  aboutDialog = (AboutDialogWidget *) malloc(sizeof(AboutDialogWidget));
  if(!aboutDialog) {
    tsmStatus("%s: Unable to allocate space", funcName);
    return NULL;
  }

  /* CREATE THE VERSION STRING */
  strcpy (aboutDialog->version, version);

  /* CREATE THE VERSION DATE STRING */
  i = 0;
  while(versionDate[i] != ':')
    i++;
  j = 0;
  i++;
  while(versionDate[i] != '$')
    aboutDialog->versionDate[j++] = versionDate[i++];
  aboutDialog->versionDate[j] = 0;
 
  aboutDialog->managed = FALSE;
    
  /* Create the main About window */

  string = tvXmString ("About TerraVision");
  XtSetArg(args[n], XmNdialogTitle, string); n++;
  XtSetArg(args[n], XmNresizable, FALSE); n++;
  XtSetArg(args[n], XmNnoResize, TRUE); n++;

  aboutDialog->mainForm = XmCreateFormDialog( parent, "About TerraVision", 
						args, n );
  XmStringFree (string);

  XtVaSetValues( aboutDialog->mainForm,
		 XmNverticalSpacing, 10,
		 XmNnoResize, TRUE,
		 NULL );

  /* Set up a rowcol widget to hold the image with the label next to it */

  rowcol = XtVaCreateManagedWidget( "rowcol", xmRowColumnWidgetClass,
				    aboutDialog->mainForm,
				    XmNorientation, XmHORIZONTAL,
				    XmNleftAttachment, XmATTACH_FORM,
				    XmNrightAttachment, XmATTACH_FORM,
				    XmNtopAttachment, XmATTACH_FORM,
				    XmNleftOffset, 15,
				    XmNrightOffset, 15,
				    NULL );
  
  /* Create the About image */

  frame = XtVaCreateManagedWidget( "frame", xmFrameWidgetClass, rowcol,
				   XmNshadowType, XmSHADOW_IN,
				   NULL );

  aboutDialog->mapWidget =
    XtVaCreateManagedWidget( "aboutDialogMapWidget",
			     glwMDrawingAreaWidgetClass, frame,
			     GLwNvisualInfo, glmXContextVisualInfo (glmCtx),
			     XmNheight, 1,
			     XmNwidth, 1,
			     NULL );
  
  /* CREATE AN OPENGL CONTEXT FOR THIS ONE. */
  aboutDialog->glmw = glmXtCreateWindow (aboutDialog->mapWidget, 
					 glmCtx, True, False);
  
  /* ADD GL CALLBACKS */
  XtAddCallback(aboutDialog->mapWidget,
		GLwNexposeCallback, 
		AboutDialogWidgetExposeCallback, 
		aboutDialog);
  
  XtAddCallback(aboutDialog->mapWidget,
		GLwNginitCallback, 
		AboutDialogWidgetInitCallback, 
		aboutDialog);
  
  callbacks[0].callback = AboutDialogWidgetX11Callback;
  callbacks[0].closure = (XtPointer) aboutDialog;
  
  /* Create the text that sits next to the image */
 
  sprintf(buffer,"Version: %s", aboutDialog->version);
  sprintf( buffer, "TerraVision\nTerrain Visualization System\n\n"
	   "http://www.tvgeo.com/\n\n"
	   "(c) SRI International 1993-2001\nAll rights reserved.\n\n"
	   "Version: %s\n\n"
	   "Release Date: %s\n\nModules:",
	   aboutDialog->version, aboutDialog->versionDate );

#ifdef DPSS
  strcat( buffer, " DPSS" );
#endif

#ifdef CAVE
  strcat( buffer, " CAVE" ); 
#endif

#ifdef TRACING
  strcat( buffer, " TRACING" );
#endif

#if defined(_pthr_sproc_h_)
  strcat( buffer, " SPROC-PTHR" );
#endif

  string = tvXmString(buffer);
  XtVaCreateManagedWidget("aboutDialogVersionLabelWidget",
			  xmLabelGadgetClass, rowcol,
			  XmNlabelString, string,
			  XmNmarginLeft, 15,
			  XmNmarginRight, 15,
			  NULL);
  XmStringFree (string);

  /* Now add the separator and Dismiss button */
  
  separator = XtVaCreateManagedWidget("Separator", xmSeparatorGadgetClass, 
				      aboutDialog->mainForm,
				      XmNleftAttachment, XmATTACH_FORM,
				      XmNrightAttachment, XmATTACH_FORM,
				      NULL);
  
  XtVaSetValues(rowcol,
		XmNbottomAttachment, XmATTACH_WIDGET,
		XmNbottomWidget, separator,
		NULL);
  
  bBoard = XtVaCreateManagedWidget( "AboutDialigWidgetBoard",
				    xmFormWidgetClass, aboutDialog->mainForm,
				    XmNbottomAttachment, XmATTACH_FORM,
				    XmNleftAttachment, XmATTACH_FORM,
				    XmNrightAttachment, XmATTACH_FORM,
				    XmNleftOffset, 15,
				    XmNrightOffset, 15,
				    XmNfractionBase, 3, NULL );
  
  XtVaSetValues(separator,
		XmNbottomAttachment, XmATTACH_WIDGET,
		XmNbottomWidget, bBoard,
		NULL);
  
  string = tvXmString("Dismiss");
  XtVaCreateManagedWidget("AboutDialogWidgetDismiss", 
			  xmPushButtonGadgetClass, bBoard,
			  XmNleftAttachment, XmATTACH_POSITION,
			  XmNleftPosition, 1,
			  XmNrightAttachment, XmATTACH_POSITION,
			  XmNrightPosition, 2,
			  XmNrightOffset, 20,
			  XmNshowAsDefault, 1,
			  XmNactivateCallback, callbacks,
			  XmNlabelString, string,
			  NULL);
  XmStringFree (string);

  return aboutDialog;
}

/*
 * Synopsis: AboutDialogManage (AboutDialogWidget const *w)
 *
 * Description: Pop up the AboutDialog widget.
 *
 * Externals: None.
 *
 * Returns: Nothing.
 *
 * Author: Lee Iverson.
 * 
 * Date: July 1999.
 *
 */

void
AboutDialogManage (AboutDialogWidget *aboutDialog)
{
  XtManageChild(aboutDialog->mainForm);
  aboutDialog->managed = TRUE;
}

/*
 * Synopsis: AboutDialogWidgetX11Callback(Widget,
 *                                        XtPointer,
 *                                        XtPointer)
 *
 * Description:
 *      Callback for widget actions
 *
 *
 * Externals:
 *
 *
 * Returns: NOTHING
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *		March 25, 1993
 *
 */
static void
AboutDialogWidgetX11Callback(Widget    w, 
			    XtPointer client_data,
			    XtPointer call_data)
{
  /*static char const funcName[] = "AboutDialogWidgetX11Callback";*/
  AboutDialogWidget * aboutDialog = (AboutDialogWidget *) client_data;

  aboutDialog->managed = FALSE;
  XtUnmanageChild(aboutDialog->mainForm);
}

/*
 * Synopsis:
 *    AboutDialogWidgetInitCallback(Widget,
 *                                  XtPointer, XtPointer)
 *	
 *
 * Description: Callback on first popup
 *
 *
 * Externals: 
 *
 *
 * Returns: NOTHING
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *		March 25, 1993
 *
 */
static void
AboutDialogWidgetInitCallback(Widget w, 
			     XtPointer client_data,
			     XtPointer call_data)
{
  static const char *funcName = "AboutDialogWidgetInitCallback";
  AboutDialogWidget *aboutDialog = (AboutDialogWidget *) client_data;
  int               imageLoaded = FALSE;
  char const        *file;

  /* GET THE NAME OF THE IMAGE */
  if (tvGetResourceAsString ("MAPIMAGE_FILE", &file)) {
    /* GET THE IMAGE */
    Image_t *image;

    if ( ( image = tsmReadImage( file ) ) != NULL ) {
      gltInvertImage( image->bitmap, image->width, image->height,
		      image->components );

      aboutDialog->usaImage = (GLubyte *) image->bitmap;
      aboutDialog->usaImageX = image->width;
      aboutDialog->usaImageY = image->height;
      aboutDialog->usaImageFmt = (image->components==1)? GL_LUMINANCE : GL_RGB;

      tsmFreeImage( image, FALSE /* don't free image->bitmap */ );
      imageLoaded = TRUE;
    }

  }

  if ( ! imageLoaded ) {
    tsmError( funcName, "Unable to find about image: %s", file );
    aboutDialog->usaImage = calloc(260 * 162, 1);
    aboutDialog->usaImageX = 260;
    aboutDialog->usaImageX = 162;
    aboutDialog->usaImageFmt = GL_LUMINANCE;
  }

  XtMakeResizeRequest (aboutDialog->mapWidget,
		       aboutDialog->usaImageX, aboutDialog->usaImageY,
		       NULL, NULL);

  /* WE NEED TO INITIALIZE THE GL WINDOW */
  glmMakeCurrentWindow (funcName, aboutDialog->glmw); {
    glViewport (0,  0,
		(GLuint) aboutDialog->usaImageX,
		(GLuint) aboutDialog->usaImageY);

    glMatrixMode (GL_PROJECTION);
    glOrtho (0, aboutDialog->usaImageX,
	     0, aboutDialog->usaImageY,
	     -1, 1);
    glMatrixMode (GL_MODELVIEW);

    gltClearColor4ub (0x00, 0x00, 0x00, 0x00);
    glClear(GL_COLOR_BUFFER_BIT);
  }; glmReleaseWindow (funcName, aboutDialog->glmw);

}

/*
 * Synopsis:
 *    AboutDialogWidgetExposeCallback(Widget,
 *                                    XtPointer, XtPointer)
 *	
 *
 * Description: Callback for expose events
 *
 *
 * Externals:
 *
 *
 * Returns: NOTHING
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *		March 25, 1993
 *
 */
static void
AboutDialogWidgetExposeCallback(Widget w, 
			      XtPointer client_data,
		       	      XtPointer call_data)
{
 static char * funcName = "AboutDialogWidgetExposeCallback";
 AboutDialogWidget * aboutDialog = (AboutDialogWidget *) client_data;

    /* PUT THE IMAGE IN THE WINDOW */
    glmMakeCurrentWindow (funcName, aboutDialog->glmw); {
      gltClearColor4ub (0, 0, 0, 0xFF);
      glClear(GL_COLOR_BUFFER_BIT);
      glRasterPos2i(0,  0);
      glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
      glDrawPixels(aboutDialog->usaImageX, aboutDialog->usaImageY,
		   aboutDialog->usaImageFmt, GL_UNSIGNED_BYTE,
		   aboutDialog->usaImage);
      
    }; glmReleaseWindow (funcName, aboutDialog->glmw);
}
