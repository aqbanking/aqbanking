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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "settings.h"
#include "userview.h"
#include "accountview.h"

#include <qtabdialog.h>
#include <qtabwidget.h>
#include <qpushbutton.h>


OfxSettings::OfxSettings(QBanking *app,
                         QWidget * parent,
                         const char * name,
                         bool modal,
                         WFlags f)
:OfxSettingsUi(parent, name, modal, f), _app(app) {
  _userView=new UserView(app, this);
  tabWidget->addTab(_userView, tr("Users"));
  _accountView=new AccountView(app, this);
  tabWidget->addTab(_accountView, tr("Accounts"));
  QObject::connect(closeButton, SIGNAL(clicked()),
                   this, SLOT(close()));
}



OfxSettings::~OfxSettings(){
}



bool OfxSettings::init(){
  GWEN_DB_NODE *db;

  db=_app->getAppData();
  assert(db);
  db=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                      "gui/views/settings/dynamic");
  if (db) {
    int w, h;

    w=GWEN_DB_GetIntValue(db, "width", 0, 0);
    h=GWEN_DB_GetIntValue(db, "height", 0, 0);
    if (w && h)
      resize(w,h);
  } /* if settings */

  if (!_userView->init())
    return false;
  if (!_accountView->init()) {
    _userView->fini();
    return false;
  }
  return true;
}



bool OfxSettings::fini(){
  bool rv=true;
  GWEN_DB_NODE *db;

  rv&=_accountView->fini();
  rv&=_userView->fini();

  db=_app->getAppData();
  assert(db);
  db=GWEN_DB_GetGroup(db,
                      GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                      "gui/views/settings/dynamic");
  assert(db);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "width", width());
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "height", height());

  return rv;
}



void OfxSettings::update(){
  _userView->update();
  _accountView->update();
}








