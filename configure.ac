# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.56)
AC_INIT
AC_CANONICAL_TARGET([])

AC_CONFIG_SRCDIR([README])
AC_CONFIG_HEADER([config.h])



###-------------------------------------------------------------------------
#
# AqBanking versions
#
AQBANKING_VERSION_MAJOR=1
AQBANKING_VERSION_MINOR=2
AQBANKING_VERSION_PATCHLEVEL=0
AQBANKING_VERSION_BUILD=7
dnl "stable", "rcX", "betaX", "cvs"
AQBANKING_VERSION_TAG="cvs"
AQBANKING_VERSION_FULL_STRING="$AQBANKING_VERSION_MAJOR.$AQBANKING_VERSION_MINOR.$AQBANKING_VERSION_PATCHLEVEL.$AQBANKING_VERSION_BUILD${AQBANKING_VERSION_TAG}"
AQBANKING_VERSION_STRING="$AQBANKING_VERSION_MAJOR.$AQBANKING_VERSION_MINOR.$AQBANKING_VERSION_PATCHLEVEL"


###-------------------------------------------------------------------------
#
# AqBanking SO version
#
AQBANKING_SO_CURRENT=8
AQBANKING_SO_AGE=8
AQBANKING_SO_REVISION=0
AQBANKING_SO_EFFECTIVE="`echo \$(($AQBANKING_SO_CURRENT-$AQBANKING_SO_AGE))`"


###-------------------------------------------------------------------------
#
# AqHBCI versions
#
AQHBCI_VERSION_MAJOR=1
AQHBCI_VERSION_MINOR=2
AQHBCI_VERSION_PATCHLEVEL=0
AQHBCI_VERSION_BUILD=4
dnl "stable", "rcX", "betaX", "cvs"
AQHBCI_VERSION_TAG="cvs"
AQHBCI_VERSION_FULL_STRING="$AQHBCI_VERSION_MAJOR.$AQHBCI_VERSION_MINOR.$AQHBCI_VERSION_PATCHLEVEL.$AQHBCI_VERSION_BUILD${AQHBCI_VERSION_TAG}"
AQHBCI_VERSION_STRING="$AQHBCI_VERSION_MAJOR.$AQHBCI_VERSION_MINOR.$AQHBCI_VERSION_PATCHLEVEL"


###-------------------------------------------------------------------------
#
# AqHBCI SO version
#
AQHBCI_SO_CURRENT=6
AQHBCI_SO_AGE=4
AQHBCI_SO_REVISION=0
AQHBCI_SO_EFFECTIVE="`echo \$(($AQHBCI_SO_CURRENT-$AQHBCI_SO_AGE))`"



###-------------------------------------------------------------------------
#
# datetime
#
DATETIME="`date +\"%Y%m%d%H%M%S\"`"
AC_SUBST(DATETIME)


###-------------------------------------------------------------------------
#
# Create AqBanking release string
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
# for AqHBCI
#
case "$AQHBCI_VERSION_TAG" in
  cvs)
     AQHBCI_VERSION_RELEASE_STRING="$AQHBCI_VERSION_FULL_STRING"
     ;;
  stable)
     AQHBCI_VERSION_RELEASE_STRING="$AQHBCI_VERSION_MAJOR.$AQHBCI_VERSION_MINOR.$AQHBCI_VERSION_PATCHLEVEL"
     ;;
  *)
     AQHBCI_VERSION_RELEASE_STRING="$AQHBCI_VERSION_MAJOR.$AQHBCI_VERSION_MINOR.$AQHBCI_VERSION_PATCHLEVEL"
     
     # add TAG
     AQHBCI_VERSION_RELEASE_STRING="$AQHBCI_VERSION_RELEASE_STRING${AQHBCI_VERSION_TAG}"
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

#AC_DEFINE_UNQUOTED(AQBANKING_VERSION_MAJOR,$AQBANKING_VERSION_MAJOR, 
#                   [major version])
#AC_DEFINE_UNQUOTED(AQBANKING_VERSION_MINOR,$AQBANKING_VERSION_MINOR, 
#                   [minor version])
#AC_DEFINE_UNQUOTED(AQBANKING_VERSION_PATCHLEVEL,$AQBANKING_VERSION_PATCHLEVEL, 
#                   [patchlevel])
#AC_DEFINE_UNQUOTED(AQBANKING_VERSION_BUILD,$AQBANKING_VERSION_BUILD, [build])
#AC_DEFINE_UNQUOTED(AQBANKING_VERSION_STRING,"$AQBANKING_VERSION_STRING", 
#                   [version string])
#AC_DEFINE_UNQUOTED(AQBANKING_VERSION_FULL_STRING,"$AQBANKING_VERSION_FULL_STRING",
#                   [full version string])
#AC_DEFINE_UNQUOTED(AQBANKING_VERSION_TAG,$AQBANKING_VERSION_TAG, [tag])

