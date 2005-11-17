# $Id$
# (c) 2004 Martin Preuss<martin@libchipcard.de>
# This function checks for QBanking

AC_DEFUN([AC_QBANKING], [
dnl searches for qbanking
dnl Arguments: 
dnl   $1: major version minimum
dnl   $2: minor version minimum
dnl   $3: patchlevel version minimum
dnl   $4: build version minimum
dnl Returns: qbanking_dir
dnl          qbanking_libs
dnl          qbanking_data
dnl          qbanking_includes
dnl          have_qbanking

if test -z "$1"; then vma="0"; else vma="$1"; fi
if test -z "$2"; then vmi="1"; else vmi="$2"; fi
if test -z "$3"; then vpl="0"; else vpl="$3"; fi
if test -z "$4"; then vbld="0"; else vbld="$4"; fi

AC_MSG_CHECKING(if qbanking support desired)
AC_ARG_ENABLE(qbanking,
  [  --enable-qbanking       enable qbanking support (default=detect)],
  enable_qbanking="$enableval",
  enable_qbanking="yes")
AC_MSG_RESULT($enable_qbanking)

have_qbanking="no"
qbanking_dir=""
qbanking_data=""
qbanking_libs=""
qbanking_includes=""
if test "$enable_qbanking" != "no"; then
  AC_MSG_CHECKING(for qbanking)
  AC_ARG_WITH(qbanking-dir, [  --with-qbanking-dir=DIR
                          uses qbanking from given dir],
    [lcc_dir="$withval"],
    [lcc_dir="${prefix} \
	     /usr/local \
             /usr \
             /"])

  for li in $lcc_dir; do
      if test -x "$li/bin/qbanking-config"; then
          qbanking_dir="$li";
          break
      fi
  done
  if test -z "$qbanking_dir"; then
      AC_MSG_RESULT([not found ])
      have_qbanking="no"
  else
      AC_MSG_RESULT($qbanking_dir)
      AC_MSG_CHECKING(for qbanking libs)
      qbanking_libs="`$qbanking_dir/bin/qbanking-config --libraries`"
      AC_MSG_RESULT($qbanking_libs)
      AC_MSG_CHECKING(for qbanking includes)
      qbanking_includes="`$qbanking_dir/bin/qbanking-config --includes`"
      AC_MSG_RESULT($qbanking_includes)
      AC_MSG_CHECKING(for qbanking data)
      qbanking_data="`$qbanking_dir/bin/qbanking-config --data`"
      AC_MSG_RESULT($qbanking_data)

      AC_MSG_CHECKING(if qbanking test desired)
      AC_ARG_ENABLE(qbanking,
        [  --enable-qbanking-test  enable qbanking-test (default=yes)],
         enable_qbanking_test="$enableval",
         enable_qbanking_test="yes")
      AC_MSG_RESULT($enable_qbanking_test)
      AC_MSG_CHECKING(for QBanking version >=$vma.$vmi.$vpl.$vbld)
      if test "$enable_qbanking_test" != "no"; then
        qbanking_versionstring="`$qbanking_dir/bin/qbanking-config --vstring`.`$qbanking_dir/bin/qbanking-config --vbuild`"
        AC_MSG_RESULT([found $qbanking_versionstring])
        if test "$vma" -gt "`$qbanking_dir/bin/qbanking-config --vmajor`"; then
          AC_MSG_ERROR([Your QBanking version is way too old.
          Please update from http://www.aquamaniac.de/aqbanking/])
        elif test "$vma" = "`$qbanking_dir/bin/qbanking-config --vmajor`"; then
          if test "$vmi" -gt "`$qbanking_dir/bin/qbanking-config --vminor`"; then
            AC_MSG_ERROR([Your QBanking version is too old.
              Please update from http://www.aquamaniac.de/aqbanking/])
          elif test "$vmi" = "`$qbanking_dir/bin/qbanking-config --vminor`"; then
              if test "$vpl" -gt "`$qbanking_dir/bin/qbanking-config --vpatchlevel`"; then
                AC_MSG_ERROR([Your QBanking version is a little bit too old.
                Please update from http://www.aquamaniac.de/aqbanking/])
              elif test "$vpl" = "`$qbanking_dir/bin/qbanking-config --vpatchlevel`"; then
                if test "$vbld" -gt "`$qbanking_dir/bin/qbanking-config --vbuild`"; then
                  AC_MSG_ERROR([Your QBanking version is a little bit too old. 
      Please update to the latest CVS version. Instructions for accessing 
      CVS can be found on http://www.aquamaniac.de/aqbanking/])
                 fi
               fi
          fi
        fi
        have_qbanking="yes"
        #AC_MSG_RESULT(yes)
        AC_DEFINE_UNQUOTED(HAVE_QBANKING, 1, [Defines if your system has the qbanking package])
      else
        have_qbanking="yes"
        AC_MSG_RESULT(assuming yes)
        AC_DEFINE_UNQUOTED(HAVE_QBANKING, 1, [Defines if your system has the qbanking package])
      fi
   fi
dnl end of "if enable-qbanking"
fi

AC_SUBST(qbanking_dir)
AC_SUBST(qbanking_libs)
AC_SUBST(qbanking_data)
AC_SUBST(qbanking_includes)
])
