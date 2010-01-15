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

#include <q4banking/qbanking.h>
#include <q4banking/qbaccountlist.h>

#include "qbmapaccount.ui.h"


class QBanking;


class Q4BANKING_API QBMapAccount: public QDialog, public Ui_QBMapAccountUi {
  Q_OBJECT
public:
  QBMapAccount(QBanking *kb,
               const char *bankCode,
               const char *accountId,
               QWidget* parent=0,
               const char* name=0,
               bool modal=FALSE,
               Qt::WFlags fl=0);
  ~QBMapAccount();

  AB_ACCOUNT *getAccount();

  void accept();

protected slots:
  void slotSelectionChanged();
  void slotHelpClicked();

private:
  QBanking *_banking;
  AB_ACCOUNT *_account;
  QBAccountListView *_accountList;
};





#endif /* QBANKING_MAPACCOUNT_H */

