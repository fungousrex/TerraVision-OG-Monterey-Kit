/*
 * Name: ThreeDControl.c
 *
 * Description: Control panel for manipulating a three D view
 *       of the terrain
 *
 * Function List:
 *
 * Dependencies:
 *
 * Revision History:
 *     $Date: 2002/10/31 05:26:50 $
 *     $Author: reddy $
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

#include "TerraVisionInc.h"

#include <math.h>

#include "ThreeDWidgetI.h"
#include "ThreeDCore.h"
#include "ThreeDControl.h"
#include "dirPixs"
#include "zoomIn2"
#include "zoomOut2"
#include "OAA.h"

#include "X11Include.h"

#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/PushBG.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/SeparatoG.h>

/* RCS INFO */
static char const ThreeDControl_rcsid[] = "$Id: ThreeDControl.c,v 1.10.2.1 2002/10/31 05:26:50 reddy Exp $";

struct ThreeDControl {
  Widget        mainForm;
  Widget        coordResetButton;
  Widget        viewerResetButton;
  Widget        orientResetButton;
  Widget        moveButton[9][2];
  Widget        velScale;
  int           currButton[2];
  float         offset;
  XtIntervalId  timerId;
  ThreeDWidget *threeDWidget;
};

static void ThreeDControlCreateWidgets(ThreeDControl *, Widget);
static void ThreeDControlCreateUtilButtons(ThreeDControl *, Widget);
static void ThreeDControlDismissCallback(Widget, XtPointer, XtPointer);
static void ThreeDControlMoveButtonCB(Widget, XtPointer, XtPointer);
static void ThreeDControlCreateMoveButtons(ThreeDControl *, Widget, int, char *);
static void ThreeDControlCreateLeftRightPixmaps(ThreeDControl *, int);
static void ThreeDControlCreateForeBackPixmaps(ThreeDControl *, int);
static void ThreeDControlCreatePixmaps(ThreeDControl *, int);
static void ThreeDControlMove(XtPointer, XtIntervalId *);
static void ThreeDControlScaleChangedCB(Widget, XtPointer, XtPointer);

typedef enum ThreeDControlMoveType
{
   moveX = 0,
   moveY,
   moveZ,
   moveHeading,
   movePitch,
   moveRoll,
   moveForeBack,
   moveLeftRight,
   moveUpDown
}ThreeDControlMoveType;

#define VEL_SCALE_DEFAULT   100

/*
 * Synopsis: ThreeDControl * CreateThreeDControl()
 *	
 *
 * Description: Creates a three d control panel
 *
 *
 * Externals:
 *
 *
 * Returns: ThreeDControl * on success
 *         NULL on failure
 *
 * Author:
 *		Stephen Lau
 * 
 * Date: July 1994
 *
 */
ThreeDControl *
CreateThreeDControl(Widget parent,
		    void * threeDWidget)
{
 static char * funcName = "CreateThreeDControl";
 ThreeDControl * threeDControl = NULL;

    threeDControl = (ThreeDControl *) malloc(sizeof(ThreeDControl));
    if(!threeDControl)
    {
        tsmError (funcName, "Unable to create the control widget!");
	return NULL;
    }
    ThreeDControlCreateWidgets(threeDControl, parent);

    threeDControl->timerId = 0;
    threeDControl->offset = VEL_SCALE_DEFAULT;
    threeDControl->threeDWidget = threeDWidget;

    return threeDControl;
}

/*
 * Synopsis: void ThreeDControlCreateWidgets()
 *	
 *
 * Description: Creates the widgets
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
 * Date: July 1994
 *
 */
