# Process this file with autoconf to produce a configure script.

###-------------------------------------------------------------------------
#
# The following line makes sure that this file is always touched when a new
# revision is checked out.
#
# $Rev: 2550 $
#


AC_PREREQ(2.56)
AC_INIT
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AC_CONFIG_SRCDIR([README])
AC_CONFIG_HEADERS([config.h])


###-------------------------------------------------------------------------
#
# AqBanking versions
#

AQBANKING_VERSION_MAJOR=5
AQBANKING_VERSION_MINOR=5
AQBANKING_VERSION_PATCHLEVEL=1
AQBANKING_VERSION_BUILD=0
dnl "stable", "rcX", "betaX", "svn"
AQBANKING_VERSION_TAG="stable"



###-------------------------------------------------------------------------
#
# SO versions
#

#
# AqBanking
#

AQBANKING_SO_CURRENT=39
AQBANKING_SO_AGE=5
AQBANKING_SO_REVISION=3
AQBANKING_SO_EFFECTIVE="`echo \$(($AQBANKING_SO_CURRENT-$AQBANKING_SO_AGE))`"

#
# backends
#

AQHBCI_SO_CURRENT=22
AQHBCI_SO_AGE=0
AQHBCI_SO_REVISION=0
AQHBCI_SO_EFFECTIVE="`echo \$(($AQHBCI_SO_CURRENT-$AQHBCI_SO_AGE))`"

AQOFXCONNECT_SO_CURRENT=7
AQOFXCONNECT_SO_AGE=0
AQOFXCONNECT_SO_REVISION=2
AQOFXCONNECT_SO_EFFECTIVE="`echo \$(($AQOFXCONNECT_SO_CURRENT-$AQOFXCONNECT_SO_AGE))`"


AQEBICS_SO_CURRENT=2
AQEBICS_SO_AGE=1
AQEBICS_SO_REVISION=9
AQEBICS_SO_EFFECTIVE="`echo \$(($AQEBICS_SO_CURRENT-$AQEBICS_SO_AGE))`"



###-------------------------------------------------------------------------
#
# Generate version strings and BUILD
#

SVN="svn"
AQBANKING_VERSION_FULL_STRING="${AQBANKING_VERSION_MAJOR}.${AQBANKING_VERSION_MINOR}.${AQBANKING_VERSION_PATCHLEVEL}.${AQBANKING_VERSION_BUILD}${AQBANKING_VERSION_TAG}"
AQBANKING_VERSION_STRING="$AQBANKING_VERSION_MAJOR.$AQBANKING_VERSION_MINOR.$AQBANKING_VERSION_PATCHLEVEL"

case "$AQBANKING_VERSION_TAG" in 
  svn|cvs)
     rev=`LC_ALL="C" $SVN info|grep 'Last Changed Rev'|cut -d' ' -f4`
     if test "x$rev" != "x" ; then
       AQBANKING_VERSION_FULL_STRING="$AQBANKING_VERSION_MAJOR.$AQBANKING_VERSION_MINOR.${AQBANKING_VERSION_PATCHLEVEL}${AQBANKING_VERSION_TAG}-r${rev}"
     fi
     ;;
     
  git)
     AQBANKING_VERSION_FULL_STRING="$AQBANKING_VERSION_MAJOR.$AQBANKING_VERSION_MINOR.${AQBANKING_VERSION_PATCHLEVEL}.${AQBANKING_VERSION_BUILD}"
     ;;
esac



###-------------------------------------------------------------------------
#
# Versions (all versions are now linked to AqBanking versions, so the lines
# here don't have to be changed)
#
AQHBCI_VERSION_MAJOR=$AQBANKING_VERSION_MAJOR
AQHBCI_VERSION_MINOR=$AQBANKING_VERSION_MINOR
AQHBCI_VERSION_PATCHLEVEL=$AQBANKING_VERSION_PATCHLEVEL
AQHBCI_VERSION_BUILD=$AQBANKING_VERSION_BUILD
AQHBCI_VERSION_TAG=$AQBANKING_VERSION_TAG
AQHBCI_VERSION_FULL_STRING="$AQBANKING_VERSION_FULL_STRING"
AQHBCI_VERSION_STRING="$AQBANKING_VERSION_STRING"

AQOFXCONNECT_VERSION_MAJOR=$AQBANKING_VERSION_MAJOR
AQOFXCONNECT_VERSION_MINOR=$AQBANKING_VERSION_MINOR
AQOFXCONNECT_VERSION_PATCHLEVEL=$AQBANKING_VERSION_PATCHLEVEL
AQOFXCONNECT_VERSION_BUILD=$AQBANKING_VERSION_BUILD
AQOFXCONNECT_VERSION_TAG=$AQBANKING_VERSION_TAG
AQOFXCONNECT_VERSION_FULL_STRING="$AQBANKING_VERSION_FULL_STRING"
AQOFXCONNECT_VERSION_STRING="$AQBANKING_VERSION_STRING"

AQEBICS_VERSION_MAJOR=$AQBANKING_VERSION_MAJOR
AQEBICS_VERSION_MINOR=$AQBANKING_VERSION_MINOR
AQEBICS_VERSION_PATCHLEVEL=$AQBANKING_VERSION_PATCHLEVEL
AQEBICS_VERSION_BUILD=$AQBANKING_VERSION_BUILD
AQEBICS_VERSION_TAG=$AQBANKING_VERSION_TAG
AQEBICS_VERSION_FULL_STRING="$AQBANKING_VERSION_FULL_STRING"
AQEBICS_VERSION_STRING="$AQBANKING_VERSION_STRING"




###-------------------------------------------------------------------------
#
# datetime
#
DATETIME="`date +\"%Y%m%d%H%M%S\"`"
AC_SUBST(DATETIME)


###-------------------------------------------------------------------------
#
# Create release strings
#
case "$AQBANKING_VERSION_TAG" in
  cvs|svn)
     AQBANKING_VERSION_RELEASE_STRING="$AQBANKING_VERSION_MAJOR.$AQBANKING_VERSION_MINOR.$AQBANKING_VERSION_PATCHLEVEL.r${AQBANKING_VERSION_BUILD}"
     ;;
  git)
     AQBANKING_VERSION_RELEASE_STRING="$AQBANKING_VERSION_MAJOR.$AQBANKING_VERSION_MINOR.$AQBANKING_VERSION_PATCHLEVEL.${AQBANKING_VERSION_BUILD}-${AQBANKING_VERSION_TAG}"
     ;;
  stable)
     AQBANKING_VERSION_RELEASE_STRING="$AQBANKING_VERSION_MAJOR.$AQBANKING_VERSION_MINOR.$AQBANKING_VERSION_PATCHLEVEL"
     ;;
  *)
     AQBANKING_VERSION_RELEASE_STRING="$AQBANKING_VERSION_MAJOR.$AQBANKING_VERSION_MINOR.$AQBANKING_VERSION_PATCHLEVEL"
     
     # add TAG
     AQBANKING_VERSION_RELEASE_STRING="${AQBANKING_VERSION_RELEASE_STRING}${AQBANKING_VERSION_TAG}"
     ;;
esac

AQHBCI_VERSION_RELEASE_STRING="$AQBANKING_RELEASE_STRING"
AQOFXCONNECT_VERSION_RELEASE_STRING="$AQBANKING_RELEASE_STRING"
AQEBICS_VERSION_RELEASE_STRING="$AQBANKING_RELEASE_STRING"



