#
# paths.tcl
#

namespace eval Paths {

    namespace export makePathsMenu

    #
    # Implementation notes:
    #
    # The "edit-paths" window is always named $id_editpath,  and there's 
    # only one instance of it *per viewier" at any one time.
    #
    
    variable saveMenuIndex         ;# indices of menu items, used to control
    variable editMenuIndex         ;# disabled/enabled state as appropriate.
    variable playMenuIndex
    variable stopMenuIndex
    variable contMenuIndex
    variable gotoStartMenuIndex
    variable gotoNextMenuIndex
    variable gotoPrevMenuIndex
    variable gotoEndMenuIndex
    variable save320MenuIndex
    variable save480MenuIndex

    variable mname                 ;# handle to the menu widget
    

    #
    # m a k e P a t h s M e n u 
    #
    proc makePathsMenu {id parent} {
	upvar Paths::saveMenuIndex($id) saveMenuIndex
	upvar Paths::editMenuIndex($id) editMenuIndex
	upvar Paths::playMenuIndex($id) playMenuIndex
	upvar Paths::stopMenuIndex($id) stopMenuIndex
	upvar Paths::contMenuIndex($id) contMenuIndex
	upvar Paths::gotoStartMenuIndex($id) gotoStartMenuIndex
	upvar Paths::gotoNextMenuIndex($id) gotoNextMenuIndex
	upvar Paths::gotoPrevMenuIndex($id) gotoPrevMenuIndex
	upvar Paths::gotoEndMenuIndex($id) gotoEndMenuIndex
	upvar Paths::save320MenuIndex($id) save320MenuIndex
	upvar Paths::save480MenuIndex($id) save480MenuIndex
	upvar Paths::mname($id) mname

	set mname $parent.paths
	menu $mname -type tearoff
	$mname add command -label "New Path" -underline 0 \
	    -command "Paths::newPathCmd $id"
	$mname add command -label "Load Path..." -underline 0 \
	    -command "Paths::loadPathCmd $id"
	$mname add command -label "Save Path As..." -underline 10 \
	    -state disabled \
	    -command "Paths::savePathCmd $id"
	set saveMenuIndex [$mname index last]

	$mname add separator
	$mname add command -label "Edit Path..." -underline 0 \
	    -state disabled \
	    -command "Paths::editPathCmd $id"
	set editMenuIndex [$mname index last]
	
	$mname add separator
	$mname add command -label Play  -underline 0 \
	    -accelerator "Ctrl+P" -state disabled \
	    -command "Paths::playStopContPauseCmd $id play"
	set playMenuIndex [$mname index last]
	$mname add command -label Stop  -underline 0 \
	    -accelerator "Ctrl+S" -state disabled \
	    -command "Paths::playStopContPauseCmd $id stop"
	set stopMenuIndex [$mname index last]
	$mname add command -label Continue  -underline 6 \
	    -accelerator "Ctrl+U" -state disabled \
	    -command "Paths::playStopContPauseCmd $id continue"
	set contMenuIndex [$mname index last]

	$mname add separator
	$mname add command -label "Goto Start" -underline 2 \
	    -accelerator "Ctrl+T" -state disabled \
	    -command "Paths::gotoCmd $id start"
	set gotoStartMenuIndex [$mname index last]
	$mname add command -label "Goto Next" -underline 7 \
	    -accelerator "Ctrl+N" -state disabled \
	    -command "Paths::gotoCmd $id next"
	set gotoNextMenuIndex [$mname index last]
	$mname add command -label "Goto Prev" -underline 8 \
	    -accelerator "Ctrl+V" -state disabled \
	    -command "Paths::gotoCmd $id prev"
	set gotoPrevMenuIndex [$mname index last]
	$mname add command -label "Goto End" -underline 7 \
	    -accelerator "Ctrl+E" -state disabled \
	    -command "Paths::gotoCmd $id end"
	set gotoEndMenuIndex [$mname index last]

	$mname add separator
	$mname add command -label "Save to Images (320x240)" -state disabled \
	    -command "Paths::pathsRecordCmd $id 320 240"
	set save320MenuIndex [$mname index last]
	$mname add command -label "Save to Images (480x360)" -state disabled \
	    -command "Paths::pathsRecordCmd $id 480 360"
	set save480MenuIndex [$mname index last]
	
	return $mname
    }
    
    
    #
    # n e w P a t h C m d 
    #
    proc newPathCmd {id} {
	
	# test for any previously opened path...
	if {![Paths::verifyClose $id]} { return }

	# make a new path...
	$id.tvbase path new
	
	# enable valid options on menu...
	configureMenuItems $id

	# post edit dialog...
	editPathCmd $id
    }
    
