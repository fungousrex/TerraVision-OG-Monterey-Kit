dnl aclocal.m4 generated automatically by aclocal 1.4-p6

dnl Copyright (C) 1994, 1995-8, 1999, 2001 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY, to the extent permitted by law; without
dnl even the implied warranty of MERCHANTABILITY or FITNESS FOR A
dnl PARTICULAR PURPOSE.

dnl Make sure that $prefix is used for configuration.
dnl
dnl Usage: TV_CONFIG_WITH_PREFIX

AC_DEFUN(TV_CONFIG_WITH_PREFIX,
[# TV_CONFIG_WITH_PREFIX
# Make sure that we're searching the $prefix lib and include directories.
if test $prefix != NONE -a $prefix != /usr; then
  if test $exec_prefix = NONE; then
    exec_prefix=$prefix
  fi
  if test -d "$exec_prefix/lib"; then
    LDFLAGS="-L$exec_prefix/lib $LDFLAGS"
  fi
  if test -d "$prefix/include"; then
    if test -d $prefix/include/db; then
      CPPFLAGS="-I$prefix/include/db $CPPFLAGS"
    fi
    CPPFLAGS="-I$prefix/include $CPPFLAGS"
  fi
fi
])

dnl Configure appropriately for -ltiff ...
dnl
dnl Usage: TV_LIBTIFF(maybe_dir)

AC_DEFUN(TV_LIBTIFF,
[# TV_LIBTIFF($1,$2)
LIBS="$LIBS -lm"

if test "$2" = "check"; then
  # First check for jpeg, and only worry about installed tiff if jpeg found.
  AC_CHECK_HEADER(jpeglib.h,, TIFFDIR=[$1])
  AC_CHECK_LIB(jpeg, jpeg_create_compress, ,
    AC_CHECK_LIB(jpeg, jpeg_CreateCompress, TIFFDIR=[$1]))

  # Look for installed TIFF library.
  if test -z "$TIFFDIR"; then
    AC_CHECK_HEADER(tiff.h,, TIFFDIR=[$1])
    AC_CHECK_LIB(tiff, TIFFOpen,,
      AC_CHECK_LIB(tiff, TIFFOpen,, TIFFDIR=[$1], -ljpeg -lz -lCsup),
      -ljpeg -lz)
  fi
else
  TIFFDIR=[$1]
fi

# Or check for tiff as a sibling directory.
if test -n "$TIFFDIR"; then
  tiffsrcdir=`(cd $srcdir; cd $TIFFDIR; pwd)`
  tiffdir=`(cd $TIFFDIR; pwd)`
  if test ! -d $tiffdir -a -d $tiffsrcdir; then
    $mkinstalldirs $tiffdir
  fi
  CPPFLAGS="-I$tiffsrcdir/libtiff $CPPFLAGS"
  LDFLAGS="-L$tiffdir/libtiff -L$tiffdir/../zlib -L$tiffdir/../jpeg -L../libgeotiff $LDFLAGS"
  LIBS="-lgeotiff -ltiff -ljpeg -lz $LIBS"
fi
])

dnl Determine library name and linking parameters for tsmApi library.
dnl
dnl Usage: TV_TSMCONFIG

