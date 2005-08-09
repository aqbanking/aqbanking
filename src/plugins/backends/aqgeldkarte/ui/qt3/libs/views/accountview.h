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



#ifndef KBANKING_ACCOUNTVIEW_H
#define KBANKING_ACCOUNTVIEW_H


#include "accountview.ui.h"


class AccountView;


#include "accountlist.h"
#include <qbanking/qbanking.h>


class AccountView: public AccountViewUi {
  Q_OBJECT
public:
  AccountView(QBanking *app,
              AB_PROVIDER *pro,
              QWidget* parent=0, const char* name=0, WFlags fl=0);
  virtual ~AccountView();

  bool init();
  bool fini();

  void update();

private:
  QBanking *_app;
  AB_PROVIDER *_provider;

protected slots:
  void slotNew();
  void slotEdit();
  void slotRemove();

};










#endif /* KBANKING_ACCOUNTVIEW_H */



