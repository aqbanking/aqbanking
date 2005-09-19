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


#include "hbcisettings.h"
#include "editaccount.h"
#include "edituser.h"
#include "wizard.h"
#include "versionpicker.h"

#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qtimer.h>

#include <aqhbci/bank.h>
#include <aqhbci/outbox.h>
#include <aqhbci/adminjobs.h>
#include <gwenhywfar/debug.h>
#include <qbanking/qbanking.h>



HBCISettings::HBCISettings(AH_HBCI *hbci,
                           QBanking *kb,
                           QWidget* parent, const char* name,
                           bool modal, WFlags fl)
:HBCISettingsUi(parent, name, modal, fl)
,_hbci(hbci), _app(kb) {
  QObject::connect((QObject*)(helpButton),
                   SIGNAL(clicked()),
                   this,
                   SLOT(slotHelp()));

  // Manually create and add layout here because the .ui-generated
  // QGroupBox doesn't have one.
  accountBox->setColumnLayout(0, Qt::Vertical );
  QBoxLayout *accountBoxLayout = new QHBoxLayout( accountBox->layout() );

  _accList=new AccountListView((QWidget*)accountBox, "Accounts");
  accountBoxLayout->addWidget(_accList);

  QObject::connect((QObject*)(newUserButton),
                   SIGNAL(clicked()),
                   this,
		   SLOT(slotNewUser()));
  QObject::connect((QObject*)(editUserButton),
                   SIGNAL(clicked()),
                   this,
                   SLOT(slotEditUser()));
  QObject::connect((QObject*)(delUserButton),
                   SIGNAL(clicked()),
                   this,
                   SLOT(slotDelUser()));
  QObject::connect((QObject*)(completeUserButton),
                   SIGNAL(clicked()),
                   this,
		   SLOT(slotCompleteUser()));
  QObject::connect((QObject*)(changeVersionButton),
                   SIGNAL(clicked()),
                   this,
                   SLOT(slotChangeVersion()));
  QObject::connect((QObject*)(updateBPDButton),
                   SIGNAL(clicked()),
                   this,
                   SLOT(slotUpdateBPD()));
  QObject::connect((QObject*)(iniLetterButton),
                   SIGNAL(clicked()),
                   this,
                   SLOT(slotIniLetter()));

  // Manually create and add layout here because the .ui-generated
  // QGroupBox doesn't have one.
  userBox->setColumnLayout(0, Qt::Vertical );
  QBoxLayout *userBoxLayout = new QHBoxLayout( userBox->layout() );

  _userList=new UserListView((QWidget*)userBox, "Users");
  userBoxLayout->addWidget(_userList);

  QObject::connect((QObject*)(newAccountButton),
                   SIGNAL(clicked()),
                   this,
                   SLOT(slotNewAccount()));
  QObject::connect((QObject*)(editAccountButton),
                   SIGNAL(clicked()),
                   this,
		   SLOT(slotEditAccount()));
  QObject::connect((QObject*)(delAccountButton),
                   SIGNAL(clicked()),
                   this,
		   SLOT(slotDelAccount()));

  QObject::connect((QObject*)(_userList),
                   SIGNAL(selectionChanged()),
                   this,
		   SLOT(slotUserSelectionChanged()));

  // Set ourself as the new parent of QBanking
  _app->setParentWidget(this);

  updateLists();

  // If user and account list is empty, directly start User->New wizard.
  AH_ACCOUNT_LIST2 *accs;
  AH_USER_LIST2 *users;
  accs=AH_HBCI_GetAccounts(_hbci, 0, "*", "*");
  users=AH_HBCI_GetUsers(_hbci, 0, "*", "*");
  bool all_empty = (!accs) && (!users);
  if (accs)
    AH_Account_List2_free(accs);
  if (users)
    AH_User_List2_free(users);
  if (all_empty) {
    // start the new user wizard right away. we can't do
    // it directly because we're in a constructor here.
    // So let's Qt handle this once the event loop is running.
    // (ipwizard@users.sourceforge.net)
    QTimer::singleShot(0, this, SLOT(slotNewUser()));
  }
  QTimer::singleShot(0, this, SLOT(adjustSize()));
}



