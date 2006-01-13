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


#ifndef AQHBCI_CFGTABPAGEUSERHBCI_H
#define AQHBCI_CFGTABPAGEUSERHBCI_H


#include "cfgtabpageuserhbci.ui.h"
#include <qbanking/qbcfgtabpageuser.h>
#include <aqhbci/hbci.h>


class QComboBox;


class CfgTabPageUserHbci: public QBCfgTabPageUser {
  Q_OBJECT
private:
  AH_HBCI *_hbci;
  CfgTabPageUserHbciUi *_realPage;
  bool _withHttp;

public:
  CfgTabPageUserHbci(QBanking *qb,
                     AB_USER *u,
                     QWidget *parent=0, const char *name=0, WFlags f=0);
  virtual ~CfgTabPageUserHbci();

  virtual bool fromGui();
  virtual bool toGui();
  virtual bool checkGui();

public slots:
  void slotStatusChanged(int i);
  void slotGetServerKeys();
  void slotGetSysId();
  void slotGetAccounts();
  void slotFinishUser();
  void accept();

private:
  void _setComboTextIfPossible(QComboBox *qb, const QString &s);

};






#endif // AQHBCI_EDITUSER_H

