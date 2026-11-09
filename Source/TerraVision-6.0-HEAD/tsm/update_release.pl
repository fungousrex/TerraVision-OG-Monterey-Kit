#!/usr/local/bin/perl

# ================================================================
# update_release -major | -minor
#
# Increment the major or minor release numbers in files MAJOR and/or
# MINOR, then use 'cvs tag' to specify a release tag for all files.
# ================================================================

# Print the usage message if necessary.

if (($#ARGV < 0) || ($ARGV[0] =~ "-h")) {
    print "Usage: update_release.pl -major | minor\n";
    exit 0;
}

# Read in the current major and minor release numbers.

if (open (MAJOR, "MAJOR")) {
    chop($major = <MAJOR>);
    close MAJOR;
}
else {
    $major = 0;
}

if (open (MINOR, "MINOR")) {
    chop($minor = <MINOR>);
    close MINOR;
}
else {
    $minor = 0;
}

# Increment them.

if ($ARGV[0] eq "-major") {
    $major++;
    $minor = 0;
    
}
elsif ($ARGV[0] eq "-minor") {
    $minor++;
}
else {
    print "Error, you must specify either -major or -minor.\n";
    exit;
}

# Let the user know what the latest release is:

print "New Release = '$major.$minor'\n";

# Write out the new release numbers.

open (MAJOR, ">MAJOR") 	|| die "Couldn't open MAJOR for writing.\n";
print MAJOR "$major\n";
close MAJOR;
open (MINOR, ">MINOR") 	|| die "Couldn't open MINOR for writing.\n";
print MINOR "$minor\n";
close MINOR;

# Update the RELEASE include file:

# $null is used to fool cvs into thinking this is the end of the rcs tag,
# otherwise it will strip the '\' escaping the final '$'. We don't need
# to use $null for the Release tag because the $release var does this for us 

$null    = "";      
$release = "$major.$minor";

open (RELEASE, ">RELEASE");
print RELEASE
    "#define TSMAPI_RELEASE $release\n",
    "static char tsmApiRelease[] = \"\$Release: $release \$\";\n",
    "static char tsmApiReleaseDate[] = \"\$Date: 1997/12/04 07:03:58 $null\$\";\n";
close RELEASE;

# Use cvs to update the tag.

$command = "cvs commit";
system ($command)
    && die "Couldn't execute system command '$command'\n";

$command = "cvs tag release-$major-$minor";
system ($command)
    && die "Couldn't execute system command '$command'\n";

exit 0;

