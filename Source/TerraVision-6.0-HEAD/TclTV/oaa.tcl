#
# oaa.tcl
#

namespace eval OAA {
    namespace export makeOAAMenu

    #
    # module globals...
    # These are all associative arrays holding the state of the rendering 
    # options for Model Loading dialogs, and indexed by the instanceID
    #

    variable connectedHost           ;# name of current oaa connection
    variable hostField               ;# the schema URL entry widget
    variable portField               ;# the hierarchy URL entry widget
    variable idField                 ;# the hierarchy URL entry widget
    variable hostString              ;# the default schema URL string
    variable portString              ;# the default hierarchy URL string
    variable idString                ;# the default hierarchy URL string
    variable infoField               ;# the connected to text field
    variable sendtoField             ;# the "Send to" text field
    variable messageField            ;# the Send Message message widget
    variable connectButton           ;# the "Connect" button
    variable disconnectButton        ;# the "Disconnect" button
    variable sendmsgButton           ;# the "Send Message" button
    variable refreshListButton       ;# the "Refresh" button for List Viewers
    variable vListField              ;# the Viewer List text widget

    variable answerField             ;# the result of the OAA command
    variable cmdField                ;# the command entry widget
    variable optField                ;# the option entry widget

    variable initialized             ;# set true when initialized

    #
    # i n i t O A A
    # 
    # initialises all of the OAA state for a TerraVision viewer
    #
    proc initOAA {id} {
	upvar OAA::connectedHost($id) connectedHost
	upvar OAA::hostString($id) hostString
	upvar OAA::portString($id) portString
	upvar OAA::idString($id) idString
	upvar OAA::initialized($id) initialized
	upvar OAA::connectButton($id) connectButton
	upvar OAA::disconnectButton($id) disconnectButton
	upvar OAA::sendmsgButton($id) sendmsgButton
	upvar OAA::refreshListButton($id) refreshListButton
	upvar OAA::vListField($id) vListField

        if { $initialized == 1 } {
	    return 0;
	}

	set hostport   [$id.tvbase OAA get]
	set idString   [$id.tvbase get TV_OAA_ID]
	set connectedHost [$id.tvbase OAA getconnected]

	set hostString [lindex $hostport 0]
	set portString [lindex $hostport 1]

	set connectButton ""
	set sendmsgButton ""
	set refreshListButton ""
	set vListField ""

	if { $portString < 0 } { set portString "" }

	set initialized 1
    }

    #
    # u p d a t e O A A W i d g e t s
    #
    # updates the state of the Disconnect button and the status
    # bar text on the Open OAA dialog
    #
    proc updateOAAWidgets {id} {
	upvar OAA::connectedHost($id) connectedHost
	upvar OAA::hostField($id) hostField
	upvar OAA::infoField($id) infoField
	upvar OAA::connectButton($id) connectButton
	upvar OAA::disconnectButton($id) disconnectButton

	initOAA $id

	if { [string length $connectedHost] > 0 } {
	    if { [string length $connectButton] > 0 } {
		$infoField configure \
		    -text "Status: Connected to OAA on $connectedHost"
		$disconnectButton configure -state normal
		$connectButton configure -state disabled
	    }
	} else {
	    if { [string length $connectButton] > 0 } {
		$infoField configure \
		    -text "Status: Not connected to OAA facilitator"
		$disconnectButton configure -state disabled
		$connectButton configure -state normal
	    }
	}

    }

    #
    # c o n n e c t C B
    #
    # callback to connect to an OAA facilitator
    #
    proc connectCB {id win} {
	upvar OAA::connectedHost($id) connectedHost
	upvar OAA::hostField($id) hostField
	upvar OAA::portField($id) portField

	set hostname [string trim [$hostField get]]
	set portnumb [string trim [$portField get]]

	# check that we have a hostname and port number
	
	if { [string length $hostname] < 1 && [string length $portnumb] < 1 } {
	    tk_messageBox -title "TerraVision Error" -type ok -icon error \
		    -message "You have not specified a host and port number"
	    return;
	}

	# set the new host and port numbers

	$id.tvbase OAA set $hostname $portnumb

	# connect to the new schema/portnumb URLs

	$win configure -cursor watch
	update

	$id.tvbase OAA disconnect
	$id.tvbase OAA connect

	#$win configure -cursor arrow
	$win configure -cursor [$id cget -cursor]

	set connectedHost [$id.tvbase OAA getconnected]
	if { [string length $connectedHost] > 0 } {
	    # udpate the dialog widgets
	    updateOAAWidgets $id
	    # close the window
	    #OAA::dismissCB $id $win
	    return 0;
	}

	tk_messageBox -title "TerraVision Error" -type ok -icon error \
	    -message "Could not connect to OAA on $hostname:$portnumb\nPlease check that the facilitator is running."
    }

