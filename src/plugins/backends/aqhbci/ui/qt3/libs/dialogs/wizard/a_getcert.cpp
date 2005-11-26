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


#include "a_getcert.h"
#include "wizard.h"
#include <qbanking/qbanking.h>

#include <aqhbci/dialog.h>
#include <gwenhywfar/debug.h>

#include <assert.h>

#include <qlabel.h>



ActionGetCert::ActionGetCert(Wizard *w)
:WizardAction(w, "GetCert", QWidget::tr("Retrieve Server Certificate")) {
  QLabel *tl;

  tl=new QLabel(this, "GetCertText");
  tl->setText("<tr>"
              "When you click <i>next</i> below we will attempt to "
              "retrieve the servers SSL certificate"
              "</tr>");
  addWidget(tl);
}



ActionGetCert::~ActionGetCert() {
}



bool ActionGetCert::apply() {
  WizardInfo *wInfo;
  QBanking *qb;
  AH_BANK *b;
  AH_CUSTOMER *cu;
  AH_DIALOG *dialog;
  int rv;
  int alwaysAskForCert;

  wInfo=getWizard()->getWizardInfo();
  assert(wInfo);
  b=wInfo->getBank();
  assert(b);
  cu=wInfo->getCustomer();
  assert(cu);
  dialog=AH_Dialog_new(cu);
  assert(dialog);

  qb=getWizard()->getBanking();
  AH_HBCI_RemoveAllBankCerts(wInfo->getHbci(), b);
  alwaysAskForCert=AB_Banking_GetAlwaysAskForCert(qb->getCInterface());
  AB_Banking_SetAlwaysAskForCert(qb->getCInterface(), 1);

  rv=AH_Dialog_Connect(dialog, 30);
  AH_Dialog_Disconnect(dialog, 2);
  AH_Dialog_free(dialog);
  AB_Banking_SetAlwaysAskForCert(qb->getCInterface(),
                                 alwaysAskForCert);
  if (rv) {
    DBG_ERROR(0, "Could not connect to server (%d)", rv);
    return false;
  }

  return true;
}






