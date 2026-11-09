#
# DatasetDialog.tcl
#
namespace eval DatasetDialog {

    namespace export postOpenDSDialog postCloseDSDialog postInfoDSDialog \
	registerDatasetObserver unregisterDatasetObserver \
        postConsole addToConsole loadedGeoset

    #
    # Implementation notes:
    #
    # Datasets and Databases are global settings. Thus, we permit only one 
    # instance (a global one at that) of any of the dialogs.  Those are:
    #
    #    .dsDialog          is the "main" geoset selection dialog
    #    .dsServers         is the add/remove/reorder server dialog
    #    .dsAdvanced        is the "advanced" server options dialog
    #    .dsAddServer       prompt to enter new server URL...
    #    .dsClose           is the "Close Dataset" dialog
    #    .dsProps           statistics and info on datasets
    #


    # module statics...
    variable geosetsList        ;# list widget displaying available geosets
    variable imageList          ;# list widget displaying images of a geoset
    variable demList            ;#  "     "        "        dems "  "   "
    variable imageServersList   ;#  "     "        "      image servers
    variable demServersList     ;#  "     "        "       dem servers
    variable openDSButton       ;# button widget for  "Open Dataset"
    variable databaseLabelField

    variable removeServerBtn    ;# widgets on the "servers" dialog
    variable moveServerUPBtn
    variable moveServerDNBtn
    variable readServerBtn
    variable readAllServersBtn
    variable serversList
    variable serverInfo

    # list of dataset-change observer callbacks
    variable observerCallbacks
    variable observerToken 0

    # observer tokens maintained for the "Close" and "Properties" dialogs...
    variable closeObserverToken
    variable propsObserverToken

    variable geosetsCloseList   ;# list widget showing opened geosets
    variable infoListbox        ;# list widget for "Properties" display
    variable infoTextbox        ;# output are for "Properties" display
    variable aggregateStr "**Aggregate Report**"
    
    variable consoleTextbox     ;# text widget for the dataset console

    variable readServerLabel    ;# label on the Read Server window

    #
    # s e l e c t D B a s e
    #
    # Let's the user select a new database file using a file requester
    #
    proc selectDBase {} {
	upvar DatasetDialog::databaseLabelField databaseField

	# get the database filename from the user...
	set file [tk_getOpenFile -initialdir [tvutils get TVU_USER_DIR] \
		      -defaultextension .xml -title "Load a Database File"]
	
	# non-empty string indicates "something" was returned
	if {[string length $file]} {
	    $databaseField delete 0 end
	    $databaseField insert 0 $file
	    DatasetDialog::changeDatabase
	}
    }

