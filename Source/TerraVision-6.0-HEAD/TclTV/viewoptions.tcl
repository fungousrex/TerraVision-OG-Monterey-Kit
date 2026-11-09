#
# viewoptions.tcl
#

source viewController.tcl


namespace eval ViewControls {

    namespace export postViewControlsDialog postViewStatusDialog \
                     postGotoDialog postGazetteerDialog postADLDialog

    #
    # module globals...
    #
    variable controllerTitle    ;# suffix of the title widget
    variable controllerCanvas   ;#   "    "   "  canvas  "
    variable controllerScale    ;#   "    "   "  scale   "
    variable labels
    set labels(XYZ) "XYZ Controller"
    set labels(FLU) "F/B L/R U/D Controller"
    set labels(HPR) "Heading/Pitch/Roll Controller"
    variable deltas
    set deltaLabels(XYZ) "delta factor (units)"
    set deltaLabels(FLU) "delta factor (units)"
    set deltaLabels(HPR) "delta factor (degrees)"

    # instance vars...
    variable viewState
    variable stepSize
    variable timers

    # goto Lat/Long vars...
    variable longitudeField
    variable latitudeField
    variable elevationField
    variable yawField
    variable pitchField
    variable rollField

    # goto address vars...
    variable streetField
    variable cityField
    variable provinceField
    variable postcodeField
    variable countryField
    variable heightOverCityField
    variable gazInfoField
    variable gazGotoField

    # ADL's goto feature vars..
    variable adlNameField
    variable adlTypeField
    variable adlHeightField
    variable adlInfoField
    variable adlGotoField

    #
    # m a k e C o n t r o l s W i n d o w
    #
    proc makeControlsWindow {inst} {
	variable labels
	variable deltaLabels
	variable controllerTitle
	variable controllerCanvas
	variable controllerScale
	upvar ViewControls::viewState($inst) viewState
	upvar ViewControls::stepSize($inst)  stepSize

	# create window name for dialog, test if already exists...
	set topname [format "%s_vod" $inst]
	if {[winfo exists $topname] == 1} {
	    wm withdraw $topname     ;# a little dance to force it to top
	    wm deiconify $topname 
	    return
	}

	set top [toplevel $topname]
	wm title $top [format "View Controllers: Viewer %d" [instnum $inst]]

	# topmost frame...

	frame $top.f1 -relief raised -borderwidth 2

	label $top.f1.l -text "Set view-point relative to:"
	radiobutton $top.f1.b1 -text "viewer position" -padx 10 \
	    -command "ViewControls::checkboxCB $inst $top.f1.b1" \
	    -variable ViewControls::viewState($inst) -value "FLU"
	$top.f1.b1 deselect
	radiobutton $top.f1.b2 -text "global coordinate system" -padx 10 \
	    -command "ViewControls::checkboxCB $inst $top.f1.b2" \
	    -variable ViewControls::viewState($inst) -value "XYZ"
	$top.f1.b2 deselect
	radiobutton $top.f1.b3 -text "heading/pitch/roll axes" -padx 10 \
	    -command "ViewControls::checkboxCB $inst $top.f1.b3" \
	    -variable ViewControls::viewState($inst) -value "HPR"
	$top.f1.b3 deselect

	grid $top.f1.l -sticky w
	grid $top.f1.b1 -sticky w
	grid $top.f1.b2 -sticky w
	grid $top.f1.b3 -sticky w

	# second frame; the action area...
	frame $top.f2 -relief raised -borderwidth 2

	set controllerTitle f2.l
	label $top.$controllerTitle -text $labels(XYZ)

	set controllerCanvas f2.c
	canvas $top.$controllerCanvas -width 150 -height 150

	set controllerScale f2.s
	scale $top.$controllerScale \
	    -orient horizontal -from 0 -to 100 -resolution 1 \
	    -showvalue 1 -length 150
	set stepSize 10
	$top.$controllerScale set $stepSize
	    

	# pack frame 2 together...
	pack $top.$controllerTitle -side top -expand 1 -fill y  -anchor w
	pack $top.$controllerCanvas -side top -expand 1 -fill both
	pack $top.$controllerScale -side top -expand 1 -fill both
	
	# dismiss button...
	button $top.dismiss -text "Dismiss" \
	    -command "ViewControls::dismissCB $inst"

	# pack it all together...
	pack $top.dismiss -side bottom -expand 1 -fill x
	pack $top.f1 $top.f2 -side left -expand 1 -fill both 

	# preset things to the viewer position controller...
	set viewState "FLU"
	checkboxCB $inst $top.f1.b1
    }

