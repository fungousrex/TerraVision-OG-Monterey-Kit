#
# viewbookmarks.tcl
#

namespace eval ViewBookmarks {
    
    namespace export postBookmarksDialog

    # various widgets from the dialog...
    variable nameFld
    variable renameBtn
    variable addbtn
    variable delBtn
    variable setBtn
    variable vbmListbox
    variable animateBtn
    variable geosetsListbox
    variable geosetsSelection
    
    # list of bookmarked viewpoints; bookmarks are module-global, and not
    # specific to any particular viewer.
    variable bookmarks
    variable bookmarksLoadedFlag 0

    # variables for generating unique viewNames
    variable viewNamePrefix "NewView"
    variable viewNameID     0

    # view-specific bookmarks...
    variable viewerBookmarks
    variable animateMode

    # tokens returned when we register as observers to geoset/viewer events...
    variable geosetObserverToken

    # -------------------------------------------------------------------------
    # NOTE regarding datastructures:
    #
    # Bookmarks are loaded from persistent store into a list, where the
    # elements are themselves lists structured as:
    #
    #     {dataset viewName {x y z} {yaw pitch roll} gccFlag}
    #
    # The "key" into this list is considered to be the dataset/viewName pair.
    #
    # Bookmarks from persistent store don't necessarily relate to any dataset
    # the user may have open, so there is a "per viewer" list that gets built
    # on demand for each viewer.  It is simply a list of dataset/viewName 
    # pairs from those datasets currently visible by the viewer; i.e., the 
    # list elements are structured as:
    #
    #        {dataset viewName}
    #
    # Finally, to facilitate display to the user the applicable viewpoints,
    # for each unique visible dataset, there's an entry with a blank viewName:
    #
    #        {dataset ""}
    #
    # This entry serves as a sub-heading in the displayed listbox, and the
    # codes dealing with selection know how to special treat it.
    # -------------------------------------------------------------------------
    

    #
    # p o s t B o o k m a r k s D i a l o g 
    #
    proc postBookmarksDialog {id} {
	upvar ViewBookmarks::nameFld($id)    nameFld
	upvar ViewBookmarks::renameBtn($id)  renameBtn
	upvar ViewBookmarks::addBtn($id)     addBtn
	upvar ViewBookmarks::delBtn($id)     delBtn
	upvar ViewBookmarks::setBtn($id)     setBtn
	upvar ViewBookmarks::vbmListbox($id) vbmListbox
 	upvar ViewBookmarks::animateBtn($id) animateBtn
	upvar ViewBookmarks::geosetObserverToken($id) geosetObserverToken

	# create window name for dialog; if already exists, try to force to top
	set winName [vbmWindowName $id]
	if {[winfo exists $winName] == 1} {
	    wm withdraw $winName
	    wm deiconify $winName
	    return
	}

	toplevel $winName
	wm withdraw $winName
	wm title $winName [format "View Bookmarks: Viewer %d" [instnum $id]]
	wm protocol $winName WM_DELETE_WINDOW \
	    "ViewBookmarks::dismissCB $winName $id"

	# topmost frame...
	set top $winName.f0
	frame $top -borderwidth 2 -relief raised

	frame $top.f1 
	set vbmListbox [makeListBox $top.f1 -width 20 -height 12 \
			    -selectmode single -exportselection false]
	pack [getListBoxParent $vbmListbox] -expand 1 -fill both

	set bindscript "eval \{ViewBookmarks::selectBookmark $id\}"
	bind $vbmListbox <ButtonRelease-1> $bindscript

	set bindscript "eval \{ViewBookmarks::setCB $id\}"
	bind $vbmListbox <Double-Button-1> $bindscript
	
	# right side / action area...
	frame $top.f2

	set renameBtn $top.f2.b1
	button $renameBtn -text "Rename:" \
	    -command "ViewBookmarks::renameCB $id"
	set nameFld $top.f2.e
	entry $nameFld -width 12 -state disabled -exportselection false
	set bindscript "eval \{ViewBookmarks::renameCB $id\}"
	bind $nameFld <Return> $bindscript

	set addBtn $top.f2.b2
	button $addBtn -text "Add View" \
	    -command "ViewBookmarks::addCB $id"

	set delBtn $top.f2.b3
	button $delBtn -text "Remove View" \
	    -command "ViewBookmarks::removeCB $id"

	set setBtn $top.f2.b4
	button $setBtn -text "Set View" \
	    -command "ViewBookmarks::setCB $id"

	set animateBtn $top.f2.cbox
	set animateMode($id) 0
	checkbutton $animateBtn -text "animate" \
	    -variable ViewBookmarks::animateMode($id)

	pack $animateBtn  -side bottom -anchor w -expand 1 -pady 5
	pack $setBtn -side bottom -expand 1 -fill x
	pack $delBtn $addBtn -side bottom -fill x -pady 5 
	pack $renameBtn $nameFld -side left -anchor w -expand 1 -fill x \
	    -pady 5

	pack $top.f1 $top.f2 -side left -expand 1 -fill both -padx 5 -pady 5

	button $winName.dismiss -text "Dismiss" \
	    -command "ViewBookmarks::dismissCB $winName $id"

	pack $top -side top -expand 1 -fill both
	pack $winName.dismiss -side top -expand 1 -fill x

	loadBookmarks
	refreshBookmarksList $id

	# register as an observer of geoset-change events...
	set geosetObserverToken [DatasetDialog::registerDatasetObserver \
				     "ViewBookmarks::refreshBookmarksList $id"]

	wm deiconify $winName
    }


