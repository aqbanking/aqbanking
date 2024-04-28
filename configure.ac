# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.56)
AC_INIT
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AC_CONFIG_SRCDIR([README])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])


###-------------------------------------------------------------------------
#
# AqBanking versions
#

AQBANKING_VERSION_MAJOR=6
AQBANKING_VERSION_MINOR=5
AQBANKING_VERSION_PATCHLEVEL=9
AQBANKING_VERSION_BUILD=0
dnl "stable", "rcX", "betaX", "svn"
AQBANKING_VERSION_TAG="beta"



###-------------------------------------------------------------------------
#
# SO versions
#

#
# AqBanking
#

AQBANKING_SO_CURRENT=49
AQBANKING_SO_AGE=5
AQBANKING_SO_REVISION=9
AQBANKING_SO_EFFECTIVE="`echo \$(($AQBANKING_SO_CURRENT-$AQBANKING_SO_AGE))`"


###-------------------------------------------------------------------------
#
# Generate version strings and BUILD
#

SVN="svn"
AQBANKING_VERSION_FULL_STRING="${AQBANKING_VERSION_MAJOR}.${AQBANKING_VERSION_MINOR}.${AQBANKING_VERSION_PATCHLEVEL}.${AQBANKING_VERSION_BUILD}${AQBANKING_VERSION_TAG}"
AQBANKING_VERSION_STRING="$AQBANKING_VERSION_MAJOR.$AQBANKING_VERSION_MINOR.$AQBANKING_VERSION_PATCHLEVEL"
AQBANKING_FINTS_VERSION_STRING="$AQBANKING_VERSION_MAJOR.$AQBANKING_VERSION_MINOR"

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
# datetime
#
AC_ARG_WITH([build-datetime],
  [AS_HELP_STRING([--with-build-datetime=DATETIME],
                  [use fixed datetime for build (default: current date and time)])],
  [AS_IF([test "x$withval" = "xyes"],
    [DATETIME="`date +\"%Y%m%d%H%M%S\"`"],
    [DATETIME="`date +\"%Y%m%d%H%M%S\" --date=\"$withval\"`"])],
  [DATETIME="`date +\"%Y%m%d%H%M%S\"`"])
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
     AQBANKING_VERSION_RELEASE_STRING="$AQBANKING_VERSION_MAJOR.$AQBANKING_VERSION_MINOR.$AQBANKING_VERSION_PATCHLEVEL.${AQBANKING_VERSION_BUILD}${AQBANKING_VERSION_TAG}"
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

AC_SUBST(AQBANKING_FINTS_VERSION_STRING)
AC_DEFINE_UNQUOTED(AQBANKING_FINTS_VERSION_STRING, "$AQBANKING_FINTS_VERSION_STRING", [Version string for FINTS])

AC_SUBST(AQBANKING_SO_CURRENT)
AC_SUBST(AQBANKING_SO_REVISION)
AC_SUBST(AQBANKING_SO_AGE)
AC_SUBST(AQBANKING_SO_EFFECTIVE)
AC_DEFINE_UNQUOTED(AQBANKING_SO_EFFECTIVE_STR, "$AQBANKING_SO_EFFECTIVE",
                   [effective SO version])

AM_INIT_AUTOMAKE(aqbanking, $AQBANKING_VERSION_RELEASE_STRING)


###-------------------------------------------------------------------------
#
# determine operating system
#
AQ_CHECK_OS


###-------------------------------------------------------------------------
#
# prefix handling
#
AC_PREFIX_DEFAULT(/usr/local)
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
AC_PROG_CPP
AC_PROG_INSTALL

AC_PROG_LIBTOOL
AC_LIBTOOL_WIN32_DLL
AC_LIBTOOL_RC
# Newer libtool doesn't have AC_LIBTOOL_RC anymore.  Need to check for
# $(RC) by ourselves.
AC_CHECK_TOOL([RC], [windres], [no])
AC_CHECK_PROG(USE_DOT,dot,YES,NO)
AC_CHECK_PROG(SED,sed,sed)