    #
    # p o s t O p e n D S D i a l o g 
    #
    proc postOpenDSDialog {} {
	global gTheDatabase
	variable geosetsList
	variable databaseLabelField
	variable imageList
	variable demList
	variable imageServersList
	variable demServersList
	variable openDSButton

	# create a new TV database if we don't have one loaded
	if {![tsmDatabase exist]} {
	    set dbaseName [tvutils get TVU_DATABASE_URL]
	    tk_messageBox -title "TerraVision Dialog" -type ok \
              -message "Created a new TerraVision database called:\n$dbaseName"
	    tsmDatabase new $dbaseName
	}
	
	# if window already exists, try to force it to the top...
	if {[winfo exists .dsDialog]} {
	    wm withdraw .dsDialog
	    wm deiconify .dsDialog
	    return 
	}

	toplevel .dsDialog -class Dialog
	wm withdraw .dsDialog
	wm title .dsDialog "Open TerraVision Dataset"
	
	# top frame: name of database...
	set f1 .dsDialog.f1
	frame $f1 -relief raised -borderwidth 2
	set padding 4
	label $f1.l -text "Database: "
	
	frame $f1.e
	entry $f1.e.e
	set databaseLabelField $f1.e.e   ; # namespace "global" 
	$databaseLabelField insert 0 $gTheDatabase
	bind $databaseLabelField <Return> {
	    DatasetDialog::changeDatabase
	}
	button $f1.e.f -text "..." -command DatasetDialog::selectDBase -padx 4 -pady 0

	pack $f1.e.e -in $f1.e -expand true -fill x -side left -padx 0 -pady 0
	pack $f1.e.f -in $f1.e -side right -padx 0 -pady 0
	
	button $f1.b -text "Servers..." -command DatasetDialog::postServers
	pack $f1.l -in $f1 -side left -padx $padding -pady $padding
	pack $f1.b -in $f1 -side right -padx $padding -pady $padding
	pack $f1.e -in $f1 -expand true -fill x
	
	# middle frame, composed of three subframes...
	set f2Pad 2
	set f2 .dsDialog.f2
	frame $f2 -relief raised -borderwidth 2
	
	# subframe 1:  the geosets list...

	set f21 $f2.1
	frame $f21
	label $f21.l -text "GeoSets"
	set geosetsList [makeListBox $f21 -width 25 -exportselection false]
	set geosetsParent [getListBoxParent $geosetsList]

	pack $f21.l -in $f21 -side top -fill x -pady $f2Pad
	pack [getListBoxParent $geosetsList] -in $f21 -side bottom \
	    -expand true -fill both -pady $f2Pad
	
	# subframe 2:  the image/dem location lists...

	set f22 $f2.2
	frame $f22
	label $f22.image -text "Image Location"
	set imageList [makeListBox $f22 -width 40 -exportselection false]
	
	label $f22.dem -text "DEM Location"
	set demList [makeListBox $f22 -width 40 -exportselection false]
	
	pack $f22.image -in $f22 -side top -fill x -pady $f2Pad
	pack [getListBoxParent $imageList] -in $f22 -expand yes -fill both \
	    -pady $f2Pad
	pack $f22.dem -in $f22 -fill x -pady $f2Pad
	pack [getListBoxParent $demList] -in $f22 -expand true -fill both \
	    -pady $f2Pad

	if {0} {   ;# THIS IS POTENTIALLY OBSOLETE -- SKIP IMPL. FOR NOW...
	# subframe 3:  image/dem servers...
	set f23 $f2.3
	frame $f23
	label $f23.iserver -text "Image Servers"
	listbox $f23.iserverList
	set imageServersList $f23.iserverList
	
	button $f23.b1 -text "Advanced..." \
	    -command "DatasetDialog::postAdvanced image"
	label $f23.dserver -text "DEM Servers"
	listbox $f23.dserverList 
	set demServersList $f23.dserverList
	
	button $f23.b2 -text "Advanced..." \
	    -command "DatasetDialog::postAdvanced dem"
	pack $f23.iserver -in $f23 -side top -fill x -pady $f2Pad
	pack $f23.iserverList -in $f23 -expand true -fill both -pady $f2Pad
	pack $f23.b1 -in $f23 -fill x -pady $f2Pad
	pack $f23.dserver -in $f23 -fill x -pady $f2Pad
	pack $f23.dserverList -in $f23 -expand true -fill both -pady $f2Pad
	pack $f23.b2 -in $f23 -fill x -pady $f2Pad

	pack $f21 $f22 $f23 -in $f2 -side left -expand true -fill both \
	    -padx 4 -pady 4
	} ;#END
	pack $f21 $f22 -in $f2 -side left -expand true -fill both \
	    -padx 4 -pady 4
	
	# bottom frame:  the action area...
	set f3 .dsDialog.f3
	frame $f3 -relief raised -borderwidth 2
	button $f3.ok -text "Open Dataset"  -state disabled \
	    -command "DatasetDialog::openGeoset"
	set openDSButton $f3.ok

	button $f3.cancel -text "Cancel"  \
	    -command "destroy .dsDialog ; \
                      if \{\[winfo exists .dsServers\]\} \{destroy .dsServers\}"
	pack $f3.ok -in $f3 -side left -padx 10
	pack $f3.cancel -in $f3 -side right -padx 10
	
	pack $f1 -side top -expand true -fill x -ipady 4
	pack $f2 -expand true -fill both -ipady 4
	pack $f3 -expand true -fill x -ipady 4
	
	# need to populate the list of available geosets in the database...
	populateListBox $geosetsList [tsmDatabase geoset list]

	# set the bindings for the various listboxes...
	bind $geosetsList <Double-Button-1> {
	    DatasetDialog::openGeoset
	}
	bind $geosetsList <ButtonRelease-1> { 
	    DatasetDialog::selectGeoset
	}

	# make it visible...
	wm deiconify .dsDialog
    }

    #
    # p o s t C l o s e D S D i a l o g 
    #
    proc postCloseDSDialog {} {
	global gGeosetsList
	variable geosetsCloseList
	variable closeObserverToken

	# if window already exists, try to force it to the top...
	set winName .dsClose
	if {[winfo exists $winName]} {
	    wm withdraw $winName
	    wm deiconify $winName
	    return 
	}

	toplevel $winName -class Dialog
	wm withdraw $winName
	wm title $winName "Close TerraVision Dataset"

	# upper portion...
	set top $winName
	frame $top.f1 -borderwidth 2 -relief raised
	label $top.f1.title -text "Select terrain dataset to be closed:"
	set geosetsCloseList [makeListBox $top.f1 -width 40 -height 12]

	pack $top.f1.title -side top -anchor nw
	pack [getListBoxParent $geosetsCloseList] -side top -anchor n \
	    -expand 1 -fill both -padx 5 -pady 5

	# action area...
	frame $top.f2 -borderwidth 2 -relief raised
	button $top.f2.ok -text "Close Terrain" \
	    -command "DatasetDialog::closeDataset close"
	button $top.f2.cancel -text "Cancel" \
	    -command "DatasetDialog::closeDataset cancel"

	pack $top.f2.ok $top.f2.cancel -side left -expand 1 -fill x

	pack $top.f1 -side top -expand 1 -fill both
	pack $top.f2 -side top -expand 1 -fill x
	
	populateListBox $geosetsCloseList $gGeosetsList
	wm deiconify $winName

	# even though quite unlikely this dialog will be visible while
	# someone opens a new geoset, register has a change observer...
	set closeObserverToken \
	    [registerDatasetObserver "global gGeosetsList; \
                $geosetsCloseList delete 0 end; \
                populateListBox $geosetsCloseList \$gGeosetsList"]
    }    

