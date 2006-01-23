/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQBANKING_ERROR_H
#define AQBANKING_ERROR_H

#include <aqbanking/system.h>


/* @FIXME: disabled until next release of GnuCash */
/*#define AQBANKING_NOWARN_DEPRECATED*/


#ifdef BUILDING_AQBANKING
# /* building AqBanking */
# if AQBANKING_SYS_IS_WINDOWS
#   /* for windows */
#   ifdef __declspec
#     define AQBANKING_API __declspec (dllexport)
#   else /* if __declspec */
#     define AQBANKING_API
#   endif /* if NOT __declspec */
# else
#   /* for non-win32 */
#   ifdef GCC_WITH_VISIBILITY_ATTRIBUTE
#     define AQBANKING_API __attribute__((visibility("default")))
#   else
#     define AQBANKING_API
#   endif
# endif
#else
# /* not building AqBanking */
# if AQBANKING_SYS_IS_WINDOWS
#   /* for windows */
#   ifdef __declspec
#     define AQBANKING_API __declspec (dllimport)
#   else /* if __declspec */
#     define AQBANKING_API
#   endif /* if NOT __declspec */
# else
#   /* for non-win32 */
#   define AQBANKING_API
# endif
#endif

#ifdef GCC_WITH_VISIBILITY_ATTRIBUTE
# define AQBANKING_EXPORT __attribute__((visibility("default")))
# define AQBANKING_NOEXPORT __attribute__((visibility("hidden")))
#else
# define AQBANKING_EXPORT
# define AQBANKING_NOEXPORT
#endif


#ifndef AQBANKING_NOWARN_DEPRECATED
# ifdef __GNUC__
#  define AQBANKING_DEPRECATED __attribute((__deprecated__))
# else
#  define AQBANKING_DEPRECATED
# endif
# else
#  define AQBANKING_DEPRECATED
#endif

#define AQBANKING_LOGDOMAIN "aqbanking"


/** @defgroup AB_ERROR (Error Codes)
 * @ingroup AB_C_INTERFACE
 */
/*@{*/
#define AB_ERROR_SUCCESS           0
#define AB_ERROR_GENERIC         (-1)
#define AB_ERROR_NOT_SUPPORTED   (-2)
#define AB_ERROR_NOT_AVAILABLE   (-3)
#define AB_ERROR_BAD_CONFIG_FILE (-4)
#define AB_ERROR_INVALID         (-5)
#define AB_ERROR_NETWORK         (-6)
#define AB_ERROR_NOT_FOUND       (-7)
#define AB_ERROR_EMPTY           (-8)
#define AB_ERROR_USER_ABORT      (-9)
#define AB_ERROR_FOUND           (-10)
#define AB_ERROR_NO_DATA         (-11)
#define AB_ERROR_NOFN            (-12)
#define AB_ERROR_UNKNOWN_ACCOUNT (-13)
#define AB_ERROR_NOT_INIT        (-14)
#define AB_ERROR_SECURITY        (-15)
#define AB_ERROR_BAD_DATA        (-16)
#define AB_ERROR_UNKNOWN         (-17)
#define AB_ERROR_ABORTED         (-18)
#define AB_ERROR_DEFAULT_VALUE   (-19)
#define AB_ERROR_BAD_PIN         (-20)
#define AB_ERROR_IO              (-21)

#define AB_ERROR_USER1           (-128)
#define AB_ERROR_USER2           (-129)
#define AB_ERROR_USER3           (-130)
#define AB_ERROR_USER4           (-131)
/*@}*/




#endif /* AQBANKING_ERROR_H */


