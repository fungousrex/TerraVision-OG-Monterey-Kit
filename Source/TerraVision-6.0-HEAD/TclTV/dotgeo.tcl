#
# dotgeo.tcl
#

namespace eval DotGeo {
    namespace export postOpenDotGeoDialog initDotGeo postDotGeoOptionsDialog \
	dotGeoDisconnect

    #
    # module globals...
    # These are all associative arrays holding the state of the rendering 
    # options for Model Loading dialogs, and indexed by the instanceID
    #

    variable connectedHost           ;# name of current .geo connection
    variable schemaField             ;# the schema URL entry widget
    variable schemaUrl               ;# the default schema URL string
    # variable hierarchyField        ;# the hierarchy URL entry widget
    # variable hierarchyUrl          ;# the default hierarchy URL string
    variable profileField            ;# the profile URL entry widget
    variable maxResultsField         ;# the max results returned by .geo
    variable delayField              ;# the ms delay between .geo queries
    variable infoField               ;# the connected to text field
    variable connectButton           ;# the "Connect" button
    variable disconnectButton        ;# the "Disconnect" button

    #
    # i n i t D o t G e o
    # 
    # initialises all of the DotGeo state for a TerraVision viewer
    #
    proc initDotGeo {id} {
	upvar DotGeo::connectedHost($id) connectedHost
	upvar DotGeo::schemaUrl($id) schemaUrl
	# upvar DotGeo::hierarchyUrl($id) hierarchyUrl

	set schemaUrl [tvutils get TVU_SCHEMA_URL]
	# set hierarchyUrl [tvutils get TVU_HIERARCHY_URL]

	if { [string length $schemaUrl] < 1 } {
	    set schemaUrl "http://www.dotgeo.net/schema.xml"
	    set connectedHost ""
	} else {
	    set connectedHost $schemaUrl
	}
    }

    #
    # u p d a t e D o t G e o W i d g e t s
    #
    # updates the state of the Disconnect button and the status
    # bar text on the Open DotGeo dialog
    #
    proc updateDotGeoWidgets {id} {
	upvar DotGeo::connectedHost($id) connectedHost
	upvar DotGeo::schemaField($id) schemaField
	upvar DotGeo::infoField($id) infoField
	upvar DotGeo::connectButton($id) connectButton
	upvar DotGeo::disconnectButton($id) disconnectButton

	# make the toplevel window and set various window-options...
	set topname [format "%s_opengeo" $id ]

	# don't do anything if the window is not open
	if { ! [winfo exists $topname] } return;

	if { [string length $connectedHost] > 0 } {
	    $infoField configure -text "Status: Connected with $connectedHost"
	    $disconnectButton configure -state normal
	} else {
	    $infoField configure -text "Status: Not connected"
	    $disconnectButton configure -state disabled
	}
    }

    #
    # c o n n e c t C B
    #
    # callback to connect to a DotGeo Schema
    #
    proc connectCB {id win} {
	upvar DotGeo::connectedHost($id) connectedHost
	upvar DotGeo::schemaField($id) schemaField
	# upvar DotGeo::hierarchyField($id) hierarchyField

	set schema [string trim [$schemaField get]]
	# set hierarchy [string trim [$hierarchyField get]]

	# check that we have a schema URL
	
	if { [string length $schema] < 1 } {
	    tk_messageBox -title "TerraVision Error" -type ok -icon error \
		    -message "You have not specified a schema URL!"
	    return;
	}

	# connect to the new schema URLs

	$win configure -cursor watch
	update

	$id.tvbase set TV_SPLASH_MESSAGE "Querying .geo for datasets..."
	# $id.tvbase set TV_HIERARCHY_URL $hierarchy
	$id.tvbase set TV_SCHEMA_URL $schema

	#$win configure -cursor arrow
	$win configure -cursor [$id cget -cursor]

	set connectedHost $schema

	# udpate the dialog widgets (only if not closing the window)
	#updateDotGeoWidgets $id

	# close the window

	DotGeo::dismissCB $id $win
    }

    #
    # d i s m i s s C B
    #
    # Close the Open DotGeo window
    #
    proc dismissCB {id win} {
	upvar DotGeo::schemaField($id) schemaField
	upvar DotGeo::schemaUrl($id) schemaUrl
	# upvar DotGeo::hierarchyField($id) hierarchyField
	# upvar DotGeo::hierarchyUrl($id) hierarchyUrl

	# store the new strings as the default when opening this dialog

	set schemaUrl [string trim [$schemaField get]]
	# set hierarchyUrl [string trim [$hierarchyField get]]

	# close the window

	destroy $win
    }
    