###-------------------------------------------------------------------------
#
# SUBST and DEFINE versions
#
AC_SUBST(AQBANKING_VERSION_MAJOR)
AC_SUBST(AQBANKING_VERSION_MINOR)
AC_SUBST(AQBANKING_VERSION_PATCHLEVEL)
AC_SUBST(AQBANKING_VERSION_BUILD)
AC_SUBST(AQBANKING_VERSION_TAG)
AC_SUBST(AQBANKING_VERSION_FULL_STRING)
AC_SUBST(AQBANKING_VERSION_STRING)
AC_SUBST(AQBANKING_VERSION_RELEASE_STRING)

AC_SUBST(AQBANKING_SO_CURRENT)
AC_SUBST(AQBANKING_SO_REVISION)
AC_SUBST(AQBANKING_SO_AGE)
AC_SUBST(AQBANKING_SO_EFFECTIVE)
AC_DEFINE_UNQUOTED(AQBANKING_SO_EFFECTIVE_STR, "$AQBANKING_SO_EFFECTIVE",
                   [effective SO version])

AM_INIT_AUTOMAKE(aqbanking, $AQBANKING_VERSION_RELEASE_STRING)


#
# same for AqHBCI
#
AC_SUBST(AQHBCI_VERSION_MAJOR)
AC_SUBST(AQHBCI_VERSION_MINOR)
AC_SUBST(AQHBCI_VERSION_PATCHLEVEL)
AC_SUBST(AQHBCI_VERSION_BUILD)
AC_SUBST(AQHBCI_VERSION_TAG)
AC_SUBST(AQHBCI_VERSION_FULL_STRING)
AC_SUBST(AQHBCI_VERSION_STRING)
AC_SUBST(AQHBCI_VERSION_RELEASE_STRING)

AC_DEFINE_UNQUOTED(AQHBCI_VERSION_MAJOR,$AQHBCI_VERSION_MAJOR, 
                   [major version])
AC_DEFINE_UNQUOTED(AQHBCI_VERSION_MINOR,$AQHBCI_VERSION_MINOR, 
                   [minor version])
AC_DEFINE_UNQUOTED(AQHBCI_VERSION_PATCHLEVEL,$AQHBCI_VERSION_PATCHLEVEL, 
                   [patchlevel])
AC_DEFINE_UNQUOTED(AQHBCI_VERSION_BUILD,$AQHBCI_VERSION_BUILD, [build])
AC_DEFINE_UNQUOTED(AQHBCI_VERSION_STRING,"$AQHBCI_VERSION_STRING", 
                   [version string])
AC_DEFINE_UNQUOTED(AQHBCI_VERSION_FULL_STRING,"$AQHBCI_VERSION_FULL_STRING",
                   [full version string])
AC_DEFINE_UNQUOTED(AQHBCI_VERSION_TAG,$AQHBCI_VERSION_TAG, [tag])

AC_SUBST(AQHBCI_SO_CURRENT)
AC_SUBST(AQHBCI_SO_REVISION)
AC_SUBST(AQHBCI_SO_AGE)
AC_SUBST(AQHBCI_SO_EFFECTIVE)
AC_DEFINE_UNQUOTED(AQHBCI_SO_EFFECTIVE_STR, "$AQHBCI_SO_EFFECTIVE",
                   [effective SO version])


#
# same for AqOFXCONNECT
#

AC_SUBST(AQOFXCONNECT_VERSION_MAJOR)
AC_SUBST(AQOFXCONNECT_VERSION_MINOR)
AC_SUBST(AQOFXCONNECT_VERSION_PATCHLEVEL)
AC_SUBST(AQOFXCONNECT_VERSION_BUILD)
AC_SUBST(AQOFXCONNECT_VERSION_TAG)
AC_SUBST(AQOFXCONNECT_VERSION_FULL_STRING)
AC_SUBST(AQOFXCONNECT_VERSION_STRING)
AC_SUBST(AQOFXCONNECT_VERSION_RELEASE_STRING)

AC_DEFINE_UNQUOTED(AQOFXCONNECT_VERSION_MAJOR,$AQOFXCONNECT_VERSION_MAJOR, 
                   [major version])
AC_DEFINE_UNQUOTED(AQOFXCONNECT_VERSION_MINOR,$AQOFXCONNECT_VERSION_MINOR, 
                   [minor version])
AC_DEFINE_UNQUOTED(AQOFXCONNECT_VERSION_PATCHLEVEL,$AQOFXCONNECT_VERSION_PATCHLEVEL, 
                   [patchlevel])
AC_DEFINE_UNQUOTED(AQOFXCONNECT_VERSION_BUILD,$AQOFXCONNECT_VERSION_BUILD, [build])
AC_DEFINE_UNQUOTED(AQOFXCONNECT_VERSION_STRING,"$AQOFXCONNECT_VERSION_STRING", 
                   [version string])
AC_DEFINE_UNQUOTED(AQOFXCONNECT_VERSION_FULL_STRING,"$AQOFXCONNECT_VERSION_FULL_STRING",
                   [full version string])
AC_DEFINE_UNQUOTED(AQOFXCONNECT_VERSION_TAG,$AQOFXCONNECT_VERSION_TAG, [tag])

AC_SUBST(AQOFXCONNECT_SO_CURRENT)
AC_SUBST(AQOFXCONNECT_SO_REVISION)
AC_SUBST(AQOFXCONNECT_SO_AGE)
AC_SUBST(AQOFXCONNECT_SO_EFFECTIVE)
AC_DEFINE_UNQUOTED(AQOFXCONNECT_SO_EFFECTIVE_STR, "$AQOFXCONNECT_SO_EFFECTIVE",
                   [effective SO version])


#
# same for AqEBICS
#
AC_SUBST(AQEBICS_VERSION_MAJOR)
AC_SUBST(AQEBICS_VERSION_MINOR)
AC_SUBST(AQEBICS_VERSION_PATCHLEVEL)
AC_SUBST(AQEBICS_VERSION_BUILD)
AC_SUBST(AQEBICS_VERSION_TAG)
AC_SUBST(AQEBICS_VERSION_FULL_STRING)
AC_SUBST(AQEBICS_VERSION_STRING)
AC_SUBST(AQEBICS_VERSION_RELEASE_STRING)

AC_DEFINE_UNQUOTED(AQEBICS_VERSION_MAJOR,$AQEBICS_VERSION_MAJOR, 
                   [major version])
AC_DEFINE_UNQUOTED(AQEBICS_VERSION_MINOR,$AQEBICS_VERSION_MINOR, 
                   [minor version])
AC_DEFINE_UNQUOTED(AQEBICS_VERSION_PATCHLEVEL,$AQEBICS_VERSION_PATCHLEVEL, 
                   [patchlevel])
AC_DEFINE_UNQUOTED(AQEBICS_VERSION_BUILD,$AQEBICS_VERSION_BUILD, [build])
AC_DEFINE_UNQUOTED(AQEBICS_VERSION_STRING,"$AQEBICS_VERSION_STRING", 
                   [version string])
AC_DEFINE_UNQUOTED(AQEBICS_VERSION_FULL_STRING,"$AQEBICS_VERSION_FULL_STRING",
                   [full version string])
AC_DEFINE_UNQUOTED(AQEBICS_VERSION_TAG,$AQEBICS_VERSION_TAG, [tag])

AC_SUBST(AQEBICS_SO_CURRENT)
AC_SUBST(AQEBICS_SO_REVISION)
AC_SUBST(AQEBICS_SO_AGE)
AC_SUBST(AQEBICS_SO_EFFECTIVE)
AC_DEFINE_UNQUOTED(AQEBICS_SO_EFFECTIVE_STR, "$AQEBICS_SO_EFFECTIVE",
                   [effective SO version])