    #
    # r e f r e s h B o o k m a r k s L i s t 
    #
    # Loads the "bookmarks" listbox with any viewer-specific bookmarks.
    #
    proc refreshBookmarksList {id} {
	global gWinList

	# we actually perform this task for any and all viewers that 
	# may have visible bookmark dialogs...
	foreach viewer $gWinList {
	    if {[winfo exists [vbmWindowName $viewer]]} {
		refreshBookmarksWorkProc $viewer
	    }
	}
    }

    #
    # r e f r e s h B o o k m a r k s W o r k P r o c
    #
    # Work proc for "refreshBookmarksList" above.
    #
    proc refreshBookmarksWorkProc {id} {
	upvar ViewBookmarks::bookmarks bookmarks
	upvar ViewBookmarks::viewerBookmarks($id) viewerBookmarks
	upvar ViewBookmarks::vbmListbox($id) vbmListbox

	$vbmListbox delete 0 end

	# get list of currently visible geosets in this viewer...
	set viewerGeosetsList [$id.tvbase geoset list]

	# reset the viewer-specific list of bookmarks...
	set viewerBookmarks [list]

	foreach bm $bookmarks {
	    # pick out this bookmark's geoset name...
	    set geoset [lindex $bm 0]

	    # is it also in the viewer's geoset list?
	    if {[lsearch $viewerGeosetsList $geoset] == -1} { continue }
	    # found it; add to the viewerBookmarks list...
	    set viewName [lindex $bm 1]
	    lappend viewerBookmarks [list $geoset $viewName]
	    
	    # we want the "special header" record in the list too, which is 
	    # useful later on for creating the elements of the listbox...
	    set headerRec [list $geoset ""]
	    if {[lsearch -exact $viewerBookmarks $headerRec] == -1} {
		lappend viewerBookmarks $headerRec
	    }
	}

	# We now have a viewer-specific list of lists containing 
	# bookmark references.  Sort them, and create a list suitable for
	# display...
	
	# This is an internal compare routine for the "lsort" below...
	proc compareProc {list1 list2} {
	    set elem1 [lindex $list1 0]
	    set elem2 [lindex $list2 0]
	    if {$elem1 < $elem2} {
		return -1
	    } elseif {$elem1 > $elem2} {
		return 1
	    }

	    # compare second elements as tie breaker...
	    set elem1 [lindex $list1 1]
	    set elem2 [lindex $list2 1]
	    if {$elem1 < $elem2} {
		return -1
	    } elseif {$elem1 > $elem2} {
		return 1
	    }
	    return 0
	}

	set viewerBookmarks [lsort -command compareProc $viewerBookmarks]

	set displayList [list]
	foreach bm $viewerBookmarks {
	    set geoset [lindex $bm 0]
	    set viewName [lindex $bm 1]
	    if {$viewName == ""} {
		lappend displayList $geoset
	    } else {
		lappend displayList [format "    %s" $viewName]
	    }
	}

	populateListBox $vbmListbox $displayList
	selectBookmark $id
    }