    #
    # m a k e V i e w S t a t u s W i n d o w
    #
    # Creates the ViewStatus window depicting details of the current 
    # viewpoint.
    #
    proc makeViewStatusWindow {inst} {
	variable timers

	# create window name for dialog, test if already exists...
	set topname [format "%s_vos" $inst]
	if {[winfo exists $topname] == 1} {
	    wm withdraw $topname     ;# a little dance to force it to top
	    wm deiconify $topname 
	    return
	}

	set top [toplevel $topname]
	wm title $top [format "View Information (Geocentric):  Viewer %d" \
			   [instnum $inst]]

	# left side shows viewpoint in XYZ
	frame $top.f1 -relief raised -borderwidth 2

	label $top.f1.l1_title -text "X position: " \
	    -justify right -padx 5 -pady 5 
	label $top.f1.l1_value -background black -foreground green \
	    -relief sunken -borderwidth 2 -justify right -width 12
	grid $top.f1.l1_title $top.f1.l1_value -sticky we

	label $top.f1.l2_title -text "Y position: " \
	    -justify right -padx 5 -pady 5
	label $top.f1.l2_value -background black -foreground green \
	    -relief sunken -borderwidth 2 -justify right -width 12
	grid $top.f1.l2_title $top.f1.l2_value -sticky we

	label $top.f1.l3_title -text "Z position: " \
	    -justify right -padx 5 -pady 5
	label $top.f1.l3_value -background black -foreground green \
	    -relief sunken -borderwidth 2 -justify right -width 12
	grid $top.f1.l3_title $top.f1.l3_value -sticky we

	# HPR frame...
	frame $top.f2 -relief raised -borderwidth 2
	label $top.f2.l1_title -text "Heading: " \
	    -justify right -padx 5 -pady 5 
	label $top.f2.l1_value -background black -foreground green \
	    -relief sunken -borderwidth 2 -justify right -width 12
	grid $top.f2.l1_title $top.f2.l1_value -sticky we

	label $top.f2.l2_title -text "Pitch: " \
	    -justify right -padx 5 -pady 5
	label $top.f2.l2_value -background black -foreground green \
	    -relief sunken -borderwidth 2 -justify right -width 12
	grid $top.f2.l2_title $top.f2.l2_value -sticky we

	label $top.f2.l3_title -text "Roll: " \
	    -justify right -padx 5 -pady 5
	label $top.f2.l3_value -background black -foreground green \
	    -relief sunken -borderwidth 2 -justify right -width 12
	grid $top.f2.l3_title $top.f2.l3_value -sticky we

	button $top.dismiss -text "Dismiss" \
	    -command "ViewControls::dismissVSCB $inst"

	pack $top.dismiss -side bottom -expand 1 -fill x
	pack $top.f1 $top.f2 -side left -ipadx 10 -expand 1 -fill both

	# set up a timer to periodically update this window...
	set timers($inst) [after 50 "ViewControls::updateTimerProc $inst"]
    }

    #
    # m a k e G o t o W i n d o w
    #
    # Creates the Goto window allowing the user to go to a 
    # specified lat/long location.
    #
    proc makeGotoWindow {inst} {
	upvar ViewControls::longitudeField($inst) longitudeField
	upvar ViewControls::latitudeField($inst) latitudeField
	upvar ViewControls::elevationField($inst) elevationField
	upvar ViewControls::yawField($inst) yawField
	upvar ViewControls::pitchField($inst) pitchField
	upvar ViewControls::rollField($inst) rollField

	# create window name for dialog, test if already exists...
	set topname [format "%s_goto" $inst]
	if {[winfo exists $topname] == 1} {
	    wm withdraw $topname     ;# a little dance to force it to top
	    wm deiconify $topname 
	    return
	}

	set top [toplevel $topname]
	wm title $top [format "Goto Lat/Long:  Viewer %d" [instnum $inst]]

	# left side shows viewpoint in XYZ
	frame $top.f1 -relief raised -borderwidth 2
	frame $top.f1.input
	set frm $top.f1.input

	label $frm.l1_title -text "Longitude (deg): " \
	    -justify right -padx 5 -pady 5 
	entry $frm.l1_value -width 12
	set longitudeField $frm.l1_value
	grid $frm.l1_title $frm.l1_value -sticky we

	label $frm.l2_title -text "Latitude (deg): " \
	    -justify right -padx 5 -pady 5
	entry $frm.l2_value -width 12
	set latitudeField $frm.l2_value
	grid $frm.l2_title $frm.l2_value -sticky we

	label $frm.l3_title -text "Elevation (m): " \
	    -justify right -padx 5 -pady 5
	entry $frm.l3_value -width 12
	set elevationField $frm.l3_value
	grid $frm.l3_title $frm.l3_value -sticky we

	button $top.f1.gotoloc -text "Goto Lat/Long" \
	    -command "ViewControls::gotoLatLongCB $inst"

	grid $frm -row 0
	grid $top.f1.gotoloc -row 1 -sticky ew -pady 5

	# HPR frame...
	frame $top.f2 -relief raised -borderwidth 2
	frame $top.f2.input
	set frm $top.f2.input

	label $frm.l1_title -text "Heading (deg): " \
	    -justify right -padx 5 -pady 5 
	entry $frm.l1_value -width 12
	set yawField $frm.l1_value
	grid $frm.l1_title $frm.l1_value -sticky we

	label $frm.l2_title -text "Pitch (deg): " \
	    -justify right -padx 5 -pady 5
	entry $frm.l2_value -width 12
	set pitchField $frm.l2_value
	grid $frm.l2_title $frm.l2_value -sticky we

	label $frm.l3_title -text "Roll (deg): " \
	    -justify right -padx 5 -pady 5
	entry $frm.l3_value -width 12
	set rollField $frm.l3_value
	grid $frm.l3_title $frm.l3_value -sticky we

	button $top.f2.setorient -text "Set Orientation" \
	    -command "ViewControls::setOrientCB $inst" 
	grid $frm -row 0
	grid $top.f2.setorient -row 1 -sticky ew -pady 5

	button $top.dismiss -text "Dismiss" \
	    -command "ViewControls::dismissGotoCB $inst"

	pack $top.dismiss -side bottom -expand 1 -fill x
	pack $top.f1 $top.f2 -side left -ipadx 10 -expand 1 -fill both
    }