###-------------------------------------------------------------------------
#
# determine operating system
#
AQ_CHECK_OS


###-------------------------------------------------------------------------
#
# prefix handling
#
AC_PREFIX_DEFAULT(/usr)
if test "x$prefix" = "xNONE"; then
  prefix=$ac_default_prefix
  ac_configure_args="$ac_configure_args --prefix $prefix"
fi
AC_SUBST(prefix)


###-------------------------------------------------------------------------
#
# static/shared handling
#
AC_ENABLE_STATIC(no)
AC_ENABLE_SHARED(yes)



###-------------------------------------------------------------------------
#
# define some needed variables
#
abs_top_builddir="`pwd`"
AC_SUBST(abs_top_builddir)


###-------------------------------------------------------------------------
#
# Enable some GNU extensions if using the GNU C library
#

AC_GNU_SOURCE



###-------------------------------------------------------------------------
#
# Checks for programs.
#
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL

AC_PROG_LIBTOOL
AC_LIBTOOL_WIN32_DLL
AC_LIBTOOL_RC
# Newer libtool doesn't have AC_LIBTOOL_RC anymore.  Need to check for
# $(RC) by ourselves.
AC_CHECK_TOOL([RC], [windres], [no])
AC_CHECK_PROG(USE_DOT,dot,YES,NO)
AC_CHECK_PROG(SED,sed,sed)

# Checks for libraries.

###-------------------------------------------------------------------------
#
# Checks for header files.
#
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h locale.h])
AC_CHECK_HEADERS([termios.h signal.h iconv.h])
AC_CHECK_DECLS([sigprocmask], [], [], [
  #ifdef HAVE_SIGNAL_H
  #  include <signal.h>
  #endif
])



###-------------------------------------------------------------------------
#
# Checks for typedefs, structures, and compiler characteristics.
#
AC_C_CONST
AC_STRUCT_TM



###-------------------------------------------------------------------------
#
# Checks for library functions.
#
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([memmove memset strcasecmp strdup strerror snprintf])
AC_CHECK_FUNCS([setlocale])



###-------------------------------------------------------------------------
#
# Backends. Those do not have a required ordering.
#

AC_ARG_WITH(backends, 
  [  --with-backends=list      space separated list of backends (default:all)],
  [aqbanking_backends="$withval"],
  [aqbanking_backends="all"]
)

if test "$aqbanking_backends" = "all"; then
  aqbanking_backends="aqhbci aqnone aqofxconnect aqebics"
fi

for f in ${aqbanking_backends}; do
  case "$f" in
    aqhbci)
      aqbanking_plugins_backends_dirs="$aqbanking_plugins_backends_dirs aqhbci"
      AC_DEFINE(AQBANKING_WITH_PLUGIN_BACKEND_AQHBCI, 1, [plugin availability])
      ;;
    aqebics)
      aqbanking_plugins_backends_dirs="$aqbanking_plugins_backends_dirs aqebics"
      AC_DEFINE(AQBANKING_WITH_PLUGIN_BACKEND_AQEBICS, 1, [plugin availability])
      ;;
    aqnone)
      aqbanking_plugins_backends_dirs="$aqbanking_plugins_backends_dirs aqnone"
      AC_DEFINE(AQBANKING_WITH_PLUGIN_BACKEND_AQNONE, 1, [plugin availability])
      ;;
    aqofxconnect)
      aqbanking_plugins_backends_dirs="$aqbanking_plugins_backends_dirs aqofxconnect"
      AC_DEFINE(AQBANKING_WITH_PLUGIN_BACKEND_AQOFXCONNECT, 1, [plugin availability])
      ;;
    aqpaypal)
      AC_MSG_WARN("WARNING: AqPaypal no longer supported, ignoring")
      ;;
    *)
      AC_MSG_ERROR("ERROR: Unknown plugin \"$f\"")
      ;;
  esac
done

AC_SUBST(aqbanking_plugins_backends_dirs)



###-------------------------------------------------------------------------
#
# ImExporters. Those do not have a required ordering.
#

AC_ARG_WITH(imexporters, 
  [  --with-imexporters=list      space separated list of im-/exporters (default:all)],
  [aqbanking_imexporters="$withval"],
  [aqbanking_imexporters="all"]
)

if test "$aqbanking_imexporters" = "all"; then
  aqbanking_imexporters="csv dtaus eri2 ofx openhbci1 swift xmldb yellownet sepa ctxfile q43"
fi

for f in ${aqbanking_imexporters}; do
  case "$f" in
    csv)
      aqbanking_plugins_imexporters_dirs="$aqbanking_plugins_imexporters_dirs csv"
      ;;
    dtaus)
      aqbanking_plugins_imexporters_dirs="$aqbanking_plugins_imexporters_dirs dtaus"
      ;;
    eri2)
      aqbanking_plugins_imexporters_dirs="$aqbanking_plugins_imexporters_dirs eri2"
      ;;
    ofx)
      aqbanking_plugins_imexporters_dirs="$aqbanking_plugins_imexporters_dirs ofx"
      ;;
    openhbci1)
      aqbanking_plugins_imexporters_dirs="$aqbanking_plugins_imexporters_dirs openhbci1"
      ;;
    swift)
      aqbanking_plugins_imexporters_dirs="$aqbanking_plugins_imexporters_dirs swift"
      ;;
    xmldb)
      aqbanking_plugins_imexporters_dirs="$aqbanking_plugins_imexporters_dirs xmldb"
      ;;
    yellownet)
      aqbanking_plugins_imexporters_dirs="$aqbanking_plugins_imexporters_dirs yellownet"
      ;;
    sepa)
      aqbanking_plugins_imexporters_dirs="$aqbanking_plugins_imexporters_dirs sepa"
      ;;
    ctxfile)
      aqbanking_plugins_imexporters_dirs="$aqbanking_plugins_imexporters_dirs ctxfile"
      ;;
    q43)
      aqbanking_plugins_imexporters_dirs="$aqbanking_plugins_imexporters_dirs q43"
      ;;
    *)
      AC_MSG_ERROR("ERROR: Unknown plugin \"$f\"")
      ;;
  esac
done

AC_SUBST(aqbanking_plugins_imexporters_dirs)



###-------------------------------------------------------------------------
#
# Parsers. Those do not have a required ordering.
#

AC_ARG_WITH(parsers, 
  [  --with-parsers=list      space separated list of parsers (default:all)],
  [aqbanking_parsers="$withval"],
  [aqbanking_parsers="all"]
)

if test "$aqbanking_parsers" = "all"; then
  aqbanking_parsers="dtaus swift"
fi

for f in ${aqbanking_parsers}; do
  case "$f" in
    dtaus)
      aqbanking_plugins_parsers_dirs="$aqbanking_plugins_parsers_dirs dtaus"
      ;;
    swift)
      aqbanking_plugins_parsers_dirs="$aqbanking_plugins_parsers_dirs swift"
      ;;
    *)
      AC_MSG_ERROR("ERROR: Unknown plugin \"$f\"")
      ;;
  esac
done

AC_SUBST(aqbanking_plugins_parsers_dirs)



###-------------------------------------------------------------------------
#
# Bankinfo plugins. Those do not have a required ordering.
#

AC_ARG_WITH(bankinfos, 
  [  --with-bankinfos=list      space separated list of bankinfo (default:all)],
  [aqbanking_bankinfos="$withval"],
  [aqbanking_bankinfos="all"]
)

if test "$aqbanking_bankinfos" = "all"; then
  aqbanking_bankinfos="at ca ch de us"
fi

