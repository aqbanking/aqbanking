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
#include <qbanking/qbanking.h>

#include <aqhbci/outbox.h>
#include <aqhbci/adminjobs.h>

#include <gwenhywfar/debug.h>

#include <qmessagebox.h>
#include <qlabel.h>

#include <assert.h>



ActionGetAccounts::ActionGetAccounts(Wizard *w)
:WizardAction(w, "GetAccounts", QWidget::tr("Retrieve list of accounts")) {
  QLabel *tl;

  tl=new QLabel(this, "GetAccountsText");
  tl->setText("<tr>"
              "When you click <i>next</i> below we will attempt to "
              "retrieve the list of accounts from the server."
              "</tr>");
  addWidget(tl);
}



ActionGetAccounts::~ActionGetAccounts() {
}



bool ActionGetAccounts::apply() {
  WizardInfo *wInfo;
  QBanking *qb;
  AH_BANK *b;
  AH_USER *u;
  AH_CUSTOMER *cu;
  AH_MEDIUM *m;
  AH_JOB *job;
  AH_OUTBOX *ob;
  AH_ACCOUNT_LIST2 *accs;

  wInfo=getWizard()->getWizardInfo();
  assert(wInfo);
  b=wInfo->getBank();
  assert(b);
  u=wInfo->getUser();
  assert(u);
  cu=wInfo->getCustomer();
  assert(cu);
  m=wInfo->getMedium();
  assert(m);

  qb=getWizard()->getBanking();
  assert(qb);


  job=AH_Job_UpdateBank_new(cu);
  if (!job) {
    DBG_ERROR(0, "Job not supported, should not happen");
    return false;
  }
  AH_Job_AddSigner(job, AH_User_GetUserId(u));

  ob=AH_Outbox_new(wInfo->getHbci());
  AH_Outbox_AddJob(ob, job);

  if (AH_Outbox_Execute(ob, 1, 1)) {
    DBG_ERROR(0, "Could not execute outbox.\n");
    AB_Banking_ProgressEnd(qb->getCInterface(), 0);
    return false;
  }

  if (AH_Job_HasErrors(job)) {
    DBG_ERROR(0, "Job has errors");
    // TODO: show errors
    AB_Banking_ProgressEnd(qb->getCInterface(), 0);
    AH_Outbox_free(ob);
    return false;
  }
  else {
    if (AH_Job_Commit(job)) {
      DBG_ERROR(0, "Could not commit result.\n");
      AB_Banking_ProgressEnd(qb->getCInterface(), 0);
      AH_Outbox_free(ob);
      return false;
    }
  }

  /* check whether we got some accounts */
  accs=AH_Job_UpdateBank_GetAccountList(job);
  assert(accs);
  if (AH_Account_List2_GetSize(accs)==0) {
    DBG_INFO(0, "No account list received");
    QMessageBox::information(this,
                             tr("No Account List"),
                             tr("<qt>"
                                "<p>"
                                "The server did not send an account list."
                                "</p>"
                                "<p>"
                                "As stated before this is quite normal "
                                "(although annoying)."
                                "</p>"
                                "<p>"
                                "Therefore you have to add your accounts "
                                "manually. Please press \"Next\" to go to the next page."
                                "</p>"
                                "</qt>"
                               ),
                             QMessageBox::Ok,QMessageBox::NoButton);
  }

  AB_Banking_ProgressEnd(qb->getCInterface(), 0);
  AH_Outbox_free(ob);

  return true;
}






