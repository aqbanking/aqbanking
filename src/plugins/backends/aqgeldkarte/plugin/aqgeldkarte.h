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


#ifndef AQGELDKARTE_AQGELDKARTE_H
#define AQGELDKARTE_AQGELDKARTE_H


#include <aqbanking/system.h>

#ifdef BUILDING_AQGELDKARTE
# /* building AqGELDKARTE */
# if AQBANKING_SYS_IS_WINDOWS
#   /* for windows */
#   ifdef __declspec
#     define AQGELDKARTE_API __declspec (dllexport)
#   else /* if __declspec */
#     define AQGELDKARTE_API
#   endif /* if NOT __declspec */
# else
#   /* for non-win32 */
#   ifdef GCC_WITH_VISIBILITY_ATTRIBUTE
#     define AQGELDKARTE_API __attribute__((visibility("default")))
#   else
#     define AQGELDKARTE_API
#   endif
# endif
#else
# /* not building AqGELDKARTE */
# if AQBANKING_SYS_IS_WINDOWS
#   /* for windows */
#   ifdef __declspec
#     define AQGELDKARTE_API __declspec (dllimport)
#   else /* if __declspec */
#     define AQGELDKARTE_API
#   endif /* if NOT __declspec */
# else
#   /* for non-win32 */
#   define AQGELDKARTE_API
# endif
#endif

#define AQGELDKARTE_LOGDOMAIN "aqgeldkarte"
#define AG_PROVIDER_NAME "aqgeldkarte"


#endif /* AQGELDKARTE_AQGELDKARTE_H */

