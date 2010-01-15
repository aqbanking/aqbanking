/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef Q4BANKING_API_H
#define Q4BANKING_API_H

#include <aqbanking/banking.h>


#ifdef BUILDING_Q4BANKING
# /* building AqBanking */
# if AQBANKING_SYS_IS_WINDOWS
#   /* for windows */
#   ifdef __declspec
#     define Q4BANKING_API __declspec (dllexport)
#   else /* if __declspec */
#     define Q4BANKING_API
#   endif /* if NOT __declspec */
# else
#   /* for non-win32 */
#   ifdef GCC_WITH_VISIBILITY_ATTRIBUTE
#     define Q4BANKING_API __attribute__((visibility("default")))
#   else
#     define Q4BANKING_API
#   endif
# endif
#else
# /* not building AqBanking */
# if AQBANKING_SYS_IS_WINDOWS
#   /* for windows */
#   ifdef __declspec
#     define Q4BANKING_API __declspec (dllimport)
#   else /* if __declspec */
#     define Q4BANKING_API
#   endif /* if NOT __declspec */
# else
#   /* for non-win32 */
#   define Q4BANKING_API
# endif
#endif

#ifdef GCC_WITH_VISIBILITY_ATTRIBUTE
# define Q4BANKING_EXPORT __attribute__((visibility("default")))
# define Q4BANKING_NOEXPORT __attribute__((visibility("hidden")))
#else
# define Q4BANKING_EXPORT
# define Q4BANKING_NOEXPORT
#endif





#endif /* Q4BANKING_API_H */