    #
    # c o n f i g G a z B u t t o n s
    #
    #
    proc configGazButtons {inst enabled} {
	upvar ViewControls::gazInfoField($inst) gazInfoField
	upvar ViewControls::gazGotoField($inst) gazGotoField

	set win [format "%s_gaz" $inst]

	if { $enabled } {
	    $gazInfoField configure -text \
		"Enter address of city to jump to that location..."
	    $gazGotoField configure -state normal
	    #$win configure -cursor arrow
	    $win configure -cursor [$inst cget -cursor]
	} else {
	    $gazInfoField configure -text \
		"Looking up address in gazetteer..."
#	    $gazInfoField configure -text \
\#		"Looking up address from whereonearth.com..."
	    $gazGotoField configure -state disabled
	    $win configure -cursor watch
	    update
	}
	
    }
	
    #
    # m a k e G a z e t t e e r W i n d o w
    #
    # Creates the Goto Address window allowing the user to enter a
    # street address and then jump to that location. Uses the tvutils
    # Gazetteer service to translate address into lat/long.
    #
    proc makeGazetteerWindow {inst} {
	upvar ViewControls::streetField($inst) streetField
	upvar ViewControls::cityField($inst) cityField
	upvar ViewControls::provinceField($inst) provinceField
	upvar ViewControls::postcodeField($inst) postcodeField
	upvar ViewControls::countryField($inst) countryField
	upvar ViewControls::heightOverCityField($inst) heightOverCityField
	upvar ViewControls::gazInfoField($inst) gazInfoField
	upvar ViewControls::gazGotoField($inst) gazGotoField

	# create window name for dialog, test if already exists...
	set topname [format "%s_gaz" $inst]
	if {[winfo exists $topname] == 1} {
	    wm withdraw $topname     ;# a little dance to force it to top
	    wm deiconify $topname 
	    return
	}

	set top [toplevel $topname]
	wm title $top [format "Goto Address:  Viewer %d" [instnum $inst]]

	# left side shows viewpoint in XYZ

	frame $top.f1 -relief raised -borderwidth 2
	frame $top.f1.input
	set frm $top.f1

	label $frm.l0_title -text "Street: " \
	    -justify right -padx 5 -pady 5 
	entry $frm.l0_value -width 30
	set streetField $frm.l0_value
	grid $frm.l0_title $frm.l0_value -sticky we

	label $frm.l1_title -text "City Name: " \
	    -justify right -padx 5 -pady 5 
	entry $frm.l1_value -width 30
	set cityField $frm.l1_value
	grid $frm.l1_title $frm.l1_value -sticky we

	label $frm.l2_title -text "State/Province: " \
	    -justify right -padx 5 -pady 5
	entry $frm.l2_value -width 30
	set provinceField $frm.l2_value
	grid $frm.l2_title $frm.l2_value -sticky we

	label $frm.l3_title -text "Zip/Postcode: " \
	    -justify right -padx 5 -pady 5
	entry $frm.l3_value -width 30
	set postcodeField $frm.l3_value
	grid $frm.l3_title $frm.l3_value -sticky we

	label $frm.l4_title -text "Country: " \
	    -justify right -padx 5 -pady 5
	entry $frm.l4_value -width 30
	set countryField $frm.l4_value
	grid $frm.l4_title $frm.l4_value -sticky we
	$frm.l4_value insert 0 "US"

	label $frm.l5_title -text "Height Above City (km): " \
	    -justify right -padx 5 -pady 5
	entry $frm.l5_value -width 30
	set heightOverCityField $frm.l5_value
	grid $frm.l5_title $frm.l5_value -sticky we
	$frm.l5_value insert 0 "100"

	pack $frm -ipadx 10 -ipady 4

	frame $top.info -relief raised -borderwidth 2
	label $top.info.text -justify left -background grey
	set gazInfoField $top.info.text
	pack $top.info.text -expand 1 -fill x -side left
	pack $top.info -side top -fill x -expand 1

	frame $top.buttons
	button $top.buttons.goto -text "Goto Address" \
	    -command "ViewControls::gotoAddressCB $inst"

	button $top.buttons.dismiss -text "Dismiss" \
	    -command "ViewControls::dismissGazCB $inst"

	set gazGotoField $top.buttons.goto

	pack $top.buttons.goto -side left -expand 1 -fill x
	pack $top.buttons.dismiss -side left -expand 1 -fill x
	pack $top.buttons -side bottom -expand 1 -fill x

	configGazButtons $inst 1

#	pack $top.dismiss -side bottom -expand 1 -fill x
#	pack $top.f1 -side left -ipadx 10 -expand 1 -fill both
    }
	
