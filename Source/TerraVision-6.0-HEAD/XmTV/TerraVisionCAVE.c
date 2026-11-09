/*
 * Name: TerraVisionCAVE.c
 *
 * Description: TerraVision CAVE code.
 *
 * Function List:
 *
 * Dependencies:
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

int numActiveWalls = 1;

static char const TerraVisionCAVE_rcsid[] = "$Id: TerraVisionCAVE.c,v 1.1.10.1 2002/10/31 05:26:49 reddy Exp $";

#ifdef CAVE

#include "TerraVisionInc.h"
#include "ThreeDWidgetI.h"
#include "ScannedMap.h"
#include "cave_ogl.h"

#include <X11/cursorfont.h>

#ifdef GATS
  #include "GATSDialogs.h"
#endif

extern ThreeDWidget *TerraVisionInitThreeDWidget(Boolean cave, Boolean mode3d);

extern Display * CAVEXDisplay(void);
extern Window CAVEXWindow(void);
extern XVisualInfo *CAVEXVisualInfo(void);
extern GLXContext CAVEGLXContext(void);

/* Globals from TerraVision.c/.h. */
extern int *scannedMapFlag;

int 	numProcs;
pid_t 	procPid[20];
char procNames[20][50];



extern void TerraVisionQuit(void);

static PRThread* generator_thread;

static ThreeDWidget *wallPtrs[CAVE_MAX_WALLS];


int TerraVisionCAVELoadData(TVDataSet *);
void TerraVisionCAVELoop(void);
void TerraVisionCAVEInitApp(void);
void ThreeDWidgetDrawCAVE(void);


int
TerraVisionStartCAVE (void)
{
  const char *funcName = "TerraVisionStartCAVE";
  int i;

  TSMTRINC ((1, 1));
  TSMTRACE ((1, funcName, "Entering."));

  i = 0;
  TerraVisionInitCAVE(&i, NULL);

  /* Go into loop to process X events. */
  TerraVisionCAVELoop();

  TSMTRACE ((1, funcName, "Leaving."));

  /* BYE BYE */
  TerraVisionQuit();
  CAVEExit();
  TSMTRACE ((1, funcName, "Done."));
  TSMTRINC ((1, -1));

  return TRUE;
}


/*
 * Synopsis: void TerraVisionCAVEInitApp()
 *	
 *
 * Description: Does some TerraVision initializations after we can get window
 *     info from the CAVE.
 *
 * Externals:
 *
 *
 * Returns: NOTHING
 *
 * Author:
 *		Nat Bletter
 * 
 * Date: 11/20/98
 *
 */
void
TerraVisionCAVEInitApp(void)
{
  const char *funcName = "TerraVisionCAVEInitApp";
  ThreeDWidget *threeD;
  busyCursor = XCreateFontCursor(CAVEXDisplay(), XC_watch);
  
  TSMTRACE ((1, funcName, "Entering CAVEInitApp."));

  if ((threeD = TerraVisionInitThreeDWidget(True,True))) {
    TSMTRACE ((1, funcName, "Created ThreeDWidget."));
    wallPtrs[CAVEWall] = threeD;
    threeD->showFullscreen = TRUE;
  }
  else {
    tsmError (funcName, "Unable to create ThreeDWidget Cave Wall %s.", CAVEWallName(CAVEWall));
  }
    
  TSMTRACE ((1, funcName, "Leaving CAVEInitApp."));
}



/*
 * Synopsis: void ThreeDWidgetDrawCAVE()
 *	
 *
 * Description: Passes on to ThreeDWidgetDraw with the correct ThreeD ptr
 *
 * Externals:
 *
 *
 * Returns: NOTHING
 *
 * Author:
 *		Nat Bletter
 * 
 * Date: 3/17/99
 *
 */
