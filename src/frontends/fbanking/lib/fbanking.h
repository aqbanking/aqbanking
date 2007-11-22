/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: qbanking.h 935 2006-02-14 02:11:55Z aquamaniac $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef FBANKING_BANKING_H
#define FBANKING_BANKING_H


#include <aqbanking/banking.h>
#include <aqbanking/system.h>

#ifdef BUILDING_FBANKING
# /* building AqBanking */
# if AQBANKING_SYS_IS_WINDOWS
#   /* for windows */
#   ifdef __declspec
#     define FBANKING_API __declspec (dllexport)
#   else /* if __declspec */
#     define FBANKING_API
#   endif /* if NOT __declspec */
# else
#   /* for non-win32 */
#   ifdef GCC_WITH_VISIBILITY_ATTRIBUTE
#     define FBANKING_API __attribute__((visibility("default")))
#   else
#     define FBANKING_API
#   endif
# endif
#else
# /* not building AqBanking */
# if AQBANKING_SYS_IS_WINDOWS
#   /* for windows */
#   ifdef __declspec
#     define FBANKING_API __declspec (dllimport)
#   else /* if __declspec */
#     define FBANKING_API
#   endif /* if NOT __declspec */
# else
#   /* for non-win32 */
#   define FBANKING_API
# endif
#endif

#ifdef GCC_WITH_VISIBILITY_ATTRIBUTE
# define FBANKING_EXPORT __attribute__((visibility("default")))
# define FBANKING_NOEXPORT __attribute__((visibility("hidden")))
#else
# define FBANKING_EXPORT
# define FBANKING_NOEXPORT
#endif



#include <fx.h>



#include <aqbanking/banking.h>
#include <aqbanking++/banking.h>
#include <aqbanking/accstatus.h>

#include <fx.h>

class FBanking;


class FBANKING_API FBanking: public AB_Banking {
protected:
  FBanking(): AB_Banking("none", NULL) {};

public:
  FBanking(const char *appName,
           const char *dirName=NULL);
  ~FBanking();

  int init();
  int fini();

};



#endif


