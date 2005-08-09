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




bool Wizard::initAccListPage() {
  QObject::connect((QObject*)(getAccListButton),
                   SIGNAL(clicked()),
                   this,
		   SLOT(slotGetAccList()));
  getAccListButton->setEnabled(true);
  setNextEnabled(accListPage, false);
  return true;
}



void Wizard::slotGetAccList(){
  AH_JOB *job;
  AH_OUTBOX *ob;
  AH_ACCOUNT_LIST2 *accs;

  if (!_customer) {
    DBG_ERROR(0, "No customer");
    return;
  }

  job=AH_Job_UpdateBank_new(_customer);
  if (!job) {
    DBG_ERROR(0, "Job not supported, should not happen");
    getAccListLabel->setText(_ResultMsg_Failed);
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
    getAccListLabel->setText(_ResultMsg_Failed);
    return;
  }

  if (AH_Job_HasErrors(job) || AH_Job_GetStatus(job)!=AH_JobStatusAnswered) {
    DBG_ERROR(0, "Job has errors (%s)",
              AH_Job_StatusName(AH_Job_GetStatus(job)));
    // TODO: show errors
    AH_HBCI_UnmountCurrentMedium(_hbci);
    AB_Banking_ProgressEnd(_app->getCInterface(), 0);
    AH_Outbox_free(ob);
    getAccListLabel->setText(_ResultMsg_Failed);
    return;
  }
  else {
    if (AH_Job_Commit(job)) {
      DBG_ERROR(0, "Could not commit result.\n");
      AH_HBCI_UnmountCurrentMedium(_hbci);
      AB_Banking_ProgressEnd(_app->getCInterface(), 0);
      AH_Outbox_free(ob);
      getAccListLabel->setText(_ResultMsg_Failed);
      return;
    }
  }

  AH_Outbox_free(ob);

  /* check whether we got some accounts */
  accs=AH_Job_UpdateBank_GetAccountList(job);
  assert(accs);
  if (AH_Account_List2_GetSize(accs)==0) {
    DBG_INFO(0, "No account list received");
    setAppropriate(addAccPage, true);
    getAccListLabel->setText(QString("<qt><font color=\"green\">")
			     + tr("Success, but no list")
			     + QString("</font></qt>"));
    QMessageBox::information(0,
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
                             tr("Dismiss"),0,0,0);
  }
  else {
    getAccListLabel->setText(_ResultMsg_Success);
    setAppropriate(addAccPage, false);
  }
  AB_Banking_ProgressEnd(_app->getCInterface(), 0);
  getAccListButton->setEnabled(false);
  setBackEnabled(accListPage, false);
  setNextEnabled(accListPage, true);
}



bool Wizard::doAccListPage(QWidget *p){
  return true;
}



bool Wizard::undoAccListPage(QWidget *p){
  return true;
}