static void
ThreeDControlCreateWidgets(ThreeDControl * threeDControl,
			   Widget          parent)
{
 /*static char * funcName = "ThreeDControlCreateWidgets";*/
 Widget       formWidget, separator;
 Widget       mainForm;
 int          n = 0; 
 Arg          args[10];
 XtCallbackRec callbacks[] = {{NULL, NULL}, {NULL, NULL}};
 XmString label;

    label = tvXmString("Viewpoint Control Panel");
    XtSetArg(args[n], XmNdialogTitle, label); n++;

    /*
     * THE UNDERLYING WIDGET IS A FORM WIDGET THAT ACTS AS A DIALOG BOX.
     * ALL OTHER WIDGETS FOR THE OVERHEAD VIEW ARE DERIVED FROM THIS ONE
     *
     */
    threeDControl->mainForm = mainForm =
      XmCreateFormDialog(parent, "threeDControlForm", args, n);

    XmStringFree (label);

    /* SET THE SPACING OF THE DIALOG BOX */
    XtVaSetValues(mainForm,
		  XmNverticalSpacing, 2,
		  XmNhorizontalSpacing, 2,
		  XmNnoResize, TRUE,
		  NULL);

    /* ALL OF THESE BUTTONS HAVE A CALLBACK TO 
     * ThreeDControMoveButtonCB AND PASS A POINTER TO 
     * THE CORRESPONDING ThreeDControl STRUCTURE
     */
    callbacks[0].callback = ThreeDControlMoveButtonCB;
    callbacks[0].closure = (XtPointer) threeDControl;

    formWidget = XtVaCreateManagedWidget("form1",
					 xmFormWidgetClass,
					 mainForm,
					 XmNtopAttachment, XmATTACH_FORM,
					 XmNleftAttachment, XmATTACH_FORM,
					 XmNrightAttachment, XmATTACH_FORM,
					 NULL);

    /* CREATE THE COORD RESET */
    threeDControl->coordResetButton =
       XtVaCreateManagedWidget("Reset to Origin",
			       xmPushButtonGadgetClass,
			       formWidget,
			       XmNpushButtonEnabled, TRUE,
			       XmNdisarmCallback, callbacks,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNleftAttachment, XmATTACH_FORM,
			       NULL);

    XtVaCreateManagedWidget("threeDControlCoordLabelWidget",
			    xmLabelGadgetClass,
			    formWidget,
			    XmNtopAttachment, XmATTACH_FORM,
			    XmNbottomAttachment, XmATTACH_FORM,
			    XmNrightAttachment, XmATTACH_FORM,
			    XmNleftAttachment, XmATTACH_WIDGET,
			    XmNleftWidget, threeDControl->coordResetButton,
			    NULL);
 
    separator =
       XtVaCreateManagedWidget("separator1",
			       xmSeparatorGadgetClass, 
			       mainForm,
			       XmNheight, 4,
			       XmNshadowThickness, 4,
			       XmNtopAttachment, XmATTACH_WIDGET,
			       XmNtopWidget, formWidget,
			       XmNtopOffset, 2,
			       XmNleftAttachment, XmATTACH_FORM,
			       XmNrightAttachment, XmATTACH_FORM,
			       NULL);

    formWidget =
       XtVaCreateManagedWidget("form2",
			       xmFormWidgetClass,
			       mainForm,
			       XmNtopAttachment, XmATTACH_WIDGET,
			       XmNtopWidget, separator,
			       XmNleftAttachment, XmATTACH_FORM,
			       XmNrightAttachment, XmATTACH_FORM,
			       XmNhorizontalSpacing, 20,
			       NULL);
 
    {
      Widget formWidget1 =
	XtVaCreateManagedWidget("form3",
				xmFormWidgetClass,
				formWidget,
				XmNtopAttachment, XmATTACH_FORM,
				XmNleftAttachment, XmATTACH_FORM,
				NULL);

      ThreeDControlCreateMoveButtons(threeDControl, formWidget1, 0, "   X  ");
    
      separator =
	XtVaCreateManagedWidget("separator2", xmSeparatorGadgetClass, 
				formWidget,
				XmNheight, 4,
				XmNshadowThickness, 4,
				XmNtopAttachment, XmATTACH_FORM,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget, formWidget1,
				XmNorientation, XmVERTICAL,
				XmNbottomAttachment, XmATTACH_FORM,
				NULL);
 
      formWidget1 =
	XtVaCreateManagedWidget("form4",
				xmFormWidgetClass,
				formWidget,
				XmNtopAttachment, XmATTACH_FORM,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget, separator,
				NULL);

      ThreeDControlCreateMoveButtons(threeDControl, formWidget1, 1, "  Y   ");

      separator =
	XtVaCreateManagedWidget("separator3", xmSeparatorGadgetClass, 
				formWidget,
				XmNheight, 4,
				XmNshadowThickness, 4,
				XmNtopAttachment, XmATTACH_FORM,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget, formWidget1,
				XmNorientation, XmVERTICAL,
				XmNbottomAttachment, XmATTACH_FORM,
				NULL);
 
      formWidget1 =
	XtVaCreateManagedWidget("form5",
				xmFormWidgetClass,
				formWidget,
				XmNtopAttachment, XmATTACH_FORM,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget, separator,
				NULL);

      ThreeDControlCreateMoveButtons(threeDControl, formWidget1, 2, "   Z  ");

    }

    separator =
       XtVaCreateManagedWidget("separator4", xmSeparatorGadgetClass, 
			       mainForm,
			       XmNheight, 4,
			       XmNshadowThickness, 4,
			       XmNtopAttachment, XmATTACH_WIDGET,
			       XmNtopWidget, formWidget,
			       XmNtopOffset, 2,
			       XmNleftAttachment, XmATTACH_FORM,
			       XmNrightAttachment, XmATTACH_FORM,
			       NULL);

    formWidget =
       XtVaCreateManagedWidget("form6",
			       xmFormWidgetClass,
			       mainForm,
			       XmNtopAttachment, XmATTACH_WIDGET,
			       XmNtopWidget, separator,
			       XmNleftAttachment, XmATTACH_FORM,
			       XmNrightAttachment, XmATTACH_FORM,
			       NULL);
 
    /* CREATE THE COORD RESET */
    threeDControl->orientResetButton =
       XtVaCreateManagedWidget("Reset to Default",
			       xmPushButtonGadgetClass,
			       formWidget,
			       XmNpushButtonEnabled, TRUE,
			       XmNdisarmCallback, callbacks,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNleftAttachment, XmATTACH_FORM,
			       NULL);

    XtVaCreateManagedWidget("threeDControlOrientLabelWidget",
			    xmLabelGadgetClass,
			    formWidget,
			    XmNtopAttachment, XmATTACH_FORM,
			    XmNbottomAttachment, XmATTACH_FORM,
			    XmNrightAttachment, XmATTACH_FORM,
			    XmNleftAttachment, XmATTACH_WIDGET,
			    XmNleftWidget, threeDControl->orientResetButton,
			    NULL);

    separator =
       XtVaCreateManagedWidget("separator5", xmSeparatorGadgetClass, 
			       mainForm,
			       XmNheight, 4,
			       XmNshadowThickness, 4,
			       XmNtopAttachment, XmATTACH_WIDGET,
			       XmNtopWidget, formWidget,
			       XmNtopOffset, 2,
			       XmNleftAttachment, XmATTACH_FORM,
			       XmNrightAttachment, XmATTACH_FORM,
			       NULL);

    formWidget =
       XtVaCreateManagedWidget("form7",
			       xmFormWidgetClass,
			       mainForm,
			       XmNtopAttachment, XmATTACH_WIDGET,
			       XmNtopWidget, separator,
			       XmNleftAttachment, XmATTACH_FORM,
			       XmNrightAttachment, XmATTACH_FORM,
			       XmNhorizontalSpacing, 20,
			       NULL);

    {
      Widget formWidget1 =
	XtVaCreateManagedWidget("form8",
				xmFormWidgetClass,
				formWidget,
				XmNtopAttachment, XmATTACH_FORM,
				XmNleftAttachment, XmATTACH_FORM,
				NULL);

      ThreeDControlCreateMoveButtons(threeDControl, formWidget1, 3, "Heading");

      separator =
	XtVaCreateManagedWidget("separator6", xmSeparatorGadgetClass, 
				formWidget,
				XmNheight, 4,
				XmNshadowThickness, 4,
				XmNtopAttachment, XmATTACH_FORM,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget, formWidget1,
				XmNorientation, XmVERTICAL,
				XmNbottomAttachment, XmATTACH_FORM,
				NULL);

      formWidget1 =
	XtVaCreateManagedWidget("form9",
				xmFormWidgetClass,
				formWidget,
				XmNtopAttachment, XmATTACH_FORM,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget, separator,
				NULL);

      ThreeDControlCreateMoveButtons(threeDControl,
				     formWidget1,
				     4, " Pitch ");

      separator =
	XtVaCreateManagedWidget("separator7", xmSeparatorGadgetClass, 
				formWidget,
				XmNheight, 4,
				XmNshadowThickness, 4,
				XmNtopAttachment, XmATTACH_FORM,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget, formWidget1,
				XmNorientation, XmVERTICAL,
				XmNbottomAttachment, XmATTACH_FORM,
				NULL);

      formWidget1 =
	XtVaCreateManagedWidget("form10",
				xmFormWidgetClass,
				formWidget,
				XmNtopAttachment, XmATTACH_FORM,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget, separator,
				NULL);

      ThreeDControlCreateMoveButtons(threeDControl, formWidget1, 5, " Roll  ");

    }

    separator =
       XtVaCreateManagedWidget("separator7a", xmSeparatorGadgetClass, 
			       mainForm,
			       XmNheight, 4,
			       XmNshadowThickness, 4,
			       XmNtopAttachment, XmATTACH_WIDGET,
			       XmNtopWidget, formWidget,
			       XmNtopOffset, 2,
			       XmNleftAttachment, XmATTACH_FORM,
			       XmNrightAttachment, XmATTACH_FORM,
			       NULL);

    formWidget =
       XtVaCreateManagedWidget("form11",
			       xmFormWidgetClass,
			       mainForm,
			       XmNtopAttachment, XmATTACH_WIDGET,
			       XmNtopWidget, separator,
			       XmNleftAttachment, XmATTACH_FORM,
			       XmNrightAttachment, XmATTACH_FORM,
			       NULL);

    separator =
       XtVaCreateManagedWidget("separator8", xmSeparatorGadgetClass, 
			       mainForm,
			       XmNheight, 4,
			       XmNshadowThickness, 4,
			       XmNtopAttachment, XmATTACH_WIDGET,
			       XmNtopWidget, formWidget,
			       XmNtopOffset, 2,
			       XmNleftAttachment, XmATTACH_FORM,
			       XmNrightAttachment, XmATTACH_FORM,
			       NULL);

    formWidget =
       XtVaCreateManagedWidget("form12",
			       xmFormWidgetClass,
			       mainForm,
			       XmNtopAttachment, XmATTACH_WIDGET,
			       XmNtopWidget, separator,
			       XmNleftAttachment, XmATTACH_FORM,
			       XmNrightAttachment, XmATTACH_FORM,
			       NULL);

    /* CREATE THE COORD RESET */
    threeDControl->viewerResetButton =
       XtVaCreateManagedWidget("Overhead Viewpoint",
			       xmPushButtonGadgetClass,
			       formWidget,
			       XmNpushButtonEnabled, TRUE,
			       XmNdisarmCallback, callbacks,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNleftAttachment, XmATTACH_FORM,
			       NULL);
 
    XtVaCreateManagedWidget("threeDControlViewerLabelWidget",
			    xmLabelGadgetClass,
			    formWidget,
			    XmNtopAttachment, XmATTACH_FORM,
			    XmNbottomAttachment, XmATTACH_FORM,
			    XmNrightAttachment, XmATTACH_FORM,
			    XmNleftAttachment, XmATTACH_WIDGET,
			    XmNleftWidget, threeDControl->viewerResetButton,
			    NULL);

    separator =
       XtVaCreateManagedWidget("separator9", xmSeparatorGadgetClass, 
			       mainForm,
			       XmNheight, 4,
			       XmNshadowThickness, 4,
			       XmNtopAttachment, XmATTACH_WIDGET,
			       XmNtopWidget, formWidget,
			       XmNtopOffset, 2,
			       XmNleftAttachment, XmATTACH_FORM,
			       XmNrightAttachment, XmATTACH_FORM,
			       NULL);
 
    formWidget =
       XtVaCreateManagedWidget("form13",
			       xmFormWidgetClass,
			       mainForm,
			       XmNtopAttachment, XmATTACH_WIDGET,
			       XmNtopWidget, separator,
			       XmNleftAttachment, XmATTACH_FORM,
			       XmNrightAttachment, XmATTACH_FORM,
			       XmNhorizontalSpacing, 20,
			       NULL);


    {
      Widget formWidget1 =
	XtVaCreateManagedWidget("form14",
				xmFormWidgetClass,
				formWidget,
				XmNtopAttachment, XmATTACH_FORM,
				XmNleftAttachment, XmATTACH_FORM,
				NULL);

      ThreeDControlCreateMoveButtons(threeDControl,
				     formWidget1, 6, "Fore / Bck");

      separator =
	XtVaCreateManagedWidget("separator10", xmSeparatorGadgetClass, 
				formWidget,
				XmNheight, 4,
				XmNshadowThickness, 4,
				XmNtopAttachment, XmATTACH_FORM,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget, formWidget1,
				XmNorientation, XmVERTICAL,
				XmNbottomAttachment, XmATTACH_FORM,
				NULL);

      formWidget1 =
	XtVaCreateManagedWidget("form15",
				xmFormWidgetClass,
				formWidget,
				XmNtopAttachment, XmATTACH_FORM,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget, separator,
				NULL);

      ThreeDControlCreateMoveButtons(threeDControl,
				     formWidget1, 7, "Lft / Rt");

      separator =
	XtVaCreateManagedWidget("separator11", xmSeparatorGadgetClass, 
				formWidget,
				XmNheight, 4,
				XmNshadowThickness, 4,
				XmNtopAttachment, XmATTACH_FORM,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget, formWidget1,
				XmNorientation, XmVERTICAL,
				XmNbottomAttachment, XmATTACH_FORM,
				NULL);

      formWidget1 =
	XtVaCreateManagedWidget("form16",
				xmFormWidgetClass,
				formWidget,
				XmNtopAttachment, XmATTACH_FORM,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget, separator,
				NULL);
 
      ThreeDControlCreateMoveButtons(threeDControl, formWidget1,
				     8, "Up / Dwn");

    }

    separator = XtVaCreateManagedWidget("separator12", xmSeparatorGadgetClass, 
					mainForm,
	  				XmNheight, 4,
					XmNshadowThickness, 4,
					XmNtopAttachment, XmATTACH_WIDGET,
    					XmNtopWidget, formWidget,
					XmNtopOffset, 2,
					XmNleftAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_FORM,
					NULL);

    formWidget = XtVaCreateManagedWidget("form17",
					 xmFormWidgetClass,
					 mainForm,
					 XmNtopAttachment, XmATTACH_WIDGET,
					 XmNtopWidget, separator,
					 XmNleftAttachment, XmATTACH_FORM,
					 XmNrightAttachment, XmATTACH_FORM,
					 NULL);

    /* CONTROLS THE ELEVATION SCALING FACTOR */
    threeDControl->velScale =
      XtVaCreateManagedWidget("threeDControlScaleWidget",
			      xmScaleWidgetClass,
			      formWidget,
			      XmNleftAttachment, XmATTACH_FORM,
			      XmNrightAttachment, XmATTACH_FORM,
			      XmNbottomAttachment, XmATTACH_FORM,
			      XmNtopAttachment, XmATTACH_FORM,
			      XmNorientation, XmHORIZONTAL,
			      XmNmaximum, 1000,
			      XmNminimum, 0,
			      XmNdecimalPoints, 1,
			      XmNshowValue, True,
			      XmNvalue, VEL_SCALE_DEFAULT,
			      NULL);

    /* CALLBACK FUNCTION FOR WHEN THE SCALE IS ACTIVATED */
    XtAddCallback(threeDControl->velScale, XmNvalueChangedCallback,
		  ThreeDControlScaleChangedCB, threeDControl);

    separator = XtVaCreateManagedWidget("separator13", xmSeparatorGadgetClass, 
					mainForm,
	  				XmNheight, 4,
					XmNshadowThickness, 4,
					XmNtopAttachment, XmATTACH_WIDGET,
    					XmNtopWidget, formWidget,
					XmNtopOffset, 2,
					XmNleftAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_FORM,
					NULL);

    ThreeDControlCreateUtilButtons(threeDControl, separator);
}