PKG_PROG_PKG_CONFIG

# Check for the tool "astyle", but if not found, replace its program call by the no-op "echo" instead
AC_CHECK_PROG(ASTYLE, astyle, astyle, echo)

# Checks for libraries.

###-------------------------------------------------------------------------
#
# Checks for header files.
#
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h locale.h])
AC_CHECK_HEADERS([iconv.h libintl.h locale.h])
AC_CHECK_HEADERS([assert.h ctype.h errno.h fcntl.h stdio.h stdlib.h string.h strings.h locale.h])



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
  aqbanking_backends="aqhbci aqofxconnect aqebics aqpaypal aqnone aqgivve"
fi

for f in ${aqbanking_backends}; do
  case "$f" in
    aqhbci)
      aqbanking_plugins_backends_dirs="$aqbanking_plugins_backends_dirs aqhbci"
      aqbanking_plugins_backends_libs="$aqbanking_plugins_backends_libs aqhbci/libaqhbci.la"
      AC_DEFINE(AQBANKING_WITH_PLUGIN_BACKEND_AQHBCI, 1, [plugin availability])
      ;;
    aqfints)
      aqbanking_plugins_backends_dirs="$aqbanking_plugins_backends_dirs aqfints"
      aqbanking_plugins_backends_libs="$aqbanking_plugins_backends_libs aqfints/libaqfintsplugin.la"
      AC_DEFINE(AQBANKING_WITH_PLUGIN_BACKEND_AQFINTS, 1, [plugin availability])
      ;;
    aqebics)
      aqbanking_plugins_backends_dirs="$aqbanking_plugins_backends_dirs aqebics"
      aqbanking_plugins_backends_libs="$aqbanking_plugins_backends_libs aqebics/libaqebics.la"
      AC_DEFINE(AQBANKING_WITH_PLUGIN_BACKEND_AQEBICS, 1, [plugin availability])
      ;;
    aqofxconnect)
      aqbanking_plugins_backends_dirs="$aqbanking_plugins_backends_dirs aqofxconnect"
      aqbanking_plugins_backends_libs="$aqbanking_plugins_backends_libs aqofxconnect/libaqofxconnect.la"
      AC_DEFINE(AQBANKING_WITH_PLUGIN_BACKEND_AQOFXCONNECT, 1, [plugin availability])
      ;;
    aqpaypal)
      aqbanking_plugins_backends_dirs="$aqbanking_plugins_backends_dirs aqpaypal"
      aqbanking_plugins_backends_libs="$aqbanking_plugins_backends_libs aqpaypal/libaqpaypal.la"
      AC_DEFINE(AQBANKING_WITH_PLUGIN_BACKEND_AQPAYPAL, 1, [plugin availability])
      ;;
    aqnone)
      aqbanking_plugins_backends_dirs="$aqbanking_plugins_backends_dirs aqnone"
      aqbanking_plugins_backends_libs="$aqbanking_plugins_backends_libs aqnone/libaqnone.la"
      AC_DEFINE(AQBANKING_WITH_PLUGIN_BACKEND_AQNONE, 1, [plugin availability])
      ;;
    aqgivve)
      aqbanking_plugins_backends_dirs="$aqbanking_plugins_backends_dirs aqgivve"
      aqbanking_plugins_backends_libs="$aqbanking_plugins_backends_libs aqgivve/libaqgivve.la"
      AC_DEFINE(AQBANKING_WITH_PLUGIN_BACKEND_AQGIVVE, 1, [plugin availability])
      ;;
    *)
      AC_MSG_ERROR("ERROR: Unknown plugin $f")
      ;;
  esac
done

# always define this
AC_DEFINE(AQBANKING_WITH_PLUGIN_BACKEND_AQNONE, 1, [plugin availability])

AC_SUBST(aqbanking_plugins_backends_dirs)
AC_SUBST(aqbanking_plugins_backends_libs)


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
  aqbanking_imexporters="csv eri2 ofx openhbci1 swift xmldb yellownet sepa ctxfile q43 camt xml"
fi