    #
    # c o n f i g A D L B u t t o n s
    #
    #
    proc configADLButtons {inst enabled} {
	upvar ViewControls::adlInfoField($inst) adlInfoField
	upvar ViewControls::adlGotoField($inst) adlGotoField

	set win [format "%s_adl" $inst]

	if { $enabled } {
	    $adlInfoField configure -text \
		"Enter a feature name or place of interest..."
	    $adlGotoField configure -state normal
	    $win configure -cursor [$inst cget -cursor]
	} else {
	    $adlInfoField configure -text \
		"Looking up address in ADL gazetteer..."
	    $adlGotoField configure -state disabled
	    $win configure -cursor watch
	    update
	}
	
    }
	
    #
    # m a k e G a z e t t e e r W i n d o w
    #
    # Creates the Goto Address window allowing the user to enter a
    # street address and then jump to that location. Uses the tvutils
    # ADL Gazetteer service to translate address into lat/long.
    #
    proc makeADLWindow {inst} {
	upvar ViewControls::adlNameField($inst)   adlNameField
	upvar ViewControls::adlInfoField($inst)   adlInfoField
	upvar ViewControls::adlGotoField($inst)   adlGotoField
	upvar ViewControls::adlHeightField($inst) adlHeightField
	global adlTypeField

	# create window name for dialog, test if already exists...
	set topname [format "%s_adl" $inst]
	if {[winfo exists $topname] == 1} {
	    wm withdraw $topname     ;# a little dance to force it to top
	    wm deiconify $topname 
	    return
	}

	set top [toplevel $topname]
	wm title $top [format "Goto Feature:  Viewer %d" [instnum $inst]]

	# left side shows viewpoint in XYZ

	frame $top.f1 -relief raised -borderwidth 2
	frame $top.f1.input
	set frm $top.f1

	label $frm.l0_title -text "Feature Name: " \
	    -justify right -padx 5 -pady 5 
	entry $frm.l0_value -width 30
	set adlNameField $frm.l0_value
	grid $frm.l0_title $frm.l0_value -sticky we

	label $frm.l5_title -text "Height Above Feature (km): " \
	    -justify right -padx 5 -pady 5
	entry $frm.l5_value -width 30
	set adlHeightField $frm.l5_value
	grid $frm.l5_title $frm.l5_value -sticky we
	$frm.l5_value insert 0 "100"

	label $frm.l4_title -text "Feature Type: " \
	    -justify right -padx 5 -pady 5
	tk_optionMenu $frm.l4_value adlTypeField "all" \
	    "transporation features" "beaches" "buildings" \
	    "deserts" "forests" "historical sites" "lakes" \
	    "military areas" "monuments" "mountains" "parks" \
	    "recreational facilities" "streams" "valleys" \
	    "volcanic features"
	grid $frm.l4_title $frm.l4_value -sticky we

	pack $frm -ipadx 10 -ipady 4

	frame $top.info -relief raised -borderwidth 2
	label $top.info.text -justify left -background grey
	set adlInfoField $top.info.text
	pack $top.info.text -expand 1 -fill x -side left
	pack $top.info -side top -fill x -expand 1

	frame $top.buttons
	button $top.buttons.goto -text "Goto Feature" \
	    -command "ViewControls::gotoFeatureCB $inst"

	button $top.buttons.dismiss -text "Dismiss" \
	    -command "ViewControls::dismissADLCB $inst"

	set adlGotoField $top.buttons.goto

	pack $top.buttons.goto -side left -expand 1 -fill x
	pack $top.buttons.dismiss -side left -expand 1 -fill x
	pack $top.buttons -side bottom -expand 1 -fill x

	configADLButtons $inst 1
    }
	