AC_SUBST(AQBANKING_SO_CURRENT)
AC_SUBST(AQBANKING_SO_REVISION)
AC_SUBST(AQBANKING_SO_AGE)
AC_SUBST(AQBANKING_SO_EFFECTIVE)


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


AM_INIT_AUTOMAKE(aqbanking, $AQBANKING_VERSION_RELEASE_STRING)


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
# Checks for programs.
#
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL
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
AC_FUNC_MALLOC
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([memmove memset strcasecmp strdup strerror snprintf])
AC_CHECK_FUNCS([setlocale])


###-------------------------------------------------------------------------
#
# Check for Gwen
#
AC_GWENHYWFAR(1,14,0,5)

all_includes="$all_includes $gwenhywfar_includes"
XMLMERGE="$gwenhywfar_dir/bin/xmlmerge"


###-------------------------------------------------------------------------
#
# KtoBlzCheck
#

have_ktoblzcheck="yes"
AC_CHECK_LIB(ktoblzcheck, AccountNumberCheck_new,
             [have_ktoblzcheck="yes"], [have_ktoblzcheck="no"])
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
#
# LibOFX
#

HAVE_OFX_WITH_CONNECT="no"
AC_MSG_CHECKING(whether OFX should be used)
AC_ARG_ENABLE(libofx,
  [  --enable-libofx         use OFX (default=auto)],
  [ case "${enableval}" in
    yes) enable_libofx="yes";;
    no)  enable_libofx="no";;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-libofx);;
    esac
  ],
  enable_libofx="yes")
AC_MSG_RESULT($enable_libofx)

if test "$enable_libofx" = "yes"; then
  HAVE_OFX="yes"
  AC_CHECK_LIB(ofx, libofx_proc_buffer, [HAVE_OFX="yes"], [HAVE_OFX="no"])
  AC_CHECK_HEADERS([libofx/libofx.h], [], [HAVE_OFX="no"])
  if test "$HAVE_OFX" != "yes"; then
    AC_MSG_WARN([
  *** LibOFX >=0.7 not found, the OFX plugin will not be compiled.
  *** Please get LibOFX from http://libofx.sf.net/.
  *** Make sure that the libofx-devel packages are also installed.])
  else
    AC_CHECK_LIB(ofx, libofx_request_statement, [HAVE_OFX_WITH_CONNECT="yes"])
    if test "$HAVE_OFX_WITH_CONNECT" = "yes"; then
       AC_DEFINE_UNQUOTED(HAVE_OFX_WITH_CONNECT, 1, [whether direct connect is available])
    fi
  fi
else
  HAVE_OFX="no"
fi

AM_CONDITIONAL(WITH_OFX_PLUGIN, [test "$HAVE_OFX" = "yes"])


###-------------------------------------------------------------------------
#
# Python
#

AC_MSG_CHECKING(whether to install python-bindings)
AC_ARG_ENABLE(python,
  [  --enable-python         install python bindings (default=no)],
  [ case "${enableval}" in
    yes) enable_python="yes";;
    no)  enable_python="no";;
    *) AC_MSG_ERROR(bad value ${enableval} for --enable-python);;
    esac
  ],
  enable_python="no")

if test "x$enable_python" = "xyes"; then
  AC_MSG_RESULT(yes)
  # Make sure at least version 2.3 is available (otherwise ctypes
  # won't work)
  AM_PATH_PYTHON( [2.3] )

  # Check for python ctypes module, otherwise terminate
  AC_PYTHON_MODULE( [ctypes], [required] )
else
  AC_MSG_RESULT(no)
fi
AM_CONDITIONAL(PYTHON_USE, [test "$enable_python" = "yes"])


###-------------------------------------------------------------------------
#
# OS dependant settings
#
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
#
# Subst
#
AS_SCRUB_INCLUDE(aqbanking_includes)
AC_SUBST(aqbanking_includes)
AC_SUBST(aqbanking_internal_libs)
AC_SUBST(aqbanking_ldflags)
AC_SUBST(aqbanking_libs)
AC_SUBST(aqbanking_libspp)
AC_SUBST(aqbanking_plugindir)
AC_SUBST(aqbanking_pkgdatadir)
AC_DEFINE_UNQUOTED(AQBANKING_DATA, "${aqbanking_pkgdatadir}", [data dir])


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
# create header directory, make symlinks
#

#
# settings
#
aqbanking_modules=". jobs types"
aqbankingpp_modules="."
aqbanking_frontends="cbanking"
aqbanking_backends="aqhbci"

#
# code
#
rm -Rf headers 2>/dev/null

lprefix="${srcdir}"
lprefix=`( cd ${lprefix} ; pwd )`

mkdir -p headers/aqbanking
AQ_HEADER_FILES="${lprefix}/version.h"