    #
    # d i s c o n n e c t C B
    #
    # callback to disconnect from a DotGeo schema
    #
    proc dotGeoDisconnect {id} {
	upvar DotGeo::connectedHost($id) connectedHost
	upvar DotGeo::schemaField($id) schemaField
	# upvar DotGeo::hierarchyField($id) hierarchyField

	# do the disconnect

	# $id.tvbase set TV_HIERARCHY_URL ""
	$id.tvbase set TV_SCHEMA_URL ""

	set connectedHost ""

	# udpate the dialog widgets
	
	updateDotGeoWidgets $id
    }
    proc disconnectCB {id win} {

	# do the disconnect

	$win configure -cursor watch
	update

	dotGeoDisconnect $id

	#$win configure -cursor arrow
	$win configure -cursor [$id cget -cursor]

    }

    #
    # p o s t O p e n D o t G e o D i a l o g
    #
    # This is the main GUI for connecting/disconnecting to DotGeo
    #
    proc postOpenDotGeoDialog {id} {
	upvar DotGeo::schemaField($id) schemaField
	upvar DotGeo::schemaUrl($id) schemaUrl
	# upvar DotGeo::hierarchyField($id) hierarchyField
	# upvar DotGeo::hierarchyUrl($id) hierarchyUrl
	upvar DotGeo::infoField($id) infoField
	upvar DotGeo::connectButton($id) connectButton
	upvar DotGeo::disconnectButton($id) disconnectButton

	# make the toplevel window and set various window-options...
	set topname [format "%s_opengeo" $id ]

	# don't want more than one of these...
	if {[winfo exists $topname] == 1} {
	    wm withdraw $topname
	    wm deiconify $topname
	    return
	}
	set top [toplevel $topname]
	wm title $top [format "Open GeoWeb: Viewer %d" [instnum $id]]

	frame $top.f1 -relief raised -borderwidth 2
	frame $top.f1.input
	set frm $top.f1

	label $frm.l1_title -text "Schema URL: " \
	    -justify right -padx 5 -pady 5 
	entry $frm.l1_value -width 45
	set schemaField $frm.l1_value
	grid $frm.l1_title $frm.l1_value -sticky we

#	label $frm.l2_title -text "Hierarchy URL: " \
#	    -justify right -padx 5 -pady 5
#	entry $frm.l2_value -width 45
#	set hierarchyField $frm.l2_value
#	grid $frm.l2_title $frm.l2_value -sticky we

	pack $frm -ipadx 10 -ipady 4 -fill both

	frame $top.info -relief raised -borderwidth 2
	label $top.info.text -justify left -background grey
	set infoField $top.info.text
	pack $top.info.text -expand 1 -fill x -side left
	pack $top.info -side top -fill x -expand 1

	$schemaField insert 0 $schemaUrl
	# $hierarchyField insert 0 $hierarchyUrl

	frame $top.buttons
	button $top.buttons.goto -text "  Connect  " \
	    -command "DotGeo::connectCB $id $top"

	button $top.buttons.settings -text " Settings " \
	    -command "DotGeo::postDotGeoOptionsDialog $id"

	button $top.buttons.discon -text "Disconnect" \
	    -command "DotGeo::disconnectCB $id $top"

	button $top.buttons.dismiss -text "Dismiss" \
	    -command "DotGeo::dismissCB $id $top"

	set connectButton $top.buttons.goto
	set disconnectButton $top.buttons.discon

	pack $top.buttons.goto -side left -expand 1 -fill x
	pack $top.buttons.discon -side left -expand 1 -fill x
	pack $top.buttons.settings -side left -expand 1 -fill x
	pack $top.buttons.dismiss -side left -expand 1 -fill x
	pack $top.buttons -side bottom -expand 1 -fill x

	updateDotGeoWidgets $id

	# make it visible...
	wm deiconify $top
    }