    #
    # d i s m i s s C B
    #
    # Close the Open OAA window
    #
    proc dismissCB {id win} {
	upvar OAA::hostField($id) hostField
	upvar OAA::portField($id) portField
	upvar OAA::hostString($id) hostString
	upvar OAA::portString($id) portString
	upvar OAA::connectButton($id) connectButton

	# store the new strings as the default when opening this dialog

	set hostString [string trim [$hostField get]]
	set portString [string trim [$portField get]]

	set connectButton ""

	# close the window

	destroy $win
    }
    
    #
    # d i s c o n n e c t C B
    #
    # callback to disconnect from an OAA facilitator
    #
    proc disconnectCB {id win} {
	upvar OAA::connectedHost($id) connectedHost
	upvar OAA::hostField($id) hostField
	upvar OAA::portField($id) portField

	# do the disconnect

	$win configure -cursor watch
	update

	$id.tvbase OAA disconnect

	#$win configure -cursor arrow
	$win configure -cursor [$id cget -cursor]

	set connectedHost [$id.tvbase OAA getconnected]

	# udpate the dialog widgets
	
	updateOAAWidgets $id
    }

    #
    # u d p a t e I D
    #
    # Renames this viewer's OAA ID when they press return over the 
    # OAA ID String field in the OAA Connect dialog
    #
    proc updateID {id} {
	upvar OAA::idField($id) idField
	set newval [string trim [$idField get]]
	$id.tvbase set TV_OAA_ID $newval
    }

    #
    # p o s t O p e n O A A D i a l o g
    #
    # This is the main GUI for connecting/disconnecting to OAA
    #
    proc postOpenOAADialog {id} {
	upvar OAA::hostField($id) hostField
	upvar OAA::portField($id) portField
	upvar OAA::idField($id) idField
	upvar OAA::hostString($id) hostString
	upvar OAA::portString($id) portString
	upvar OAA::idString($id) idString
	upvar OAA::infoField($id) infoField
	upvar OAA::connectButton($id) connectButton
	upvar OAA::disconnectButton($id) disconnectButton

	initOAA $id

	# make the toplevel window and set various window-options...
	set topname [format "%s_oaacon" $id ]

	# don't want more than one of these...
	if {[winfo exists $topname] == 1} {
	    wm withdraw $topname
	    wm deiconify $topname
	    return
	}
	set top [toplevel $topname]
	wm title $top [format "OAA Connect: Viewer %d" [instnum $id]]

	frame $top.f1 -relief raised -borderwidth 2
	frame $top.f1.input
	set frm $top.f1

	label $frm.l1_title -text "Facilitator Host: " \
	    -justify right -padx 5 -pady 5 
	entry $frm.l1_value -width 25
	set hostField $frm.l1_value
	grid $frm.l1_title $frm.l1_value -sticky we

	label $frm.l2_title -text "Facilitator Port: " \
	    -justify right -padx 5 -pady 5
	entry $frm.l2_value -width 25
	set portField $frm.l2_value
	grid $frm.l2_title $frm.l2_value -sticky we

	label $frm.l3_title -text "OAA ID String: " \
	    -justify right -padx 5 -pady 5
	entry $frm.l3_value -width 25
	set idField $frm.l3_value
	bind $idField <Return> "eval \{ OAA::updateID $id \}"
	grid $frm.l3_title $frm.l3_value -sticky we

	pack $frm -ipadx 10 -ipady 4 -fill both

	frame $top.info -relief raised -borderwidth 2
	label $top.info.text -justify left -background grey
	set infoField $top.info.text
	pack $top.info.text -expand 1 -fill x -side left
	pack $top.info -side top -fill x -expand 1

	$hostField insert 0 $hostString
	$portField insert 0 $portString
	$idField insert 0 $idString

	frame $top.buttons
	button $top.buttons.goto -text "  Connect  " \
	    -command "OAA::connectCB $id $top"

	button $top.buttons.discon -text "Disconnect" \
	    -command "OAA::disconnectCB $id $top"

	button $top.buttons.dismiss -text "Dismiss" \
	    -command "OAA::dismissCB $id $top"

	set connectButton $top.buttons.goto
	set disconnectButton $top.buttons.discon

	pack $top.buttons.goto -side left -expand 1 -fill x
	pack $top.buttons.discon -side left -expand 1 -fill x
	pack $top.buttons.dismiss -side left -expand 1 -fill x
	pack $top.buttons -side bottom -expand 1 -fill x

	updateOAAWidgets $id

	# make it visible...
	wm deiconify $top
    }

