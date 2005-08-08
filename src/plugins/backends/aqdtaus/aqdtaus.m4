# $Id$
# (c) 2004 Martin Preuss<martin@libchipcard.de>
# This function checks for AqDTAUS

AC_DEFUN([AC_AQDTAUS], [
dnl searches for aqdtaus
dnl Arguments: 
dnl   $1: major version minimum
dnl   $2: minor version minimum
dnl   $3: patchlevel version minimum
dnl   $4: build version minimum
dnl Returns: aqdtaus_dir
dnl          aqdtaus_libs
dnl          aqdtaus_data
dnl          aqdtaus_plugins
dnl          aqdtaus_includes
dnl          have_aqdtaus

if test -z "$1"; then vma="0"; else vma="$1"; fi
if test -z "$2"; then vmi="1"; else vmi="$2"; fi
if test -z "$3"; then vpl="0"; else vpl="$3"; fi
if test -z "$4"; then vbld="0"; else vbld="$4"; fi

AC_MSG_CHECKING(if aqdtaus support desired)
AC_ARG_ENABLE(aqdtaus,
  [  --enable-aqdtaus      enable aqdtaus support (default=yes)],
  enable_aqdtaus="$enableval",
  enable_aqdtaus="yes")
AC_MSG_RESULT($enable_aqdtaus)

have_aqdtaus="no"
aqdtaus_dir=""
aqdtaus_plugins=""
aqdtaus_libs=""
aqdtaus_includes=""
if test "$enable_aqdtaus" != "no"; then
  AC_MSG_CHECKING(for aqdtaus)
  AC_ARG_WITH(aqdtaus-dir, [  --with-aqdtaus-dir=DIR
                            uses aqdtaus from given dir],
    [lcc_dir="$withval"],
    [lcc_dir="${prefix} \
	     /usr/local \
             /usr \
             /"])

  for li in $lcc_dir; do
      if test -x "$li/bin/aqdtaus-config"; then
          aqdtaus_dir="$li";
          break
      fi
  done
  if test -z "$aqdtaus_dir"; then
      AC_MSG_RESULT([not found ])
      AC_MSG_ERROR([
*** The library AqDTAUS was not found. Obtain it from 
*** http://www.aquamaniac.de. 
*** If it is already installed (including the -devel package), 
*** you might need to specify the location with the 
*** option --with-aqdtaus-dir=DIR.])
  else
      AC_MSG_RESULT($aqdtaus_dir)
      AC_MSG_CHECKING(for aqdtaus libs)
      aqdtaus_libs="`$aqdtaus_dir/bin/aqdtaus-config --libraries`"
      AC_MSG_RESULT($aqdtaus_libs)
      AC_MSG_CHECKING(for aqdtaus includes)
      aqdtaus_includes="`$aqdtaus_dir/bin/aqdtaus-config --includes`"
      AC_MSG_RESULT($aqdtaus_includes)
      AC_MSG_CHECKING(for aqdtaus plugins)
      aqdtaus_plugins="`$aqdtaus_dir/bin/aqdtaus-config --plugins`"
      AC_MSG_RESULT($aqdtaus_plugins)
      AC_MSG_CHECKING(for aqdtaus data)
      aqdtaus_data="`$aqdtaus_dir/bin/aqdtaus-config --data`"
      AC_MSG_RESULT($aqdtaus_data)
  fi
  AC_MSG_CHECKING(if aqdtaus test desired)
  AC_ARG_ENABLE(aqdtaus,
    [  --enable-aqdtaus-test   enable aqdtaus-test (default=yes)],
     enable_aqdtaus_test="$enableval",
     enable_aqdtaus_test="yes")
  AC_MSG_RESULT($enable_aqdtaus_test)
  AC_MSG_CHECKING(for AqDTAUS version >=$vma.$vmi.$vpl.$vbld)
  if test "$enable_aqdtaus_test" != "no"; then
    aqdtaus_versionstring="`$aqdtaus_dir/bin/aqdtaus-config --vstring`.`$aqdtaus_dir/bin/aqdtaus-config --vbuild`"
    AC_MSG_RESULT([found $aqdtaus_versionstring])
    if test "$vma" -gt "`$aqdtaus_dir/bin/aqdtaus-config --vmajor`"; then
      AC_MSG_ERROR([Your AqDTAUS version is way too old.
      Please update from http://www.aquamaniac.de/aqdtaus/])
    elif test "$vma" = "`$aqdtaus_dir/bin/aqdtaus-config --vmajor`"; then
      if test "$vmi" -gt "`$aqdtaus_dir/bin/aqdtaus-config --vminor`"; then
        AC_MSG_ERROR([Your AqDTAUS version is too old.
          Please update from http://www.aquamaniac.de/aqdtaus/])
      elif test "$vmi" = "`$aqdtaus_dir/bin/aqdtaus-config --vminor`"; then
          if test "$vpl" -gt "`$aqdtaus_dir/bin/aqdtaus-config --vpatchlevel`"; then
            AC_MSG_ERROR([Your AqDTAUS version is a little bit too old.
            Please update from http://www.aquamaniac.de/aqdtaus/])
          elif test "$vpl" = "`$aqdtaus_dir/bin/aqdtaus-config --vpatchlevel`"; then
            if test "$vbld" -gt "`$aqdtaus_dir/bin/aqdtaus-config --vbuild`"; then
              AC_MSG_ERROR([Your AqDTAUS version is a little bit too old. 
  Please update to the latest CVS version. Instructions for accessing 
  CVS can be found on http://www.aquamaniac.de/aqdtaus/])
             fi
           fi
      fi
    fi
    have_aqdtaus="yes"
    #AC_MSG_RESULT(yes)
  else
    have_aqdtaus="yes"
    AC_MSG_RESULT(assuming yes)
  fi
dnl end of "if enable-aqdtaus"
fi

AC_SUBST(aqdtaus_dir)
AC_SUBST(aqdtaus_plugins)
AC_SUBST(aqdtaus_libs)
AC_SUBST(aqdtaus_data)
AC_SUBST(aqdtaus_includes)
])