    #
    # l o a d P a t h C m d 
    #
    proc loadPathCmd {id} {
	global gPathsDirectory

	# verify is previously opened path exists...
	if {![verifyClose $id]} { return }
	
	# NOTE: we need to set the status bar to reflect the new path, but
	# several details to be worked out first!
	#	global $id.status.bookmark
	
	# get the filename from the user...
	set file [tk_getOpenFile -initialdir $gPathsDirectory \
		      -defaultextension .path -title "Load a \'Path\' file"]
	
	# non-empty string indicates "something" was returned; try to open it..
	if {[string length $file]} {
	    if { ![$id.tvbase path load $file] } {
		tk_messageBox -icon -error -title "Load Path Error" \
		    -type ok \
		    -message "Unable to load that Path file\nPlease make sure a valid file was given"
	    } else {
		configureMenuItems $id
	    }
	}
    }
    
    # 
    # s a v e P a t h C m d
    #
    # Returns 0 if user "cancels out"; returns 1 otherwise.
    #
    proc savePathCmd {id} {
	global gPathsDirectory
	
	# get save filename...
	set file [tk_getSaveFile -initialdir $gPathsDirectory \
		      -defaultextension .path -title "Load a \'Path\' file"]
	
	if {[string length $file] == 0} {
	    # canceled out...
	    return 0
	}

	# non-empty string indicates "something" was returned; be cautious 
	# about overwriting exist files...
	
	if { ![$id.tvbase path save $file] } {
	    tk_messageBox -icon -error -title "Save Path Error" \
		-type ok \
		-message "Unable to save that Path file\nPlease make sure a valid file was given"
	} 
	
	return 1
    }

    #
    # p l a y S t o p C o n t P a u s e C m d 
    #
    # Callback for the Play/Stop commands.
    #
    proc playStopContPauseCmd {id action} {
	if {![pathExists $id]} { return }	
	$id.tvbase path $action
	configureMenuItems $id
	if {$action == "stop"} {
	    statusBarSetViewName $id ""
	} else {
	    statusBarSetViewName $id [$id.tvbase path name]
	}
    }

    #
    # g o t o C m d 
    #
    # Callback for the "goto" path commands.
    #
    proc gotoCmd {id where} {
	if {![pathExists $id]} { return }
	$id.tvbase path goto $where
    }

    #
    # p a t h s R e c o r d C m d 
    #
    proc pathsRecordCmd {id xres yres} {
	if {![pathExists $id]} { return }
	$id.tvbase path record $xres $yres
    }