    #
    # p o s t D o t G e o O p t i o n s D i a l o g
    #
    proc postDotGeoOptionsDialog {id} {
	upvar DotGeo::profileField profileField
	upvar DotGeo::maxResultsField maxResultsField
	upvar DotGeo::delayField delayField

	# make the toplevel window and set various window-options...
	set topname [format "%s_dotgeo_settings" $id ]

	# don't want more than one of these...
	if {[winfo exists $topname] == 1} {
	    wm withdraw $topname
	    wm deiconify $topname
	    return
	}

	set top [toplevel $topname]
	wm title $top [format "GeoWeb Options: Viewer %d" [instnum $id]]
	    
	# topmost frame of checkboxes...
	frame $top.f1 -borderwidth 2 -relief raised

	set lframe $top.f1.left
	frame $lframe 

	# -state can be normal, active, or disabled
	checkbutton $lframe.01 -text "Compress Results" \
	    -command ""

	checkbutton $lframe.02 -text "Cache Cell Queries" \
	    -command ""

	checkbutton $lframe.03 -text "Query To Top" \
	    -command ""

	setButton $lframe.01 [$id.tvbase get TV_DOTGEO_COMPRESS]
	setButton $lframe.02 [$id.tvbase get TV_DOTGEO_CACHE_CELLS]
	setButton $lframe.03 [$id.tvbase get TV_DOTGEO_QUERY_TO_TOP]

	pack $lframe.01 $lframe.02 $lframe.03 -side top -anchor w

	set rframe $top.f1.right
	frame $rframe 

        frame $rframe.01 -borderwidth 1
	label $rframe.01.title -text "Profile: " \
	    -justify right -padx 5 -pady 5
	entry $rframe.01.value -width 20 -validate focusout \
	    -vcmd "DotGeo::setProfile $id"
	set profileField $rframe.01.value
	button $rframe.01.btn -text "..." \
	    -command "DotGeo::selectProfile $id" -padx 4 -pady 0
	grid $rframe.01.title $rframe.01.value $rframe.01.btn -sticky we

        frame $rframe.02 -borderwidth 1
	label $rframe.02.title -text "Secs between queries: " \
	    -justify right -padx 5 -pady 5
	entry $rframe.02.value -width 6 -validate focusout \
	    -vcmd "DotGeo::setDelay $id"
	set delayField $rframe.02.value
	grid $rframe.02.title $rframe.02.value -sticky we

        frame $rframe.04 -borderwidth 1
	label $rframe.04.title -text "Max Results: " \
	    -justify right -padx 5 -pady 5
	entry $rframe.04.value -width 6 -validate focusout \
	    -vcmd "DotGeo::setMaxResults $id"
	set maxResultsField $rframe.04.value
	grid $rframe.04.title $rframe.04.value -sticky we

	set delay [expr [$id.tvbase get TV_DOTGEO_PAUSE] / 1000]
	$maxResultsField insert 0 [$id.tvbase get TV_DOTGEO_MAX_RESULTS]
	$delayField insert 0 $delay
	$profileField insert 0 [$id.tvbase get TV_DOTGEO_PROFILE_URL]

	pack $rframe.01 $rframe.02 $rframe.04 -side top -anchor w

	pack $lframe $rframe -side left

	button $top.dismiss -text "Dismiss" -relief raised \
            -command "destroy $topname"

	pack $top.f1 -side top -expand 1 -fill both
	pack $top.dismiss -side bottom -expand 1 -fill x
    }

    #
    # s e l e c t P r o f i l e
    #
    # Let's the user select a new .geo profile file
    #
    proc selectProfile {id} {
	upvar DotGeo::profileField profileField

	# get the database filename from the user...
	set file [tk_getOpenFile -initialdir [tvutils get TVU_USER_DIR] \
		      -defaultextension .xml -title "Load a GeoWeb Profile"]
	
	# non-empty string indicates "something" was returned
	if {[string length $file]} {
	    $profileField delete 0 end
	    $profileField insert 0 $file
	    setProfile $id
	}
    }

    #
    # s e t M a x R e s u l t s
    #
    proc setMaxResults {id} {
	upvar DotGeo::maxResultsField maxResultsField
	set value [$maxResultsField get]
	if { $value < 10 || ![string is integer $value]} {
	    set value "10"
	} 
	$maxResultsField delete 0 end
	$maxResultsField insert 0 $value
	$id.tvbase set TV_DOTGEO_MAX_RESULTS $value
	return 1;
    }
    
    #
    # s e t D e l a y
    #
    proc setDelay {id} {
	upvar DotGeo::delayField delayField
	set value [$delayField get]
	if { $value < 1 || ![string is double $value]} {
	    set value "1.0"
	} 
	$delayField delete 0 end
	$delayField insert 0 $value
        set msvalue [format "%1.0f" [expr $value * 1000.0]]
	$id.tvbase set TV_DOTGEO_PAUSE $msvalue
	return 1;
    }
    
    #
    # s e t P r o f i l e
    #
    proc setProfile {id} {
	upvar DotGeo::profileField profileField
	set file [$profileField get]

	$id.tvbase set TV_DOTGEO_PROFILE_URL $file

	# check whether we could load the profile okay
	set actual_file [$id.tvbase get TV_DOTGEO_PROFILE_URL]
	tk_messageBox -message "Curr Profile = $actual_file"
	if { [string length $actual_file] == 0 && [string length $file] > 0 } {
	    tk_messageBox -title "TerraVision Error" -icon error \
		-message "Specified GeoWeb Profile is Not Valid!"
	    $profileField delete 0 end
	}

	return 1;
    }
    
    #
    # c h e c k b o x C B 
    #
    proc checkboxCB {id widget tvVariable} {
	return;
	set state [$id.tvbase get $tvVariable]
	set state [expr !$state]
	setButton $widget $state
	$id.tvbase set $tvVariable $state
    }

    #
    # s e t B u t t o n 
    #
    proc setButton {widget value} {
	if {$value == 0} {
	    $widget deselect
	} else {
	    $widget select
	}
    }

}
