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