for f in ${aqbanking_imexporters}; do
  case "$f" in
    csv)
      aqbanking_plugins_imexporters_dirs="$aqbanking_plugins_imexporters_dirs csv"
      aqbanking_plugins_imexporters_libs="$aqbanking_plugins_imexporters_libs csv/libabimexporters_csv.la"
      AC_DEFINE(AQBANKING_WITH_PLUGIN_IMEXPORTER_CSV, 1, [plugin availability])
      ;;
    eri2)
      aqbanking_plugins_imexporters_dirs="$aqbanking_plugins_imexporters_dirs eri2"
      aqbanking_plugins_imexporters_libs="$aqbanking_plugins_imexporters_libs eri2/libabimexporters_eri2.la"
      AC_DEFINE(AQBANKING_WITH_PLUGIN_IMEXPORTER_ERI2, 1, [plugin availability])
      ;;
    ofx)
      aqbanking_plugins_imexporters_dirs="$aqbanking_plugins_imexporters_dirs ofx"
      aqbanking_plugins_imexporters_libs="$aqbanking_plugins_imexporters_libs ofx/libabimexporters_ofx.la"
      AC_DEFINE(AQBANKING_WITH_PLUGIN_IMEXPORTER_OFX, 1, [plugin availability])
      ;;
    openhbci1)
      aqbanking_plugins_imexporters_dirs="$aqbanking_plugins_imexporters_dirs openhbci1"
      aqbanking_plugins_imexporters_libs="$aqbanking_plugins_imexporters_libs openhbci1/libabimexporters_openhbci1.la"
      AC_DEFINE(AQBANKING_WITH_PLUGIN_IMEXPORTER_OPENHBCI1, 1, [plugin availability])
      ;;
    swift)
      aqbanking_plugins_imexporters_dirs="$aqbanking_plugins_imexporters_dirs swift"
      aqbanking_plugins_imexporters_libs="$aqbanking_plugins_imexporters_libs swift/libabimexporters_swift.la"
      AC_DEFINE(AQBANKING_WITH_PLUGIN_IMEXPORTER_SWIFT, 1, [plugin availability])
      ;;
    xmldb)
      aqbanking_plugins_imexporters_dirs="$aqbanking_plugins_imexporters_dirs xmldb"
      aqbanking_plugins_imexporters_libs="$aqbanking_plugins_imexporters_libs xmldb/libabimexporters_xmldb.la"
      AC_DEFINE(AQBANKING_WITH_PLUGIN_IMEXPORTER_XMLDB, 1, [plugin availability])
      ;;
    yellownet)
      aqbanking_plugins_imexporters_dirs="$aqbanking_plugins_imexporters_dirs yellownet"
      aqbanking_plugins_imexporters_libs="$aqbanking_plugins_imexporters_libs yellownet/libabimexporters_yellownet.la"
      AC_DEFINE(AQBANKING_WITH_PLUGIN_IMEXPORTER_YELLOWNET, 1, [plugin availability])
      ;;
    sepa)
      aqbanking_plugins_imexporters_dirs="$aqbanking_plugins_imexporters_dirs sepa"
      aqbanking_plugins_imexporters_libs="$aqbanking_plugins_imexporters_libs sepa/libabimexporters_sepa.la"
      AC_DEFINE(AQBANKING_WITH_PLUGIN_IMEXPORTER_SEPA, 1, [plugin availability])
      ;;
    ctxfile)
      aqbanking_plugins_imexporters_dirs="$aqbanking_plugins_imexporters_dirs ctxfile"
      aqbanking_plugins_imexporters_libs="$aqbanking_plugins_imexporters_libs ctxfile/libabimexporters_ctxfile.la"
      AC_DEFINE(AQBANKING_WITH_PLUGIN_IMEXPORTER_CTXFILE, 1, [plugin availability])
      ;;
    q43)
      aqbanking_plugins_imexporters_dirs="$aqbanking_plugins_imexporters_dirs q43"
      aqbanking_plugins_imexporters_libs="$aqbanking_plugins_imexporters_libs q43/libabimexporters_q43.la"
      AC_DEFINE(AQBANKING_WITH_PLUGIN_IMEXPORTER_Q43, 1, [plugin availability])
      ;;
    camt)
      aqbanking_plugins_imexporters_dirs="$aqbanking_plugins_imexporters_dirs camt"
      aqbanking_plugins_imexporters_libs="$aqbanking_plugins_imexporters_libs camt/libabimexporters_camt.la"
      AC_DEFINE(AQBANKING_WITH_PLUGIN_IMEXPORTER_CAMT, 1, [plugin availability])
      ;;
    xml)
      aqbanking_plugins_imexporters_dirs="$aqbanking_plugins_imexporters_dirs xml"
      aqbanking_plugins_imexporters_libs="$aqbanking_plugins_imexporters_libs xml/libabimexporters_xml.la"
      AC_DEFINE(AQBANKING_WITH_PLUGIN_IMEXPORTER_XML, 1, [plugin availability])
      ;;
    *)
      AC_MSG_ERROR("ERROR: Unknown plugin \"$f\"")
      ;;
  esac
