#
# debugoptions.tcl
#

namespace eval DebugOptions {

    namespace export makeDebugMenu postDebugOptionsDialog

    #
    # Global windows:
    #   .debugMask       # dialog for the "debug mask" settings.
    #

    #
    # Module variables.  Note that these are global settings, and thus are
    # not arrays indexed by the instance-ID, as is done in other modules.
    #
    variable debugLevel
    variable logFileTarget    ;# either "stderr" or "file"
    variable logFilenameField ;# entry field widget 
    variable logFilename  ""  ;# the most recent filename for logging output
    variable debugMasks       ;# array holding the various debug-mask settings

    # module instance variables...
    variable freezeTiles
    variable debugHUD
    variable debugTiles
    variable returnBtn

    #
    # m a k e G l o b a l s M e n u 
    #
    proc makeGlobalsMenu {id parent} {
	global gInstID
	upvar DebugOptions::debugLevel debugLevel

	set mname $parent.globals
	menu $mname -type tearoff

	$mname add cascade -label "Debug Level" -underline 0 \
	    -menu $mname.level
	$mname add command -label "Debug Masks..." -underline 6 \
	    -command "DebugOptions::postDebugMaskDialog"
	$mname add separator
	$mname add command -label "Debug Settings" -underline 0 \
	    -command "DebugOptions::postDebugOptionsDialog $id"

	set DebugOptions::debugLevel [pthr trace getlevel]
	menu $mname.level
	$mname.level add radiobutton -label none -underline 0 -value 0 \
	    -variable DebugOptions::debugLevel \
	    -command DebugOptions::debugLevelCB
	$mname.level add radiobutton -label 1 -underline 0 -value 1 \
	    -variable DebugOptions::debugLevel \
	    -command DebugOptions::debugLevelCB
	$mname.level add radiobutton -label 2 -underline 0 -value 2 \
	    -variable DebugOptions::debugLevel \
	    -command DebugOptions::debugLevelCB
	$mname.level add radiobutton -label 3 -underline 0 -value 3 \
	    -variable DebugOptions::debugLevel \
	    -command DebugOptions::debugLevelCB
	$mname.level add radiobutton -label 4 -underline 0 -value 4 \
	    -variable DebugOptions::debugLevel \
	    -command DebugOptions::debugLevelCB

	menu $mname.logging
	$mname.logging add radiobutton -label on -value on -underline 0   \
	    -command "dummyCB $gInstID"
	$mname.logging add radiobutton -label off -value off -underline 1 \
	    -command "dummyCB $gInstID"

	return $mname
    }


    #
    # p o s t D e b u g O p t i o n s D i a l o g
    #
    proc postDebugOptionsDialog {id} {
	upvar DebugOptions::freezeTiles($id)  freezeTiles
	upvar DebugOptions::debugHUD($id)     debugHUD
	upvar DebugOptions::debugTiles($id)   debugTiles
	upvar DebugOptions::returnBtn($id)    returnBtn
	
	# make toplevel window...
	set topname [format "%s_dbod" $id]
	
	# if it already exists, try to force it to top...
	if {[winfo exists $topname] == 1} {
	    wm withdraw $topname     ;# a little dance to force it to top
	    wm deiconify $topname 
	    return
	}

	set top [toplevel $topname]
	wm title $top [format "Debug Options: Viewer %d" [instnum $id]]

	# topmost frame...
	set f $top.f1
	frame $f -borderwidth 2 -relief raised

	checkbutton $f.01 \
	    -text "freeze tiles" \
	    -command "$id.tvbase set TV_FREEZE_TILES \
                      \$DebugOptions::freezeTiles($id); \
                      DebugOptions::updateGUIstate $id" \
	    -variable DebugOptions::freezeTiles($id)
	set freezeTiles [$id.tvbase get TV_FREEZE_TILES]

	set returnBtn $f.02
	button $returnBtn -text "return to frozen view" \
	    -command "$id.tvbase view setFrozen"

	pack $f.01 $returnBtn -side top -expand 1 -pady 2

	# lower frame...
	set f $top.f2 
	frame $f -borderwidth 2 -relief raised

	checkbutton $f.01 \
	    -text "Debug HUD" \
	    -command "$id.tvbase set TV_SHOW_HEADS_UP_DEBUG \
                      \$DebugOptions::debugHUD($id)" \
	    -variable DebugOptions::debugHUD($id)
	set debugHUD [$id.tvbase get TV_SHOW_HEADS_UP_DEBUG]

	checkbutton $f.02 \
	    -text "Debug Tiles" \
	    -command "$id.tvbase set TV_SHOW_TILE_DEBUG \
                      \$DebugOptions::debugTiles($id)" \
	    -variable DebugOptions::debugTiles($id)
	set debugTiles [$id.tvbase get TV_SHOW_TILE_DEBUG]

	pack $f.01 $f.02 -side top -expand 1 -fill x

	# dismiss button...
	button $top.dismiss -text "dismiss" \
	    -command "DebugOptions::dismissCB $top $id"

	pack $top.f1 $top.f2 $top.dismiss -side top -expand 1 -fill both

	updateGUIstate $id
    }


