#
# viewController.tcl
#

namespace eval ViewController {

    namespace export makeXYZController
    namespace export makeHPRController
    namespace export makeFBLRUDController 

    variable pi 3.14159265359
    variable pi2 [expr $pi / 2.]

    variable thickness 28
    variable radius 28
    variable gapAngle [expr 7 * $pi / 180.]
    variable stepIncrements 25

    variable callbacks
    variable normalBackground grey75
    variable brightHighlight  grey95
    variable softHighlight    grey90
    variable darkShadow       grey50
    variable softShadow       grey60
    variable invertColor      black
    variable textColor        white

    proc angle {x1 y1 x2 y2} {
	set y1 [expr -$y1]
	set y2 [expr -$y2]
	
	# expression for the vector between the 2 points...
	set Vi [expr $x2 - $x1]
	set Vj [expr ($y2 - $y1)]  ; #negate because inverted Y-axis
	
	# find its normal...
	set Ni $Vj
	set Nj [expr 0. - $Vi]
	
	# find the magnitude of the normal...
	set mag [expr sqrt($Ni*$Ni + $Nj*$Nj)]
	
	# find the angle between normal and our "light vector: <1,-1>"
	set angle [expr acos( ($Ni - $Nj) / ($mag*sqrt(2)) )]
	return $angle
    }
	

    proc makeXYButtons {parent xCen yCen angle procCB tag label} {
	variable radius 
	variable gapAngle 
	variable thickness 
	variable pi2 
	variable pi
	variable callbacks
	variable normalBackground
	variable brightHighlight 
	variable softHighlight
	variable darkShadow
	variable softShadow
	variable invertColor
	variable textColor
	variable stepIncrements

	# begin building up a "create polygon" command;  we calculate the
	# coordinates, appending them to a string which will ultimately be
	# "eval"d to instantiate the polygon in the canvas...

	lappend polyCmd $parent create polygon
	
	set step [expr $pi2 / $stepIncrements]
	set start [expr $angle + $gapAngle]
	set stop [expr $angle + $pi2-$gapAngle]
	
	# inner arc...
	for {set alpha $start} {$alpha < $stop} \
	    {set alpha [expr $alpha + $step]} {
		set x [expr $xCen + $radius*cos($alpha)]
		set y [expr $yCen + $radius*sin($alpha)]
		lappend polyList $x $y
	    }
	set alpha $stop
	set x [expr $xCen + $radius*cos($alpha)]
	set y [expr $yCen + $radius*sin($alpha)]
	lappend polyList $x $y
	
	# outer arc...
	set r [expr $radius + $thickness]
	for {set alpha $stop} {$alpha > $start} \
	    {set alpha [expr $alpha - $step]} {
		set x [expr $xCen + $r*cos($alpha)]
		set y [expr $yCen + $r*sin($alpha)]
		lappend polyList $x $y
	    }
	set x [expr $xCen + $r*cos($start)]
	set y [expr $yCen + $r*sin($start)]
	lappend polyList $x $y
	
	set polyCmd [concat $polyCmd $polyList]
	lappend polyCmd -outline $normalBackground \
	    -fill $normalBackground -width 1 -tags "$tag $tag.POLY"
	
	set name [eval $polyCmd]
	$parent bind $tag <Button-1> {
	    set tag [lindex [%W itemcget current -tags] 0]
	    %W itemconfigure $tag.POLY -fill $ViewController::invertColor
	}

	set callbacks($parent.$tag) "$procCB $tag"
	$parent bind $tag <ButtonRelease-1> {
	    set tag [lindex [%W itemcget current -tags] 0]
	    %W itemconfigure $tag.POLY -fill $ViewController::normalBackground
	    eval "$ViewController::callbacks(%W.$tag)"
	}
	
	#
	# now calculate and draw "shadows"...
	#
	set threshAngle [expr 25.*$pi/180.]
	set len [llength $polyList]
	for {set i 0} {$i < $len} {set i [expr $i+2]} {
	    if {$i == 0} {
		set x1 [lindex $polyList [expr $len-2]]
		set y1 [lindex $polyList [expr $len-1]]
	    } else {
		set x1 [lindex $polyList [expr $i-2]]
		set y1 [lindex $polyList [expr $i-1]]
	    }
	    set x2 [lindex $polyList $i]
	    set y2 [lindex $polyList [expr $i+1]]
	    
	    set a [angle $x1 $y1 $x2 $y2]
	    if {[expr abs($a - $pi2)] < $threshAngle} {
		if {$a < $pi2} {
		    set color $softShadow
		} else {
		    set color $softHighlight
		}
	    } elseif {$a < $pi2} {
		set color $darkShadow
	    } else {
		set color $brightHighlight
	    }
	    $parent create line $x1 $y1 $x2 $y2 -fill $color -width 2 \
		-smooth 1 -joinstyle bevel -splinesteps 2 -tag $tag
	    
	}

	#
	# finally, place the text label...
	#
	set a [expr $angle + $pi2/2.]
	set r [expr $radius + $thickness/2.]
	set x [expr $xCen + $r*cos($a)]
	set y [expr $yCen + $r*sin($a)]
	$parent create text $x $y -text $label -fill $textColor -anchor c \
	    -tag $tag -justify center
	
    }