#
# symlink all headers from src/libs
#
hfiles=`( cd "${srcdir}/src/libs/" && ls *.h 2>/dev/null )`
for f in ${hfiles}; do
  case ${f} in
    *_p.h | *_l.h)
      ;;
    *)
      AQ_HEADER_FILES="${AQ_HEADER_FILES} ${lprefix}/src/libs/${f}"
      ln -s "${lprefix}/src/libs/${f}" "headers/aqbanking/${f}"
      ;;
  esac
done


#
# symlink all headers from src/libs/*
#
for sm in $aqbanking_modules; do
  hfiles=`( cd "${srcdir}/src/libs/aqbanking/${sm}" && ls *.h 2>/dev/null )`
  for f in ${hfiles}; do
    case ${f} in
      *_p.h | *_l.h)
         ;;
      *)   
         AQ_HEADER_FILES="${AQ_HEADER_FILES} ${lprefix}/src/libs/aqbanking/${sm}/${f}"
         ln -s "${lprefix}/src/libs/aqbanking/${sm}/${f}" "headers/aqbanking/${f}"
         ;;
    esac
  done
done

ln -s "${lprefix}/version.h" headers/aqbanking/version.h


#
# symlink all headers from src/libs/aqbanking++
#
mkdir headers/aqbanking++

# symlink all headers from src/libs/*
for sm in $aqbankingpp_modules; do
  hfiles=`( cd "${srcdir}/src/libs/aqbanking++/${sm}" && ls *.h 2>/dev/null )`
  for f in ${hfiles}; do
    case ${f} in
      *_p.h | *_l.h)
         ;;
      *)   
         AQ_HEADER_FILES="${AQ_HEADER_FILES} src/libs/aqbanking++/${sm}/${f}"
         ln -s "${lprefix}/src/libs/aqbanking++/${sm}/${f}" "headers/aqbanking++/${f}"
         ;;
    esac
  done
done



# symlink all headers from src/frontends

for frontend in ${aqbanking_frontends}; do
  rm -Rf ${frontend}
  mkdir -p headers/${frontend}
  
  # symlink all headers
  hfiles=`( cd "${srcdir}/src/frontends/${frontend}" && ls *.h 2>/dev/null )`
  for f in ${hfiles}; do
    case ${f} in
      *_p.h | *_l.h)
        ;;
      *)
        AQ_HEADER_FILES="${AQ_HEADER_FILES} ${lprefix}/src/frontends/${frontend}/${f}"
        ln -s "${lprefix}/src/frontends/${frontend}/${f}" "headers/${frontend}/${f}"
        ;;
    esac
  done
done


# symlink all headers from src/plugins/backends/

for backend in ${aqbanking_backends}; do
  mkdir -p headers/${backend}

  dfiles=`( cd "${srcdir}/src/plugins/backends/${backend}/plugin" && ls 2>/dev/null )`
  for d in ${dfiles}; do
    if test -d "${srcdir}/src/plugins/backends/${backend}/plugin/$d"; then
      case $d in
        . | ..)
          ;;
        *)
          hfiles=`( cd "${srcdir}/src/plugins/backends/${backend}/plugin/$d" && ls *.h 2>/dev/null )`
          for f in ${hfiles}; do
            case ${f} in
              *_p.h | *_l.h)
                ;;
              *)
                AQ_HEADER_FILES="${AQ_HEADER_FILES} ${lprefix}/src/plugins/backends/${backend}/plugin/$d/${f}"
                ln -s "${lprefix}/src/plugins/backends/${backend}/plugin/$d/${f}" "headers/${backend}/${f}"
                ;;
            esac
          done
          ;;
        esac
    else
      case ${d} in
        *_p.h | *_l.h)
          ;;
        *.h)
          AQ_HEADER_FILES="${AQ_HEADER_FILES} ${lprefix}/src/plugins/backends/${backend}/plugin/$d"
          ln -s "${lprefix}/src/plugins/backends/${backend}/plugin/$d" "headers/${backend}/$d"
          ;;
      esac
    fi
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
#
# Configure stuff for backend AqHBCI
#

case "$OS_TYPE" in
    posix)
        aqhbci_internal_libs="-laqhbci"
        aqhbci_libs="${aqhbci_internal_libs}"
        aqhbci_ldflags="-L\${libdir}"
        aqhbci_includes="-I`eval echo $prefix/include`"
        aqhbci_plugindir="\${libdir}/aqhbci/plugins/${AQHBCI_SO_EFFECTIVE}"
        aqhbci_data="`eval echo $prefix/share`/aqhbci"
    	;;
    windows)
        AC_DEFINE_UNQUOTED(BUILDING_AQHBCI_DLL,1, [Define if DLL is built])
        ACX_WINDOWS_PATHS
        aqhbci_internal_libs="-laqhbci32_${AQHBCI_SO_EFFECTIVE}"
        aqhbci_libs="${aqhbci_internal_libs}"
        aqhbci_ldflags="-L\${libdir}"
        aqhbci_includes="-I`eval echo $prefix/include`"
        aqhbci_plugindir="\${libdir}/aqhbci/plugins/${AQHBCI_SO_EFFECTIVE}"
        aqhbci_data="c:/mingw/1.0`eval echo $prefix/share`/aqhbci"
    	;;