for f in ${aqbanking_bankinfos}; do
  case "$f" in
    at)
      aqbanking_plugins_bankinfo_dirs="$aqbanking_plugins_bankinfo_dirs at"
      ;;
    ca)
      aqbanking_plugins_bankinfo_dirs="$aqbanking_plugins_bankinfo_dirs ca"
      ;;
    ch)
      aqbanking_plugins_bankinfo_dirs="$aqbanking_plugins_bankinfo_dirs ch"
      ;;
    de)
      aqbanking_plugins_bankinfo_dirs="$aqbanking_plugins_bankinfo_dirs de"
      ;;
    us)
      aqbanking_plugins_bankinfo_dirs="$aqbanking_plugins_bankinfo_dirs us"
      ;;
    *)
      AC_MSG_ERROR("ERROR: Unknown plugin \"$f\"")
      ;;
  esac
done

AC_SUBST(aqbanking_plugins_bankinfo_dirs)



###-------------------------------------------------------------------------
#
# KtoBlzCheck
#

pkg_modules="ktoblzcheck >= 1.10"
PKG_CHECK_MODULES(ktoblzcheck, [$pkg_modules], [have_ktoblzcheck="yes"], [have_ktoblzcheck="no"])
if test "$have_ktoblzcheck" = "yes"; then
  AC_DEFINE(HAVE_KTOBLZCHECK, 1, [if KTOBLZCHECK is available])
  ktoblzcheck_includes="$ktoblzcheck_CFLAGS"
  ktoblzcheck_libs="$ktoblzcheck_LIBS"
else
  AC_MSG_WARN([
*** ktoblzcheck is missing. 
*** Account number checking will not be available.
*** We strongly suggest installing ktoblzcheck and its devel
*** package.])
fi

AM_CONDITIONAL(WITH_KTOBLZCHECK, [test "$have_ktoblzcheck" = "yes"])
AC_SUBST(ktoblzcheck_libs)
AC_SUBST(ktoblzcheck_includes)



###-------------------------------------------------------------------------
#
# Check for Gwen
#

# Variable used in aqbanking-config.cmake
AC_SUBST(GWENHYWFAR_MINIMUM_VERSION, "4.10.0.0")
AC_GWENHYWFAR(4,10,0,0)

all_includes="$all_includes $gwenhywfar_includes"

AC_ARG_ENABLE(gui-tests,
  [  --enable-gui-tests         allow GUI tests (default=no)],
  enable_gui_tests="$enableval",
  enable_gui_tests="no")
AC_MSG_RESULT($enable_gui_tests)

if test "$enable_gui_tests" != "no"; then
  # For libgwenhywfar > 3.99.20, look for its library gwengui-gtk2.
  PKG_CHECK_MODULES([GWENGUI_GTK2], [gwengui-gtk2 gwenhywfar], [
      HAVE_GWENGUI_GTK2=1
      PKG_CHECK_MODULES([GTK2], [gtk+-2.0])
    ], [
      HAVE_GWENGUI_GTK2=0
    ])
  AC_SUBST(HAVE_GWENGUI_GTK2)
  if test "$HAVE_GWENGUI_GTK2" = "1"; then
    AC_DEFINE(USE_GWENGUI_GTK2, 1, [GTK2Gui availability])
  fi
else
  HAVE_GWENGUI_GTK2=0
fi
AM_CONDITIONAL(WITH_GWENGUI_GTK2, [test "$HAVE_GWENGUI_GTK2" = "1"])


###-------------------------------------------------------------------------
#
# check typemaker2
#

AC_MSG_CHECKING(typemaker2 binary)
AC_ARG_WITH(typemaker2-exe, 
  [  --with-typemaker2-exe=EXE      path and name of the executable typemaker2],
  [typemaker2_exe="$withval"],
  [typemaker2_exe="\$(gwenhywfar_bindir)/typemaker2"]
)
AC_MSG_RESULT($typemaker2_exe)
AC_SUBST(typemaker2_exe)



###-------------------------------------------------------------------------
#
# check xmlmerge
#
AC_MSG_CHECKING(xmlmerge)
AC_ARG_WITH(xmlmerge, [  --with-xmlmerge=PATH path and name of tool xmlmerge],
  [XMLMERGE="$withval"],
  [XMLMERGE="$gwenhywfar_dir/bin/xmlmerge"])
AC_SUBST(XMLMERGE)
AC_MSG_RESULT($XMLMERGE)



###-------------------------------------------------------------------------
#
# Check for GMP
#

# Allow the user to override the gmp_libs detection here
if test "x$gmp_libs" = "x"; then
  AC_CHECK_LIB(gmp, __gmpz_init, [have_gmp="yes"], [have_gmp="no"])
  AC_CHECK_HEADERS(gmp.h, [], [have_gmp="no"])
  if test "$have_gmp" != "yes"; then
    AC_MSG_ERROR([GMP is required, see http://swox.com/gmp])
  else
    gmp_libs="-lgmp"
  fi
fi
AC_SUBST(gmp_libs)



###-------------------------------------------------------------------------
#
# OS dependant settings
#

# Common settings; can be overridden per-OS below.
aqbanking_internal_libs="${abs_top_builddir}/src/libs/aqbanking/libaqbanking.la"
aqbanking_ldflags="-L\${libdir}"
aqbanking_libs="-laqbanking"
aqbanking_includes="-I\${includedir}/aqbanking5"
aqbanking_headerdir_am="\$(includedir)/aqbanking5"
aqbanking_headerdir="\${includedir}/aqbanking5"
aqbanking_plugindir="\${libdir}/aqbanking/plugins/${AQBANKING_SO_EFFECTIVE}"
aqbanking_pkgdatadir="\${datadir}/aqbanking"
aqbanking_symlinkdir="\$(top_builddir)/aqbanking5"

case "$OS_TYPE" in
    posix)
        aqbanking_sys_is_windows=0
    	;;
    windows)
        aqbanking_sys_is_windows=1
        AC_DEFINE_UNQUOTED(BUILDING_AQBANKING_DLL,1, [Define if DLL is built])
    	;;
esac
AM_CONDITIONAL(IS_WINDOWS, [test "$OS_TYPE" = "windows"])



###-------------------------------------------------------------------------
#
# check whether local installation mode is enabled
#

AC_MSG_CHECKING(whether local installation mode is wanted)
AC_ARG_ENABLE(local-install,
  [  --enable-local-install         allow local installation mode (default=no)],
  enable_local_install="$enableval",
  enable_local_install="no")
AC_MSG_RESULT($enable_local_install)

if test "$enable_local_install" != "no"; then
  AC_DEFINE(ENABLE_LOCAL_INSTALL, 1, [whether local install is wanted])
fi



###-------------------------------------------------------------------------
#
# check plugin search path
#
AC_MSG_CHECKING(plugin search dir)
AC_ARG_WITH(plugin-searchdir, [  --with-plugin-searchdir=DIR where to search for plugins],
  [aqbanking_plugin_searchdir="$withval"],
  [aqbanking_plugin_searchdir=""])

if test -z "${aqbanking_plugin_searchdir}"; then
  if test "$OSYSTEM" = "windows"; then
    aqbanking_plugin_searchdir="lib/aqbanking/plugins/${AQBANKING_SO_EFFECTIVE}"
  else
    if test "$enable_local_install" != "no"; then
      aqbanking_plugin_searchdir="lib/aqbanking/plugins/${AQBANKING_SO_EFFECTIVE}"
    else
      aqbanking_plugin_searchdir="$aqbanking_plugindir"
    fi
  fi
fi

AC_SUBST(aqbanking_plugin_searchdir)
AC_MSG_RESULT($aqbanking_plugin_searchdir)



