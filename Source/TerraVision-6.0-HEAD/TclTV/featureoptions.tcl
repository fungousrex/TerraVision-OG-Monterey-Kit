#
# featureoptions.tcl
#

namespace eval Features {
    namespace export postOpenFeaturesDialog postCloseFeaturesDialog \
	             postFeatureOptionsDialog closeAllModels loadedModel

    #
    # module globals...
    # These are all associative arrays holding the state of the rendering 
    # options for Model Loading dialogs, and indexed by the instanceID
    #

    variable featureList             ;# full model database
    variable featureListWidget       ;# list of available models
    variable locationListWidget      ;# list for model location
    variable infoListWidget          ;# list for model information
    variable closeListWidget         ;# list of models to close
    variable modelUrlWidget          ;# list of models to close
    variable openModelButton         ;# button widget for "Open Model"
    variable infoModelButton         ;# button widget for "Get Information"
    variable loadedList              ;# list of models currently loaded
    variable modelObserverToken      ;# for refreshing on new geoset events

    variable showmodels              ;# display VRML models?
    variable textured                ;# VRML models are textures?
    variable lighting                ;# VRML models are lit?
    variable wireframe               ;# Render models in wireframe?
    variable loadtextures            ;# Load textures for models?
    variable backfacecull            ;# perform backface culling of models?

    #
    # p o s t O p e n F e a t u r e s D i a l o g
    #
    # This is the main GUI for loading GeoVRML models.
    #
    proc postOpenFeaturesDialog {id} {
	upvar Features::featureListWidget($id) featureListWidget
	upvar Features::locationListWidget($id) locationListWidget
	upvar Features::infoListWidget($id) infoListWidget
	upvar Features::modelUrlWidget($id) modelUrlWidget
	upvar Features::openModelButton($id) openModelButton
	upvar Features::infoModelButton($id) infoModelButton
	upvar ViewBookmarks::modelObserverToken($id) modelObserverToken

	# make the toplevel window and set various window-options...
	set topname [format "%s_openmodels" $id ]

	# don't want more than one of these...
	if {[winfo exists $topname] == 1} {
	    wm withdraw $topname
	    wm deiconify $topname
	    return
	}
	set top [toplevel $topname]
	wm title $top [format "Open GeoVRML Model: Viewer %d" [instnum $id]]

	set f1 $top.f1
	frame $f1 -relief raised -borderwidth 2

	# middle frame, composed of three subframes...
	set f2Pad 2
	set f2 $top.f2
	frame $f2 
	
	# subframe 1:  the geosets list...

	set f21 $f2.1
	frame $f21
	label $f21.l -text "GeoVRML 3D Models"
	set featureListWidget [makeListBox $f21 -width 30 -height 14 \
	                       -exportselection false]
	set geosetsParent [getListBoxParent $featureListWidget]

	pack $f21.l -in $f21 -side top -fill x -pady $f2Pad
	pack [getListBoxParent $featureListWidget] -in $f21 -side bottom \
	    -expand true -fill both -pady $f2Pad
	
	# subframe 2:  the location/information location lists...

	set f22 $f2.2
	frame $f22
	label $f22.loc -text "Model Location"
	set locationListWidget [makeListBox $f22 -height 4 -width 40 \
	                        -exportselection false]
	

	set f221 $f22.1
	frame $f221
	label $f221.info -text "Model Information"
	set infoListWidget $f221.infolist
	text $infoListWidget -width 40 -height 8 -state disabled \
	    -relief groove -wrap none \
	    -xscrollcommand "$f221.hsb set" \
	    -yscrollcommand "$f221.vsb set"
	scrollbar $f221.vsb -orient vertical -command "$infoListWidget yview"
	scrollbar $f221.hsb -orient horizontal -command "$infoListWidget xview"

	pack $f221.info -side top -anchor n -pady $f2Pad
	pack $f221.hsb -side bottom -expand 1 -fill x 
	pack $infoListWidget -side top -expand 1 -fill both
	pack $f221.vsb -side right -expand 1 -fill y

	pack $f22.loc -in $f22 -side top -fill x -pady $f2Pad
	pack [getListBoxParent $locationListWidget] -in $f22 -expand yes \
	     -fill both -pady $f2Pad

	pack $f221 -expand true -fill both -pady $f2Pad

	button $f22.getdesc -text "Get Description" -state disabled \
	    -command "Features::showModelInfo $id $top"
	set infoModelButton $f22.getdesc
	pack $infoModelButton -expand true -fill x

	pack $f21 $f22 -in $f2 -side left -expand true -fill both \
	    -padx 4 -pady 4

	# display the URL string

	set furl $top.furl
	frame $furl

	label $furl.label -text "   URL:"
	set modelUrlWidget $furl.input
	entry $modelUrlWidget -width 30

        pack $furl.label -in $furl -side left -padx 0 -pady 4
        pack $furl.input -in $furl -side left -expand true -fill x -padx 10

	# bottom frame:  the action area...
	set f3 $top.f3
	frame $f3 -relief raised -borderwidth 2
	button $f3.ok -text "Load Model" \
	    -command "Features::openModel $id"
	set openModelButton $f3.ok

	button $f3.settings -text "Model Settings" \
	    -command "Features::postFeatureOptionsDialog $id"

	button $f3.cancel -text "Cancel" \
	    -command "Features::dismissOpenModelDialog $id"

	pack $f3.ok -in $f3 -side left -padx 10
	pack $f3.settings -in $f3 -side left -padx 0
	pack $f3.cancel -in $f3 -side right -padx 10

	pack $f2 -in $f1 -expand true -fill both -ipady 4
        pack $furl -in $f1 -expand true -fill x -ipady 4

	pack $f1 -expand true -fill both -ipady 4
	pack $f3 -expand true -fill x -ipady 4

	# refresh the contents of the model list
	refreshOpenModelList $id

	# set the bindings for the various listboxes...

	set bindscript "eval \{Features::openModel $id\}"
	bind $featureListWidget <Double-Button-1> $bindscript
	bind $locationListWidget <Double-Button-1> $bindscript
	set bindscript "eval \{Features::selectModel $id\}"
	bind $featureListWidget <ButtonRelease-1> $bindscript
	set bindscript "eval \{Features::selectModelUrl $id\}"
	bind $locationListWidget <ButtonRelease-1> $bindscript

	# register as an observer of geoset-change events...
	set modelObserverToken [DatasetDialog::registerDatasetObserver \
				     "Features::refreshOpenModelList $id"]
	# make it visible...
	wm deiconify $top
    }