    #
    # p o s t I n f o D S D i a l o g 
    #
    proc postInfoDSDialog {} {
	variable infoListbox
	variable infoTextbox
	variable aggregateStr

	# if window already exists, try to force it to the top...
	set winName .dsInfo
	if {[winfo exists $winName]} {
	    wm withdraw $winName
	    wm deiconify $winName
	    return 
	}

	toplevel $winName -class Dialog
	wm withdraw $winName
	wm title $winName "Dataset Properties"
	
	set top $winName
	frame $top.f0 -borderwidth 2 -relief raised
	set top $top.f0 

	frame $top.f1	
	label $top.f1.l -text "Select item to detail:"
	set infoListbox [makeListBox $top.f1 -width 30]

	bind $infoListbox <ButtonRelease-1> {
	    DatasetDialog::infoCB
	}

	pack $top.f1.l -side top -anchor nw
	pack [getListBoxParent $infoListbox] -side top -expand 1 -fill both

	frame $top.f2
	label $top.f2.l -text "Details"
	set infoTextbox $top.f2.tb
	text $infoTextbox -width 40 -state disabled -relief groove -wrap none \
	    -xscrollcommand "$top.f2.hsb set" \
	    -yscrollcommand "$top.f2.vsb set"
	scrollbar $top.f2.vsb -orient vertical \
	    -command "$infoTextbox yview"
	scrollbar $top.f2.hsb -orient horizontal \
	    -command "$infoTextbox xview"

	pack $top.f2.l -side top -anchor n
	pack $top.f2.hsb -side bottom -expand 1 -fill x 
	pack $infoTextbox -side left -expand 1 -fill both
	pack $top.f2.vsb -side left -expand 1 -fill y

	pack $top.f1 $top.f2 -side left -expand 1 -fill both
	pack $top -side top -expand 1 -fill both

	# action area...
	button $winName.dismiss -text "Dismiss" ;# -command set below...
	pack $winName.dismiss -side top -expand 1 -fill x

	set geoList [tsGeoPyramid list]
	set geoList [linsert $geoList 0 $aggregateStr]

	populateListBox $infoListbox $geoList

	wm deiconify $winName 
	
	# register as an observer of dataset-change events...
	set propsObserverToken \
	    [registerDatasetObserver "DatasetDialog::propsObserverAction"]

	$winName.dismiss configure -command "destroy .dsInfo\; \
            DatasetDialog::unregisterDatasetObserver $propsObserverToken"
    }


