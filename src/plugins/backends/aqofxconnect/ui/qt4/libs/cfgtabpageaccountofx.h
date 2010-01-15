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


#ifndef AQOFX_CFGTABPAGEACCOUNT_H
#define AQOFX_CFGTABPAGEACCOUNT_H


#include <q4banking/qbcfgtabpageaccount.h>
#include "cfgtabpageaccountofx.ui.h"


class CfgTabPageAccountOfx: public QBCfgTabPageAccount {
  Q_OBJECT
private:
  Ui_CfgTabPageAccountOfxUi _realPage;

public:
  CfgTabPageAccountOfx(QBanking *qb,
                       AB_ACCOUNT *a,
                       QWidget *parent=0, const char *name=0, Qt::WFlags f=0);
  virtual ~CfgTabPageAccountOfx();

  virtual bool fromGui();
  virtual bool toGui();
  virtual bool checkGui();

};

#endif // AQDTAUS_CFGTABPAGEACCOUNT_H