/*
 *
 * Synopsis:
 *     static void ThreeDControlCreateUtilButtons(ThreeDControl *, Widget);
 *	
 * Description: Create utility buttons, such as "Dismiss', "help" etc.
 *
 * Externals: NONE
 *
 * Returns: NOTHING
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *	July 1994
 *
 */
static void
ThreeDControlCreateUtilButtons(ThreeDControl * threeDControl,
			      Widget         topWidget)
{
 /*static char * funcName = "ThreeDControlCreateUtilButtons";*/
 Widget bBoard;
 XtCallbackRec callbacks[] = {{NULL, NULL}, {NULL, NULL}};
 XmString label;

    /* ALL OF THESE BUTTONS HAVE A CALLBACK TO 
     * ThreeDControlDismissCallback AND PASS A POINTER TO 
     * THE CORRSEPONDING ThreeDControl STRUCTURE
     */
    callbacks[0].callback = ThreeDControlDismissCallback;
    callbacks[0].closure = (XtPointer) threeDControl;

    bBoard = XtVaCreateManagedWidget("ThreeDControlBoard",
				     xmRowColumnWidgetClass,
				     threeDControl->mainForm,
				     XmNbottomAttachment, XmATTACH_FORM,
				     XmNleftAttachment, XmATTACH_FORM,
				     XmNrightAttachment, XmATTACH_FORM,
				     XmNentryAlignment, XmALIGNMENT_CENTER,
				     XmNorientation, XmHORIZONTAL,
				     XmNpacking, XmPACK_COLUMN,
				     XmNspacing, 200,
				     XmNmarginWidth, 5,
				     XmNmarginHeight, 5,
				     XmNnoResize, TRUE,
				     NULL);

    XtVaSetValues(topWidget,
	   	  XmNbottomAttachment, XmATTACH_WIDGET,
		  XmNbottomWidget, bBoard,
		  NULL);

    /* CREATE THE DISMISS BUTTON */
    label = tvXmString("Dismiss");
    XtVaCreateManagedWidget("ThreeDControlDismiss",
			    xmPushButtonGadgetClass,
			    bBoard,
			    XmNleftOffset, 20,
			    XmNrightOffset, 20,
			    XmNactivateCallback, callbacks,
			    XmNshowAsDefault, 1,
			    XmNlabelString, label,
			    NULL);
    XmStringFree (label);
}