    #
    # p o s t S e r v e r s 
    #
    proc postServers {} {
	global gTheDatabase
	upvar DatasetDialog::removeServerBtn removeServerBtn
	upvar DatasetDialog::moveServerUPBtn moveServerUPBtn
	upvar DatasetDialog::moveServerDNBtn moveServerDNBtn
	upvar DatasetDialog::readServerBtn readServerBtn
	upvar DatasetDialog::readAllServersBtn readAllServersBtn
	upvar DatasetDialog::serversList serversList
	upvar DatasetDialog::serverInfo serverInfo

	# if window already exists, try to force it to the top...
	if {[winfo exists .dsServers]} {
	    wm withdraw .dsServers
	    wm deiconify .dsServers
	    return 
	}

	toplevel .dsServers -class Dialog
	wm withdraw .dsServers
	wm title .dsServers "Database Servers"
	
	# top part...
	set f1 .dsServers.f1
	frame $f1 -relief raised -borderwidth 2
	
	# this has a subframe area containing a bunch of buttons...
	set f11 $f1.1
	frame $f11
	button $f11.addServer -text "Add Server" \
	    -command "DatasetDialog::addServerCB"

	button $f11.removeServer -text "Remove Server" -state disabled \
	    -command "DatasetDialog::removeServerCB"
	set removeServerBtn $f11.removeServer

	button $f11.up -text "Move Up" -state disabled \
	    -command "DatasetDialog::moveServerCB UP"
	set moveServerUPBtn $f11.up

	button $f11.down -text "Move Down" -state disabled \
	    -command "DatasetDialog::moveServerCB DOWN"
	set moveServerDNBtn $f11.down

	button $f11.readServer -text "Re-read Server" -state disabled \
	    -command "DatasetDialog::readServerCB SELECTED"
	set readServerBtn $f11.readServer

	button $f11.readAll -text "Re-read All" -state disabled \
	    -command "DatasetDialog::readServerCB ALL"
	set readAllServersBtn $f11.readAll

	pack $f11.addServer $f11.removeServer -in $f11 -side top -fill x \
	    -padx 8 -pady 2 
	pack $f11.up $f11.down -in $f11 -fill x -padx 8 -pady 8
	pack $f11.readServer $f11.readAll -in $f11 -fill x -padx 8 -pady 2
	
	set serversList [makeListBox $f1 -width 40]

	label $f1.label -relief ridge -justify center
	set serverInfo $f1.label

	pack $f1.label -in $f1 -side bottom -expand true -fill x -pady 4
	pack [getListBoxParent $serversList] -in $f1 -side left \
	    -expand true -fill both
	pack $f11 -in $f1 -side right -fill y
	
	# action area...
	set f2 .dsServers.f2
	frame $f2 -relief raised -borderwidth 2
	button $f2.dismiss -text "Dismiss" -command "destroy .dsServers"
	pack $f2.dismiss -in $f2 -side left -padx 8 -pady 8
	
	pack $f1 -in .dsServers -side top -expand true -fill both
	pack $f2 -in .dsServers -side bottom -expand true -fill x

	# finally, populate the listbox with any available servers...
	populateListBox $serversList [tsmDatabase server list]
	if {[$serversList size] > 0} {
	    $readAllServersBtn configure -state normal
	}

	bind $serversList <ButtonRelease-1> { 
	    DatasetDialog::selectServer
	}

	# make the dialog visible...
	wm deiconify .dsServers
	focus .dsServers
    }
    
    
    #
    # p o s t A d v a n c e d 
    #
    # Potentially obsolete -- skip implementation for now.
    #
    proc postAdvanced imageOrDem {
	toplevel .adv -class Dialog
	wm withdraw .adv
	if {$imageOrDem == "image"} {
	    set title "Advanced Imagery Settings"
	} else {
	    set title "Advanced DEM Settings"
	}
	wm title .adv $title
	
	set padding 4
	
	# top part...
	set f1 .adv.f1
	frame $f1 -relief raised -borderwidth 2
	
	# upper part consists of three subframes, plus a couple of checkboxes...
	frame $f1.a
	set f11 $f1.a.1
	frame $f11
	label $f11.l -text "Image Format"
	frame $f11.f -relief sunken -borderwidth 2
	radiobutton $f11.f.b1 -text "Raw uncompressed" -variable f11Var -value raw
	radiobutton $f11.f.b2 -text "JPEG compressed"  -variable f11Var -value jpg
	radiobutton $f11.f.b3 -text "LZW compressed"   -variable f11Var -value lzw
	pack $f11.f.b1 $f11.f.b2 $f11.f.b3 -in $f11.f -side top \
	    -padx $padding -pady $padding -anchor w
	pack $f11.l -in $f11 -side top
	pack $f11.f -in $f11 -expand true -fill both
	$f11.f.b1 invoke
	
	# 2nd subframe...
	set f12 $f1.a.2
	frame $f12
	label $f12.l -text "Local Tile Format"
	frame $f12.f -relief sunken -borderwidth 2
	radiobutton $f12.f.b1 -text "TIFF files (.tiff)" \
	    -variable f12Var -value tiff
	radiobutton $f12.f.b2 -text "Raw tiles (.tile)" \
	    -variable f12Var -value raw
	pack $f12.f.b1 $f12.f.b2 -in $f12.f -side top \
	    -padx $padding -pady $padding -anchor w
	pack $f12.l -in $f12 -side top
	pack $f12.f -in $f12 -expand true -fill both
	$f12.f.b1 invoke
	
	# 3rd subframe...
	set f13 $f1.a.3
	frame $f13
	label $f13.l -text "DPSS tspecs"
	frame $f13.f -relief sunken -borderwidth 2
	radiobutton $f13.f.b1 -text "Check on DPSS first"  \
	    -variable f13Var -value DPSS
	radiobutton $f13.f.b2 -text "Check Metadata URLs only" \
	    -variable f13Var -value URL
	pack $f13.f.b1 $f13.f.b2 -in $f13.f -side top \
	    -padx $padding -pady $padding -anchor w
	pack $f13.l -in $f13 -side top
	pack $f13.f -in $f13 -expand true -fill both
	$f13.f.b1 invoke
	
	# pack 'em all together...
	pack $f11 $f12 -in $f1.a -side left -fill y -padx $padding -pady $padding
	pack $f13 -in $f1.a -side left -expand true -fill both \
	    -padx $padding -pady $padding
	
	# now add in the two check boxes...
	frame $f1.b
	checkbutton $f1.b.cb1 -text "Fail if above options are not available" \
	    -justify left
	checkbutton $f1.b.cb2 -text "Validate tspec file on load" \
	    -justify left
	pack $f1.b.cb1 $f1.b.cb2 -in $f1.b -side top -anchor w \
	    -padx $padding -pady $padding
	
	pack $f1.a -in $f1 -expand true -fill both
	pack $f1.b -in $f1 -expand true -fill x
	
	# finally, the action area, consisting of only a dismiss button...
	set f2 .adv.f2
	frame $f2 -relief raised -borderwidth 2
	button $f2.dismiss -text "Dismiss" -command "destroy .adv"
	pack $f2.dismiss -in $f2 -side left -ipadx $padding -ipady $padding \
	    -padx $padding -pady $padding
	
	pack $f1 -expand true -fill both
	pack $f2 -side bottom -fill x
	
	wm deiconify .adv
    }


    #
    # r e g i s t e r D a t a s e t O b s e r v e r
    #
    # Public interface, where clients can register to be notified on changes 
    # in the set of opened datasets (i.e., through opens and closes).
    #
    # Returns a unique token that should be used later to de-register.
    #
    proc registerDatasetObserver {callbackString} {
	upvar DatasetDialog::observerCallbacks  observerCallbacks
	upvar DatasetDialog::observerToken      observerToken

	# make a unique token...
	incr observerToken
	set observerCallbacks($observerToken) $callbackString
	return $observerToken
    }

    #
    # n o t i f y O b s e r v e r s 
    #
    # Utility proc to invoke the callback of all the registered observers.
    #
    proc notifyObservers {} {
	upvar DatasetDialog::observerCallbacks  observerCallbacks

	if {![array exists observerCallbacks]} { return }

	set iter [array startsearch observerCallbacks]
	while {[array anymore observerCallbacks $iter]} {
	    set index [array nextelement observerCallbacks $iter]
	    eval $observerCallbacks($index)
	}
	array donesearch observerCallbacks $iter
    }

