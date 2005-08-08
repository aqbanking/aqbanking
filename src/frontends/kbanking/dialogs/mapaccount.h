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

#ifndef AQBANKING_KDE_MAPACCOUNT_H
#define AQBANKING_KDE_MAPACCOUNT_H


#include "mapaccount.ui.h"
#include "accountlist.h"


class KBanking;


class MapAccount: public MapAccountUi {
  Q_OBJECT
public:
  MapAccount(KBanking *kb,
             const char *bankCode,
             const char *accountId,
             QWidget* parent=0,
             const char* name=0,
             bool modal=FALSE,
             WFlags fl=0);
  ~MapAccount();

  AB_ACCOUNT *getAccount();

  void accept();

protected slots:
  void slotSelectionChanged();

private:
  KBanking *_banking;
  AB_ACCOUNT *_account;
  AccountListView *_accountList;
};





#endif /* AQBANKING_KDE_MAPACCOUNT_H */