    #
    # s e l e c t B o o k m a r k 
    #
    # Called when things are selected/added/deleted to the bookmarks listbox,
    # to enable/disable the GUI buttons as appropriate.
    #
    proc selectBookmark {id} {
	upvar ViewBookmarks::nameFld($id)    nameFld
	upvar ViewBookmarks::renameBtn($id)  renameBtn
	upvar ViewBookmarks::addBtn($id)     addBtn
	upvar ViewBookmarks::delBtn($id)     delBtn
	upvar ViewBookmarks::setBtn($id)     setBtn
	upvar ViewBookmarks::vbmListbox($id) vbmListbox
	upvar ViewBookmarks::viewerBookmarks($id) viewerBookmarks

	set selIndex [$vbmListbox curselection]
	if {[llength $selIndex] == 0} {
	    $nameFld delete 0 end
	    $nameFld configure -state disabled
	    $renameBtn configure -state disabled
	    $delBtn configure -state disabled
	    $setBtn configure -state disabled
	    return
	}

	# get the selection, and if it's a geoset label (as opposed to a 
	# bookmark label), deselect and call this proc recursively.
	set theSelection [$vbmListbox get $selIndex]
	if {[string index $theSelection 0] != " "} {
		$vbmListbox selection clear 0 end
		selectBookmark $id
		return
	}

	# if we're here, a bookmark has been selected, so enable GUI...
	$nameFld configure -state normal
	$nameFld delete 0 end
	$nameFld insert 0 [string trim [$vbmListbox get $selIndex]]
	$renameBtn configure -state normal
	$delBtn configure -state normal
	$setBtn configure -state normal	
    }


    #
    # r e n a m e C B
    #
    # Callback invoked on a "rename bookmark" action.
    #
    proc renameCB {id} {
	upvar ViewBookmarks::nameFld($id)         nameFld
	upvar ViewBookmarks::vbmListbox($id)      vbmListbox
	upvar ViewBookmarks::viewerBookmarks($id) viewerBookmarks
	upvar ViewBookmarks::bookmarks            bookmarks

	set key [lindex $viewerBookmarks [$vbmListbox curselection]]
	set bmIndex [findBookmark $key]
	if {$bmIndex == -1} { return }
	set theBookmark [lindex $bookmarks $bmIndex]

	# extract the old viewName...
	set oldName [lindex $theBookmark 1]

	# get the new one...
	set newName [$nameFld get]

	if {$newName == ""} {
	    tk_messageBox -icon error -type yes -title "Renaming Error" \
		-message "Can't have an empty view-name; please enter something"
	    return
	}

	if {$oldName == $newName} { return }

	# update our Tcl list
	set theBookmark [lreplace $theBookmark 1 1 $newName]
	set bookmarks [lreplace $bookmarks $bmIndex $bmIndex $theBookmark]

	# tell tvutils about the new change
	tvutils bookmarks remove [lindex $theBookmark 0] $oldName
	tvutils bookmarks add $theBookmark

	refreshBookmarksList $id
	findAndSelect $id [lindex $key 0] $newName
	saveBookmarks
    }

    #
    # a d d C B 
    #
    # Callback invoked when a new bookmark is added to the set.
    #
    proc addCB {id} {
	upvar ViewBookmarks::vbmListbox($id)      vbmListbox
	upvar ViewBookmarks::viewerBookmarks($id) viewerBookmarks
	upvar ViewBookmarks::bookmarks            bookmarks
	upvar ViewBookmarks::viewNamePrefix       viewNamePrefix
	upvar ViewBookmarks::viewNameID           viewNameID

	# if there's more than one visible dataset, we request the user
	# specify which one to associate with the new bookmark...
	set geosets [$id.tvbase geoset list]
	set numGeosets [llength $geosets]

	if {$numGeosets == 0} { return }
	if {$numGeosets == 1} {
	    set theGeoset [lindex $geosets 0]
	} else {
	    set theGeoset [postGeosetSelectionDialog $id $geosets]
	    if {$theGeoset == ""} { return }
	}

	# gather the current view parameters...
	set currView [$id.tvbase view current]
	
	set eyePnt [getViewEyePoint $currView]
	set ypr    [getViewYPR $currView]
	
	# make one of our bookmark records out of the info (using a 
	# generated viewName), and place it into the bookmarks list...
	set viewName [format "%s%02d" $viewNamePrefix [incr viewNameID]]

	set bookmark [list $theGeoset $viewName $eyePnt $ypr 1]
	lappend bookmarks $bookmark

	# add this bookmark to the tvutils list of bookmarks */
	tvutils bookmarks add $bookmark
	
	refreshBookmarksList $id	
	findAndSelect $id $theGeoset $viewName
	saveBookmarks
    }