    #
    # d i s m i s s O p e n M o d e l D i a l o g
    #
    proc dismissOpenModelDialog {id} {
	upvar ViewBookmarks::modelObserverToken($id) modelObserverToken

	# nothing to do if window not open
	set topname [format "%s_openmodels" $id ]
	if {[winfo exists $topname] != 1} {
	    return
	}

	# close the window
	destroy $topname

	# unregister as an observer of dataset open/close events...
	DatasetDialog::unregisterDatasetObserver $modelObserverToken
	unset modelObserverToken
    }

    #
    # r e f r e s h O p e n M o d e l L i s t
    #
    # updates the display of the list widgets in the Open Model dialog
    #
    proc refreshOpenModelList {id} {
	upvar Features::featureList($id) featureList
	upvar Features::featureListWidget($id) featureListWidget
	upvar Features::locationListWidget($id) locationListWidget
	upvar Features::infoListWidget($id) infoListWidget

	# get the latest model list from the database

	set featureList [$id.tvbase feature list]
	set featureLen  [llength $featureList]
	set nameList    []

	# create a list of model names 
	
	foreach i $featureList {
	    set name [lindex $i 0]
	    set title [lindex $i 1]
	    if {[string length $title] < 1} {
		lappend nameList $name
	    } else {
		lappend nameList $title
	    }
	}

	# populate the list of available models in the database...
	populateListBox $featureListWidget $nameList
	# clear the contents of the other list widgets
	populateListBox $locationListWidget []
	populateListBox $infoListWidget []
    }

