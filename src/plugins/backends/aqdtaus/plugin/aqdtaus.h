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


#ifndef AQDTAUS_AQDTAUS_H
#define AQDTAUS_AQDTAUS_H

#include <aqbanking/system.h>

#ifdef BUILDING_AQDTAUS
# /* building AqDTAUS */
# if AQBANKING_SYS_IS_WINDOWS
#   /* for windows */
#   ifdef __declspec
#     define AQDTAUS_API __declspec (dllexport)
#   else /* if __declspec */
#     define AQDTAUS_API
#   endif /* if NOT __declspec */
# else
#   /* for non-win32 */
#   ifdef GCC_WITH_VISIBILITY_ATTRIBUTE
#     define AQDTAUS_API __attribute__((visibility("default")))
#   else
#     define AQDTAUS_API
#   endif
# endif
#else
# /* not building AqDTAUS */
# if AQBANKING_SYS_IS_WINDOWS
#   /* for windows */
#   ifdef __declspec
#     define AQDTAUS_API __declspec (dllimport)
#   else /* if __declspec */
#     define AQDTAUS_API
#   endif /* if NOT __declspec */
# else
#   /* for non-win32 */
#   define AQDTAUS_API
# endif
#endif


#define AD_PROVIDER_NAME "aqdtaus"

#define AQDTAUS_LOGDOMAIN "aqdtaus"



#endif /* AQDTAUS_AQDTAUS_H */