/*
 * Synopsis:
 *    static void ThreeDControlDismissCallback(Widget,
 *                                        XtPointer,
 *                                        XtPointer)
 *
 * Description: Callback function for actions such as "Dismiss" button,
 *         Function unmanages the threeDControl widget
 *
 * Externals: NONE
 *
 * Returns: NOTHING
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *	July 1994
 *
 */
static void
ThreeDControlDismissCallback(Widget    w,
			    XtPointer client_data,
			    XtPointer call_data)
{
  /*static char * funcName = "ThreeDControlDismissCallback";*/
 ThreeDControl * threeDControl;

    threeDControl = (ThreeDControl *) client_data;

    /* UNMANAGE THE WIDGET */
    XtUnmanageChild(threeDControl->mainForm);
}

/*
 * Synopsis:void ThreeDControlCreateButtons(ThreeDControl *, Widget)
 *
 * Description: Create the buttons for the overhead view. This is only
 *	done once, but has to be done seperately from the original
 *	initialization procedure because of the bitmaps for the buttons.
 *
 * Externals:
 *
 * Returns: void
 *
 * Author:
 *      Stephen Lau
 *
 * Date: July, 1994
 *
 */
static void
ThreeDControlCreateMoveButtons(ThreeDControl * threeDControl,
			       Widget          parent,
			       int             i,
			       char *          labelName)
{
 /*static char * funcName = "ThreeDControlCreateMoveButtons";*/
 static XtCallbackRec callbacks[] = {{NULL, NULL}, {NULL, NULL}};
 Widget labelWidget;
 XmString label;

    /* FOR THE ZOOM BUTTONS, WE WANT TO ADD THIS CALLBACK */
    callbacks[0].callback = ThreeDControlMoveButtonCB;
    callbacks[0].closure = (XtPointer) threeDControl;

    label = tvXmString(labelName);
    labelWidget = XtVaCreateManagedWidget("threeDControlLabelWidget",
					  xmLabelGadgetClass,
					  parent,
					  XmNtopAttachment, XmATTACH_FORM,
					  XmNleftAttachment, XmATTACH_FORM,
					  XmNrightAttachment, XmATTACH_FORM,
					  XmNlabelString, label,
					  NULL);
    XmStringFree (label);

    /* CREATE THE MOVE UP BUTTON */
    /* Note: these need to be Widgets, rather than Gadgets, as later 
     * operations install custom pixmaps, and we use XtWindow() for that
     * purpose, etc.      --RLB
     */
    threeDControl->moveButton[i][0] =
      XtVaCreateManagedWidget("moveUp",
			      xmPushButtonWidgetClass, parent,
			      XmNpushButtonEnabled, TRUE,
			      XmNarmCallback, callbacks,
			      XmNdisarmCallback, callbacks,
			      XmNtopAttachment, XmATTACH_WIDGET,
			      XmNtopWidget, labelWidget,
			      XmNleftAttachment, XmATTACH_FORM,
			      XmNrightAttachment, XmATTACH_FORM,
			      NULL);

    /* CREATE THE MOVE DOWN BUTTON */
    threeDControl->moveButton[i][1] =
      XtVaCreateManagedWidget("moveDown",
			      xmPushButtonWidgetClass, parent,
			      XmNpushButtonEnabled, TRUE,
			      XmNarmCallback, callbacks,
			      XmNdisarmCallback, callbacks,
			      XmNtopAttachment, XmATTACH_WIDGET,
			      XmNtopWidget, threeDControl->moveButton[i][0],
			      XmNleftAttachment, XmATTACH_FORM,
			      XmNrightAttachment, XmATTACH_FORM,
			      XmNbottomAttachment, XmATTACH_FORM,
			      NULL);
}