done

AC_SUBST(aqbanking_plugins_imexporters_dirs)
AC_SUBST(aqbanking_plugins_imexporters_libs)



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
  aqbanking_parsers="swift"
fi

for f in ${aqbanking_parsers}; do
  case "$f" in
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
  aqbanking_bankinfos="de"
fi

for f in ${aqbanking_bankinfos}; do
  case "$f" in
    de)
      aqbanking_plugins_bankinfo_dirs="$aqbanking_plugins_bankinfo_dirs de"
      aqbanking_plugins_bankinfo_libs="$aqbanking_plugins_bankinfo_libs de/libbankinfo_de.la"
      AC_DEFINE(AQBANKING_WITH_PLUGIN_BANKINFO_DE, 1, [plugin availability])
      ;;
    *)
      AC_MSG_ERROR("ERROR: Unknown plugin \"$f\"")
      ;;
  esac
done

AC_SUBST(aqbanking_plugins_bankinfo_dirs)
AC_SUBST(aqbanking_plugins_bankinfo_libs)



###-------------------------------------------------------------------------
#
# Check for Gwen
#

# Variable used in aqbanking-config.cmake
AC_SUBST(GWENHYWFAR_MINIMUM_VERSION, "5.11.2.0")
AC_GWENHYWFAR(5,11,2,0)

all_includes="$all_includes $gwenhywfar_includes"

AC_MSG_CHECKING(if gui-tests desired)
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
aqbanking_internal_libs="${abs_top_builddir}/src/libs/libaqbanking.la"
aqbanking_ldflags="-L\${libdir}"
aqbanking_libs="-laqbanking"
aqbanking_includes="-I\${includedir}/aqbanking6"
aqbanking_headerdir_am="\$(includedir)/aqbanking6"
aqbanking_headerdir="\${includedir}/aqbanking6"
aqbanking_plugindir="\${libdir}/aqbanking/plugins/${AQBANKING_SO_EFFECTIVE}"
aqbanking_pkgdatadir="\${datadir}/aqbanking"
aqbanking_symlinkdir="\$(top_builddir)/aqbanking6"

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


# special variables for AqFinTS
aqfints_datadir="\$(pkgdatadir)/backends/aqfints"
AC_SUBST(aqfints_datadir)



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
# Compiler check for symbol visibility
#
AC_CACHE_CHECK([whether compiler supports symbol visibility],
  ac_cv_hidden_visibility_attribute, [
    echo 'int __attribute__ ((visibility ("hidden"))) foo (void) { return 1; }' > conftest.c
    ac_cv_hidden_visibility_attribute=no
    if AC_TRY_COMMAND(${CC-cc} -Werror -S conftest.c -o conftest.s 1>&AS_MESSAGE_LOG_FD);
    then
      if grep '\.hidden.*foo' conftest.s >/dev/null;
      then
        ac_cv_hidden_visibility_attribute=yes
      fi
    fi
    rm -f conftest.*
  ])
