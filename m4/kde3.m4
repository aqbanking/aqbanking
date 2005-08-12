# $Id$
# (c) 2002 Martin Preuss<martin@libchipcard.de>
# These functions search for KDE 3


AC_DEFUN([AQ_CHECK_KDE3],[
dnl PREREQUISITES:
dnl   none
dnl IN: 
dnl   $1 = "yes" if KDE3 is needed, "no" if KDE3 is optional
dnl   $2 = subdirs to include when KDE3 is available
dnl   You may preset the return variables.
dnl   All variables which already have a value will not be altered
dnl OUT:
dnl   Variables:
dnl     have_kde3 - set to "yes" if KDE3 exists
dnl     kde3_includes - CFLAGS for includes (-I)
dnl     kde3_include_dir - path to includes
dnl     kde3_libs - LDFLAGS for linking the kde3 libraries (-L)
dnl     kde3_app - kde3 apps given as the argument to this funtion
dnl     kde3_install_dir - install directory for kde3 applications
dnl   Defines:
dnl     HAVE_KDE3
dnl USED MACROS:
dnl  AQ_CHECK_QT3


kde3_local_lforce="$1"
kde3_local_lsd="$2"

dnl check if kde apps are desired
AC_MSG_CHECKING(if KDE3 is allowed)
AC_ARG_ENABLE(kde3,
  [  --enable-kde3         enable kde3 (default=yes)],
  enable_kde3="$enableval",
  enable_kde3="yes")
AC_MSG_RESULT($enable_kde3)

if test "$enable_kde3" = "no"; then
   kde3_libs=""
   kde3_includes=""
   kde3_include_dir=""
   kde3_app=""
   kde3_install_dir=""
else


dnl check for QT3
AQ_CHECK_QT3("$kde3_local_lforce")
AC_MSG_CHECKING(if qt3 version is fully useable)
if test "$have_qt3" != "yes"; then
   	kde3_libs=""
   	kde3_includes=""
   	kde3_include_dir=""
   	kde3_app=""
   	kde3_install_dir=""
	AC_MSG_RESULT(no, so no KDE3 without qt3)
else
	AC_MSG_RESULT(yes)


dnl paths for kde install dir
AC_MSG_CHECKING(for kde3 install dir)
AC_ARG_WITH(kde3-prefix, 
  [  --with-kde3-prefix=DIR      installs kde3 apps to the given dir],
  [local_kde3_prefix="$withval"],
  [local_kde3_prefix="\
  	$KDEDIR \
        /usr/local \
        /usr \
        /opt/kde3 \
        /opt/kde \
        "
  ]
)
if test -z "$kde3_install_dir"; then
	for i in $local_kde3_prefix; do
		if test -z "$kde3_install_dir"; then
        	   if test -x "$i/bin/kde-config"; then
                      	$i/bin/kde-config --prefix &>/dev/null && \
                          kde3_install_dir="`$i/bin/kde-config --prefix`"
                   fi
 		fi
        done
fi

if test -z "$kde3_install_dir"; then
      	AC_MSG_RESULT(not found)
else
      	AC_MSG_RESULT($kde3_install_dir)
fi


dnl paths for kde includes
AC_MSG_CHECKING(for kde3 includes)
AC_ARG_WITH(kde3-includes, 
  [  --with-kde3-includes=DIR      uses kde3 includes from given dir],
  [local_kde3_includes="$withval"],
  [local_kde3_includes="\
  	$KDEDIR/include \
        /usr/include/kde3 \
        /usr/local/include/kde3 \
        /usr/include/kde \
        /usr/local/include/kde \
        /opt/kde3/include \
        /opt/kde/include \
        /usr/include \
        /usr/local/include \
        "
  ]
)

if test -z "$kde3_include_dir"; then
	for i in $local_kde3_includes; do
		if test -z "$kde3_include_dir"; then
        		if test -r "$i/kdeversion.h"; then
                        	tmp=`grep "KDE_VERSION_MAJOR 3" "$i/kdeversion.h"`
                                if test -n "$tmp"; then
                                        kde3_include_dir="$i"
                                fi
                	fi
 		fi
        done
fi
if test -n "$kde3_include_dir"; then
	kde3_includes="-I$kde3_include_dir"
	AC_MSG_RESULT($kde3_include_dir)
else
	AC_MSG_RESULT(not found)
fi


# Check for x86_64 architecture; potentially set lib-directory suffix
if test "$target_cpu" = "x86_64"; then
  libdirsuffix="64"
else
  libdirsuffix=""
fi

dnl paths for kde libs
AC_MSG_CHECKING(for kde3 libraries)
AC_ARG_WITH(kde3-libs, 
  [  --with-kde3-libs=DIR      uses kde3 libs from given dir],
  [local_kde3_libs="$withval"],
  [local_kde3_libs="\
  	$KDEDIR/lib${libdirsuffix} \
        /usr/lib/kde3 \
        /usr/local/lib/kde3 \
        /usr/lib/kde \
        /usr/local/lib/kde \
        /opt/kde3/lib${libdirsuffix} \
        /opt/kde/lib${libdirsuffix} \
        /usr/lib${libdirsuffix} \
        /usr/local/lib${libdirsuffix} \
  	$KDEDIR/lib \
        "
  ]
)

if test -z "$kde3_libs"; then
	AQ_SEARCH_FOR_PATH([libkdeui.so.4],[$local_kde3_libs])
       	if test -n "$found_dir" ; then
       		kde3_libs="-L$found_dir"
       	fi
fi
if test -n "$kde3_libs"; then
      	AC_MSG_RESULT($kde3_libs)
else
       	AC_MSG_RESULT(not found)
fi


# check if all necessary kde components where found
if test -z "$kde3_includes" || \
   test -z "$kde3_install_dir" || \
   test -z "$kde3_libs"; then
	kde3_libs=""
   	kde3_includes=""
   	kde3_app=""
   	have_kde3="no"
   	if test "$kde3_local_lforce" = "yes"; then
        	AC_MSG_WARN([
 Compilation of KDE3 applications is enabled but I could not find some KDE3
 components (see which are missing in messages above).
 If you don't want to compile KDE3 applications please use "--disable-kde3".
 ])
   	else
        	AC_MSG_WARN([
 KDE3 is not explicitly disabled and I could not find some KDE3 components 
 (see which are missing in messages above).
 If you don't want to compile KDE3 applications please use "--disable-kde3".
 ])
   	fi
else
dnl TODO: AC_TRY_RUN, check whether kdeversion.h has matching versions
   kde3_app="$kde3_local_lsd"
   have_kde3="yes"
   AC_DEFINE(HAVE_KDE3, 1, [if KDE3 is available])
fi


dnl end of if QT3 is useable
fi

dnl end of if "$enable_kdeapps"
fi

AS_SCRUB_INCLUDE(kde3_includes)
AC_SUBST(kde3_app)
AC_SUBST(kde3_libs)
AC_SUBST(kde3_includes)
AC_SUBST(kde3_include_dir)
AC_SUBST(kde3_install_dir)

])