    #
    # u n r e g i s t e r D a t a s e t O b s e r v e r 
    #
    # Unregisters a previously register observer callback.
    #
    proc unregisterDatasetObserver {token} {
	upvar DatasetDialog::observerCallbacks  observerCallbacks

	if {[array names observerCallbacks $token] != ""} {
	    unset observerCallbacks($token)
	}
    }


    ##########################################
    ## Callbacks
    ##########################################


    #
    # s e l e c t G e o s e t 
    #
    # Callback for when the user "selects" an item in the "geosets" listbox.
    #
    proc selectGeoset {} {
	upvar DatasetDialog::geosetsList geosetsList
	upvar DatasetDialog::imageList imageList
	upvar DatasetDialog::demList demList
	upvar DatasetDialog::openDSButton openButton

	# if nothing selected, merely disable Open button...
	if {[llength [$geosetsList curselection]] == 0} {
	    $openButton configure -state disabled
	    $imageList delete 0 end
	    $demList delete 0 end
	    return
	}

	# if we're here, we activate the "Open Geoset" button, as its now
	# valid...
	$openButton configure -state normal

	# find out what was selected...
	set theSelection [$geosetsList get [$geosetsList curselection]]
        
        # get the datasets associated with the "geoset"...
	set datasets [tsmDatabase geoset getDatasets $theSelection]

        # place them into the image/dem lists; preselect the 1st element...
	$imageList delete 0 end
	populateListBox $imageList [lindex $datasets 0]
	$imageList selection set 0
	
	$demList delete 0 end
	populateListBox $demList [lindex $datasets 1]
	$demList selection set 0
    }

    #
    # o p e n G e o s e t 
    #
    # Callback for when a user "opens" a geoset, either by double-clicking on
    # an item in the geosets listbox, or by selecting it and then pressing the
    # "Open Geoset" button.
    #
    proc openGeoset {} {
	global gGeosetsList
	global gWinList
	upvar DatasetDialog::geosetsList geosetsListBox
	upvar DatasetDialog::imageList imageListBox
	upvar DatasetDialog::demList demListBox

	# make sure the geoset has not already been opened...
	set geoset [$geosetsListBox get [$geosetsListBox curselection]]
	if {[lsearch $gGeosetsList $geoset] != -1} {
	    tk_messageBox -title "TerraVision" \
		-message [format "Geoset \"%s\" already opened" $geoset] \
		-icon info \
		-type ok
	    return
	}

	# get the datasets selected...
	set imageDataset [$imageListBox get [$imageListBox curselection]]
	set demDataset [$demListBox get [$demListBox curselection]]

	# create a "geopyramid" out of it...
	# the dataset loading happens asyncronously in another
	# thread. Once loaded, loadedGeoset{} is called, below
	# set the splash screen for some feedback - this is only
	# seen when no datasets are currently loaded

	foreach {i} $gWinList {
	    $i.tvbase set TV_SPLASH_MESSAGE "Loading dataset..."
	}

        tsGeoPyramid create $imageDataset {} $demDataset {}

	# last (and least), remove our dialog windows...

	destroy .dsDialog
	if {[winfo exists .dsServers]} {
	    destroy .dsServers
	}
    }

    #
    # l o a d e d G e o s e t 
    #
    # Callback for when a dataset has successfully loaded.
    #
    proc loadedGeoset { geopyramid } {
	global gGeosetsList
	global gWinList

	# check for dataset load failure
	if { [ string length $geopyramid ] == 0 } {
	    foreach {i} $gWinList {
		$i.tvbase set TV_SPLASH_MESSAGE "Failed to load dataset!"
	    }
	    return;
	}

	# record that we've opened the geoset...
	lappend gGeosetsList $geopyramid

	# inform all the rendering threads of the new geoset...
	set numGeosetsOpened [llength $gGeosetsList]
	foreach {i} $gWinList {
	    global $i.status.datasets

	    $i.tvbase geoset add $geopyramid
	    set $i.status.datasets $numGeosetsOpened
	}

	# notify any observers of the change...
	notifyObservers

    }


    #
    # c l o s e D a t a s e t 
    #
    # Callback for the CloseDataset dialog;  the dataset is actually closed
    # here.  Also called if that dialog's cancel button was invoked.
    #
    proc closeDataset {action} {
	global gGeosetsList
	upvar DatasetDialog::geosetsCloseList   geosetsCloseList
	upvar DatasetDialog::closeObserverToken closeObserverToken

	if {$action == "close"} {

	    set selIndex [$geosetsCloseList curselection]
	    if {[llength $selIndex] == 0} { return }

	    set geoset [$geosetsCloseList get $selIndex]
	    tsGeoPyramid destroy $geoset

	    # remove from global list...
	    set index [lsearch -exact $gGeosetsList $geoset]
	    if {$index != -1} {
		set gGeosetsList [lreplace $gGeosetsList $index $index]
	    } else {
		tk_message -icon error -title "TerraVision" -type ok \
		    -message "Logic error: DatasetDialog::closeDataset"
	    }

	    # notify observers of the change...
	    notifyObservers
	}

	# unregister ourselves as an observer...
	unregisterDatasetObserver $closeObserverToken
	destroy .dsClose

    }