    #
    # r e f r e s h V i e w L i s t
    #
    proc refreshViewerList {id win} {
	upvar OAA::connectedHost($id) connectedHost
	upvar OAA::refreshListButton($id) refreshListButton
	upvar OAA::vListField($id) vListField

	$win configure -cursor watch
	update                          ;# needed to flush the cursor change

	if { [string length $connectedHost] > 0 } {
	    if { [string length $refreshListButton] > 0 } { 
		$refreshListButton configure -state normal
                refreshList $id
	    }
	} else {
	    if { [string length $refreshListButton] > 0 } { 
		$refreshListButton configure -state disabled
	    }
	    if { [string length $vListField] > 0 } { 
		$vListField configure -state normal
                $vListField delete 1.0 end
		$vListField configure -state disabled
	    }
	}

	$win configure -cursor [$id cget -cursor]
    }

    #
    # s e n d m s g C B
    #
    # callback to connect to an OAA facilitator
    #
    proc sendmsgCB {id} {
	upvar OAA::sendtoField($id) sendtoField
	upvar OAA::messageField($id) messageField
	upvar OAA::connectedHost($id) connectedHost

	set msg [string trim [$messageField get 1.0 end]]
	set opts [string trim [$sendtoField get]]
	set isself [string equal -nocase $opts "self"]

	if { [string length $connectedHost] == 0 && $isself == 0 } {
	    tk_messageBox -title "TerraVision Error" -type ok -icon error \
		-message "Not currently connected to an OAA facilitator!\nSpecify the \"self\" option to control this viewer"
	    return 0;
	}

	if { [string equal -nocase $opts "all"] == 1 } {
	    set opts ""
	}

	if { [string equal -nocase $opts "self"] == 1 } {
	    statusBarSetViewName $id $msg
	} else {
            set thisid [$id.tvbase get TV_OAA_ID]
	    $id.tvbase OAA solve "message('Message from $thisid: $msg')" \
		$opts {[reflexive(false),broadcast]}
	}
    }

    #
    # p o s t M e s s a g e D i a l o g
    #
    # This function lets users send messages to each other
    #
    proc postMessageDialog {id} {
	upvar OAA::hostField($id) hostField
	upvar OAA::portField($id) portField
	upvar OAA::idField($id) idField
	upvar OAA::hostString($id) hostString
	upvar OAA::portString($id) portString
	upvar OAA::idString($id) idString
	upvar OAA::infoField($id) infoField
	upvar OAA::sendtoField($id) sendtoField
	upvar OAA::messageField($id) messageField
	upvar OAA::sendmsgButton($id) sendmsgButton

	initOAA $id

	# make the toplevel window and set various window-options...
	set topname [format "%s_oaamsg" $id ]

	# don't want more than one of these...
	if {[winfo exists $topname] == 1} {
	    wm withdraw $topname
	    wm deiconify $topname
	    return
	}
	set top [toplevel $topname]
	wm title $top [format "Send Message: Viewer %d" [instnum $id]]

	frame $top.f1 -relief raised -borderwidth 2
	frame $top.f1.input
	set frm $top.f1

	label $frm.l1_title -text "Send to: " \
	    -justify right -padx 5 -pady 5 
	entry $frm.l1_value -width 30
	set sendtoField $frm.l1_value
	$sendtoField insert 0 "all"

	grid $frm.l1_title $frm.l1_value -sticky we
	pack $frm -ipadx 10 -ipady 4 -fill both

	frame $top.info -relief raised -borderwidth 2

	set messageField $top.info.msg
	text $messageField -width 30 -height 10
	pack $messageField -expand 1 -fill both -side top

	pack $top.info -side top -fill x -expand 1

	frame $top.buttons
	button $top.buttons.sendmsg -text "Send Message" \
	    -command "OAA::sendmsgCB $id"

	button $top.buttons.dismiss -text "Dismiss" \
	    -command "OAA::dismissMsgCB $id $top"

	set sendmsgButton $top.buttons.sendmsg

	pack $top.buttons.sendmsg -side left -expand 1 -fill x
	pack $top.buttons.dismiss -side left -expand 1 -fill x
	pack $top.buttons -side bottom -expand 1 -fill x

	# make it visible...
	wm deiconify $top
    }

