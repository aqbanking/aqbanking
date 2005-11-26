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


#include "a_getsysid.h"
#include "wizard.h"
#include <qbanking/qbanking.h>

#include <aqhbci/outbox.h>
#include <aqhbci/adminjobs.h>

#include <gwenhywfar/debug.h>

#include <qmessagebox.h>
#include <qlabel.h>

#include <assert.h>



ActionGetSysId::ActionGetSysId(Wizard *w)
:WizardAction(w, "GetSysId", QWidget::tr("Retrieve System Id")) {
  QLabel *tl;

  tl=new QLabel(this, "GetSysIdText");
  tl->setText("<tr>"
              "When you click <i>next</i> below we will attempt to "
              "retrieve a system id from the server"
              "</tr>");
  addWidget(tl);
}



ActionGetSysId::~ActionGetSysId() {
}



bool ActionGetSysId::apply() {
  WizardInfo *wInfo;
  QBanking *qb;
  AH_BANK *b;
  AH_USER *u;
  AH_CUSTOMER *cu;
  AH_MEDIUM *m;
  AH_JOB *job;
  AH_OUTBOX *ob;
  const char *s;

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

  job=AH_Job_GetSysId_new(cu);
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

  s=AH_Job_GetSysId_GetSysId(job);
  if (!s) {
    AB_Banking_ProgressEnd(qb->getCInterface(), 0);
    AH_Outbox_free(ob);
    DBG_ERROR(0, "No system Id");
    QMessageBox::critical(getWizard()->getWidgetAsParent(),
			  QWidget::tr("Job Error"),
			  QWidget::tr("An empty system id "
				      "has been received."),
			  QMessageBox::Ok,QMessageBox::NoButton);
    return false;
  }

  if (AH_Medium_SelectContext(m, AH_User_GetContextIdx(u))) {
    DBG_ERROR(0, "Could not select user");
    QMessageBox::critical(getWizard()->getWidgetAsParent(),
			  QWidget::tr("Medium Error"),
			  QWidget::tr("Could not select user context "
				      "on medium.\n"
				      "Please check the logs."
				     ),
                          QMessageBox::Ok,QMessageBox::NoButton);
    AB_Banking_ProgressEnd(qb->getCInterface(), 0);
    AH_Outbox_free(ob);
    return false;
  }

  AH_Customer_SetSystemId(cu, s);

  AB_Banking_ProgressEnd(qb->getCInterface(), 0);
  AH_Outbox_free(ob);

  return true;
}






