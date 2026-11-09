#
# renderoptions.tcl
#

namespace eval RenderOptions {
    namespace export postRenderOptionsDialog

    #
    # module globals...
    # These are all associative arrays holding the state of the rendering 
    # options for 2D/3D view instances, and indexed by the instanceID
    #
    variable terrLight
    variable textured
    variable scaleIPD 
    variable hud 
    variable mipmap 
    variable backcull 
    variable ground 
    variable wireframe 
    variable datasets 
    variable horizoncull 
    variable stereo 
    variable popping1
    variable popping2
    variable popping3

    #
    # m a k e W i n d o w 
    #
    proc makeWindow {id} {
	upvar RenderOptions::terrLight($id)   terrLight
	upvar RenderOptions::textured($id)    textured
	upvar RenderOptions::scaleIPD($id)    scaleIPD 
	upvar RenderOptions::hud($id)         hud 
	upvar RenderOptions::mipmap($id)      mipmap
	upvar RenderOptions::backcull($id)    backcull
	upvar RenderOptions::ground($id)      ground
	upvar RenderOptions::wireframe($id)   wireframe
	upvar RenderOptions::datasets($id)    datasets
	upvar RenderOptions::horizoncull($id) horizoncull  
	upvar RenderOptions::stereo($id)      stereo
	upvar RenderOptions::popping1($id)    popping1
	upvar RenderOptions::popping2($id)    popping2
	upvar RenderOptions::popping3($id)    popping3
	
	# make the toplevel window and set various window-options...
	set topname [format "%s_rod" $id ]

	# don't want more than one of these...
	if {[winfo exists $topname] == 1} {
	    wm withdraw $topname
	    wm deiconify $topname
	    return
	}
	set top [toplevel $topname]
	wm title $top [format "Rendering Options: Viewer %d" [instnum $id]]
	    
	# topmost frame of checkboxes/radiobutttons...
	frame $top.f1 -borderwidth 2 -relief raised

	# need internal frames to organize the buttons into 3 columns...
	# 1st column...
	set f $top.f1.f1
	frame $f

	checkbutton $f.01 \
	    -text "Terrain Lighting" \
	    -command "RenderOptions::checkboxCB $id $f.01 \
                      TV_SHOW_LIGHTING RenderOptions::terrLight($id)" \
	    -variable RenderOptions::terrLight($id)

	checkbutton $f.02 \
	    -text "HUD" \
	    -command "RenderOptions::checkboxCB $id $f.02 \
                      TV_SHOW_HEADS_UP RenderOptions::hud($id)" \
	    -variable RenderOptions::hud($id)

#	checkbutton $f.03 \
	    -text "MipMap" \
	    -command "RenderOptions::checkboxCB $id $f.03 \
                      TV_MIPMAP_TEXTURES RenderOptions::mipmap($id)" \
	    -variable RenderOptions::mipmap($id)

	checkbutton $f.04 \
	    -text "Backface Culling" \
	    -command "RenderOptions::checkboxCB $id $f.04 \
                      TV_BACKFACE_CULL RenderOptions::backcull($id)" \
	    -variable RenderOptions::backcull($id)

	checkbutton $f.05 \
	    -text "Horizon Culling" \
	    -command "RenderOptions::checkboxCB $id $f.05 \
                      TV_HORIZON_CULL RenderOptions::horizoncull($id)" \
	    -variable RenderOptions::horizoncull($id)

	checkbutton $f.06 \
	    -text "Wireframe" \
	    -command "RenderOptions::checkboxCB $id $f.06 \
                      TV_SHOW_WIREFRAME RenderOptions::wireframe($id)" \
	    -variable RenderOptions::wireframe($id)

	pack $f.01 $f.02 $f.04 $f.05 $f.06 -side top -anchor w

	
	# middle column...
	set f $top.f1.f2
	frame $f

	checkbutton $f.01 \
	    -text "Textured" \
	    -command "RenderOptions::checkboxCB $id $f.01 \
                      TV_SHOW_TEXTURES RenderOptions::textured($id)" \
	    -variable RenderOptions::textured($id)

	checkbutton $f.02 \
	    -text "Show Ground" \
	    -command "RenderOptions::checkboxCB $id $f.02 \
                      TV_SHOW_GROUND RenderOptions::ground($id)" \
	    -variable RenderOptions::ground($id)

	checkbutton $f.03 \
	    -text "Outline Datasets" \
	    -command "RenderOptions::checkboxCB $id $f.03 \
                      TV_SHOW_DATASETS RenderOptions::datasets($id)" \
	    -variable RenderOptions::datasets($id)

	checkbutton $f.04 \
	    -text "Stereo Mode" \
	    -command "RenderOptions::unImplemented1CB STEREO"

	checkbutton $f.05 \
	    -text "Scale IPD w/Dist" \
	    -command "RenderOptions::checkboxCB $id $f.05 \
                      TV_SCALE_IPD RenderOptions::scaleIPD($id)" \
	    -variable RenderOptions::scaleIPD($id)

	pack $f.01 $f.02 $f.03 $f.04 $f.05 -side top -anchor w

	# righmost column...
	set f $top.f1.f3
	frame $f

#	label $f.02 -text " "   ;# used to make a blank row...

	label $f.03 -text "Popping:"

	set popping1 $f.04
	radiobutton $popping1 -text "None" -padx 15 -value 0 \
	    -command "RenderOptions::poppingCB $id $f.04"

	set popping2 $f.05
	radiobutton $popping2 -text "Low Res" -padx 15 -value 1 \
	    -command "RenderOptions::poppingCB $id $f.05"

	set popping3 $f.06
	radiobutton $popping3 -text "Low Frag" -padx 15 -value 2 \
	    -command "RenderOptions::poppingCB $id $f.06"

	# set the default state for the radio button

	set popping [$id.tvbase get TV_POPPING_STRATEGY]

	if { $popping == 1 } { 
	    $popping2 select
	} elseif { $popping == 2 } {
	    $popping3 select
	} else {
	    $popping1 select
	}
        
	pack $f.03 $f.04 $f.05 $f.06 -side top -anchor w

	# pack the three columns...
	pack $top.f1.f1 $top.f1.f2 $top.f1.f3 -side left -padx 5 -pady 5 \
	    -expand 1 -fill both
	
	#
	# next, fill out middle frame of sliders...
	#

	frame $top.f2 -relief raised -borderwidth 2

	# as with the buttons above, we organize things here into two columns
	# of sliders.
	# first column...
	set f $top.f2.f1
	frame $f

	scale $f.s1 \
	    -label "Inter-Pupillary Distance (cm)" -orient horiz \
	    -resolution 0.1 -from 0 -to 20.0
	$f.s1 set [$id.tvbase get TV_ABSOLUTE_IPD]
	setScaleBindings $f.s1 \
	    "RenderOptions::scaleCB $id $f.s1 TV_ABSOLUTE_IPD"

	scale $f.s2 \
	    -label "Resolution Adjustment" -orient horiz \
	    -resolution 0.1 -from "-0.5" -to 2.0 
	$f.s2 set [$id.tvbase get TV_RESOLUTION_ADJUST]
	setScaleBindings $f.s2 \
	    "RenderOptions::scaleCB $id $f.s2 TV_RESOLUTION_ADJUST"

	scale $f.s3 \
	    -label "Fog Density" -orient horiz \
	    -resolution 0.01 -from 0. -to 1.
	$f.s3 set [$id.tvbase get TV_FOG_DENSITY]
	setScaleBindings $f.s3 \
	    "RenderOptions::scaleCB $id $f.s3 TV_FOG_DENSITY"

	scale $f.s4 \
	    -label "Drop to Wireframe Threshold (Hz)" -orient horiz \
	    -resolution .1 -from 0 -to 10
	$f.s4 set [$id.tvbase get TV_MOTION_THRESHOLD]
	setScaleBindings $f.s4 \
	    "RenderOptions::scaleCB $id $f.s4 TV_MOTION_THRESHOLD"

	pack $f.s1 $f.s2 $f.s3 $f.s4 -side top -anchor w -expand 0 -fill x

	# second column...
	set f $top.f2.f2
	frame $f

	scale $f.s1 \
	    -label "Field of View (deg)" -orient horiz \
	    -from 2 -to 90.0  \
	    -command "RenderOptions::fovCB $id"
	$f.s1 set [getViewFOV [$id.tvbase view current]]
	
	# NOTE: we disable this until implemented in libtv2
	scale $f.s2 \
	    -label "Elevation Scale" -orient horiz \
	    -resolution 0.1 -from 0.0 -to 20.0 \
	    -state disabled  -background grey50 ;#<<------
	$f.s2 set [$id.tvbase get TV_ELEVATION_SCALE]
	### TEMPORARILY DISABLED:   setScaleBindings $f.s2 \
	    "RenderOptions::scaleCB $id $f.s2 TV_ELEVATION_SCALE"

	# NOTE: we disable this until implemented in libtv2
	scale $f.s3 \
	    -label "Time of Day (hour)" -orient horiz \
	    -resolution 0.1 -from 5.0 -to 19.0 \
	    -state disabled -background grey50 ;#<<------
	$f.s3 set [$id.tvbase get TV_TIME_OF_DAY]
	### TEMPORARILY DISABLED:  setScaleBindings $f.s3 \
	    "RenderOptions::scaleCB $id $f.s3 TV_TIME_OF_DAY"

	pack $f.s1 $f.s2 $f.s3 -side top -anchor w -expand 0 -fill x

	pack $top.f2.f1 $top.f2.f2 -side left -padx 5 -pady 5 \
	    -expand 1 -fill both

	# dismiss button...
	button $top.dismiss -text "Dismiss" -relief raised -borderwidth 2 \
            -command "RenderOptions::dismissCB $id"
	
	pack $top.f1 -side top -expand 1 -fill both -ipadx 5 -ipady 5
	pack $top.f2 -side top -expand 1 -fill both -ipadx 5 -ipady 5 
	pack $top.dismiss -side top -expand 1 -fill x

	set tk_strictMotif 1
    }