###-------------------------------------------------------------------------
#
# check cfg search dir
#
AC_MSG_CHECKING(cfg search dir)
AC_ARG_WITH(cfg-searchdir, [  --with-cfg-searchdir=DIR where to search for cfg files],
  [aqbanking_cfg_searchdir="$withval"],
  [aqbanking_cfg_searchdir=""])

if test -z "$aqbanking_cfg_searchdir"; then
  if test "$OSYSTEM" = "windows"; then
    aqbanking_cfg_searchdir="etc"
  else
    if test "$enable_local_install" != "no"; then
      aqbanking_cfg_searchdir="etc"
    else
      aqbanking_cfg_searchdir="\$(sysconfdir)"
    fi
  fi
fi


AC_SUBST(aqbanking_cfg_searchdir)
AC_MSG_RESULT($aqbanking_cfg_searchdir)



###-------------------------------------------------------------------------
#
# check locale search dir
#
AC_MSG_CHECKING(locale search dir)
AC_ARG_WITH(locale-searchdir, [  --with-locale-searchdir=DIR where to search for locale files],
  [aqbanking_locale_searchdir="$withval"],
  [aqbanking_locale_searchdir=""])

if test -z "${aqbanking_locale_searchdir}"; then
  if test "$OSYSTEM" = "windows"; then
    aqbanking_locale_searchdir="share/locale"
  else
    if test "$enable_local_install" != "no"; then
      aqbanking_locale_searchdir="share/locale"
    else
      aqbanking_locale_searchdir="\$(localedir)"
    fi
  fi
fi

AC_SUBST(aqbanking_locale_searchdir)
AC_MSG_RESULT($aqbanking_locale_searchdir)



###-------------------------------------------------------------------------
#
# check data search dir
#
AC_MSG_CHECKING(data search dir)
AC_ARG_WITH(data-searchdir, [  --with-data-searchdir=DIR where to search for data files],
  [aqbanking_data_searchdir="$withval"],
  [aqbanking_data_searchdir=""])

if test -z "${aqbanking_data_searchdir}"; then
  if test "$OSYSTEM" = "windows"; then
    aqbanking_data_searchdir="share"
  else
    if test "$enable_local_install" != "no"; then
      aqbanking_data_searchdir="share"
    else
      aqbanking_data_searchdir="\$(datadir)"
    fi
  fi
fi

AC_SUBST(aqbanking_data_searchdir)
AC_MSG_RESULT($aqbanking_data_searchdir)



###-------------------------------------------------------------------------
# check for Distribution

AQ_DISTRIBUTION



###-------------------------------------------------------------------------
#
# check for I18N
#


# check whether i18n files are to be created
AC_CHECK_PROG(XGETTEXT,xgettext,xgettext, missing)

MAKE_I18N_FILES="yes"
if test "$XGETTEXT" = "missing"; then
  MAKE_I18N_FILES="no"
  AC_MSG_WARN([xgettext is missing. Locale suport is disabled.])
fi

AC_PATH_PROG([MSGFMT], [msgfmt], [missing])
if test "$MSGFMT" = "missing"; then
  MAKE_I18N_FILES="no"
  AC_MSG_WARN([msgfmt is missing. Locale suport is disabled.])
else
  # check for --qt argument of msgfmt
  AC_MSG_CHECKING(whether msgfmt understands --qt)
  if test "x${MSGFMT}" != "x"; then
    if ${MSGFMT} --qt --help >/dev/null 2>&1; then
      MSGFMT_SUPPORTS_QT=yes
    else
      MSGFMT_SUPPORTS_QT=no
    fi
  else
    MSGFMT_SUPPORTS_QT=no
  fi
  AC_MSG_RESULT($MSGFMT_SUPPORTS_QT)
fi

AC_SUBST(MAKE_I18N_FILES)
AC_SUBST(MSGFMT_SUPPORTS_QT)
AM_CONDITIONAL(IF_MAKE_I18N_FILES, [test "$MAKE_I18N_FILES" = "yes"])


# then check whether i18n support is to be compiled in
i18n_libs=""
HAVE_I18N="yes"
AC_CHECK_HEADERS([locale.h libintl.h], [], [HAVE_I18N="no"])
AC_CHECK_FUNCS([setlocale])

if test "$OS_TYPE" != "windows"; then
  oldlibs="$LIBS"
  LIBS=""
  AC_SEARCH_LIBS(gettext, intl, [], [HAVE_I18N="no"])
  i18n_libs="$LIBS"
  LIBS="$oldlibs"
else
  i18n_libs="-lintl"
fi

if test "$HAVE_I18N" != "NO"; then
  AC_DEFINE_UNQUOTED(HAVE_I18N, 1, [whether I18N is available])
fi

AC_SUBST(HAVE_I18N)
AC_SUBST(i18n_libs)



###-------------------------------------------------------------------------
#
# check for release
#
AC_MSG_CHECKING(whether this is an official release)
AC_ARG_ENABLE(release,
  [  --enable-release         make this an official release (default=no)],
  [ case "${enableval}" in
    yes) enable_release="yes";;
    no)  enable_release="no";;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-release);;
    esac
  ],
  enable_release="no")

if test "$enable_release" = "yes"; then
  STRIPALL="-s"
else
  STRIPALL=""
fi
AC_SUBST(STRIPALL)
AC_MSG_RESULT($enable_release)



###-------------------------------------------------------------------------
# Debug arguments for compilation

ACX_COMPILE_WARN()



###-------------------------------------------------------------------------
#
# GCC version (check for usability)
#
AC_MSG_CHECKING(if symbol visibility is supported)
visibility_cflags=""
case `basename $CC` in 
  *gcc*)
     dnl Determine version number (watch out: the outside square
     dnl parenteses are vitally important here!)
     gccversion=["` ${CC} --version | head -1 | ${SED} 's/.*\([0-9]\.[0-9]\.[0-9]*\).*/\1/' `"]
     case $gccversion in
       4.*)
	 # Activate -fvisibility=hidden only for explicit 4.x gcc
	 # versions. We never now what future gcc's might do with
	 # that argument, and we also can't be sure that other
	 # gcc's happen to show very weird version
	 # numbers. Therefore leave the default at non-activated.
         visibility_supported="yes, gcc-${gccversion}"
         AC_DEFINE(GCC_WITH_VISIBILITY_ATTRIBUTE, 1, [visibility])
         visibility_cflags="-fvisibility=hidden"
         ;;
       *)
	 # Default: -fvisibility=hidden not activated.
         visibility_supported="no (needs gcc >=4.0, you have gcc-${gccversion})"
         ;;
      esac
      ;;
  *)
    visibility_supported="no (needs gcc >=4.0, you use $CC)"
    ;;
esac

# -fvisibility is at least not available on MinGW/gcc-3.4.4 (will
# give an "unrecognized cmdline option"). Also, unfortunately I
# don't know an easy way to ask the compiler here. Maybe
# http://autoconf-archive.cryp.to/ax_cflags_gcc_option.html
case "$OS_TYPE" in
  windows)
        visibility_supported="no (not yet on MinGW/Windows)"
        visibility_cflags=""
	;;
  *)
	;;
esac


AC_MSG_RESULT(${visibility_supported})
AC_SUBST(visibility_cflags)



###-------------------------------------------------------------------------
#
# Subst
#
AS_SCRUB_INCLUDE(aqbanking_includes)
AC_SUBST(aqbanking_sys_is_windows)
AC_SUBST(aqbanking_includes)
AC_SUBST(aqbanking_internal_libs)
AC_SUBST(aqbanking_ldflags)
AC_SUBST(aqbanking_libs)
AC_SUBST(aqbanking_plugindir)
AC_SUBST(aqbanking_pkgdatadir)
AC_SUBST(aqbanking_symlinkdir)
AC_SUBST(aqbanking_headerdir_am)
AC_SUBST(aqbanking_headerdir)


