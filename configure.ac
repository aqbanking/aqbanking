# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.56)
AC_INIT
AC_CANONICAL_TARGET([])

AC_CONFIG_SRCDIR([README])
AC_CONFIG_HEADER([config.h])



#
# versions
#
AQBANKING_VERSION_MAJOR=1
AQBANKING_VERSION_MINOR=0
AQBANKING_VERSION_PATCHLEVEL=6
AQBANKING_VERSION_BUILD=4
dnl "stable", "rcX", "betaX", "cvs"
AQBANKING_VERSION_TAG="cvs"
AQBANKING_VERSION_FULL_STRING="$AQBANKING_VERSION_MAJOR.$AQBANKING_VERSION_MINOR.$AQBANKING_VERSION_PATCHLEVEL.$AQBANKING_VERSION_BUILD${AQBANKING_VERSION_TAG}"
AQBANKING_VERSION_STRING="$AQBANKING_VERSION_MAJOR.$AQBANKING_VERSION_MINOR.$AQBANKING_VERSION_PATCHLEVEL"


#
# SO version
#
AQBANKING_SO_CURRENT=1
AQBANKING_SO_REVISION=7
AQBANKING_SO_AGE=1
AQBANKING_SO_EFFECTIVE="`echo \$(($AQBANKING_SO_CURRENT-$AQBANKING_SO_AGE))`"

#
# Create release string
#
case "$AQBANKING_VERSION_TAG" in
  cvs)
     AQBANKING_VERSION_RELEASE_STRING="$AQBANKING_VERSION_FULL_STRING"
     ;;
  stable)
     AQBANKING_VERSION_RELEASE_STRING="$AQBANKING_VERSION_MAJOR.$AQBANKING_VERSION_MINOR.$AQBANKING_VERSION_PATCHLEVEL"
     ;;
  *)
     AQBANKING_VERSION_RELEASE_STRING="$AQBANKING_VERSION_MAJOR.$AQBANKING_VERSION_MINOR.$AQBANKING_VERSION_PATCHLEVEL"
     
     # add TAG
     AQBANKING_VERSION_RELEASE_STRING="$AQBANKING_VERSION_RELEASE_STRING${AQBANKING_VERSION_TAG}"
     ;;
esac
   
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

AC_DEFINE_UNQUOTED(AQBANKING_VERSION_MAJOR,$AQBANKING_VERSION_MAJOR, 
                   [major version])
AC_DEFINE_UNQUOTED(AQBANKING_VERSION_MINOR,$AQBANKING_VERSION_MINOR, 
                   [minor version])
AC_DEFINE_UNQUOTED(AQBANKING_VERSION_PATCHLEVEL,$AQBANKING_VERSION_PATCHLEVEL, 
                   [patchlevel])
AC_DEFINE_UNQUOTED(AQBANKING_VERSION_BUILD,$AQBANKING_VERSION_BUILD, [build])
AC_DEFINE_UNQUOTED(AQBANKING_VERSION_STRING,"$AQBANKING_VERSION_STRING", 
                   [version string])
AC_DEFINE_UNQUOTED(AQBANKING_VERSION_FULL_STRING,"$AQBANKING_VERSION_FULL_STRING",
                   [full version string])
AC_DEFINE_UNQUOTED(AQBANKING_VERSION_TAG,$AQBANKING_VERSION_TAG, [tag])

AC_SUBST(AQBANKING_SO_CURRENT)
AC_SUBST(AQBANKING_SO_REVISION)
AC_SUBST(AQBANKING_SO_AGE)
AC_SUBST(AQBANKING_SO_EFFECTIVE)


AM_INIT_AUTOMAKE(aqbanking, $AQBANKING_VERSION_RELEASE_STRING)


###-------------------------------------------------------------------------
dnl prefix handling
AC_PREFIX_DEFAULT(/usr)
if test "x$prefix" = "xNONE"; then
  prefix=$ac_default_prefix
  ac_configure_args="$ac_configure_args --prefix $prefix"
fi
AC_SUBST(prefix)


###-------------------------------------------------------------------------
# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_CHECK_PROG(USE_DOT,dot,YES,NO)
AC_CHECK_PROG(SED,sed,sed)

# Checks for libraries.

###-------------------------------------------------------------------------
# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h locale.h])

###-------------------------------------------------------------------------
# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_STRUCT_TM

###-------------------------------------------------------------------------
# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([memmove memset strcasecmp strdup strerror snprintf])
AC_CHECK_FUNCS([setlocale])


###-------------------------------------------------------------------------
AC_GWENHYWFAR(1,9,0,0)

all_includes="$all_includes $gwenhywfar_includes"


###-------------------------------------------------------------------------
# KtoBlzCheck

have_ktoblzcheck="yes"
AC_CHECK_LIB(ktoblzcheck, AccountNumberCheck_new,
             [], [have_ktoblzcheck="no"])
AC_CHECK_HEADERS(ktoblzcheck.h, [], [have_ktoblzcheck="no"])
if test "$have_ktoblzcheck" != "yes"; then
  ktoblzcheck_libs=""