    #
    # d i s m i s s M s g C B
    #
    # Close the Send Message OAA window
    #
    proc dismissMsgCB {id win} {
	upvar OAA::sendmsgButton($id) sendmsgButton
	set sendmsgButton ""
	destroy $win
    }

    #
    # p o s t L i s t A l l D i a l o g
    #
    # This function lets users send messages to each other
    #
    proc postListAllDialog {id} {
	upvar OAA::refreshListButton($id) refreshListButton
	upvar OAA::vListField($id) vListField

	initOAA $id

	# make the toplevel window and set various window-options...
	set topname [format "%s_oaalst" $id ]

	# don't want more than one of these...
	if {[winfo exists $topname] == 1} {
	    wm withdraw $topname
	    wm deiconify $topname
	    return
	}
	set top [toplevel $topname]
	wm title $top [format "List Viewers: Viewer %d" [instnum $id]]

	frame $top.f1 -relief raised -borderwidth 2

	frame $top.info -relief raised -borderwidth 2

	set vListField $top.info.msg
	text $vListField -width 60 -height 10 -wrap none -state disabled
	pack $vListField -expand 1 -fill both -side top

	pack $top.info -side top -fill x -expand 1

	frame $top.buttons
	button $top.buttons.sendmsg -text "Refresh" \
	    -command "OAA::refreshViewerList $id $top"

	button $top.buttons.dismiss -text "Dismiss" \
	    -command "OAA::dismissListCB $id $top"

	set refreshListButton $top.buttons.sendmsg

	pack $top.buttons.sendmsg -side left -expand 1 -fill x
	pack $top.buttons.dismiss -side left -expand 1 -fill x
	pack $top.buttons -side bottom -expand 1 -fill x

	# make it visible...
	wm deiconify $top

	refreshViewerList $id $top
    }

    #
    # d i s m i s s L i s t C B
    #
    # Close the List Viewers OAA window
    #
    proc dismissListCB {id win} {
	upvar OAA::refreshListButton($id) refreshListButton
	upvar OAA::vListField($id) vListField
	set refreshListButton ""
	set vListField ""
	destroy $win
    }

    #
    # r e f r e s h L i s t
    #
    # Refreshes the list of all viewers
    #
    proc refreshList {id} {
	upvar OAA::vListField($id) vListField
	upvar OAA::refreshListButton($id) refreshListButton

	if { [string length $refreshListButton] < 1 ||
	     [string length $vListField] < 1 } {
	    return 0;
	}

	$vListField configure -state normal
	$vListField delete 1.0 end

	set idList [$id.tvbase OAA list]
	set listLen [llength $idList]
        set i 0
	while { $i < $listLen } {
	    set item [lindex $idList $i]
	    set idStr [lindex $item 0]
	    set hostStr [lindex $item 1]
	    set osStr [lindex $item 2]
	    set archStr [lindex $item 3]
	    set procStr [lindex $item 4]
	    set desc "ID $idStr on $hostStr - $osStr ($archStr), $procStr"
	    if { $procStr == "1" } {
		set desc "$desc cpu\n"
	    } else {
		set desc "$desc cpus\n"
	    }
	    $vListField insert end $desc
	    incr i 1
	}

	set thisId [$id.tvbase get TV_OAA_ID]
	$vListField insert end "ID $thisId is this viewer"
	$vListField configure -state disabled
    }

