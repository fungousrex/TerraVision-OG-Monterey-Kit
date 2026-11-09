#
# mainwin.tcl
#


#
# A note on instance variables:
#   Each "View Window" has a unique instance-ID associated with it.  Variables
#   and widgets associated with that view window are addressed by prefixing 
#   the instance-ID to the variable/widget name.  
#
#   Of special note is the "__.tvbase" Togl widget, which is directly 
#   associated with a TerraVision "tvBase" instance. Thus commands to the
#   TerraVision library are accessed through this widget.
#
# Key widgets associated with a view window are (presuming an instance 
# named "inst1"):
#
#    inst1                           # the toplevel widget
#    inst1.tvbase                    # the Togl widget (and associated tvbase)
#    inst1.status.bookmark           # the bookmarks field in status bar
#    inst1.status.datasets           # the datasets    "   "     "    "
#    inst1.status.navmode            # the Nav-Mode    "   "     "    "
#    inst1.status.viewmode           # the View Mode   "   "     "    "
# 

#
# n e w V i e w W i n d o w 
#
# Creates a new 2D/3D view window.  Parameter is expected to be either 
# "2D" or "3D".
#
proc newViewWindow {viewType} {
    global gInstBase gInstNum gInstID
    global gWinList
    global gGeosetsList

    # create a new instance-ID from our global vars...
    set gInstID $gInstBase[incr gInstNum]

    # for use locally...
    set id $gInstID

    set top [toplevel $id]
    wm title $top [format "TerraVisionII: Viewer %d" $gInstNum]

    # make the menubar...
    ### THIS IS A Tk BUG: contrary to docs and books, do NOT use the
    ### "-type menubar" clause;  keyboard menu commands will cause a
    ### grab-failed error.
    ### set mbar [menu $gInstID.menubar -type menubar]
    set mbar [menu $id.menubar]

    $mbar add cascade -label "File" -underline 0 \
	-menu [makeFileMenu $id $mbar]
    $mbar add cascade -label "View" -underline 0 \
	-menu [makeViewMenu $id $mbar]
    $mbar add cascade -label "Settings" -underline 0 \
	-menu [makeSettingsMenu $id $mbar] 
    $mbar add cascade -label "Paths" -underline 0 \
	-menu [Paths::makePathsMenu $id $mbar]
    $mbar add cascade -label "Links" -underline 0 \
	-menu [Links::makeLinksMenu $mbar]
    $mbar add cascade -label "OAA" -underline 0 \
	-menu [OAA::makeOAAMenu $id $mbar]
    $mbar add cascade -label "Debug" -underline 0 \
	-menu [DebugOptions::makeGlobalsMenu $id $mbar]
    $mbar add cascade -label "Help" -underline 0 \
	-menu [makeHelpMenu $id $mbar]

    $top configure -menu $mbar

    # make the status bar...
    set status [makeStatusBar $id]

    #
    # make graphics area...
    #
    # NOTE: This widget is our Togl widget, and it is the key to 
    # interfacing with the TerraVision library; these widgets introduce 
    # a number of commands, accessable via the widget name. For example:
    #    [$id.tvbase ......]
    #

    set win_width [tvutils get TVU_WIDTH]
    set win_height [tvutils get TVU_HEIGHT]

    if { $win_width <= 0 } { set win_width 640 }
    if { $win_height <= 0 } { set win_height 480 }

    togl $id.tvbase -ident $id.tvbase -double true -depth true \
	-stencil true -width $win_width -height $win_height

    if {$viewType == "2D"} {
	$id.tvbase set TV_TWOD_CONSTRAIN true
    }
    setTVDefaults $id

    pack $status -side bottom -fill x
    pack $id.tvbase -anchor center -expand 1 -fill both

    # install the various bindings...
    mainWinBindings $id

    # fill in initial values on status bar...
    global $id.status.datasets
    set $id.status.datasets [llength $gGeosetsList]

    # enter this window's base ID into our global list...
    lappend gWinList $id

    #
    # Implement the -cmd options now that everything has started up
    #

    set numcmds [tvutils get TVU_NUM_COMMANDS]
    set i 0
    while { $i < $numcmds } {
	set thiscmd [tvutils get TVU_COMMAND $i]
	$id.tvbase OAA interpret $thiscmd
	incr i
    }

    return $id
}


