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

#include "qbcfgtabpageuser.h"


class QBCfgTabPageUserGeneralUi;


class QBCfgTabPageUserGeneral: public QBCfgTabPageUser {
  Q_OBJECT
private:
  QBCfgTabPageUserGeneralUi *_realPage;

public:
  QBCfgTabPageUserGeneral(QBanking *qb,
                          AB_USER *u,
                          QWidget *parent=0, const char *name=0, WFlags f=0);
  virtual ~QBCfgTabPageUserGeneral();

  virtual bool fromGui();
  virtual bool toGui();
  virtual bool checkGui();

public slots:
  void slotBankIdButtonClicked();

};


#endif