    #
    # c o n f i g u r e M e n u I t e m s
    #
    # Utility proc to enable/disable items in the "Paths" menu to appropriately
    # reflect the program state.
    #
    proc configureMenuItems {id} {
	upvar Paths::saveMenuIndex($id) saveMenuIndex
	upvar Paths::editMenuIndex($id) editMenuIndex
	upvar Paths::playMenuIndex($id) playMenuIndex
	upvar Paths::stopMenuIndex($id) stopMenuIndex
	upvar Paths::contMenuIndex($id) contMenuIndex
	upvar Paths::gotoStartMenuIndex($id) gotoStartMenuIndex
	upvar Paths::gotoNextMenuIndex($id) gotoNextMenuIndex
	upvar Paths::gotoPrevMenuIndex($id) gotoPrevMenuIndex
	upvar Paths::gotoEndMenuIndex($id) gotoEndMenuIndex
	upvar Paths::save320MenuIndex($id) save320MenuIndex
	upvar Paths::save480MenuIndex($id) save480MenuIndex
	upvar Paths::mname($id) mname

	# enable those commands that are now valid...
	set state disabled
	if {[$id.tvbase path isActive]} {
	    set state normal
	}
	$mname entryconfigure $saveMenuIndex -state $state
	$mname entryconfigure $editMenuIndex -state $state
	$mname entryconfigure $gotoStartMenuIndex -state $state
	$mname entryconfigure $gotoNextMenuIndex -state $state
	$mname entryconfigure $gotoPrevMenuIndex -state $state
	$mname entryconfigure $gotoEndMenuIndex -state $state
	$mname entryconfigure $save320MenuIndex -state $state
	$mname entryconfigure $save480MenuIndex -state $state
	$mname entryconfigure $contMenuIndex -state $state

	set startedState disabled
	set stoppedState normal
	if {[$id.tvbase path isPlaying]} {
	    set startedState normal
	    set stoppedState disabled
	    $mname entryconfigure $contMenuIndex -label "Pause"
	    $mname entryconfigure $contMenuIndex -underline 3 \
		-command "Paths::playStopContPauseCmd $id pause"
	} else {
	    $mname entryconfigure $contMenuIndex -label "Continue"
	    $mname entryconfigure $contMenuIndex -underline 6 \
		-command "Paths::playStopContPauseCmd $id continue"
	}
	$mname entryconfigure $playMenuIndex -state $stoppedState
 	$mname entryconfigure $stopMenuIndex -state $startedState
    }

    #
    # Additional "static vars" concerned with path editing...
    #
    variable pathTitleField     ;# entry widget holding the title 
    variable waypointList       ;# list of waypoints
    variable addBtn
    variable deleteBtn
    variable upBtn
    variable downBtn
    variable updateBtn
    variable duplicateBtn
    variable hscaleBtn
    variable velocityField
    variable loopStatus 
    variable wrapStatus
    variable hscaleStatus
    variable interpStatus
    