#
# m a k e F i l e M e n u 
#
proc makeFileMenu {id parent} {

    set mname $parent.file
    menu $mname -type tearoff
    $mname add cascade -label "New Viewer" -menu $mname.view -underline 0 
    $mname add command -label "Close Viewer"  -underline 6 \
	-command "fileMenuCloseViewerCB $id"
    $mname add separator

    $mname add cascade -label "Open Dataset" -menu $mname.open -underline 0 
    $mname add cascade -label "Close Dataset" -menu $mname.close -underline 0 

    $mname add separator
    $mname add command -label "Servers..." -underline 0 \
	-accelerator "S" -command "DatasetDialog::postServers"

    $mname add separator
    $mname add command -label "Console..." -underline 0 \
	-accelerator "C" -command "DatasetDialog::postConsole"

    $mname add separator
    $mname add command -label "Properties..." -underline 0 \
	-command "DatasetDialog::postInfoDSDialog"
    $mname add separator
    $mname add command -label Quit -underline 0 -accelerator "Ctrl+Q" \
	-command "quitCB" 

    menu $mname.view
    $mname.view add command -label "3D Viewer" -underline 0 \
	-command "newViewWindow 3D"
    $mname.view add command -label "2D Viewer" -underline 0 \
	-command "newViewWindow 2D"
    # Map viewer is not implemented yet 
	#$mname.view add command -label "Map Viewer" -underline 0 \
	#-command "dummyCB $id"

    menu $mname.open
    $mname.open add command -label "Open Terrain..." -underline 0 \
	-command "DatasetDialog::postOpenDSDialog"
    $mname.open add command -label "Open 3D Model..." -underline 8 \
	-command "Features::postOpenFeaturesDialog $id"

    # GeoWeb support is disabled in the Open Source release of 
    # TerraVision because the GeoWeb libraries are not open sourced
    #$mname.open add command -label "Open GeoWeb..." -underline 5 \
    #	-command "DotGeo::postOpenDotGeoDialog $id"

    menu $mname.close
    $mname.close add command -label "Close Terrain..." -underline 0 \
	-command "DatasetDialog::postCloseDSDialog"
    $mname.close add command -label "Close 3D Model..." -underline 1 \
	-command "Features::postCloseFeaturesDialog $id"
  
    # GeoWeb support is disabled in the Open Source release of 
    # TerraVision because the GeoWeb libraries are not open sourced
    #$mname.close add command -label "Close GeoWeb..." -underline 5 \
    #	-command "DotGeo::postOpenDotGeoDialog $id"

    $mname.close add command -label "Close All" -underline 7 \
	-command "DatasetDialog::closeAllDatasets"


    return $mname
}

#
# m a k e V i e w M e n u 
#
proc makeViewMenu {id parent} {
    global gViewMode

    set mname $parent.view
    menu $mname -type tearoff
    $mname add cascade -label Mode -underline 0 -menu $mname.mode
    $mname add separator
    $mname add command -label "Viewpoint Controls" -underline 10 \
	-command "ViewControls::postViewControlsDialog $id"
    $mname add command -label "Viewpoint Status" -underline 10 \
	-accelerator "P" \
	-command "ViewControls::postViewStatusDialog $id"
    $mname add command -label "Viewpoint Bookmarks" -underline 10 \
	-accelerator "B" \
	-command "ViewBookmarks::postBookmarksDialog $id"
    $mname add separator
    $mname add command -label "Goto Address" -underline 10 \
	-accelerator "A" -command "ViewControls::postGazetteerDialog $id"
    $mname add command -label "Goto Feature" -underline 10 \
	-accelerator "F" -command "ViewControls::postADLDialog $id"
    $mname add command -label "Goto Lat/Long" -underline 10 \
	-accelerator "G" \
	-command "ViewControls::postGotoDialog $id"
    $mname add command -label "Goto Origin" -underline 10 \
	-accelerator "O" \
	-command "$id.tvbase view origin"

    menu $mname.mode \
	-postcommand "set gViewMode($id) \[$id.tvbase get TV_TWOD_CONSTRAIN\]"
    $mname.mode add radiobutton -label "2D View" -underline 0 \
	-accelerator "2" \
	-variable gViewMode($id) -value 1 \
	-command "$id.tvbase set TV_TWOD_CONSTRAIN 1;
                  statusBarSetViewMode $id 1"
    $mname.mode add radiobutton -label "3D View" -underline 0 \
	-accelerator "3" \
	-variable gViewMode($id) -value 0 \
	-command "$id.tvbase set TV_TWOD_CONSTRAIN 0;
                  statusBarSetViewMode $id 0"

    return $mname
}

#
# m a k e S e t t i n g s M e n u 
#
proc makeSettingsMenu {id parent} {
    global gShowMenu gShowLogo gShowStatus

    set mname $parent.settings
    menu $mname -type tearoff
    $mname add command -label "Dataset Settings..." -underline 0 \
	-accelerator "D" \
	-command "DatasetOptions::postDatasetOptionsDialog $id"
    $mname add separator

    $mname add command -label "Navigation Settings..." -underline 0 \
	-accelerator "N" \
	-command "NavOptions::postNavOptionsDialog $id"
    $mname add command -label "Render Settings..." -underline 0 \
	-accelerator "R" \
	-command "RenderOptions::postRenderOptionsDialog $id"
    $mname add command -label "3D Model Settings..." -underline 0 \
	-accelerator "V" \
	-command "Features::postFeatureOptionsDialog $id"

    # GeoWeb support is disabled in the Open Source release of 
    # TerraVision because the GeoWeb libraries are not open sourced
    #$mname add command -label "GeoWeb Settings..." -underline 0 \
    #	-accelerator "G" \
    #	-command "DotGeo::postDotGeoOptionsDialog $id"

    $mname add command -label "Doodle Settings..." -underline 1 \
	-command "AnnotateOptions::postAnnotateOptionsDialog $id"
    $mname add separator
    $mname add cascade -label "Window" -menu $mname.window -underline 0

    # window settings submenu...
    menu $mname.window
    $mname.window add checkbutton -label "Show menubar" -underline 5 \
	-accelerator "M" \
	-variable gShowMenu($id) -onvalue 1 -offvalue 0 \
	-command "showMenuCB $id"
    set gShowMenu($id) 1

    $mname.window add checkbutton -label "Show statusbar" -underline 5 \
	-accelerator "A" \
	-variable gShowStatus($id) -onvalue 1 -offvalue 0 \
	-command "showStatusCB $id"
    set gShowStatus($id) 1

    $mname.window add checkbutton -label "Show logo" -underline 5 \
	-accelerator "L" \
	-variable gShowLogo($id) -onvalue 1 -offvalue 0 \
	-command "showLogoCB $id"
    set gShowLogo($id) 1

    return $mname
}