HBCISettings::~HBCISettings(){
}



void HBCISettings::updateLists() {
  AH_ACCOUNT_LIST2 *accs;
  AH_USER_LIST2 *users;

  _accList->clear();
  accs=AH_HBCI_GetAccounts(_hbci, 0, "*", "*");
  if (accs) {
    _accList->addAccounts(accs);
    AH_Account_List2_free(accs);
  }

  _userList->clear();
  users=AH_HBCI_GetUsers(_hbci, 0, "*", "*");
  if (users) {
    _userList->addUsers(users);
    AH_User_List2_free(users);
  }
  slotUserSelectionChanged();
}



void HBCISettings::slotUserSelectionChanged(){
  AH_USER_LIST2 *ul;

  ul=_userList->getSelectedUsers();
  if (ul) {
    AH_USER *u;

    u=AH_User_List2_GetFront(ul);
    AH_User_List2_free(ul);
    if (u) {
      completeUserButton->setEnabled(AH_User_GetStatus(u)==
                                     AH_UserStatusPending);
    }
    else {
      completeUserButton->setEnabled(false);
    }
  }
  else {
    completeUserButton->setEnabled(false);
  }
}




void HBCISettings::slotNewUser(){
  Wizard *w;

  w=new Wizard(_hbci, _app, this, "New User Wizard", true);
  //connect(w, SIGNAL(accepted()), this, SLOT(updateLists()));
  //connect(w, SIGNAL(rejected()), this, SLOT(updateLists()));
  w->show(); // necessary for qt4
  w->exec();
  updateLists();
  //w->show();
}



void HBCISettings::slotEditUser(){
  AH_USER_LIST2 *ul;

  ul=_userList->getSelectedUsers();
  if (ul) {
    AH_USER *u;

    u=AH_User_List2_GetFront(ul);
    AH_User_List2_free(ul);
    if (u) {
      EditUser w(_hbci, u, this, "EditUser", true);
      w.show(); // necessary for qt4
      if (w.exec()==QDialog::Accepted)
        updateLists();
    }
  }
}



void HBCISettings::slotIniLetter(){
  AH_USER_LIST2 *ul;

  ul=_userList->getSelectedUsers();
  if (ul) {
    AH_USER *u;

    u=AH_User_List2_GetFront(ul);
    AH_User_List2_free(ul);
    if (u) {
      AH_MEDIUM *m;

      m=AH_User_GetMedium(u);
      if (AH_User_GetCryptMode(u)==AH_CryptMode_Rdh) {
        Wizard w(_hbci, _app, this, "New User Wizard", true);
        w.show(); // necessary for qt4
        w.showIniLetter(u);
      }
      else {
        QMessageBox::critical(this,
                              tr("Error"),
                              tr("<qt>"
                                 "<p>"
                                 "This action is only supported in keyfile "
                                 "mode."
                                 "</p>"
                                 "</qt>"
                                ),
                              QMessageBox::Ok,QMessageBox::NoButton);
        return;
      }
    }
  }
}


void HBCISettings::slotDelUser(){
  AH_USER_LIST2 *ul;

  ul=_userList->getSelectedUsers();
  if (ul) {
    AH_USER *u;

    u=AH_User_List2_GetFront(ul);
    AH_User_List2_free(ul);
    if (u) {
      if (QMessageBox::question(this,
                                tr("Delete User"),
                                "<qt>"+tr("Are you sure you want to delete the selected user?")+"</qt>",
                                QMessageBox::Yes,QMessageBox::No)==0) {
	AH_Bank_RemoveUser(AH_User_GetBank(u), u);
	updateLists();
      }
    }
  }
}



void HBCISettings::slotCompleteUser(){
  AH_USER_LIST2 *ul;

  ul=_userList->getSelectedUsers();
  if (ul) {
    AH_USER *u;

    u=AH_User_List2_GetFront(ul);
    AH_User_List2_free(ul);
    if (u) {
      Wizard *w;

      w=new Wizard(_hbci, _app, this, "New User Wizard", true);
      w->show(); // necessary for qt4
      if (w->completeUser(u))
        updateLists();
      delete w;
    }
  }
}