    #
    # c l o s e A l l D a t a s e t s 
    #
    proc closeAllDatasets {} {
	global gGeosetsList
	global gWinList

	# unload each dataset from all viewers and the global list
	foreach {g} $gGeosetsList {
	    tsGeoPyramid destroy $g
	}

	# do a number of per-window updates
	foreach {i} $gWinList {
	    global $i.status.datasets

	    # set the splash screen message appropriately again
	    $i.tvbase set TV_SPLASH_MESSAGE "Please Select A Terrain Model..."

	    # close all of the VRML models for this viewer
	    Features::closeAllModels $i

	    # tell all open windows that there are no datasets open
	    set $i.status.datasets 0

	    # disconnect from .geo
	    DotGeo::dotGeoDisconnect $i
	}

	# clear the global list of datasets
	set gGeosetsList [list]

	# notify any observers of the change...
	notifyObservers
    }

    #
    # p r o p s O b s e r v e r A c t i o n 
    #
    # Utility proc registered as a callback on behalf of the "Dataset Settings"
    # for dataset-change events.
    #
    proc propsObserverAction {} {
	variable infoListbox
	variable infoTextbox
	variable aggregateStr

	set geoList [tsGeoPyramid list]
	set geoList [linsert $geoList 0 $aggregateStr]

	# clear listbox, textbox, and repopulate the listbox
	$infoListbox delete 0 end
	$infoTextbox configure -state normal
	$infoTextbox delete 1.0 end
	$infoTextbox configure -state disabled
	populateListBox $infoListbox $geoList
    }


    #
    # s e l e c t S e r v e r 
    # 
    # Called when a server is selected in the "Servers" dialog.  Simply 
    # activates buttons that are inactive until a valid selection exists.
    #
    proc selectServer {} {
	upvar DatasetDialog::removeServerBtn removeServerBtn
	upvar DatasetDialog::moveServerUPBtn moveServerUPBtn
	upvar DatasetDialog::moveServerDNBtn moveServerDNBtn
	upvar DatasetDialog::readServerBtn readServerBtn
	upvar DatasetDialog::readAllServersBtn readAllServersBtn
	upvar DatasetDialog::serversList serversList
	upvar DatasetDialog::serverInfo serverInfo

	set listLen [$serversList size]
	set selIndex [$serversList curselection]
	
	if {$listLen == 0 || [llength $selIndex] == 0} {
	    # no selection -- disable inappropriate commands...
	    $removeServerBtn configure -state disabled
	    $readServerBtn configure -state disabled
	    $readAllServersBtn configure -state disabled
	    $moveServerUPBtn configure -state disabled
	    $moveServerDNBtn configure -state disabled
	    $serverInfo configure \
	    -text ""

	} else {
	    
	    $removeServerBtn configure -state normal
	    $readServerBtn configure -state normal
	    $readAllServersBtn configure -state normal

	    
	    if {$selIndex == 0} {
		$moveServerUPBtn configure -state disabled
	    } else {
		$moveServerUPBtn configure -state normal
	    }

	    if {$selIndex == [expr $listLen - 1]} {
		$moveServerDNBtn configure -state disabled
	    } else {
		$moveServerDNBtn configure -state normal
	    }

	    $serverInfo configure \
		-text [format "last contacted: %s" \
			   [tsmDatabase server info \
				[$serversList get $selIndex]]]
	
	}
    }

    #
    # r e a d S e r v e r S t a t u s
    #
    proc readServerStatus { msg } {
	variable readServerLabel

	set win .dsReadServerMsg

	# if window already exists just update the message
	if {[winfo exists $win]} {
	    $readServerLabel configure -text $msg
	    raise $win
	    focus $win
	} else {
	    toplevel $win -class Dialog
	    wm title $win "Reading Server..."

	    frame $win.f1 -relief raised -borderwidth 2
	    set readServerLabel $win.f1.l
	    label $readServerLabel -text $msg -justify left
	    pack $readServerLabel -side top -anchor w -expand 1 -fill both -padx 5 -pady 3
	    pack $win.f1 -expand 1 -fill both -side top
	    focus $win
	    grab $win
	}

	update
    }

    #
    # c l o s e S e r v e r S t a t u s
    #
    proc closeServerStatus {} {
	set win .dsReadServerMsg

	# if window already exists, close it
	if {[winfo exists $win]} {
	    destroy $win
	}
    }

