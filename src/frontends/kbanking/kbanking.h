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

#ifndef AQHBCI_KDE_BANKING_H
#define AQHBCI_KDE_BANKING_H


#ifdef BUILDING_KBANKING
# /* building KBanking */
# if AQBANKING_SYS_IS_WINDOWS
#   /* for windows */
#   ifdef __declspec
#     define KBANKING_API __declspec (dllexport)
#   else /* if __declspec */
#     define KBANKING_API
#   endif /* if NOT __declspec */
# else
#   /* for non-win32 */
#   ifdef GCC_WITH_VISIBILITY_ATTRIBUTE
#     define KBANKING_API __attribute__((visibility("default")))
#   else
#     define KBANKING_API
#   endif
# endif
#else
# /* not building KBanking */
# if AQBANKING_SYS_IS_WINDOWS
#   /* for windows */
#   ifdef __declspec
#     define KBANKING_API __declspec (dllimport)
#   else /* if __declspec */
#     define KBANKING_API
#   endif /* if NOT __declspec */
# else
#   /* for non-win32 */
#   define KBANKING_API
# endif
#endif

#ifdef GCC_WITH_VISIBILITY_ATTRIBUTE
# define KBANKING_EXPORT __attribute__((visibility("default")))
# define KBANKING_NOEXPORT __attribute__((visibility("hidden")))
#else
# define KBANKING_EXPORT
# define KBANKING_NOEXPORT
#endif


#include <aqbanking/banking.h>
#include <aqbanking/accstatus.h>

#include <qobject.h>
#include <qdatetime.h>
#include <qstring.h>

#include <list>

class QTranslator;

class KBanking;

#include <qbanking/qbanking.h>
#include <kbanking/kbflagstaff.h>


class KBanking: public QBanking {
private:
  KBFlagStaff *_flagStaff;
  QTranslator *_translator;

  AB_ACCOUNT *_getAccount(const char *accountId);

public:
  KBanking(const char *appname,
           const char *fname=0);
  virtual ~KBanking();

  int init();
  int fini();


  KBFlagStaff *flagStaff();

  int executeQueue(AB_IMEXPORTER_CONTEXT *ctx);

  virtual bool importContext(AB_IMEXPORTER_CONTEXT *ctx);
  virtual bool importAccountInfo(AB_IMEXPORTER_ACCOUNTINFO *ai);

  virtual bool importContext(AB_IMEXPORTER_CONTEXT *ctx,
                             GWEN_TYPE_UINT32 flags);

  virtual bool importAccountInfo(AB_IMEXPORTER_ACCOUNTINFO *ai,
                                 GWEN_TYPE_UINT32 flags);

};




#endif /* AQHBCI_KDE_BANKING_H */