###-------------------------------------------------------------------------
#
# Check for additional aclocal flags
#
ADD_ACLOCAL_FLAGS=""
for A in "${GWEN_PREFIX}/share/aclocal" \
	 "${prefix}/share/aclocal"; do 
  if test -d "${A}"; then 
    ADD_ACLOCAL_FLAGS="${ADD_ACLOCAL_FLAGS} -I ${A}"
  fi
done
AC_SUBST(ADD_ACLOCAL_FLAGS)



###-------------------------------------------------------------------------
#
# AqBanking modules
#

aqbanking_modules=". jobs types"
aqbankingpp_modules="."

AC_SUBST(aqbanking_modules)
AC_SUBST(aqbankingpp_modules)



###-------------------------------------------------------------------------
#
# docpath
#
AC_MSG_CHECKING(docpath)
AC_ARG_WITH(docpath, [  --with-docpath=DIR where to store the apidoc],
  [docpath="$withval"],
  [docpath="${HOME}/apidoc"])
AC_SUBST(docpath)
AC_MSG_RESULT($docpath)


###-------------------------------------------------------------------------
#
# check for doc type
#
AC_MSG_CHECKING(if full docu should be created)
AC_ARG_ENABLE(full-doc,
  [  --enable-full-doc         enable creating full apidoc (default=no)],
  [ case "${enableval}" in
    yes) enable_fulldoc="yes";;
    no)  enable_fulldoc="no";;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-full-doc);;
    esac
  ],
  enable_fulldoc="no")

if test "$enable_fulldoc" = "yes"; then
	DOXYGEN_INPUT="listdoc.h src"
        DOXYGEN_DEFINE=""
        DOXYGEN_FILE_PATTERNS="*.h *.hpp *.c *.cpp"
else
	DOXYGEN_DEFINE="DOXYGEN_HIDE"
	DOXYGEN_INPUT="listdoc.h aqbanking5"
        DOXYGEN_FILE_PATTERNS="*.h *.hpp"
fi
AC_SUBST(DOXYGEN_INPUT)
AC_SUBST(DOXYGEN_DEFINE)
AC_SUBST(DOXYGEN_FILE_PATTERNS)
AC_MSG_RESULT($enable_fulldoc)



###-------------------------------------------------------------------------
#
# search for tag files
#
AC_MSG_CHECKING(doxygen tag files)
DOXYGEN_TAGFILES=""
if test -d "${docpath}"; then
  DOXYGEN_TAGFILES="`cd ${docpath} && ls *.tag`"
  if test -n "${DOXYGEN_TAGFILES}"; then
   DOXYGEN_TAGFILES="`echo ${DOXYGEN_TAGFILES} | ${SED} -e s/${PACKAGE}.tag//`"
  fi
  realfiles=""
  for ff in ${DOXYGEN_TAGFILES}; do
    realfiles="${realfiles} ${docpath}/${ff}"
  done
  DOXYGEN_TAGFILES="${realfiles}"
fi
if test -z "${DOXYGEN_TAGFILES}"; then
  AC_MSG_RESULT(none)
else
  AC_MSG_RESULT(found)
fi
AC_SUBST(DOXYGEN_TAGFILES)



###-------------------------------------------------------------------------
#
# check for dependencies of backends
#  (nothing to check for right now)

dependencies_ok="yes"



###-------------------------------------------------------------------------
#
# Configure stuff for backend AqHBCI
#

# Common settings; can be overridden per-OS below.
aqhbci_internal_libs="${abs_top_builddir}/src/plugins/backends/aqhbci/plugin/libaqhbci.la"
aqhbci_ldflags="-L\${libdir}"
aqhbci_includes="${aqbanking_includes}"
aqhbci_libs="-laqhbci"
with_aqhbci="no"

case "$aqbanking_backends" in *aqhbci*)
  with_aqhbci="yes"
  case "$OS_TYPE" in
      posix)
          ;;
      windows)
          AC_DEFINE_UNQUOTED(BUILDING_AQHBCI_DLL,1, [Define if DLL is built])
          ;;
  esac
  ;;
esac

AS_SCRUB_INCLUDE(aqhbci_includes)
AC_SUBST(aqhbci_includes)
AC_SUBST(aqhbci_internal_libs)
AC_SUBST(aqhbci_libs)
AC_SUBST(aqhbci_ldflags)
AC_SUBST(with_aqhbci)



###-------------------------------------------------------------------------
#
# check AqHBCI data dir
#
AC_MSG_CHECKING(aqhbci data dir)
AC_ARG_WITH(aqhbci-datadir, [  --with-aqhbci-datadir=DIR where to search for data files],
  [aqhbci_datadir="$withval"],
  [aqhbci_datadir="$aqhbci_data"])
AC_SUBST(aqhbci_datadir)
AC_MSG_RESULT($aqhbci_datadir)



###-------------------------------------------------------------------------
#
# Configure stuff for backend AqOfxConnect
#

# Common settings
aqofxconnect_internal_libs="${abs_top_builddir}/src/plugins/backends/aqofxconnect/plugin/libaqofxconnect.la"
aqofxconnect_includes="${aqbanking_includes}"
aqofxconnect_libs="-L\${libdir} -laqofxconnect"

case "$aqbanking_backends" in *aqofxconnect*)
  case "$OS_TYPE" in
      posix)
          ;;
      windows)
          AC_DEFINE_UNQUOTED(BUILDING_AQOFXCONNECT_DLL,1, [Define if DLL is built])
          ;;
  esac
  ;;
esac

AS_SCRUB_INCLUDE(aqofxconnect_includes)
AC_SUBST(aqofxconnect_includes)
AC_SUBST(aqofxconnect_internal_libs)
AC_SUBST(aqofxconnect_libs)
#AC_DEFINE_UNQUOTED(AQOFXCONNECT_DATA, "${aqofxconnect_data}", [data dir])



###-------------------------------------------------------------------------
#
# Configure stuff for backend AqEBICS
#

# Common settings; can be overridden per-OS below.
aqebics_internal_libs="${abs_top_builddir}/src/plugins/backends/aqebics/plugin/libaqebics.la"
aqebics_ldflags="-L\${libdir}"
aqebics_includes="${aqbanking_includes}"
aqebics_libs="-laqebics"
with_aqebics="no"

case "$aqbanking_backends" in *aqebics*)
  with_aqebics="yes"
  case "$OS_TYPE" in
      posix)
          ;;
      windows)
          AC_DEFINE_UNQUOTED(BUILDING_AQEBICS_DLL,1, [Define if DLL is built])
          ;;
  esac
  ;;
esac

AS_SCRUB_INCLUDE(aqebics_includes)
AC_SUBST(aqebics_includes)
AC_SUBST(aqebics_internal_libs)
AC_SUBST(aqebics_libs)
AC_SUBST(aqebics_ldflags)
AC_SUBST(with_aqebics)



###-------------------------------------------------------------------------
#
# check AqEBICS data dir
#
AC_MSG_CHECKING(aqebics data dir)
AC_ARG_WITH(aqebics-datadir, [  --with-aqebics-datadir=DIR where to search for data files],
  [aqebics_datadir="$withval"],
  [aqebics_datadir="$aqebics_data"])
AC_SUBST(aqebics_datadir)
AC_MSG_RESULT($aqebics_datadir)




