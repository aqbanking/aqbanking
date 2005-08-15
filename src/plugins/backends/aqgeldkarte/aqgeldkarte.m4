# $Id$
# (c) 2004 Martin Preuss<martin@libchipcard.de>
# This function checks for AqGeldKarte

AC_DEFUN([AC_AQGELDKARTE], [
dnl searches for aqgeldkarte
dnl Arguments: 
dnl   $1: major version minimum
dnl   $2: minor version minimum
dnl   $3: patchlevel version minimum
dnl   $4: build version minimum
dnl Returns: aqgeldkarte_dir
dnl          aqgeldkarte_libs
dnl          aqgeldkarte_data
dnl          aqgeldkarte_plugins
dnl          aqgeldkarte_includes
dnl          have_aqgeldkarte

if test -z "$1"; then vma="0"; else vma="$1"; fi
if test -z "$2"; then vmi="1"; else vmi="$2"; fi
if test -z "$3"; then vpl="0"; else vpl="$3"; fi
if test -z "$4"; then vbld="0"; else vbld="$4"; fi

AC_MSG_CHECKING(if aqgeldkarte support desired)
AC_ARG_ENABLE(aqgeldkarte,
  [  --enable-aqgeldkarte      enable aqgeldkarte support (default=yes)],
  enable_aqgeldkarte="$enableval",
  enable_aqgeldkarte="yes")
AC_MSG_RESULT($enable_aqgeldkarte)

have_aqgeldkarte="no"
aqgeldkarte_dir=""
aqgeldkarte_plugins=""
aqgeldkarte_libs=""
aqgeldkarte_includes=""
if test "$enable_aqgeldkarte" != "no"; then
  AC_MSG_CHECKING(for aqgeldkarte)
  AC_ARG_WITH(aqgeldkarte-dir, [  --with-aqgeldkarte-dir=DIR
                            uses aqgeldkarte from given dir],
    [lcc_dir="$withval"],
    [lcc_dir="${prefix} \
	     /usr/local \
             /usr \
             /"])

  for li in $lcc_dir; do
      if test -x "$li/bin/aqgeldkarte-config"; then
          aqgeldkarte_dir="$li";
          break
      fi
  done
  if test -z "$aqgeldkarte_dir"; then
      AC_MSG_RESULT([not found ])
      AC_MSG_ERROR([
*** The library AqGeldKarte was not found. Obtain it from 
*** http://www.aquamaniac.de. 
*** If it is already installed (including the -devel package), 
*** you might need to specify the location with the 
*** option --with-aqgeldkarte-dir=DIR.])
  else
      AC_MSG_RESULT($aqgeldkarte_dir)
      AC_MSG_CHECKING(for aqgeldkarte libs)
      aqgeldkarte_libs="`$aqgeldkarte_dir/bin/aqgeldkarte-config --libraries`"
      AC_MSG_RESULT($aqgeldkarte_libs)
      AC_MSG_CHECKING(for aqgeldkarte includes)
      aqgeldkarte_includes="`$aqgeldkarte_dir/bin/aqgeldkarte-config --includes`"
      AC_MSG_RESULT($aqgeldkarte_includes)
      AC_MSG_CHECKING(for aqgeldkarte plugins)
      aqgeldkarte_plugins="`$aqgeldkarte_dir/bin/aqgeldkarte-config --plugins`"
      AC_MSG_RESULT($aqgeldkarte_plugins)
      AC_MSG_CHECKING(for aqgeldkarte data)
      aqgeldkarte_data="`$aqgeldkarte_dir/bin/aqgeldkarte-config --data`"
      AC_MSG_RESULT($aqgeldkarte_data)
  fi
  AC_MSG_CHECKING(if aqgeldkarte test desired)
  AC_ARG_ENABLE(aqgeldkarte,
    [  --enable-aqgeldkarte-test   enable aqgeldkarte-test (default=yes)],
     enable_aqgeldkarte_test="$enableval",
     enable_aqgeldkarte_test="yes")
  AC_MSG_RESULT($enable_aqgeldkarte_test)
  AC_MSG_CHECKING(for AqGeldKarte version >=$vma.$vmi.$vpl.$vbld)
  if test "$enable_aqgeldkarte_test" != "no"; then
    aqgeldkarte_versionstring="`$aqgeldkarte_dir/bin/aqgeldkarte-config --vstring`.`$aqgeldkarte_dir/bin/aqgeldkarte-config --vbuild`"
    AC_MSG_RESULT([found $aqgeldkarte_versionstring])
    if test "$vma" -gt "`$aqgeldkarte_dir/bin/aqgeldkarte-config --vmajor`"; then
      AC_MSG_ERROR([Your AqGeldKarte version is way too old.
      Please update from http://www.aquamaniac.de/aqgeldkarte/])
    elif test "$vma" = "`$aqgeldkarte_dir/bin/aqgeldkarte-config --vmajor`"; then
      if test "$vmi" -gt "`$aqgeldkarte_dir/bin/aqgeldkarte-config --vminor`"; then
        AC_MSG_ERROR([Your AqGeldKarte version is too old.
          Please update from http://www.aquamaniac.de/aqgeldkarte/])
      elif test "$vmi" = "`$aqgeldkarte_dir/bin/aqgeldkarte-config --vminor`"; then
          if test "$vpl" -gt "`$aqgeldkarte_dir/bin/aqgeldkarte-config --vpatchlevel`"; then
            AC_MSG_ERROR([Your AqGeldKarte version is a little bit too old.
            Please update from http://www.aquamaniac.de/aqgeldkarte/])
          elif test "$vpl" = "`$aqgeldkarte_dir/bin/aqgeldkarte-config --vpatchlevel`"; then
            if test "$vbld" -gt "`$aqgeldkarte_dir/bin/aqgeldkarte-config --vbuild`"; then
              AC_MSG_ERROR([Your AqGeldKarte version is a little bit too old. 
  Please update to the latest CVS version. Instructions for accessing 
  CVS can be found on http://www.aquamaniac.de/aqgeldkarte/])
             fi
           fi
      fi
    fi
    have_aqgeldkarte="yes"
    #AC_MSG_RESULT(yes)
  else
    have_aqgeldkarte="yes"
    AC_MSG_RESULT(assuming yes)
  fi
dnl end of "if enable-aqgeldkarte"
fi

AC_SUBST(aqgeldkarte_dir)
AC_SUBST(aqgeldkarte_plugins)
AC_SUBST(aqgeldkarte_libs)
AC_SUBST(aqgeldkarte_data)
AC_SUBST(aqgeldkarte_includes)
])
