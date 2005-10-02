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


#include "wizard.h"
#include <qbanking/qbanking.h>
#include <aqhbci/outbox.h>
#include <aqhbci/adminjobs.h>

#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qwizard.h>
#include <qcombobox.h>
#include <qtextbrowser.h>

#include <qlineedit.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qpalette.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qprinter.h>
#include <qsimplerichtext.h>
#include <qtextview.h>
#include <qlabel.h>

#include <gwenhywfar/debug.h>




bool Wizard::initSystemIdPage() {
  QObject::connect((getSysIdButton),
                   SIGNAL(clicked()),
                   this,
		   SLOT(slotGetSysId()));
  setNextEnabled(systemIdPage, false);
  return true;
}



void Wizard::slotGetSysId(){
  AH_JOB *job;
  AH_OUTBOX *ob;
  const char *s;

  if (!_customer) {
    DBG_ERROR(0, "No customer");
    return;
  }

  job=AH_Job_GetSysId_new(_customer);
  if (!job) {
    DBG_ERROR(0, "Job not supported, should not happen");
    getSysIdLabel->setText(_ResultMsg_Failed);
    return;
  }
  AH_Job_AddSigner(job, AH_User_GetUserId(_user));

  ob=AH_Outbox_new(_hbci);
  AH_Outbox_AddJob(ob, job);

  if (AH_Outbox_Execute(ob, 1, 1)) {
    DBG_ERROR(0, "Could not execute outbox.\n");
    AH_HBCI_UnmountCurrentMedium(_hbci);
    AB_Banking_ProgressEnd(_app->getCInterface(), 0);
    AH_Outbox_free(ob);
    getSysIdLabel->setText(_ResultMsg_Failed);
    return;
  }

  if (AH_Job_HasErrors(job)) {
    DBG_ERROR(0, "Job has errors");
    // TODO: show errors
    getSysIdLabel->setText(_ResultMsg_Failed);
  }
  else {
    if (AH_Job_Commit(job)) {
      DBG_ERROR(0, "Could not commit result.\n");
      AH_HBCI_UnmountCurrentMedium(_hbci);
      AB_Banking_ProgressEnd(_app->getCInterface(), 0);
      AH_Outbox_free(ob);
      getSysIdLabel->setText(_ResultMsg_Failed);
      return;
    }
  }

  s=AH_Job_GetSysId_GetSysId(job);
  if (!s) {
    AH_HBCI_UnmountCurrentMedium(_hbci);
    AB_Banking_ProgressEnd(_app->getCInterface(), 0);
    AH_Outbox_free(ob);
    DBG_ERROR(0, "No system Id");
    getSysIdLabel->setText(_ResultMsg_Failed);
    return;
  }

  /* store system id */
  if (!AH_Medium_IsMounted(_medium)) {
    if (AH_Medium_Mount(_medium)) {
      DBG_ERROR(0, "Could not mount medium");
      AH_HBCI_UnmountCurrentMedium(_hbci);
      AB_Banking_ProgressEnd(_app->getCInterface(), 0);
      AH_Outbox_free(ob);
      getSysIdLabel->setText(_ResultMsg_Failed);
      return;
    }
  }

  if (AH_Medium_SelectContext(_medium, AH_User_GetContextIdx(_user))) {
    DBG_ERROR(0, "Could not select user");
    QMessageBox::critical(this,
			  tr("Medium Error"),
			  tr("Could not select user context on medium.\n"
			     "Please check the logs."
			    ),
			  QMessageBox::Ok,QMessageBox::NoButton);
    AH_HBCI_UnmountCurrentMedium(_hbci);
    AB_Banking_ProgressEnd(_app->getCInterface(), 0);
    AH_Outbox_free(ob);
    getSysIdLabel->setText(_ResultMsg_Failed);
    return;
  }

  AH_Customer_SetSystemId(_customer, s);

  AB_Banking_ProgressEnd(_app->getCInterface(), 0);
  AH_Outbox_free(ob);
  getSysIdLabel->setText(_ResultMsg_Success);
  getSysIdButton->setEnabled(false);
  setBackEnabled(systemIdPage, false);
  setNextEnabled(systemIdPage, true);
}



bool Wizard::doSystemIdPage(QWidget *p){
  return true;
}



bool Wizard::undoSystemIdPage(QWidget *p){
  return true;
}