AC_DEFUN(TV_TSMCONFIG,
[AC_ARG_WITH(dpss, 
[  --with-dpss             Search for DPSS includes and libraries.],
    [
	dpss=$withval
	if test -n "$withval" -a -d "$withval"; then
	  LDFLAGS="-L$withval/lib $LDFLAGS"
	  CPPFLAGS="-I$withval/include -I$withval/include/db $CPPFLAGS"
	fi
    ],[
	AC_CHECK_HEADER(iss/iss.h, [dpss=yes], [dpss=no])
    ])

TSMNAME=tsmApi
if test "$dpss" != no; then
        TSMNAME=tsmApi-DPSS
	AC_DEFINE(DPSS)
dnl	AC_CHECK_LIB(threadswrap, u_mutex_lock,
dnl	  LIBS="-lthreadswrap $LIBS")
dnl	if test "$ac_cv_lib_threadswrap_u_mutex_lock" != yes; then
dnl	  AC_CHECK_LIB(threadswrap_nothreads, u_mutex_lock,
dnl	    LIBS="-lthreadswrap_nothreads $LIBS")
dnl	fi
dnl	AC_CHECK_LIB(spkm, SPKM_wrap,
dnl	  LIBS="-lspkm -lssh -lx509cap -ltdatastr -lcrypto -lgmp $LIBS",,
dnl	  -lssh -lx509cap -ltdatastr -lcrypto -lgmp)
	old_LIBS="$LIBS"
	LIBS="-lpthread $LIBS"
	AC_CHECK_FUNC(pthread_mutex_lock, old_LIBS="$LIBS")
	LIBS="-ldpss -ldpssutils -ljnetutils -ldimage -ldb $old_LIBS"
fi

dnl Maybe include -lnsl if xdr_int is not in libc.
AC_CHECK_FUNC(xdr_int,, AC_CHECK_LIB(nsl, xdr_int))

TSMLIB=lib$TSMNAME.a

AC_SUBST(TSMNAME)
AC_SUBST(TSMLIB)
])

dnl Work out if we need to define fixed-size types
dnl
dnl Usage: SRI_INT8_T, SRI_INT16_T, SRI_INT32_T, SRI_INT64_T

AC_DEFUN(SRI_INT8_T,[
AC_CHECK_TYPE(int8_t, signed char)
AC_CHECK_TYPE(u_int8_t, unsigned char)
])

AC_DEFUN(SRI_INT16_T,[
AC_CHECK_SIZEOF(short, 0)
AC_CHECK_SIZEOF(int, 0)

if test $ac_cv_sizeof_short -eq 2; then
  AC_CHECK_TYPE(int16_t, short)
  AC_CHECK_TYPE(u_int16_t, unsigned short)
elif test $ac_cv_sizeof_int -eq 2; then
  AC_CHECK_TYPE(int16_t, int)
  AC_CHECK_TYPE(u_int16_t, unsigned in)
else
  echo ERROR: No 16 bit integer type.
  exit 1
fi

])

AC_DEFUN(SRI_INT32_T,[
AC_CHECK_SIZEOF(short, 0)
AC_CHECK_SIZEOF(int, 0)
AC_CHECK_SIZEOF(long, 0)

if test $ac_cv_sizeof_short -eq 4; then
  AC_CHECK_TYPE(int32_t, short)
  AC_CHECK_TYPE(u_int32_t, unsigned short)
elif test $ac_cv_sizeof_int -eq 4; then
  AC_CHECK_TYPE(int32_t, int)
  AC_CHECK_TYPE(u_int32_t, unsigned int)
elif test $ac_cv_sizeof_long -eq 4; then
  AC_CHECK_TYPE(int32_t, long)
  AC_CHECK_TYPE(u_int32_t, unsigned long)
else
  echo ERROR: No 32 bit integer type.
  exit 1
fi

])

AC_DEFUN(SRI_INT64_T,[
AC_CHECK_SIZEOF(int, 0)
AC_CHECK_SIZEOF(long, 0)
AC_CHECK_SIZEOF(long long, 0)
AC_CHECK_SIZEOF(__int64, 0)

if test $ac_cv_sizeof_int -eq 8; then
  AC_CHECK_TYPE(int64_t, int)
  AC_CHECK_TYPE(u_int64_t, unsigned int)
elif test $ac_cv_sizeof_long -eq 8; then
  AC_CHECK_TYPE(int64_t, long)
  AC_CHECK_TYPE(u_int64_t, unsigned long)
elif test $ac_cv_sizeof_long_long -eq 8; then
  AC_CHECK_TYPE(int64_t, long long)
  AC_CHECK_TYPE(u_int64_t, unsigned long long)
elif test $ac_cv_sizeof___int64 -eq 8; then
  AC_CHECK_TYPE(int64_t, __int64)
  AC_CHECK_TYPE(u_int64_t, unsigned __int64)
else
  echo ERROR: No 64 bit integer type.
  exit 1
fi

])

