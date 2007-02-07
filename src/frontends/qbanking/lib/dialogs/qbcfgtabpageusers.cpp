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


// QBanking includes
#include "qbcfgtabpageusers.h"
#include "qbcfgtabpageusers.ui.h"
#include "qbedituser.h"
#include "qbuserlist.h"
#include "qbselectbackend.h"
#include "qbcfgmodule.h"
#include "qbanking.h"

// Gwenhywfar includes
#include <gwenhywfar/debug.h>

// QT includes
#include <qlineedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include <qtextcodec.h>




QBCfgTabPageUsers::QBCfgTabPageUsers(QBanking *qb,
                                     QWidget *parent,
                                     const char *name,
                                     WFlags f)
:QBCfgTabPage(qb, tr("Users"), parent, name, f){
  _realPage=new QBCfgTabPageUsersUi(this);
  addWidget(_realPage);
  _realPage->show();

  setHelpSubject("QBCfgTabPageUsers");
  setDescription(tr("This page allows you to create, edit and remove"
                    " users from AqBanking."));

  QObject::connect(_realPage->userNewButton, SIGNAL(clicked()),
                   this, SLOT(slotUserNew()));
  QObject::connect(_realPage->userEditButton, SIGNAL(clicked()),
                   this, SLOT(slotUserEdit()));
  QObject::connect(_realPage->userDeleteButton, SIGNAL(clicked()),
                   this, SLOT(slotUserDel()));
}



QBCfgTabPageUsers::~QBCfgTabPageUsers() {
}



void QBCfgTabPageUsers::_userRescan(){
  _realPage->userList->clear();
  _realPage->userList->addUsers(getBanking()->getUsers());
}



bool QBCfgTabPageUsers::toGui() {
  GWEN_DB_NODE *dbSettings;
  int i, j;

  dbSettings=getBanking()->getSharedData("qbanking");
  assert(dbSettings);
  dbSettings=GWEN_DB_GetGroup(dbSettings, GWEN_DB_FLAGS_DEFAULT,
                              "settings");
  assert(dbSettings);

  /* setup user list view */
  _realPage->userList->setResizeMode(QListView::NoColumn);
  for (i=0; i<_realPage->userList->columns(); i++) {
    _realPage->userList->setColumnWidthMode(i, QListView::Manual);
    j=GWEN_DB_GetIntValue(dbSettings, "gui/userList/columns", i, -1);
    if (j!=-1)
      _realPage->userList->setColumnWidth(i, j);
  } /* for */
  _realPage->userList->setSelectionMode(QListView::Single);

  _userRescan();
  return true;
}



bool QBCfgTabPageUsers::fromGui() {
  GWEN_DB_NODE *dbSettings;
  int i, j;

  dbSettings=getBanking()->getSharedData("qbanking");
  assert(dbSettings);
  dbSettings=GWEN_DB_GetGroup(dbSettings, GWEN_DB_FLAGS_DEFAULT,
                              "settings");
  assert(dbSettings);

  /* save user list view settings */
  GWEN_DB_DeleteVar(dbSettings, "gui/userList/columns");
  for (i=0; i<_realPage->userList->columns(); i++) {
    j=_realPage->userList->columnWidth(i);
    GWEN_DB_SetIntValue(dbSettings, GWEN_DB_FLAGS_DEFAULT,
                        "gui/userList/columns", j);
  } /* for */

  return true;
}



void QBCfgTabPageUsers::slotUserNew() {
  QString backend;
  QString preBackend;
  const char *l;

  l=QTextCodec::locale();
  if (l) {
    QString ql;

    ql=QString::fromUtf8(l).lower();
    if (ql=="de" || ql=="de_de")
      preBackend="aqhbci";
  }
  backend=QBSelectBackend::selectBackend(getBanking(),
                                         preBackend,
                                         this);
  if (backend.isEmpty()) {
    DBG_INFO(0, "Aborted");
  }
  else {
    QBCfgModule *mod;
    std::string s;

    s=QBanking::QStringToUtf8String(backend);
    DBG_ERROR(0, "Selected backend: %s", s.c_str());
    mod=getBanking()->getConfigModule(s.c_str());
    if (mod) {
      if (mod->getFlags() & QBCFGMODULE_FLAGS_CAN_CREATE_USER) {
        int rv;

        rv=mod->createNewUser(this);
        if (rv) {
          DBG_ERROR(0, "No user created (%d)", rv);
        }
        else {
          DBG_NOTICE(0, "User created");
          getCfgTab()->updateViews();
        }
      }
      else {
        AB_USER *u;

        DBG_INFO(0, "Backend module does not provide a user wizard");
        u=AB_Banking_CreateUser(getBanking()->getCInterface(),
                                s.c_str());
        assert(u);
        if (QBEditUser::editUser(getBanking(), u, this)) {
          DBG_INFO(0, "Accepted, adding user");
          AB_Banking_AddUser(getBanking()->getCInterface(), u);
        }
        else {
          DBG_INFO(0, "Rejected");
          AB_User_free(u);
        }
      }
    }
    else {
      DBG_ERROR(0, "Config module for backend \"%s\" not found",
                s.c_str());
    }
    updateView();
  }
}



void QBCfgTabPageUsers::slotUserEdit() {
  std::list<AB_USER*> ul;
  AB_USER *u;

  ul=_realPage->userList->getSelectedUsers();
  if (ul.empty()) {
    QMessageBox::critical(this,
                          tr("Selection Error"),
                          tr("No user selected."),
                          QMessageBox::Retry,QMessageBox::NoButton);
    return;
  }
  u=ul.front();
  if (QBEditUser::editUser(getBanking(), u, this)) {
    DBG_INFO(0, "Accepted");
  }
  else {
    DBG_INFO(0, "Rejected");
  }
  updateView();
}



void QBCfgTabPageUsers::slotUserDel() {
  QMessageBox::critical(this,
                        tr("Error"),
                        tr("Not yet supported.\n"),
                        tr("Dismiss"));
}



void QBCfgTabPageUsers::updateView() {
  _userRescan();
}



#include "qbcfgtabpageusers.moc"



