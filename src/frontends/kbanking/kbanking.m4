# $Id$
# (c) 2004 Martin Preuss<martin@libchipcard.de>
# This function checks for KBanking

AC_DEFUN([AC_KBANKING], [
dnl searches for kbanking
dnl Arguments: 
dnl   $1: major version minimum
dnl   $2: minor version minimum
dnl   $3: patchlevel version minimum
dnl   $4: build version minimum
dnl Returns: kbanking_dir
dnl          kbanking_libs
dnl          kbanking_data
dnl          kbanking_includes
dnl          have_kbanking

if test -z "$1"; then vma="0"; else vma="$1"; fi
if test -z "$2"; then vmi="1"; else vmi="$2"; fi
if test -z "$3"; then vpl="0"; else vpl="$3"; fi
if test -z "$4"; then vbld="0"; else vbld="$4"; fi

AC_MSG_CHECKING(if kbanking support desired)
AC_ARG_ENABLE(kbanking,
  [  --enable-kbanking       enable kbanking support (default=detect)],
  enable_kbanking="$enableval",
  enable_kbanking="yes")
AC_MSG_RESULT($enable_kbanking)

have_kbanking="no"
kbanking_dir=""
kbanking_data=""
kbanking_libs=""
kbanking_includes=""
if test "$enable_kbanking" != "no"; then
  AC_MSG_CHECKING(for kbanking)
  AC_ARG_WITH(kbanking-dir, [  --with-kbanking-dir=DIR
                          uses kbanking from given dir],
    [lcc_dir="$withval"],
    [lcc_dir="${prefix} \
	     /usr/local \
             /usr \
             /"])

  for li in $lcc_dir; do
      if test -x "$li/bin/kbanking-config"; then
          kbanking_dir="$li";
          break
      fi
  done
  if test -z "$kbanking_dir"; then
      AC_MSG_RESULT([not found ])
      have_kbanking="no"
  else
      AC_MSG_RESULT($kbanking_dir)
      AC_MSG_CHECKING(for kbanking libs)
      kbanking_libs="`$kbanking_dir/bin/kbanking-config --libraries`"
      AC_MSG_RESULT($kbanking_libs)
      AC_MSG_CHECKING(for kbanking includes)
      kbanking_includes="`$kbanking_dir/bin/kbanking-config --includes`"
      AC_MSG_RESULT($kbanking_includes)
      AC_MSG_CHECKING(for kbanking data)
      kbanking_data="`$kbanking_dir/bin/kbanking-config --data`"
      AC_MSG_RESULT($kbanking_data)

      AC_MSG_CHECKING(if kbanking test desired)
      AC_ARG_ENABLE(kbanking,
        [  --enable-kbanking-test  enable kbanking-test (default=yes)],
         enable_kbanking_test="$enableval",
         enable_kbanking_test="yes")
      AC_MSG_RESULT($enable_kbanking_test)
      AC_MSG_CHECKING(for KBanking version >=$vma.$vmi.$vpl.$vbld)
      if test "$enable_kbanking_test" != "no"; then
        kbanking_versionstring="`$kbanking_dir/bin/kbanking-config --vstring`.`$kbanking_dir/bin/kbanking-config --vbuild`"
        AC_MSG_RESULT([found $kbanking_versionstring])
        if test "$vma" -gt "`$kbanking_dir/bin/kbanking-config --vmajor`"; then
          AC_MSG_ERROR([Your KBanking version is way too old.
          Please update from http://www.aquamaniac.de/kbanking/])
        elif test "$vma" = "`$kbanking_dir/bin/kbanking-config --vmajor`"; then
          if test "$vmi" -gt "`$kbanking_dir/bin/kbanking-config --vminor`"; then
            AC_MSG_ERROR([Your KBanking version is too old.
              Please update from http://www.aquamaniac.de/kbanking/])
          elif test "$vmi" = "`$kbanking_dir/bin/kbanking-config --vminor`"; then
              if test "$vpl" -gt "`$kbanking_dir/bin/kbanking-config --vpatchlevel`"; then
                AC_MSG_ERROR([Your KBanking version is a little bit too old.
                Please update from http://www.aquamaniac.de/kbanking/])
              elif test "$vpl" = "`$kbanking_dir/bin/kbanking-config --vpatchlevel`"; then
                if test "$vbld" -gt "`$kbanking_dir/bin/kbanking-config --vbuild`"; then
                  AC_MSG_ERROR([Your KBanking version is a little bit too old. 
      Please update to the latest CVS version. Instructions for accessing 
      CVS can be found on http://www.aquamaniac.de/kbanking/])
                 fi
               fi
          fi
        fi
        have_kbanking="yes"
        #AC_MSG_RESULT(yes)
        AC_DEFINE_UNQUOTED(HAVE_KBANKING, 1, [Defines if your system has the kbanking package])
      else
        have_kbanking="yes"
        AC_MSG_RESULT(assuming yes)
        AC_DEFINE_UNQUOTED(HAVE_KBANKING, 1, [Defines if your system has the kbanking package])
      fi
   fi
dnl end of "if enable-kbanking"
fi

AC_SUBST(kbanking_dir)
AC_SUBST(kbanking_libs)
AC_SUBST(kbanking_data)
AC_SUBST(kbanking_includes)
])