    #
    # e d i t P a t h C m d 
    #
    # Posts the "Edit Path" dialog box.
    #
    proc editPathCmd {id} {
	upvar Paths::pathTitleField($id) pathTitleField
	upvar Paths::waypointList($id)   waypointList 
	upvar Paths::addBtn($id)         addBtn
	upvar Paths::deleteBtn($id)      deleteBtn
	upvar Paths::upBtn($id)          upBtn
	upvar Paths::downBtn($id)        downBtn
	upvar Paths::updateBtn($id)      updateBtn 
	upvar Paths::duplicateBtn($id)   duplicateBtn
	upvar Paths::velocityField($id)  velocityField
	upvar Paths::loopStatus($id)     loopStatus
	upvar Paths::wrapStatus($id)     wrapStatus
	upvar Paths::hscaleStatus($id)   hscaleStatus
	upvar Paths::interpStatus($id)   interpStatus
    
	# make new toplevel window...
	set topname [editWindowName $id]

	# don't want more than one...
	if {[winfo exists $topname] == 1} {
	    wm withdraw $topname
	    wm deiconify $topname
	    return
	}

	set top [toplevel $topname -class Dialog] 
	wm title $top [format "Edit Path: Viewer %d" [instnum $id]]

	# upper frame for the label and entry to set the path's title...
	frame $top.f1 -borderwidth 2 -relief raised
	label $top.f1.l -text "Path title:" -justify left
	entry $top.f1.e
	set pathTitleField $top.f1.e
	pack $top.f1.l -side left
	pack $top.f1.e -side left -expand 1 -fill x 

	set bindscript "eval \{$id.tvbase path name \[$pathTitleField get\]\}"
	bind $pathTitleField <Return> $bindscript

	# middle area consists of two, side-by-side panels of controls...
	frame $top.f2

	# left side holds the "waypoints" that make up the path...
	frame $top.f2.left -borderwidth 2 -relief raised
	label $top.f2.left.l -text "Waypoints"
	set waypointList [makeListBox $top.f2.left -width 25]
	
	# kinda painful...have to generate a binding script with all the 
	# substitutions pre-evaluated....
	set bindscript "eval \{ $id.tvbase path goto \[$waypointList curselection\] ; \
            Paths::checkSelectionStatus $id \}"
	bind $waypointList <ButtonRelease-1> $bindscript

	frame $top.f2.left.f
	button $top.f2.left.f.b1 -text "Add" \
	    -command "Paths::addWaypoint $id"
	set addBtn $top.f2.left.f.b1

	button $top.f2.left.f.b2 -text "Delete" \
	    -command "Paths::deleteWaypoint $id"
	set deleteBtn $top.f2.left.f.b2

	button $top.f2.left.f.b3 -text "Move up" \
	    -command "Paths::moveWaypoint $id up"
	set upBtn $top.f2.left.f.b3

	button $top.f2.left.f.b4 -text "Move down" \
	    -command "Paths::moveWaypoint $id down"
	set downBtn $top.f2.left.f.b4

	button $top.f2.left.f.b5 -text "Update" \
	    -command "Paths::updateWaypoint $id"
	set updateBtn $top.f2.left.f.b5

	button $top.f2.left.f.b6 -text "Duplicate" \
	    -command "Paths::duplicateWaypoint $id"
	set duplicateBtn $top.f2.left.f.b6

	pack $top.f2.left.f.b1 \
	     $top.f2.left.f.b2 \
	     $top.f2.left.f.b3 \
	     $top.f2.left.f.b4 \
	     $top.f2.left.f.b5 \
	     $top.f2.left.f.b6 -side top -expand 1 -fill x 

	pack $top.f2.left.l -side top -anchor c
	pack [getListBoxParent $waypointList] $top.f2.left.f -side left \
	    -expand 1 -fill both -padx 2
	
	#
	# right side has various controls concerned with velocity, looping, etc
	#
	frame $top.f2.right -borderwidth 2 -relief raised

	frame $top.f2.right.f0
	label $top.f2.right.f0.l -text "Velocity (m/s):"
	entry $top.f2.right.f0.e -width 10
	set velocityField $top.f2.right.f0.e
	pack $top.f2.right.f0.l $top.f2.right.f0.e  \
	    -side left -fill x -expand 1

	set bindscript "eval \{ $id.tvbase path velocity \[$velocityField get\]\}"
	bind $velocityField <Return> $bindscript
	
	frame $top.f2.right.f1 
	label $top.f2.right.f1.l -text "Interpolation:"
	set Paths::interpStatus($id) none
	radiobutton $top.f2.right.f1.b1 -text "none"  -value none \
	    -variable "Paths::interpStatus($id)" \
	    -command "Paths::setInterp $id"
	radiobutton $top.f2.right.f1.b2 -text "linear" -value linear \
	    -variable "Paths::interpStatus($id)" \
	    -command "Paths::setInterp $id"
	radiobutton $top.f2.right.f1.b3 -text "spline" -value spline \
	    -variable "Paths::interpStatus($id)" \
	    -command "Paths::setInterp $id"
	pack $top.f2.right.f1.l -side top -anchor w
	pack $top.f2.right.f1.b1 \
	     $top.f2.right.f1.b2 \
	     $top.f2.right.f1.b3 -side top -ipadx 10 -anchor w
	     
	frame $top.f2.right.f2 
	set Paths::loopStatus($id) off
	set loopBtn $top.f2.right.f2.c1
	checkbutton $top.f2.right.f2.c1 -text "continual loop" \
	    -variable "Paths::loopStatus($id)" -onvalue on -offvalue off \
	    -command "Paths::setLoop $id"

	set Paths::wrapStatus($id) off
	set closeBtn $top.f2.right.f2.c2
	checkbutton $top.f2.right.f2.c2 -text "close the loop" \
	    -variable "Paths::wrapStatus($id)" -onvalue on -offvalue off \
	    -command "Paths::setWrap $id"

	set Paths::hscaleStatus($id) off
	set hscaleBtn $top.f2.right.f2.c3
	checkbutton $top.f2.right.f2.c3 -text "high-speed scaling" \
	    -variable "Paths::hscaleStatus($id)" -onvalue on -offvalue off \
	    -command "Paths::setHscale $id"

	pack $top.f2.right.f2.c1 \
	     $top.f2.right.f2.c2 \
	     $top.f2.right.f2.c3 -side top -anchor w
	
	# pack highlevel components of the "right" panel...
	pack $top.f2.right.f0 -side top -fill x -padx 4
	pack $top.f2.right.f2 -side top -expand 1 -fill x \
	    -anchor n -pady 10 -padx 4
	pack $top.f2.right.f1 -side top -expand 1 -fill x \
	    -anchor n -padx 4

	# pack the two panels of the entire middle section...
	pack $top.f2.left $top.f2.right -side left \
	    -expand 1 -fill y -ipadx 2 -ipady 2

	# lastly, we'll need a dismiss button...
	button $top.dismiss -text "Dismiss" -command "Paths::dismissCB $id"

	# smooch 'em all together...
	pack $top.f1 -side top -fill x -ipady 4 -ipady 4
	pack $top.f2 -side top -fill both -expand 1
	pack $top.dismiss -side top -fill x -expand 1


	# populate some of the widgets with info we already know...
	$pathTitleField insert 0 [$id.tvbase path name]
	$velocityField insert 0 [$id.tvbase path velocity]
	set loopStatus [$id.tvbase path loop]
	set wrapStatus [$id.tvbase path loop close]
	set hscaleStatus [$id.tvbase path velocity scale]
	set interpStatus [$id.tvbase path interpolate]
	populateListBox $waypointList [prefixNumbers [$id.tvbase path list]]
	checkSelectionStatus $id
    }