    proc makeInnerButtons {parent xCen yCen angle procCB tag label} {
	variable gapAngle
	variable radius
	variable pi
	variable pi2
	variable callbacks
	variable normalBackground
	variable brightHighlight 
	variable softHighlight
	variable darkShadow
	variable softShadow
	variable invertColor
	variable textColor
	variable stepIncrements

	lappend polyCmd $parent create polygon
	
	set r [expr $radius - 5]
	set step [expr -($pi / $stepIncrements)]
	set start [expr $angle + $pi - $gapAngle]
	set stop [expr $start - $pi + 2*$gapAngle]
	
	# overall shape...
	for {set alpha $start} {$alpha > $stop} \
	    {set alpha [expr $alpha + $step]} {
		set x [expr $xCen + $r*cos($alpha)]
		set y [expr $yCen + $r*sin($alpha)]
		lappend polyList $x $y
	    }
	set alpha $stop
	set x [expr $xCen + $r*cos($alpha)]
	set y [expr $yCen + $r*sin($alpha)]
	lappend polyList $x $y
	
	set polyCmd [concat $polyCmd $polyList]
	lappend polyCmd -outline $normalBackground \
	    -fill $normalBackground -width 1 -tags "$tag $tag.POLY"
	
	set name [eval $polyCmd]
	$parent bind $tag <Button-1> {
	    set tag [lindex [%W itemcget current -tags] 0]
	    %W itemconfigure $tag.POLY -fill $ViewController::invertColor
	}

	set callbacks($parent.$tag) "$procCB $tag"
	$parent bind $tag <ButtonRelease-1> {
	    set tag [lindex [%W itemcget current -tags] 0]
	    %W itemconfigure $tag.POLY -fill $ViewController::normalBackground
	    eval "$ViewController::callbacks(%W.$tag)"
	}
	
	#
	# now calculate and draw "shadows"...
	#
	set threshAngle [expr 25.*$pi/180.]
	set len [llength $polyList]
	for {set i 0} {$i < $len} {set i [expr $i+2]} {
	    if {$i == 0} {
		set x1 [lindex $polyList [expr $len-2]]
		set y1 [lindex $polyList [expr $len-1]]
	    } else {
		set x1 [lindex $polyList [expr $i-2]]
		set y1 [lindex $polyList [expr $i-1]]
	    }
	    set x2 [lindex $polyList $i]
	    set y2 [lindex $polyList [expr $i+1]]
	    
	    set a [angle $x1 $y1 $x2 $y2]
	    if {[expr abs($a - $pi2)] < $threshAngle} {
		if {$a < $pi2} {
		    set color $softShadow
		} else {
		    set color $softHighlight
		}
	    } elseif {$a < $pi2} {
		set color $darkShadow
	    } else {
		set color $brightHighlight
	    }
	    $parent create line $x1 $y1 $x2 $y2 -fill $color -width 2 \
		-smooth 1 -joinstyle bevel -splinesteps 2 -tag $tag
	    
	}

	#
	# finally, place the text label...
	#
	set a [expr $angle + $pi2]
	set x [expr $xCen + $r/2*cos($a)]
	set y [expr $yCen + $r/2*sin($a)]
	$parent create text $x $y -text $label -fill $textColor -anchor c \
	    -tag $tag -justify center

    }	
    
    proc makeXYZController {parent xCen yCen procCB} {
	variable pi
	makeXYButtons $parent $xCen $yCen [expr (0+45) * $pi / 180.] \
	    $procCB S "-Y"
	makeXYButtons $parent $xCen $yCen [expr (90+45) * $pi / 180.]  \
	    $procCB W "-X"
	makeXYButtons $parent $xCen $yCen [expr (180+45) * $pi / 180.] \
	    $procCB N "+Y"
	makeXYButtons $parent $xCen $yCen [expr (270+45) * $pi / 180.] \
	    $procCB E "+X"

	makeInnerButtons $parent $xCen $yCen 0. $procCB D "-z"
	makeInnerButtons $parent $xCen $yCen $pi $procCB U "+z"
    }

    proc makeFBLRUDController {parent xCen yCen procCB} {
	variable pi
	makeXYButtons $parent $xCen $yCen [expr (0+45) * $pi / 180.] \
	    $procCB S "B"
	makeXYButtons $parent $xCen $yCen [expr (90+45) * $pi / 180.]  \
	    $procCB W "L"
	makeXYButtons $parent $xCen $yCen [expr (180+45) * $pi / 180.] \
	    $procCB N "F"
	makeXYButtons $parent $xCen $yCen [expr (270+45) * $pi / 180.] \
	    $procCB E "R"

	makeInnerButtons $parent $xCen $yCen 0. $procCB D "dn"
	makeInnerButtons $parent $xCen $yCen $pi $procCB U "up"
    }


    proc makeHPRController {parent xCen yCen procCB} {
	variable pi
	variable radius

	makeXYButtons $parent $xCen $yCen [expr (0+45) * $pi / 180.] \
	    $procCB S "-P"
	makeXYButtons $parent $xCen $yCen [expr (90+45) * $pi / 180.]  \
	    $procCB W "-R"
	makeXYButtons $parent $xCen $yCen [expr (180+45) * $pi / 180.] \
	    $procCB N "+P"
	makeXYButtons $parent $xCen $yCen [expr (270+45) * $pi / 180.] \
	    $procCB E "+R"

	makeInnerButtons $parent $xCen $yCen [expr 90.*$pi/180.] $procCB D "-h"
	makeInnerButtons $parent $xCen $yCen [expr 270.*$pi/180.] $procCB U "+h"
	

    }


}       ; # namespace ViewController

