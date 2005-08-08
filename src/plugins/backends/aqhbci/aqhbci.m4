# $Id$
# (c) 2004 Martin Preuss<martin@libchipcard.de>
# This function checks for AqHBCI

AC_DEFUN([AC_AQHBCI], [
dnl searches for aqhbci
dnl Arguments: 
dnl   $1: major version minimum
dnl   $2: minor version minimum
dnl   $3: patchlevel version minimum
dnl   $4: build version minimum
dnl Returns: aqhbci_dir
dnl          aqhbci_libs
dnl          aqhbci_data
dnl          aqhbci_plugins
dnl          aqhbci_includes
dnl          have_aqhbci

if test -z "$1"; then vma="0"; else vma="$1"; fi
if test -z "$2"; then vmi="1"; else vmi="$2"; fi
if test -z "$3"; then vpl="0"; else vpl="$3"; fi
if test -z "$4"; then vbld="0"; else vbld="$4"; fi

AC_MSG_CHECKING(if aqhbci support desired)
AC_ARG_ENABLE(aqhbci,
  [  --enable-aqhbci      enable aqhbci support (default=yes)],
  enable_aqhbci="$enableval",
  enable_aqhbci="yes")
AC_MSG_RESULT($enable_aqhbci)

have_aqhbci="no"
aqhbci_dir=""
aqhbci_plugins=""
aqhbci_libs=""
aqhbci_includes=""
if test "$enable_aqhbci" != "no"; then
  AC_MSG_CHECKING(for aqhbci)
  AC_ARG_WITH(aqhbci-dir, [  --with-aqhbci-dir=DIR
                            uses aqhbci from given dir],
    [lcc_dir="$withval"],
    [lcc_dir="${prefix} \
	     /usr/local \
             /usr \
             /"])

  for li in $lcc_dir; do
      if test -x "$li/bin/aqhbci-config"; then
          aqhbci_dir="$li";
          break
      fi
  done
  if test -z "$aqhbci_dir"; then
      AC_MSG_RESULT([not found ])
      AC_MSG_ERROR([
*** The library AqHBCI was not found. Obtain it from 
*** http://www.aquamaniac.de. 
*** If it is already installed (including the -devel package), 
*** you might need to specify the location with the 
*** option --with-aqhbci-dir=DIR.])
  else
      AC_MSG_RESULT($aqhbci_dir)
      AC_MSG_CHECKING(for aqhbci libs)
      aqhbci_libs="`$aqhbci_dir/bin/aqhbci-config --libraries`"
      AC_MSG_RESULT($aqhbci_libs)
      AC_MSG_CHECKING(for aqhbci includes)
      aqhbci_includes="`$aqhbci_dir/bin/aqhbci-config --includes`"
      AC_MSG_RESULT($aqhbci_includes)
      AC_MSG_CHECKING(for aqhbci plugins)
      aqhbci_plugins="`$aqhbci_dir/bin/aqhbci-config --plugins`"
      AC_MSG_RESULT($aqhbci_plugins)
      AC_MSG_CHECKING(for aqhbci data)
      aqhbci_data="`$aqhbci_dir/bin/aqhbci-config --data`"
      AC_MSG_RESULT($aqhbci_data)
  fi
  AC_MSG_CHECKING(if aqhbci test desired)
  AC_ARG_ENABLE(aqhbci,
    [  --enable-aqhbci-test   enable aqhbci-test (default=yes)],
     enable_aqhbci_test="$enableval",
     enable_aqhbci_test="yes")
  AC_MSG_RESULT($enable_aqhbci_test)
  AC_MSG_CHECKING(for AqHBCI version >=$vma.$vmi.$vpl.$vbld)
  if test "$enable_aqhbci_test" != "no"; then
    aqhbci_versionstring="`$aqhbci_dir/bin/aqhbci-config --vstring`.`$aqhbci_dir/bin/aqhbci-config --vbuild`"
    AC_MSG_RESULT([found $aqhbci_versionstring])
    if test "$vma" -gt "`$aqhbci_dir/bin/aqhbci-config --vmajor`"; then
      AC_MSG_ERROR([Your AqHBCI version is way too old.
      Please update from http://www.aquamaniac.de/aqhbci/])
    elif test "$vma" = "`$aqhbci_dir/bin/aqhbci-config --vmajor`"; then
      if test "$vmi" -gt "`$aqhbci_dir/bin/aqhbci-config --vminor`"; then
        AC_MSG_ERROR([Your AqHBCI version is too old.
          Please update from http://www.aquamaniac.de/aqhbci/])
      elif test "$vmi" = "`$aqhbci_dir/bin/aqhbci-config --vminor`"; then
          if test "$vpl" -gt "`$aqhbci_dir/bin/aqhbci-config --vpatchlevel`"; then
            AC_MSG_ERROR([Your AqHBCI version is a little bit too old.
            Please update from http://www.aquamaniac.de/aqhbci/])
          elif test "$vpl" = "`$aqhbci_dir/bin/aqhbci-config --vpatchlevel`"; then
            if test "$vbld" -gt "`$aqhbci_dir/bin/aqhbci-config --vbuild`"; then
              AC_MSG_ERROR([Your AqHBCI version is a little bit too old. 
  Please update to the latest CVS version. Instructions for accessing 
  CVS can be found on http://www.aquamaniac.de/aqhbci/])
             fi
           fi
      fi
    fi
    have_aqhbci="yes"
    #AC_MSG_RESULT(yes)
  else
    have_aqhbci="yes"
    AC_MSG_RESULT(assuming yes)
  fi
dnl end of "if enable-aqhbci"
fi

AC_SUBST(aqhbci_dir)
AC_SUBST(aqhbci_plugins)
AC_SUBST(aqhbci_libs)
AC_SUBST(aqhbci_data)
AC_SUBST(aqhbci_includes)
])