dnl Use NSPR libraries.
dnl
AC_DEFUN(TV_NSPR,[
# TV_NSPR($1)
. $1/config/nsprincl.sh

nsprdir=$1/dist
CPPFLAGS="-I$nsprdir/include $CPPFLAGS"
LDFLAGS="-L$nsprdir/lib $LDFLAGS"
NSPR_LIBS="$NSPR_LIB $NSPR_EXTRA_LIBS"
LIBS="$NSPR_LIBS $LIBS"

AC_SUBST(NSPR_LIBS)
])

dnl Use Tcl/Tk libraries
dnl
dnl Usage: TV_TCL

AC_DEFUN(TV_TCL_FIND,
[
TCL_FOUND="no"

AC_ARG_WITH(tcl-lib,
[  --with-tcl-lib[=DIR]    Search for Tcl libraries.],[
  if test -d $withval -a -r $withval/tclConfig.sh; then
    . $withval/tclConfig.sh
    TCL_FOUND="yes"
    LDFLAGS="$TCL_LIB_SPEC $LDFLAGS"
    CPPFLAGS="-I$TCL_INC_DIR $CPPFLAGS "
  else
    echo Argument --with-tcl-lib=$withval must point to directory.
  fi
])

AC_ARG_WITH(tk-lib,
[  --with-tk-lib[=DIR]     Search for Tk libraries.],[
  if test -d $withval -a -r $withval/tkConfig.sh; then
    . $withval/tkConfig.sh
    TCL_FOUND="yes"
    LDFLAGS="$TK_LIB_SPEC $LDFLAGS"
    CPPFLAGS="-I$TK_INC_DIR $CPPFLAGS "
  else
    echo Argument --with-tk-lib=$withval must point to directory.
  fi
])

AC_ARG_WITH(tcl,
[  --with-tcl[=DIR]        Search for Tcl includes and libraries.],[
  if test -d $withval; then
      if test -r $withval/lib/tkConfig.sh; then
        . $withval/lib/tkConfig.sh
        . $withval/lib/tclConfig.sh
      else
        if test -z "$TCL_LIB_FLAG"; then
          TCL_LIB_FLAG="-ltk83 -ltcl83"
        fi
      fi
      TCL_FOUND="yes"
      LDFLAGS="-L$withval/lib $LDFLAGS"
      CPPFLAGS="-I$withval/include $CPPFLAGS "
  else
    echo Argument --with-tcl=$withval must point to directory.
  fi
],[
  for i in /usr/lib /usr/local/lib; do
    if test -z "$TCL_LIB_FLAG" -a -f $i/tclConfig.sh; then
      . $i/tkConfig.sh
      . $i/tclConfig.sh
      TCL_FOUND="yes"
      if test "$TK_PREFIX" != "/usr"; then
        LDFLAGS="-L$TK_PREFIX/lib $LDFLAGS"
        CPPFLAGS="-I$TK_PREFIX/include $CPPFLAGS "
      fi
    fi
  done
])
])

AC_DEFUN(TV_TCL,
[# TV_TCL($1)
TV_TCL_FIND

if test "$TCL_FOUND" != "yes"; then
  if test -r $1/tcl/unix; then
    . $1/tcl/unix/tclConfig.sh
  fi

  if test -r $1/tk/unix; then
    . $1/tk/unix/tkConfig.sh
  fi

  if test -z "$TCL_LIB_FLAG"; then
    TCL_LIB_FLAG="-ltk83 -ltcl83"
  fi

  TCL_FOUND="yes"
  LDFLAGS="-L$1/tcl/unix -L$1/tk/unix $LDFLAGS"
  CPPFLAGS="-I$srcdir/$1/tcl/generic -I$srcdir/$1/tk/generic $CPPFLAGS "
fi

if test -z "$TCL_LIB_FLAG"; then
  echo Cannot find tcl distribution. Must provide --with-tcl argument.
  exit 1
fi

dnl This fails under Windows even though the header exists!?
AC_CHECK_HEADERS(tk.h)
dnl AM_CONDITIONAL(HAVE_TCL, test "$ac_cv_header_tk_h" = yes)
AM_CONDITIONAL(HAVE_TCL, test "$TCL_FOUND" = yes)

])

