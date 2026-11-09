#
# util.tcl
# 
# Various general-purpose utilities.
#

# constants and procs for deg<-->radian conversions...
set PI 3.14159265359
set PI2 [expr $PI/2.0]
set RAD_DEG_FACTOR [expr $PI / 180.]

#
# r a d 2 d e g
#
proc rad2deg {rad} { 
    global RAD_DEG_FACTOR
    return [expr $rad / $RAD_DEG_FACTOR]
}

#
# d e g 2 r a d 
#
proc deg2rad {deg} { 
    global RAD_DEG_FACTOR
    return [expr $deg * $RAD_DEG_FACTOR]
} 

#
# i n s t n u m 
#
# Returns the numerical instance encoded into an "instanceID".  That is, 
# instanceIDs are of the form .tvinstN, where N is an integer;  we return N.
#
proc instnum {id} {
    global gInstBase

    if {[regsub $gInstBase $id "" N] == 0 || ![string is integer $N]} {
	return "**unknown**"
    } else {
	return $N
    }
}


#
# Utility procs for creating ListBoxes with scrollbars that are managed *only*
# if needed.
#
# Listboxes are created by "makeListBox", which generates a frame to encompass
# the listbox and its horizontal/vertical scrollbars.  The scrollbars are
# managed/unmanaged as appropriate by the utility "populateListBox".  Access to
# the frame is obtained via "getListBoxParent", for the purposes of packing the
# whole conglomerate into a larger context.
#

set listBoxInst  0    ;# for generating unique frame names

#
# m a k e L i s t B o x
#
# Takes a parent widget, and a list of arguments appropriate to Tk's "listbox"
# command.  Returns the name of the interior "listbox" widget.
#
proc makeListBox {parent args} {
    global listBoxInst

    # need a containing frame...
    set boxid [format "lbox_%d" [incr listBoxInst]]
    set theFrame $parent.$boxid
    frame $theFrame

    # make the listbox...
    eval "listbox $theFrame.list $args"

    # make the scrollbars, and wire them up...
    scrollbar $theFrame.vscroll -orient vertical \
	-command "$theFrame.list yview"
    $theFrame.list configure -yscrollcommand "$theFrame.vscroll set"

    scrollbar $theFrame.hscroll -orient horizontal \
	-command "$theFrame.list xview"
    $theFrame.list configure -xscrollcommand "$theFrame.hscroll set"

    # preliminary packing...
    pack $theFrame.list -fill both -expand 1 

    return $theFrame.list
}

#
# g e t L i s t B o x P a r e n t
#
# Returns the encompassing frame from the listbox widget created via 
# "makeListBox".
#
proc getListBoxParent {listBox} {
    return [winfo parent $listBox]
}

#
# p o p u l a t e L i s t B o x
#
# Populates a listbox, as created by "makeListBox", with a list of items.
# Horizontal/vertical scrollbars are enabled as necessary.
#
proc populateListBox {listbox itemsList} {
    # get the names of the scrollbars...
    set parent [getListBoxParent $listbox]
    set vscroll $parent.vscroll
    set hscroll $parent.hscroll

    # clear the listbox...
    $listbox delete 0 end

    set numLinesAvail [$listbox cget -height]
    set numColsAvail [$listbox cget -width]
    
    set numLinesNeeded [llength $itemsList]
    set numColsNeeded 0
    foreach i $itemsList {
	set itemLen [string length $i]
	if {$numColsNeeded < $itemLen} { 
	    set numColsNeeded $itemLen
	}
    }
    
    # temporarily unmanage the listbox, so that if scrollbars are enabled,
    # packing happens "correctly"...
    pack forget $listbox

    # do we need scrollbars or not?
    if {$numLinesNeeded > $numLinesAvail} {
	pack $vscroll -side right -fill y
    } else {
	pack forget $vscroll
    }

    if {$numColsNeeded > $numColsAvail} {
	pack $hscroll -side bottom -fill x
    } else {
	pack forget $hscroll
    }
    
    # remanage the listbox...
    pack $listbox -anchor nw -fill both -expand 1

    # populate the list...
    foreach i $itemsList {
	$listbox insert end $i
    }
}