    #
    # u p d a t e G U I s t a t e
    #
    # Local proc to merely enable/disable the "return to frozen view" button,
    # as appropriate.
    #
    proc updateGUIstate {id} {
	upvar DebugOptions::freezeTiles($id)   freezeTiles
	upvar DebugOptions::returnBtn($id)     returnBtn

	if {$freezeTiles} {
	    $returnBtn configure -state normal
	} else {
	    $returnBtn configure -state disabled
	}
    }

    #
    # d i s m i s s C B 
    #
    proc dismissCB {window id} {
	upvar DebugOptions::freezeTiles($id)  freezeTiles
	upvar DebugOptions::debugHUD($id)     debugHUD
	upvar DebugOptions::debugTiles($id)   debugTiles
	upvar DebugOptions::returnBtn($id)    returnBtn

	# clean up the instance variables...
	unset freezeTiles
	unset debugHUD
	unset debugTiles
	unset returnBtn

	destroy $window
    }

    #
    # d e b u g L e v e l C B 
    #
    # Called in response to changes in debug-level via menu command.
    #
    proc debugLevelCB {} {
	upvar DebugOptions::debugLevel debugLevel
	pthr trace level $debugLevel
    }

    #
    # p o s t D e b u g M a s k D i a l o g 
    #
    proc postDebugMaskDialog {} {
	upvar DebugOptions::logFileTarget     logFileTarget
	upvar DebugOptions::logFilenameField  logFilenameField
	upvar DebugOptions::debugMasks        debugMasks

	set top .debugMask

	if {[winfo exists $top]} {
	    # try to raise it to the top...
	    wm iconify $top
	    wm deiconify $top
	    return
	}

	toplevel $top
	wm iconify $top
	wm title $top "Debugging Options"

	frame $top.f1 -borderwidth 2 -relief raised

	set logFileTarget stderr
	radiobutton $top.f1.b1 -text "write to stderr" -value stderr \
	    -variable DebugOptions::logFileTarget \
	    -command "DebugOptions::debugMaskTargetCB stderr"
	frame $top.f1.f
	radiobutton $top.f1.f.b2 -text "write to file: " -value file \
	    -variable DebugOptions::logFileTarget \
	    -command "DebugOptions::debugMaskTargetCB file"
	entry $top.f1.f.e -width 20 
	set logFilenameField $top.f1.f.e

	set tvu_trace_file [tvutils get TVU_TRACE_FILE]
	if { $tvu_trace_file == "stderr" } {
	    $logFilenameField insert 0 "tvdebug.log"
	    $logFilenameField configure -state disabled
	} else {
	    $logFilenameField insert 0 $tvu_trace_file
	    $top.f1.f.b2 select
	}


	pack $top.f1.f.b2 -side left
	pack $top.f1.f.e  -side left -fill x
	pack $top.f1.b1 $top.f1.f -side top -anchor w 

	frame $top.f2 -borderwidth 2 -relief raised
	label $top.f2.00 -text "Program aspects to trace:"
	grid $top.f2.00

	checkbutton $top.f2.01 -text "tiles" \
	    -offvalue 0 -onvalue TILE_MASK \
	    -variable DebugOptions::debugMasks(1)
	checkbutton $top.f2.02 -text "texture" \
	    -offvalue 0 -onvalue TEXTURE_MASK \
	    -variable DebugOptions::debugMasks(2)
	grid $top.f2.01 $top.f2.02 -sticky w

	checkbutton $top.f2.03 -text "initialization" \
	    -offvalue 0 -onvalue INIT_MASK \
	    -variable DebugOptions::debugMasks(3)
	checkbutton $top.f2.04 -text "visibility" \
	    -offvalue 0 -onvalue VIS_MASK \
	    -variable DebugOptions::debugMasks(4)
	grid $top.f2.03 $top.f2.04 -sticky w

	checkbutton $top.f2.05 -text "2D viewer" \
	    -offvalue 0 -onvalue 2D_MASK \
	    -variable DebugOptions::debugMasks(5)
	checkbutton $top.f2.06 -text "3D viewer" \
	    -offvalue 0 -onvalue 3D_MASK \
	    -variable DebugOptions::debugMasks(6)
	grid $top.f2.05 $top.f2.06 -sticky w

	checkbutton $top.f2.07 -text "map viewer" \
	    -offvalue 0 -onvalue MAP_MASK \
	    -variable DebugOptions::debugMasks(7)
	checkbutton $top.f2.08 -text "OpenGL" \
	    -offvalue 0 -onvalue OPENGL_MASK \
	    -variable DebugOptions::debugMasks(8)
	grid $top.f2.07 $top.f2.08 -sticky w

	checkbutton $top.f2.09 -text "OAA" \
	    -offvalue 0 -onvalue OAA_MASK \
	    -variable DebugOptions::debugMasks(9)
	checkbutton $top.f2.10 -text "logging" \
	    -offvalue 0 -onvalue LOGGING_MASK \
	    -variable DebugOptions::debugMasks(10)
	grid $top.f2.09 $top.f2.10 -sticky w

	checkbutton $top.f2.11 -text "cache" \
	    -offvalue 0 -onvalue CACHE_MASK \
	    -variable DebugOptions::debugMasks(11)
	checkbutton $top.f2.12 -text ".geo" \
	    -offvalue 0 -onvalue GEO_MASK \
	    -variable DebugOptions::debugMasks(12)
	grid $top.f2.11 $top.f2.12 -sticky w

	set tvu_trace_mask [tvutils get TVU_TRACE_MASK]
	if { [expr $tvu_trace_mask & 8] != 0 } { $top.f2.01 select }
	if { [expr $tvu_trace_mask & 128] != 0 } { $top.f2.02 select }
	if { [expr $tvu_trace_mask & 512] != 0 } { $top.f2.03 select }
	if { [expr $tvu_trace_mask & 16] != 0 } { $top.f2.04 select }
	if { [expr $tvu_trace_mask & 32] != 0 } { $top.f2.05 select }
	if { [expr $tvu_trace_mask & 64] != 0 } { $top.f2.06 select }
	if { [expr $tvu_trace_mask & 256] != 0 } { $top.f2.07 select }
	if { [expr $tvu_trace_mask & 1024] != 0 } { $top.f2.08 select }
	if { [expr $tvu_trace_mask & 2048] != 0 } { $top.f2.09 select }
	if { [expr $tvu_trace_mask & 4096] != 0 } { $top.f2.10 select }
	if { [expr $tvu_trace_mask & 8192] != 0 } { $top.f2.11 select }
	if { [expr $tvu_trace_mask & 16384] != 0 } { $top.f2.12 select }

	frame $top.f3 -borderwidth 2 -relief raised
	button $top.f3.ok -text "OK" \
	    -command "DebugOptions::debugMaskApplyCB ; \
                      DebugOptions::debugMaskCancelCB"
	button $top.f3.apply -text "Apply" \
	    -command "DebugOptions::debugMaskApplyCB"
	button $top.f3.cancel -text "Cancel" \
	    -command "DebugOptions::debugMaskCancelCB"
	pack $top.f3.ok $top.f3.apply $top.f3.cancel -side left -expand 1 
	
	pack $top.f1 $top.f2 -side top -expand 1 -fill both
	pack $top.f3 -side top -expand 1 -fill x

	wm deiconify $top
    }

