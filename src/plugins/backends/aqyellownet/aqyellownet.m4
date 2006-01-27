# $Id$
# (c) 2004 Martin Preuss<martin@libchipcard.de>
# This function checks for AqYellowNet

AC_DEFUN(AC_AQYELLOWNET, [
dnl searches for aqyellownet
dnl Arguments: 
dnl   $1: major version minimum
dnl   $2: minor version minimum
dnl   $3: patchlevel version minimum
dnl   $4: build version minimum
dnl Returns: aqyellownet_dir
dnl          aqyellownet_libs
dnl          aqyellownet_data
dnl          aqyellownet_plugins
dnl          aqyellownet_includes
dnl          have_aqyellownet

if test -z "$1"; then vma="0"; else vma="$1"; fi
if test -z "$2"; then vmi="1"; else vmi="$2"; fi
if test -z "$3"; then vpl="0"; else vpl="$3"; fi
if test -z "$4"; then vbld="0"; else vbld="$4"; fi

AC_MSG_CHECKING(if aqyellownet support desired)
AC_ARG_ENABLE(aqyellownet,
  [  --enable-aqyellownet      enable aqyellownet support (default=yes)],
  enable_aqyellownet="$enableval",
  enable_aqyellownet="yes")
AC_MSG_RESULT($enable_aqyellownet)

have_aqyellownet="no"
aqyellownet_dir=""
aqyellownet_plugins=""
aqyellownet_libs=""
aqyellownet_includes=""
if test "$enable_aqyellownet" != "no"; then
  AC_MSG_CHECKING(for aqyellownet)
  AC_ARG_WITH(aqyellownet-dir, [  --with-aqyellownet-dir=DIR
                            uses aqyellownet from given dir],
    [lcc_dir="$withval"],
    [lcc_dir="${prefix} \
	     /usr/local \
             /usr \
             /"])

  for li in $lcc_dir; do
      if test -x "$li/bin/aqyellownet-config"; then
          aqyellownet_dir="$li";
          break
      fi
  done
  if test -z "$aqyellownet_dir"; then
      AC_MSG_RESULT([not found ])
      AC_MSG_ERROR([
*** The library AqYellowNet was not found. Obtain it from 
*** http://www.aquamaniac.de. 
*** If it is already installed (including the -devel package), 
*** you might need to specify the location with the 
*** option --with-aqyellownet-dir=DIR.])
  else
      AC_MSG_RESULT($aqyellownet_dir)
      AC_MSG_CHECKING(for aqyellownet libs)
      aqyellownet_libs="`$aqyellownet_dir/bin/aqyellownet-config --libraries`"
      AC_MSG_RESULT($aqyellownet_libs)
      AC_MSG_CHECKING(for aqyellownet includes)
      aqyellownet_includes="`$aqyellownet_dir/bin/aqyellownet-config --includes`"
      AC_MSG_RESULT($aqyellownet_includes)
      AC_MSG_CHECKING(for aqyellownet plugins)
      aqyellownet_plugins="`$aqyellownet_dir/bin/aqyellownet-config --plugins`"
      AC_MSG_RESULT($aqyellownet_plugins)
      AC_MSG_CHECKING(for aqyellownet data)
      aqyellownet_data="`$aqyellownet_dir/bin/aqyellownet-config --data`"
      AC_MSG_RESULT($aqyellownet_data)
  fi
  AC_MSG_CHECKING(if aqyellownet test desired)
  AC_ARG_ENABLE(aqyellownet,
    [  --enable-aqyellownet-test   enable aqyellownet-test (default=yes)],
     enable_aqyellownet_test="$enableval",
     enable_aqyellownet_test="yes")
  AC_MSG_RESULT($enable_aqyellownet_test)
  AC_MSG_CHECKING(for AqYellowNet version >=$vma.$vmi.$vpl.$vbld)
  if test "$enable_aqyellownet_test" != "no"; then
    aqyellownet_versionstring="`$aqyellownet_dir/bin/aqyellownet-config --vstring`.`$aqyellownet_dir/bin/aqyellownet-config --vbuild`"
    AC_MSG_RESULT([found $aqyellownet_versionstring])
    if test "$vma" -gt "`$aqyellownet_dir/bin/aqyellownet-config --vmajor`"; then
      AC_MSG_ERROR([Your AqYellowNet version is way too old.
      Please update from http://www.aquamaniac.de/aqyellownet/])
    elif test "$vma" = "`$aqyellownet_dir/bin/aqyellownet-config --vmajor`"; then
      if test "$vmi" -gt "`$aqyellownet_dir/bin/aqyellownet-config --vminor`"; then
        AC_MSG_ERROR([Your AqYellowNet version is too old.
          Please update from http://www.aquamaniac.de/aqyellownet/])
      elif test "$vmi" = "`$aqyellownet_dir/bin/aqyellownet-config --vminor`"; then
          if test "$vpl" -gt "`$aqyellownet_dir/bin/aqyellownet-config --vpatchlevel`"; then
            AC_MSG_ERROR([Your AqYellowNet version is a little bit too old.
            Please update from http://www.aquamaniac.de/aqyellownet/])
          elif test "$vpl" = "`$aqyellownet_dir/bin/aqyellownet-config --vpatchlevel`"; then
            if test "$vbld" -gt "`$aqyellownet_dir/bin/aqyellownet-config --vbuild`"; then
              AC_MSG_ERROR([Your AqYellowNet version is a little bit too old. 
  Please update to the latest CVS version. Instructions for accessing 
  CVS can be found on http://www.aquamaniac.de/aqyellownet/])
             fi
           fi
      fi
    fi
    have_aqyellownet="yes"
    #AC_MSG_RESULT(yes)
  else
    have_aqyellownet="yes"
    AC_MSG_RESULT(assuming yes)
  fi
dnl end of "if enable-aqyellownet"
fi

AC_SUBST(aqyellownet_dir)
AC_SUBST(aqyellownet_plugins)
AC_SUBST(aqyellownet_libs)
AC_SUBST(aqyellownet_data)
AC_SUBST(aqyellownet_includes)
])