    proc postViewControlsDialog {inst} {
	makeControlsWindow $inst
    }

    proc postViewStatusDialog {inst} {
	makeViewStatusWindow $inst
    }

    proc postGotoDialog {inst} {
	makeGotoWindow $inst
    }

    proc postGazetteerDialog {inst} {
	makeGazetteerWindow $inst
    }

    proc postADLDialog {inst} {
	makeADLWindow $inst
    }

    ###############################
    ## Callbacks...              ##
    ###############################

    # 
    # c h e c k b o x C B
    #
    # Callback invoked when the radiobuttons change.  Installs the
    # appropriate view-controller in the canvas area of the GUI.
    #
    proc checkboxCB {inst btn} {
	variable labels
	variable deltaLabels
	variable controllerTitle
	variable controllerCanvas
	variable controllerScale
	upvar ViewControls::viewState($inst) viewState

	set index [$btn cget -value]
	set top [format "%s_vod" $inst]
	$top.$controllerTitle configure -text $labels($index)
	
	$top.$controllerCanvas delete all
	if {$index == "XYZ"} {
	    ViewController::makeXYZController $top.$controllerCanvas 75 75 \
		"ViewControls::xyzCB $inst"
	    $top.$controllerScale configure -label $deltaLabels($index)
	} elseif {$index == "FLU"} {
	    ViewController::makeFBLRUDController $top.$controllerCanvas 75 75 \
		"ViewControls::fluCB $inst"
	    $top.$controllerScale configure -label $deltaLabels($index)
	} else {
	    ViewController::makeHPRController $top.$controllerCanvas 75 75 \
		"ViewControls::hprCB $inst"
	    $top.$controllerScale configure -label $deltaLabels($index)
	}

    }

    #
    # c a l c M o v e m e n t F a c t o r
    #
    # Local utility procedure to determine the "deltas" in changes to 
    # the viewpoint.  Minimally uses the value taken from the slider in the
    # GUI.  May additionally scale that if "height-based navigation" is in 
    # effect.
    #
    proc calcMovementFactor {inst view} {
	variable controllerScale

	set top [format "%s_vod" $inst]
	set delta [$top.$controllerScale get]

	if {[$inst.tvbase get TV_HEIGHT_BASED_NAVIGATION] == 1} {
	    #
	    # NOTE: these magical constants were taken from the XmTV version
	    # of TerraVision.
	    #
	    set stepSize [expr $delta/5.0 * [getViewAltitude $view] / 100.]
	} else {
	    set stepSize $delta
	}
	
	return $stepSize
    }

    #
    # x y z C B 
    #
    # Callback to affect changes in the viewpoint position, w.r.t. the global
    # coordinate system.
    #
    proc xyzCB {inst dir} {
	# get the view...
	set view [$inst.tvbase view current]

	set moveFactor [ViewControls::calcMovementFactor $inst $view]
	set eyePt [getViewEyePoint $view]

	switch $dir {
	    N {
		setViewEyePoint $view \
		    [lreplace $eyePt 1 1  \
		     [expr [lindex $eyePt 1] + $moveFactor]]
	    }
	    E {
		setViewEyePoint $view \
		    [lreplace $eyePt 0 0 \
		     [expr [lindex $eyePt 0] + $moveFactor]]
	    }
	    S {
		setViewEyePoint $view \
		    [lreplace $eyePt 1 1 \
		     [expr [lindex $eyePt 1] - $moveFactor]]
	    }
	    W {
		setViewEyePoint $view \
		    [lreplace $eyePt 0 0 \
		     [expr [lindex $eyePt 0] - $moveFactor]]
	    }
	    U {
		setViewEyePoint $view \
		    [lreplace $eyePt 2 2 \
		     [expr [lindex $eyePt 2] + $moveFactor]]
	    }
	    D {
		setViewEyePoint $view \
		    [lreplace $eyePt 2 2 \
		     [expr [lindex $eyePt 2] - $moveFactor]]
	    }
	}

	$inst.tvbase view current $view
    }