AC_DEFUN(TV_TCL_CONFIG,
[
TV_TCL_FIND

if test "$TCL_FOUND" != "yes"; then
  if test ! -d tcl/unix; then
    mkdir -p tcl
  fi

  if test ! -d tk/unix; then
    mkdir -p tk
  fi

  TCL_SUBDIRS="tcl/unix tk/unix"
  TCL_FOUND="yes"
fi

dnl This fails under Windows even though the header exists!?
AC_CHECK_HEADERS(tk.h)
# This is unconditional, for now.
AM_CONDITIONAL(HAVE_TCL, true)
])

# Define a conditional.

AC_DEFUN([AM_CONDITIONAL],
[AC_SUBST($1_TRUE)
AC_SUBST($1_FALSE)
if $2; then
  $1_TRUE=
  $1_FALSE='#'
else
  $1_TRUE='#'
  $1_FALSE=
fi])

# Do all the work for Automake.  This macro actually does too much --
# some checks are only needed if your package does certain things.
# But this isn't really a big deal.

# serial 1

dnl Usage:
dnl AM_INIT_AUTOMAKE(package,version, [no-define])

AC_DEFUN([AM_INIT_AUTOMAKE],
[AC_REQUIRE([AM_SET_CURRENT_AUTOMAKE_VERSION])dnl
AC_REQUIRE([AC_PROG_INSTALL])
PACKAGE=[$1]
AC_SUBST(PACKAGE)
VERSION=[$2]
AC_SUBST(VERSION)
dnl test to see if srcdir already configured
if test "`cd $srcdir && pwd`" != "`pwd`" && test -f $srcdir/config.status; then
  AC_MSG_ERROR([source directory already configured; run "make distclean" there first])
fi
ifelse([$3],,
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE", [Name of package])
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", [Version number of package]))
AC_REQUIRE([AM_SANITY_CHECK])
AC_REQUIRE([AC_ARG_PROGRAM])
dnl FIXME This is truly gross.
missing_dir=`cd $ac_aux_dir && pwd`
AM_MISSING_PROG(ACLOCAL, aclocal-${am__api_version}, $missing_dir)
AM_MISSING_PROG(AUTOCONF, autoconf, $missing_dir)
AM_MISSING_PROG(AUTOMAKE, automake-${am__api_version}, $missing_dir)
AM_MISSING_PROG(AUTOHEADER, autoheader, $missing_dir)
AM_MISSING_PROG(MAKEINFO, makeinfo, $missing_dir)
AC_REQUIRE([AC_PROG_MAKE_SET])])

# Copyright 2002  Free Software Foundation, Inc.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA

# AM_AUTOMAKE_VERSION(VERSION)
# ----------------------------
# Automake X.Y traces this macro to ensure aclocal.m4 has been
# generated from the m4 files accompanying Automake X.Y.
AC_DEFUN([AM_AUTOMAKE_VERSION],[am__api_version="1.4"])

# AM_SET_CURRENT_AUTOMAKE_VERSION
# -------------------------------
# Call AM_AUTOMAKE_VERSION so it can be traced.
# This function is AC_REQUIREd by AC_INIT_AUTOMAKE.
AC_DEFUN([AM_SET_CURRENT_AUTOMAKE_VERSION],
	 [AM_AUTOMAKE_VERSION([1.4-p6])])

#
# Check to make sure that the build environment is sane.
#

AC_DEFUN([AM_SANITY_CHECK],
[AC_MSG_CHECKING([whether build environment is sane])
# Just in case
sleep 1
echo timestamp > conftestfile
# Do `set' in a subshell so we don't clobber the current shell's
# arguments.  Must try -L first in case configure is actually a
# symlink; some systems play weird games with the mod time of symlinks
# (eg FreeBSD returns the mod time of the symlink's containing
# directory).
if (
   set X `ls -Lt $srcdir/configure conftestfile 2> /dev/null`
   if test "[$]*" = "X"; then
      # -L didn't work.
      set X `ls -t $srcdir/configure conftestfile`
   fi
   if test "[$]*" != "X $srcdir/configure conftestfile" \
      && test "[$]*" != "X conftestfile $srcdir/configure"; then

      # If neither matched, then we have a broken ls.  This can happen
      # if, for instance, CONFIG_SHELL is bash and it inherits a
      # broken ls alias from the environment.  This has actually
      # happened.  Such a system could not be considered "sane".
      AC_MSG_ERROR([ls -t appears to fail.  Make sure there is not a broken
alias in your environment])
   fi

   test "[$]2" = conftestfile
   )
then
   # Ok.
   :
else
   AC_MSG_ERROR([newly created file is older than distributed files!
Check your system clock])
fi
rm -f conftest*
AC_MSG_RESULT(yes)])

dnl AM_MISSING_PROG(NAME, PROGRAM, DIRECTORY)
dnl The program must properly implement --version.
AC_DEFUN([AM_MISSING_PROG],
[AC_MSG_CHECKING(for working $2)
# Run test in a subshell; some versions of sh will print an error if
# an executable is not found, even if stderr is redirected.
# Redirect stdin to placate older versions of autoconf.  Sigh.
if ($2 --version) < /dev/null > /dev/null 2>&1; then
   $1=$2
   AC_MSG_RESULT(found)
else
   $1="$3/missing $2"
   AC_MSG_RESULT(missing)
fi
AC_SUBST($1)])

# Like AC_CONFIG_HEADER, but automatically create stamp file.

AC_DEFUN([AM_CONFIG_HEADER],
[AC_PREREQ([2.12])
AC_CONFIG_HEADER([$1])
dnl When config.status generates a header, we must update the stamp-h file.
dnl This file resides in the same directory as the config header
dnl that is generated.  We must strip everything past the first ":",
dnl and everything past the last "/".
AC_OUTPUT_COMMANDS(changequote(<<,>>)dnl
ifelse(patsubst(<<$1>>, <<[^ ]>>, <<>>), <<>>,
<<test -z "<<$>>CONFIG_HEADERS" || echo timestamp > patsubst(<<$1>>, <<^\([^:]*/\)?.*>>, <<\1>>)stamp-h<<>>dnl>>,
<<am_indx=1
for am_file in <<$1>>; do
  case " <<$>>CONFIG_HEADERS " in
  *" <<$>>am_file "*<<)>>
    echo timestamp > `echo <<$>>am_file | sed -e 's%:.*%%' -e 's%[^/]*$%%'`stamp-h$am_indx
    ;;
  esac
  am_indx=`expr "<<$>>am_indx" + 1`
done<<>>dnl>>)
changequote([,]))])

# Add --enable-maintainer-mode option to configure.
# From Jim Meyering

# serial 1

AC_DEFUN([AM_MAINTAINER_MODE],
[AC_MSG_CHECKING([whether to enable maintainer-specific portions of Makefiles])
  dnl maintainer-mode is disabled by default
  AC_ARG_ENABLE(maintainer-mode,
[  --enable-maintainer-mode enable make rules and dependencies not useful
                          (and sometimes confusing) to the casual installer],
      USE_MAINTAINER_MODE=$enableval,
      USE_MAINTAINER_MODE=no)
  AC_MSG_RESULT($USE_MAINTAINER_MODE)
  AM_CONDITIONAL(MAINTAINER_MODE, test $USE_MAINTAINER_MODE = yes)
  MAINT=$MAINTAINER_MODE_TRUE
  AC_SUBST(MAINT)dnl
]
)