    proc sendCommandCB {id} {
	upvar OAA::answerField($id) answerField
	upvar OAA::cmdField($id) cmdField
	upvar OAA::optField($id) optField
	upvar OAA::connectedHost($id) connectedHost

	set cmd [string trim [$cmdField get]]
	set opt [string trim [$optField get]]
	set isself [string equal -nocase $opt "self"]

	if { [string length $connectedHost] == 0 && $isself == 0 } {
	    tk_messageBox -title "TerraVision Error" -type ok -icon error \
		-message "Not currently connected to an OAA facilitator!\nSpecify the \"self\" option to control this viewer"
	    return 0;
	}

	if { $isself == 1 } {
	    $id.tvbase OAA interpret $cmd
	    $answerField configure -state normal
	    $answerField delete 1.0 end
	    $answerField insert end "[]"
	    $answerField configure -state disabled
	    return 0;
	}

	if { [string equal -nocase $opt "all" ]  } {
	    set opt ""
	}

	set ans [$id.tvbase OAA solve $cmd $opt {}]
	$answerField configure -state normal
	$answerField delete 1.0 end
	$answerField insert end $ans
	$answerField configure -state disabled
    }

    #
    # p o s t C o m m a n d D i a l o g
    #
    # This function lets users send OAA commands
    #
    proc postCommandDialog {id} {
	upvar OAA::answerField($id) answerField
	upvar OAA::cmdField($id) cmdField
	upvar OAA::optField($id) optField

	initOAA $id

	# make the toplevel window and set various window-options...
	set topname [format "%s_oaacmd" $id ]

	# don't want more than one of these...
	if {[winfo exists $topname] == 1} {
	    wm withdraw $topname
	    wm deiconify $topname
	    return
	}
	set top [toplevel $topname]
	wm title $top [format "OAA Command: Viewer %d" [instnum $id]]

	frame $top.f1 -relief raised -borderwidth 2

	set frm $top.f1

	label $frm.l1_title -text "Command: " \
	    -justify right -padx 5 -pady 5 
	entry $frm.l1_value -width 30
	set cmdField $frm.l1_value
	grid $frm.l1_title $frm.l1_value -sticky we

	label $frm.l2_title -text "Option(s): " \
	    -justify right -padx 5 -pady 5
	entry $frm.l2_value -width 30
	set optField $frm.l2_value
	$optField insert 0 "self"
	grid $frm.l2_title $frm.l2_value -sticky we

	pack $frm -ipadx 10 -ipady 4 -fill both

	frame $top.info -relief raised -borderwidth 2

	set answerField $top.info.msg
	text $answerField -width 40 -height 3 -state disabled
	pack $answerField -expand 1 -fill both -side top

	pack $top.info -side top -fill x -expand 1

	frame $top.buttons
	button $top.buttons.sendmsg -text "Send" \
	    -command "OAA::sendCommandCB $id"

	button $top.buttons.dismiss -text "Dismiss" \
	    -command "destroy $top"

	pack $top.buttons.sendmsg -side left -expand 1 -fill x
	pack $top.buttons.dismiss -side left -expand 1 -fill x
	pack $top.buttons -side bottom -expand 1 -fill x

	# make it visible...
	wm deiconify $top
    }

    #
    # d i s m i s s C m d C B
    #
    # Close the OAA Command window
    #
    proc dismissCmdCB {id win} {
	upvar OAA::refreshListButton($id) refreshListButton
	set refreshListButton ""
	destroy $win
    }

    #
    # m a k e G l o b a l s M e n u 
    #
    proc makeOAAMenu {id parent} {
	upvar OAA::initialized($id) initialized

	set initialized 0

	set mname $parent.oaa
	menu $mname -type tearoff

	$mname add command -label "Connect..." -underline 0 \
	    -command "OAA::postOpenOAADialog $id"

	$mname add separator

	$mname add command -label "List Viewers..." -underline 5 \
	    -command "OAA::postListAllDialog $id"

	$mname add separator

	$mname add command -label "Send Message..." -underline 5 \
	    -command "OAA::postMessageDialog $id"

	$mname add command -label "Send Command..." -underline 5 \
	    -command "OAA::postCommandDialog $id"

	return $mname
    }
}
