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

#ifndef QBANKING_EDITACCOUNT_H
#define QBANKING_EDITACCOUNT_H


#include <q4banking/qbcfgtab.h>

class QBanking;


class Q4BANKING_API QBEditAccount: public QBCfgTab {
private:
  AB_ACCOUNT *_account;

public:
  QBEditAccount(QBanking *kb,
                AB_ACCOUNT *a,
                QWidget* parent=0,
                const char* name=0,
                Qt::WFlags fl=0);
  ~QBEditAccount();

  bool fromGui(bool doLock);

  static bool editAccount(QBanking *kb, AB_ACCOUNT *a,
			  bool doLock,
			  QWidget* parent=0);

};



#endif // QBANKING_EDITACCOUNT_H