    #
    # p o s t R e n d e r O p t i o n s D i a l o g 
    #
    proc postRenderOptionsDialog {inst} {
	upvar RenderOptions::terrLight($inst)   terrLight
	upvar RenderOptions::textured($inst)    textured
	upvar RenderOptions::scaleIPD($inst)    scaleIPD
	upvar RenderOptions::hud($inst)         hud
	upvar RenderOptions::mipmap($inst)      mipmap
	upvar RenderOptions::backcull($inst)    backcull
	upvar RenderOptions::ground($inst)      ground
	upvar RenderOptions::wireframe($inst)   wireframe
	upvar RenderOptions::datasets($inst)    datasets
	upvar RenderOptions::horizoncull($inst) horizoncull
	upvar RenderOptions::stereo($inst)      stereo

	set terrLight    [$inst.tvbase get TV_SHOW_LIGHTING]
	set textured     [$inst.tvbase get TV_SHOW_TEXTURES]
	set scaleIPD     [$inst.tvbase get TV_SCALE_IPD]
	set hud          [$inst.tvbase get TV_SHOW_HEADS_UP]
	set mipmap       [$inst.tvbase get TV_MIPMAP_TEXTURES]
	set backcull     [$inst.tvbase get TV_BACKFACE_CULL]
	set ground       [$inst.tvbase get TV_SHOW_GROUND]
	set wireframe    [$inst.tvbase get TV_SHOW_WIREFRAME]
	set datasets     [$inst.tvbase get TV_SHOW_DATASETS]
	set horizoncull  [$inst.tvbase get TV_HORIZON_CULL]
	set stereo       [$inst.tvbase get TV_STEREO]
	
	makeWindow $inst
    }