    #
    # f l u C B 
    #
    # Callback for fore/back left/right up/down changes to the viewpoint 
    # (i.e., viewer-centric changes).
    #
    proc fluCB {inst dir} {
	# get the view, and possibly scaled delta factor...
	set view [$inst.tvbase view current]
	set moveFactor [ViewControls::calcMovementFactor $inst $view]

	switch $dir {
	    N {
		$inst.tvbase view move $moveFactor 0. 0.
	    }
	    E {
		$inst.tvbase view move 0. $moveFactor 0.
	    }
	    S {
		$inst.tvbase view move [expr -$moveFactor] 0. 0.
	    }
	    W {
		$inst.tvbase view move 0. [expr -$moveFactor] 0.
	    }
	    U {
		$inst.tvbase view move 0. 0. $moveFactor
	    }
	    D {
		$inst.tvbase view move 0. 0. [expr -$moveFactor]
	    }
	}
    }

    #
    # h p r C B 
    #
    # Callback for heading/pitch/roll changes in the viewpoint.
    #
    proc hprCB {inst dir} {
	variable controllerScale

	# get the view and ypr...
	set view [$inst.tvbase view current]
	set ypr [getViewYPR $view]

	# get the delta factor...
	set top [format "%s_vod" $inst]
	set delta [$top.$controllerScale get]

	# convert to radians...
	set moveFactor [deg2rad $delta]

	switch $dir {
	    N {
                $inst.tvbase view rotate 0. $moveFactor 0.
	    }
	    E {
                $inst.tvbase view rotate 0. 0. $moveFactor
	    }
	    S {
		$inst.tvbase view rotate 0. [expr -$moveFactor] 0.
	    }
	    W {
		$inst.tvbase view rotate 0. 0. [expr -$moveFactor]
	    }
	    U {
		$inst.tvbase view rotate [expr -$moveFactor] 0. 0.
	    }
	    D {
		$inst.tvbase view rotate $moveFactor 0. 0.
	    }
	}

    }

    #
    # u p d a t e T i m e r P r o c 
    #
    # Called on a timer to update the "View Status" window.
    #
    proc updateTimerProc {inst} {
	upvar ViewControls::timers timers

	# get current view...
	set view [$inst.tvbase view current]

	# get the name of the status window...
	set top [format "%s_vos" $inst]

	# update it's fields...
	set eye [getViewEyePoint $view]
	$top.f1.l1_value configure -text [format "%.1f" [lindex $eye 0]]
	$top.f1.l2_value configure -text [format "%.1f" [lindex $eye 1]]
	$top.f1.l3_value configure -text [format "%.1f" [lindex $eye 2]]

	set ypr [getViewYPR $view]
	$top.f2.l1_value configure \
	    -text [format "%.2f" [rad2deg [lindex $ypr 0]]]
	$top.f2.l2_value configure \
	    -text [format "%.2f" [rad2deg [lindex $ypr 1]]]
	$top.f2.l3_value configure \
	    -text [format "%.2f" [rad2deg [lindex $ypr 2]]]

	# reschedule ourselves...
	set timers($inst) [after 5000 "ViewControls::updateTimerProc $inst"]
    }


    #
    # d i s m i s s C B 
    #
    # Dismisses (and destroys) the "View Controls" window.
    #
    proc dismissCB {inst} {
	destroy [format "%s_vod" $inst]
    }

    #
    # d i s m i s s G o t o C B 
    #
    # Dismisses (and destroys) the "Goto Lat/Long" window.
    #
    proc dismissGotoCB {inst} {
	destroy [format "%s_goto" $inst]
    }

    #
    # d i s m i s s G a z C B 
    #
    # Dismisses (and destroys) the "Goto Address" window.
    #
    proc dismissGazCB {inst} {
	destroy [format "%s_gaz" $inst]
    }

    #
    # d i s m i s s A D L C B 
    #
    # Dismisses (and destroys) the "Goto Feature" window.
    #
    proc dismissADLCB {inst} {
	destroy [format "%s_adl" $inst]
    }