esac

AS_SCRUB_INCLUDE(aqhbci_includes)
AC_SUBST(aqhbci_includes)
AC_SUBST(aqhbci_internal_libs)
AC_SUBST(aqhbci_libs)
AC_SUBST(aqhbci_ldflags)
AC_SUBST(aqhbci_plugindir)
AC_SUBST(aqhbci_data)
AC_SUBST(XMLMERGE)
AC_DEFINE_UNQUOTED(AQHBCI_DATA, "${aqhbci_data}", [data dir])



###-------------------------------------------------------------------------
#
# Cleanup includes, define all_XXX
#
AS_SCRUB_INCLUDE(all_includes)
AC_SUBST(all_includes)
AC_SUBST(all_libraries)


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
  src/libs/aqbanking++/ressource.rc
  src/frontends/Makefile
  src/frontends/cbanking/Makefile
  src/plugins/Makefile
  src/plugins/bankinfo/Makefile
  src/plugins/bankinfo/at/Makefile
  src/plugins/bankinfo/at/at.xml
  src/plugins/bankinfo/ch/Makefile
  src/plugins/bankinfo/ch/ch.xml
  src/plugins/bankinfo/de/Makefile
  src/plugins/bankinfo/de/de.xml
  src/plugins/bankinfo/us/Makefile
  src/plugins/bankinfo/us/us.xml
  src/plugins/bankinfo/generic/Makefile
  src/plugins/imexporters/Makefile
  src/plugins/imexporters/dbio/Makefile
  src/plugins/imexporters/dbio/dbio.xml
  src/plugins/imexporters/dtaus/Makefile
  src/plugins/imexporters/dtaus/dtaus.xml
  src/plugins/imexporters/dtaus/ressource.rc
  src/plugins/imexporters/dtaus/profiles/Makefile
  src/plugins/imexporters/dtaus/profiles/default.conf
  src/plugins/imexporters/eri/Makefile
  src/plugins/imexporters/eri/eri.xml
  src/plugins/imexporters/eri/ressource.rc
  src/plugins/imexporters/eri/profiles/Makefile
  src/plugins/imexporters/eri/profiles/default.conf
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
  src/plugins/imexporters/csv/profiles/comdirect.conf
  src/plugins/imexporters/csv/profiles/mijnpostbank.conf
  src/plugins/imexporters/csv/profiles/test.conf
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
  src/plugins/backends/Makefile
  src/plugins/backends/aqhbci/Makefile
  src/plugins/backends/aqhbci/media/Makefile
  src/plugins/backends/aqhbci/media/crypttoken/Makefile
  src/plugins/backends/aqhbci/media/crypttoken/pintan/Makefile
  src/plugins/backends/aqhbci/media/crypttoken/pintan/pintan.xml
  src/plugins/backends/aqhbci/media/crypttoken/pintan/ressource.rc
  src/plugins/backends/aqhbci/plugin/Makefile
  src/plugins/backends/aqhbci/plugin/aqhbci.xml
  src/plugins/backends/aqhbci/plugin/ressource.rc
  src/plugins/backends/aqhbci/plugin/banking/Makefile
  src/plugins/backends/aqhbci/plugin/msglayer/Makefile
  src/plugins/backends/aqhbci/plugin/joblayer/Makefile
  src/plugins/backends/aqhbci/plugin/applayer/Makefile
  src/plugins/backends/aqhbci/plugin/applayer/header.xml
  src/plugins/backends/aqhbci/plugin/applayer/accountjobs/Makefile
  src/plugins/backends/aqhbci/plugin/applayer/xml/Makefile
  src/plugins/backends/aqhbci/plugin/version.h
  src/plugins/backends/aqhbci/tools/Makefile
  src/plugins/backends/aqhbci/tools/hbcixml/Makefile
  src/plugins/backends/aqhbci/tools/aqhbci-tool/Makefile
  src/plugins/backends/aqhbci/Doxyfile
  src/plugins/backends/aqhbci/aqhbci-config.in
  src/test/Makefile
  src/tools/Makefile
  src/tools/aqbanking-tool/Makefile
  po/Makefile
  bindings/python/Makefile
  bindings/Makefile
  Doxyfile
  Makefile
  aqbanking-config.in
  aqbanking.spec
  aqbanking.iss
  version.h
])
AC_OUTPUT