    ########################
    ##   Callbacks        ##
    ########################

    #
    # c h e c k b o x C B
    #
    proc checkboxCB {id widget tvVariable tvValue} {
	upvar $tvValue val
	$id.tvbase set $tvVariable $val
    }

    #
    # p o p p i n g C B
    #
    proc poppingCB {id widget} {
	upvar RenderOptions::popping1($id) popping1
	upvar RenderOptions::popping2($id) popping2
	upvar RenderOptions::popping3($id) popping3

	set value 0
	if {$widget == $popping1} { 
	    set value 0 
	} elseif {$widget == $popping2} {
	    set value 1 
	} elseif {$widget == $popping3} {
	    set value 2 
	}

	$id.tvbase set TV_POPPING_STRATEGY $value
    }

    #
    # s c a l e C B
    #
    proc scaleCB {id widget tvVariable} {
	set value [$widget get]

	if {$tvVariable == "TV_ABSOLUTE_IPD"} {
	    # displayed in units of cm, set in units of meters...
	    set value [expr $value / 10.]
	}

	$id.tvbase set $tvVariable $value
    }

    #
    # f o v C B
    #
    proc fovCB {id val} {
        set view [$id.tvbase view current]
        setViewFOV $view $val
        $id.tvbase view current $view
    }
	