void
ThreeDWidgetDrawCAVE(void)
{
  const char *funcName = "TerraVisionCAVEDraw";
  ThreeDWidget *threeD;

  TSMTRACE ((1, funcName, "Entering CAVEDraw."));

  if ((threeD = wallPtrs[CAVEWall])) {
    TSMTRACE ((1, funcName, "Running 3DDraw."));
    ThreeDWidgetDraw(threeD->glmw, (void *)threeD);
  }
  else
    tsmError(funcName, "Draw called for non-existent Cave Wall %s.", CAVEWallName(CAVEWall));

  TSMTRACE ((1, funcName, "Leaving CAVEDraw."));
}

/*
 * Synopsis: XtAppContext TerraVisionInitCAVE (int *argcp, char *argv[])
 *	
 *
 * Description: 
 *
 * Externals:
 *
 *
 * Returns: TRUE - on succcess
 *          FALSE - on failure
 *
 * Author:
 *		Nat Bletter 11/16/98
 * 
 */
XtAppContext
TerraVisionInitCAVE(int *argcp, char *argv[])
{
  const char *funcName = "TerraVisionInitCAVE";
  int i;

  TSMTRINC ((1, 1));
  TSMTRACE ((1, funcName, "Entering."));

  /* INITIALIZE CAVE VIEWING ANGLES */
  caveYaw = (float *) malloc(sizeof(float));
  *caveYaw = 0;
  cavePitch = (float *) malloc(sizeof(float));
  *cavePitch = M_PI_2;

  /* CREATE FLAG FOR VIEWING MAP */
  scannedMapFlag = (int *) malloc(sizeof(int));
  *scannedMapFlag = FALSE;

  /* INTIALIZE THE CAVE */
  CAVEConfigure (argcp, argv, NULL);
  numActiveWalls = CAVEConfig->ActiveWalls;

  /* Initialize the wallPtrs array */
  for(i = 0; i < CAVE_MAX_WALLS; i++) {
    wallPtrs[i] = NULL;
  }

  /* CAVE housekeeping. */
  CAVEInit();
  CAVEInitApplication(TerraVisionCAVEInitApp, 0);
  CAVEDisplay(ThreeDWidgetDrawCAVE, 0);
	
  TSMTRACE ((1, funcName, "Done."));
  TSMTRINC ((1, -1));

  return NULL;
  /*  return XtWidgetToApplicationContext (ThreeDWidgetMainForm ((ThreeDWidget *)list_value(list_first(widgetList))));*/
}



/*
 * Synopsis: int TerraVisionInitCAVEGraphics (int *argcp, char *argv[])
 *	
 *
 * Description: 
 *
 * Externals:
 *
 *
 * Returns: TRUE - on succcess
 *          FALSE - on failure
 *
 * Author:
 *		Yvan Leclerc
 * 
 */
XtAppContext
TerraVisionInitCAVEGraphics (int *argcp, char *argv[])
{
  const char *funcName = "TerraVisionInitCAVEGraphics";
  int i;

  TSMTRINC ((1, 1));
  TSMTRACE ((1, funcName, "Entering."));

  /* INITIALIZE CAVE VIEWING ANGLES */
  caveYaw = (float *) malloc(sizeof(float));
  *caveYaw = 0;
  cavePitch = (float *) malloc(sizeof(float));
  *cavePitch = M_PI_2;

  /* INTIALIZE THE CAVE */
  CAVEConfigure (argcp, argv, NULL);
  numActiveWalls = CAVEConfig->ActiveWalls;

  /* CREATE FLAG FOR VIEWING MAP */
  scannedMapFlag = (int *) malloc(sizeof(int));
  *scannedMapFlag = FALSE;

  /* Create multiple 3D widgets if necessary. */
  for(i = 0; i < numActiveWalls; i++) {
    ThreeDWidget *threeD;
    threeD = CreateThreeDWidget(applicationShell, True);
  }

  /* create SCANNED MAP WIDGET */
  scannedMap = CreateScannedMap(applicationShell);
  if(!scannedMap) {
    tsmError (funcName, "Unable to create the scanned map structure!");
    TSMTRINC ((1, -1));
    return (FALSE);
  }


  TSMTRACE ((1, funcName, "Done."));
  TSMTRINC ((1, -1));

  if (widgetList && ! list_empty (widgetList)) {
    ThreeDWidget *w = list_value (list_first (widgetList));
    return XtWidgetToApplicationContext (ThreeDWidgetMainForm (w));
  }

  return NULL;
}