    #
    # r e m o v e C B
    #
    # Callback invoked when a bookmark is deleted.
    #
    proc removeCB {id} {
	upvar ViewBookmarks::vbmListbox($id)      vbmListbox
	upvar ViewBookmarks::viewerBookmarks($id) viewerBookmarks
	upvar ViewBookmarks::bookmarks            bookmarks

	# find the selected bookmark...
	set key [lindex $viewerBookmarks [$vbmListbox curselection]]
	set bmIndex [findBookmark $key]
	if {$bmIndex == -1} { return }

	set bmItem    [lindex $bookmarks $bmIndex]
	set bmDataset [lindex $bmItem 0]
	set bmName    [lindex $bmItem 1]

	set answer [tk_messageBox -icon question -title "Confirm Deletion" \
			-type yesno \
			-message "Permanently remove bookmark \"$bmName\"?"]
	if {$answer == "yes"} {
	    set bookmarks [lreplace $bookmarks $bmIndex $bmIndex]
	    tvutils bookmarks remove $bmDataset $bmName
	    refreshBookmarksList $id
	    saveBookmarks
	}
    }

    #
    # s e t C B
    #
    # Callback invoked when a bookmark is "activated".
    #
    proc setCB {id} {
	upvar ViewBookmarks::vbmListbox($id)      vbmListbox
	upvar ViewBookmarks::viewerBookmarks($id) viewerBookmarks
	upvar ViewBookmarks::bookmarks            bookmarks
	upvar ViewBookmarks::animateMode($id)     animateMode
	
	# get the selection index, use it to look up the "key" in the
	# viewerBookmarks...
	set key [lindex $viewerBookmarks [$vbmListbox curselection]]
	set bmIndex [findBookmark $key]
	if {$bmIndex == -1} { return }
	set theBookmark [lindex $bookmarks $bmIndex]

	# We should convert to a GCC-based view-ref if currently local-based, 
	# but...  we're going to punt here as this is impossible to test at 
	# the moment, lacking test examples :-(   --RLB.
	set gcc [lindex $theBookmark 4]
	if {$gcc == 0} {
	    tk_messageBox -title "TerraVision Error" -type yes -icon warning \
		-message "Encountered an older, local-coordinate view-reference;  can't cope with these presently"
	    return
	}

	# pull out the viewing parameters...
	set coords [lindex $theBookmark 2]
	set ypr    [lindex $theBookmark 3]

	if {$animateMode == 1} {
	    $id.tvbase view flyto [lindex $coords 0] [lindex $coords 1] \
		[lindex $coords 2] [lindex $ypr 0] [lindex $ypr 1]  \
		[lindex $ypr 2]
	} else {
	    $id.tvbase view set3D [lindex $coords 0] [lindex $coords 1] \
		[lindex $coords 2] [lindex $ypr 0] [lindex $ypr 1]  \
		[lindex $ypr 2]
	}

	# update the status bar...
	statusBarSetViewName $id [lindex $theBookmark 1]	
    }

    #
    # f i n d B o o k m a r k 
    #
    # Utility proc, called to find a bookmark in the module-global "bookmarks",
    # given a "key" (i.e., dataset/viewName pair).
    #
    # Returns the index into "bookmarks" if found, -1 otherwise.
    #
    proc findBookmark {key} {
	upvar ViewBookmarks::bookmarks  bookmarks

	set dataset [lindex $key 0]
	set viewName [lindex $key 1]

	# iterate over the bookmarks of each dataset until we find a match...
	set index 0
	foreach bm $bookmarks {
	    if {$dataset == [lindex $bm 0] && \
		    $viewName == [lindex $bm 1]} {
		return $index
	    }
	    incr index
	}

	return -1
    }

    #
    # f i n d A n d S e l e c t 
    #
    # Utility proc, given a geoset/viewName pair, finds the occurance in 
    # the displayed listbox, and "selects" it...
    #
    proc findAndSelect {id geoset viewName} {
	upvar ViewBookmarks::vbmListbox($id)       vbmListbox
	upvar ViewBookmarks::viewerBookmarks($id)  viewerBookmarks
	
	set index 0
	foreach bm $viewerBookmarks {
	    if {$geoset == [lindex $bm 0] && $viewName == [lindex $bm 1]} {
		$vbmListbox selection set $index
		selectBookmark $id
		break
	    }
	    incr index
	}

	# silently returns if not found...
    }

