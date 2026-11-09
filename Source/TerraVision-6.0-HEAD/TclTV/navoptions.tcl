#
# navoptions.tcl
#

namespace eval NavOptions {
    namespace export postNavOptionsDialog

    #
    # module globals...
    #

    #
    # p o s t N a v O p t i o n s D i a l o g
    #
    proc postNavOptionsDialog {inst} {

	makeWindow $inst

    }

    #
    # m a k e W i n d o w 
    #
    proc makeWindow {id} {
	global gFlightMode gFlightControlMode gViewMode

	# make the toplevel window and set various window-options...
	set topname [format "%s_nav" $id ]

	# don't want more than one of these...
	if {[winfo exists $topname] == 1} {
	    wm withdraw $topname
	    wm deiconify $topname
	    return
	}
	set top [toplevel $topname]
	wm title $top [format "Navigation Options: Viewer %d" [instnum $id]]
	    
	# topmost frame of checkboxes...
	frame $top.f1 -borderwidth 2 -relief raised

	checkbutton $top.f1.01 \
	    -text "Airplane Controls" \
	    -variable gFlightControlMode($id) \
	    -onvalue "airplane" -offvalue "none"

	checkbutton $top.f1.02 \
	    -text "Height Scaled Navigation" \
	    -command "NavOptions::checkboxCB $id $top.f1.02 \
                TV_HEIGHT_BASED_NAVIGATION"
	setButton $top.f1.02 [$id.tvbase get TV_HEIGHT_BASED_NAVIGATION]

	checkbutton $top.f1.03 \
	    -text "Fly To Mode" \
	    -variable gFlightMode($id) \
	    -onvalue "FlyTo" -offvalue "Fly" \
	    -command "statusBarSetNavMode $id \$gFlightMode($id)"

	checkbutton $top.f1.04 \
	    -text "2D Navigation" \
	    -variable gViewMode($id) -onvalue 1 -offvalue 0 \
	    -command "NavOptions::checkboxCB $id $top.f1.04 \
                TV_TWOD_CONSTRAIN ; 
                statusBarSetViewMode $id \[$id.tvbase get TV_TWOD_CONSTRAIN\]"
	setButton $top.f1.04 [$id.tvbase get TV_TWOD_CONSTRAIN]

	pack $top.f1.01 $top.f1.02 $top.f1.03 $top.f1.04 \
	    -side top -anchor w

	button $top.dismiss -text "Dismiss" -relief raised \
            -command "NavOptions::dismissCB $id"
	

	pack $top.f1 -side top -expand 1 -fill both
	pack $top.dismiss -side bottom -expand 1 -fill x
    }

    #
    # c h e c k b o x C B 
    #
    proc checkboxCB {id widget tvVariable} {
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

    #
    # d i s m i s s C B 
    #
    proc dismissCB {id} {
	destroy [format "%s_nav" $id]
    }

} ; # end of namespace