    proc unImplemented1CB {feature} {
	tk_messageBox -type ok -title "TerraVision Error" -icon error \
	 -message [format "**UNIMPLEMENTED FEATURE: %s" $feature]
    }


    #
    # d i s m i s s C B 
    #
    proc dismissCB {id} {
        # ideally, we'd like to merely unmanage the window, but not sure
        # how to do this...
        destroy [format "%s_rod" $id]
    }


    #
    #   All this nonsense below arises because the standard behavior of Tk's 
    # scales has some really undesirable properties; i.e., when one drags the
    # slider, it invokes the callback continuously. From the callback side,
    # its impossible to tell the event that caused the invocation. So we 
    # rewrite things here to issue a callback only after the slider has been
    # released at the end of a drag. 
    #   Its further been observed that *sometimes* the sliders issue callbacks 
    # continuously when active; i.e., when there's motion in their focus.
    # The rewrite here takes care of that problem as well.
    #   These procs were heavily borrowed from the standard Tk library.
    #

    
    # 
    # s e t S c a l e B i n d i n g s
    #
    proc setScaleBindings {scale command} {
	bind $scale <1> {
	    RenderOptions::scaleButtonDown %W %x %y
            break
	}
	
	bind $scale <B1-Motion> {
	    RenderOptions::scaleDrag %W %x %y
            break
	}

        set script \
	    "eval \{RenderOptions::scaleButtonRelease %W %x %y \"$command\"\}"
	bind $scale <ButtonRelease-1> $script
    }

    variable dragStatus
    variable dragOffsetX 
    variable dragOffsetY

    #
    # s c a l e B u t t o n D o w n 
    #
    proc scaleButtonDown {widget x y} {
	upvar RenderOptions::dragStatus dragStatus
	upvar RenderOptions::dragOffsetX dragOffsetX
	upvar RenderOptions::dragOffsetY dragOffsetY

	set dragStatus 0
	set where [$widget identify $x $y]
	if {![string compare $where "trough1"]} {
	    $widget set [expr [$widget get] - [$widget cget -resolution]]

	} elseif {![string compare $where "trough2"]} {
	    $widget set [expr [$widget get] + [$widget cget -resolution]]

	} elseif {![string compare $where "slider"]} {
	    set dragStatus 1
	    set coords [$widget coords]
	    set dragOffsetX [expr $x - [lindex $coords 0]]
	    set dragOffsetY [expr $y - [lindex $coords 1]]
	    $widget configure -sliderrelief sunken
	}
    }

    #
    # s c a l e D r a g 
    #
    proc scaleDrag {widget x y} {
	upvar RenderOptions::dragStatus dragStatus
	upvar RenderOptions::dragOffsetX dragOffsetX
	upvar RenderOptions::dragOffsetY dragOffsetY

	if {!$dragStatus} { return }
	
	set x [expr $x - $dragOffsetX]
	set y [expr $y - $dragOffsetY]
	$widget set [$widget get $x $y]
    }

    #
    # s c a l e B u t t o n R e l e a s e 
    #
    proc scaleButtonRelease {widget x y command} {
	upvar RenderOptions::dragStatus dragStatus

	if {!$dragStatus} {
	    $widget configure -sliderrelief raised
	    set dragStatus 0
	}

	eval $command
    }


}  ;# end of namespace
