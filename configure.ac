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
AQBANKING_VERSION_BUILD=9
dnl "stable", "rcX", "betaX", "cvs"
AQBANKING_VERSION_TAG="cvs"
AQBANKING_VERSION_FULL_STRING="$AQBANKING_VERSION_MAJOR.$AQBANKING_VERSION_MINOR.$AQBANKING_VERSION_PATCHLEVEL.$AQBANKING_VERSION_BUILD${AQBANKING_VERSION_TAG}"
AQBANKING_VERSION_STRING="$AQBANKING_VERSION_MAJOR.$AQBANKING_VERSION_MINOR.$AQBANKING_VERSION_PATCHLEVEL"


###-------------------------------------------------------------------------
#
# SO versions
#
AQBANKING_SO_CURRENT=8
AQBANKING_SO_AGE=8
AQBANKING_SO_REVISION=0
AQBANKING_SO_EFFECTIVE="`echo \$(($AQBANKING_SO_CURRENT-$AQBANKING_SO_AGE))`"

#
# backends
#

AQHBCI_SO_CURRENT=6
AQHBCI_SO_AGE=4
AQHBCI_SO_REVISION=0
AQHBCI_SO_EFFECTIVE="`echo \$(($AQHBCI_SO_CURRENT-$AQHBCI_SO_AGE))`"

AQDTAUS_SO_CURRENT=1
AQDTAUS_SO_REVISION=2
AQDTAUS_SO_AGE=0
AQDTAUS_SO_EFFECTIVE="`echo \$(($AQDTAUS_SO_CURRENT-$AQDTAUS_SO_AGE))`"

AQGELDKARTE_SO_CURRENT=1
AQGELDKARTE_SO_REVISION=2
AQGELDKARTE_SO_AGE=0
AQGELDKARTE_SO_EFFECTIVE="`echo \$(($AQGELDKARTE_SO_CURRENT-$AQGELDKARTE_SO_AGE))`"

AQOFXCONNECT_SO_CURRENT=1
AQOFXCONNECT_SO_REVISION=0
AQOFXCONNECT_SO_AGE=0
AQOFXCONNECT_SO_EFFECTIVE="`echo \$(($AQOFXCONNECT_SO_CURRENT-$AQOFXCONNECT_SO_AGE))`"

#
# frontends
#

G2BANKING_SO_CURRENT="0"
G2BANKING_SO_REVISION="0"
G2BANKING_SO_AGE="0"
G2BANKING_SO_EFFECTIVE="`echo \$(($G2BANKING_SO_CURRENT-$G2BANKING_SO_AGE))`"

KBANKING_SO_CURRENT=2
KBANKING_SO_REVISION=1
KBANKING_SO_AGE=1
KBANKING_SO_EFFECTIVE="`echo \$(($KBANKING_SO_CURRENT-$KBANKING_SO_AGE))`"



###-------------------------------------------------------------------------
#
# AqHBCI versions
#
AQHBCI_VERSION_MAJOR=$AQBANKING_VERSION_MAJOR
AQHBCI_VERSION_MINOR=$AQBANKING_VERSION_MINOR
AQHBCI_VERSION_PATCHLEVEL=$AQBANKING_VERSION_PATCHLEVEL
AQHBCI_VERSION_BUILD=$AQBANKING_VERSION_BUILD
dnl "stable", "rcX", "betaX", "cvs"
AQHBCI_VERSION_TAG=$AQBANKING_VERSION_TAG
AQHBCI_VERSION_FULL_STRING="$AQHBCI_VERSION_MAJOR.$AQHBCI_VERSION_MINOR.$AQHBCI_VERSION_PATCHLEVEL.$AQHBCI_VERSION_BUILD${AQHBCI_VERSION_TAG}"
AQHBCI_VERSION_STRING="$AQHBCI_VERSION_MAJOR.$AQHBCI_VERSION_MINOR.$AQHBCI_VERSION_PATCHLEVEL"



###-------------------------------------------------------------------------
#
# AqDTAUS versions
#
AQDTAUS_VERSION_MAJOR=$AQBANKING_VERSION_MAJOR
AQDTAUS_VERSION_MINOR=$AQBANKING_VERSION_MINOR
AQDTAUS_VERSION_PATCHLEVEL=$AQBANKING_VERSION_PATCHLEVEL
AQDTAUS_VERSION_BUILD=$AQBANKING_VERSION_BUILD
dnl "stable", "rcX", "betaX", "cvs"
AQDTAUS_VERSION_TAG=$AQBANKING_VERSION_TAG
AQDTAUS_VERSION_FULL_STRING="$AQDTAUS_VERSION_MAJOR.$AQDTAUS_VERSION_MINOR.$AQDTAUS_VERSION_PATCHLEVEL.$AQDTAUS_VERSION_BUILD${AQDTAUS_VERSION_TAG}"
AQDTAUS_VERSION_STRING="$AQDTAUS_VERSION_MAJOR.$AQDTAUS_VERSION_MINOR.$AQDTAUS_VERSION_PATCHLEVEL"



