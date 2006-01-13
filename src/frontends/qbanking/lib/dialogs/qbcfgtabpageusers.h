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

#ifndef QBANKING_CFGTABPAGEUSERS_H
#define QBANKING_CFGTABPAGEUSERS_H


#include <gwenhywfar/types.h>
#include <aqbanking/banking.h>
#include <aqbanking/user.h>

#include "qbcfgtabpage.h"


class QBCfgTabPage;
class QBCfgTabPageUsersUi;


class QBCfgTabPageUsers: public QBCfgTabPage {
  Q_OBJECT
private:
  QBCfgTabPageUsersUi *_realPage;

  void _userRescan();

public:
  QBCfgTabPageUsers(QBanking *qb,
                    QWidget *parent=0,
                    const char *name=0,
                    WFlags f=0);
  virtual ~QBCfgTabPageUsers();

  virtual bool toGui();
  virtual bool fromGui();
  virtual void updateView();

public slots:
  void slotUserNew();
  void slotUserEdit();
  void slotUserDel();

};


#endif