    #
    # s e l e c t M o d e l
    #
    # Callback for when the user "selects" an item in the "models" listbox.
    #
    proc selectModel {id} {
	upvar Features::featureList($id) featureList
	upvar Features::featureListWidget($id) featureListWidget
	upvar Features::locationListWidget($id) locationListWidget
	upvar Features::infoListWidget($id) infoListWidget
	upvar Features::modelUrlWidget($id) modelUrlWidget
	upvar Features::openModelButton($id) openButton
	upvar Features::infoModelButton($id) infoButton

	# if nothing selected, merely disable Open button...
	if {[llength [$featureListWidget curselection]] == 0} {
	    $openButton configure -state disabled
	    $infoButton configure -state disabled
	    $locationListWidget delete 0 end
	    $infoListWidget delete 0 end
	    return
	}

	# if we're here, we activate the "Open Geoset" button, as its now
	# valid...
	$openButton configure -state normal

	# find out what was selected...
	set selNumb  [$featureListWidget curselection]
	set selected [lindex $featureList $selNumb]
        set servers  [lreplace $selected 0 3]

	# set the location list and clear the info list
	populateListBox $locationListWidget $servers
	populateListBox $infoListWidget []

	# select the first location and enable the "Get Information" button
	$locationListWidget selection set 0
	$infoListWidget configure -state normal
	$infoListWidget delete 1.0 end	
	$infoListWidget configure -state disabled
	$infoButton configure -state normal

	# set the URL string to be the first URL
	set first_url [lindex $servers 0]
	$modelUrlWidget delete 0 end
	$modelUrlWidget insert 0 $first_url
    }

    #
    # s e l e c t M o d e l U r l 
    #
    # Callback for when the user "selects" an item in the list of URLs
    #
    proc selectModelUrl {id} {
	upvar Features::locationListWidget($id) locationListWidget
	upvar Features::modelUrlWidget($id) modelUrlWidget

	# if nothing selected, merely disable Open button...
	if {[llength [$locationListWidget curselection]] == 0} {
	    set url ""
	} else {
	    # find out what was selected...
	    set selNumb  [$locationListWidget curselection]
	    set url [$locationListWidget get $selNumb]
        }

	# set the URL string to be the first URL
	$modelUrlWidget delete 0 end
	$modelUrlWidget insert 0 $url
    }

    #
    # o p e n M o d e l
    #
    # Callback for when a user "opens" a model, either by double-clicking on
    # an item in the models/location listbox, or by selecting it and then
    # pressing the "Open Model" button.
    #
    proc openModel {id} {
	upvar Features::locationListWidget($id) locationListWidget
	upvar Features::modelUrlWidget($id) modelUrlWidget
	upvar Features::loadedList($id) loadedList
	upvar Features::closeListWidget($id) closeListWidget
	
	# find out what was selected...
	# set url [$locationListWidget get [$locationListWidget curselection]]
	set url [string trim [$modelUrlWidget get]]

	if { $url == "" } {
	    tk_messageBox -title "TerraVision" -type ok -icon error \
		    -message "No URL specified for the GeoVRML model"
	    return;
        }

	# make sure the geoset has not already been opened...
	if {[lsearch $loadedList $url] != -1} {
	    tk_messageBox -title "TerraVision" \
		-message [format "Model is already opened:\n%s" $url] \
		-icon info \
		-type ok
	    return
	}

        # check that the URL does indeed have something there
        if { [tsmUtil urlExist $url] == 0 } {
	    tk_messageBox -title "TerraVision" -icon error -type ok \
		-message [format \
                "That file does not exist or cannot be accessed:\n%s" $url] 
	    return
        }

	# start the loading the model... if this works, then
	# we get a callback (via the MODEL_ADD message) to
	# the loadedModel function below
	$id.tvbase feature load $url

	# close the window afterwards
	Features::dismissOpenModelDialog $id
    }

    #
    # l o a d e d M o d e l 
    #
    # This callback is used to add a VRML model to our global
    # list of open models. This is called whenever we receive
    # a MODEL_ADD message from the C API, e.g. from our TclTV
    # layer or from the GeoWeb.
    #
    proc loadedModel {id url} {
	upvar Features::loadedList($id) loadedList
	upvar Features::closeListWidget($id) closeListWidget

	# make sure the model has not already been opened...
	if { [lsearch $loadedList $url] != -1 } {
	    return
	}

	# add it to our global list
	lappend loadedList $url

	# update the "Close Model" dialog (only if dialog is open)
	if {[winfo exists [format "%s_closemodels" $id]] == 1} {
	    populateListBox $closeListWidget $loadedList
	}
    }