if test $ac_cv_hidden_visibility_attribute = yes;
then
  AC_DEFINE(GCC_WITH_VISIBILITY_ATTRIBUTE, 1, [visibility])
  visibility_cflags="-fvisibility=hidden"
fi
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

aqbanking_modules=". types"
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
        DOXYGEN_EXCLUDE_PATTERNS=""
else
	DOXYGEN_DEFINE="DOXYGEN_HIDE"
	DOXYGEN_INPUT="listdoc.h \
                       aqbanking6/banking.h \
                       aqbanking6/banking_bankinfo.h \
                       aqbanking6/banking_dialogs.h \
                       aqbanking6/banking_online.h \
                       aqbanking6/banking_transaction.h \
                       aqbanking6/error.h \
                       aqbanking6/account_type.h \
                       aqbanking6/value.h \
                       aqbanking6/account_spec.h \
                       aqbanking6/balance.h \
                       aqbanking6/bankinfo.h \
                       aqbanking6/bankinfoservice.h \
                       aqbanking6/document.h \
                       aqbanking6/imexporter_accountinfo.h \
                       aqbanking6/imexporter_context.h \
                       aqbanking6/message.h \
                       aqbanking6/security.h \
                       aqbanking6/transaction.h \
                       aqbanking6/transactionlimits.h\
                      "
        DOXYGEN_FILE_PATTERNS="*.h *.hpp"
        DOXYGEN_EXCLUDE_PATTERNS="*_p.h"
fi
AC_SUBST(DOXYGEN_INPUT)
AC_SUBST(DOXYGEN_DEFINE)
AC_SUBST(DOXYGEN_FILE_PATTERNS)
AC_SUBST(DOXYGEN_EXCLUDE_PATTERNS)
AC_MSG_RESULT($enable_fulldoc)



###-------------------------------------------------------------------------
#
# search for tag files
#
AC_MSG_CHECKING(doxygen tag files)
DOXYGEN_TAGFILES=""
DOXYGEN_TAGFILEPATHS=""
if test -d "${docpath}"; then
  tagfiles="`cd ${docpath} && ls *.tag`"
  if test -n "${tagfiles}"; then
    # remove own package from list
    tagfiles="`echo ${tagfiles} | ${SED} -e s/${PACKAGE}.tag//`"

    # add every remaining tag file
    for ff in ${tagfiles}; do
      DOXYGEN_TAGFILES="${DOXYGEN_TAGFILES} ${docpath}/${ff}=${docpath}/`basename -s .tag ${ff}`"
    done
  fi
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
aqhbci_internal_libs="${abs_top_builddir}/src/libs/plugins/backends/aqhbci/libaqhbci.la"
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
aqofxconnect_internal_libs="${abs_top_builddir}/src/libs/plugins/backends/aqofxconnect/libaqofxconnect.la"
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
# Configure stuff for backend AqPaypal
#

# Common settings
aqpaypal_internal_libs="${abs_top_builddir}/src/libs/plugins/backends/aqpaypal/libaqpaypal.la"
aqpaypal_includes="${aqbanking_includes}"
aqpaypal_libs="-L\${libdir} -laqpaypal"

case "$aqbanking_backends" in *aqpaypal*)
  case "$OS_TYPE" in
      posix)
          ;;
      windows)
          AC_DEFINE_UNQUOTED(BUILDING_AQPAYPAL_DLL,1, [Define if DLL is built])
          ;;
  esac
  ;;
esac

AS_SCRUB_INCLUDE(aqpaypal_includes)
AC_SUBST(aqpaypal_includes)
AC_SUBST(aqpaypal_internal_libs)
AC_SUBST(aqpaypal_libs)
#AC_DEFINE_UNQUOTED(AQPAYPAL_DATA, "${aqpaypal_data}", [data dir])


###-------------------------------------------------------------------------
#
# Configure stuff for backend AqEBICS
#