#
# display a help page using a web browser. Here, we do some
# special magic to use the default web browser under Windows,
# otherwise, we fall back to the tsmUtil command showUrl.
#
proc showWebPage { url } {
    global tcl_platform
    global env
    if { $tcl_platform(platform) == "windows" } {
	if { [file exists $env(COMSPEC)] } {
	    exec $env(COMSPEC) /c start $url &
	} else {
	  tsmUtil showUrl $url
	}
    } else {
	tsmUtil showUrl $url
    }
}

#
# m a k e H e l p M e n u
#
proc makeHelpMenu {id parent} {

    set mname $parent.help
    menu $mname -type tearoff 
    $mname add command -label "Help on Navigation"  -underline 0 \
	-command "showWebPage http://www.tvgeo.com/user_guide/tcltk/chapter_nav.shtml"

    $mname add command -label "Help on Menu Options"  -underline 0 \
	-command "showWebPage http://www.tvgeo.com/user_guide/tcltk/welcome.shtml"

    $mname add command -label "Help on TerraVision" -underline 8 \
	-command "showWebPage http://www.tvgeo.com/user_guide/welcome.shtml"

    $mname add separator
    $mname add command -label About -underline 0 -command "aboutTV $id"
    # $mname add command -label License  -underline 0 -command "tvLicense"
    $mname add separator

    $mname add command -label Overview  -underline 0 \
	-command "showWebPage http://www.tvgeo.com/pubs/overview/overview.html"

    $mname add command -label "TV FAQ" -underline 0 \
	-command "showWebPage http://www.tvgeo.com/faq.shtml"

    $mname add command -label "TV Home Page" -underline 1 \
	-command "showWebPage http://www.tvgeo.com/"

    return $mname
}

#
# m a k e S t a t u s B a r 
#
proc makeStatusBar {parent} {

    frame $parent.f -borderwidth 2 -height 30 -relief raised

    frame $parent.f.f1 \
        -borderwidth 2 -height 23 -relief sunken -width 236 
    label $parent.f.f1.label \
	-borderwidth 1 -textvariable $parent.status.bookmark
    pack $parent.f.f1.label -side left -anchor w -expand 1

    frame $parent.f.f2 \
        -borderwidth 2 -height 23 -relief sunken -width 125 
    label $parent.f.f2.label1 \
        -borderwidth 1 -text "Datasets: " 
    label $parent.f.f2.label2 \
	-borderwidth 1 -textvariable $parent.status.datasets
    pack $parent.f.f2.label1 $parent.f.f2.label2 -side left -anchor center \
	-expand 1

    frame $parent.f.f3 \
        -borderwidth 2 -height 23 -relief sunken -width 125
    label $parent.f.f3.label1 \
        -borderwidth 1 -text "Nav Mode: "
    label $parent.f.f3.label2 \
	-borderwidth 1 -textvariable $parent.status.navmode
    pack $parent.f.f3.label1 $parent.f.f3.label2 -side left -anchor center \
	-expand 1

    frame $parent.f.f4 \
        -borderwidth 2 -height 23 -relief sunken -width 125 
    label $parent.f.f4.label1 \
        -borderwidth 1 -text "View Mode: "
    label $parent.f.f4.label2 \
	-borderwidth 1 -textvariable $parent.status.viewmode
    pack $parent.f.f4.label1 $parent.f.f4.label2 -side left -anchor center \
	-expand 1

    #pack 'em all into a status bar...
    pack $parent.f.f2 $parent.f.f3 $parent.f.f4 -side right -expand 0 -fill x
    pack $parent.f.f1 -side left -expand 1 -fill x

    return $parent.f
}

#
# s e t T V D e f a u l t s
#
proc setTVDefaults {id} {
    $id.tvbase set TV_SHOW_LOGO true
    $id.tvbase set TV_SHOW_SKY true
    $id.tvbase set TV_SHOW_CROSSHAIR 0
    $id.tvbase set TV_SHOW_LIGHTING 1
    $id.tvbase set TV_SHOW_HEADS_UP 1
    $id.tvbase set TV_SHOW_TEXTURES true
    $id.tvbase set TV_SHOW_GROUND true
    $id.tvbase set TV_SHOW_DATASETS 0
    $id.tvbase set TV_SHOW_TILES 1        ;# important for link->show_tiles !
    $id.tvbase set TV_MOTION_THRESHOLD [tvutils get TVU_DROP_TO_WIREFRAME]

    # make default entries in the global arrays for modes, etc.
    global gFlightMode gFlightControlMode
    set gFlightMode($id) "Fly"              ;# versus "FlyTo", "none", "doodle"
    set gFlightControlMode($id) "normal"    ;# versus "airplane"

    statusBarSetNavMode $id $gFlightMode($id)
    statusBarSetViewMode $id [$id.tvbase get TV_TWOD_CONSTRAIN]

    # initialise the GeoVRML features module
    Features::initFeatures $id

    # initialise the DotGeo interface
    DotGeo::initDotGeo $id
}

#
# s t a t u s B a r S e t V i e w N a m e
#
proc statusBarSetViewName {id name} {
    
    # need to construct the variable-name of the nav-field label widget...
    set nameLabel $id.status.bookmark
    global $nameLabel
    set $nameLabel $name
}

