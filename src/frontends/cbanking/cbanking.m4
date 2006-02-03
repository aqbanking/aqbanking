# $Id$
# (c) 2004 Martin Preuss<martin@libchipcard.de>
# This function checks for CBanking

AC_DEFUN([AC_CBANKING], [
dnl searches for cbanking
dnl Arguments: 
dnl   $1: major version minimum
dnl   $2: minor version minimum
dnl   $3: patchlevel version minimum
dnl   $4: build version minimum
dnl Returns: cbanking_dir
dnl          cbanking_libs
dnl          cbanking_data
dnl          cbanking_includes
dnl          have_cbanking

if test -z "$1"; then vma="0"; else vma="$1"; fi
if test -z "$2"; then vmi="1"; else vmi="$2"; fi
if test -z "$3"; then vpl="0"; else vpl="$3"; fi
if test -z "$4"; then vbld="0"; else vbld="$4"; fi

AC_MSG_CHECKING(if cbanking support desired)
AC_ARG_ENABLE(cbanking,
  [  --enable-cbanking       enable cbanking support (default=detect)],
  enable_cbanking="$enableval",
  enable_cbanking="yes")
AC_MSG_RESULT($enable_cbanking)

have_cbanking="no"
cbanking_dir=""
cbanking_data=""
cbanking_libs=""
cbanking_includes=""
if test "$enable_cbanking" != "no"; then
  AC_MSG_CHECKING(for cbanking)
  AC_ARG_WITH(cbanking-dir, [  --with-cbanking-dir=DIR
                          uses cbanking from given dir],
    [lcc_dir="$withval"],
    [lcc_dir="${prefix} \
	     /usr/local \
             /usr \
             /"])

  for li in $lcc_dir; do
      if test -x "$li/bin/cbanking-config"; then
          cbanking_dir="$li";
          break
      fi
  done
  if test -z "$cbanking_dir"; then
      AC_MSG_RESULT([not found ])
      have_cbanking="no"
  else
      AC_MSG_RESULT($cbanking_dir)
      AC_MSG_CHECKING(for cbanking libs)
      cbanking_libs="`$cbanking_dir/bin/cbanking-config --libraries`"
      AC_MSG_RESULT($cbanking_libs)
      AC_MSG_CHECKING(for cbanking includes)
      cbanking_includes="`$cbanking_dir/bin/cbanking-config --includes`"
      AC_MSG_RESULT($cbanking_includes)
      AC_MSG_CHECKING(for cbanking data)
      cbanking_data="`$cbanking_dir/bin/cbanking-config --data`"
      AC_MSG_RESULT($cbanking_data)

      AC_MSG_CHECKING(if cbanking test desired)
      AC_ARG_ENABLE(cbanking,
        [  --enable-cbanking-test  enable cbanking-test (default=yes)],
         enable_cbanking_test="$enableval",
         enable_cbanking_test="yes")
      AC_MSG_RESULT($enable_cbanking_test)
      AC_MSG_CHECKING(for CBanking version >=$vma.$vmi.$vpl.$vbld)
      if test "$enable_cbanking_test" != "no"; then
        cbanking_versionstring="`$cbanking_dir/bin/cbanking-config --vstring`.`$cbanking_dir/bin/cbanking-config --vbuild`"
        AC_MSG_RESULT([found $cbanking_versionstring])
        if test "$vma" -gt "`$cbanking_dir/bin/cbanking-config --vmajor`"; then
          AC_MSG_ERROR([Your CBanking version is way too old.
          Please update from http://www.aquamaniac.de/acbanking/])
        elif test "$vma" = "`$cbanking_dir/bin/cbanking-config --vmajor`"; then
          if test "$vmi" -gt "`$cbanking_dir/bin/cbanking-config --vminor`"; then
            AC_MSG_ERROR([Your CBanking version is too old.
              Please update from http://www.aquamaniac.de/acbanking/])
          elif test "$vmi" = "`$cbanking_dir/bin/cbanking-config --vminor`"; then
              if test "$vpl" -gt "`$cbanking_dir/bin/cbanking-config --vpatchlevel`"; then
                AC_MSG_ERROR([Your CBanking version is a little bit too old.
                Please update from http://www.aquamaniac.de/acbanking/])
              elif test "$vpl" = "`$cbanking_dir/bin/cbanking-config --vpatchlevel`"; then
                if test "$vbld" -gt "`$cbanking_dir/bin/cbanking-config --vbuild`"; then
                  AC_MSG_ERROR([Your CBanking version is a little bit too old. 
      Please update to the latest CVS version. Instructions for accessing 
      CVS can be found on http://www.aquamaniac.de/acbanking/])
                 fi
               fi
          fi
        fi
        have_cbanking="yes"
        #AC_MSG_RESULT(yes)
        AC_DEFINE_UNQUOTED(HAVE_CBANKING, 1, [Defines if your system has the cbanking package])
      else
        have_cbanking="yes"
        AC_MSG_RESULT(assuming yes)
        AC_DEFINE_UNQUOTED(HAVE_CBANKING, 1, [Defines if your system has the cbanking package])
      fi
   fi
dnl end of "if enable-cbanking"
fi

AC_SUBST(cbanking_dir)
AC_SUBST(cbanking_libs)
AC_SUBST(cbanking_data)
AC_SUBST(cbanking_includes)
])
