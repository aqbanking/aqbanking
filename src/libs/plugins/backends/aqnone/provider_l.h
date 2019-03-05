/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AN_PROVIDER_L_H
#define AN_PROVIDER_L_H

#include <aqbanking/backendsupport/provider_be.h>

#include <aqbanking/system.h>

/* ___________________________________________________________________________*/
#if 0

#ifdef BUILDING_AQNONE
# /* building AqNONE */
# if AQBANKING_SYS_IS_WINDOWS
#   /* for windows */
#   ifdef __declspec
#     define AQNONE_API __declspec (dllexport)
#   else /* if __declspec */
#     define AQNONE_API
#   endif /* if NOT __declspec */
# else
#   /* for non-win32 */
#   ifdef GCC_WITH_VISIBILITY_ATTRIBUTE
#     define AQNONE_API __attribute__((visibility("default")))
#   else
#     define AQNONE_API
#   endif
# endif
#else
# /* not building AqNONE */
# if AQBANKING_SYS_IS_WINDOWS
#   /* for windows */
#   ifdef __declspec
#     define AQNONE_API __declspec (dllimport)
#   else /* if __declspec */
#     define AQNONE_API
#   endif /* if NOT __declspec */
# else
#   /* for non-win32 */
#   define AQNONE_API
# endif
#endif

#endif
/* ___________________________________________________________________________*/


/* no longer export symbols */
#define AQNONE_API


#define AQNONE_BACKENDNAME "aqnone"
#define AQNONE_LOGDOMAIN "aqnone"


#ifdef __cplusplus
extern "C" {
#endif

AQNONE_API AB_PROVIDER *AN_Provider_new(AB_BANKING *ab);


#ifdef __cplusplus
}
#endif


#endif

