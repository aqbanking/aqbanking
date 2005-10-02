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




bool Wizard::initSendKeysPage() {
  QObject::connect((sendKeysButton),
                   SIGNAL(clicked()),
                   this,
                   SLOT(slotSendKeys()));
  sendKeysButton->setEnabled(true);
  setBackEnabled(sendKeysPage, false);
  setNextEnabled(sendKeysPage, false);
  return true;
}



void Wizard::slotSendKeys(){
  AH_JOB *job;
  AH_OUTBOX *ob;
  GWEN_CRYPTKEY *signKey;
  GWEN_CRYPTKEY *cryptKey;

  DBG_NOTICE(0, "About to send keys");
  if (!_customer) {
    DBG_ERROR(0, "No customer");
    return;
  }

  if (!AH_Medium_IsMounted(_medium)) {
    if (AH_Medium_Mount(_medium)) {
      DBG_ERROR(0, "Could not mount medium");
      createKeysLabel->setText(_ResultMsg_Failed);
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
    createKeysLabel->setText(_ResultMsg_Failed);
    return;
  }

  signKey=AH_Medium_GetLocalPubSignKey(_medium);
  cryptKey=AH_Medium_GetLocalPubCryptKey(_medium);
  if (!signKey || !cryptKey) {
    DBG_ERROR(0, "Either sign- or crypt key missing");
    GWEN_CryptKey_free(signKey);
    GWEN_CryptKey_free(cryptKey);
    createKeysLabel->setText(_ResultMsg_Failed);
    return;
  }

  job=AH_Job_SendKeys_new(_customer, cryptKey, signKey);
  if (!job) {
    DBG_ERROR(0, "Job not supported, should not happen");
    sendKeysLabel->setText(_ResultMsg_Failed);
    GWEN_CryptKey_free(signKey);
    GWEN_CryptKey_free(cryptKey);
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
    sendKeysLabel->setText(_ResultMsg_Failed);
    GWEN_CryptKey_free(signKey);
    GWEN_CryptKey_free(cryptKey);
    return;
  }

  if (AH_Job_HasErrors(job)) {
    DBG_ERROR(0, "Job has errors");
    // TODO: show errors
    sendKeysLabel->setText(_ResultMsg_Failed);
    GWEN_CryptKey_free(signKey);
    GWEN_CryptKey_free(cryptKey);
    AH_HBCI_UnmountCurrentMedium(_hbci);
    AB_Banking_ProgressEnd(_app->getCInterface(), 0);
    AH_Outbox_free(ob);
  }
  else {
    if (AH_Job_GetStatus(job)!=AH_JobStatusAnswered) {
      DBG_ERROR(0, "No answer for this job");
      sendKeysLabel->setText(_ResultMsg_Failed);
    }
    else {
      if (AH_Job_Commit(job)) {
        DBG_ERROR(0, "Could not commit result.\n");
        AH_HBCI_UnmountCurrentMedium(_hbci);
        AB_Banking_ProgressEnd(_app->getCInterface(), 0);
        AH_Outbox_free(ob);
        sendKeysLabel->setText(_ResultMsg_Failed);
        GWEN_CryptKey_free(signKey);
        GWEN_CryptKey_free(cryptKey);
        return;
      }
      sendKeysLabel->setText(_ResultMsg_Success);
      sendKeysButton->setEnabled(false);
      setNextEnabled(sendKeysPage, true);
    }
    GWEN_CryptKey_free(signKey);
    GWEN_CryptKey_free(cryptKey);
    AB_Banking_ProgressEnd(_app->getCInterface(), 0);
    AH_Outbox_free(ob);
  }
}



bool Wizard::doSendKeysPage(QWidget *p){
  return true;
}



bool Wizard::undoSendKeysPage(QWidget *p){
  return false;
}














