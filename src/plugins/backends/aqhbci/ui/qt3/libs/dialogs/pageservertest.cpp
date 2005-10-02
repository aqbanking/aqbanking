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
#include "versionpicker.h"
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




bool Wizard::initServerTestPage() {
  QObject::connect((serverTestButton),
                   SIGNAL(clicked()),
                   this,
		   SLOT(slotServerTest()));
  serverTestButton->setEnabled(true);
  setNextEnabled(serverTestPage, true);
  return true;
}



void Wizard::slotServerTest(){
  AH_JOB_TESTVERSION_RESULT result;
  static int versions[3]={201,210,220};
  int i;
  AH_JOB *job;
  AH_OUTBOX *ob;

  if (!_customer) {
    DBG_ERROR(0, "No customer");
    return;
  }

  i=versionCombo->currentItem();
  if (i<0 || i>=3) {
    DBG_ERROR(0, "Internal error: bad entry in version combo.");
    return;
  }

  DBG_INFO(0, "Trying HBCI version %d", versions[i]);
  AH_Customer_SetHbciVersion(_customer, versions[i]);

  serverTestLabel->setText(tr("<qt>"
			      "Checking..."
			      "</qt>"));

  job=AH_Job_TestVersion_new(_customer, 1 /*_firstInitMode*/);
  if (!job) {
    DBG_ERROR(0, "Job not supported, should not happen");
    serverTestLabel->setText(_ResultMsg_Failed);
    return;
  }
  if (0 /*!_firstInitMode*/)
    AH_Job_AddSigner(job, AH_User_GetUserId(_user));

  ob=AH_Outbox_new(_hbci);
  AH_Outbox_AddJob(ob, job);

  if (AH_Outbox_Execute(ob, 1, 1)) {
    DBG_ERROR(0, "Could not execute outbox.\n");
    AH_Job_free(job);
    AH_HBCI_UnmountCurrentMedium(_hbci);
    AB_Banking_ProgressEnd(_app->getCInterface(), 0);
    AH_Outbox_free(ob);
    return;
  }
  else {
    result=AH_Job_TestVersion_GetResult(job);
  }
  AH_Job_free(job);
  AH_Outbox_free(ob);
  AB_Banking_ProgressEnd(_app->getCInterface(), 0);

  if (result==AH_JobTestVersion_ResultSupported) {
    DBG_INFO(0, "HBCI version %d supported, selecting", versions[i]);
    AH_Customer_SetHbciVersion(_customer, versions[i]);
    serverTestLabel->setText(_ResultMsg_Supported);
    setNextEnabled(serverTestPage, true);
  }
  else if (result==AH_JobTestVersion_ResultMaybeSupported) {
    DBG_INFO(0, "HBCI version %d maybe supported, selecting", versions[i]);
    AH_Customer_SetHbciVersion(_customer, versions[i]);
    serverTestLabel->setText(QString("<qt><font color=\"green\">")+
			     tr("Maybe supported")+"</font></qt>");
    setNextEnabled(serverTestPage, true);
  }
  else {
    DBG_INFO(0, "HBCI version %d not supported", versions[i]);
    serverTestLabel->setText(_ResultMsg_NotSupported);
  }
}



bool Wizard::doServerTestPage(QWidget *p){
  static int versions[3]={201,210,220};
  int i;

  i=versionCombo->currentItem();
  if (i<0 || i>=3) {
    DBG_ERROR(0, "Internal error: bad entry in version combo.");
    return false;
  }

  DBG_INFO(0, "Setting HBCI version to %d", versions[i]);
  AH_Customer_SetHbciVersion(_customer, versions[i]);

  return true;
}



bool Wizard::undoServerTestPage(QWidget *p){
  return true;
}