/*
 * Synopsis: void ThreeDControlMoveButtonCB()
 *
 * Description:  Callback for the movement buttons
 *
 * Externals: 
 *
 *
 * Returns: NOTHING
 *
 * Author:
 *		Stephen Lau
 * 
 * Date: 1994
 *
 */
static void
ThreeDControlMoveButtonCB(Widget w, 
			  XtPointer client_data, 
			  XtPointer call_data)
{
/* static char * funcName = "ThreeDControlMoveButtonCB";*/
 ThreeDControl * threeDControl = (ThreeDControl *) client_data;
 tvBase * threeD = threeDControl->threeDWidget->threeD;
 int i;
 XtAppContext appContext;
 XmPushButtonCallbackStruct *buttonData =
    (XmPushButtonCallbackStruct *) call_data;

    appContext = XtWidgetToApplicationContext(threeDControl->mainForm);
    if(threeDControl->timerId)
    {
        XtRemoveTimeOut(threeDControl->timerId);
        threeDControl->timerId = 0;
    }

    /* WE PUSHING A BUTTON DOWN? */
    if(buttonData->reason == XmCR_ARM)
    {
	/* FIGURE OUT WHICH BUTTON IT IS AND REACT */
        for(i = 0; i < 9; i++)
        {
            if(threeDControl->moveButton[i][0] == w)
	    {
	        threeDControl->currButton[0] = i;
		threeDControl->currButton[1] = 0;
		break;
	    }
	    if(threeDControl->moveButton[i][1] == w)
	    {
	        threeDControl->currButton[0] = i;
		threeDControl->currButton[1] = 1;
		break;
	    }
        }
        threeDControl->timerId =
	   XtAppAddTimeOut(appContext, 10, ThreeDControlMove, threeDControl);
    }
    else
    {
        threeDControl->currButton[0] = -1;
	threeDControl->currButton[1] = -1;
	if(w == threeDControl->orientResetButton)
	{
	    tvViewpoint view;
	    tvBaseGetView (threeD, &view);
	    view.pitch = 0;
	    view.yaw = -M_PI/2;
	    view.roll = 0;
	    tvSetView (threeD, &view, TRUE);
	    tvStopMotion( threeD );
        }
	if(w == threeDControl->coordResetButton)
	{
	    tvResetView (threeD);
	    tvStopMotion (threeD);
        }
	if(w == threeDControl->viewerResetButton)
	{
	    tvBaseCentralView (threeD, tvBaseTop (threeD));
	    tvStopMotion (threeD);
        }
    }
}

