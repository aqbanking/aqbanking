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



#ifndef KBANKING_USERVIEW_H
#define KBANKING_USERVIEW_H


#include "userview.ui.h"


class UserView;
class OfxSettings;


#include "userlist.h"
#include <qbanking/qbanking.h>


class UserView: public UserViewUi {
  Q_OBJECT
public:
  UserView(QBanking *app,
           OfxSettings *settings,
           QWidget* parent=0, const char* name=0, WFlags fl=0);
  virtual ~UserView();

  bool init();
  bool fini();

  void update();

private:
  QBanking *_app;
  AB_PROVIDER *_provider;
  OfxSettings *_settings;

protected slots:
  void slotNew();
  void slotEdit();
  void slotRemove();
  void slotGetAccounts();

};










#endif /* KBANKING_USERVIEW_H */