#
# s t a t u s B a r S e t N a v M o d e 
#
proc statusBarSetNavMode {id mode} {
    
    # need to construct the variable-name of the nav-field label widget...
    set navLabel $id.status.navmode
    global $navLabel
    set $navLabel $mode
}

#
# s t a t u s B a r S e t V i e w M o d e 
#
proc statusBarSetViewMode {id mode} {
    
    # need to construct the variable-name of the view-field label widget...
    set viewLabel $id.status.viewmode
    global $viewLabel

    if {$mode} {
	set labelText "2-D"
    } else {
	set labelText "3-D"
    }

    set $viewLabel $labelText
}

#######################
#  Callbacks          #
#######################

#
# f i l e M e n u C l o s e V i e w e r C B
#
proc fileMenuCloseViewerCB {inst} {
    global gWinList

    # if this is the last window, it becomes a quit...
    if {[llength $gWinList] == 1} {
	quitCB
	return
    }

    # remove this window from the global list of windows...
    set index [lsearch -exact $gWinList $inst]
    set gWinList [lreplace $gWinList $index $index]

    $inst.tvbase destroy
    destroy $inst

}

#
# q u i t T V
#
# call this routine to exit TerraVision without any questions
#
proc quitTV {} {
    
    # clean up any open viewers...
    global gWinList
    foreach window $gWinList {
	$window.tvbase destroy
	destroy $window
    }

    # shutdown any opened geosets...
    global gGeosetsList
    foreach geoset $gGeosetsList {
	tsGeoPyramid destroy $geoset
    }

    exit 0
}

#
# q u i t C B 
#
proc quitCB {} {
    
    if {[tk_messageBox -icon question -title "TerraVision Dialog" \
	     -type yesno -default no \
	     -message "Do you want to quit TerraVision?"] == "yes"} {

	 quitTV
    }
}

#
# h e l p C B
#
proc helpCB {urlFragment} {
    global gInstallDirectory

    if {$urlFragment == "null"} {
	set urlFragment "index.html"
    }

    # do we have help files locally?
    set url [format "%s/etc/help/%s" $gInstallDirectory $urlFragment]
    if {![file exists $url]} {
	# if can't find it locally, try to show it from home pages...
	set url \
	    [format "http://www.ai.sri.com/TerraVision/guide/%s" $urlFragment]
    }
    
    showWebPage $url
}

#
# d u m m y C B 
#
# Place holder for planned, but as yet unimplemented features.
#
proc dummyCB {id} {
    tk_messageBox -type ok -title "TerraVision Error" -icon warning \
	-message "Feature not yet implemented"
}

#
# a b o u t T V
#
# Display the Help/About window
#

proc aboutTV {id} {
    set vers [tvutils get TVU_TV_VERSION]
    set msg "TerraVision\n3D Terrain Visualization System\n\nVersion $vers\n"
    set msg "$msg\nhttp://www.tvgeo.com/\n\n"
	set msg "$msg\nReleased under the Mozilla Public License 1.1."
	set msg "$msg\n(c) SRI International 1993-2002."

    tk_messageBox -type ok -title "About TerraVision" -icon info \
    -message $msg
}

#
# s h o w M e n u C B 
#
# Turns a view-window's menubar on/off.
#
proc showMenuCB {id} {
    global gShowMenu
    if {[$id cget -menu] == ""} {
	$id configure -menu $id.menubar
	set gShowMenu($id) 1
    } else {
	$id configure -menu ""
	set gShowMenu($id) 0
    }
}

#
# s h o w S t a t u s C B
#
# Turns a view-window's status bar on/off.
#
proc showStatusCB {id} {
    global gShowStatus
    if {[winfo ismapped $id.f]} {
	pack forget $id.f
	set gShowStatus($id) 0
    } else {
	pack $id.f -side bottom -expand 1 -fill x
	set gShowStatus($id) 1
    }
}

#
# s h o w L o g o C B
#
# Toggles the SRI logo in a view-window.
#
proc showLogoCB {id} {
    global gShowLogo
    set val [$id.tvbase get TV_SHOW_LOGO]
    set gShowLogo($id) [expr ! $val]
    $id.tvbase set TV_SHOW_LOGO $gShowLogo($id)
}

##############################
#  Event handlers...         #
##############################


#
# B u t t o n 1 D o w n
#
proc Button1Down {id x y} {
    global gMouseDownX gMouseDownY gMoving gAltDown gFlightMode
    global gCtrlDown gShiftDown gDoodleMode gMouseDown
 
    set gMouseDown 1

    # no navigation during animation...
    if {[$id.tvbase path isPlaying]} { return }

    # doodle mode?
    if {[array names gDoodleMode $id] != ""} {
	$id.tvbase stroke begin $x $y
	set gMoving 1
	return
    }

    if {$gAltDown || $gFlightMode($id) == "FlyTo"} {
	$id.tvbase view flytoPick $x $y
	return
    }

    # have we clicked on a hyperlink?
    set link_result [$id.tvbase anchor click]
    if { $link_result != 0 } {
	set gMouseDown 0
	return
    }

    # Otherwise, we're navigating...    
    # record these into globals for further flight control...
    set gMouseDownX $x
    set gMouseDownY $y
    set gMoving 1
    
    $id.tvbase view accelerate
    
    $id.tvbase set TV_CROSSHAIR $x $y
    $id.tvbase set TV_SHOW_CROSSHAIR true
    statusBarSetViewName $id ""
}

