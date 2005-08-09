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

#ifndef QT_OFXSETTINGS_H
#define QT_OFXSETTINGS_H


#include "settings.ui.h"


class QBanking;
class AccountView;
class UserView;


class OfxSettings: public OfxSettingsUi {
  Q_OBJECT
private:
  QBanking *_app;
  AccountView *_accountView;
  UserView *_userView;
public:
  OfxSettings(QBanking *app,
              QWidget * parent=0,
              const char * name=0,
              bool modal=FALSE,
              WFlags f=0);
  ~OfxSettings();

  bool init();
  bool fini();

  void update();

};


#endif  // QT_OFXSETTINGS_H

