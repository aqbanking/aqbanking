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


#include "qbcfgtabpageaccounts.h"
#include "qbcfgtabpageaccounts.ui.h"
#include "qbeditaccount.h"
#include "qbaccountlist.h"

#include <qbanking/qbanking.h>

#include <qlineedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qlayout.h>

#include <gwenhywfar/debug.h>



QBCfgTabPageAccounts::QBCfgTabPageAccounts(QBanking *qb,
                                           QWidget *parent,
                                           const char *name,
                                           WFlags f)
:QBCfgTabPage(qb, tr("Accounts"), parent, name, f){
  _realPage=new QBCfgTabPageAccountsUi(this);
  addWidget(_realPage);
  _realPage->show();

  setHelpSubject("QBCfgTabPageAccounts");
  setDescription(tr("This page allows you to map, create, edit and remove"
                    " accounts from AqBanking."));

  QObject::connect(_realPage->accountMapButton, SIGNAL(clicked()),
                   this, SLOT(slotAccountMap()));

  QObject::connect(_realPage->accountNewButton, SIGNAL(clicked()),
                   this, SLOT(slotAccountNew()));
  QObject::connect(_realPage->accountEditButton, SIGNAL(clicked()),
                   this, SLOT(slotAccountEdit()));
  QObject::connect(_realPage->accountDeleteButton, SIGNAL(clicked()),
                   this, SLOT(slotAccountDel()));
}



QBCfgTabPageAccounts::~QBCfgTabPageAccounts() {
}



void QBCfgTabPageAccounts::_accountRescan(){
  _realPage->accountList->clear();
  _realPage->accountList->addAccounts(getBanking()->getAccounts());
}



bool QBCfgTabPageAccounts::toGui() {
  GWEN_DB_NODE *dbSettings;
  int i, j;

  dbSettings=getBanking()->getSharedData("qbanking");
  assert(dbSettings);
  dbSettings=GWEN_DB_GetGroup(dbSettings, GWEN_DB_FLAGS_DEFAULT,
                              "settings");
  assert(dbSettings);

  /* setup account list view */
  _realPage->accountList->setResizeMode(QListView::NoColumn);
  for (i=0; i<_realPage->accountList->columns(); i++) {
    _realPage->accountList->setColumnWidthMode(i, QListView::Manual);
    j=GWEN_DB_GetIntValue(dbSettings, "gui/accountList/columns", i, -1);
    if (j!=-1)
      _realPage->accountList->setColumnWidth(i, j);
  } /* for */
  _realPage->accountList->setSelectionMode(QListView::Single);

  _accountRescan();
  return true;
}



bool QBCfgTabPageAccounts::fromGui() {
  GWEN_DB_NODE *dbSettings;
  int i, j;

  dbSettings=getBanking()->getSharedData("qbanking");
  assert(dbSettings);
  dbSettings=GWEN_DB_GetGroup(dbSettings, GWEN_DB_FLAGS_DEFAULT,
                              "settings");
  assert(dbSettings);

  /* save account list view settings */
  GWEN_DB_DeleteVar(dbSettings, "gui/accountList/columns");
  for (i=0; i<_realPage->accountList->columns(); i++) {
    j=_realPage->accountList->columnWidth(i);
    GWEN_DB_SetIntValue(dbSettings, GWEN_DB_FLAGS_DEFAULT,
                        "gui/accountList/columns", j);
  } /* for */

  return true;
}



void QBCfgTabPageAccounts::slotAccountMap(){
  std::list<AB_ACCOUNT*> al;
  AB_ACCOUNT *a;

  al=_realPage->accountList->getSelectedAccounts();
  if (al.empty()) {
    QMessageBox::critical(this,
                          tr("Selection Error"),
                          tr("No account selected.\n"),
                          QMessageBox::Retry,QMessageBox::NoButton);
  }
  a=al.front();
  getBanking()->mapAccount(a);
}



void QBCfgTabPageAccounts::slotAccountNew() {
}



void QBCfgTabPageAccounts::slotAccountEdit() {
  std::list<AB_ACCOUNT*> al;
  AB_ACCOUNT *a;

  al=_realPage->accountList->getSelectedAccounts();
  if (al.empty()) {
    QMessageBox::critical(this,
                          tr("Selection Error"),
                          tr("No user selected.\n"),
                          QMessageBox::Retry,QMessageBox::NoButton);
  }
  a=al.front();
  if (QBEditAccount::editAccount(getBanking(), a, this)) {
    DBG_INFO(0, "Accepted");
  }
  else {
    DBG_INFO(0, "Rejected");
  }
}



void QBCfgTabPageAccounts::slotAccountDel() {
}



void QBCfgTabPageAccounts::updateView() {
  _accountRescan();
}


