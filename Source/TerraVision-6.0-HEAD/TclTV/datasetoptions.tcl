#
# datasetoptions.tcl
#
# Dialogs and controls for dataset priorities and visibilities.
#

namespace eval DatasetOptions {

    namespace export postDatasetOptionsDialog

    # instance variables...
    variable geosetsListbox
    variable visibilityBtn
    variable upBtn
    variable downBtn
    variable zbHackBtn
    variable popFrontBtn
    variable observerToken

    # namespace static variables...
    variable visOffStr "\[    \]  "
    variable visOnStr  "\[ X \]  "

    #
    # p o s t D a t a s e t O p t i o n s D i a l o g
    #
    proc postDatasetOptionsDialog {id} {
	global gGeosetsList
	upvar DatasetOptions::geosetsListbox($id) geosetsListbox
	upvar DatasetOptions::visibilityBtn($id)  visibilityBtn
	upvar DatasetOptions::upBtn($id)          upBtn
	upvar DatasetOptions::downBtn($id)        downBtn
	upvar DatasetOptions::zbHackBtn($id)      zbHackBtn
	upvar DatasetOptions::popFrontBtn($id)    popFrontBtn
	upvar DatasetOptions::observerToken($id)  observerToken

	# make the toplevel window and set various window-options...
	set topname [format "%s_dod" $id ]

	# don't want more than one of these...
	if {[winfo exists $topname] == 1} {
	    wm withdraw $topname
	    wm deiconify $topname
	    return
	}

	set top [toplevel $topname]
	wm title $top "Dataset Settings"

	frame $top.f -borderwidth 2 -relief raised
	set t $top.f

	frame $t.f1 
	label $t.f1.title -text "Datasets (checked \[X\] means \"visible\"):"
	set geosetsListbox [makeListBox $t.f1 -width 40 -height 10]
	set bindScript "eval \{DatasetOptions::selectGeoset $id\}"
	bind $geosetsListbox <ButtonRelease-1> $bindScript

	pack $t.f1.title [getListBoxParent $geosetsListbox]\
	    -side top -anchor w -fill y
	
	frame $t.f2

	set visibilityBtn $t.f2.b1
	button $visibilityBtn -text "Visibility" \
	    -command "DatasetOptions::toggleVisibility $id"

	set upBtn $t.f2.b2
	button $upBtn -text "Raise Priority" \
	    -command "DatasetOptions::priority $id up"

	set downBtn $t.f2.b3
	button $downBtn -text "Lower Priority" \
	    -command "DatasetOptions::priority $id down"
	
	set zbHackBtn $t.f2.b4
	checkbutton $t.f2.b4 -text "Z-Buffer Hack" \
	    -command "DatasetOptions::zbHackCB $id"
	set val [$id.tvbase get TV_USE_ZBUFFER_HACK]
	if {$val} { 
	    $zbHackBtn select
	} else {
	    $zbHackBtn deselect
	}

	set popFrontBtn $t.f2.b5
	checkbutton $t.f2.b5 -text "Auto Pop2Front" \
	    -command "DatasetOptions::popFrontCB $id"
	set val [$id.tvbase get TV_AUTO_POP_FRONT]
	if {$val} { 
	    $popFrontBtn select
	} else {
	    $popFrontBtn deselect
	}

	pack $t.f2.b1 -side top -anchor n -pady 15 -expand 1 -fill x
	pack $t.f2.b2 $t.f2.b3 -side top -anchor n -pady 4 -expand 1 -fill x
	pack $t.f2.b4 -side top -anchor n -expand 1
	pack $t.f2.b5 -side top -anchor n -expand 1

	pack $t.f1 $t.f2 -side left

	button $top.dismiss -text "Dismiss" \
	    -command "DatasetOptions::dismissCB $topname $id"

	pack $top.f -side top
	pack $top.dismiss -side top -expand 1 -fill x

	refreshGeoList $id

	# register as an observer of geoset-change events...
	set observerToken [DatasetDialog::registerDatasetObserver \
			       "DatasetOptions::refreshGeoList $id"]
    }

    #
    # m e r g e A n d A n n o t a t e L i s t s 
    #
    # Utility function that takes the list of geosets associated with a
    # tvBase, plus the global list of all geosets opened during the runtime
    # session, and merges them. Those geosets found in both lists are deemed
    # "visible", and annotated accordingly.
    #
    proc mergeAndAnnotateLists {tvList globalList} {
	upvar DatasetOptions::visOffStr visOffStr
	upvar DatasetOptions::visOnStr  visOnStr

	set retList [list]

	foreach i $globalList {
	    # see if this element exists in the local list...
	    if {[lsearch -exact $tvList $i] == -1} {
		lappend retList [format "%s%s" $visOffStr $i]
	    }
	}

	foreach i $tvList {
	    # see if this element exists in the local list...
	    lappend retList [format "%s%s" $visOnStr $i]
	}

	# reverse the list order

	set theList ""
	set i [expr [llength $retList] - 1]
	while {$i >= 0} {
	    lappend theList [lindex $retList $i]
	    incr i -1
	}

	return $theList
    }

