# $Id$
# (c) 2002 Martin Preuss<martin@libchipcard.de>
# These functions guess your operation system

AC_DEFUN(AQ_CHECK_OS,[
dnl IN: 
dnl   - AC_CANONICAL_SYSTEM muste be called before
dnl OUT:
dnl   Variables:
dnl     OSYSTEM: Short name of your system (subst)
dnl     OS_TYPE: either "posix" or "windows" (subst)
dnl     MAKE_DLL_TARGET: under windows this is set to "dll" (subst)
dnl     INSTALL_DLL_TARGET: under Windows this is set to "dll-install" (subst)
dnl   Defines:
dnl     OS_NAME: full name of your system
dnl     OS_SHORTNAME: short name of your system
dnl     Depending on your system one of the following is defined in addition:
dnl      OS_LINUX, OS_OPENBSD, OS_FREEBSD, OS_BEOS, OS_WIN32

# check for OS
AC_MSG_CHECKING([target system type])
OSYSTEM=""
OS_TYPE=""
MAKE_DLL_TARGET=""
INSTALL_DLL_TARGET=""
AC_DEFINE_UNQUOTED(OS_NAME,"$target", [target system])
case "$target" in
    *-linux*)
	OSYSTEM="linux"
	AC_DEFINE(OS_LINUX,1,[if linux is used])
	OS_TYPE="posix"
	;;
    *-openbsd*)
	OSYSTEM="openbsd"
	AC_DEFINE(OS_OPENBSD,1,[if OpenBSD is used])
	OS_TYPE="posix"
	;;
    *-freebsd*)
	OSYSTEM="freebsd"
	AC_DEFINE(OS_FREEBSD,1,[if FreeBSD is used])
	OS_TYPE="posix"
	;;
    *-beos*)
	OSYSTEM="beos"
	AC_DEFINE(OS_BEOS,1,[if BeOS is used])
	OS_TYPE="posix"
	;;
    *-win32*)
    	OSYSTEM="windows"
	AC_DEFINE(OS_WIN32,1,[if WIN32 is used])
	OS_TYPE="windows"
        AC_DEFINE_UNQUOTED(BUILDING_DLL,1,[if DLL is to be built])
	MAKE_DLL_TARGET="dll"
	INSTALL_DLL_TARGET="dll-install"
	;;
    *-mingw32*)
	OSYSTEM="windows"
	AC_DEFINE(OS_WIN32,1,[if WIN32 is used])
	OS_TYPE="windows"
        AC_DEFINE_UNQUOTED(BUILDING_DLL,1,[if DLL is to be built])
	MAKE_DLL_TARGET="dll"
	INSTALL_DLL_TARGET="dll-install"
	;;
    *)
	AC_MSG_WARN([Sorry, but target $target is not supported.
        Please report if it works anyway. We will assume that your system
        is a posix system and continue.])
	OSYSTEM="unknown"
	OS_TYPE="posix"
	;;
esac

AC_SUBST(OSYSTEM)
AC_DEFINE_UNQUOTED(OS_SHORTNAME,"$OSYSTEM",[target system])
AC_SUBST(OS_TYPE)
AC_DEFINE_UNQUOTED(OS_TYPE,"$OS_TYPE",[system type])
AC_SUBST(MAKE_DLL_TARGET)
AC_SUBST(INSTALL_DLL_TARGET)

AC_MSG_RESULT($OS_TYPE)
])