    #
    # a d d S e r v e r C B
    # 
    # Dialog to add a new server to the current Database...
    #
    proc addServerCB {} {

	# if window already exists, try to force it to the top...
	if {[winfo exists .dsAddServer]} {
	    wm withdraw .dsAddServer
	    wm deiconify .dsAddServer
	    return 
	}

	# create a dialogBox to gather the server URL...
	toplevel .dsAddServer -class Dialog
	wm withdraw .dsAddServer
	wm title .dsAddServer "Add New Server"

	set d .dsAddServer
	frame $d.f1 -relief raised -borderwidth 2
	
	label $d.f1.l -text "Enter URL to new server:" -justify right
	entry $d.f1.e -width 40 
	label $d.f1.i -text "e.g.,\n  C:/TileSets/Pyramids (local disk)\n  http://www.tvgeo.com/TileSets/Pyramids (http)\n  http://www.tvgeo.com/wms? (OGC WMS)" -justify left
	pack $d.f1.l -side top -anchor w -expand 1 -fill both
	pack $d.f1.e -side top -expand 1 -fill x
	pack $d.f1.i -side top -expand 1 -fill x

	set bindscript "eval \{ DatasetDialog::addServer $d.f1.e \}"
	bind $d.f1.e <Return> $bindscript

	frame $d.f2 -relief raised -borderwidth 2
	button $d.f2.ok -text "OK" \
	    -command "DatasetDialog::addServer $d.f1.e"
	button $d.f2.cancel -text "cancel" \
	    -command "destroy .dsAddServer"
	grid columnconfigure $d.f2 {0 4}  -weight 1 -weight 1 -weight 1 \
	    -weight 1 -weight 1
	grid x $d.f2.ok x $d.f2.cancel x -sticky ew

	pack $d.f1.e -side top -expand 1 -fill x -padx 20
	pack $d.f1 -side top -expand 1 -fill both -ipady 10
	pack $d.f2 -side bottom -expand 1 -fill x -ipady 4

	focus $d.f1.e

	wm deiconify .dsAddServer

	tkwait window .dsAddServer
    }

    #
    # a d d S e r v e r 
    #
    # Called to perform the actual work of adding a new server to the 
    # database.
    #
    proc addServer {entryWidget} {
	global theDatabase
	upvar DatasetDialog::serversList serversList 

	set url [$entryWidget get]
	if {[catch {tsmDatabase server add $url} result]} {
	    tk_messageBox -title "TerraVision" -icon error \
		          -message $result -type ok
	    return
	} else {
	    set url $result
	}

	destroy .dsAddServer

	# insert new server in listbox...
	$serversList selection clear 0 end
	$serversList insert end $url
	$serversList selection set end
	DatasetDialog::selectServer

	if {[tk_messageBox -icon question -message "Read server now?" \
		 -type yesno -title "TerraVision Dialog"] == "yes"} {
	    DatasetDialog::readServerCB SELECTED
	} else {
	    # Update the database persistent store;  readServerCB does this
	    # for us...
	    tsmDatabase write
	}

    }

    #
    # r e m o v e S e r v e r C B 
    #
    # Called to remove a selected server from the current Database...
    #
    proc removeServerCB {} {
	upvar DatasetDialog::serversList serversList
	upvar DatasetDialog::geosetsList geosetsList
	
	set serverIndex [$serversList curselection]
	if {[tk_messageBox -title "TerraVision Dialog" \
		 -message "Really remove this server from the database?" \
		 -icon  question -type yesno] 
	    == "no"} {
	    return
	}

	tsmDatabase server remove [$serversList get $serverIndex]
	tsmDatabase write

	# clear the listbox, reload it, and reset the selection...
	$serversList delete $serverIndex
	DatasetDialog::selectServer

	# reload the list of geosets, reset the selection...
	$geosetsList delete 0 end
	populateListBox $geosetsList [tsmDatabase geoset list]
	DatasetDialog::selectGeoset
    }

    #
    # m o v e S e r v e r C B 
    #
    # Called to move a selected server up/down in the list (thus affecting
    # read priority).
    #
    proc moveServerCB {dir} {
	upvar DatasetDialog::serversList serversList

	set serverIndex [$serversList curselection]
	if {$dir == "UP"} {set dir up}
	if {$dir == "DOWN"} {set dir down}

	# move the selected server in the direction indicated...
	tsmDatabase server $dir [$serversList get $serverIndex]

	# clear the listbox, reload it, and reset the selection...
	$serversList delete 0 end
	populateListBox $serversList [tsmDatabase server list]

	if {$dir == "up"} {
	    set newIndex [expr $serverIndex - 1]
	} else {
	    set newIndex [expr $serverIndex + 1]
	}

	$serversList selection set $newIndex
	DatasetDialog::selectServer
    }

    #
    # r e a d S e r v e r C B
    #
    # Called to re-read either the selected server, or all servers of the
    # Database.
    #
    # Argument is expected to be either "SELECTED" or "ALL".
    #
    proc readServerCB {allOrOne} {
	upvar DatasetDialog::serversList serversList
	upvar DatasetDialog::geosetsList geosetsList

	if {$allOrOne == "SELECTED"} {
	    set server [$serversList get [$serversList curselection]]
	    readServerStatus "Reading datasets on $server... Please wait."
	    tsmDatabase server read $server
	} else {
	    set servers [$serversList get 0 end]
	    foreach server $servers {
		readServerStatus "Reading datasets on $server... Please wait."
		tsmDatabase server read $server
	    }
	}

	# close the "Reading Server" progress dialog
	closeServerStatus

	# update the database persistent store...
	tsmDatabase write

	# reload the list of geosets, reset the selection...
	$geosetsList delete 0 end
	populateListBox $geosetsList [tsmDatabase geoset list]
	DatasetDialog::selectGeoset
    }