#
# B u t t o n M o t i o n 
#
# Handles motion events for all three buttons.
#
proc ButtonMotion {id x y} {
    global gMouseDownX gMouseDownY gShiftDown gCtrlDown gAltDown
    global gMoving gFlightControlMode gDoodleMode gMouseDown

    # don't return if not moving - need to check for hyperlinks
    # if {!$gMoving} {return}

    # doodle mode?
    if {[array names gDoodleMode $id] != ""} {
	$id.tvbase stroke continue $x $y
	return
    }

    if { $gShiftDown || $gCtrlDown || $gAltDown || $gMouseDown } {

	set width  [lindex [split [$id.tvbase configure -width]] 4]
	set height [lindex [split [$id.tvbase configure -height]] 4]

	# if {![string is integer $width]} {
	#    puts [ format "Width is not an integer? (%s,%s)" $width $height ]
	# }

	# if { $width < 2 } {
	#     puts [ format "Width is real small! (%s,%s)" $width $height ]
	# } elseif { $width > 1300 } {
	#     puts [ format "Width is real big! (%s,%s)" $width $height ]
	# }
    
	set scale $width
	if {$width < $height} {
	    set scale $height
	}
	set scale [expr $scale * 0.5]

	if { ![string is double $scale] } {
	    puts [format "Scale is not a double!! (%s), [%sx%s]" \
		      $scale $width $height ]
	}

	# if { $scale == 0.0 } {
	#    puts "Scale is zero!!!"
	# }

	set deltaX [expr ($x - $gMouseDownX) / $scale]
	set deltaY [expr -($y - $gMouseDownY) / $scale]

	if {$gShiftDown || $gCtrlDown || [$id.tvbase get TV_TWOD_CONSTRAIN]} {
	    $id.tvbase set TV_SLIDE_DELTA $deltaX $deltaY
	} else {
	    if {$gFlightControlMode($id) != "airplane"} {
		set deltaY [expr -1.0 * $deltaY]
	    }
	    $id.tvbase set TV_ATTITUDE_DELTA $deltaX $deltaY
	}

	# clear viewpoint field of status bar - if we have moved,
	# then we are not at the named viewpoint anymore
	statusBarSetViewName $id ""

    } else {
	
	# implement hyperlinks...
	set link_result [$id.tvbase anchor move $x $y]
        if { [string length $link_result] > 0 } {
	    statusBarSetViewName $id $link_result
	}
    }
        
}

#
# B u t t o n U P
#
# Handles up events for both buttons 1 and 3.
#
proc ButtonUp {id x y} {
    global gAltDown gFlightMode gMoving gDoodleMode gMouseDown

    # doodle mode?
    if {[array names gDoodleMode $id] != ""} {
	$id.tvbase stroke end $x $y
	set gMoving 0
	return
    }

    # no navigation during animation...
    if {$gAltDown || $gFlightMode($id) == "FlyTo" || 
	[$id.tvbase path isPlaying]} { 
	return 
    }

    set link_result [$id.tvbase anchor release]
    if { $link_result != 0 } {
	statusBarSetViewName $id ""
	return;
    }

    # Otherwise, we're navigating... 
    $id.tvbase view startMovement

    ## /* Update the map view with the new position */  -- TBDone
    ## ScannedMapRedraw();
}

#
# B u t t o n 2 D o w n 
#
proc Button2Down {id x y} {
    global gMouseDownX gMouseDownY gMoving gDoodleMode gMouseDown

    set gMouseDown 1

    # no navigation during animation...
    if {[$id.tvbase path isPlaying]} { return }

    # doodle mode?
    if {[array names gDoodleMode $id] != ""} {
	$id.tvbase stroke begin $x $y
	set gMoving 1
	return
    }

    # we're navigating...Set speed back to zero when we start...
    $id.tvbase set TV_VELOCITY 0.
    $id.tvbase set TV_ACCELERATION 0.

    set gMouseDownX $x
    set gMouseDownY $y
    set gMoving 1
    
    $id.tvbase view startMovement
    
    $id.tvbase set TV_CROSSHAIR $x $y
    $id.tvbase set TV_SHOW_CROSSHAIR true
    statusBarSetViewName $id ""
}

#
# B u t t o n 2 U p
#
proc Button2Up {id x y} {
    global gMoving gDoodleMode gMouseDown

    set gMoving 0
    set gMouseDown 0

    # doodle mode?
    if {[array names gDoodleMode $id] != ""} {
	$id.tvbase stroke end $x $y
	return
    }

    $id.tvbase set TV_SHOW_CROSSHAIR false
    if {[$id.tvbase path isPlaying]} { return }

    $id.tvbase view stopMovement

    ##  /* Update the map view with the new position */  --TBDone
    ##  ScannedMapRedraw(); 
}

#
# B u t t o n 3 D o w n 
#
proc Button3Down {id x y} {
    global gMouseDownX gMouseDownY gMoving gDoodleMode gMouseDown

    set gMouseDown 1

    # no navigation during animation...
    if {[$id.tvbase path isPlaying]} { return }

    # doodle mode?
    if {[array names gDoodleMode $id] != ""} {
	$id.tvbase stroke begin $x $y
	set gMoving 1
	return
    }

    # Otherwise, we're navigating...
    # record these into globals for further flight control...
    set gMouseDownX $x
    set gMouseDownY $y
    set gMoving 1
    
    $id.tvbase view deccelerate
    
    $id.tvbase set TV_CROSSHAIR $x $y
    $id.tvbase set TV_SHOW_CROSSHAIR true
    statusBarSetViewName $id ""
}