/*
 * Synopsis:
 *      void ThreeDControlCreatePixmaps(void)
 *
 * Description:
 *    Each of the buttons have a pixmap associated with it. The pixmaps
 * are created here and attached to the widgets.
 *
 * Externals:
 *    This is X windows, of course we have globals.
 * dirButton[] - The directional button widgets
 * zoomButton - The zoom button widgets
 * The various bitmaps. See the include files.
 *
 * Returns:
 *   void
 *
 * Author:
 *      Stephen Lau
 *
 * Date:  July 1994,
 *
 */
static void
ThreeDControlCreatePixmaps(ThreeDControl * threeDControl,
			   int             index)
{
/* static char * funcName = "ThreeDControlCreatePixmaps";*/
 int     depth;
 Pixel   fore, back, armColor;
 Pixmap  pixmap, armedPixmap;

    /* GET COLOR AND DEPTH OF BUTTON */
    XtVaGetValues(threeDControl->moveButton[index][0],
		  XmNdepth, &depth,
		  XmNbackground, &back,
		  XmNforeground, &fore,
		  XmNarmColor, &armColor,
		  NULL);

    pixmap = XCreatePixmapFromBitmapData(XtDisplay(threeDControl->moveButton[index][0]),
					 XtWindow(threeDControl->moveButton[index][0]),
					 dirBits[0],
					 north_height,
					 north_width,
					 fore, back,
					 depth);
 
    armedPixmap = XCreatePixmapFromBitmapData(
					      XtDisplay(threeDControl->moveButton[index][0]),
					      XtWindow(threeDControl->moveButton[index][0]),
					      dirBits[0],
					      north_height,
					      north_width,
					      fore, armColor,
					      depth);

    XtVaSetValues(threeDControl->moveButton[index][0],
		  XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap, pixmap,
		  XmNarmPixmap, armedPixmap,
		  NULL);

    pixmap = XCreatePixmapFromBitmapData(XtDisplay(threeDControl->moveButton[index][1]),
					 XtWindow(threeDControl->moveButton[index][1]),
					 dirBits[4],
					 north_height,
					 north_width,
					 fore, back,
					 depth);
 
    armedPixmap = XCreatePixmapFromBitmapData(
					      XtDisplay(threeDControl->moveButton[index][1]),
					      XtWindow(threeDControl->moveButton[index][1]),
					      dirBits[4],
					      north_height,
					      north_width,
					      fore, armColor,
					      depth);

    XtVaSetValues(threeDControl->moveButton[index][1],
		  XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap, pixmap,
		  XmNarmPixmap, armedPixmap,
		  NULL);
}
/*
 * Synopsis:
 *      void ThreeDControlCreatePixmaps(void)
 *
 * Description:
 *    Each of the buttons have a pixmap associated with it. The pixmaps
 * are created here and attached to the widgets.
 *
 * Externals:
 *    This is X windows, of course we have globals.
 * dirButton[] - The directional button widgets
 * zoomButton - The zoom button widgets
 * The various bitmaps. See the include files.
 *
 * Returns:
 *   void
 *
 * Author:
 *      Stephen Lau
 *
 * Date:  July 1994,
 *
 */
