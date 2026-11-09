#!/bin/perl
#
# make_html.pl
#
# This Perl script will automatically generate the API documentation
# from the source code in the tsm directory, using the doc++ utility.
# This script fixes some problems with doc++ and prettifies the HTML
# output a bit.
#
# $Id: make_html.pl,v 1.3 2000/06/09 19:08:52 brownrig Exp $
#
# NOTE: this script originated with Martin Reddy, who employed it for 
# generating API docs for tsm.  Its been modified for use here in the 
# TerraVision library API.   --RLB.

# don't let the user run this in the tsm source directory as it will
# really clutter things up, suggest doing a "md doc;cd doc;../make_html.pl"

open( SRCTEST, "ThreeDCore.h" ) && die "Do not run this in the tsm source dir\n";

# get on with it...

$srcdir = "../../../TerraVision/";
$banner = "banner.html";

print "Making banner...\n";

open( BANNER, ">$banner" ) || die "Couldn't create $banner\n";

# create a banner file for the bottom of each doc page

print BANNER "<!-- sribanner -->\n";
print BANNER "<IMG SRC=\"../img/back.gif\" ALT=\"Back to\" HEIGHT=21 WIDTH=20>\n";
print BANNER "<A HREF=\"../api.html\" TARGET=\"_top\">TerraVision API Home Page</A>";
print BANNER "\n";

close( BANNER );

print "Generating documentation...\n";

$| = 1; # do flushes after every print
foreach $source ( `ls $srcdir/*.[chx]` ) {
    chop $source; $_ = $source;

    # doc++ can't handle the % comments in xdr (.x) files

    if ( /\.x/ ) {
	$tmpfile = "xdr.tmp";
	system( "grep -v '^\%' $source > $tmpfile" );
	$source = $tmpfile;
    } 

    system( "doc++ -H -B $banner $source 2> /dev/null > /dev/null" );
    print ".";
}
$| = 0; # back to buffered mode
print "\n";

system( "rm -f xdr.tmp > /dev/null 2> /dev/null" );
system( "rm -f $banner" );

print "Fixing HTML...\n";

foreach $source ( `ls *.html` ) {
    chop $source;
    $tmpfile = $source . ".tmp";

    print "$source\n";

    open( INFILE, "$source" ) || die "Couldn't open $source\n";
    open( OUTFILE, ">$tmpfile" ) || die "Couldn't open $tmpfile\n";

    $see_also = 0;
    
    while ( <INFILE> ) {

	# let's have a white background

	s/\<body\>/\<body bgcolor=\"\#ffffff\" text=\"\#000000\"\>/;

	# remove the alphabetic link at the bottom of the entry

	s/\<A.*alphabetic.*A\>//;

	# make solid dividing lines

	s/\<[hH][Rr]\>/\<hr noshade size=2\>/g;

	# have we reached the See Also section yet? 

	$see_also = 1 if ( /See Also/ );
	$see_also = 0 if ( /sribanner/ );

	if ( $see_also ) {

	    # fix all of the See Also links that didn't get hyperlinked

	    s/([\ |\>])([A-Z|a-z|0-9|_]+)([\,|\.|\n])/$1\<a href=\"$2.html\"\>$2\<\/a\>$3/g;
	    s/^([A-Z|a-z|0-9|_]+)([\,|\.|\n])/\<a href=\"$1.html\"\>$1\<\/a\>$2/;

	    # what are those <!1> tags for??

	    s/\<\!1\>//g;

	}
	# remove doram c++ link at bottom of page

	$_ = "" if ( /www\.zib\.de/ );

	print OUTFILE $_;
    }

    close( OUTFILE );
    close( INFILE );

    system( "mv $tmpfile $source" );

}
