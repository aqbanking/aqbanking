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

#include "getkeys.h"
#include "wizard.h"
#include "winfo.h"

#include <qbanking/qbanking.h>

#include <aqhbci/outbox.h>
#include <aqhbci/adminjobs.h>

#include <gwenhywfar/debug.h>

#include <qmessagebox.h>
#include <qstring.h>
#include <qlabel.h>





GetKeys::GetKeys(Wizard *w,
		 QWidget* parent, const char* name, WFlags fl)
:GetKeysUi(parent, name, fl)
,_wizard(w)
,_result(false) {

}



GetKeys::~GetKeys() {
}



bool GetKeys::getResult() const {
  return _result;
}



void GetKeys::slotGetKeys() {
  WizardInfo *wInfo;
  QBanking *qb;
  AH_BANK *b;
  AH_USER *u;
  AH_CUSTOMER *cu;
  AH_MEDIUM *m;
  AH_JOB *job;
  AH_OUTBOX *ob;
  QString failed=QString("<qt><font colour=\"red\">"
			 "%1</font></qt>").arg(tr("Failed"));
  QString success=QString("<qt><font colour=\"green\">"
			  "%1</font></qt>").arg(tr("Success"));
  QString checking=QString("<qt><font colour=\"blue\">"
			   "%1</font></qt>").arg(tr("Checking..."));

  wInfo=_wizard->getWizardInfo();
  assert(wInfo);
  b=wInfo->getBank();
  assert(b);
  u=wInfo->getUser();
  assert(u);
  cu=wInfo->getCustomer();
  assert(cu);
  m=wInfo->getMedium();
  assert(m);

  qb=_wizard->getBanking();
  assert(qb);

  getKeysLabel->setText(checking);

  job=AH_Job_GetKeys_new(cu);
  if (!job) {
    DBG_ERROR(0, "Job not supported, should not happen");
    qb->progressLog(0, AB_Banking_LogLevelError,
		    tr("Job not supported, should not happen."));
    _result=false;
    getKeysLabel->setText(failed);
    return;
  }

  ob=AH_Outbox_new(wInfo->getHbci());
  AH_Outbox_AddJob(ob, job);

  if (AH_Outbox_Execute(ob, 1, 1)) {
    DBG_ERROR(0, "Could not execute outbox.\n");
    qb->progressLog(0, AB_Banking_LogLevelError,
		    tr("Could not execute outbox."));
    qb->progressEnd(0);
    _result=false;
    getKeysLabel->setText(failed);
    return;
  }

  if (AH_Job_GetKeys_GetCryptKey(job)==0) {
    qb->progressLog(0, AB_Banking_LogLevelError,
		    tr("No crypt key received."));
    QMessageBox::critical(this,
			  tr("No Crypt Key"),
			  tr("<qt>"
			     "<p>"
			     "The server did not send a crypt key."
			     "</p>"
			     "</qt>"
			    ),
			  QMessageBox::Ok,QMessageBox::NoButton);
    _result=false;
    getKeysLabel->setText(failed);
    return;
  }

  if (AH_Job_HasErrors(job)) {
    DBG_ERROR(0, "Job has errors");
    qb->progressLog(0, AB_Banking_LogLevelError,
		    tr("Jobs contains errors."));
    qb->progressEnd(0);
    AH_Outbox_free(ob);
    _result=false;
    getKeysLabel->setText(failed);
    return;
  }
  else {
    if (AH_Job_Commit(job)) {
      DBG_ERROR(0, "Could not commit result.\n");
      qb->progressLog(0, AB_Banking_LogLevelError,
		      tr("Could not commit result."));
      qb->progressEnd(0);
      AH_Outbox_free(ob);
      _result=false;
      getKeysLabel->setText(failed);
      return;
    }
  }

  if (AH_Job_GetKeys_GetSignKey(job)==0) {
    qb->progressLog(0, AB_Banking_LogLevelNotice,
		    tr("No sign key received."));
    QMessageBox::information(this,
			     tr("No Sign Key"),
			     tr("<qt>"
				"<p>"
				"The server did not send a crypt key."
				"</p>"
				"<p>"
				"Some banks do not sign their messages. "
				"While this is a quite unfair behaviour "
				"it is allowed by the HBCI specs."
				"</p>"
				"<p>"
				"We call this unfair because the bank asks "
				"you to sign your messages (which makes sure "
				"that you are the originator of a message) "
				"while not granting this assurance to you. "
				"</p>"
				"<p>"
				"So without the bank signing its messages we "
				"can never be sure that a message really "
                                "was sent by the bank."
				"</p>"
				"</qt>"
			       ),
			     QMessageBox::Ok,QMessageBox::NoButton);
  }

  qb->progressLog(0, AB_Banking_LogLevelNotice, tr("Keys saved."));
  qb->progressEnd(0);

  AH_Outbox_free(ob);

  _result=true;
  getKeysLabel->setText(success);
}



