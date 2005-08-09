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


#include "editcustomer.h"
#include <qbanking/qbanking.h>

#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qlistview.h>
#include <qcheckbox.h>
#include <qtimer.h>
#include <qgroupbox.h>

#include <aqhbci/outbox.h>
#include <aqhbci/adminjobs.h>
#include <aqbanking/banking.h>

#include <gwenhywfar/debug.h>

#ifdef WIN32
# define strcasecmp stricmp
#endif



EditCustomer::EditCustomer(AH_HBCI *h,
                           AH_CUSTOMER *c,
                           QWidget* parent, const char* name,
                           bool modal, WFlags fl)
:EditCustomerUi(parent, name, modal, fl)
,_hbci(h)
,_customer(c)
,_withHttp(true) {
  const char *s;
  QString qs;
  AH_MEDIUM *m;
  AH_USER *u;

  u=AH_Customer_GetUser(c);
  assert(u);
  s=AH_Customer_GetFullName(c);
  if (s)
    fullNameEdit->setText(QString::fromUtf8(s));

  preferSingleCheck->setChecked(AH_Customer_GetPreferSingleTransfer(c));

  m=AH_User_GetMedium(u);
  assert(m);

  getServerKeysButton->setEnabled(false);
  getSysIdButton->setEnabled(false);

  if (AH_User_GetCryptMode(u)==AH_CryptMode_Pintan) {
    _withHttp=true;
    httpVersionCombo->insertItem(tr("1.0"));
    httpVersionCombo->insertItem(tr("1.1"));
    qs=QString::number(AH_Customer_GetHttpVMajor(c));
    qs+=".";
    qs+=QString::number(AH_Customer_GetHttpVMinor(c));
    _setComboTextIfPossible(httpVersionCombo, qs);

    s=AH_Customer_GetHttpUserAgent(c);
    if (s)
      userAgentEdit->setText(QString::fromUtf8(s));

    s=AH_Customer_GetHttpHost(c);
    if (s)
      hostEdit->setText(QString::fromUtf8(s));

    getSysIdButton->setEnabled(true);
  }
  else {
    _withHttp=false;
    httpBox->hide();

    if (AH_User_GetCryptMode(u)==AH_CryptMode_Rdh) {
      getServerKeysButton->setEnabled(true);
      getSysIdButton->setEnabled(true);
    }
  }

  bankSignCheck->setChecked(AH_Customer_GetBankSigns(c));
  bankCounterCheck->setChecked(AH_Customer_GetBankUsesSignSeq(c));

  QObject::connect((QObject*)(getServerKeysButton),
                   SIGNAL(clicked()),
                   this,
                   SLOT(slotGetServerKeys()));
  QObject::connect((QObject*)(getSysIdButton),
		   SIGNAL(clicked()),
                   this,
                   SLOT(slotGetSysId()));

  QTimer::singleShot(0, this, SLOT(adjustSize()));
}



EditCustomer::~EditCustomer() {
}



bool EditCustomer::init() {
  return true;
}



bool EditCustomer::fini() {
  return true;
}




void EditCustomer::accept() {
  std::string s;
  QString qs;

  s=QBanking::QStringToUtf8String(fullNameEdit->text());
  if (s.empty())
    AH_Customer_SetFullName(_customer, 0);
  else
    AH_Customer_SetFullName(_customer, s.c_str());

  AH_Customer_SetPreferSingleTransfer(_customer,
				      preferSingleCheck->isChecked());

  if (_withHttp) {
    s=QBanking::QStringToUtf8String(httpVersionCombo->currentText());
    if (strcasecmp(s.c_str(), "1.0")==0) {
      AH_Customer_SetHttpVMajor(_customer, 1);
      AH_Customer_SetHttpVMinor(_customer, 0);
    }
    else if (strcasecmp(s.c_str(), "1.1")==0) {
      AH_Customer_SetHttpVMajor(_customer, 1);
      AH_Customer_SetHttpVMinor(_customer, 1);
    }

    s=QBanking::QStringToUtf8String(userAgentEdit->text());
    if (s.empty())
      AH_Customer_SetHttpUserAgent(_customer, 0);
    else
      AH_Customer_SetHttpUserAgent(_customer, s.c_str());

    s=QBanking::QStringToUtf8String(hostEdit->text());
    if (s.empty())
      AH_Customer_SetHttpHost(_customer, 0);
    else
      AH_Customer_SetHttpHost(_customer, s.c_str());
  }

  AH_Customer_SetBankSigns(_customer,
			   bankSignCheck->isChecked());
  AH_Customer_SetBankUsesSignSeq(_customer,
				 bankCounterCheck->isChecked());

  EditCustomerUi::accept();
}



void EditCustomer::_setComboTextIfPossible(QComboBox *qb,
                                           const QString &s){
  int i;

  for (i=0; i<qb->count(); i++) {
    if (qb->text(i)==s) {
      qb->setCurrentItem(i);
      break;
    }
  }
}