    #
    # c h e c k S e l e c t i o n S t a t u s
    #
    # Utility to enable/disable buttons that apply in the presense/absense 
    # of a selection in the waypoints list.
    #
    proc checkSelectionStatus {id} {
	upvar Paths::waypointList($id) waypointList
	upvar Paths::deleteBtn($id)    deleteBtn
	upvar Paths::upBtn($id)        upBtn
	upvar Paths::downBtn($id)      downBtn
	upvar Paths::updateBtn($id)    updateBtn
	upvar Paths::duplicateBtn($id) duplicateBtn

	# is there a current selection?
	set currSelIndex [$waypointList curselection]
	if {$currSelIndex == ""} {
	    # nothing selected...
	    $deleteBtn configure -state disabled
	    $upBtn configure -state disabled
	    $downBtn configure -state disabled
	    $updateBtn configure -state disabled
	    $duplicateBtn configure -state disabled
	    return
	}

	$deleteBtn configure -state normal
	$updateBtn configure -state normal
	$duplicateBtn configure -state normal

	if {$currSelIndex == 0} {
	    $upBtn configure -state disabled
	} else {
	    $upBtn configure -state normal
	}

	if {$currSelIndex == [expr [$waypointList size] - 1]} {
	    $downBtn configure -state disabled
	} else {
	    $downBtn configure -state normal
	}
    }

    #
    # a d d W a y p o i n t 
    #
    proc addWaypoint {id} {
	upvar Paths::waypointList($id) waypointList

	$id.tvbase path add

	$waypointList delete 0 end
	populateListBox $waypointList [prefixNumbers [$id.tvbase path list]]
	checkSelectionStatus $id
    }	

    #
    # d e l e t e W a y p o i n t 
    #
    proc deleteWaypoint {id} {
	upvar Paths::waypointList($id) waypointList

	set index [$waypointList curselection]
	if { [catch {$id.tvbase path delete $index} msg] } {
	    tk_messageBox -message $msg -icon error -type ok
	    return
	}

	$waypointList delete 0 end
	populateListBox $waypointList [prefixNumbers [$id.tvbase path list]]
	checkSelectionStatus $id
    }	

    #
    # m o v e W a y p o i n t 
    #
    proc moveWaypoint {id direction} {
	upvar Paths::waypointList($id) waypointList

	set index [$waypointList curselection]
	if { [catch {$id.tvbase path $direction $index} msg] } {
	    tk_messageBox -message $msg -icon error -type ok
	    return
	}

	$waypointList delete 0 end
	populateListBox $waypointList [prefixNumbers [$id.tvbase path list]]
	
	# reselect the previously selected element...
	if {$direction == "up"} {
	    set index [expr $index - 1]
	} else {
	    set index [expr $index + 1]
	}
	$waypointList selection set $index
	checkSelectionStatus $id
    }	
    
