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

#ifndef QBANKING_CFGTABPAGEACCOUNT_H
#define QBANKING_CFGTABPAGEACCOUNT_H


#include <gwenhywfar/types.h>
#include <aqbanking/banking.h>

#include "qbcfgtabpage.h"



class QBCfgTabPageAccount: public QBCfgTabPage {
private:
  AB_ACCOUNT *_account;

public:
  QBCfgTabPageAccount(QBanking *qb,
		      const QString &title,
		      AB_ACCOUNT *a,
		      QWidget *parent=0, const char *name=0, WFlags f=0);
  virtual ~QBCfgTabPageAccount();

  AB_ACCOUNT *getAccount();

};


#endif
