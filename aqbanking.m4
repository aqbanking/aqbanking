# $Id$
# (c) 2004 Martin Preuss<martin@libchipcard.de>
# This function checks for AqBanking

AC_DEFUN([AC_AQBANKING], [
dnl searches for aqbanking
dnl Arguments: 
dnl   $1: major version minimum
dnl   $2: minor version minimum
dnl   $3: patchlevel version minimum
dnl   $4: build version minimum
dnl Returns: aqbanking_dir
dnl          aqbanking_libs
dnl          aqbanking_libspp
dnl          aqbanking_data
dnl          aqbanking_plugins
dnl          aqbanking_includes
dnl          have_aqbanking

if test -z "$1"; then vma="0"; else vma="$1"; fi
if test -z "$2"; then vmi="1"; else vmi="$2"; fi
if test -z "$3"; then vpl="0"; else vpl="$3"; fi
if test -z "$4"; then vbld="0"; else vbld="$4"; fi

AC_MSG_CHECKING(if aqbanking support desired)
AC_ARG_ENABLE(aqbanking,
  [  --enable-aqbanking      enable aqbanking support (default=yes)],
  enable_aqbanking="$enableval",
  enable_aqbanking="yes")
AC_MSG_RESULT($enable_aqbanking)

have_aqbanking="no"
aqbanking_dir=""
aqbanking_data=""
aqbanking_plugins=""
aqbanking_libs=""
aqbanking_libspp=""
aqbanking_includes=""

qbanking_libs=""
qbanking_plugindir=""
qbanking_helpdir=""

aqhbci_libs=""

if test "$enable_aqbanking" != "no"; then
  AC_MSG_CHECKING(for aqbanking)
  AC_ARG_WITH(aqbanking-dir,
    [  --with-aqbanking-dir=DIR    obsolete - set PKG_CONFIG_PATH environment variable instead],
    [AC_MSG_RESULT([obsolete configure option '--with-aqbanking-dir' used])
     AC_MSG_ERROR([
*** Configure switch '--with-aqbanking-dir' is obsolete.
*** If you want to use aqbanking from a non-system location
*** then locate the file 'aqbanking.pc' and add its parent directory
*** to environment variable PKG_CONFIG_PATH. For example
*** configure <options> PKG_CONFIG_PATH="<path-to-aqbanking.pc's-dir>:\${PKG_CONFIG_PATH}"])],
    [])

  $PKG_CONFIG --exists aqbanking
  result=$?
  if test $result -ne 0; then
      AC_MSG_RESULT(not found)
      AC_MSG_ERROR([
*** Package aqbanking was not found in the pkg-config search path.
*** Perhaps you should add the directory containing `aqbanking.pc'
*** to the PKG_CONFIG_PATH environment variable])
  else
      aqbanking_dir="`$PKG_CONFIG --variable=prefix aqbanking`"
      AC_MSG_RESULT($aqbanking_dir)
  fi

  AC_MSG_CHECKING(for aqbanking libs)
  aqbanking_libs="`$PKG_CONFIG --libs aqbanking`"
  AC_MSG_RESULT($aqbanking_libs)

  AC_MSG_CHECKING(for aqbanking includes)
  aqbanking_includes="`$PKG_CONFIG --cflags aqbanking`"
  AC_MSG_RESULT($aqbanking_includes)

  AC_MSG_CHECKING(for aqbanking plugins)
  aqbanking_plugins="`$PKG_CONFIG --variable=plugindir aqbanking`"
  AC_MSG_RESULT($aqbanking_plugins)

  AC_MSG_CHECKING(for aqbanking data)
  aqbanking_data="`$PKG_CONFIG --variable=pkgdatadir aqbanking`"
  AC_MSG_RESULT($aqbanking_data)

  AC_MSG_CHECKING(whether QBanking is supported)
  have_qbanking="`$PKG_CONFIG --variable=has-qbanking aqbanking`"
  AC_MSG_RESULT($have_qbanking)

  AC_MSG_CHECKING(for qbanking libs)
  qbanking_libs="`$PKG_CONFIG --variable=qbanking-libraries aqbanking`"
  AC_MSG_RESULT($aqbanking_libs)

  AC_MSG_CHECKING(for qbanking plugins)
  qbanking_plugins="`$PKG_CONFIG --variable=qbanking-plugins aqbanking`"
  AC_MSG_RESULT($qbanking_plugins)

  AC_MSG_CHECKING(for qbanking helpdir)
  qbanking_helpdir="`$PKG_CONFIG --variable=qbanking-helpdir aqbanking`"
  AC_MSG_RESULT($qbanking_helpdir)

  AC_MSG_CHECKING(whether AqHBCI is supported)
  have_aqhbci="`$PKG_CONFIG --variable=has-aqhbci aqbanking`"
  AC_MSG_RESULT($have_aqhbci)

  AC_MSG_CHECKING(for AqHBCI libs)
  aqhbci_libs="`$PKG_CONFIG --variable=aqhbci-libraries aqbanking`"
  AC_MSG_RESULT($aqhbci_libs)
      
  AC_MSG_CHECKING(if aqbanking test desired)
  AC_ARG_ENABLE(aqbanking,
    [  --enable-aqbanking-test   enable aqbanking-test (default=yes)],
     enable_aqbanking_test="$enableval",
     enable_aqbanking_test="yes")
  AC_MSG_RESULT($enable_aqbanking_test)
  AC_MSG_CHECKING(for AqBanking version >=$vma.$vmi.$vpl.$vbld)
  if test "$enable_aqbanking_test" != "no"; then
    aqb_vmajor="`$PKG_CONFIG --variable=vmajor aqbanking`"
    aqb_vminor="`$PKG_CONFIG --variable=vminor aqbanking`"
    aqb_vpatchlevel="`$PKG_CONFIG --variable=vpatchlevel aqbanking`"
    aqb_vstring="`$PKG_CONFIG --variable=vstring aqbanking`"
    aqb_vbuild="`$PKG_CONFIG --variable=vbuild aqbanking`"
    aqb_versionstring="$aqb_vstring.$aqb_vbuild"
    AC_MSG_RESULT([found $aqb_versionstring])
    if test "$vma" -gt "$aqb_vmajor"; then
      AC_MSG_ERROR([Your Aqbanking version is way too old.
      Please update from https://www.aquamaniac.de])
    elif test "$vma" = "$aqb_vmajor"; then
      if test "$vmi" -gt "$aqb_vminor"; then
        AC_MSG_ERROR([Your Aqbanking version is too old.
          Please update from https://www.aquamaniac.de])
      elif test "$vmi" = "$aqb_vminor"; then
          if test "$vpl" -gt "$aqb_vpatchlevel"; then
            AC_MSG_ERROR([Your Aqbanking version is a little bit too old.
            Please update from https://www.aquamaniac.de])
          elif test "$vpl" = "$aqb_vpatchlevel"; then
            if test "$vbld" -gt "$aqb_vbuild"; then
              AC_MSG_ERROR([Your Aqbanking version is a little bit too old.
  Please update to the latest git version. Instructions for accessing
  git can be found on https://www.aquamaniac.de])
             fi
           fi
      fi
    fi
    have_aqbanking="yes"
    #AC_MSG_RESULT(yes)
  else
    have_aqbanking="yes"
    AC_MSG_RESULT(assuming yes)
  fi
dnl end of "if enable-aqbanking"
fi

AC_SUBST(aqhbci_libs)

AC_SUBST(qbanking_libs)
AC_SUBST(qbanking_plugins)
AC_SUBST(qbanking_helpdir)

AC_SUBST(aqbanking_dir)
AC_SUBST(aqbanking_plugins)
AC_SUBST(aqbanking_libs)
AC_SUBST(aqbanking_libspp)
AC_SUBST(aqbanking_data)
AC_SUBST(aqbanking_includes)
])
