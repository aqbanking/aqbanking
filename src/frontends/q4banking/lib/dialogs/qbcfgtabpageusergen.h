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

#ifndef QBANKING_CFGTABPAGEUSERGEN_H
#define QBANKING_CFGTABPAGEUSERGEN_H


#include <gwenhywfar/types.h>
#include <aqbanking/banking.h>
#include <q4banking/banking.h>

#include "qbcfgtabpageuser.h"
#include "qbcfgtabpageusergen.ui.h"




class QBCfgTabPageUserGeneral: public QBCfgTabPageUser {
  Q_OBJECT
private:
  Ui_QBCfgTabPageUserGeneralUi _realPage;

public:
  QBCfgTabPageUserGeneral(QBanking *qb,
                          AB_USER *u,
                          QWidget *parent=0, const char *name=0, Qt::WFlags f=0);
  virtual ~QBCfgTabPageUserGeneral();

  virtual bool fromGui();
  virtual bool toGui();
  virtual bool checkGui();

  virtual void updateView();

public slots:
  void slotBankIdButtonClicked();

};


#endif