    #
    # s h o w M o d e l I n f o
    #
    # Callback for when a user clicks on the "Get Information" button
    # for a model. Loads, parses, and displays the model's metadata.
    #
    proc showModelInfo {id win} {
	upvar Features::locationListWidget($id) locationListWidget
	upvar Features::infoListWidget($id) infoListWidget

	set url [$locationListWidget get [$locationListWidget curselection]]

	# get the mode information
	$win configure -cursor watch
	update                          ;# needed to flush the cursor change
	set modelInfo [$id.tvbase feature info $url]
	#$win configure -cursor arrow
	$win configure -cursor [$id cget -cursor]

	$infoListWidget configure -state normal
	$infoListWidget delete 1.0 end	
	$infoListWidget insert end $modelInfo
	$infoListWidget configure -state disabled
    }

    #
    # p o s t C l o s e D S D i a l o g 
    #
    proc postCloseFeaturesDialog {id} {
	upvar Features::loadedList($id) loadedList
	upvar Features::closeListWidget($id) closeListWidget

	# make the toplevel window and set various window-options...
	set winName [format "%s_closemodels" $id ]

	# don't want more than one of these...
	if {[winfo exists $winName] == 1} {
	    wm withdraw $winName
	    wm deiconify $winName
	    return
	}

	toplevel $winName -class Dialog
	wm withdraw $winName
	wm title $winName "Close GeoVRML Model"

	# upper portion...
	set top $winName
	frame $top.f1 -borderwidth 2 -relief raised
	label $top.f1.title -text "Select model URL to be closed:"
	set closeListWidget [makeListBox $top.f1 -width 40 -height 12]

	pack $top.f1.title -side top -anchor nw
	pack [getListBoxParent $closeListWidget] -side top -anchor n \
	    -expand 1 -fill both -padx 5 -pady 5

	# action area...
	frame $top.f2 -borderwidth 2 -relief raised
	button $top.f2.ok -text "Close Model" \
	    -command "Features::closeModel $id $top"
	button $top.f2.closeall -text "Close ALL" \
	    -command "Features::closeAllModels $id"
	button $top.f2.cancel -text "Cancel" -command "destroy $top"

	pack $top.f2.ok $top.f2.closeall $top.f2.cancel \
	    -side left -expand 1 -fill x

	pack $top.f1 -side top -expand 1 -fill both
	pack $top.f2 -side top -expand 1 -fill x
	
	populateListBox $closeListWidget $loadedList
	wm deiconify $winName
    }    

    #
    # c l o s e M o d e l
    #
    # Closes an opened model as selected via the "Close Model" dialog
    #
    proc closeModel {id top} {
	upvar Features::loadedList($id) loadedList
	upvar Features::closeListWidget($id) closeListWidget

	# get the model URL to be closed
	set selected [$closeListWidget curselection]
	if { $selected < 0 } { return }
	set url [$closeListWidget get $selected]

	# close it
	$id.tvbase feature unload $url
	
	# remove from global list...
	set loadedList [lreplace $loadedList $selected $selected]

	# close the dialog
	destroy $top
    }

    #
    # c l o s e A l l M o d e l s
    #
    proc closeAllModels {id} {
	upvar Features::loadedList($id) loadedList
	upvar Features::closeListWidget($id) closeListWidget

	# close all of the models for this viewer
	$id.tvbase feature unloadall
	set loadedList []

	# close the Close Models window if open
	set winName [format "%s_closemodels" $id ]
	if {[winfo exists $winName] == 1} {
	    populateListBox $closeListWidget $loadedList
	}
    }

