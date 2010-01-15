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

#ifndef QBANKING_CFGTABPAGEACCOUNTS_H
#define QBANKING_CFGTABPAGEACCOUNTS_H


#include <gwenhywfar/types.h>
#include <aqbanking/banking.h>
#include <aqbanking/account.h>

#include "qbcfgtabpage.h"
#include "qbcfgtabpageaccounts.ui.h"


class QBCfgTabPage;
class QBCfgTabPageAccountsUi;


class QBCfgTabPageAccounts: public QBCfgTabPage {
  Q_OBJECT
private:
  Ui_QBCfgTabPageAccountsUi _realPage;

  void _accountRescan();

public:
  QBCfgTabPageAccounts(QBanking *qb,
                       QWidget *parent=0,
                       const char *name=0,
                       Qt::WFlags f=0);
  virtual ~QBCfgTabPageAccounts();

  virtual bool toGui();
  virtual bool fromGui();
  virtual void updateView();

signals:
  void signalUpdate();

public slots:
  void slotAccountNew();
  void slotAccountEdit();
  void slotAccountDel();
  void slotUpdate();
};


#endif
