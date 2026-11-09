#
# links.tcl
#

namespace eval Links {

    namespace export makeLinksMenu


    #
    # m a k e L i n k s M e n u 
    #
    proc makeLinksMenu {parent} {
	global gInstID
	set mname $parent.links
	menu $mname -type tearoff
	$mname add cascade -label "Control View of..." -underline 0 \
	    -menu $mname.view
	$mname add cascade -label "Show View Frustum of..." -underline 10 \
	    -menu $mname.frustum
	$mname add cascade -label "Show Tiles of..." -underline 5 \
	    -menu $mname.tiles
	$mname add cascade -label "Show Paths of..." -underline 5 \
	    -menu $mname.paths
	
	# figure out our instance ID, then get our window's name...
	set myBaseID \
	    [string range $parent 0 [expr [string first . $parent 1] - 1]]
	set myTitle [wm title $myBaseID]

	menu $mname.view \
	    -postcommand "Links::makeSubMenus $mname.view $myBaseID \
                          \"$myTitle\"  view false"
	menu $mname.frustum \
	    -postcommand "Links::makeSubMenus $mname.frustum $myBaseID \
                          \"$myTitle\"  frustum true"
	menu $mname.tiles \
	    -postcommand "Links::makeSubMenus $mname.tiles $myBaseID \
                          \"$myTitle\"  tiles true"
	menu $mname.paths \
	    -postcommand "Links::makeSubMenus $mname.paths $myBaseID \
                          \"$myTitle\"  paths true"
	return $mname
    }


    #
    # m a k e S u b M e n u s
    #
    # Called when the "Link" menu is invoked off the main window; this proc 
    # dynamically builds the associated submenus.
    #
    proc makeSubMenus {menu myID myTitle linkType includeSelf} {
	global gWinList

	# gather up current list of windows...
	set winList [list]
	foreach i $gWinList {
	    lappend winList [wm title $i]
	}

	$menu delete 0 end

	#
	# standard content...
	#
##	$menu add command -label On -command "put HUH?"
##	$menu add separator 

	#
	# The "Select Window" option was used in the X/Motif version
	# but isn't critical as we list all windows on this display
	# for selection later in the menu.
	#
	#$menu add command -label "Select Window..." -underline 0 \
	    -command "Links::linkCB $myID select $linkType $includeSelf"

	if {$includeSelf} {
	    $menu add command -label "Self" -underline 3 \
		-command "Links::linkCB $myID self $linkType $includeSelf"
	}

	$menu add command -label "Select All" -underline 7 \
	    -command "Links::linkCB $myID all $linkType $includeSelf"
	$menu add command -label "Deselect All" -underline 0 \
	    -command "Links::linkCB $myID none $linkType $includeSelf"
	$menu add separator

	#
	# dynamic content;  Note that we *know* the two arrays gWinList and
	# winList below are parallel, by construction.
	#
	foreach id $gWinList title $winList {
	    if {[string equal $title $myTitle]} {
		if {$includeSelf} {
		    $menu add command -label $title \
			-command "Links::linkCB $myID $id $linkType $includeSelf"
		}
		continue
	    }

	    $menu add command -label $title \
		-command "Links::linkCB $myID $id $linkType $includeSelf"
	}
    }

    #
    # l i n k C B 
    #
    proc linkCB {fromID target linkType includeSelf} {
	global gWinList

	if {[string equal $target "all"] || [string equal $target "none"]} {
	    set winList $gWinList
	    if {[string equal $includeSelf "false"]} {
		set index [lindex gWinList $fromID]
		set winList [lreplace gWinList index index]
	    }
	} elseif {[string equal $target select]} {
	    puts "NEED TO IMPLEMENT SELECT"
	    set winList [list]
	    
	} elseif {[string equal $target self]} {
	    set winList $fromID

	} else {
	    set winList $target
	}

	if {[string equal $target "none"]} {
	    set action delete
	} else {
	    set action add
	}

	foreach i $winList {
	    $fromID.tvbase link $linkType $action $i.tvbase
	} 
    }

}
