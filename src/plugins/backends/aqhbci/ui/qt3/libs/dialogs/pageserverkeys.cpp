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




bool Wizard::initServerKeysPage() {
  QObject::connect((getKeysButton),
                   SIGNAL(clicked()),
                   this,
                   SLOT(slotGetKeys()));
  getKeysButton->setEnabled(true);
  return true;
}



void Wizard::slotGetKeys(){
  AH_JOB *job;
  AH_OUTBOX *ob;

  if (!_customer) {
    DBG_ERROR(0, "No customer");
    return;
  }

  job=AH_Job_GetKeys_new(_customer);
  if (!job) {
    DBG_ERROR(0, "Job not supported, should not happen");
    getKeysLabel->setText(_ResultMsg_Failed);
    return;
  }

  ob=AH_Outbox_new(_hbci);
  AH_Outbox_AddJob(ob, job);

  if (AH_Outbox_Execute(ob, 1, 1)) {
    DBG_ERROR(0, "Could not execute outbox.\n");
    AH_HBCI_UnmountCurrentMedium(_hbci);
    AB_Banking_ProgressEnd(_app->getCInterface(), 0);
    AH_Outbox_free(ob);
    getKeysLabel->setText(_ResultMsg_Failed);
    return;
  }

  if (AH_Job_GetStatus(job)!=AH_JobStatusAnswered) {
    DBG_ERROR(0, "Job has errors");
    // TODO: show errors
    AH_Outbox_free(ob);
    getKeysLabel->setText(_ResultMsg_Failed);
    AH_HBCI_UnmountCurrentMedium(_hbci);
    AB_Banking_ProgressEnd(_app->getCInterface(), 0);
    return;
  }
  else {
    AB_Banking_ProgressLog(_app->getCInterface(),
                           0,
                           AB_Banking_LogLevelNotice,
                           tr("Saving keys, please wait").utf8());
    if (AH_Job_Commit(job)) {
      DBG_ERROR(0, "Could not commit result.\n");
      AH_HBCI_UnmountCurrentMedium(_hbci);
      AB_Banking_ProgressEnd(_app->getCInterface(), 0);
      AH_Outbox_free(ob);
      getKeysLabel->setText(_ResultMsg_Failed);
      return;
    }
  }
  AB_Banking_ProgressLog(_app->getCInterface(),
                         0,
                         AB_Banking_LogLevelNotice,
                         tr("Keys saved.").utf8());
  AB_Banking_ProgressEnd(_app->getCInterface(), 0);

  AH_Outbox_free(ob);
  getKeysLabel->setText(_ResultMsg_Success);
  getKeysButton->setEnabled(false);
  setNextEnabled(serverKeysPage, true);
}



bool Wizard::doServerKeysPage(QWidget *p){
  return true;
}



bool Wizard::undoServerKeysPage(QWidget *p){
  return true;
}














