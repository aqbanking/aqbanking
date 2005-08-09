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


#include "userview.h"
#include "edituser.h"
#include "settings.h"

#include <qevent.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qmessagebox.h>
#include <qlayout.h>

#include <aqofxconnect/provider.h>
#include <aqofxconnect/user.h>
#include <gwenhywfar/debug.h>

#ifdef WIN32
# define strcasecmp stricmp
#endif


#define BUTTON_WIDTH 110


UserView::UserView(QBanking *app,
                   OfxSettings *settings,
                   QWidget* parent,
                   const char* name,
                   WFlags fl)
:UserViewUi(parent, name, fl)
, _app(app)
, _provider(0)
,_settings(settings){
  _provider=AB_Banking_GetProvider(_app->getCInterface(), "aqofxconnect");
  assert(_provider);
  QObject::connect((QObject*)newButton, SIGNAL(clicked()),
                   this, SLOT(slotNew()));
  QObject::connect((QObject*)editButton, SIGNAL(clicked()),
                   this, SLOT(slotEdit()));
  QObject::connect((QObject*)removeButton, SIGNAL(clicked()),
                   this, SLOT(slotRemove()));
  QObject::connect((QObject*)getAccountListButton, SIGNAL(clicked()),
                   this, SLOT(slotGetAccounts()));
}



UserView::~UserView(){
}



void UserView::update(){
  AO_BANK_LIST *bl;

  userListView->clear();
  bl=AO_Provider_GetBanks(_provider);
  if (bl) {
    AO_BANK *b;

    b=AO_Bank_List_First(bl);
    while(b) {
      AO_USER_LIST *ul;
      ul=AO_Bank_GetUsers(b);
      if (ul)
        userListView->addUsers(ul);
      b=AO_Bank_List_Next(b);
    }
  }
}



bool UserView::init(){
  GWEN_DB_NODE *db;

  db=_app->getAppData();
  assert(db);
  db=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                      "gui/views/userview/dynamic");
  if (db) {
    int i, j;
    const char *p;

    p=GWEN_DB_GetCharValue(db, "sortOrder", 0, "ascending");
    if (p) {
      if (strcasecmp(p, "ascending")==0)
        userListView->setSortOrder(Qt::Ascending);
      else
        if (strcasecmp(p, "descending")==0)
          userListView->setSortOrder(Qt::Descending);
    }
    i=GWEN_DB_GetIntValue(db, "sortColumn", 0, -1);
    if (i!=-1)
      userListView->setSortColumn(i);

    /* found settings */
    for (i=0; i<userListView->columns(); i++) {
      userListView->setColumnWidthMode(i, QListView::Manual);
      j=GWEN_DB_GetIntValue(db, "columns", i, -1);
      if (j!=-1)
        userListView->setColumnWidth(i, j);
    } /* for */
  } /* if settings */

  update();
  return true;
}



bool UserView::fini(){
  GWEN_DB_NODE *db;
  int i, j;

  db=_app->getAppData();
  assert(db);
  db=GWEN_DB_GetGroup(db,
                      GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                      "gui/views/userview/dynamic");

  switch(userListView->sortOrder()) {
  case Qt::Ascending:
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT,
                         "sortOrder", "ascending");
    break;
  case Qt::Descending:
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT,
                         "sortOrder", "descending");
    break;
  default:
    break;
  }

  for (i=0; i<userListView->columns(); i++) {
    j=userListView->columnWidth(i);
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT,
                        "columns", j);
  } /* for */

  return true;
}



void UserView::slotNew(){
  AO_USER *u;

  /* get user */
  u=AO_User_new(0, 0);
  EditUser eu(_app, u, false, this, "EditUser", true);
  eu.setCaption(tr("Create User"));
  eu.init();
  if (eu.exec()==QDialog::Accepted) {
    update();
  }
  else
    AO_User_free(u);
}



void UserView::slotEdit(){
  AO_USER *u;

  /* get user */
  u=userListView->getCurrentUser();
  if (!u) {
    DBG_NOTICE(0, "No user selected");
    QMessageBox::warning(0,
                         tr("No Selection"),
                         tr("Please select u user first."),
                         tr("Dismiss"), 0, 0, 0);
    return;
  }

  EditUser eu(_app, u, false, this, "EditUser", true);
  eu.init();
  if (eu.exec()==QDialog::Accepted) {
    update();
  }
}



void UserView::slotRemove(){
}



void UserView::slotGetAccounts() {
  AO_USER *u;
  AO_BANK *b;
  const char *country;
  const char *bankId;
  const char *userId;

  /* get user */
  u=userListView->getCurrentUser();
  if (!u) {
    DBG_NOTICE(0, "No user selected");
    QMessageBox::warning(0,
                         tr("No Selection"),
                         tr("Please select u user first."),
                         tr("Dismiss"), 0, 0, 0);
    return;
  }

  b=AO_User_GetBank(u);
  assert(b);
  country=AO_Bank_GetCountry(b);
  bankId=AO_Bank_GetBankId(b);
  userId=AO_User_GetUserId(u);
  if (!country || !bankId || !userId) {
    DBG_NOTICE(0, "Bad user selected");
    QMessageBox::warning(0,
                         tr("Bad Selection"),
                         tr("The selected user has an incomplete setup."),
                         tr("Dismiss"), 0, 0, 0);
    return;
  }
  AO_Provider_RequestAccounts(_provider, country, bankId, userId);
  _settings->update();
}






