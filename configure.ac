#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.56)
AC_INIT
AC_CANONICAL_TARGET([])

AC_CONFIG_SRCDIR([README])
AC_CONFIG_HEADER([config.h])



#
# versions
#
AQBANKING_VERSION_MAJOR=0
AQBANKING_VERSION_MINOR=1
AQBANKING_VERSION_PATCHLEVEL=2
AQBANKING_VERSION_BUILD=5
dnl "stable", "rcX", "betaX", "cvs"
AQBANKING_VERSION_TAG="cvs"
AQBANKING_VERSION_FULL_STRING="$AQBANKING_VERSION_MAJOR.$AQBANKING_VERSION_MINOR.$AQBANKING_VERSION_PATCHLEVEL.$AQBANKING_VERSION_BUILD${AQBANKING_VERSION_TAG}"
AQBANKING_VERSION_STRING="$AQBANKING_VERSION_MAJOR.$AQBANKING_VERSION_MINOR.$AQBANKING_VERSION_PATCHLEVEL"


#
# Create release string
#
case "$AQBANKING_VERSION_TAG" in
  cvs)
     AQBANKING_VERSION_RELEASE_STRING="$AQBANKING_VERSION_FULL_STRING"
     ;;
  stable)
     AQBANKING_VERSION_RELEASE_STRING="$AQBANKING_VERSION_MAJOR.$AQBANKING_VERSION_MINOR"
     # should we add patchlevel ?
     if test "$AQBANKING_VERSION_PATCHLEVEL" != "0"; then
       AQBANKING_VERSION_RELEASE_STRING="$AQBANKING_VERSION_RELEASE_STRING.$AQBANKING_VERSION_PATCHLEVEL"
     fi
     ;;
  *)
     AQBANKING_VERSION_RELEASE_STRING="$AQBANKING_VERSION_MAJOR.$AQBANKING_VERSION_MINOR"
     # should we add patchlevel ?
     if test "$AQBANKING_VERSION_PATCHLEVEL" != "0"; then
       AQBANKING_VERSION_RELEASE_STRING="$AQBANKING_VERSION_RELEASE_STRING.$AQBANKING_VERSION_PATCHLEVEL"
     fi
     
     # add TAG
     AQBANKING_VERSION_RELEASE_STRING="$AQBANKING_VERSION_RELEASE_STRING${AQBANKING_VERSION_TAG}"
     ;;
esac
   

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

#
# SO version
#
AQBANKING_SO_CURRENT=0
AQBANKING_SO_REVISION=0
AQBANKING_SO_AGE=0
AQBANKING_SO_EFFECTIVE="`echo \$(($AQBANKING_SO_CURRENT-$AQBANKING_SO_AGE))`"
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
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h])

###-------------------------------------------------------------------------
# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_STRUCT_TM

###-------------------------------------------------------------------------
# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([memmove memset strcasecmp strdup strerror snprintf])

###-------------------------------------------------------------------------
AC_GWENHYWFAR(0,9,14,1)

all_includes="$all_includes $gwenhywfar_includes"


###-------------------------------------------------------------------------
dnl OS dependant settings

AQ_CHECK_OS

case "$OS_TYPE" in
    posix)
        aqbanking_libs="-L`eval echo $prefix/lib` -laqbanking"
        aqbanking_libspp="-L`eval echo $prefix/lib` -laqbankingpp"
        aqbanking_includes="-I`eval echo $prefix/include`"
        aqbanking_plugins="`eval echo $prefix/lib`/aqbanking/plugins/${AQBANKING_SO_EFFECTIVE}"
        aqbanking_data="`eval echo $prefix/share`/aqbanking"
    	;;
    windows)
        AC_DEFINE_UNQUOTED(BUILDING_AQBANKING_DLL,1, [Define if DLL is built])
        aqbanking_libs="-L`eval echo $prefix/lib` -laqbanking32_${AQBANKING_SO_EFFECTIVE}"
        aqbanking_libspp="-L`eval echo $prefix/lib` -laqbankingpp32"
        aqbanking_includes="-I`eval echo $prefix/include`"
        aqbanking_plugins="`eval echo $prefix/lib`/aqbanking/plugins/${AQBANKING_SO_EFFECTIVE}"
        aqbanking_data="`eval echo $prefix/share`/aqbanking"
    	;;
esac
AM_CONDITIONAL(IS_WINDOWS, [test "$OS_TYPE" = "windows"])

###-------------------------------------------------------------------------

# Debug arguments for compilation
ACX_COMPILE_WARN()

###-------------------------------------------------------------------------
AS_SCRUB_INCLUDE(all_includes)
AC_SUBST(all_includes)
AC_SUBST(all_libraries)
AS_SCRUB_INCLUDE(aqbanking_includes)
AC_SUBST(aqbanking_includes)
AC_SUBST(aqbanking_libs)
AC_SUBST(aqbanking_libspp)
AC_SUBST(aqbanking_plugins)
AC_SUBST(aqbanking_data)
AC_DEFINE_UNQUOTED(AQBANKING_DATA, "${aqbanking_data}", [data dir])


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
mkdir aqbanking
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
  src/test/Makefile
  Doxyfile
  Makefile
  aqbanking-config.in
  aqbanking.spec
  version.h
])
AC_OUTPUT