    #
    # d u p l i c a t e W a y p o i n t 
    #
    proc duplicateWaypoint {id} {
	upvar Paths::waypointList($id) waypointList

	set index [$waypointList curselection]
	if { [catch {$id.tvbase path duplicate $index} msg] } {
	    tk_messageBox -message $msg -icon error -type ok
	    return
	}

	$waypointList delete 0 end
	populateListBox $waypointList [prefixNumbers [$id.tvbase path list]]

	$waypointList selection set $index
	checkSelectionStatus $id
    }	
    
    #
    # u p d a t e W a y p o i n t
    #
    proc updateWaypoint {id} {
	upvar Paths::waypointList($id) waypointList

	set index [$waypointList curselection]
	if { [catch {$id.tvbase path update $index} msg] } {
	    tk_messageBox -message $msg -icon error -type ok
	    return
	}

	$waypointList delete 0 end
	populateListBox $waypointList [prefixNumbers [$id.tvbase path list]]

	$waypointList selection set $index
	checkSelectionStatus $id
    }	

    #
    # s e t I n t e r p 
    #
    proc setInterp {id} {
	upvar Paths::interpStatus($id) interpStatus

	$id.tvbase path interpolate $interpStatus
    }

    #
    # s e t L o o p
    #
    proc setLoop {id} {
	upvar Paths::loopStatus($id) loopStatus

	$id.tvbase path loop $loopStatus
    }
    
    #
    # s e t W r a p 
    #
    proc setWrap {id} {
	upvar Paths::wrapStatus($id) wrapStatus

	$id.tvbase path loop close $wrapStatus
    }

    #
    # s e t H s c a l e
    #
    proc setHscale {id} {
	upvar Paths::hscaleStatus($id) hscaleStatus

	$id.tvbase path velocity scale $hscaleStatus
    }

    #
    # d i s m i s s C B 
    #
    proc dismissCB {id} {
	upvar Paths::pathTitleField($id) pathTitleField
	upvar Paths::velocityField($id) velocityField

	# apply any values entered into the text fields...
	$id.tvbase path name [$pathTitleField get]
	$id.tvbase path velocity [$velocityField get]

	destroy [editWindowName $id]
    }

    #
    # p r e f i x N u m b e r s 
    #
    # Utility proc to prefix the elements of a list with a "line number"
    # string.
    #
    proc prefixNumbers {theList} {
	set returnList [list]
	set counter 0
	foreach i $theList {
	    lappend returnList [format "%d: %s" [incr counter] $i]
	}
	
	return $returnList
    }

    #
    # e d i t W i n d o w N a m e
    #
    # Utility proc to construct the name of the "edit window" from an 
    # an instance ID.
    #
    proc editWindowName {id} {
	return [format "%s_editpath" $id]
    }

    #
    # p a t h E x i s t s 
    #
    # Utility proc to test if a path has been established.  Simply tests for
    # a non-null name.
    #
    proc pathExists {id} {
	if {[$id.tvbase path name] == ""} {
	    return 0
	} else {
	    return 1
	}
    }

    #
    # v e r i f y C l o s e
    #
    # Utility proc called whenever a new Path is to be created in the
    # rendering instance.  If there is an existing path, the user is
    # warned that it will be replaced, and is presented the chance to save 
    # it first.  NOTE: this is not exact, in the sense that we do not
    # detect whether the "existing" path was modified -- we just blindly
    # present the save option.
    #
    # Returns 0 if user "cancels out"; 1 otherwise.
    #
    proc verifyClose {id} {
	
	# does a path currently exist?
	if {![$id.tvbase path isActive]} { return 1 }

	set response [tk_messageBox -type yesnocancel -icon warning \
			  -title "Confirm Existing Path Replacement" \
			  -message "The current path will be replaced.\nDo you wish to save it first?"]

	if {$response == "yes"} {
	    if {![Paths::savePathCmd $id]} {return 0}
	} elseif {$response == "cancel"} {
	    return 0
	}

	#
	# If there's an existing "edit path" window displayed for the
	# old path, remove it...

	set win [editWindowName $id]
	if {[winfo exists $win]} {
	    destroy $win
	}
	
	return 1
    }


}

