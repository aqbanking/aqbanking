# $Id$
# (c) 2004 Martin Preuss<martin@libchipcard.de>
# This function checks for chipcard2-client

AC_DEFUN([AC_CHIPCARD_CLIENT], [
dnl searches for chipcard_client
dnl Arguments: 
dnl   $1: major version minimum
dnl   $2: minor version minimum
dnl   $3: patchlevel version minimum
dnl   $4: build version minimum
dnl Returns: chipcard_client_dir
dnl          chipcard_client_datadir
dnl          chipcard_client_libs
dnl          chipcard_client_includes
dnl          chipcard_client_servicedir
dnl          chipcard_client_infolib
dnl          chipcard_client_servicelib
dnl          have_chipcard_client

if test -z "$1"; then vma="0"; else vma="$1"; fi
if test -z "$2"; then vmi="1"; else vmi="$2"; fi
if test -z "$3"; then vpl="0"; else vpl="$3"; fi
if test -z "$4"; then vbld="0"; else vbld="$4"; fi

AC_MSG_CHECKING(if chipcard_client support desired)
AC_ARG_ENABLE(chipcard-client,
  [  --enable-chipcard-client      enable chipcard_client support (default=yes)],
  enable_chipcard_client="$enableval",
  enable_chipcard_client="yes")
AC_MSG_RESULT($enable_chipcard_client)

have_chipcard_client="no"
chipcard_client_dir=""
chipcard_client_datadir=""
chipcard_client_libs=""
chipcard_client_infolib=""
chipcard_client_servicelib=""
chipcard_client_includes=""
chipcard_client_servicedir=""
if test "$enable_chipcard_client" != "no"; then
  AC_MSG_CHECKING(for chipcard_client)
  AC_ARG_WITH(chipcard-client-dir, [  --with-chipcard-client-dir=DIR
                            uses chipcard_client from given dir],
    [lcc_dir="$withval"],
    [lcc_dir="${prefix} \
	     /usr/local \
             /usr \
	     /chipcard-client \
             /sw \
             /"])

  for li in $lcc_dir; do
      if test -x "$li/bin/chipcard2-client-config"; then
          chipcard_client_dir="$li";
          break
      fi
  done
  if test -z "$chipcard_client_dir"; then
      AC_MSG_RESULT([not found ])
      AC_MSG_ERROR([
*** The library libchipcard2c was not found. Obtain it from 
*** http://www.libchipcard.de.
*** If it is already installed (including the -devel package), 
*** you might need to specify the location with the 
*** option --with-chipcard-client-dir=DIR.
***
*** Please note that it is not Libchipcard this package requested, it is the
*** successor chipcard2.])
  else
      AC_MSG_RESULT($chipcard_client_dir)
      AC_MSG_CHECKING(for chipcard-client libs)
      chipcard_client_libs="`$chipcard_client_dir/bin/chipcard2-client-config --libraries`"
      AC_MSG_RESULT($chipcard_client_libs)
      AC_MSG_CHECKING(for chipcard-client includes)
      chipcard_client_includes="`$chipcard_client_dir/bin/chipcard2-client-config --includes`"
      AC_MSG_RESULT($chipcard_client_includes)
      AC_MSG_CHECKING(for chipcard-client datadir)
      chipcard_client_datadir="`$chipcard_client_dir/bin/chipcard2-client-config --datadir`"
      AC_MSG_RESULT($chipcard_client_datadir)
      AC_MSG_CHECKING(for chipcard-client servicedir)
      chipcard_client_servicedir="`$chipcard_client_dir/bin/chipcard2-client-config --servicedir`"
      AC_MSG_RESULT($chipcard_client_servicedir)
      AC_MSG_CHECKING(for chipcard-client infolib)
      chipcard_client_infolib="`$chipcard_client_dir/bin/chipcard2-client-config --infolib`"
      AC_MSG_RESULT($chipcard_client_infolib)
      AC_MSG_CHECKING(for chipcard-client servicelib)
      chipcard_client_servicelib="`$chipcard_client_dir/bin/chipcard2-client-config --servicelib`"
      AC_MSG_RESULT($chipcard_client_servicelib)
  fi
  AC_MSG_CHECKING(if chipcard_client test desired)
  AC_ARG_ENABLE(chipcard-client-test,
    [  --enable-chipcard-client-test   enable chipcard_client-test (default=yes)],
     enable_chipcard_client_test="$enableval",
     enable_chipcard_client_test="yes")
  AC_MSG_RESULT($enable_chipcard_client_test)
  AC_MSG_CHECKING(for Chipcard-Client version >=$vma.$vmi.$vpl.$vbld)
  if test "$enable_chipcard_client_test" != "no"; then
    chipcard_client_versionstring="`$chipcard_client_dir/bin/chipcard2-client-config --vstring`.`$chipcard_client_dir/bin/chipcard2-client-config --vbuild`"
    AC_MSG_RESULT([found $chipcard_client_versionstring])
    if test "$vma" -gt "`$chipcard_client_dir/bin/chipcard2-client-config --vmajor`"; then
      AC_MSG_ERROR([Your Chipcard-Client version is way too old.
      Please update from http://sf.net/projects/libchipcard])
    elif test "$vma" = "`$chipcard_client_dir/bin/chipcard2-client-config --vmajor`"; then
      if test "$vmi" -gt "`$chipcard_client_dir/bin/chipcard2-client-config --vminor`"; then
        AC_MSG_ERROR([Your Chipcard-Client version is too old.
          Please update from http://sf.net/projects/libchipcard])
      elif test "$vmi" = "`$chipcard_client_dir/bin/chipcard2-client-config --vminor`"; then
          if test "$vpl" -gt "`$chipcard_client_dir/bin/chipcard2-client-config --vpatchlevel`"; then
            AC_MSG_ERROR([Your Chipcard-Client version is a little bit too old.
            Please update from http://sf.net/projects/libchipcard])
          elif test "$vpl" = "`$chipcard_client_dir/bin/chipcard2-client-config --vpatchlevel`"; then
            if test "$vbld" -gt "`$chipcard_client_dir/bin/chipcard2-client-config --vbuild`"; then
              AC_MSG_ERROR([Your Chipcard-Client version is a little bit too old. 
  Please update to the latest CVS version. Instructions for accessing 
  CVS can be found on http://sf.net/projects/libchipcard])
             fi
           fi
      fi
    fi
    have_chipcard_client="yes"
    #AC_MSG_RESULT(yes)
  else
    have_chipcard_client="yes"
    AC_MSG_RESULT(assuming yes)
  fi
dnl end of "if enable-chipcard-client"
fi

AC_SUBST(chipcard_client_dir)
AC_SUBST(chipcard_client_datadir)
AC_SUBST(chipcard_client_servicedir)
AC_SUBST(chipcard_client_infolib)
AC_SUBST(chipcard_client_servicelib)
AC_SUBST(chipcard_client_libs)
AC_SUBST(chipcard_client_includes)
])