###-------------------------------------------------------------------------
#
# AqGELDKARTE versions
#
AQGELDKARTE_VERSION_MAJOR=$AQBANKING_VERSION_MAJOR
AQGELDKARTE_VERSION_MINOR=$AQBANKING_VERSION_MINOR
AQGELDKARTE_VERSION_PATCHLEVEL=$AQBANKING_VERSION_PATCHLEVEL
AQGELDKARTE_VERSION_BUILD=$AQBANKING_VERSION_BUILD
dnl "stable", "rcX", "betaX", "cvs"
AQGELDKARTE_VERSION_TAG=$AQBANKING_VERSION_TAG
AQGELDKARTE_VERSION_FULL_STRING="$AQGELDKARTE_VERSION_MAJOR.$AQGELDKARTE_VERSION_MINOR.$AQGELDKARTE_VERSION_PATCHLEVEL.$AQGELDKARTE_VERSION_BUILD${AQGELDKARTE_VERSION_TAG}"
AQGELDKARTE_VERSION_STRING="$AQGELDKARTE_VERSION_MAJOR.$AQGELDKARTE_VERSION_MINOR.$AQGELDKARTE_VERSION_PATCHLEVEL"



###-------------------------------------------------------------------------
#
# AqOFXCONNECT versions
#
AQOFXCONNECT_VERSION_MAJOR=$AQBANKING_VERSION_MAJOR
AQOFXCONNECT_VERSION_MINOR=$AQBANKING_VERSION_MINOR
AQOFXCONNECT_VERSION_PATCHLEVEL=$AQBANKING_VERSION_PATCHLEVEL
AQOFXCONNECT_VERSION_BUILD=$AQBANKING_VERSION_BUILD
dnl "stable", "rcX", "betaX", "cvs"
AQOFXCONNECT_VERSION_TAG=$AQBANKING_VERSION_TAG
AQOFXCONNECT_VERSION_FULL_STRING="$AQOFXCONNECT_VERSION_MAJOR.$AQOFXCONNECT_VERSION_MINOR.$AQOFXCONNECT_VERSION_PATCHLEVEL.$AQOFXCONNECT_VERSION_BUILD${AQOFXCONNECT_VERSION_TAG}"
AQOFXCONNECT_VERSION_STRING="$AQOFXCONNECT_VERSION_MAJOR.$AQOFXCONNECT_VERSION_MINOR.$AQOFXCONNECT_VERSION_PATCHLEVEL"



###-------------------------------------------------------------------------
#
# G2Banking versions
#
G2BANKING_VERSION_MAJOR=$AQBANKING_VERSION_MAJOR
G2BANKING_VERSION_MINOR=$AQBANKING_VERSION_MINOR
G2BANKING_VERSION_PATCHLEVEL=$AQBANKING_VERSION_PATCHLEVEL
G2BANKING_VERSION_BUILD=$AQBANKING_VERSION_BUILD
dnl "stable", "rcX", "betaX", "cvs"
G2BANKING_VERSION_TAG=$AQBANKING_VERSION_TAG
G2BANKING_VERSION_FULL_STRING="$G2BANKING_VERSION_MAJOR.$G2BANKING_VERSION_MINOR.$G2BANKING_VERSION_PATCHLEVEL.$G2BANKING_VERSION_BUILD${G2BANKING_VERSION_TAG}"
G2BANKING_VERSION_STRING="$G2BANKING_VERSION_MAJOR.$G2BANKING_VERSION_MINOR.$G2BANKING_VERSION_PATCHLEVEL"