static void
ThreeDControlCreateForeBackPixmaps(ThreeDControl * threeDControl,
				   int             index)
{
/* static char * funcName = "ThreeDControlCreateForeBackPixmaps";*/
 int     depth;
 Pixel   fore, back, armColor;
 Pixmap  pixmap, armedPixmap;

    /* GET COLOR AND DEPTH OF BUTTON */
    XtVaGetValues(threeDControl->moveButton[index][0],
		  XmNdepth, &depth,
		  XmNbackground, &back,
		  XmNforeground, &fore,
		  XmNarmColor, &armColor,
		  NULL);

    pixmap = XCreatePixmapFromBitmapData(XtDisplay(threeDControl->moveButton[index][0]),
					 XtWindow(threeDControl->moveButton[index][0]),
					 (char *) zoomIn2_bits,
					 zoomIn2_height,
					 zoomIn2_width,
					 fore, back,
					 depth);
 
    armedPixmap = XCreatePixmapFromBitmapData(
					      XtDisplay(threeDControl->moveButton[index][0]),
					      XtWindow(threeDControl->moveButton[index][0]),
					      (char *) zoomIn2_bits,
					      zoomIn2_height,
					      zoomIn2_width,
					      fore, armColor,
					      depth);

    XtVaSetValues(threeDControl->moveButton[index][0],
		  XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap, pixmap,
		  XmNarmPixmap, armedPixmap,
		  NULL);

    pixmap = XCreatePixmapFromBitmapData(XtDisplay(threeDControl->moveButton[index][1]),
					 XtWindow(threeDControl->moveButton[index][1]),
					 (char *) zoomOut2_bits,
					 zoomOut2_height,
					 zoomOut2_width,
					 fore, back,
					 depth);
 
    armedPixmap = XCreatePixmapFromBitmapData(
					      XtDisplay(threeDControl->moveButton[index][1]),
					      XtWindow(threeDControl->moveButton[index][1]),
					      (char *) zoomOut2_bits,
					      zoomOut2_height,
					      zoomOut2_width,
					      fore, armColor,
					      depth);

    XtVaSetValues(threeDControl->moveButton[index][1],
		  XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap, pixmap,
		  XmNarmPixmap, armedPixmap,
		  NULL);
}
/*
 * Synopsis:
 *      void ThreeDControlCreateLeftRightPixmaps(void)
 *
 * Description:
 *    Each of the buttons have a pixmap associated with it. The pixmaps
 * are created here and attached to the widgets.
 *
 * Externals:
 *    This is X windows, of course we have globals.
 * dirButton[] - The directional button widgets
 * zoomButton - The zoom button widgets
 * The various bitmaps. See the include files.
 *
 * Returns:
 *   void
 *
 * Author:
 *      Stephen Lau
 *
 * Date:  July 1994,
 *
 */
static void
ThreeDControlCreateLeftRightPixmaps(ThreeDControl * threeDControl,
				    int             index)
{
/* static char * funcName = "ThreeDControlCreateLeftRightPixmaps";*/
 int     depth;
 Pixel   fore, back, armColor;
 Pixmap  pixmap, armedPixmap;

    /* GET COLOR AND DEPTH OF BUTTON */
    XtVaGetValues(threeDControl->moveButton[index][0],
		  XmNdepth, &depth,
		  XmNbackground, &back,
		  XmNforeground, &fore,
		  XmNarmColor, &armColor,
		  NULL);

    pixmap = XCreatePixmapFromBitmapData(XtDisplay(threeDControl->moveButton[index][0]),
					 XtWindow(threeDControl->moveButton[index][0]),
					 dirBits[6],
					 north_height,
					 north_width,
					 fore, back,
					 depth);
 
    armedPixmap = XCreatePixmapFromBitmapData(
					      XtDisplay(threeDControl->moveButton[index][0]),
					      XtWindow(threeDControl->moveButton[index][0]),
					      dirBits[6],
					      north_height,
					      north_width,
					      fore, armColor,
					      depth);

    XtVaSetValues(threeDControl->moveButton[index][0],
		  XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap, pixmap,
		  XmNarmPixmap, armedPixmap,
		  NULL);

    pixmap = XCreatePixmapFromBitmapData(XtDisplay(threeDControl->moveButton[index][1]),
					 XtWindow(threeDControl->moveButton[index][1]),
					 dirBits[2],
					 north_height,
					 north_width,
					 fore, back,
					 depth);
 
    armedPixmap = XCreatePixmapFromBitmapData(
					      XtDisplay(threeDControl->moveButton[index][1]),
					      XtWindow(threeDControl->moveButton[index][1]),
					      dirBits[2],
					      north_height,
					      north_width,
					      fore, armColor,
					      depth);

    XtVaSetValues(threeDControl->moveButton[index][1],
		  XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap, pixmap,
		  XmNarmPixmap, armedPixmap,
		  NULL);
}

/*
 * Synopsis: Boolean ThreeDControlIsManaged(ThreeDControl * threeDControl)
 *
 * Description: Determines whether or not the widget is managed/visible.
 *
 * Externals: None.
 *
 * Returns: True or False.
 *
 * Author: Lee Iverson
 * 
 * Date: Feb 1998
 */
Boolean
ThreeDControlIsManaged (ThreeDControl const * threeDControl)
{
  return XtIsManaged (threeDControl->mainForm);
}

/*
 * Synopsis: void ThreeDControlManageWidget()
 *	
 *
 * Description: The first time this widget gets managed,
 *    we need to create all the pixmaps
 *
 * Externals: 
 *
 *
 * Returns: NOTHING
 *
 * Author:
 *		Stephen Lau
 * 
 * Date: July 1994
 *
 */