int
TerraVisionSpawnCAVEProcs ()
{
  const char *funcName = "TerraVisionSpawnCAVEProcs";
  int i;
  ThreeDWidget *threeD;

  TSMTRINC ((1, 1));
  TSMTRACE ((1, funcName, "Entering."));

  /* We want a visibility process for each wall. */
  for (i = 0; i < CAVE_MAX_WALLS; i++) 
    if ((threeD = wallPtrs[i]) != NULL) {
      sprintf (procNames[numProcs], "Visibility Generator[%d]", i);
      procPid[numProcs] = pthr_create (&generator_thread, "Visiblity 3D", ThreeDWidgetVisibleThread, threeD);
      if (procPid[numProcs] < 0) {
	tsmError (funcName, "Unable to spawn the %s thread", 
		  procNames[numProcs]);
	TSMTRINC ((1, -1));
	return (FALSE);
      }
      TSMTRACE ((1, funcName, "PID for %s Thread = %d.", 
		 procNames[numProcs], procPid[numProcs]));
      numProcs++;
    }

#ifdef GATS
  /* We also want GATS. */
  sprintf (procNames[numProcs], "GATS Server");
  procPid[numProcs] = sproc (CAVEConnectToGATS, PR_SALL, NULL);
  if (procPid[numProcs] < 0) {
    tsmError (funcName, "Unable to spawn the %s thread", procNames[numProcs]);
    TSMTRINC ((1, -1));
    return (FALSE);
  }
  TSMTRACE ((1, funcName, "PID for %s Thread = %d.", 
	     procNames[numProcs], procPid[numProcs]));
  numProcs++;
#endif
  
  TSMTRACE ((1, funcName, "Done."));
  TSMTRINC ((1, -1));
  return (TRUE);
}

/*
 * Synopsis: int TerraVisionCAVELoadDAta()
 *	
 *
 * Description: 
 *     When using the CAVE, this function
 *     contacts the TSM and also reads from
 *     the configuration file which tile set
 *     to use. It's cheesy, because the user
 *     can't select it themselves without editing
 *     the configuration file
 *
 * Externals:
 *
 *
 * Returns: TRUE - on succcess
 *          FALSE - on failure
 *
 * Author:
 *		Stephen Lau
 * 
 * Date: 1995
 *
 */
int
TerraVisionCAVELoadData(TVDataSet *tsm)
{
  static char *funcName = "TerraVisionCAVELoadData";
  const char * config_val;

  TSMTRINC ((2, 1));
  TSMTRACE ((2, funcName, "Entering."));

  /* GET WHICH DATA SET WE SHOULD LOAD */
  if (!tvGetResourceAsString ("CAVE_GEO", &(tsm->name))) {
    return (FALSE);
  }

  if (!tvGetResourceAsString("CAVE_DEM", &config_val)) {
    return (FALSE);
  }
  tsm->dem_url = strdup(config_val);

  if (!tvGetResourceAsString("CAVE_DEM_SERVER", &config_val)) {
    return (FALSE);
  }
  tsmSetParam(tsm->dem_params, TSM_PARAM_SERVERS, config_val);

  if (! tvGetResourceAsString("CAVE_MOSAIC", &config_val)) {
    return (FALSE);
  }
  tsm->oi_url = strdup(config_val);

  if (! tvGetResourceAsString("CAVE_MOSAIC_SERVER", &config_val)) {
    return (FALSE);
  }
  tsmSetParam(tsm->oi_params, TSM_PARAM_SERVERS, config_val);

  TSMTRACE ((1, funcName, "Loading DEMS."));
  if(!TVDataSetConnect(tsm)) {
    tsmError (funcName, " Failed to initialize data sets.");
    return (FALSE);
  }

  /* MAKE SURE WE REFLECT THE CHANGE */
  /*ThreeDWidgetChangeDataSet(threeD, geo);*/
  TSMTRACE ((2, funcName, "Done."));
  TSMTRINC ((2, -2));
  return (TRUE);
}