###-------------------------------------------------------------------------
#
# KBanking versions
#
KBANKING_VERSION_MAJOR=$AQBANKING_VERSION_MAJOR
KBANKING_VERSION_MINOR=$AQBANKING_VERSION_MINOR
KBANKING_VERSION_PATCHLEVEL=$AQBANKING_VERSION_PATCHLEVEL
KBANKING_VERSION_BUILD=$AQBANKING_VERSION_BUILD
dnl "stable", "rcX", "betaX", "cvs"
KBANKING_VERSION_TAG=$AQBANKING_VERSION_TAG
KBANKING_VERSION_FULL_STRING="$KBANKING_VERSION_MAJOR.$KBANKING_VERSION_MINOR.$KBANKING_VERSION_PATCHLEVEL.$KBANKING_VERSION_BUILD${KBANKING_VERSION_TAG}"
KBANKING_VERSION_STRING="$KBANKING_VERSION_MAJOR.$KBANKING_VERSION_MINOR.$KBANKING_VERSION_PATCHLEVEL"



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


#
# for AqDTAUS
#
case "$AQDTAUS_VERSION_TAG" in
  cvs)
     AQDTAUS_VERSION_RELEASE_STRING="$AQDTAUS_VERSION_FULL_STRING"
     ;;
  stable)
     AQDTAUS_VERSION_RELEASE_STRING="$AQDTAUS_VERSION_MAJOR.$AQDTAUS_VERSION_MINOR.$AQDTAUS_VERSION_PATCHLEVEL"
     ;;
  *)
     AQDTAUS_VERSION_RELEASE_STRING="$AQDTAUS_VERSION_MAJOR.$AQDTAUS_VERSION_MINOR.$AQDTAUS_VERSION_PATCHLEVEL"
     
     # add TAG
     AQDTAUS_VERSION_RELEASE_STRING="$AQDTAUS_VERSION_RELEASE_STRING${AQDTAUS_VERSION_TAG}"
     ;;
esac


#
# for AqGELDKARTE
#
case "$AQGELDKARTE_VERSION_TAG" in
  cvs)
     AQGELDKARTE_VERSION_RELEASE_STRING="$AQGELDKARTE_VERSION_FULL_STRING"
     ;;
  stable)
     AQGELDKARTE_VERSION_RELEASE_STRING="$AQGELDKARTE_VERSION_MAJOR.$AQGELDKARTE_VERSION_MINOR.$AQGELDKARTE_VERSION_PATCHLEVEL"
     ;;
  *)
     AQGELDKARTE_VERSION_RELEASE_STRING="$AQGELDKARTE_VERSION_MAJOR.$AQGELDKARTE_VERSION_MINOR.$AQGELDKARTE_VERSION_PATCHLEVEL"
     
     # add TAG
     AQGELDKARTE_VERSION_RELEASE_STRING="$AQGELDKARTE_VERSION_RELEASE_STRING${AQGELDKARTE_VERSION_TAG}"
     ;;
esac


#
# for AqOFXCONNECT
#
case "$AQOFXCONNECT_VERSION_TAG" in
  cvs)
     AQOFXCONNECT_VERSION_RELEASE_STRING="$AQOFXCONNECT_VERSION_FULL_STRING"
     ;;
  stable)
     AQOFXCONNECT_VERSION_RELEASE_STRING="$AQOFXCONNECT_VERSION_MAJOR.$AQOFXCONNECT_VERSION_MINOR.$AQOFXCONNECT_VERSION_PATCHLEVEL"
     ;;
  *)
     AQOFXCONNECT_VERSION_RELEASE_STRING="$AQOFXCONNECT_VERSION_MAJOR.$AQOFXCONNECT_VERSION_MINOR.$AQOFXCONNECT_VERSION_PATCHLEVEL"
     
     # add TAG
     AQOFXCONNECT_VERSION_RELEASE_STRING="$AQOFXCONNECT_VERSION_RELEASE_STRING${AQOFXCONNECT_VERSION_TAG}"
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
# same for AqDTAUS
#
AC_SUBST(AQDTAUS_VERSION_MAJOR)
AC_SUBST(AQDTAUS_VERSION_MINOR)
AC_SUBST(AQDTAUS_VERSION_PATCHLEVEL)
AC_SUBST(AQDTAUS_VERSION_BUILD)
AC_SUBST(AQDTAUS_VERSION_TAG)
AC_SUBST(AQDTAUS_VERSION_FULL_STRING)
AC_SUBST(AQDTAUS_VERSION_STRING)
AC_SUBST(AQDTAUS_VERSION_RELEASE_STRING)

AC_DEFINE_UNQUOTED(AQDTAUS_VERSION_MAJOR,$AQDTAUS_VERSION_MAJOR, 
                   [major version])
AC_DEFINE_UNQUOTED(AQDTAUS_VERSION_MINOR,$AQDTAUS_VERSION_MINOR, 
                   [minor version])