    #
    # g o t o L a t L o n g C B 
    #
    # Dismisses (and destroys) the "Goto Lat/Long" window.
    #
    proc gotoLatLongCB {inst} {
	upvar ViewControls::longitudeField($inst) longitudeField
	upvar ViewControls::latitudeField($inst) latitudeField
	upvar ViewControls::elevationField($inst) elevationField

	# extract the location form the input fields

	set longitude [$longitudeField get]
	set latitude  [$latitudeField get]
	set elevation [$elevationField get]

	# check that they are at least numbers!

	if { [string trim $longitude] == "" } {
	    set longitude 0.0
	}

	if { [string trim $latitude] == "" } {
	    set latitude 0.0
	}

	if { [string trim $elevation] == "" } {
	    set elevation 10000000.0
	}

	if {![string is double $longitude]} {
	   tk_messageBox -type ok -icon error -title "Coordinate Error" \
             -message "Invalid longitude value supplied! The value must\nbe a number. Use a negative number for longitudes\ndue west of the Prime Meridian."
           return 0;
	}

	if {![string is double $latitude]} {
	   tk_messageBox -type ok -icon error -title "Coordinate Error" \
             -message "Invalid latitude value supplied! The value must\nbe a number. Use a negative number for latitudes\ndue south of the equator."
           return 0;
	}

	if {![string is double $elevation]} {
	   tk_messageBox -type ok -icon error -title "Coordinate Error" \
             -message "Invalid elevation value supplied! The value must\nbe a number in units of meters."
           return 0;
	}

	# get the current orienation

	set curpos [$inst.tvbase view getgdc]
        set yaw    [lindex $curpos 3]
        set pitch  [lindex $curpos 4]
        set roll   [lindex $curpos 5]

	# set the new lat/long/elev location

	$inst.tvbase view setgdc $latitude $longitude $elevation \
		$yaw $pitch $roll

	# update the status bar appropriately...
	statusBarSetViewName $inst [format "Lon: %f, Lat: %f, Elev: %f" \
					$longitude $latitude $elevation]
    }

    #
    # s e t O r i e n t C B 
    #
    # sets the user's orientation from yaw,pitch,roll (deg) input
    #
    proc setOrientCB {inst} {
	upvar ViewControls::yawField($inst) yawField
	upvar ViewControls::pitchField($inst) pitchField
	upvar ViewControls::rollField($inst) rollField

	# extract the orientation from the input fields

	set yaw    [$yawField get]
	set pitch  [$pitchField get]
	set roll   [$rollField get]

	# check that they are at least numbers!

	if { [string trim $yaw] == "" } {
	    set yaw 0.0
	}

	if { [string trim $pitch] == "" } {
	    set pitch -90.0
	}

	if { [string trim $roll] == "" } {
	    set roll 0.0
	}

	if {![string is double $yaw]} {
	   tk_messageBox -type ok -icon error -title "Coordinate Error" \
             -message "Invalid yaw value supplied! Number must be in degrees."
           return 0;
	}

	if {![string is double $pitch]} {
	   tk_messageBox -type ok -icon error -title "Coordinate Error" \
             -message "Invalid pitch supplied! Number must be in degrees."
           return 0;
	}

	if {![string is double $roll]} {
	   tk_messageBox -type ok -icon error -title "Coordinate Error" \
             -message "Invalid roll value supplied! Number must be in degrees."
           return 0;
	}

	# convert the values to degrees

	set yaw_rad    [deg2rad $yaw]
	set pitch_rad  [deg2rad $pitch]
	set roll_rad   [deg2rad $roll]

	# get the current position

	set curpos    [$inst.tvbase view getgdc]
        set latitude  [lindex $curpos 0]
        set longitude [lindex $curpos 1]
        set elevation [lindex $curpos 2]

	# set the new orientation

	$inst.tvbase view setgdc $latitude $longitude $elevation \
		$yaw_rad $pitch_rad $roll_rad
    }

    #
    # g o t o A d d r e s s C B 
    #
    # Translates an address into a lat/long and jumps to that location
    #
    proc gotoAddressCB {inst} {
	upvar ViewControls::streetField($inst) streetField
	upvar ViewControls::cityField($inst) cityField
	upvar ViewControls::provinceField($inst) provinceField
	upvar ViewControls::postcodeField($inst) postcodeField
	upvar ViewControls::countryField($inst) countryField
	upvar ViewControls::heightOverCityField($inst) heightOverCityField
	upvar ViewControls::gazInfoField($inst) gazInfoField

	# extract the location form the input fields

	set street    [string trim [$streetField get]]
	set city      [string trim [$cityField get]]
	set province  [string trim [$provinceField get]]
	set postcode  [string trim [$postcodeField get]]
	set country   [string trim [$countryField get]]
	set height    [string trim [$heightOverCityField get]]

	# convert height from km to m

	set height    [expr $height * 1000.0]

	# check that we have a city

	if {[string length $city] == 0 && [string length $postcode] == 0} {
	   tk_messageBox -type ok -icon error -title "Gazetteer Error" \
             -message "You must enter a city name or postcode at least!"
           return 0;
	}

	# get the current orienation

	configGazButtons $inst 0

	set gazresponse \
	    [tvutils gazetteer $street $city $province $postcode $country]

	configGazButtons $inst 1

	# check for an error (list is empty or just ["error"])

	set listlen [llength $gazresponse]
	if { $listlen <= 1 } {
	    tk_messageBox -type ok -icon error -title "Gazetteer Error" \
	      -message "Sorry: could not locate that city name"
	    return 0;
	}

	# check for a found location and fly to the lat/long

	if { [lindex $gazresponse 0] == "location" } {
	    set latitude  [lindex $gazresponse 1]
	    set longitude [lindex $gazresponse 2]
	    $inst.tvbase view setgdc $latitude $longitude $height 0 -1.57 0

	    # update status bar...
	    statusBarSetViewName $inst [format "%s/%s/%s/%s" $city $province \
					    $postcode $country $height]
	    return 0;
	}

	# otherwise, we have a list of choices

	set info "Found a number of choices for your city"
	set info "$info\nTry refining your search with one of these:\n"

	set i 1
	while { $i <= $listlen } {
	    set choice [lindex $gazresponse $i]
	    set info "$info\n$choice"
	    incr i
	}

	tk_messageBox -type ok -icon info -title "Gazetteer Result" \
	    -message $info

    }

