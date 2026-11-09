#
# TerraVision.tcl
#

# dismiss the regular "toplevel" window created by Tk; we'll create our own
wm withdraw .

# we don't like the default messageBox properties, so we'll override 'em...
option add *Dialog.msg.font {helvetica 12 bold}
option add *Dialog.msg.wrapLength 6i

# drag in the supporting modules;  this should probably be done as a 
# "package", but for now this scheme simple to deal with.
source "util.tcl"
source "paths.tcl"
source "links.tcl"
source "mainwin.tcl"
source "datasetDialogs.tcl"
source "renderoptions.tcl"
source "navoptions.tcl"
source "viewoptions.tcl"
source "datasetoptions.tcl"
source "viewbookmarks.tcl"
source "debugoptions.tcl"
source "annotateoptions.tcl"
source "featureoptions.tcl"
source "dotgeo.tcl"
source "oaa.tcl"

#
# Global variables...
#   Note our convention is that globals have the lower-case "g" prefix.

# we keep a list of all the opened toplevel 3D/map views...
set gWinList [list]

# keep a list of all open geosets...
set gGeosetsList [list]

# vars used to generate unique instance IDs
set gInstBase .tvinst
set gInstNum  0
set gInstID   ""

# used to keep track of state of mouse/button events...
set gMouseDownX 0
set gMouseDownY 0
set gMouseDown 0
set gShiftDown 0 
set gCtrlDown 0 
set gAltDown 0
set gMoving 0

# deltas used in keyboard-based view "tweaking" commands...
set gSlideDelta  50.0
set gZoomDelta   50.0
set gPitchDelta  [expr 2.0 * $PI / 180.0]
set gYawDelta    [expr 2.0 * $PI / 180.0]


# arrays used to keep track of state not maintained by instances of tvBase,
# rather by the GUI, such as flight modes, etc.  Entries are created/deleted
# when Viewers are created/deleted; we'll create a couple of dummy entries
# here simply to document their existance as globals...

set gFlightMode(0) ""
set gFlightControlMode(0) ""
set gViewMode(0) ""
set gShowMenu(0) ""
set gShowLogo(0) ""
set gShowStatus(0) ""
set gDoodleMode(0) ""


#########################################################
# mainline...
#########################################################


# instance our first toplevel window...
if {[tvutils get TVU_INITIAL_VIEWER] == 3} {
    newViewWindow 3D
} else {
    newViewWindow 2D
}

#
# Find out the default location of various resources, such as the
# database and viewpoints file, by using the get function with the
# TVU_DATABASE_URL, etc. keys (see <tvutils/tvutils.h>).
#

set gInstallDirectory [tvutils get TVU_INSTALL_DIR]
set gPathsDirectory [tvutils get TVU_PATHS_DIR]
set gBookmarksFile [tvutils get TVU_VIEWPOINT_URL]
set gTheDatabase [tvutils get TVU_DATABASE_URL]

if {[file exists $gTheDatabase]} {
    tsmDatabase open $gTheDatabase
}

#
# If this is the first time that this version of TerraVision
# has been used, then pop up the license agreement.
#

set show_license     [tvutils get TVU_SHOW_LICENSE]
set license_accepted [tvutils get TVU_LICENSE_ACCEPTED]
set curr_version     [tvutils get TVU_TV_VERSION]
set curr_accepted    [string compare $license_accepted $curr_version]

# don't display license any more now that TV is Open Source
#if { $show_license || $curr_accepted < 0 } {
#   tvLicense
#}

#
# Implement the -oi and -dem command line options
#

set numsets [tvutils get TVU_NUM_DATASETS]
set i 0
while { $i < $numsets } {
    set thisoi  [tvutils get TVU_OI_URL $i]
    set thisdem [tvutils get TVU_DEM_URL $i]
    tsGeoPyramid create $thisoi "" $thisdem ""
    incr i
}

# This is here as a milepost for testing purposes; lets us know we didn't die
# somewhere along the way.
# puts  "Are we still alive?"