    #
    # p o s t G e o s e t S e l e c t i o n D i a l o g
    #
    # Presents a list of opened geosets, and requests the user to choose one
    # to associate with a newly added bookmark.
    # Returns the selected geoset name, or "" on a cancel.
    #
    proc postGeosetSelectionDialog {id geosets} {
	upvar ViewBookmarks::geosetsListbox($id)   geosetsListbox
	upvar ViewBookmarks::geosetsSelection($id) geosetsSelection
	
	set top [format "%s_pgsd" $id]
	toplevel $top -class Dialog
	wm withdraw $top
	wm title $top "Select Geoset"
	wm transient $top [vbmWindowName $id]

	frame $top.f1 -borderwidth 2 -relief raised
	label $top.f1.l -text "Select a geoset to associate with the new view:"
	set geosetsListbox [makeListBox $top.f1 -width 20 \
				-height [llength $geosets]]
	pack $top.f1.l [getListBoxParent $geosetsListbox] \
	    -side top -anchor w -expand 1 -fill both

	frame $top.f2 -borderwidth 2 -relief raised

	set okScript "eval \{set ViewBookmarks::geosetsSelection($id) \
               \[$ViewBookmarks::geosetsListbox($id) get \
                   \[$ViewBookmarks::geosetsListbox($id) curselection \]\] ; \
                       destroy $top\}"
	button $top.f2.ok -text "Select" \
	    -command $okScript

	set cancelScript "eval \{set ViewBookmarks::geosetsSelection($id) \"\"\
                ; destroy $top \}"
	button $top.f2.cancel -text "cancel" \
	    -command $cancelScript
	pack $top.f2.ok $top.f2.cancel -side left -expand 1 -pady 5

	pack $top.f1 -side top -expand 1 -fill both
	pack $top.f2 -side top -expand 1 -fill x

	foreach i $geosets {
	    $geosetsListbox insert end $i
	}
	$geosetsListbox selection set 0  ;# preselect 1st item...

	wm deiconify $top
	tkwait window $top
	
	return $geosetsSelection
    }


    #
    # a n i m a t e C B 
    #
    # Callback for the "animate bookmarks" checkbutton.
    #
    proc animateCB {id} {

    }

    #
    # l o a d B o o k m a r k s 
    #
    # Reads and loads the bookmarks file using the tvutils functions.
    #
    # Bookmarks are kept as a list of "view records", which in turn are lists
    # structured as:
    #
    proc loadBookmarks {} {
	upvar ViewBookmarks::bookmarks bookmarks
	upvar ViewBookmarks::bookmarksLoadedFlag bookmarksLoadedFlag

	if {$bookmarksLoadedFlag} { return }

	tvutils bookmarks open
	set count [tvutils bookmarks size]

	for {set i 0} {$i < $count} {set i [expr $i + 1]} {
	    set viewRec [tvutils bookmarks entry $i]
	    if {[llength $viewRec]} {
		lappend bookmarks $viewRec
	    }
	}

	set bookmarksLoadedFlag 1
	return;
    }

    #
    # s a v e B o o k m a r k s 
    #
    # Writes the global bookmarks list to persistent store.
    #
    proc saveBookmarks {} {
	tvutils bookmarks write
    }

    #
    # d i s m i s s C B 
    #
    # Destroys the dialog widgets and unsets the instance variables...
    #
    proc dismissCB {winName id} {
	upvar ViewBookmarks::nameFld($id)             nameFld
	upvar ViewBookmarks::renameBtn($id)           renameBtn
	upvar ViewBookmarks::addBtn($id)              addBtn
	upvar ViewBookmarks::delBtn($id)              delBtn
	upvar ViewBookmarks::setBtn($id)              setBtn
	upvar ViewBookmarks::vbmListbox($id)          vbmListbox
 	upvar ViewBookmarks::animateBtn($id)          animateBtn
	upvar ViewBookmarks::viewerBookmarks($id)     viewerBookmarks
	upvar ViewBookmarks::geosetObserverToken($id) geosetObserverToken

	# these may not exist for a particular id, so more care needed...
	upvar ViewBookmarks::geosetsListbox($id)   geosetsListbox
	upvar ViewBookmarks::geosetsSelection($id) geosetsSelection

 	destroy $winName

	unset nameFld
	unset renameBtn
	unset addBtn
	unset setBtn
	unset vbmListbox
	unset animateBtn
	unset viewerBookmarks

	if {[info exists geosetsListbox($id)]} {unset geosetsListbox}
	if {[info exists geosetsSelection($id)]} {unset geosetsSelection}

	# unregister as an observer of dataset open/close events...
	DatasetDialog::unregisterDatasetObserver $geosetObserverToken
	unset geosetObserverToken
    }

    #
    # v b m W i n d o w N a m e
    #
    # Convenience proc to return the name of the top-level window for
    # the view bookmarks dialog.
    #
    proc vbmWindowName {id} {
	return [format "%s_vbm" $id]
    }

}