void HBCISettings::slotNewAccount(){
  AH_BANK *b;
  AH_ACCOUNT *a;
  EditAccount *w;

  b=AH_HBCI_FindBank(_hbci, 0, "*");
  if (!b) {
    QMessageBox::critical(this,
                          tr("No Bank"),
                          tr("<qt>"
                             "<p>"
                             "No bank settings found."
                             "</p>"
                             "<p>"
                             "Please add a user before and come back later"
                             "</p>"
                             "</qt>"
                            ),
                          QMessageBox::Ok,QMessageBox::NoButton);
    return;
  }
  a=AH_Account_new(b, AH_Bank_GetBankId(b), "0");
  w=new EditAccount(_hbci, a, this, "EditAccount", true);
  w->setCaption(tr("New Account"));
  if (!w->init()) {
    DBG_ERROR(0, "Internal error: Could not init EditAccount");
  }
  else {
    if (w->exec()==QDialog::Accepted) {
      AH_Bank_AddAccount(b, a);
      updateLists();
    }
    else {
      AH_Account_free(a);
    }
  }
  delete w;
}



void HBCISettings::slotEditAccount(){
  AH_ACCOUNT_LIST2 *al;

  al=_accList->getSelectedAccounts();
  if (al) {
    AH_ACCOUNT *a;

    a=AH_Account_List2_GetFront(al);
    AH_Account_List2_free(al);
    if (a) {
      EditAccount *w;

      w=new EditAccount(_hbci, a, this, "EditAccount", true);
      w->setCaption(tr("Edit Account"));
      if (!w->init()) {
        DBG_ERROR(0, "Could not init EditAccount");
      }
      else {
        if (w->exec()==QDialog::Accepted)
          updateLists();
      }
      delete w;
    }
  }
}



void HBCISettings::slotDelAccount(){
  AH_ACCOUNT_LIST2 *al;

  al=_accList->getSelectedAccounts();
  if (al) {
    AH_ACCOUNT *a;

    a=AH_Account_List2_GetFront(al);
    AH_Account_List2_free(al);
    if (a) {
      if (QMessageBox::question(this,
                                tr("Delete Account"),
                                tr("<qt>"
                                   "Are you sure you want to delete "
                                   "this account?"
                                   "</qt>"
                                  ),
                                QMessageBox::Yes,QMessageBox::No)!=0)
        return;
      if (AH_Bank_RemoveAccount(AH_Account_GetBank(a), a)) {
        QMessageBox::critical(this,
                              tr("Error"),
                              tr("<qt>"
                                 "<p>"
                                 "Could not delete this account."
                                 "</p>"
                                 "</qt>"
                                ),
                              QMessageBox::Ok,QMessageBox::NoButton);
      }
      else {
        updateLists();
      }
    }
  }
}




void HBCISettings::slotHelp(){
#if QT_VERSION != 0x040000
  // causes linker error in qt-4.0.0, but likely to be fixed in future
  // qt4 versions
  QWhatsThis::enterWhatsThisMode();
#endif
}