AC_DEFINE_UNQUOTED(AQDTAUS_VERSION_PATCHLEVEL,$AQDTAUS_VERSION_PATCHLEVEL, 
                   [patchlevel])
AC_DEFINE_UNQUOTED(AQDTAUS_VERSION_BUILD,$AQDTAUS_VERSION_BUILD, [build])
AC_DEFINE_UNQUOTED(AQDTAUS_VERSION_STRING,"$AQDTAUS_VERSION_STRING", 
                   [version string])
AC_DEFINE_UNQUOTED(AQDTAUS_VERSION_FULL_STRING,"$AQDTAUS_VERSION_FULL_STRING",
                   [full version string])
AC_DEFINE_UNQUOTED(AQDTAUS_VERSION_TAG,$AQDTAUS_VERSION_TAG, [tag])

AC_SUBST(AQDTAUS_SO_CURRENT)
AC_SUBST(AQDTAUS_SO_REVISION)
AC_SUBST(AQDTAUS_SO_AGE)
AC_SUBST(AQDTAUS_SO_EFFECTIVE)
AC_DEFINE_UNQUOTED(AQDTAUS_SO_EFFECTIVE_STR, "$AQDTAUS_SO_EFFECTIVE",
                   [effective SO version])

#
# same for AqGELDKARTE
#

AC_SUBST(AQGELDKARTE_VERSION_MAJOR)
AC_SUBST(AQGELDKARTE_VERSION_MINOR)
AC_SUBST(AQGELDKARTE_VERSION_PATCHLEVEL)
AC_SUBST(AQGELDKARTE_VERSION_BUILD)
AC_SUBST(AQGELDKARTE_VERSION_TAG)
AC_SUBST(AQGELDKARTE_VERSION_FULL_STRING)
AC_SUBST(AQGELDKARTE_VERSION_STRING)
AC_SUBST(AQGELDKARTE_VERSION_RELEASE_STRING)

AC_DEFINE_UNQUOTED(AQGELDKARTE_VERSION_MAJOR,$AQGELDKARTE_VERSION_MAJOR, 
                   [major version])
AC_DEFINE_UNQUOTED(AQGELDKARTE_VERSION_MINOR,$AQGELDKARTE_VERSION_MINOR, 
                   [minor version])
AC_DEFINE_UNQUOTED(AQGELDKARTE_VERSION_PATCHLEVEL,$AQGELDKARTE_VERSION_PATCHLEVEL, 
                   [patchlevel])
AC_DEFINE_UNQUOTED(AQGELDKARTE_VERSION_BUILD,$AQGELDKARTE_VERSION_BUILD, [build])
AC_DEFINE_UNQUOTED(AQGELDKARTE_VERSION_STRING,"$AQGELDKARTE_VERSION_STRING", 
                   [version string])
AC_DEFINE_UNQUOTED(AQGELDKARTE_VERSION_FULL_STRING,"$AQGELDKARTE_VERSION_FULL_STRING",
                   [full version string])
AC_DEFINE_UNQUOTED(AQGELDKARTE_VERSION_TAG,$AQGELDKARTE_VERSION_TAG, [tag])

