/***************************************************************************
    begin       : Sat May 08 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQPAYPAL_AQPAYPAL_H
#define AQPAYPAL_AQPAYPAL_H



#include <aqbanking/system.h>
#include <gwenhywfar/types.h>

/* ___________________________________________________________________________*/
#if 0

#ifdef BUILDING_AQPAYPAL
# /* building AqEBICS */
# if AQBANKING_SYS_IS_WINDOWS
#   /* for windows */
#   ifdef __declspec
#     define AQPAYPAL_API __declspec (dllexport)
#   else /* if __declspec */
#     define AQPAYPAL_API
#   endif /* if NOT __declspec */
# else
#   /* for non-win32 */
#   ifdef GCC_WITH_VISIBILITY_ATTRIBUTE
#     define AQPAYPAL_API __attribute__((visibility("default")))
#   else
#     define AQPAYPAL_API
#   endif
# endif
#else
# /* not building AqEBICS */
# if AQBANKING_SYS_IS_WINDOWS
#   /* for windows */
#   ifdef __declspec
#     define AQPAYPAL_API __declspec (dllimport)
#   else /* if __declspec */
#     define AQPAYPAL_API
#   endif /* if NOT __declspec */
# else
#   /* for non-win32 */
#   define AQPAYPAL_API
# endif
#endif

#endif
/* ___________________________________________________________________________*/


/* no longer export symbols */
#define AQPAYPAL_API


#define APY_PROVIDER_NAME "AQPAYPAL"

#define AQPAYPAL_LOGDOMAIN "aqpaypal"


#endif