#
# m a i n W i n B i n d i n g s 
#
proc mainWinBindings {id} {
    global gShiftDown gCtrlDown gAltDown
    global gFlightMode

    # NOTE: most (all?) of the events we're interested in happen in the Togl 
    # widget.
    set widget $id.tvbase

    # button events...

    set bindscript "eval \{ Button1Down $id %x %y \}"
    bind $widget <ButtonPress-1> $bindscript

    set bindscript "eval \{ ButtonUp $id %x %y \}"
    bind $widget <ButtonRelease-1> $bindscript

    set bindscript "eval \{ Button2Down $id %x %y \}"
    bind $widget <ButtonPress-2> $bindscript

    set bindscript "eval \{ Button2Up $id %x %y \}"
    bind $widget <ButtonRelease-2> $bindscript

    set bindscript "eval \{ Button3Down $id %x %y \}"
    bind $widget <ButtonPress-3> $bindscript

    set bindscript "eval \{ ButtonUp $id %x %y \}"
    bind $widget <ButtonRelease-3> $bindscript

    set bindscript "eval \{ ButtonMotion $id %x %y \}"
    bind $widget <Motion> $bindscript

    # keyboard events...
    focus $widget           ;# VERY IMPORTANT...

    foreach key {Alt_L Alt_R} {   ;# kinda clumsy...
	set bindscript "eval \{set gAltDown 1 ;
                               statusBarSetNavMode $id \"FlyTo\"
                             \}"
	bind $widget <KeyPress-$key> $bindscript
    }

    foreach key {Alt_L Alt_R} {
	set bindscript \
            "eval \{set gAltDown 0 ;
                    statusBarSetNavMode $id \$gFlightMode($id) ;
                  \}"
	bind $widget <KeyRelease-$key> $bindscript
    }
    
    foreach key {Shift_L Shift_R Control_L Control_R} { 
	set bindscript "eval \{  if \{\[string compare \"Shift\" %K\] > 0\} \{
                                    set gCtrlDown 1 ;
	                         \} else \{ 
		                    set gShiftDown 1
	                         \}
	                         statusBarSetNavMode $id \"Slide\"
                             \}"
	bind $widget <KeyPress-$key> $bindscript
    }

    
    foreach key {Shift_L Shift_R Control_L Control_R} {
	set bindscript \
            "eval \{ \
                      if \{\[string compare \"Shift\" $key\] > 0\} \{
                         set gCtrlDown 0;
                      \} else \{ 
                         set gShiftDown 0; 
                      \}
                      statusBarSetNavMode $id \$gFlightMode($id)
                  \}"
	bind $widget <KeyRelease-$key> $bindscript 
    }

    ###
    ### The following key sequences are currently unimplemented, and were
    ### obtained by examining code in XmTV/ThreeDWidget.c
    ###
    ### W/w toggles "web icon" panel on/off -- don't have that here
    ### F/f toggles full screen mode -- window manager does this better than I
    ### V/v VRML feature selection window
    ### I/i dataset priority -- this menu option was collapsed into another
    ### "-" toggle z-buffer hack
    ### S/s toggle stereo mode
    ### Z/z toggle stereo-monitor 
    ### PageUp:  fly to next bookmark in list
    ### PageDn:  fly to prev bookmark

    # menu shortcut to "File:CloseViewer"...
    set bindscript "eval \{ fileMenuCloseViewer $id \}"
    bind $widget <Escape> $bindscript

    # set viewer to 2D mode...
    set bindscript "eval \{
                           $id.tvbase set TV_TWOD_CONSTRAIN 1 ;
                           set gViewMode($id) 1 ;
                           statusBarSetViewMode $id 1 ;
                         \}"
    bind $widget 2 $bindscript

    # set viewer to 3D mode...
    set bindscript "eval \{
                           $id.tvbase set TV_TWOD_CONSTRAIN 0 ;
                           set gViewMode($id) 0 ;
                           statusBarSetViewMode $id 0 ;
                         \}"
    bind $widget 3 $bindscript

    # remove the status bar...
    foreach key {a A} {
	set bindscript "eval \{showStatusCB $id \}"
	bind $widget $key $bindscript
    }

    # post view bookmarks dialog...
    set bindscript "eval \{ViewBookmarks::postBookmarksDialog $id \}"
    foreach key {b B} {
	bind $widget $key $bindscript
    }

    # post view controls dialog....
    set bindscript "eval \{ DatasetDialog::postConsole \}"
    foreach key {c C} {
	bind $widget $key $bindscript
    }

    # post debug options dialog....
    set bindscript "eval \{ DatasetOptions::postDatasetOptionsDialog $id \}"
    foreach key {d D} {
	bind $widget $key $bindscript
    }

    # toggle head-up display...
    foreach key {h H} {
	set bindscript "eval \{
                           set val \[$id.tvbase get TV_SHOW_HEADS_UP\] ;
                           set val \[expr ! \$val\] ;
                           $id.tvbase set TV_SHOW_HEADS_UP \$val ;
                         \}"
	bind $widget $key $bindscript
	set bindscript "eval \{
                           set val \[$id.tvbase get TV_SHOW_HEADS_UP_DEBUG\] ;
                           set val \[expr ! \$val\] ;
                           $id.tvbase set TV_SHOW_HEADS_UP_DEBUG \$val ;
                         \}"
	bind $widget <Shift-KeyPress-$key> $bindscript
    }
                 
    # toggle SRI logo...
    foreach key {l L} {
	set bindscript "eval \{showLogoCB $id \}"
	bind $widget $key $bindscript
    }

    # toggle menubar...
    foreach key {m M} {
	set bindscript "eval \{ showMenuCB $id \}"
	bind $widget $key $bindscript
    }

    # post navigation-options dialog...
    set bindscript "eval \{NavOptions::postNavOptionsDialog $id\}"
    foreach key {n N} {
	bind $widget $key $bindscript
    }

    # post view-status dialog...
    set bindscript "eval \{ViewControls::postViewStatusDialog $id\}"
    foreach key {p P} {
	bind $widget $key $bindscript
    }

    # post rendering-options dialog...
    set bindscript "eval \{RenderOptions::postRenderOptionsDialog $id\}"
    foreach key {r R} {
	bind $widget $key $bindscript
    }

    # post 3d model options dialog...
    set bindscript "eval \{Features::postFeatureOptionsDialog $id\}"
    foreach key {v V} {
	bind $widget $key $bindscript
    }

    # post 3d model options dialog...
    set bindscript "eval \{DatasetDialog::postServers\}"
    foreach key {s S} {
	bind $widget $key $bindscript
    }

    # post dotgeo options dialog...
    set bindscript "eval \{DotGeo::postDotGeoOptionsDialog $id\}"
    foreach key {g G} {
	bind $widget $key $bindscript
    }

    # return to Origin viewpoint...
    set bindscript "eval \{$id.tvbase view origin\}"
    foreach key {o O} {
	bind $widget $key $bindscript
    }

    # space means stop moving
    set bindscript "eval \{ Button2Down $id 0 0 ; Button2Up $id 0 0 \}"
    bind $widget <KeyPress-space> $bindscript

    # left-arrow navigation commands...
    # XXX reddy - KP_Left not supported under Windows?
    foreach key {Left} {
	# move left
	set bindscript "eval \{viewMove $id 0 -10 0\}"
	bind $widget <KeyPress-$key> $bindscript
	# shift/ctrl = rotate left
	set bindscript "eval \{viewRotate $id 1 0 0\}"
	bind $widget <Shift-KeyPress-$key> $bindscript
	bind $widget <Control-KeyPress-$key> $bindscript
    }

    # right-arrow navigation commands...
    # XXX reddy - KP_Right not supported under Windows?
    foreach key {Right} {
	# move right
	set bindscript "eval \{viewMove $id 0 10 0\}"
	bind $widget <KeyPress-$key> $bindscript
	# shift/ctrl = rotate right
	set bindscript "eval \{viewRotate $id -1 0 0\}"
	bind $widget <Shift-KeyPress-$key> $bindscript
	bind $widget <Control-KeyPress-$key> $bindscript
    }

    # up-arrow navigation commands...
    # XXX reddy - KP_Up not supported under Windows?
    foreach key {Up} {
	# move up
	set bindscript "eval \{viewMove $id 0 0 10\}"
	bind $widget <KeyPress-$key> $bindscript
	# ctrl = zoom in
	set bindscript "eval \{viewMove $id 20 0 0\}"
	bind $widget <Control-KeyPress-$key> $bindscript
	# shift = rotate up
	set bindscript "eval \{viewRotate $id 0 1 0\}"
	bind $widget <Shift-KeyPress-$key> $bindscript
    }

    # down-arrow navigation commands...
    # XXX reddy - KP_Down not supported under Windows?
    foreach key {Down} {
	# move down
	set bindscript "eval \{viewMove $id 0 0 -10\}"
	bind $widget <KeyPress-$key> $bindscript
	# ctrl = zoom out
	set bindscript "eval \{viewMove $id -20 0 0\}"
	bind $widget <Control-KeyPress-$key> $bindscript
	# shift = rotate down
	set bindscript "eval \{viewRotate $id 0 -1 0\}"
	bind $widget <Shift-KeyPress-$key> $bindscript
    }

    foreach key {p P} {
	set bindscript "eval \{Paths::playStopContPauseCmd $id play\}"
	bind $widget <Control-KeyPress-$key> $bindscript
    }

    foreach key {s S} {
	set bindscript "eval \{Paths::playStopContPauseCmd $id stop\}"
	bind $widget <Control-KeyPress-$key> $bindscript
    }

    foreach key {u U} {
	set bindscript "eval \{Paths::playStopContPauseCmd $id continue\}"
	bind $widget <Control-KeyPress-$key> $bindscript
    }

    foreach key {t T} {
	set bindscript "eval \{Paths::gotoCmd $id start\}"
	bind $widget <Control-KeyPress-$key> $bindscript
    }

    foreach key {n N} {
	set bindscript "eval \{Paths::gotoCmd $id next\}"
	bind $widget <Control-KeyPress-$key> $bindscript
    }

    foreach key {v V} {
	set bindscript "eval \{Paths::gotoCmd $id prev\}"
	bind $widget <Control-KeyPress-$key> $bindscript
    }

    foreach key {e E} {
	set bindscript "eval \{Paths::gotoCmd $id end\}"
	bind $widget <Control-KeyPress-$key> $bindscript
    }

    foreach key {o O} {
	set bindscript "eval \{Paths::gotoCmd $id end\}"
	bind $widget <Control-KeyPress-$key> $bindscript
    }

    # menu shortcut to "File:Quit"...
    foreach key {q Q} {
	bind $widget <Control-$key> {quitCB}
    }

    # set up a handler for the case when the user closes
    # the window by clicking on the close window button
    # rather than selecting the File/Exit option
    wm protocol $id WM_DELETE_WINDOW quitTV

    # set up the idle task to listen for messages sent
    # to us from the C interface after 0.5 secs
    after 500 idleCallback
}