    #
    # d e b u g M a s k T a r g e t C B 
    #
    # Called when the radiobuttons denoting the logging output target 
    # (either stderr or to a file) change.  This merely enables/disables
    # the entry field as appropriate.
    #
    proc debugMaskTargetCB {target} {
	upvar DebugOptions::logFilenameField logFilenameField

	if {$target == "file"} {
	    $logFilenameField configure -state normal
	} else {
	    $logFilenameField configure -state disabled
	}
    }

    #
    # d e b u g M a s k A p p l y C B
    #
    proc debugMaskApplyCB {} {
	upvar DebugOptions::debugMasks       debugMasks
	upvar DebugOptions::logFileTarget    logFileTarget
	upvar DebugOptions::logFilenameField logFilenameField
	upvar DebugOptions::logFilename      logFilename
	
	set pthrCommand "pthr trace mask "
	foreach i [array names debugMasks] {
	    if {$debugMasks($i) != 0} {
		append pthrCommand " " $debugMasks($i)
	    }
	}

	eval $pthrCommand

	# set the target for the logging output...	
	if {$logFileTarget == "file"} {
	    set filename [$logFilenameField get]
	    tvutils set TVU_TRACE_FILE $filename
	    set logFilename $filename
	} else {
	    tvutils set TVU_TRACE_FILE stderr
	    set logFilename stderr
	}

    }

    #
    # d e b u g M a s k C a n c e l C B 
    #
    proc debugMaskCancelCB {} {
	destroy .debugMask
    }

}
