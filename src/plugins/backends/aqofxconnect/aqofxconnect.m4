# $Id$
# (c) 2004 Martin Preuss<martin@libchipcard.de>
# This function checks for AqOFXCONNECT

AC_DEFUN(AC_AQOFXCONNECT, [
dnl searches for aqofxconnect
dnl Arguments: 
dnl   $1: major version minimum
dnl   $2: minor version minimum
dnl   $3: patchlevel version minimum
dnl   $4: build version minimum
dnl Returns: aqofxconnect_dir
dnl          aqofxconnect_libs
dnl          aqofxconnect_data
dnl          aqofxconnect_plugins
dnl          aqofxconnect_includes
dnl          have_aqofxconnect

if test -z "$1"; then vma="0"; else vma="$1"; fi
if test -z "$2"; then vmi="1"; else vmi="$2"; fi
if test -z "$3"; then vpl="0"; else vpl="$3"; fi
if test -z "$4"; then vbld="0"; else vbld="$4"; fi

AC_MSG_CHECKING(if aqofxconnect support desired)
AC_ARG_ENABLE(aqofxconnect,
  [  --enable-aqofxconnect      enable aqofxconnect support (default=yes)],
  enable_aqofxconnect="$enableval",
  enable_aqofxconnect="yes")
AC_MSG_RESULT($enable_aqofxconnect)

have_aqofxconnect="no"
aqofxconnect_dir=""
aqofxconnect_plugins=""
aqofxconnect_libs=""
aqofxconnect_includes=""
if test "$enable_aqofxconnect" != "no"; then
  AC_MSG_CHECKING(for aqofxconnect)
  AC_ARG_WITH(aqofxconnect-dir, [  --with-aqofxconnect-dir=DIR
                            uses aqofxconnect from given dir],
    [lcc_dir="$withval"],
    [lcc_dir="${prefix} \
	     /usr/local \
             /usr \
             /"])

  for li in $lcc_dir; do
      if test -x "$li/bin/aqofxconnect-config"; then
          aqofxconnect_dir="$li";
          break
      fi
  done
  if test -z "$aqofxconnect_dir"; then
      AC_MSG_RESULT([not found ])
      AC_MSG_ERROR([
*** The library AqOFXCONNECT was not found. Obtain it from 
*** http://www.aquamaniac.de. 
*** If it is already installed (including the -devel package), 
*** you might need to specify the location with the 
*** option --with-aqofxconnect-dir=DIR.])
  else
      AC_MSG_RESULT($aqofxconnect_dir)
      AC_MSG_CHECKING(for aqofxconnect libs)
      aqofxconnect_libs="`$aqofxconnect_dir/bin/aqofxconnect-config --libraries`"
      AC_MSG_RESULT($aqofxconnect_libs)
      AC_MSG_CHECKING(for aqofxconnect includes)
      aqofxconnect_includes="`$aqofxconnect_dir/bin/aqofxconnect-config --includes`"
      AC_MSG_RESULT($aqofxconnect_includes)
      AC_MSG_CHECKING(for aqofxconnect plugins)
      aqofxconnect_plugins="`$aqofxconnect_dir/bin/aqofxconnect-config --plugins`"
      AC_MSG_RESULT($aqofxconnect_plugins)
      AC_MSG_CHECKING(for aqofxconnect data)
      aqofxconnect_data="`$aqofxconnect_dir/bin/aqofxconnect-config --data`"
      AC_MSG_RESULT($aqofxconnect_data)
  fi
  AC_MSG_CHECKING(if aqofxconnect test desired)
  AC_ARG_ENABLE(aqofxconnect,
    [  --enable-aqofxconnect-test   enable aqofxconnect-test (default=yes)],
     enable_aqofxconnect_test="$enableval",
     enable_aqofxconnect_test="yes")
  AC_MSG_RESULT($enable_aqofxconnect_test)
  AC_MSG_CHECKING(for AqOFXCONNECT version >=$vma.$vmi.$vpl.$vbld)
  if test "$enable_aqofxconnect_test" != "no"; then
    aqofxconnect_versionstring="`$aqofxconnect_dir/bin/aqofxconnect-config --vstring`.`$aqofxconnect_dir/bin/aqofxconnect-config --vbuild`"
    AC_MSG_RESULT([found $aqofxconnect_versionstring])
    if test "$vma" -gt "`$aqofxconnect_dir/bin/aqofxconnect-config --vmajor`"; then
      AC_MSG_ERROR([Your AqOFXCONNECT version is way too old.
      Please update from http://www.aquamaniac.de/aqofxconnect/])
    elif test "$vma" = "`$aqofxconnect_dir/bin/aqofxconnect-config --vmajor`"; then
      if test "$vmi" -gt "`$aqofxconnect_dir/bin/aqofxconnect-config --vminor`"; then
        AC_MSG_ERROR([Your AqOFXCONNECT version is too old.
          Please update from http://www.aquamaniac.de/aqofxconnect/])
      elif test "$vmi" = "`$aqofxconnect_dir/bin/aqofxconnect-config --vminor`"; then
          if test "$vpl" -gt "`$aqofxconnect_dir/bin/aqofxconnect-config --vpatchlevel`"; then
            AC_MSG_ERROR([Your AqOFXCONNECT version is a little bit too old.
            Please update from http://www.aquamaniac.de/aqofxconnect/])
          elif test "$vpl" = "`$aqofxconnect_dir/bin/aqofxconnect-config --vpatchlevel`"; then
            if test "$vbld" -gt "`$aqofxconnect_dir/bin/aqofxconnect-config --vbuild`"; then
              AC_MSG_ERROR([Your AqOFXCONNECT version is a little bit too old. 
  Please update to the latest CVS version. Instructions for accessing 
  CVS can be found on http://www.aquamaniac.de/aqofxconnect/])
             fi
           fi
      fi
    fi
    have_aqofxconnect="yes"
    #AC_MSG_RESULT(yes)
  else
    have_aqofxconnect="yes"
    AC_MSG_RESULT(assuming yes)
  fi
dnl end of "if enable-aqofxconnect"
fi

AC_SUBST(aqofxconnect_dir)
AC_SUBST(aqofxconnect_plugins)
AC_SUBST(aqofxconnect_libs)
AC_SUBST(aqofxconnect_data)
AC_SUBST(aqofxconnect_includes)
])