###-------------------------------------------------------------------------
#
# Check for xmlsec
#
if test "$with_aqebics" = "yes"; then
  AC_MSG_CHECKING(for build requirements needed by EBICS backend)
  AC_MSG_RESULT()
  PKG_CHECK_MODULES(XMLSEC,  [xmlsec1-gnutls >= 1.0.0])
  PKG_CHECK_MODULES(LIBXML,  [libxml-2.0])
  PKG_CHECK_MODULES(LIBXSLT, [libxslt])
  AQEBICS_CFLAGS="$XMLSEC_CFLAGS $LIBXML_CFLAGS $LIBXSLT_CFLAGS"
  AQEBICS_LIBS="$XMLSEC_LIBS $LIBXML_LIBS $LIBXSLT_LIBS"
  AC_SUBST(AQEBICS_CFLAGS)
  AC_SUBST(AQEBICS_LIBS)
fi




###-------------------------------------------------------------------------
#
# check for plugin init
#
AC_MSG_CHECKING(if plugin init is wanted)
AC_ARG_ENABLE(init-plugins,
  [  --enable-init-plugins             enable plugin init (default=yes)],
  enable_init_plugins="$enableval",
  enable_init_plugins="yes")
AC_MSG_RESULT($enable_init_plugins)

if test "$enable_init_plugins" != "no"; then
  AC_DEFINE(AQBANKING_ENABLE_INIT_PLUGINS, 1, [if plugin init is wanted])
fi



###-------------------------------------------------------------------------
#
# check whether aqbanking is a subproject to another one
#

AC_MSG_CHECKING(if AqBanking is a subproject)
AC_ARG_ENABLE(aqb-subproject,
  [  --enable-aqb-subproject         make gwen only a subproject (default=no)],
  enable_aqb_subproject="$enableval",
  enable_aqb_subproject="no")
AC_MSG_RESULT($enable_aqb_subproject)

if test "$enable_aqb_subproject" != "no"; then
  AC_DEFINE(AQBANKING_IS_SUBPROJECT, 1, [whether aqb is a subproject])
fi



###-------------------------------------------------------------------------
#
# check whether aqbanking-cli is enabled
#

AC_MSG_CHECKING(whether aqbanking-cli is wanted)
AC_ARG_ENABLE(cli,
  [  --enable-cli         build aqbanking-cli (default=yes)],
  enable_cli="$enableval",
  enable_cli="yes")
AC_MSG_RESULT($enable_cli)

AM_CONDITIONAL(ENABLE_CLI, [test "$enable_cli" = "yes"])

AC_ARG_WITH(cli-version-string, 
  [  --with-cli-version-string      list of additional libraries],
  [AQBANKING_CLI_VERSION_STRING="$withval"],
  [AQBANKING_CLI_VERSION_STRING="$AQBANKING_VERSION_STRING"]
)

AC_SUBST(AQBANKING_CLI_VERSION_STRING)


###-------------------------------------------------------------------------
#
# check whether tutorials are to be built
#

AC_MSG_CHECKING(whether tutorials are to be built)
AC_ARG_ENABLE(tutorials,
  [  --enable-tutorials         build tutorials (default=yes)],
  [ case "${enableval}" in
    yes) TUTORIALS="tutorials";;
    no)  TUTORIALS="";;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-tutorials);;
    esac
  ],
  [
   TUTORIALS="tutorials"
   enableval="yes"
  ])
AC_MSG_RESULT($enableval)
AC_SUBST(TUTORIALS)


###-------------------------------------------------------------------------
#
# Cleanup includes, define all_XXX
#

AS_SCRUB_INCLUDE(all_includes)
AC_SUBST(all_includes)
AC_SUBST(all_libraries)

AC_SUBST(aqbanking_backends)


# Adjust CFLAGS and LDFLAGS on Windows
case "$OS_TYPE" in
    windows)
	# GnuCash compiles with this; maybe we need this, too?
	#CFLAGS="${CFLAGS} -mms-bitfields"
	LDFLAGS="${LDFLAGS} -no-undefined -mwindows"
    ;;
esac



###-------------------------------------------------------------------------
#
# Output files
#