void EditCustomer::slotGetServerKeys() {
  AH_JOB *job;
  AH_OUTBOX *ob;

  job=AH_Job_GetKeys_new(_customer);
  if (!job) {
    DBG_ERROR(0, "Job not supported, should not happen");
    return;
  }

  ob=AH_Outbox_new(_hbci);
  AH_Outbox_AddJob(ob, job);

  if (AH_Outbox_Execute(ob, 1, 1)) {
    DBG_ERROR(0, "Could not execute outbox.\n");
    AH_HBCI_UnmountCurrentMedium(_hbci);
    AB_Banking_ProgressEnd(
			   AH_HBCI_GetBankingApi(_hbci), 0);
    AH_Outbox_free(ob);
    return;
  }

  if (AH_Job_GetStatus(job)!=AH_JobStatusAnswered) {
    DBG_ERROR(0, "Job has errors");
    // TODO: show errors
    AH_Outbox_free(ob);
    AH_HBCI_UnmountCurrentMedium(_hbci);
    AB_Banking_ProgressEnd(AH_HBCI_GetBankingApi(_hbci), 0);
    return;
  }
  else {
    AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(_hbci),
                           0,
                           AB_Banking_LogLevelNotice,
                           tr("Saving keys, please wait").utf8());
    if (AH_Job_Commit(job)) {
      DBG_ERROR(0, "Could not commit result.\n");
      AH_HBCI_UnmountCurrentMedium(_hbci);
      AB_Banking_ProgressEnd(AH_HBCI_GetBankingApi(_hbci), 0);
      AH_Outbox_free(ob);
      return;
    }
  }
  AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(_hbci),
                         0,
                         AB_Banking_LogLevelNotice,
                         tr("Keys saved.").utf8());
  AB_Banking_ProgressEnd(AH_HBCI_GetBankingApi(_hbci), 0);

  AH_Outbox_free(ob);
}



void EditCustomer::slotGetSysId() {
  AH_JOB *job;
  AH_OUTBOX *ob;
  const char *s;
  AH_USER *u;
  AH_BANK *b;
  AH_MEDIUM *m;

  u=AH_Customer_GetUser(_customer);
  assert(u);

  b=AH_User_GetBank(u);
  assert(b);

  m=AH_User_GetMedium(u);
  assert(m);

  job=AH_Job_GetSysId_new(_customer);
  if (!job) {
    DBG_ERROR(0, "Job not supported, should not happen");
    return;
  }
  AH_Job_AddSigner(job, AH_User_GetUserId(u));

  ob=AH_Outbox_new(_hbci);
  AH_Outbox_AddJob(ob, job);

  if (AH_Outbox_Execute(ob, 1, 1)) {
    DBG_ERROR(0, "Could not execute outbox.\n");
    AH_HBCI_UnmountCurrentMedium(_hbci);
    AB_Banking_ProgressEnd(AH_HBCI_GetBankingApi(_hbci), 0);
    AH_Outbox_free(ob);
    return;
  }

  if (AH_Job_HasErrors(job)) {
    DBG_ERROR(0, "Job has errors");
    // TODO: show errors
  }
  else {
    if (AH_Job_Commit(job)) {
      DBG_ERROR(0, "Could not commit result.\n");
      AH_HBCI_UnmountCurrentMedium(_hbci);
      AB_Banking_ProgressEnd(AH_HBCI_GetBankingApi(_hbci), 0);
      AH_Outbox_free(ob);
      return;
    }
  }

  s=AH_Job_GetSysId_GetSysId(job);
  if (!s) {
    AH_HBCI_UnmountCurrentMedium(_hbci);
    AB_Banking_ProgressEnd(AH_HBCI_GetBankingApi(_hbci), 0);
    AH_Outbox_free(ob);
    DBG_ERROR(0, "No system Id");
    return;
  }

  /* store system id */
  if (!AH_Medium_IsMounted(m)) {
    if (AH_Medium_Mount(m)) {
      DBG_ERROR(0, "Could not mount medium");
      AH_HBCI_UnmountCurrentMedium(_hbci);
      AB_Banking_ProgressEnd(AH_HBCI_GetBankingApi(_hbci), 0);
      AH_Outbox_free(ob);
      return;
    }
  }

  if (AH_Medium_SelectContext(m, AH_User_GetContextIdx(u))) {
    DBG_ERROR(0, "Could not select user");
    QMessageBox::critical(0,
			  tr("Medium Error"),
			  tr("Could not select user context on medium.\n"
			     "Please check the logs."
			    ),
			  tr("Dismiss"),0,0,0);
    AH_HBCI_UnmountCurrentMedium(_hbci);
    AB_Banking_ProgressEnd(AH_HBCI_GetBankingApi(_hbci), 0);
    AH_Outbox_free(ob);
    return;
  }

  AH_Customer_SetSystemId(_customer, s);

  AB_Banking_ProgressEnd(AH_HBCI_GetBankingApi(_hbci), 0);
  AH_Outbox_free(ob);
}







