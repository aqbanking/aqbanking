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

#ifndef QBANKING_CFGTABPAGEBACKENDS_H
#define QBANKING_CFGTABPAGEBACKENDS_H


#include <gwenhywfar/types.h>
#include <aqbanking/banking.h>

#include "qbcfgtabpage.h"
#include "qbcfgtabpagebackends.ui.h"


class QBCfgTabPage;


class QBCfgTabPageBackends: public QBCfgTabPage {
  Q_OBJECT
private:
  Ui_QBCfgTabPageBackendsUi _realPage;

  void _backendRescan();

public:
  QBCfgTabPageBackends(QBanking *qb,
                       QWidget *parent=0,
                       const char *name=0,
                       Qt::WFlags f=0);
  virtual ~QBCfgTabPageBackends();

  virtual bool toGui();
  virtual bool fromGui();
  virtual void updateView();

signals:
  void signalUpdate();

public slots:
  void slotUpdate();
};


#endif