AC_SUBST(AQGELDKARTE_SO_CURRENT)
AC_SUBST(AQGELDKARTE_SO_REVISION)
AC_SUBST(AQGELDKARTE_SO_AGE)
AC_SUBST(AQGELDKARTE_SO_EFFECTIVE)
AC_DEFINE_UNQUOTED(AQGELDKARTE_SO_EFFECTIVE_STR, "$AQGELDKARTE_SO_EFFECTIVE",
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
# same for G2Banking
#

AC_SUBST(G2BANKING_VERSION_MAJOR)
AC_SUBST(G2BANKING_VERSION_MINOR)
AC_SUBST(G2BANKING_VERSION_PATCHLEVEL)
AC_SUBST(G2BANKING_VERSION_BUILD)
AC_SUBST(G2BANKING_VERSION_TAG)
AC_SUBST(G2BANKING_VERSION_FULL_STRING)
AC_SUBST(G2BANKING_VERSION_STRING)
AC_SUBST(G2BANKING_VERSION_RELEASE_STRING)

AC_DEFINE_UNQUOTED(G2BANKING_VERSION_MAJOR,$G2BANKING_VERSION_MAJOR, 
                   [major version])
AC_DEFINE_UNQUOTED(G2BANKING_VERSION_MINOR,$G2BANKING_VERSION_MINOR, 
                   [minor version])
AC_DEFINE_UNQUOTED(G2BANKING_VERSION_PATCHLEVEL,$G2BANKING_VERSION_PATCHLEVEL, 
                   [patchlevel])
AC_DEFINE_UNQUOTED(G2BANKING_VERSION_BUILD,$G2BANKING_VERSION_BUILD, [build])
AC_DEFINE_UNQUOTED(G2BANKING_VERSION_STRING,"$G2BANKING_VERSION_STRING", 
                   [version string])
AC_DEFINE_UNQUOTED(G2BANKING_VERSION_FULL_STRING,"$G2BANKING_VERSION_FULL_STRING",
                   [full version string])
AC_DEFINE_UNQUOTED(G2BANKING_VERSION_TAG,$G2BANKING_VERSION_TAG, [tag])

AC_SUBST(G2BANKING_SO_CURRENT)
AC_SUBST(G2BANKING_SO_REVISION)
AC_SUBST(G2BANKING_SO_AGE)
AC_SUBST(G2BANKING_SO_EFFECTIVE)
AC_DEFINE_UNQUOTED(G2BANKING_SO_EFFECTIVE_STR, "$G2BANKING_SO_EFFECTIVE",
                   [effective SO version])


#
# same for KBanking
#

AC_SUBST(KBANKING_VERSION_MAJOR)
AC_SUBST(KBANKING_VERSION_MINOR)
AC_SUBST(KBANKING_VERSION_PATCHLEVEL)
AC_SUBST(KBANKING_VERSION_BUILD)
AC_SUBST(KBANKING_VERSION_TAG)
AC_SUBST(KBANKING_VERSION_FULL_STRING)
AC_SUBST(KBANKING_VERSION_STRING)
AC_SUBST(KBANKING_VERSION_RELEASE_STRING)

AC_DEFINE_UNQUOTED(KBANKING_VERSION_MAJOR,$KBANKING_VERSION_MAJOR, 
                   [major version])
AC_DEFINE_UNQUOTED(KBANKING_VERSION_MINOR,$KBANKING_VERSION_MINOR, 
                   [minor version])
AC_DEFINE_UNQUOTED(KBANKING_VERSION_PATCHLEVEL,$KBANKING_VERSION_PATCHLEVEL, 
                   [patchlevel])
AC_DEFINE_UNQUOTED(KBANKING_VERSION_BUILD,$KBANKING_VERSION_BUILD, [build])
AC_DEFINE_UNQUOTED(KBANKING_VERSION_STRING,"$KBANKING_VERSION_STRING", 
                   [version string])
AC_DEFINE_UNQUOTED(KBANKING_VERSION_FULL_STRING,"$KBANKING_VERSION_FULL_STRING",
                   [full version string])
AC_DEFINE_UNQUOTED(KBANKING_VERSION_TAG,$KBANKING_VERSION_TAG, [tag])

AC_SUBST(KBANKING_SO_CURRENT)
AC_SUBST(KBANKING_SO_REVISION)
AC_SUBST(KBANKING_SO_AGE)
AC_SUBST(KBANKING_SO_EFFECTIVE)
AC_DEFINE_UNQUOTED(KBANKING_SO_EFFECTIVE_STR, "$KBANKING_SO_EFFECTIVE",
                   [effective SO version])



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
# Check for LibChipcard2
#

AC_CHIPCARD_CLIENT(1,9,11)
AC_MSG_CHECKING(if chipcard_client is useable)
if test "x$have_chipcard_client" != "xyes"; then
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi

all_includes="$all_includes $chipcard2_client_includes"



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
# GTK2
#

pkg_modules="gtk+-2.0 >= 2.0.0"
PKG_CHECK_MODULES(GTK2, [$pkg_modules])
all_includes="$all_includes $GTK2_CFLAGS"
AC_SUBST(GTK2_CFLAGS)
AC_SUBST(GTK2_LIBS)



###-------------------------------------------------------------------------
#
# KDE3
#

AQ_CHECK_KDE3
if test "$have_kde3" = "yes"; then
  all_includes="$all_includes $qt3_includes $kde3_includes"

  # Define this macro to disable the cast from QString to const char
  # which implicitly uses latin1()!
  AC_DEFINE(QT_NO_ASCII_CAST,[1],[Disable the cast from QString to const char.])
  
  dnl Check for Qts lupdate tools
  AC_PATH_PROG([LUPDATE], [lupdate], [NO], [$PATH:"` echo ${qt3_moc} | sed 's-/moc--' `"])
  AC_PATH_PROG([LRELEASE], [lrelease], [NO], [$PATH:"` echo ${qt3_moc} | sed 's-/moc--' `"])
  AM_CONDITIONAL([HAVE_LUPDATE], [test "$LUPDATE" != "NO"])
  AM_CONDITIONAL([HAVE_LRELEASE], [test "$LRELEASE" != "NO"])
fi



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
  case "${f}" in
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
    case "${f}" in
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
    case "${f}" in
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
    case "${f}" in
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
      case "$d" in
        . | ..)
          ;;
        *)
          hfiles=`( cd "${srcdir}/src/plugins/backends/${backend}/plugin/$d" && ls *.h 2>/dev/null )`
          for f in ${hfiles}; do
            case "${f}" in
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
      case "${d}" in
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
# backends and frontends
#