proc idleCallback {} {
    set message [tvutils msg]

    set mode [lindex $message 0]
    if { $mode != "NONE" } {
	set id   [lindex $message 1]
	set msg  [lindex $message 2]
	set opt  [lindex $message 3]

	if { $mode == "STATUS" } {
	    statusBarSetViewName $id $msg
	} elseif { $mode == "MESSAGE" } {
	    tk_messageBox -type ok -icon info -title "TerraVision Message" \
		-message $msg
	} elseif { $mode == "ERROR" } {
	    tk_messageBox -type ok -icon error -title "TerraVision Error" \
		-message $msg
	} elseif { $mode == "BUSY" } {
	} elseif { $mode == "TERRAIN_ADD" } {

	    # do some sanity checks on the geopyramid name,
	    # then add it to our known list of open datasets
	    if { [ string length $msg ] > 0 } {
		DatasetDialog::loadedGeoset $msg
	    }

	} elseif { $mode == "TERRAIN_DEL" } {
	} elseif { $mode == "MODEL_ADD" } {

	    # do some sanity checks on the VRML model name,
	    # then add it to all our known list of open models
	    if { [ string length $msg ] > 0 } {
		global gWinList
		foreach window $gWinList {
		    Features::loadedModel $window $msg
		}
	    }

	} elseif { $mode == "MODEL_DEL" } {
	} elseif { $mode == "NOTIFY" } {
	} elseif { $mode == "CONSOLE" } {
	    DatasetDialog::addToConsole $msg
	}
    }

    # call this idle task again in 0.2 seconds time
    after 200 idleCallback
}

