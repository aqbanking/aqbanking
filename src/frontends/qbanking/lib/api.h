/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: qbanking.h 1137 2007-01-19 19:48:38Z martin $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef QBANKING_API_H
#define QBANKING_API_H

#include <aqbanking/banking.h>


#ifdef BUILDING_QBANKING
# /* building AqBanking */
# if AQBANKING_SYS_IS_WINDOWS
#   /* for windows */
#   ifdef __declspec
#     define QBANKING_API __declspec (dllexport)
#   else /* if __declspec */
#     define QBANKING_API
#   endif /* if NOT __declspec */
# else
#   /* for non-win32 */
#   ifdef GCC_WITH_VISIBILITY_ATTRIBUTE
#     define QBANKING_API __attribute__((visibility("default")))
#   else
#     define QBANKING_API
#   endif
# endif
#else
# /* not building AqBanking */
# if AQBANKING_SYS_IS_WINDOWS
#   /* for windows */
#   ifdef __declspec
#     define QBANKING_API __declspec (dllimport)
#   else /* if __declspec */
#     define QBANKING_API
#   endif /* if NOT __declspec */
# else
#   /* for non-win32 */
#   define QBANKING_API
# endif
#endif

#ifdef GCC_WITH_VISIBILITY_ATTRIBUTE
# define QBANKING_EXPORT __attribute__((visibility("default")))
# define QBANKING_NOEXPORT __attribute__((visibility("hidden")))
#else
# define QBANKING_EXPORT
# define QBANKING_NOEXPORT
#endif





#endif /* QBANKING_API_H */


