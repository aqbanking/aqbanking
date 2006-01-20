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

#ifndef QBANKING_MAPACCOUNT_H
#define QBANKING_MAPACCOUNT_H


#include "qbmapaccount.ui.h"
#include "qbaccountlist.h"


class QBanking;


class QBANKING_API QBMapAccount: public QBMapAccountUi {
  Q_OBJECT
public:
  QBMapAccount(QBanking *kb,
               const char *bankCode,
               const char *accountId,
               QWidget* parent=0,
               const char* name=0,
               bool modal=FALSE,
               WFlags fl=0);
  ~QBMapAccount();

  AB_ACCOUNT *getAccount();

  void accept();

protected slots:
  void slotSelectionChanged();

private:
  QBanking *_banking;
  AB_ACCOUNT *_account;
  QBAccountListView *_accountList;
};





#endif /* QBANKING_MAPACCOUNT_H */