AC_CONFIG_FILES([
  mksymlinks.sh
  admin/Makefile
  doc/Makefile
  m4/Makefile
  src/Makefile
  src/libs/Makefile
  src/libs/aqbanking/Makefile
  src/libs/aqbanking/jobs/Makefile
  src/libs/aqbanking/types/Makefile
  src/libs/aqbanking/gui/Makefile
  src/libs/aqbanking/dialogs/Makefile
  src/libs/aqbanking/typemaker2/Makefile
  src/libs/aqbanking/typemaker2/c/Makefile
  src/libs/aqbanking/backendsupport/Makefile
  src/libs/aqbankingpp/Makefile
  src/plugins/Makefile
  src/plugins/bankinfo/Makefile
  src/plugins/bankinfo/at/Makefile
  src/plugins/bankinfo/at/at.xml
  src/plugins/bankinfo/ca/Makefile
  src/plugins/bankinfo/ca/ca.xml
  src/plugins/bankinfo/ch/Makefile
  src/plugins/bankinfo/ch/ch.xml
  src/plugins/bankinfo/de/Makefile
  src/plugins/bankinfo/de/de.xml
  src/plugins/bankinfo/us/Makefile
  src/plugins/bankinfo/us/us.xml
  src/plugins/bankinfo/generic/Makefile
  src/plugins/imexporters/Makefile
  src/plugins/imexporters/dtaus/Makefile
  src/plugins/imexporters/dtaus/dtaus.xml
  src/plugins/imexporters/dtaus/profiles/Makefile
  src/plugins/imexporters/dtaus/profiles/default.conf
  src/plugins/imexporters/dtaus/profiles/debitnote.conf
  src/plugins/imexporters/dtaus/profiles/transfer.conf
  src/plugins/imexporters/eri2/Makefile
  src/plugins/imexporters/eri2/eri2.xml
  src/plugins/imexporters/eri2/profiles/Makefile
  src/plugins/imexporters/eri2/profiles/default.conf
  src/plugins/imexporters/ofx/Makefile
  src/plugins/imexporters/ofx/ofx.xml
  src/plugins/imexporters/ofx/parser/Makefile
  src/plugins/imexporters/ofx/profiles/Makefile
  src/plugins/imexporters/ofx/profiles/default.conf
  src/plugins/imexporters/ofx/profiles/iso8859.conf
  src/plugins/imexporters/swift/Makefile
  src/plugins/imexporters/swift/swift.xml
  src/plugins/imexporters/swift/profiles/Makefile
  src/plugins/imexporters/swift/profiles/swiftmt940.conf
  src/plugins/imexporters/swift/profiles/swiftmt942.conf
  src/plugins/imexporters/swift/profiles/amro940.conf
  src/plugins/imexporters/swift/profiles/amro942.conf
  src/plugins/imexporters/csv/Makefile
  src/plugins/imexporters/csv/csv.xml
  src/plugins/imexporters/csv/profiles/Makefile
  src/plugins/imexporters/csv/profiles/default.conf
  src/plugins/imexporters/csv/profiles/amex.conf
  src/plugins/imexporters/csv/profiles/aqmoney1.conf
  src/plugins/imexporters/csv/profiles/aqmoney2.conf
  src/plugins/imexporters/csv/profiles/comdirect.conf
  src/plugins/imexporters/csv/profiles/consors-tagesgeld.conf
  src/plugins/imexporters/csv/profiles/mijnpostbank.conf
  src/plugins/imexporters/csv/profiles/oko.conf
  src/plugins/imexporters/csv/profiles/spk-aachen.conf
  src/plugins/imexporters/csv/profiles/spk-steiermark.conf
  src/plugins/imexporters/csv/profiles/tonline-banking.conf
  src/plugins/imexporters/csv/profiles/test.conf
  src/plugins/imexporters/csv/profiles/vrnetworld.conf
  src/plugins/imexporters/csv/profiles/lbbberlin.conf
  src/plugins/imexporters/csv/profiles/bankaustria.conf
  src/plugins/imexporters/csv/profiles/psk.conf
  src/plugins/imexporters/csv/profiles/diba.conf
  src/plugins/imexporters/csv/profiles/dkb.conf
  src/plugins/imexporters/csv/profiles/dkb-cash.conf
  src/plugins/imexporters/csv/profiles/sparda.conf
  src/plugins/imexporters/csv/profiles/full.conf
  src/plugins/imexporters/csv/profiles/hypovereinsbank.conf
  src/plugins/imexporters/csv/profiles/postbankffm.conf
  src/plugins/imexporters/csv/profiles/cli-transfers.conf
  src/plugins/imexporters/csv/profiles/sns.conf
  src/plugins/imexporters/csv/profiles/transfers.conf
  src/plugins/imexporters/csv/profiles/sepadebitnotes.conf
  src/plugins/imexporters/openhbci1/Makefile
  src/plugins/imexporters/openhbci1/openhbci1.xml
  src/plugins/imexporters/openhbci1/profiles/Makefile
  src/plugins/imexporters/openhbci1/profiles/default.conf
  src/plugins/imexporters/qif/Makefile
  src/plugins/imexporters/qif/qif.xml
  src/plugins/imexporters/qif/profiles/Makefile
  src/plugins/imexporters/qif/profiles/default.conf
  src/plugins/imexporters/yellownet/Makefile
  src/plugins/imexporters/yellownet/yellownet.xml
  src/plugins/imexporters/yellownet/profiles/Makefile
  src/plugins/imexporters/yellownet/profiles/default.conf
  src/plugins/imexporters/xmldb/Makefile
  src/plugins/imexporters/xmldb/xmldb.xml
  src/plugins/imexporters/xmldb/profiles/Makefile
  src/plugins/imexporters/xmldb/profiles/default.conf
  src/plugins/imexporters/sepa/Makefile
  src/plugins/imexporters/sepa/sepa.xml
  src/plugins/imexporters/sepa/profiles/Makefile
  src/plugins/imexporters/sepa/profiles/default.conf
  src/plugins/imexporters/sepa/profiles/001_001_02.conf
  src/plugins/imexporters/sepa/profiles/001_002_03.conf
  src/plugins/imexporters/sepa/profiles/001_003_03.conf
  src/plugins/imexporters/sepa/profiles/008_001_01.conf
  src/plugins/imexporters/sepa/profiles/008_002_02.conf
  src/plugins/imexporters/sepa/profiles/008_003_02.conf
  src/plugins/imexporters/ctxfile/Makefile
  src/plugins/imexporters/ctxfile/ctxfile.xml
  src/plugins/imexporters/ctxfile/profiles/Makefile
  src/plugins/imexporters/ctxfile/profiles/default.conf
  src/plugins/imexporters/q43/Makefile
  src/plugins/imexporters/q43/q43.xml
  src/plugins/imexporters/q43/profiles/Makefile
  src/plugins/imexporters/q43/profiles/default.conf
  src/plugins/parsers/Makefile
  src/plugins/parsers/dtaus/Makefile
  src/plugins/parsers/dtaus/dtaus.xml
  src/plugins/parsers/swift/Makefile
  src/plugins/parsers/swift/swift.xml
  src/plugins/backends/Makefile
  src/plugins/backends/aqhbci/Makefile
  src/plugins/backends/aqhbci/plugin/Makefile
  src/plugins/backends/aqhbci/plugin/aqhbci.xml
  src/plugins/backends/aqhbci/plugin/dialogs/Makefile
  src/plugins/backends/aqhbci/plugin/header.xml
  src/plugins/backends/aqhbci/plugin/banking/Makefile
  src/plugins/backends/aqhbci/plugin/msglayer/Makefile
  src/plugins/backends/aqhbci/plugin/joblayer/Makefile
  src/plugins/backends/aqhbci/plugin/applayer/Makefile
  src/plugins/backends/aqhbci/plugin/ajobs/Makefile
  src/plugins/backends/aqhbci/plugin/applayer/xml/Makefile
  src/plugins/backends/aqhbci/plugin/version.h
  src/plugins/backends/aqhbci/tools/Makefile
  src/plugins/backends/aqhbci/tools/hbcixml/Makefile
  src/plugins/backends/aqhbci/tools/aqhbci-tool/Makefile
  src/plugins/backends/aqofxconnect/Makefile
  src/plugins/backends/aqofxconnect/version.h
  src/plugins/backends/aqofxconnect/plugin/Makefile
  src/plugins/backends/aqofxconnect/plugin/aqofxconnect.xml
  src/plugins/backends/aqofxconnect/plugin/libofxhome/Makefile
  src/plugins/backends/aqnone/Makefile
  src/plugins/backends/aqnone/aqnone.xml
  src/plugins/backends/aqebics/Makefile
  src/plugins/backends/aqebics/plugin/Makefile
  src/plugins/backends/aqebics/plugin/aqebics.xml
  src/plugins/backends/aqebics/plugin/client/Makefile
  src/plugins/backends/aqebics/plugin/dialogs/Makefile
  src/plugins/backends/aqebics/plugin/msg/Makefile
  src/plugins/backends/aqebics/tools/Makefile
  src/plugins/backends/aqebics/tools/aqebics-tool/Makefile
  src/test/Makefile
  src/tools/Makefile
  src/tools/aqbanking-cli/Makefile
  tutorials/Makefile
  po/Makefile
  Doxyfile
  Makefile
  aqbanking-config.in
  aqbanking.pc
  aqbanking.spec
  aqbanking.iss
  version.h
  system.h
  aqbanking-config.cmake
  aqbanking-config-version.cmake
])
AC_OUTPUT



###-------------------------------------------------------------------------
#
# Summary
#

echo
echo
echo "Summary"
echo "----------------------------------------------------"
echo "AqBanking version       : $AQBANKING_VERSION_FULL_STRING"
echo "Local Installation Mode : $enable_local_install"
echo "Prefix                  : $prefix"
echo "Exec-Prefix             : $exec_prefix"
echo "Libdir                  : $libdir"
echo "Cfg Search Folder       : $aqbanking_cfg_searchdir"
echo "Data Search Folder      : $aqbanking_data_searchdir"
echo "Locale Search Folder    : $aqbanking_locale_searchdir"
echo "Plugin Search Folder    : $aqbanking_plugin_searchdir"
echo "Symbol Visibility       : $visibility_supported"
echo "Use KtoBlzCheck         : $have_ktoblzcheck"
echo "Build AqBanking-CLI     : $enable_cli"
echo "CLI Version String      : $AQBANKING_CLI_VERSION_STRING"
echo

echo
echo "Plugins"
echo "----------------------------------------------------"
echo "  Backends              : $aqbanking_backends"
echo "  Im-/Exporters         : $aqbanking_imexporters"
echo "  Parsers               : $aqbanking_parsers"
echo "  Bank Infos            : $aqbanking_bankinfos"
echo

echo "Additional Information"
echo "----------------------------------------------------"
if test "$have_ktoblzcheck" != "yes"; then
  echo " - you really should consider installing ktoblzcheck "
  echo "   in order to enable account number validation!"
fi
echo

echo "----------------------------------------------------"
echo "You can now run \"make\" (or gmake on FreeBSD) to compile."


