#
# annotateoptions.tcl
#

namespace eval AnnotateOptions {

    namespace export postAnnotateOptionsDialog

    #
    # instance vars...
    #
    variable doodleMode
    variable projectMode
    variable timeOut


    #
    # p o s t A n n o t a t e O p t i o n s D i a l o g
    #
    proc postAnnotateOptionsDialog {id} {
	upvar AnnotateOptions::doodleMode($id)  doodleMode
	upvar AnnotateOptions::projectMode($id)  projectMode
	upvar AnnotateOptions::timeOut($id)      timeOut

	# create window name for dialog, test if already exists...
	set topname [format "%s_ann" $id]
	if {[winfo exists $topname] == 1} {
	    wm withdraw $topname     ;# a little dance to force it to top
	    wm deiconify $topname 
	    return
	}

	set top [toplevel $topname]
	wm title $top [format "Doodle: Viewer %d" [instnum $id]]

	# topmost frame...

	frame $top.f -relief raised -borderwidth 2

	checkbutton $top.f.b1 -text "doodle on screen" -padx 30 \
	    -command "AnnotateOptions::doodleModeCB $id" \
	    -variable AnnotateOptions::doodleMode($id) -onvalue 1 -offvalue 0
	set doodleMode 0  ;# preselect this...
	doodleModeCB $id

	checkbutton $top.f.b2 -text "project to terrain" -padx 30 \
	    -command "AnnotateOptions::projectModeCB $id" \
	    -variable AnnotateOptions::projectMode($id) -onvalue 1 -offvalue 0
	set projectMode 0
	projectModeCB $id

	label $top.f.l -text "Timeout (secs.)"
	
	frame $top.f.f
	entry $top.f.f.e -width 10 \
	    -textvariable AnnotateOptions::timeOut($id)
	set timeOut 1.0
	set bindscript "eval \{AnnotateOptions::setTimeOutCB $id\}"
	bind $top.f.f.e <Return> $bindscript
	setTimeOutCB $id

	button $top.f.f.b -text "clear" \
	    -command "AnnotateOptions::clearCB $id"
	
	pack $top.f.f.e $top.f.f.b -side left -expand 1 -fill x

	pack $top.f.b1 $top.f.b2 -side top -expand 1 \
	    -fill both -anchor w
	pack $top.f.l -side top -anchor w
	pack $top.f.f -side top -anchor w -fill x

	button $top.dismiss -text "Dismiss" \
	    -command "AnnotateOptions::dismissCB $id"

	pack $top.f -side top -expand 1 -fill both
	pack $top.dismiss -side top -expand 1 -fill x
    }

    #
    # d o o d l e M o d e C B
    #
    proc doodleModeCB {id} {
	upvar AnnotateOptions::doodleMode($id)   doodleMode

	if {$doodleMode} {
	    # set a flag in the global space for the button callbacks,
	    # which indicates we are in "doodle mode"...
	    global gDoodleMode
	    set gDoodleMode($id) 1
	    $id.tvbase view stopMovement
	    statusBarSetViewName $id " * * * doodle mode * * * "
	} else {
	    global gDoodleMode
	    if {[array names gDoodleMode $id] != ""} {
		unset gDoodleMode($id)
	    }
	    statusBarSetViewName $id ""
	}
    }

    #
    # p r o j e c t M o d e C B 
    #
    proc projectModeCB {id} {
	upvar AnnotateOptions::projectMode($id)  projectMode

	$id.tvbase set TV_DOODLE_ON_TERRAIN $projectMode
    }


    #
    # c l e a r C B
    #
    proc clearCB {id} {
	$id.tvbase stroke clear
    }

    #
    # s e t T i m e O u t C B 
    #
    proc setTimeOutCB {id} {
	upvar AnnotateOptions::timeOut($id)  timeOut
	$id.tvbase stroke timeOut $timeOut
    }

    #
    # d i s m i s s C B 
    #
    proc dismissCB {id} {
	upvar AnnotateOptions::doodleMode($id) doodleMode
	
	# do the user a favor (?) and deselect doodle-mode on exit...
	set doodleMode 0
	doodleModeCB $id

	# nuke the instance variables...
	unset AnnotateOptions::doodleMode($id)
	unset AnnotateOptions::projectMode($id)
	unset AnnotateOptions::timeOut($id)

	# finally, the dialog itself...
	destroy [format "%s_ann" $id]
    }

}    ;# AnnotateOptions namespace