    #
    # s e l e c t G e o s e t
    #
    # Utility called whenever an element of the geosetsListbox is selected
    # or otherwise altered.  This proc merely enables/disabled buttons in
    # the dialog to reflect their appropriateness, given the list selection.
    #
    proc selectGeoset {id} {
	upvar DatasetOptions::geosetsListbox($id) geosetsListbox
	upvar DatasetOptions::visibilityBtn($id)  visibilityBtn
	upvar DatasetOptions::upBtn($id)          upBtn
	upvar DatasetOptions::downBtn($id)        downBtn

	set listLen [$geosetsListbox size]
	set selIndex [$geosetsListbox curselection]

	if {[llength $selIndex] == 0} {
	    # no selection -- disable everything...
	    $visibilityBtn configure -state disabled
	    $upBtn configure -state disabled
	    $downBtn configure -state disabled
	    return
	}

	$visibilityBtn configure -state normal

	if {$selIndex == 0} {
	    $upBtn configure -state disabled
	} else {
	    $upBtn configure -state normal
	}

	if {$selIndex == [expr $listLen - 1]} {
	    $downBtn configure -state disabled
	} else {
	    $downBtn configure -state normal
	}
    }

    #
    # t o g g l e V i s i b i l i t y 
    #
    # Callback proc that toggles the "visibility" of the selected geoset.
    #
    proc toggleVisibility {id} {
	# find the selected item in the global list of geosets...
	set geoset [getSelectedGeo $id]

	# see if it's  in the tvBase's list...
	if {[lsearch -exact [$id.tvbase geoset list] $geoset] != -1} {
	    # on both lists... remove it from tvBase's...
	    $id.tvbase geoset remove $geoset
	} else {
	    # add it to the tvbase's list...
	    $id.tvbase geoset add $geoset
	}

	refreshGeoList $id
    }

    #
    # p r i o r i t y 
    #
    # Callback proc that adjusts the "display priority" of the selected
    # geoset.
    #
    proc priority {id dir} {
	set geoName [getSelectedGeo $id]

	if {[lsearch -exact [$id.tvbase geoset list] $geoName] == -1} {
	    tk_messageBox -message "Selected dataset is not visible" \
		-type ok -title "TerraVision Error"
	    return
	}

	$id.tvbase geoset priority $geoName $dir

	refreshGeoList $id
    }

    #
    # getSelectedGeo
    #
    # returns the name of the geoset that is currently selected in
    # the Dataset Settings window
    #
    proc getSelectedGeo {id} {
	upvar DatasetOptions::geosetsListbox($id) geosetsListbox
	upvar DatasetOptions::visOffStr visOffStr
	upvar DatasetOptions::visOnStr visOnStr

	set selIndex [$geosetsListbox curselection]
	set selName  [$geosetsListbox get $selIndex]
	if {[string index $selName 2] == "X"} {
	    set strStart [expr [string length $visOnStr]]
	} else {
	    set strStart [expr [string length $visOffStr]]
	}
	set geoName  [string range $selName $strStart [string length $selName]]

    }

    #
    # r e f r e s h G e o L i s t
    #
    # refreshes the display of available geosets in the
    # Dataset Settings window.
    #
    proc refreshGeoList {id} {
	upvar DatasetOptions::geosetsListbox($id) geosetsListbox

	set geosetsList \
	    [mergeAndAnnotateLists \
		 [$id.tvbase geoset list] [tsGeoPyramid list] ]

	$geosetsListbox delete 0 end
	populateListBox $geosetsListbox $geosetsList
	selectGeoset $id
    }

    #
    # z b H a c k C B
    #
    # Callback for Z-buffer-hack; toggles the setting.
    #
    proc zbHackCB {id} {
	upvar DatasetOptions::zbHackBtn($id) zbHackBtn

    	set val [$id.tvbase get TV_USE_ZBUFFER_HACK]
	set val [expr !$val]
	$id.tvbase set TV_USE_ZBUFFER_HACK $val

	if {$val} { 
	    $zbHackBtn select
	} else {
	    $zbHackBtn deselect
	}
    }

    #
    # p o p 2 F r o n t C B
    #
    # Callback to toggle the Auto Pop to Front setting
    #
    proc popFrontCB {id} {
	upvar DatasetOptions::popFrontBtn($id) popFrontBtn

    	set val [$id.tvbase get TV_AUTO_POP_FRONT]
	set val [expr !$val]
	$id.tvbase set TV_AUTO_POP_FRONT $val

	if {$val} { 
	    $popFrontBtn select
	} else {
	    $popFrontBtn deselect
	}
    }

    
    #
    # d i s m i s s C B 
    #
    # Called to dismiss the "Dataset Settings" dialog, and clean up...
    #
    proc dismissCB {window id} {
	upvar DatasetOptions::geosetsListbox($id) geosetsListbox
	upvar DatasetOptions::visibilityBtn($id)  visibilityBtn
	upvar DatasetOptions::upBtn($id)          upBtn
	upvar DatasetOptions::downBtn($id)        downBtn
	upvar DatasetOptions::zbHackBtn($id)      zbHackBtn
	upvar DatasetOptions::popFrontBtn($id)    popFrontBtn
	upvar DatasetOptions::observerToken($id)  observerToken

	# unset the various instance variables...
	unset geosetsListbox
	unset visibilityBtn
	unset upBtn
	unset downBtn
	unset zbHackBtn
	unset popFrontBtn

	# unpost the window...
	destroy $window

	# unregister as an observer...
	DatasetDialog::unregisterDatasetObserver $observerToken
	unset observerToken
    }

}

    