AC_ARG_WITH(frontends, 
  [  --with-frontends=list      space separated list of frontends (default:all)],
  [aqbanking_frontends="$withval"],
  [aqbanking_frontends="cbanking g2banking kbanking"]
)


AC_ARG_WITH(backends, 
  [  --with-backends=list      space separated list of backends (default:all)],
  [aqbanking_backends="$withval"],
  [aqbanking_backends="aqhbci aqdtaus aqgeldkarte aqofxconnect"]
)


#
# check for dependencies of backends
#
for backend in $aqbanking_backends; do
  case "$backend" in
    aqofxconnect)
      if test "x$HAVE_OFX_WITH_CONNECT" != "xyes"; then
        AC_MSG_ERROR([LibOFX>0.7.0 is required for OFX direct connect.])
      fi
      ;;
    aqgeldkarte)
      if test "x$have_chipcard_client" != "xyes"; then
        AC_MSG_ERROR([Libchipcard2-client is required for AqGeldKarte.])
      fi
      ;;
  esac
done



#
# check for dependencies of frontends
#
for frontend in $aqbanking_frontends; do
  case "$frontend" in
    g2banking)
      if test -z "$GTK2_LIBS"; then
        AC_MSG_ERROR([GTK2 is required for GTK2-frontend.])
      fi
      ;;
    kbanking)
      if test "x$have_kde3" != "xyes"; then
        AC_MSG_ERROR([KDE3 is required for KDE-frontend.])
      fi
      ;;
  esac
done



###-------------------------------------------------------------------------
#
# Configure stuff for backend AqHBCI
#

case "$aqbanking_backends" in *aqhbci*)
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
# Configure stuff for backend AqDTAUS
#

case "$aqbanking_backends" in *aqdtaus*)
  case "$OS_TYPE" in
      posix)
          aqdtaus_internal_libs="-laqdtaus"
          aqdtaus_libs="-L\${libdir} ${aqdtaus_internal_libs}"
          aqdtaus_includes="-I`eval echo $prefix/include`"
          aqdtaus_plugins="\${libdir}/aqdtaus/plugins/${AQDTAUS_SO_EFFECTIVE}"
          aqdtaus_data="`eval echo $prefix/share`/aqdtaus"
          ;;
      windows)
          AC_DEFINE_UNQUOTED(BUILDING_AQDTAUS_DLL,1, [Define if DLL is built])
          ACX_WINDOWS_PATHS
          aqdtaus_internal_libs="-laqdtaus32_${AQDTAUS_SO_EFFECTIVE}"
          aqdtaus_libs="-L\${libdir} ${aqdtaus_internal_libs}"
          aqdtaus_includes="-I`eval echo $prefix/include`"
          aqdtaus_plugins="\${libdir}/aqdtaus/plugins/${AQDTAUS_SO_EFFECTIVE}"
          aqdtaus_data="c:/mingw/1.0`eval echo $prefix/share`/aqdtaus"
          ;;
  esac
  ;;
esac

AS_SCRUB_INCLUDE(aqdtaus_includes)
AC_SUBST(aqdtaus_includes)
AC_SUBST(aqdtaus_internal_libs)
AC_SUBST(aqdtaus_libs)
AC_SUBST(aqdtaus_plugins)
AC_SUBST(aqdtaus_data)
AC_DEFINE_UNQUOTED(AQDTAUS_DATA, "${aqdtaus_data}", [data dir])



###-------------------------------------------------------------------------
#
# Configure stuff for backend AqGeldKarte
#

