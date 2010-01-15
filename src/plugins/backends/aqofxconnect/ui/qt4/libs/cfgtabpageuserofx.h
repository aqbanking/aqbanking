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


#ifndef AQOFX_CFGTABPAGEUSER_H
#define AQOFX_CFGTABPAGEUSER_H


#include <q4banking/qbcfgtabpageuser.h>
#include "cfgtabpageuserofx.ui.h"




class CfgTabPageUserOfx: public QBCfgTabPageUser {
  Q_OBJECT
private:
  Ui_CfgTabPageUserOfxUi _realPage;

public:
  CfgTabPageUserOfx(QBanking *qb,
                    AB_USER *u,
                    QWidget *parent=0, const char *name=0, Qt::WFlags f=0);
  virtual ~CfgTabPageUserOfx();

  virtual bool fromGui();
  virtual bool toGui();
  virtual bool checkGui();

public slots:
  void slotPickFid();
  void slotServerTest();
  void slotServerChanged(const QString &qs);
  void slotAccountCheckToggled(bool on);
  void slotGetAccounts();
};

#endif // AQDTAUS_CFGTABPAGEUSER_H
