# (c) 2008 Martin Preuss<martin@libchipcard.de>
# This function checks for AqFinance

AC_DEFUN([AC_AQFINANCE], [
dnl searches for aqfinance
dnl Arguments: 
dnl   $1: major version minimum
dnl   $2: minor version minimum
dnl   $3: patchlevel version minimum
dnl   $4: build version minimum
dnl Returns: aqfinance_dir
dnl          aqfinance_libs
dnl          aqfinance_data
dnl          aqfinance_plugins
dnl          aqfinance_includes
dnl          have_aqfinance

if test -z "$1"; then vma="0"; else vma="$1"; fi
if test -z "$2"; then vmi="1"; else vmi="$2"; fi
if test -z "$3"; then vpl="0"; else vpl="$3"; fi
if test -z "$4"; then vbld="0"; else vbld="$4"; fi

AC_MSG_CHECKING(if aqfinance support desired)
AC_ARG_ENABLE(aqfinance,
  [  --enable-aqfinance      enable aqfinance support (default=yes)],
  enable_aqfinance="$enableval",
  enable_aqfinance="yes")
AC_MSG_RESULT($enable_aqfinance)

have_aqfinance="no"
aqfinance_dir=""
aqfinance_data=""
aqfinance_plugins=""
aqfinance_libs=""
aqfinance_includes=""

if test "$enable_aqfinance" != "no"; then
  AC_MSG_CHECKING(for aqfinance)
  AC_ARG_WITH(aqfinance-dir, [  --with-aqfinance-dir=DIR
                            uses aqfinance from given dir],
    [lcc_dir="$withval"],
    [lcc_dir="${prefix} \
	     /usr/local \
             /usr \
             /"])

  for li in $lcc_dir; do
      if test -x "$li/bin/aqfinance-config"; then
          aqfinance_dir="$li";
          break
      fi
  done
  if test -z "$aqfinance_dir"; then
      AC_MSG_RESULT([not found ])
      AC_MSG_ERROR([
*** The library AqFinance was not found. Obtain it from 
*** http://www.aquamaniac.de. 
*** If it is already installed (including the -devel package), 
*** you might need to specify the location with the 
*** option --with-aqfinance-dir=DIR.])
  else
      AC_MSG_RESULT($aqfinance_dir)

      AC_MSG_CHECKING(for aqfinance libs)
      aqfinance_libs="`$aqfinance_dir/bin/aqfinance-config --libraries`"
      AC_MSG_RESULT($aqfinance_libs)

      AC_MSG_CHECKING(for aqfinance includes)
      aqfinance_includes="`$aqfinance_dir/bin/aqfinance-config --includes`"
      AC_MSG_RESULT($aqfinance_includes)

      AC_MSG_CHECKING(for aqfinance plugins)
      aqfinance_plugins="`$aqfinance_dir/bin/aqfinance-config --plugins`"
      AC_MSG_RESULT($aqfinance_plugins)

      AC_MSG_CHECKING(for aqfinance data)
      aqfinance_data="`$aqfinance_dir/bin/aqfinance-config --data`"
      AC_MSG_RESULT($aqfinance_data)
  fi
  AC_MSG_CHECKING(if aqfinance test desired)
  AC_ARG_ENABLE(aqfinance,
    [  --enable-aqfinance-test   enable aqfinance-test (default=yes)],
     enable_aqfinance_test="$enableval",
     enable_aqfinance_test="yes")
  AC_MSG_RESULT($enable_aqfinance_test)
  AC_MSG_CHECKING(for AqFinance version >=$vma.$vmi.$vpl.$vbld)
  if test "$enable_aqfinance_test" != "no"; then
    aqfinance_versionstring="`$aqfinance_dir/bin/aqfinance-config --vstring`.`$aqfinance_dir/bin/aqfinance-config --vbuild`"
    AC_MSG_RESULT([found $aqfinance_versionstring])
    if test "$vma" -gt "`$aqfinance_dir/bin/aqfinance-config --vmajor`"; then
      AC_MSG_ERROR([Your AqFinance version is way too old.
      Please update from http://www.aquamaniac.de/aqfinance/])
    elif test "$vma" = "`$aqfinance_dir/bin/aqfinance-config --vmajor`"; then
      if test "$vmi" -gt "`$aqfinance_dir/bin/aqfinance-config --vminor`"; then
        AC_MSG_ERROR([Your AqFinance version is too old.
          Please update from http://www.aquamaniac.de/aqfinance/])
      elif test "$vmi" = "`$aqfinance_dir/bin/aqfinance-config --vminor`"; then
          if test "$vpl" -gt "`$aqfinance_dir/bin/aqfinance-config --vpatchlevel`"; then
            AC_MSG_ERROR([Your AqFinance version is a little bit too old.
            Please update from http://www.aquamaniac.de/aqfinance/])
          elif test "$vpl" = "`$aqfinance_dir/bin/aqfinance-config --vpatchlevel`"; then
            if test "$vbld" -gt "`$aqfinance_dir/bin/aqfinance-config --vbuild`"; then
              AC_MSG_ERROR([Your AqFinance version is a little bit too old. 
  Please update to the latest CVS version. Instructions for accessing 
  CVS can be found on http://www.aquamaniac.de/aqfinance/])
             fi
           fi
      fi
    fi
    have_aqfinance="yes"
    #AC_MSG_RESULT(yes)
  else
    have_aqfinance="yes"
    AC_MSG_RESULT(assuming yes)
  fi
dnl end of "if enable-aqfinance"
fi

AC_SUBST(aqfinance_dir)
AC_SUBST(aqfinance_plugins)
AC_SUBST(aqfinance_libs)
AC_SUBST(aqfinance_data)
AC_SUBST(aqfinance_includes)
])
