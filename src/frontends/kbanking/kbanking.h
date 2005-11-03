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

  int executeQueue();

  virtual bool importContext(AB_IMEXPORTER_CONTEXT *ctx);
  virtual bool importAccountInfo(AB_IMEXPORTER_ACCOUNTINFO *ai);

  virtual bool importContext(AB_IMEXPORTER_CONTEXT *ctx,
                             GWEN_TYPE_UINT32 flags);

  virtual bool importAccountInfo(AB_IMEXPORTER_ACCOUNTINFO *ai,
                                 GWEN_TYPE_UINT32 flags);

};




#endif /* AQHBCI_KDE_BANKING_H */