void HBCISettings::slotChangeVersion(){
  AH_USER_LIST2 *ul;

  ul=_userList->getSelectedUsers();
  if (ul) {
    AH_USER *u;

    u=AH_User_List2_GetFront(ul);
    AH_User_List2_free(ul);
    if (u) {
      AH_CUSTOMER *cu;
      int v;
      VersionPicker *w;

      cu=AH_User_FindCustomer(u, "*");
      if (!cu) {
	QMessageBox::critical(this,
			      tr("Error"),
			      tr("<qt>"
				 "<p>"
				 "No customer for this user."
				 "</p>"
				 "<p>"
				 "This is an internal error."
				 "</p>"
				 "</qt>"
				),
			      QMessageBox::Ok,QMessageBox::NoButton);
        return;
      }

      v=AH_Customer_GetHbciVersion(cu);
      w=new VersionPicker(v, this, "VersionPicker", TRUE);
      if (w->exec()==QDialog::Accepted) {
	int nv;

	nv=w->getVersion();
	delete w;
	if (nv!=v) {
	  AH_BPD *bpd;
	  const int *vl;

	  // version changed, check whether it is supported
	  bpd=AH_Customer_GetBpd(cu);
	  if (!bpd) {
	    QMessageBox::critical(this,
				  tr("Error"),
				  tr("<qt>"
				     "<p>"
				     "We have no bank info for this user."
				     "</p>"
				     "<p>"
				     "This is an internal error."
				     "</p>"
				     "</qt>"
				    ),
				  QMessageBox::Ok,QMessageBox::NoButton);
	    return;
	  }
          vl=AH_Bpd_GetHbciVersions(bpd);
	  assert(vl);
	  while(*vl) {
	    if (*vl==nv)
              break;
            vl++;
	  } // while
	  if (!*vl) {
	    if (QMessageBox::critical(this,
				      tr("Version Not Supported"),
				      tr("<qt>"
					 "<p>"
					 "The version you selected is not "
                                         "supported by your bank."
					 "</p>"
					 "<p>"
					 "Do you want to try it anyway?"
					 "</p>"
					 "</qt>"
					),
				      QMessageBox::Yes,QMessageBox::No)!=0) {
	      return;
	    }
	  }

	  AH_Customer_SetHbciVersion(cu, nv);
          AH_Customer_SetBpdVersion(cu, 0);
          AH_Customer_SetUpdVersion(cu, 0);
          if (!_updateBPD(cu)) {
            AH_Customer_SetHbciVersion(cu, v);
          }
	} // if version changed
      } // if accepted
      else {
	delete w;
      }
    }
  }
}



bool HBCISettings::_updateBPD(AH_CUSTOMER *cu){
  AH_OUTBOX *ob;
  AH_JOB *j;
  int rv;
  AH_USER *u;

  assert(cu);
  u=AH_Customer_GetUser(cu);
  assert(u);
  j=AH_Job_UpdateBank_new(cu);
  if (!j) {
    DBG_ERROR(0, "Job not supported, should not happen");
    QMessageBox::critical(this,
                          tr("Error"),
                          tr("<qt>"
                             "<p>"
                             "UpdateBankInfo job not available."
                             "</p>"
                             "<p>"
                             "This is an internal error."
                             "</p>"
                             "</qt>"
                            ),
			      QMessageBox::Ok,QMessageBox::NoButton);
    return false;
  }
  AH_Job_AddSigner(j, AH_User_GetUserId(u));
  ob=AH_Outbox_new(_hbci);
  AH_Outbox_AddJob(ob, j);

  rv=AH_Outbox_Execute(ob, 1, 0);
  if (rv) {
    DBG_ERROR(0, "Could not execute outbox (%d)", rv);
    QMessageBox::critical(this,
                          tr("Error"),
                          tr("<qt>"
                             "<p>"
                             "Could not execute outbox."
                             "</p>"
                             "</qt>"
                            ),
                          QMessageBox::Ok,QMessageBox::NoButton);
    AH_Outbox_free(ob);
    AH_Job_free(j);
    return false;
  }
  else {
    if (AH_Job_Commit(j)) {
      DBG_ERROR(0, "Could not commit result.\n");
      QMessageBox::critical(this,
                            tr("Error"),
                            tr("<qt>"
                               "<p>"
                               "Could not commit result."
                               "</p>"
                               "</qt>"
                              ),
                            QMessageBox::Ok,QMessageBox::NoButton);
      AH_Outbox_free(ob);
      AH_Job_free(j);
      return false;
    }
  }

  AH_Outbox_free(ob);
  AH_Job_free(j);
  return true;
}



void HBCISettings::slotUpdateBPD(){
  AH_USER_LIST2 *ul;

  ul=_userList->getSelectedUsers();
  if (ul) {
    AH_USER *u;

    u=AH_User_List2_GetFront(ul);
    AH_User_List2_free(ul);
    if (u) {
      AH_CUSTOMER *cu;

      cu=AH_User_FindCustomer(u, "*");
      if (!cu) {
        QMessageBox::critical(this,
			      tr("Error"),
			      tr("<qt>"
				 "<p>"
				 "No customer for this user."
				 "</p>"
				 "<p>"
				 "This is an internal error."
				 "</p>"
				 "</qt>"
				),
			      QMessageBox::Ok,QMessageBox::NoButton);
	return;
      }
      _updateBPD(cu);
    } // if user
  } // if user selected
}