else
  ktoblzcheck_libs="-lktoblzcheck"
  AC_DEFINE_UNQUOTED(HAVE_KTOBLZCHECK, 1, [if KTOBLZCHECK is available])
fi
AC_SUBST(ktoblzcheck_libs)

AM_CONDITIONAL(WITH_KTOBLZCHECK, [test "$have_ktoblzcheck" = "yes"])


###-------------------------------------------------------------------------
# LibOFX

HAVE_OFX="yes"
AC_CHECK_LIB(ofx, libofx_proc_buffer, [], [HAVE_OFX="no"])
AC_CHECK_HEADERS([libofx/libofx.h], [], [HAVE_OFX="no"])
if test "$HAVE_OFX" != "yes"; then
  AC_MSG_WARN([
*** LibOFX >=0.7 not found, the OFX plugin will not be compiled.
*** Please get LibOFX from http://libofx.sf.net/.
*** Make sure that the libofx-devel packages are also installed.])
fi

AM_CONDITIONAL(WITH_OFX_PLUGIN, [test "$HAVE_OFX" = "yes"])


###-------------------------------------------------------------------------
dnl OS dependant settings

AQ_CHECK_OS

case "$OS_TYPE" in
    posix)
        aqbanking_internal_libs="-laqbanking"
	aqbanking_ldflags="-L\${libdir}"
        aqbanking_libs="${aqbanking_internal_libs}"
        aqbanking_libspp="-laqbankingpp"
        aqbanking_includes="-I`eval echo $prefix/include`"
        aqbanking_plugindir="\${libdir}/aqbanking/plugins/${AQBANKING_SO_EFFECTIVE}"
        aqbanking_pkgdatadir="`eval echo $prefix/share`/aqbanking"
    	;;
    windows)
        AC_DEFINE_UNQUOTED(BUILDING_AQBANKING_DLL,1, [Define if DLL is built])
        ACX_WINDOWS_PATHS
        aqbanking_internal_libs="-laqbanking32_${AQBANKING_SO_EFFECTIVE}"
	aqbanking_ldflags="-L\${libdir}"
        aqbanking_libs="${aqbanking_internal_libs}"
        aqbanking_libspp="-laqbankingpp32"
        aqbanking_includes="-I`eval echo $prefix/include`"
        aqbanking_plugindir="\${libdir}/aqbanking/plugins/${AQBANKING_SO_EFFECTIVE}"
        aqbanking_pkgdatadir="`eval echo $prefix/share`/aqbanking"
    	;;
esac
AM_CONDITIONAL(IS_WINDOWS, [test "$OS_TYPE" = "windows"])


###-------------------------------------------------------------------------
#
# check for I18N
#
HAVE_I18N="yes"
AC_CHECK_PROG(XGETTEXT,xgettext,xgettext, missing)
if test "$XGETTEXT" != "missing"; then
  AC_CHECK_HEADERS([locale.h libintl.h], [], [HAVE_I18N="no"])
  AC_SEARCH_LIBS(gettext, intl, [], [HAVE_I18N="no"])
else
  HAVE_I18N="no"
  AC_MSG_WARN([xgettext is missing. Locale suport is disabled.])
fi
AC_SUBST(HAVE_I18N)
if test "$HAVE_I18N" = "yes"; then
  AC_DEFINE_UNQUOTED(HAVE_I18N, 1, [whether I18N is available])
fi
AM_CONDITIONAL(USE_I18N, [test "$HAVE_I18N" = "yes"])



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
AS_SCRUB_INCLUDE(all_includes)
AC_SUBST(all_includes)
AC_SUBST(all_libraries)
AS_SCRUB_INCLUDE(aqbanking_includes)
AC_SUBST(aqbanking_includes)
AC_SUBST(aqbanking_internal_libs)
AC_SUBST(aqbanking_ldflags)
AC_SUBST(aqbanking_libs)
AC_SUBST(aqbanking_libspp)
AC_SUBST(aqbanking_plugindir)
AC_SUBST(aqbanking_pkgdatadir)
AC_DEFINE_UNQUOTED(AQBANKING_DATA, "${aqbanking_pkgdatadir}", [data dir])


# Check for additional aclocal flags
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
# create header directory, make symlinks
#
aqbanking_modules=". jobs types"

lprefix="${srcdir}"
lprefix=`( cd ${lprefix} ; pwd )`

rm -Rf aqbanking
mkdir -p aqbanking
AQ_HEADER_FILES="${lprefix}/version.h"

# symlink all headers from src/libs
hfiles=`( cd "${srcdir}/src/libs/" && ls *.h 2>/dev/null )`
for f in ${hfiles}; do
  case ${f} in
    *_p.h | *_l.h)
      ;;
    *)
      AQ_HEADER_FILES="${AQ_HEADER_FILES} ${lprefix}/src/libs/${f}"
      ln -s "${lprefix}/src/libs/${f}" "aqbanking/${f}"
      ;;
  esac