    #
    # c h a n g e D a t a b a s e
    #
    # Called when user enters a different string in the "database" entry
    # widget.  If the given database exists, it is read.  If not, the user
    # is prompted to create it and enter servers for it.
    #
    proc changeDatabase {} {
	global gTheDatabase
	upvar DatasetDialog::databaseLabelField databaseField
	upvar DatasetDialog::geosetsList        geosetsList
	upvar DatasetDialog::imageList          imageList
	upvar DatasetDialog::demList            demList
	upvar DatasetDialog::serversList        serversList

	set database [$databaseField get]
	if {[string equal $database $gTheDatabase]} { return } ;# do nothing

	# clear out any GUI components which might contain residual data...
	$geosetsList delete 0 end
	$imageList delete 0 end
	$demList delete 0 end
	if {[winfo exists .dsServers]} {
	    $serversList delete 0 end
	}

	#
	# new database?
	#
	if { ![file exists $database] } {
	    set result [tk_messageBox -icon question -type yesno \
		            -title "TerraVision Dialog" \
			    -message "Database file doesn't exist -- create a new database?"]
	    if {$result == "no"} { return }
	    
	    if { [catch {tsmDatabase new $database} result] } {
		tk_messageBox -icon error -type ok -message $result
		return
	    }

	    set gTheDatabase $database

	    # post the servers dialog, as this will need to be user's first 
	    # step...
	    postServers
	    populateListBox $serversList [tsmDatabase server list] 
	    selectServer
	    
	    return
	}

	#
	# existing database...
	#
	if { [catch {tsmDatabase open $database} result] } {
	    tk_messageBox -icon error -message $result -type ok
	    return
	}

	set gTheDatabase $database

	populateListBox $geosetsList [tsmDatabase geoset list]
	if {[winfo exists .dsServers]} {
	    populateListBox $serversList [tsmDatabase server list]
	}

	#
	# Store the new database name in the user's config file
	#

	tvutils set TVU_DATABASE_URL $database
	tvutils config add1 database $database
	tvutils config save
    }

    #
    # i n f o C B 
    #
    # Callback invoked when an item is selected in the "Properties" dialog.
    # Here, we display detailed information about the selected geoset.
    #
    #
    proc infoCB {} {
	upvar DatasetDialog::infoListbox infoListbox
	upvar DatasetDialog::infoTextbox infoTextbox
	upvar DatasetDialog::aggregateStr aggregateStr

	if {[llength [$infoListbox curselection]] == 0} {
	    return
	}

	set geoset [$infoListbox get [$infoListbox curselection]]
	if {[string compare $geoset $aggregateStr] == 0} {
	    set msg [tsGeoPyramid info]
	} else {
	    set msg [tsGeoPyramid info $geoset]
	}

	$infoTextbox configure -state normal
	$infoTextbox delete 1.0 end	
	$infoTextbox insert end $msg
	$infoTextbox configure -state disabled
    }

    #
    # c l e a r C o n s o l e
    #
    proc clearConsole {} {
	variable consoleTextbox
	$consoleTextbox delete 1.0 end
    }

    #
    # a d d T o C o n s o l e
    #
    proc addToConsole { text } {
	variable consoleTextbox
        DatasetDialog::createConsole
	$consoleTextbox insert end $text
	$consoleTextbox insert end "\n"

	# prune the first line if the text gets too big
        if { [string length [$consoleTextbox get 1.0 end]] > 10240 } {
	    $consoleTextbox delete 1.0 "1.0 lineend + 1 char"
	}
    }

    #
    # c r e a t e  C o n s o l e
    #
    proc createConsole {} {
	variable consoleTextbox

	# if window already exists, try to force it to the top...
	set winName .consoleWin
	if {[winfo exists $winName]} {
	    return
	}

	toplevel $winName -class Dialog
	wm withdraw $winName
	wm title $winName "Dataset Console"
	
	set top $winName
	frame $top.f0 -borderwidth 2 -relief raised
	set top $top.f0 

	frame $top.f1	

	frame $top.f2 -borderwidth 3
	set consoleTextbox $top.f2.tb
	text $consoleTextbox -width 70 -height 22 -relief groove -wrap none \
           -background black -foreground green -state normal \
	   -xscrollcommand "$top.f2.hsb set" -yscrollcommand "$top.f2.vsb set"
	scrollbar $top.f2.vsb -orient vertical \
           -command "$consoleTextbox yview"
	scrollbar $top.f2.hsb -orient horizontal \
           -command "$consoleTextbox xview"

	pack $top.f2.hsb -side bottom -expand 1 -fill x 
	pack $consoleTextbox -side left -expand 1 -fill both
	pack $top.f2.vsb -side left -expand 1 -fill y

	pack $top.f1 $top.f2 -side left -expand 1 -fill both
	pack $top -side top -expand 1 -fill both

	# action area...
	button $winName.dismiss -text "Dismiss" ;# -command set below...
	button $winName.clear -text "Clear Console" ;# -command set below...
	pack $winName.dismiss -side left -expand 1 -fill x
	pack $winName.clear -side left -expand 1 -fill x

	$winName.clear configure -command "DatasetDialog::clearConsole"
	$winName.dismiss configure -command "wm withdraw .consoleWin"

        DatasetDialog::addToConsole "TerraVision Console v1.0"
        DatasetDialog::addToConsole "Progress for dataset loading is reported here\n"
    }

    #
    # p o s t C o n s o l e
    #
    proc postConsole {} {
	variable consoleTextbox

        DatasetDialog::createConsole

	set winName .consoleWin
	wm withdraw $winName
	wm deiconify $winName
    }

}