# Common settings; can be overridden per-OS below.
aqebics_internal_libs="${abs_top_builddir}/src/libs/plugins/backends/aqebics/plugin/libaqebics.la"
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
  PKG_CHECK_MODULES(XMLSEC,  [xmlsec1 >= 1.0.0])
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
  admin/Makefile
  doc/Makefile
  m4/Makefile
  src/Makefile
  src/libs/Makefile
  src/libs/aqbanking/Makefile
  src/libs/aqbanking/version.h
  src/libs/aqbanking/system.h
  src/libs/aqbanking/dialogs/Makefile
  src/libs/aqbanking/types/Makefile
  src/libs/aqbanking/gui/Makefile
  src/libs/aqbanking/typemaker2/Makefile
  src/libs/aqbanking/typemaker2/c/Makefile
  src/libs/aqbanking/backendsupport/Makefile
  src/libs/aqbankingpp/Makefile
  src/libs/plugins/Makefile
  src/libs/plugins/bankinfo/Makefile
  src/libs/plugins/bankinfo/de/Makefile
  src/libs/plugins/bankinfo/de/de.xml
  src/libs/plugins/bankinfo/generic/Makefile
  src/libs/plugins/imexporters/Makefile
  src/libs/plugins/imexporters/eri2/Makefile
  src/libs/plugins/imexporters/eri2/eri2.xml
  src/libs/plugins/imexporters/eri2/profiles/Makefile
  src/libs/plugins/imexporters/ofx/Makefile
  src/libs/plugins/imexporters/ofx/ofx.xml
  src/libs/plugins/imexporters/ofx/parser/Makefile
  src/libs/plugins/imexporters/ofx/profiles/Makefile
  src/libs/plugins/imexporters/swift/Makefile
  src/libs/plugins/imexporters/swift/swift.xml
  src/libs/plugins/imexporters/swift/profiles/Makefile
  src/libs/plugins/imexporters/csv/Makefile
  src/libs/plugins/imexporters/csv/csv.xml
  src/libs/plugins/imexporters/csv/profiles/Makefile
  src/libs/plugins/imexporters/openhbci1/Makefile
  src/libs/plugins/imexporters/openhbci1/openhbci1.xml
  src/libs/plugins/imexporters/openhbci1/profiles/Makefile
  src/libs/plugins/imexporters/qif/Makefile
  src/libs/plugins/imexporters/qif/qif.xml
  src/libs/plugins/imexporters/qif/profiles/Makefile
  src/libs/plugins/imexporters/yellownet/Makefile
  src/libs/plugins/imexporters/yellownet/yellownet.xml
  src/libs/plugins/imexporters/yellownet/profiles/Makefile
  src/libs/plugins/imexporters/xmldb/Makefile
  src/libs/plugins/imexporters/xmldb/xmldb.xml
  src/libs/plugins/imexporters/xmldb/profiles/Makefile
  src/libs/plugins/imexporters/sepa/Makefile
  src/libs/plugins/imexporters/sepa/sepa.xml
  src/libs/plugins/imexporters/sepa/profiles/Makefile
  src/libs/plugins/imexporters/ctxfile/Makefile
  src/libs/plugins/imexporters/ctxfile/ctxfile.xml
  src/libs/plugins/imexporters/ctxfile/profiles/Makefile
  src/libs/plugins/imexporters/q43/Makefile
  src/libs/plugins/imexporters/q43/q43.xml
  src/libs/plugins/imexporters/q43/profiles/Makefile
  src/libs/plugins/imexporters/camt/Makefile
  src/libs/plugins/imexporters/camt/camt.xml
  src/libs/plugins/imexporters/camt/profiles/Makefile
  src/libs/plugins/imexporters/xml/Makefile
  src/libs/plugins/imexporters/xml/xml.xml
  src/libs/plugins/imexporters/xml/data/Makefile
  src/libs/plugins/imexporters/xml/profiles/Makefile
  src/libs/plugins/parsers/Makefile
  src/libs/plugins/parsers/swift/Makefile
  src/libs/plugins/parsers/swift/swift.xml
  src/libs/plugins/backends/Makefile
  src/libs/plugins/backends/aqhbci/Makefile
  src/libs/plugins/backends/aqhbci/aqhbci.xml
  src/libs/plugins/backends/aqhbci/dialogs/Makefile
  src/libs/plugins/backends/aqhbci/header.xml
  src/libs/plugins/backends/aqhbci/banking/Makefile
  src/libs/plugins/backends/aqhbci/msglayer/Makefile
  src/libs/plugins/backends/aqhbci/tan/Makefile
  src/libs/plugins/backends/aqhbci/joblayer/Makefile
  src/libs/plugins/backends/aqhbci/applayer/Makefile
  src/libs/plugins/backends/aqhbci/ajobs/Makefile
  src/libs/plugins/backends/aqhbci/admjobs/Makefile
  src/libs/plugins/backends/aqhbci/applayer/xml/Makefile
  src/libs/plugins/backends/aqhbci/control/Makefile
  src/libs/plugins/backends/aqhbci/version.h
  src/libs/plugins/backends/aqofxconnect/Makefile
  src/libs/plugins/backends/aqofxconnect/aqofxconnect.xml
  src/libs/plugins/backends/aqofxconnect/libofxhome/Makefile
  src/libs/plugins/backends/aqofxconnect/v1/Makefile
  src/libs/plugins/backends/aqofxconnect/v2/Makefile
  src/libs/plugins/backends/aqofxconnect/control/Makefile
  src/libs/plugins/backends/aqofxconnect/common/Makefile
  src/libs/plugins/backends/aqofxconnect/dialogs/Makefile
  src/libs/plugins/backends/aqpaypal/Makefile
  src/libs/plugins/backends/aqpaypal/aqpaypal.xml
  src/libs/plugins/backends/aqpaypal/control/Makefile
  src/libs/plugins/backends/aqnone/Makefile
  src/libs/plugins/backends/aqnone/aqnone.xml
  src/libs/plugins/backends/aqgivve/Makefile
  src/libs/plugins/backends/aqgivve/aqgivve.xml
  src/libs/plugins/backends/aqebics/Makefile
  src/libs/plugins/backends/aqebics/aqebics.xml
  src/libs/plugins/backends/aqebics/client/Makefile
  src/libs/plugins/backends/aqebics/dialogs/Makefile
  src/libs/plugins/backends/aqebics/msg/Makefile
  src/libs/plugins/backends/aqebics/control/Makefile
  src/libs/plugins/backends/aqebics/requests/Makefile
  src/libs/plugins/backends/aqfints/Makefile
  src/libs/plugins/backends/aqfints/aqfints.xml
  src/libs/plugins/backends/aqfints/banking/Makefile
  src/libs/plugins/backends/aqfints/banking/control/Makefile
  src/libs/plugins/backends/aqfints/libaqfints/Makefile
  src/libs/plugins/backends/aqfints/libaqfints/msg/Makefile
  src/libs/plugins/backends/aqfints/libaqfints/parser/Makefile
  src/libs/plugins/backends/aqfints/libaqfints/service/Makefile
  src/libs/plugins/backends/aqfints/libaqfints/service/xml/Makefile
  src/libs/plugins/backends/aqfints/libaqfints/service/bpd/Makefile
  src/libs/plugins/backends/aqfints/libaqfints/service/upd/Makefile
  src/libs/plugins/backends/aqfints/libaqfints/service/jobs/Makefile
  src/libs/plugins/backends/aqfints/libaqfints/transport/Makefile
  src/libs/plugins/backends/aqfints/libaqfints/session/Makefile
  src/libs/plugins/backends/aqfints/libaqfints/session/pintan/Makefile
  src/libs/plugins/backends/aqfints/libaqfints/session/hbci/Makefile
  src/test/Makefile
  src/tools/Makefile
  src/tools/aqbanking-cli/Makefile
  src/tools/mkdeinfo/Makefile
  tutorials/Makefile
  po/Makefile
  Doxyfile
  Makefile
  aqbanking-config.in
  aqbanking.pc
  aqbanking.spec
  aqbanking.iss
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

echo "----------------------------------------------------"
echo "You can now run \"make\" (or gmake on FreeBSD) to compile."


