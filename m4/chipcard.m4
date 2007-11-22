# $Id: chipcard.m4 79 2005-05-31 22:50:34Z aquamaniac $
# (c) 2004-2006 Martin Preuss<martin@libchipcard.de>
# This function checks for chipcard-client and chipcard-server

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
      if test -x "$li/bin/chipcard-config"; then
          chipcard_client_dir="$li";
          break
      fi
  done
  if test -z "$chipcard_client_dir"; then
      AC_MSG_RESULT([not found ])
      AC_MSG_ERROR([
*** The library libchipcardc was not found. Obtain it from 
*** http://www.libchipcard.de.
*** If it is already installed (including the -devel package), 
*** you might need to specify the location with the 
*** option --with-chipcard-client-dir=DIR.
***
*** Please note that it is not Libchipcard this package requested, it is the
*** successor chipcard.])
  else
      AC_MSG_RESULT($chipcard_client_dir)
      AC_MSG_CHECKING(for chipcard-client libs)
      chipcard_client_libs="`$chipcard_client_dir/bin/chipcard-config --client-libs`"
      AC_MSG_RESULT($chipcard_client_libs)
      AC_MSG_CHECKING(for chipcard-client includes)
      chipcard_client_includes="`$chipcard_client_dir/bin/chipcard-config --includes`"
      AC_MSG_RESULT($chipcard_client_includes)
      AC_MSG_CHECKING(for chipcard-client datadir)
      chipcard_client_datadir="`$chipcard_client_dir/bin/chipcard-config --client-datadir`"
      AC_MSG_RESULT($chipcard_client_datadir)
  fi
  AC_MSG_CHECKING(if chipcard_client test desired)
  AC_ARG_ENABLE(chipcard-client-test,
    [  --enable-chipcard-client-test   enable chipcard_client-test (default=yes)],
     enable_chipcard_client_test="$enableval",
     enable_chipcard_client_test="yes")
  AC_MSG_RESULT($enable_chipcard_client_test)
  AC_MSG_CHECKING(for Chipcard-Client version >=$vma.$vmi.$vpl.$vbld)
  if test "$enable_chipcard_client_test" != "no"; then
    chipcard_client_versionstring="`$chipcard_client_dir/bin/chipcard-config --vstring`.`$chipcard_client_dir/bin/chipcard-config --vbuild`"
    AC_MSG_RESULT([found $chipcard_client_versionstring])
    if test "$vma" -gt "`$chipcard_client_dir/bin/chipcard-config --vmajor`"; then
      AC_MSG_ERROR([Your Chipcard-Client version is way too old.
      Please update from http://sf.net/projects/libchipcard])
    elif test "$vma" = "`$chipcard_client_dir/bin/chipcard-config --vmajor`"; then
      if test "$vmi" -gt "`$chipcard_client_dir/bin/chipcard-config --vminor`"; then
        AC_MSG_ERROR([Your Chipcard-Client version is too old.
          Please update from http://sf.net/projects/libchipcard])
      elif test "$vmi" = "`$chipcard_client_dir/bin/chipcard-config --vminor`"; then
          if test "$vpl" -gt "`$chipcard_client_dir/bin/chipcard-config --vpatchlevel`"; then
            AC_MSG_ERROR([Your Chipcard-Client version is a little bit too old.
            Please update from http://sf.net/projects/libchipcard])
          elif test "$vpl" = "`$chipcard_client_dir/bin/chipcard-config --vpatchlevel`"; then
            if test "$vbld" -gt "`$chipcard_client_dir/bin/chipcard-config --vbuild`"; then
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
AC_SUBST(chipcard_client_libs)
AC_SUBST(chipcard_client_includes)
])





AC_DEFUN([AC_CHIPCARD_SERVER], [
dnl searches for chipcard_server
dnl Arguments: 
dnl   $1: major version minimum
dnl   $2: minor version minimum
dnl   $3: patchlevel version minimum
dnl   $4: build version minimum
dnl Returns: chipcard_server_datadir
dnl          chipcard_server_driverdir
dnl          chipcard_server_servicedir
dnl          have_chipcard_server

if test -z "$1"; then vma="0"; else vma="$1"; fi
if test -z "$2"; then vmi="1"; else vmi="$2"; fi
if test -z "$3"; then vpl="0"; else vpl="$3"; fi
if test -z "$4"; then vbld="0"; else vbld="$4"; fi

AC_MSG_CHECKING(if chipcard_server support desired)
AC_ARG_ENABLE(chipcard-server,
  [  --enable-chipcard-server      enable chipcard_server support (default=yes)],
  enable_chipcard_server="$enableval",
  enable_chipcard_server="yes")
AC_MSG_RESULT($enable_chipcard_server)

have_chipcard_server="no"
chipcard_server_dir=""
chipcard_server_servicedir=""
chipcard_server_driverdir=""
chipcard_server_datadir=""
if test "$enable_chipcard_server" != "no"; then
  AC_MSG_CHECKING(for chipcard_server)
  AC_ARG_WITH(chipcard-server-dir, [  --with-chipcard-server-dir=DIR
                            uses chipcard_server from given dir],
    [lcc_dir="$withval"],
    [lcc_dir="${prefix} \
	     /usr/local \
             /usr \
	     /chipcard-server \
             /"])

  for li in $lcc_dir; do
      if test -x "$li/bin/chipcard-config"; then
          chipcard_server_dir="$li";
          break
      fi
  done
  if test -z "$chipcard_server_dir"; then
      AC_MSG_RESULT([not found ])
      AC_MSG_ERROR([
*** The library libchipcards was not found. Obtain it from 
*** http://www.libchipcard.de.
*** If it is already installed (including the -devel package), 
*** you might need to specify the location with the 
*** option --with-chipcard-server-dir=DIR.
***
*** Please note that it is not Libchipcard this package requested, it is the
*** successor chipcard.])
  else
      AC_MSG_RESULT($chipcard_server_dir)
      AC_MSG_CHECKING(for chipcard-server datadir)
      chipcard_server_datadir="`$chipcard_server_dir/bin/chipcard-config --server-datadir`"
      AC_MSG_RESULT($chipcard_server_datadir)
      AC_MSG_CHECKING(for chipcard-server driver dir)
      chipcard_server_driverdir="`$chipcard_server_dir/bin/chipcard-config --driverdir`"
      AC_MSG_RESULT($chipcard_server_driverdir)
      AC_MSG_CHECKING(for chipcard-server service dir)
      chipcard_server_servicedir="`$chipcard_server_dir/bin/chipcard-config --servicedir`"
      AC_MSG_RESULT($chipcard_server_servicedir)

  fi
  AC_MSG_CHECKING(if chipcard_server test desired)
  AC_ARG_ENABLE(chipcard-server-test,
    [  --enable-chipcard-server-test   enable chipcard_server-test (default=yes)],
     enable_chipcard_server_test="$enableval",
     enable_chipcard_server_test="yes")
  AC_MSG_RESULT($enable_chipcard_server_test)
  AC_MSG_CHECKING(for Chipcard-Server version >=$vma.$vmi.$vpl.$vbld)
  if test "$enable_chipcard_server_test" != "no"; then
    chipcard_server_versionstring="`$chipcard_server_dir/bin/chipcard-config --vstring`.`$chipcard_server_dir/bin/chipcard-config --vbuild`"
    AC_MSG_RESULT([found $chipcard_server_versionstring])
    if test "$vma" -gt "`$chipcard_server_dir/bin/chipcard-config --vmajor`"; then
      AC_MSG_ERROR([Your Chipcard-Server version is way too old.
      Please update from http://sf.net/projects/libchipcard])
    elif test "$vma" = "`$chipcard_server_dir/bin/chipcard-config --vmajor`"; then
      if test "$vmi" -gt "`$chipcard_server_dir/bin/chipcard-config --vminor`"; then
        AC_MSG_ERROR([Your Chipcard-Server version is too old.
          Please update from http://sf.net/projects/libchipcard])
      elif test "$vmi" = "`$chipcard_server_dir/bin/chipcard-config --vminor`"; then
          if test "$vpl" -gt "`$chipcard_server_dir/bin/chipcard-config --vpatchlevel`"; then
            AC_MSG_ERROR([Your Chipcard-Server version is a little bit too old.
            Please update from http://sf.net/projects/libchipcard])
          elif test "$vpl" = "`$chipcard_server_dir/bin/chipcard-config --vpatchlevel`"; then
            if test "$vbld" -gt "`$chipcard_server_dir/bin/chipcard-config --vbuild`"; then
              AC_MSG_ERROR([Your Chipcard-Server version is a little bit too old. 
  Please update to the latest CVS version. Instructions for accessing 
  CVS can be found on http://sf.net/projects/libchipcard])
             fi
           fi
      fi
    fi
    have_chipcard_server="yes"
    #AC_MSG_RESULT(yes)
  else
    have_chipcard_server="yes"
    AC_MSG_RESULT(assuming yes)
  fi
dnl end of "if enable-chipcard-server"
fi

AC_SUBST(chipcard_server_servicedir)
AC_SUBST(chipcard_server_driverdir)
AC_SUBST(chipcard_server_datadir)
])


