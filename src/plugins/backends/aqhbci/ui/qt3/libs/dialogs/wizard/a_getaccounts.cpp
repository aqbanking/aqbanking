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


#include "a_getaccounts.h"
#include "wizard.h"
#include "actionwidget.h"

#include <qpushbutton.h>
#include <qmessagebox.h>

#include <qbanking/qbanking.h>
#include <aqhbci/provider.h>
#include <gwenhywfar/debug.h>

#include <assert.h>



ActionGetAccounts::ActionGetAccounts(Wizard *w)
:WizardAction(w, "GetAccounts", QWidget::tr("Retrieve Account List")) {
  _realDialog=new ActionWidget
    (tr("<qt>"
        "We will now retrieve the list of our accounts from the bank server."
        "</qt>"),
     tr("<qt>"
        "<p>"
        "Some banks do not send a list of accounts we are allowed to manage."
        " In such a case you will have to setup the accounts manually."
        "</p>"
        "<p>"
        "You may skip this step here if you already know that your bank is "
        "one of those candidates."
        "</p>"
        "</qt>"),
     tr("Get Account List"),
     this, "GetAccounts");
  _realDialog->setStatus(ActionWidget::StatusNone);
  connect(_realDialog->getButton(), SIGNAL(clicked()),
          this, SLOT(slotButtonClicked()));

  addWidget(_realDialog);
  _realDialog->show();
}



ActionGetAccounts::~ActionGetAccounts() {
}



bool ActionGetAccounts::apply() {
  return true;
}



void ActionGetAccounts::enter() {
  _realDialog->setStatus(ActionWidget::StatusNone);
}



void ActionGetAccounts::slotButtonClicked() {
  WizardInfo *wInfo;
  QBanking *qb;
  AB_USER *u;
  AB_PROVIDER *pro;
  GWEN_TYPE_UINT32 pid;
  int rv;
  AB_IMEXPORTER_CONTEXT *ctx = AB_ImExporterContext_new();

  wInfo=getWizard()->getWizardInfo();
  assert(wInfo);
  u=wInfo->getUser();
  assert(u);
  qb=getWizard()->getBanking();
  assert(qb);
  pro=AH_HBCI_GetProvider(wInfo->getHbci());
  assert(pro);

  _realDialog->setStatus(ActionWidget::StatusChecking);

  DBG_INFO(0, "Retrieving accounts");
  pid=qb->progressStart(tr("Getting List of Accounts"),
                        tr("<qt>"
                           "Retrieving the list of our accounts from the "
                           "bank server."
                           "</qt>"),
                        1);
  rv=AH_Provider_GetAccounts(pro, u, ctx, 1);
  AB_ImExporterContext_free(ctx);
  if (rv) {
    if (rv==AB_ERROR_NO_DATA) {
      QMessageBox::information(this,
                               tr("No Account List"),
                               tr("<qt>"
                                  "<p>"
                                  "Your bank does not send a list of "
                                  "accounts."
                                  "</p>"
                                  "<p>"
                                  "You will have to setup the accounts for "
                                  "this user manually."
                                  "</p>"
                                  "</qt>"),
                               QMessageBox::Ok,QMessageBox::NoButton);
    }
    else {
      DBG_ERROR(0, "Error getting accounts");
      _realDialog->setStatus(ActionWidget::StatusFailed);
      qb->progressEnd(pid);
      return;
    }
  }

  qb->progressEnd(pid);

  _realDialog->setStatus(ActionWidget::StatusSuccess);
}