/*
 * Synopsis: TerraVisionCAVELoop(void)
 *	
 * Description: 
 *     When using the CAVE, this function 
 *     loops untilt he user quits.
 *     It process keyboard, wand, mouse
 *     events so user can fly around
 *
 * Externals:
 *
 *
 * Returns: NONE
 *
 * Author:
 *		Stephen Lau
 * 
 * Date: 1996
 *
 */
void
TerraVisionCAVELoop(void)
{
  const char *funcName = "TerraVisionCAVELoop";
  float jx, jy;
  float wandFront[3];
  float t, dt;
  float prevTime = 0;
  float orient[3];
#define SPEED 1000.0
#define ACCEL 1.0
  int i;
  int exitFlag = 0;
  int btnState;
  double mouseX, mouseY;
  float vel = 0.0;
  View * view;
  float dYaw, dPitch;
  typedef enum {
    NONE = 0,
    ACCELERATE,
    DECCELERATE,
    CRUISE,
    SITANDSPIN
  }FlyMode;
  FlyMode flying = NONE;
  short dev, val;
  ThreeDWidget *threeD;

#define DoAllCAVEWidgets(opr) { \
    int iter; \
    ThreeDWidget *threeD; \
    for (iter = 0; iter < CAVE_MAX_WALLS; iter ++) \
      if (threeD = wallPtrs[iter]) { \
	opr; \
      } \
  } 
  
  TSMTRINC ((1, 1));
  TSMTRACE ((1, funcName, "Entering."));
  
  DoAllCAVEWidgets(ThreeDWidgetCAVEBtnDown(threeD));
  while(!CAVEgetbutton(CAVE_ESCKEY) && !exitFlag){
    if(CAVEgetbutton(CAVE_WKEY))
	{
	    while(CAVEgetbutton(CAVE_WKEY));
	    DoAllCAVEWidgets(threeD->threeDPanel->showWireframe = !(threeD->threeDPanel->showWireframe));
	}
	if(CAVEgetbutton(CAVE_AKEY))
	{
	    while(CAVEgetbutton(CAVE_AKEY));
	    DoAllCAVEWidgets(threeD->threeDPanel->airplaneControl = (threeD->threeDPanel->airplaneControl));
        }
	if(CAVEgetbutton(CAVE_PAGEUPKEY))
	{
	    while(CAVEgetbutton(CAVE_PAGEUPKEY));
    	    DoAllCAVEWidgets(ThreeDWidgetCAVEChangeTimeOfDay(threeD, .5));
	}
	if(CAVEgetbutton(CAVE_INSERTKEY))
	{
	    while(CAVEgetbutton(CAVE_INSERTKEY));
	    DoAllCAVEWidgets(threeD->fog_params[0] += .00005;
	    if(threeD->fog_params[0] > 1)
	    {
		threeD->fog_params[0] = 1;
	    }
	    );
	}
	if(CAVEgetbutton(CAVE_DELKEY))
	{
	    while(CAVEgetbutton(CAVE_DELKEY));
	    DoAllCAVEWidgets(threeD->fog_params[0] -= .00005;
	    if(threeD->fog_params[0] < 0)
	    {
		threeD->fog_params[0] = 0;
	    }
	    );
	}
	if(CAVEgetbutton(CAVE_MKEY))
	{
	    while(CAVEgetbutton(CAVE_MKEY));
	    if(*scannedMapFlag)
	    {
		*scannedMapFlag = FALSE;
	    }
	    else
	    {
		*scannedMapFlag = TRUE;
	    }
        }
	if(CAVEgetbutton(CAVE_PAGEDOWNKEY))
	{
	    while(CAVEgetbutton(CAVE_PAGEDOWNKEY));
	    DoAllCAVEWidgets(ThreeDWidgetCAVEChangeTimeOfDay(threeD, -.5));
	}
	if(CAVEgetbutton(CAVE_HOMEKEY))
        {
	    while(CAVEgetbutton(CAVE_HOMEKEY));
	    CAVENavLoadIdentity();
	    *caveYaw = 0;
	    *cavePitch = M_PI_2;
  	    vel = 0;
  	    dYaw = 0;
	    dPitch = 0;
	    flying = NONE;
	    DoAllCAVEWidgets(ThreeDWidgetInitView(threeD));
#ifdef DEBUG
	    tsmStatus("%s: Home Key pressed", funcName);
#endif
        }
        if(CAVEgetbutton(CAVE_VKEY))
        {
	   while(CAVEgetbutton(CAVE_VKEY));
	   DoAllCAVEWidgets(threeD->threeDPanel->showVehicles = !threeD->threeDPanel->showVehicles);
        }
        if(CAVEgetbutton(CAVE_BKEY))
        {
	   while(CAVEgetbutton(CAVE_BKEY));
	   DoAllCAVEWidgets(threeD->threeDPanel->showModels = !threeD->threeDPanel->showModels);
        }
#ifdef GATS
        if(CAVEgetbutton(CAVE_GKEY))
        {
	   while(CAVEgetbutton(CAVE_GKEY));
           gatsServer.currentLog = -1;
           gatsServer.currentLog = 6;
        }
#endif
	t = CAVEGetTime();
	dt = t - prevTime;
	prevTime = t;
#ifdef DEBUG
	/* THIS IS FOR USING THE WAND IN THE CAVE */
	jx = CAVE_JOYSTICK_X;
	jy = CAVE_JOYSTICK_Y;
	if(fabs(jy) > 0.3)
	{
	    CAVEGetOrientation(CAVE_WAND, orient);
#ifdef DEBUG
	    tsmStatus("%s: %f %f %f\n", 
		      funcName,
		      orient[0],
		      orient[1],
		      orient[2]);
#endif
	    if(orient[0] >= 0.0)
	    {
		orient[0] = 90.0 - orient[0];
	    }
	    else
	    {
		orient[0] = 90.0 + fabs(orient[0]);
	    }
	    for(i = 0; i < 3; i++)
	    {
		orient[i] *= M_PI / 180.0;
	    }
	    orient[1] += *caveYaw;

	    for(i = 0; i < CAVE_MAX_WALLS; i++)
	      if (threeD = wallPtrs[i]) {
		threeD->view.eye[0] += sin(orient[0]) * cos(orient[1]) *
		jy*SPEED*dt,
		threeD->view.eye[1] += cos(orient[0]) * jy*SPEED*dt,
		threeD->view.eye[2] += sin(orient[0]) * sin(orient[1]) *
		  jy*SPEED*dt;
	      }
	}
	else
	{
	    if(fabs(jx) > 0.1)
	    {
		*caveYaw -= (jx * 30.0 * dt) * M_PI / 180.0;
		CAVENavRot(-(jx * 30.0 * dt), 'y');
	    }
	}
	DoAllCAVEWidgets(ThreeDWidgetCalcView(threeD, &threeD->view[i], FALSE));
#endif
        switch(flying)
        {
	    case ACCELERATE:
#ifdef DEBUG
		tsmStatus("%s: Accelerate", funcName);
#endif
    		/* INCREASE THE VELOCITY BASED ON ACCELERATION */
    		vel += ACCEL;
	        break;

	    case DECCELERATE:
		/* DECCELERATE BASED UPON THE ACCELERATION VALUE */
#ifdef DEBUG
		tsmStatus("%s: deccelerate", funcName);
#endif
    		vel -= ACCEL;
	        break;

	    default:
	        break;
        }
        if(flying)
        {
	  threeD = NULL;
	  for (i = 0; i < CAVE_MAX_WALLS; i++)
	    if (threeD = wallPtrs[i]) {
	      break;
	    }
	  if (!threeD) {
	    tsmError(funcName, "Couldn't find front CAVE wall!");
	    return;
	  }

            /* find distance from point of mouse down */
            mouseX = (double) CAVEgetvaluator(CAVE_MOUSEX) - threeD->xMouseDown;
            mouseY = (double) CAVEgetvaluator(CAVE_MOUSEY) - threeD->yMouseDown;
            mouseX *= -1;

    	    if (!threeD->threeDPanel->airplaneControl)
	    {
       		mouseY *= -1;
	    }
            dYaw = (M_PI / 180.0) * mouseX / ((float) threeD->width / 2.0);
            *caveYaw += dYaw;
	    dPitch = (M_PI / 180.0) * mouseY / ((float) threeD->height / 2.0);
	    *cavePitch += dPitch;

	    /* GET THE NEW VIEWPOINT FOR ALL RENDERERS */
	    for(i = 0; i < CAVE_MAX_WALLS; i++)
	      if (wallPtrs[i])
		{
		  view = &wallPtrs[i]->view;

		  /* MOVE THE VIEWER FORWARD OR BACKWARD BASED ON VELOCITY */
		  view->eye[0] += dt * vel * sin(*cavePitch) * cos(*caveYaw);
		  view->eye[1] += dt * vel * cos(*cavePitch);
		  view->eye[2] += dt * vel * sin(*cavePitch) * sin(*caveYaw);

		  /* MAKE SURE WE STAY ABOVE GROUND */
		  ThreeDWidgetConstrainView (wallPtrs[i], view);

		  /* GET WHERE WE ARE LOOKING AT */
		  view->view[0] = view->eye[0] + 
		    500.0 * sin(*cavePitch) * cos(*caveYaw);
		  view->view[1] = view->eye[1] + 
		    500.0 * cos(*cavePitch);
		  view->view[2] = view->eye[2] + 
		    500.0 * sin(*cavePitch) * sin(*caveYaw);
		}
#ifdef DEBUG
	    tsmStatus("%s: Theta = %f Phi = %f", funcName,
		      *caveYaw * (180.0 / M_PI), *cavePitch * (180.0 / M_PI));
	    ThreeDWidgetDumpView(threeD, &threeD->view);
#endif

	    /* SET THE NEW VIEWPOINT */
	    CAVENavLoadIdentity();
    	    CAVENavRot((180.0 / M_PI) * *caveYaw, 'y');
            CAVENavRot((180.0 / M_PI) * (*cavePitch - M_PI_2), 'x');
        }

        /* MOUSE BUTTON TOGGLES */
        btnState = CAVEButtonChange(1);
        if(btnState == 1)
        {
	    DoAllCAVEWidgets(ThreeDWidgetCAVEBtnDown(threeD));
#ifdef DEBUG
	    tsmStatus("%s: 1 Down!", funcName);
#endif
	    flying = ACCELERATE;
        }
        if(btnState == -1)
        {
    	    flying = CRUISE;
        }
        btnState = CAVEButtonChange(2);
        if(btnState == 1)
        {
	    DoAllCAVEWidgets(ThreeDWidgetCAVEBtnDown(threeD));
	    flying = SITANDSPIN;
    	    vel = 0;
        }
        if(btnState == -1)
        {
	    flying = NONE;
    	    DoAllCAVEWidgets(threeD->MouseDown = FALSE);
    	    vel = 0;
        }
        btnState = CAVEButtonChange(3);
        if(btnState == 1)
        {
	    DoAllCAVEWidgets(ThreeDWidgetCAVEBtnDown(threeD));
    	    flying = DECCELERATE;
        }
        if(btnState == -1 && flying == DECCELERATE)
        {
	    flying = CRUISE;
        }
        sginap(1);
    }

  TSMTRACE ((1, funcName, "Done."));
  TSMTRINC ((1, -1));

    /* BYE BYE CAVE */
    if(CAVEDistribMaster)
    {
      /*        CAVEDistribClose(); Don't worry about this for now since we don't open and distributed connections */
    }
}

#endif /* CAVE */
