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

#ifndef QBANKING_CFGTABPAGEACCOUNTGEN_H
#define QBANKING_CFGTABPAGEACCOUNTGEN_H


#include <gwenhywfar/types.h>
#include <aqbanking/banking.h>
#include <aqbanking/account.h>

#include "qbcfgtabpageaccount.h"


class QBCfgTabPageAccountGeneralUi;


class QBCfgTabPageAccountGeneral: public QBCfgTabPageAccount {
  Q_OBJECT
private:
  QBCfgTabPageAccountGeneralUi *_realPage;

  bool _listHasUser(AB_USER_LIST2 *ul, AB_USER *u);
  void _addUsersToLists(AB_USER_LIST2 *ulAll, AB_USER_LIST2 *ulSel);

public:
  QBCfgTabPageAccountGeneral(QBanking *qb,
                             AB_ACCOUNT *a,
                             QWidget *parent=0,
                             const char *name=0,
                             WFlags f=0);
  virtual ~QBCfgTabPageAccountGeneral();

  virtual bool fromGui();
  virtual bool toGui();
  virtual bool checkGui();

  virtual void updateView();

public slots:
  void slotBankIdButtonClicked();
  void slotLeftButtonClicked();
  void slotRightButtonClicked();
};


#endif
