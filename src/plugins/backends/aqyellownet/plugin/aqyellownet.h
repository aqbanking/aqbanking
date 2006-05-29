/***************************************************************************
 $RCSfile: aqyellownet.h,v $
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQYELLOWNET_AQYELLOWNET_H
#define AQYELLOWNET_AQYELLOWNET_H


#include <aqbanking/system.h>

#ifdef BUILDING_AQYELLOWNET
# /* building AqYELLOWNET */
# if AQBANKING_SYS_IS_WINDOWS
#   /* for windows */
#   ifdef __declspec
#     define AQYELLOWNET_API __declspec (dllexport)
#   else /* if __declspec */
#     define AQYELLOWNET_API
#   endif /* if NOT __declspec */
# else
#   /* for non-win32 */
#   ifdef GCC_WITH_VISIBILITY_ATTRIBUTE
#     define AQYELLOWNET_API __attribute__((visibility("default")))
#   else
#     define AQYELLOWNET_API
#   endif
# endif
#else
# /* not building AqYELLOWNET */
# if AQBANKING_SYS_IS_WINDOWS
#   /* for windows */
#   ifdef __declspec
#     define AQYELLOWNET_API __declspec (dllimport)
#   else /* if __declspec */
#     define AQYELLOWNET_API
#   endif /* if NOT __declspec */
# else
#   /* for non-win32 */
#   define AQYELLOWNET_API
# endif
#endif


#define AQYELLOWNET_LOGDOMAIN "aqyellownet"


#endif /* AQYELLOWNET_AQYELLOWNET_H */

