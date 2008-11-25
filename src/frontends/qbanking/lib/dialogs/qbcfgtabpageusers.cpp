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
  GWEN_DB_NODE *dbConfig=NULL;
  int rv;

  rv=getBanking()->loadSharedSubConfig("qbanking",
				       "settings/gui/userList",
				       &dbConfig,
				       0);
  if (rv==0) {
    int i, j;

    assert(dbConfig);

    /* setup user list view */
    _realPage->userList->setResizeMode(QListView::NoColumn);
    for (i=0; i<_realPage->userList->columns(); i++) {
      _realPage->userList->setColumnWidthMode(i, QListView::Manual);
      j=GWEN_DB_GetIntValue(dbConfig, "columns", i, -1);
      if (j!=-1)
	_realPage->userList->setColumnWidth(i, j);
    } /* for */
    _realPage->userList->setSelectionMode(QListView::Single);

    _userRescan();
    GWEN_DB_Group_free(dbConfig);
    return true;
  }
  else {
    DBG_INFO(0, "here (%d)", rv);
    return false;
  }
}



bool QBCfgTabPageUsers::fromGui() {
  GWEN_DB_NODE *dbConfig;
  int i, j;
  int rv;

  dbConfig=GWEN_DB_Group_new("config");
  assert(dbConfig);

  /* save account list view settings */
  for (i=0; i<_realPage->userList->columns(); i++) {
    j=_realPage->userList->columnWidth(i);
    GWEN_DB_SetIntValue(dbConfig, GWEN_DB_FLAGS_DEFAULT,
			"columns", j);
  } /* for */

  rv=getBanking()->saveSharedSubConfig("qbanking",
				       "settings/gui/userList",
				       dbConfig,
				       0);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    GWEN_DB_Group_free(dbConfig);
    return false;
  }
  GWEN_DB_Group_free(dbConfig);
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
    emit signalUpdate();
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
                          QMessageBox::Ok,QMessageBox::NoButton);
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
  emit signalUpdate();
}



void QBCfgTabPageUsers::slotUserDel() {

  std::list<AB_USER*> ul =
    _realPage->userList->getSelectedUsers();
  if (ul.empty()) {
    QMessageBox::critical(this,
                          tr("Selection Error"),
                          tr("No user selected."),
                          QMessageBox::Retry,QMessageBox::NoButton);
    return;
  }

  AB_USER *u = ul.front();
  AB_BANKING *ab = getBanking()->getCInterface();
  AB_ACCOUNT *oneacc = AB_Banking_FindFirstAccountOfUser(ab, u);
  if (oneacc != NULL) {
    // FIXME: Instead of this message, the user should be asked
    // whether that account should be deleted as well, and then
    // FindFirstAccountOfUser should be queried again until it returns
    // NULL.
    QMessageBox::critical(this,
                          tr("User belongs to an account"),
                          tr("This user still belongs to an existing account. "
			     "Please remove the user from the accounts "
			     "or delete the account first."),
                          QMessageBox::Retry,QMessageBox::NoButton);
    return;
  }
  int r = QMessageBox::warning(this,
				tr("Really delete user?"),
				tr("You are about to delete a user. This action will "
				   "take effect immediately and cannot be undone. "
				   "(You can add the user later again, of course.)\n\n"
				   "Do you want to delete this user?"),
				QMessageBox::Yes,QMessageBox::Abort);
  if (r != 0 && r != QMessageBox::Yes) {
    return;
  }
  int rv = AB_Banking_DeleteUser(ab, u);
  if (rv == 0) {
    DBG_INFO(0, "Accepted");
  }
  else {
    DBG_INFO(0, "Rejected");
  }
  emit signalUpdate();
  updateView();
}



void QBCfgTabPageUsers::updateView() {
  _userRescan();
}



void QBCfgTabPageUsers::slotUpdate() {
  DBG_INFO(AQBANKING_LOGDOMAIN, "Updating users view");
  updateView();
}



#include "qbcfgtabpageusers.moc"



