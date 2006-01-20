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

#ifndef QBANKING_CFGTABSETTINGS_H
#define QBANKING_CFGTABSETTINGS_H


#include <gwenhywfar/types.h>
#include <aqbanking/banking.h>
#include <aqbanking/account.h>

#include "qbcfgtab.h"


class QBCfgTab;


class QBANKING_API QBCfgTabSettings: public QBCfgTab {
  Q_OBJECT
private:

public:
  QBCfgTabSettings(QBanking *qb,
                   QWidget *parent=0,
                   const char *name=0,
                   WFlags f=0);
  virtual ~QBCfgTabSettings();

  bool toGui();
  bool fromGui();

  void addAccountsPage();
  void addUsersPage();
  void addBackendsPage();

signals:
  void signalUpdate();

public slots:
  void slotUpdate();
};


#endif