case "$aqbanking_backends" in *aqgeldkarte*)
  case "$OS_TYPE" in
      posix)
          aqgeldkarte_internal_libs="-laqgeldkarte"
          aqgeldkarte_libs="-L\${libdir} ${aqgeldkarte_internal_libs}"
          aqgeldkarte_includes="-I`eval echo $prefix/include`"
          aqgeldkarte_plugins="\${libdir}/aqgeldkarte/plugins/${AQGELDKARTE_SO_EFFECTIVE}"
          aqgeldkarte_data="`eval echo $prefix/share`/aqgeldkarte"
          ;;
      windows)
          AC_DEFINE_UNQUOTED(BUILDING_AQGELDKARTE_DLL,1, [Define if DLL is built])
          ACX_WINDOWS_PATHS
          aqgeldkarte_internal_libs="-laqgeldkarte32_${AQGELDKARTE_SO_EFFECTIVE}"
          aqgeldkarte_libs="-L\${libdir} ${aqgeldkarte_internal_libs}"
          aqgeldkarte_includes="-I`eval echo $prefix/include`"
          aqgeldkarte_plugins="\${libdir}/aqgeldkarte/plugins/${AQGELDKARTE_SO_EFFECTIVE}"
          aqgeldkarte_data="c:/mingw/1.0`eval echo $prefix/share`/aqgeldkarte"
          ;;
  esac
  ;;
esac

AS_SCRUB_INCLUDE(aqgeldkarte_includes)
AC_SUBST(aqgeldkarte_includes)
AC_SUBST(aqgeldkarte_internal_libs)
AC_SUBST(aqgeldkarte_libs)
AC_SUBST(aqgeldkarte_plugins)
AC_SUBST(aqgeldkarte_data)
AC_DEFINE_UNQUOTED(AQGELDKARTE_DATA, "${aqgeldkarte_data}", [data dir])



###-------------------------------------------------------------------------
#
# Configure stuff for backend AqOfxConnect
#

case "$aqbanking_backends" in *aqofxconnect*)
  case "$OS_TYPE" in
      posix)
          aqofxconnect_internal_libs="-laqofxconnect"
          aqofxconnect_libs="-L\${libdir} ${aqofxconnect_internal_libs}"
          aqofxconnect_includes="-I`eval echo $prefix/include`"
          aqofxconnect_plugins="\${libdir}/aqofxconnect/plugins/${AQOFXCONNECT_SO_EFFECTIVE}"
          aqofxconnect_data="`eval echo $prefix/share`/aqofxconnect"
          ;;
      windows)
          AC_DEFINE_UNQUOTED(BUILDING_AQOFXCONNECT_DLL,1, [Define if DLL is built])
          ACX_WINDOWS_PATHS
          aqofxconnect_internal_libs="-laqofxconnect32_${AQOFXCONNECT_SO_EFFECTIVE}"
          aqofxconnect_libs="-L\${libdir} ${aqofxconnect_internal_libs}"
          aqofxconnect_includes="-I`eval echo $prefix/include`"
          aqofxconnect_plugins="\${libdir}/aqofxconnect/plugins/${AQOFXCONNECT_SO_EFFECTIVE}"
          aqofxconnect_data="c:/mingw/1.0`eval echo $prefix/share`/aqofxconnect"
          ;;
  esac
  ;;
esac

AS_SCRUB_INCLUDE(aqofxconnect_includes)
AC_SUBST(aqofxconnect_includes)
AC_SUBST(aqofxconnect_internal_libs)
AC_SUBST(aqofxconnect_libs)
AC_SUBST(aqofxconnect_plugins)
AC_SUBST(aqofxconnect_data)
AC_DEFINE_UNQUOTED(AQOFXCONNECT_DATA, "${aqofxconnect_data}", [data dir])



###-------------------------------------------------------------------------
#
# Configure stuff for frontend g2banking
#

case "$OS_TYPE" in
    posix)
        g2banking_libs="-L`eval echo $prefix/lib` -lg2banking"
        g2banking_includes="-I`eval echo $prefix/include`"
        g2banking_data="`eval echo $prefix/share`/g2banking"
    	;;
    windows)
        AC_DEFINE_UNQUOTED(BUILDING_AQBANKING_DLL,1, [Define if DLL is built])
        g2banking_libs="-L`eval echo $prefix/lib` -lg2banking32_${G2BANKING_SO_EFFECTIVE}"
        g2banking_includes="-I`eval echo $prefix/include`"
        g2banking_data="`eval echo $prefix/share`/g2banking"
	all_includes="$all_includes -mms-bitfields"
    	;;
esac

AC_SUBST(g2banking_includes)
AC_SUBST(g2banking_libs)
AC_SUBST(g2banking_data)