proc viewMove {id x y z} {
    set view [$id.tvbase view current]

    if {[$id.tvbase get TV_HEIGHT_BASED_NAVIGATION] == 1} {
	set moveFactor [expr 1.0/5.0 * [lindex $view 1] / 100.0]
    } else {
	set moveFactor 1.0
    }

    set x [expr $x * $moveFactor]
    set y [expr $y * $moveFactor]
    set z [expr $z * $moveFactor]
    $id.tvbase view move $x $y $z
}

proc viewRotate {id h p r} {
    set h [deg2rad $h]
    set p [deg2rad $p]
    set r [deg2rad $r]
    $id.tvbase view rotate $h $p $r
}

# 
# TerraVision License routines
# 
# These routines will manage the displaying of the TerraVision license
# and the resulting operations when the user accepts or rejects the
# license. The license window is "modal" so that no other TerraVision
# function can be accessed until an answer is given.
# 

proc acceptLicense {win} {
    set currver [tvutils get TVU_TV_VERSION]
    tvutils config add1 license $currver
    tvutils config save
    destroy $win
}

proc rejectLicense {win} {
    tvutils config add1 license no
    tvutils config save
    quitTV
}

proc keepLicenseRaised win {
    if {[winfo exists $win]} {
	raise $win
	focus $win
	after 1000 [info level 0]
    }
}

proc tvLicense {} {
    set lic "\nReleased under the Mozilla Public License (MPL 1.1)"
    set w .tvlicense
    catch {destroy $w}
    toplevel $w
    wm title $w "TerraVision License"
    wm iconname $w "License"
    set c $w.c

    frame $w.buttons
    pack $w.buttons -side bottom -fill x -pady 2m
    button $w.buttons.dismiss -text "Okay" -command "acceptLicense $w"
    # button $w.buttons.code -text "I Disagree" -command "rejectLicense $w"
    # pack $w.buttons.dismiss $w.buttons.code -side left -expand 1
    pack $w.buttons.dismiss -side left -expand 1

    frame $w.grid
    scrollbar $w.hscroll -orient horiz -command "$c xview"
    scrollbar $w.vscroll -command "$c yview"
    canvas $c -relief sunken -borderwidth 2 -width 580 -height 480 \
	    -scrollregion {0 0 580 1200} \
	    -xscrollcommand "$w.hscroll set" \
	    -yscrollcommand "$w.vscroll set" -background white
    pack $w.grid -expand yes -fill both -padx 1 -pady 1

    grid rowconfig    $w.grid 0 -weight 1 -minsize 0
    grid columnconfig $w.grid 0 -weight 1 -minsize 0

    grid $c -padx 1 -in $w.grid -pady 1 \
	    -row 0 -column 0 -rowspan 1 -columnspan 1 -sticky news
    grid $w.vscroll -in $w.grid -padx 1 -pady 1 \
	    -row 0 -column 1 -rowspan 1 -columnspan 1 -sticky news
    grid $w.hscroll -in $w.grid -padx 1 -pady 1 \
	    -row 1 -column 0 -rowspan 1 -columnspan 1 -sticky news

    $c create text 0 0 -text $lic -anchor nw -tags text -font {courier 10}

    focus $w
    grab $w
    raise $w
    bind $w <Unmap> { wm deiconify .tvlicense }
    keepLicenseRaised $w
}
