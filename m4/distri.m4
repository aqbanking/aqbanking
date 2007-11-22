
# distri.m4
# (c) 2006 Martin Preuss<martin@libchipcard.de>
# This function checks for the Linux distribution

AC_DEFUN([AQ_DISTRIBUTION], [
dnl searches for libusb
dnl Arguments: none
dnl Returns:   aq_distrib_name (name of the Linux distribution)
dnl            aq_distrib_tag  (tag for packages, like "suse")
dnl            aq_distrib_version (not for Debian-like systems)

AC_MSG_CHECKING(for preselected distribution name)
if test -n "$aq_distrib_name"; then
  AC_MSG_RESULT([yes, $aq_distrib_name])
else
  AC_MSG_RESULT([no, will have to determine it])
  aq_distrib_name=""
  aq_distrib_version=""
  aq_distrib_tag=""
  
  # Debian-style

  AC_MSG_CHECKING(whether this is a Debian derivate)
  case `basename $CC` in 
    gcc | gcc-* | *-gcc | *-gcc-*)
       debtype=["` ${CC} --version | head -1`"]
       case $debtype in
         *\(Debian\ *)
           aq_distrib_name="Debian"
           aq_distrib_tag="debian"
           AC_MSG_RESULT([yes, $aq_distrib_name])
           ;;
         *\(Ubuntu\ * | *\(KUbuntu\ *)
           aq_distrib_name="Ubuntu"
           aq_distrib_tag="ubuntu"
           AC_MSG_RESULT([yes, $aq_distrib_name])
           ;;
         *)
           AC_MSG_RESULT([no])
           ;;
        esac
        ;;
    *)
      AC_MSG_RESULT([no])
      ;;
  esac
fi


if test -z "$aq_distrib_name"; then

  # RPM-style

  AC_MSG_CHECKING(whether this is an RPM distribution)
  RPM_COMMAND="`which rpm`"
  aq_distrib_name=""
  aq_distrib_tag=""
  aq_distrib_version=""
  if test -n "${RPM_COMMAND}"; then
    if test -x "${RPM_COMMAND}"; then
      if test -e "/etc/mandriva-release"; then
        aq_distrib_name="mandriva"
        aq_distrib_tag="mdk"
        aq_distrib_version="`rpm -q --queryformat='%{VERSION}' mandriva-release 2>/dev/null`"
        AC_MSG_RESULT([yes, $aq_distrib_name])
      elif test -e "/etc/mandrake-release"; then
        aq_distrib_name="mandrake"
        aq_distrib_tag="mdk"
        aq_distrib_version="`rpm -q --queryformat='%{VERSION}' mandrake-release 2>/dev/null`"
        AC_MSG_RESULT([yes, $aq_distrib_name])
      elif test -e "/etc/SuSE-release"; then
        aq_distrib_name="suse"
        aq_distrib_tag="suse"
        read v1 v2 v3 v4 </etc/SuSE-release
        case "$v2" in 
          *.*)
            aq_distrib_version=$v2
            ;;
          *)
            aq_distrib_version=$v3
            ;;
        esac
        AC_MSG_RESULT([yes, $aq_distrib_name])
      elif test -e "/etc/fedora-release"; then
        aq_distrib_name="fedora"
        aq_distrib_tag="fc"
        aq_distrib_version="`rpm -q --queryformat='%{VERSION}' fedora-release 2>/dev/null`"
        AC_MSG_RESULT([yes, $aq_distrib_name])
      else
        AC_MSG_RESULT([no (RPM found, but unknown distribution)])
      fi
    else
      AC_MSG_RESULT([no (RPM not found)])
    fi
  else
    AC_MSG_RESULT([no (no RPM installed)])
  fi
fi

AC_SUBST(aq_distrib_name)
AC_SUBST(aq_distrib_tag)
AC_SUBST(aq_distrib_version)

])