###-------------------------------------------------------------------------
#
# Configure stuff for frontend kbanking
#

case "$OS_TYPE" in
    posix)
        libsuffix=`eval kde-config --libsuffix`
        kbanking_libs="-L`eval echo $prefix/lib`$libsuffix -lkbanking"
        kbanking_includes="-I`eval echo $prefix/include`"
        kbanking_data="`eval echo $prefix/share`/kbanking"
    	;;
    windows)
        AC_DEFINE_UNQUOTED(BUILDING_KBANKING_DLL,1, [Define if DLL is built])
        kbanking_libs="-L`eval echo $prefix/lib` -lkbanking32_${KBANKING_SO_EFFECTIVE}"
        kbanking_includes="-I`eval echo $prefix/include`"
        kbanking_data="`eval echo $prefix/share`/kbanking"
    	;;
esac

AC_SUBST(kbanking_includes)
AC_SUBST(kbanking_libs)
AC_SUBST(kbanking_data)



###-------------------------------------------------------------------------
#
# Cleanup includes, define all_XXX
#

AS_SCRUB_INCLUDE(all_includes)
AC_SUBST(all_includes)
AC_SUBST(all_libraries)

AC_SUBST(aqbanking_frontends)
AC_SUBST(aqbanking_backends)



###-------------------------------------------------------------------------
#
# Output files
#

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
  src/frontends/g2banking/Makefile
  src/frontends/g2banking/g2banking-config.in
  src/frontends/g2banking/version.h
  src/frontends/g2banking/ressource.rc
  src/frontends/g2banking/dialogs/Makefile
  src/frontends/g2banking/views/Makefile
  src/frontends/g2banking/widgets/Makefile
  src/frontends/kbanking/Makefile
  src/frontends/kbanking/dialogs/Makefile
  src/frontends/kbanking/prg/Makefile
  src/frontends/kbanking/views/Makefile
  src/frontends/kbanking/widgets/Makefile
  src/frontends/kbanking/version.h
  src/frontends/kbanking/kbanking-config.in
  src/frontends/kbanking/ressource.rc
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
  src/plugins/backends/aqhbci/plugin/header.xml
  src/plugins/backends/aqhbci/plugin/ressource.rc
  src/plugins/backends/aqhbci/plugin/banking/Makefile
  src/plugins/backends/aqhbci/plugin/msglayer/Makefile
  src/plugins/backends/aqhbci/plugin/joblayer/Makefile
  src/plugins/backends/aqhbci/plugin/applayer/Makefile
  src/plugins/backends/aqhbci/plugin/accountjobs/Makefile
  src/plugins/backends/aqhbci/plugin/applayer/xml/Makefile
  src/plugins/backends/aqhbci/plugin/version.h
  src/plugins/backends/aqhbci/tools/Makefile
  src/plugins/backends/aqhbci/tools/hbcixml/Makefile
  src/plugins/backends/aqhbci/tools/aqhbci-tool/Makefile
  src/plugins/backends/aqhbci/aqhbci-config.in
  src/plugins/backends/aqdtaus/Makefile
  src/plugins/backends/aqdtaus/version.h
  src/plugins/backends/aqdtaus/aqdtaus-config.in
  src/plugins/backends/aqdtaus/plugin/Makefile
  src/plugins/backends/aqdtaus/plugin/aqdtaus.xml
  src/plugins/backends/aqgeldkarte/Makefile
  src/plugins/backends/aqgeldkarte/version.h
  src/plugins/backends/aqgeldkarte/aqgeldkarte-config.in
  src/plugins/backends/aqgeldkarte/plugin/Makefile
  src/plugins/backends/aqgeldkarte/plugin/aqgeldkarte.xml
  src/plugins/backends/aqofxconnect/Makefile
  src/plugins/backends/aqofxconnect/version.h
  src/plugins/backends/aqofxconnect/aqofxconnect-config.in
  src/plugins/backends/aqofxconnect/plugin/Makefile
  src/plugins/backends/aqofxconnect/plugin/aqofxconnect.xml
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



###-------------------------------------------------------------------------
#
# Summary
#

echo
echo
echo "Summary"
echo "----------------------------------------------------"
echo "AqBanking version  : $AQBANKING_VERSION_FULL_STRING"
echo "Supported backends : $aqbanking_backends"
echo "Supported frontends: $aqbanking_frontends"
echo
echo "You can now run \"make\" (or gmake on FreeBSD) to compile."