void
ThreeDControlManageWidget(ThreeDControl * threeDControl)
{
/*  static char const funcName[] = "ThreeDControlManageWidget";*/
  static int pixmapFlag = FALSE;
  int i;

  XtManageChild(threeDControl->mainForm);

  if (! pixmapFlag) {
    for (i = 0; i < 3; i++)
      ThreeDControlCreatePixmaps(threeDControl, i);
    ThreeDControlCreateLeftRightPixmaps(threeDControl, moveHeading);
    ThreeDControlCreatePixmaps(threeDControl, movePitch);
    ThreeDControlCreateLeftRightPixmaps(threeDControl, moveRoll);
    ThreeDControlCreateForeBackPixmaps(threeDControl, moveForeBack);
    ThreeDControlCreateLeftRightPixmaps(threeDControl, moveLeftRight);
    ThreeDControlCreatePixmaps(threeDControl, moveUpDown);
    pixmapFlag = TRUE;
  }
}

/*
 * Synopsis: void ThreeDControlUnmanageWidget()
 *	
 * Description: Remove the widget from the display.
 *
 * Externals: 
 *
 * Returns: NOTHING
 *
 * Author: Lee Iverson
 * 
 * Date: July 1994
 *
 */
void
ThreeDControlUnmanageWidget(ThreeDControl * threeDControl)
{
/*  static char const funcName[] = "ThreeDControlUnmanageWidget";*/

  if (ThreeDControlIsManaged (threeDControl))
    XtUnmanageChild(threeDControl->mainForm);
}

/*
 * Synopsis: void ThreeDControlMove(XtPointer, XtIntervalId *)
 *
 *
 * Description: The zoom function is actually a timer that goes off at a
 *	set time. The window coordinate system is decreased or increased by
 *	10% and the timer is reset. The timer is cleared when the button is
 *	released, but that is done external to this callback.
 *
 * Externals:
 *
 * Returns: NULL
 *
 * Author:
 *      Stephen Lau
 *
 * Date: 1993
 *
 */
void
ThreeDControlMove(XtPointer      client_data,
		  XtIntervalId * id)
{ 
  static char * funcName = "ThreeDControlMove";
  ThreeDControl * threeDControl = (ThreeDControl *) client_data;
  tvBase *threeD = threeDControl->threeDWidget->threeD;
  XtAppContext appContext;
  tvViewpoint view_s, *view = &view_s;
  float offset;

  tvBaseGetView (threeD, view);
    
  if (tvGetBoolean (threeD, TV_HEIGHT_BASED_NAVIGATION)) {
    /* scale it with the height */
    offset = (threeDControl->offset / 5
	      * view->altitude / ACCEL_BASE_HEIGHT);
  }
  else
    offset = threeDControl->offset;

  switch(threeDControl->currButton[0])
  {
  case moveX:
    if (threeDControl->currButton[1])
      view->eye[0] -= offset;
    else
      view->eye[0] += offset;
    break;

  case moveY:
    if (threeDControl->currButton[1])
      view->eye[1] -= offset;
    else
      view->eye[1] += offset;
    break;

  case moveZ:
    if (threeDControl->currButton[1])
      view->eye[2] -= offset;
    else
      view->eye[2] += offset;
    break;

  case moveHeading:
    if (threeDControl->currButton[1])
      view->yaw -= M_PI / 180.0;
    else
      view->yaw += M_PI / 180.0;
    break;

  case movePitch:
    if (threeDControl->currButton[1])
      view->pitch -= M_PI / 180.0;
    else
      view->pitch += M_PI / 180.0;
    break;

  case moveRoll:
    if (threeDControl->currButton[1])
      view->roll += 10.0*M_PI/180.0;
    else
      view->roll -= 10.0*M_PI/180.0;
    break;

  case moveForeBack:
    if (threeDControl->currButton[1])
      offset = -offset;
    tvAdjustView (threeD, view, offset, 0, 0);
    break;

  case moveLeftRight:
    if (threeDControl->currButton[1])
      offset = -offset;
    tvAdjustView (threeD, view, 0, -offset, 0);
    break;

  case moveUpDown:
    if (threeDControl->currButton[1])
      offset = -offset;
    tvAdjustView (threeD, view, 0, 0, offset);
    break;

  default:
    tsmError (funcName, "Unknown button pressed!");
  }

  tvSetView (threeD, view, FALSE);

  /* RESET THE TIMER */
  appContext = XtWidgetToApplicationContext(threeDControl->mainForm);
  threeDControl->timerId = 
    XtAppAddTimeOut(appContext, 50, ThreeDControlMove, threeDControl);
}

/*
 * Synopsis: void ThreeDControlScaleChangedCB(Widget,
 *					  XtPointer,
 *					  Xtpointer)
 *
 * Description: When the user clicks and drags the sacle widget, this callback
 *      is called and the value is retrieved from the widget. The value is
 *      copied to the twoDWidget * for later use.
 *
 * Externals:
 *
 *
 * Returns: NULL
 *
 * Author:
 *		Stephen Lau
 *
 * Date: July 1994
 *
 */
static void
ThreeDControlScaleChangedCB(Widget    w,
			 XtPointer call_data,
			 XtPointer client_data)
{
/* static char * funcName = "ThreeDControlScaleChangedCB";*/
 ThreeDControl * threeDControl;
 int val;

    threeDControl = (ThreeDControl *) call_data;

    /* GET THE VALUE FROM THE WIDGET AND SAVE IT FOR LATER USE */
    XmScaleGetValue(w, &val);
 
    threeDControl->offset = ((float) val);
}