    #
    # p o s t F e a t u r e O p t i o n s D i a l o g
    #
    proc postFeatureOptionsDialog {id} {
	upvar Features::textured($id) textured
	upvar Features::lighting($id) lighting
	upvar Features::wireframe($id) wireframe
	upvar Features::loadtextures($id) loadtextures
	upvar Features::backfacecull($id) backfacecull
	upvar Features::showmodels($id) showmodels 

	# make the toplevel window and set various window-options...
	set topname [format "%s_modelopts" $id ]

	# don't want more than one of these...
	if {[winfo exists $topname] == 1} {
	    wm withdraw $topname
	    wm deiconify $topname
	    return
	}
	set top [toplevel $topname]
	wm title $top [format "3D Model Settings: Viewer %d" [instnum $id]]
	    
	# topmost frame of checkboxes/radiobutttons...
	frame $top.f1 -borderwidth 2 -relief raised

	# need internal frames to organize the buttons into 2 columns...
	# 1st column...
	set f $top.f1.f1
	frame $f

	checkbutton $f.01 \
	    -text "Show 3D Models" \
	    -command "RenderOptions::checkboxCB $id $f.01 \
                      TV_SHOW_MODELS Features::showmodels($id)" \
	    -variable Features::showmodels($id)

	checkbutton $f.02 \
	    -text "Model Lighting" \
	    -command "RenderOptions::checkboxCB $id $f.02 \
                      TV_FEATURES_LIGHTING Features::lighting($id)" \
	    -variable Features::lighting($id)

	checkbutton $f.03 \
	    -text "Textured Models" \
	    -command "RenderOptions::checkboxCB $id $f.03 \
                      TV_FEATURES_TEXTURED Features::textured($id)" \
	    -variable Features::textured($id)

	pack $f.01 $f.02 $f.03  -side top -anchor w

	# 2nd column...
	set f $top.f1.f2
	frame $f

	checkbutton $f.01 \
	    -text "Wireframe Models" \
	    -command "RenderOptions::checkboxCB $id $f.01 \
                      TV_FEATURES_WIREFRAME Features::wireframe($id)" \
	    -variable Features::wireframe($id)

	checkbutton $f.02 \
	    -text "Load Textures" \
	    -command "RenderOptions::checkboxCB $id $f.02 \
                      TV_FEATURES_LOAD_TEXTURES Features::loadtextures($id)" \
	    -variable Features::loadtextures($id)

	checkbutton $f.03 \
	    -text "Backface Culling" \
	    -command "RenderOptions::checkboxCB $id $f.03 \
                      TV_FEATURES_BACKFACE_CULL Features::backfacecull($id)" \
	    -variable Features::backfacecull($id)

	pack $f.01 $f.02 $f.03 -side top -anchor w

	# pack the two columns...
	pack $top.f1.f1 $top.f1.f2 -side left -padx 5 -pady 5 \
		-expand 1 -fill both
	
	# dismiss button...
	button $top.dismiss -text "Dismiss" -relief raised -borderwidth 2 \
            -command "destroy $top"
	
	pack $top.f1 -side top -expand 1 -fill both -ipadx 5 -ipady 5
	pack $top.dismiss -side top -expand 1 -fill x

	set tk_strictMotif 1
    }

    #
    # i n i t F e a t u r e s
    #
    # Performs all necessary initializations of the features support
    #
    proc initFeatures {id} {
	upvar Features::loadedList($id) loadedList
	upvar Features::featureList($id) featureList
	upvar Features::showmodels($id) showmodels
	upvar Features::textured($id) textured
	upvar Features::lighting($id) lighting
	upvar Features::wireframe($id) wireframe
	upvar Features::loadtextures($id) loadtextures
	upvar Features::backfacecull($id) backfacecull

	set featureList  []
	set loadedList   []
	set showmodels   [$id.tvbase get TV_SHOW_MODELS]
	set textured     [$id.tvbase get TV_FEATURES_TEXTURED]
	set lighting     [$id.tvbase get TV_FEATURES_LIGHTING]
	set wireframe    [$id.tvbase get TV_FEATURES_WIREFRAME]
	set loadtextures [$id.tvbase get TV_FEATURES_LOAD_TEXTURES]
	set backfacecull 1   ;# [$id.tvbase get TV_FEATURES_BACKFACE_CULL]
    }

}
