# $Id$
# (c) 2002 Martin Preuss<martin@libchipcard.de>
# This function checks for libgwenhywfar

AC_DEFUN([AC_GWENHYWFAR], [
dnl searches for gwenhywfar
dnl Arguments: 
dnl   $1: major version minimum
dnl   $2: minor version minimum
dnl   $3: patchlevel version minimum
dnl   $4: build version minimum
dnl Returns: gwenhywfar_dir
dnl          gwenhywfar_bindir
dnl          gwenhywfar_libs
dnl          gwenhywfar_plugins
dnl          gwenhywfar_includes
dnl          gwenhywfar_headers
dnl          gwenhywfar_has_crypt
dnl          have_gwenhywfar

if test -z "$1"; then vma="0"; else vma="$1"; fi
if test -z "$2"; then vmi="1"; else vmi="$2"; fi
if test -z "$3"; then vpl="0"; else vpl="$3"; fi
if test -z "$4"; then vbld="0"; else vbld="$4"; fi

AC_MSG_CHECKING(if gwenhywfar support desired)
AC_ARG_ENABLE(gwenhywfar,
  [  --enable-gwenhywfar     enable gwenhywfar support (default=yes)],
  enable_gwenhywfar="$enableval",
  enable_gwenhywfar="yes")
AC_MSG_RESULT($enable_gwenhywfar)

have_gwenhywfar="no"
gwenhywfar_dir=""
gwenhywfar_plugins=""
gwenhywfar_libs=""
gwenhywfar_bindir=""
gwenhywfar_libspp=""
gwenhywfar_includes=""
gwenhywfar_has_crypt="yes"
if test "$enable_gwenhywfar" != "no"; then
  AC_MSG_CHECKING(for gwenhywfar)
  AC_ARG_WITH(gwen-dir,
    [  --with-gwen-dir=DIR     obsolete - set PKG_CONFIG_PATH environment variable instead],
    [AC_MSG_RESULT([obsolete configure option '--with-gwen-dir' used])
     AC_MSG_ERROR([
*** Configure switch '--with-gwen-dir' is obsolete.
*** If you want to use gwenhywfar from a non-system location
*** then locate the file 'gwenhywfar.pc' and add its parent directory
*** to environment variable PKG_CONFIG_PATH. For example
*** configure <options> PKG_CONFIG_PATH="<path-to-gwenhywfar.pc's-dir>:\${PKG_CONFIG_PATH}"])],
    [])

  $PKG_CONFIG --exists gwenhywfar
  result=$?
  if test $result -ne 0; then
      AC_MSG_RESULT(not found)
      AC_MSG_ERROR([
*** Package gwenhywfar was not found in the pkg-config search path.
*** Perhaps you should add the directory containing `gwenhywfar.pc'
*** to the PKG_CONFIG_PATH environment variable])
  else
      gwenhywfar_dir="`$PKG_CONFIG --variable=prefix gwenhywfar`"
      AC_MSG_RESULT($gwenhywfar_dir)
  fi

  AC_MSG_CHECKING(for gwen libs)
  gwenhywfar_libs="`$PKG_CONFIG --libs gwenhywfar`"
  AC_MSG_RESULT($gwenhywfar_libs)
  AC_MSG_CHECKING(for gwen includes)
  gwenhywfar_includes="`$PKG_CONFIG --cflags gwenhywfar`"
  AC_MSG_RESULT($gwenhywfar_includes)
  AC_MSG_CHECKING(for gwen binary tools)
  gwenhywfar_bindir="`$PKG_CONFIG --variable=bindir gwenhywfar`"
  AC_MSG_RESULT($gwenhywfar_bindir)
  AC_MSG_CHECKING(for gwen plugins)
  gwenhywfar_plugins="`$PKG_CONFIG --variable=plugindir gwenhywfar`"
  AC_MSG_RESULT($gwenhywfar_plugins)
  AC_MSG_CHECKING(for gwen headers)
  gwenhywfar_headers="`$PKG_CONFIG --variable=headerdir gwenhywfar`"
  AC_MSG_RESULT($gwenhywfar_headers)

  AC_MSG_CHECKING(if gwenhywfar test desired)
  AC_ARG_ENABLE(gwenhywfar,
    [  --enable-gwenhywfar-test   enable gwenhywfar-test (default=yes)],
     enable_gwenhywfar_test="$enableval",
     enable_gwenhywfar_test="yes")
  AC_MSG_RESULT($enable_gwenhywfar_test)
  AC_MSG_CHECKING(for Gwenhywfar version >=$vma.$vmi.$vpl.$vbld)
  if test "$enable_gwenhywfar_test" != "no"; then
    gwen_vmajor="`$PKG_CONFIG --variable=vmajor gwenhywfar`"
    gwen_vminor="`$PKG_CONFIG --variable=vminor gwenhywfar`"
    gwen_vpatchlevel="`$PKG_CONFIG --variable=vpatchlevel gwenhywfar`"
    gwen_vstring="`$PKG_CONFIG --variable=vstring gwenhywfar`"
    gwen_vbuild="`$PKG_CONFIG --variable=vbuild gwenhywfar`"
    gwen_versionstring="$gwen_vstring.$gwen_vbuild"
    AC_MSG_RESULT([found $gwen_versionstring])
    if test "$vma" -gt "$gwen_vmajor"; then
      AC_MSG_ERROR([Your Gwenhywfar version is way too old.
      Please update from https://www.aquamaniac.de])
    elif test "$vma" = "$gwen_vmajor"; then
      if test "$vmi" -gt "$gwen_vminor"; then
        AC_MSG_ERROR([Your Gwenhywfar version is too old.
          Please update from https://www.aquamaniac.de])
      elif test "$vmi" = "$gwen_vminor"; then
          if test "$vpl" -gt "$gwen_vpatchlevel"; then
            AC_MSG_ERROR([Your Gwenhywfar version is a little bit too old.
            Please update from https://www.aquamaniac.de])
          elif test "$vpl" = "$gwen_vpatchlevel"; then
            if test "$vbld" -gt "$gwen_vbuild"; then
              AC_MSG_ERROR([Your Gwenhywfar version is a little bit too old. 
  Please update to the latest git version. Instructions for accessing
  git can be found on https://www.aquamaniac.de])
             fi
           fi
      fi
    fi
    have_gwenhywfar="yes"
    #AC_MSG_RESULT(yes)
  else
    have_gwenhywfar="yes"
    AC_MSG_RESULT(assuming yes)
  fi
  if test -n "$save_path"; then
      export PGK_CONFIG_PATH="$save_path"
  fi
dnl end of "if enable-gwenhywfar"
fi

AC_SUBST(gwenhywfar_dir)
AC_SUBST(gwenhywfar_plugins)
AC_SUBST(gwenhywfar_bindir)
AC_SUBST(gwenhywfar_libs)
AC_SUBST(gwenhywfar_includes)
AC_SUBST(gwenhywfar_headers)
AC_SUBST(gwenhywfar_has_crypt)
])