    #
    # g o t o F e a t u r e C B 
    #
    # Translates a feature into a lat/long and jumps to that location
    #
    proc gotoFeatureCB {inst} {
	upvar ViewControls::adlNameField($inst) adlNameField
	upvar ViewControls::adlInfoField($inst) adlInfoField
	upvar ViewControls::adlGotoField($inst) adlGotoField
	upvar ViewControls::adlHeightField($inst) adlHeightField
	global adlTypeField

	# extract the location form the input fields

	set adl_name  [string trim [$adlNameField get]]
	set height    [string trim [$adlHeightField get]]
	set adl_type  $adlTypeField

	# validate the parameters 

	set height    [expr $height * 1000.0]

	if { $adl_type == "all" } {
	    set adl_type ""
	}

	# check that we have a name

	if { [string length $adl_name] == 0 } {
	   tk_messageBox -type ok -icon error -title "ADL Gazetteer Error" \
             -message "You must enter a feature name, e.g. yosemite"
           return 0;
	}

	# get the current orienation

	configADLButtons $inst 0

	set adlresponse [tvutils adl $adl_name $adl_type]

	configADLButtons $inst 1

	set matches [lindex $adlresponse 0]
	set total   [lindex $adlresponse 1]

	# check for an error (list is empty or just ["error"])

	set listlen [llength $adlresponse]
	if { $listlen <= 1 } {
	    tk_messageBox -type ok -icon error -title "ADL Gazetteer Error" \
	      -message "Sorry: could not locate anything for that feature"
	    return 0;
	}

	# check for a found location and fly to the lat/long

	if { $matches == 1 } {
	    set fullname  [lindex $adlresponse 2]
	    set fulltype  [lindex $adlresponse 3]
	    set latitude  [lindex $adlresponse 4]
	    set longitude [lindex $adlresponse 5]

	    $inst.tvbase view setgdc $latitude $longitude $height 0 -1.57 0

	    # update status bar...
	    statusBarSetViewName $inst [format "%s" $fullname]
	    return 0;
	}

	# otherwise, we have a list of choices

	set info "Found a number of choices for your feature"
	set info "$info\nTry refining your search with one of these"
	set info "$info ($matches out of $total):\n"

	set i 2
        set j 0
	while { $j < $matches } {
	    set fullname  [lindex $adlresponse $i]
	    set fulltype  [lindex $adlresponse [expr $i+1]]
	    set latitude  [lindex $adlresponse [expr $i+2]]
	    set longitude [lindex $adlresponse [expr $i+3]]

	    set info "$info\n$fullname"
            if { $fulltype != "" } {
		set info "$info ($fulltype)"
	    }

	    set i [expr $i + 4]
	    incr j
	}

	tk_messageBox -type ok -icon info -title "Gazetteer Result" \
	    -message $info

    }

    #
    # d i s m i s s V S C B 
    #
    # Dismisses (and destroys) the "View Status" window.
    #
    proc dismissVSCB {inst} {
	upvar ViewControls::timers timers
	# cancel the timer...
	after cancel $timers($inst)
	# nuke the window...
	destroy [format "%s_vos" $inst]
    }


}  ; # ViewControls namespace



#
# Global utilities for manipulating Tcl-stringified "tvViewpoint"s...
#

proc getViewAltitude {view} {
    return [lindex $view 1]
}

proc setViewAltitude {view altitude} {
    upvar view v
    set v [lreplace $v 1 1 $altitude]
}

proc getViewEyePoint {view} {
    return [lindex $view 0]
}

proc setViewEyePoint {view eye} {
    upvar view v
    set v [lreplace $v 0 0 $eye]
}

proc getViewYPR {view} {
    return [lindex $view 8]
}

proc setViewYPR {view ypr} {
    upvar view v
    set v [lreplace $v 8 8 $ypr]
}

proc getViewFOV {view} {
    return [lindex $view 6]
}

proc setViewFOV {view fov} {
    upvar view v
    set v [lreplace $v 6 6 $fov]
}