done

# symlink all headers from src/libs/*
for sm in $aqbanking_modules; do
  hfiles=`( cd "${srcdir}/src/libs/aqbanking/${sm}" && ls *.h 2>/dev/null )`
  for f in ${hfiles}; do
    case ${f} in
      *_p.h | *_l.h)
         ;;
      *)   
         AQ_HEADER_FILES="${AQ_HEADER_FILES} ${lprefix}/src/libs/aqbanking/${sm}/${f}"
         ln -s "${lprefix}/src/libs/aqbanking/${sm}/${f}" "aqbanking/${f}"
         ;;
    esac
  done
done

ln -s "${lprefix}/version.h" aqbanking/version.h



# symlink all headers from src/libs/aqbanking++
aqbankingpp_modules="."
rm -Rf aqbanking++
mkdir aqbanking++

# symlink all headers from src/libs/*
for sm in $aqbankingpp_modules; do
  hfiles=`( cd "${srcdir}/src/libs/aqbanking++/${sm}" && ls *.h 2>/dev/null )`
  for f in ${hfiles}; do
    case ${f} in
      *_p.h | *_l.h)
         ;;
      *)   
         AQ_HEADER_FILES="${AQ_HEADER_FILES} src/libs/aqbanking++/${sm}/${f}"
         ln -s "${lprefix}/src/libs/aqbanking++/${sm}/${f}" "aqbanking++/${f}"
         ;;
    esac
  done
done
AC_SUBST(AQ_HEADER_FILES)


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
else
	DOXYGEN_DEFINE="DOXYGEN_HIDE"
	DOXYGEN_INPUT="listdoc.h ${AQ_HEADER_FILES}"
fi
AC_SUBST(DOXYGEN_INPUT)
AC_SUBST(DOXYGEN_DEFINE)
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
AC_CONFIG_FILES([
  doc/Makefile
  m4/Makefile
  src/Makefile
  src/libs/Makefile
  src/libs/aqbanking/Makefile
  src/libs/aqbanking/ressource.rc
  src/libs/aqbanking/jobs/Makefile
  src/libs/aqbanking/types/Makefile
  src/libs/aqbanking++/Makefile
  src/plugins/Makefile
  src/plugins/bankinfo/Makefile
  src/plugins/bankinfo/de/Makefile
  src/plugins/bankinfo/de/de.xml
  src/plugins/imexporters/Makefile
  src/plugins/imexporters/dbio/Makefile
  src/plugins/imexporters/dbio/dbio.xml
  src/plugins/imexporters/dtaus/Makefile
  src/plugins/imexporters/dtaus/dtaus.xml
  src/plugins/imexporters/dtaus/ressource.rc
  src/plugins/imexporters/dtaus/profiles/Makefile
  src/plugins/imexporters/dtaus/profiles/default.conf
  src/plugins/imexporters/ofx/Makefile
  src/plugins/imexporters/ofx/ofx.xml
  src/plugins/imexporters/ofx/ressource.rc
  src/plugins/imexporters/ofx/profiles/Makefile
  src/plugins/imexporters/ofx/profiles/default.conf
  src/plugins/imexporters/swift/Makefile
  src/plugins/imexporters/swift/swift.xml
  src/plugins/imexporters/swift/ressource.rc
  src/plugins/imexporters/swift/profiles/Makefile
  src/plugins/imexporters/swift/profiles/swiftmt940.conf
  src/plugins/imexporters/swift/profiles/swiftmt942.conf
  src/plugins/imexporters/csv/Makefile
  src/plugins/imexporters/csv/csv.xml
  src/plugins/imexporters/csv/ressource.rc
  src/plugins/imexporters/csv/profiles/Makefile
  src/plugins/imexporters/csv/profiles/default.conf
  src/plugins/imexporters/csv/profiles/aqmoney2.conf
  src/plugins/imexporters/openhbci1/Makefile
  src/plugins/imexporters/openhbci1/openhbci1.xml
  src/plugins/imexporters/openhbci1/ressource.rc
  src/plugins/imexporters/openhbci1/profiles/Makefile
  src/plugins/imexporters/openhbci1/profiles/default.conf
  src/plugins/imexporters/qif/Makefile
  src/plugins/imexporters/qif/qif.xml
  src/plugins/imexporters/qif/ressource.rc
  src/plugins/imexporters/qif/profiles/Makefile
  src/plugins/imexporters/qif/profiles/default.conf
  src/plugins/parsers/Makefile
  src/plugins/parsers/dtaus/Makefile
  src/plugins/parsers/dtaus/dtaus.xml
  src/plugins/parsers/dtaus/ressource.rc
  src/plugins/parsers/swift/Makefile
  src/plugins/parsers/swift/swift.xml
  src/plugins/parsers/swift/ressource.rc
  src/test/Makefile
  po/Makefile
  Doxyfile
  Makefile
  aqbanking-config.in
  aqbanking.spec
  aqbanking.iss
  version.h
])
AC_OUTPUT

