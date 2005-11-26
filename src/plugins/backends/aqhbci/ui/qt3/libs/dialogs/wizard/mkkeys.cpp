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

#include "mkkeys.h"
#include "wizard.h"
#include "winfo.h"

#include <qbanking/qbanking.h>

#include <aqhbci/outbox.h>
#include <aqhbci/adminjobs.h>

#include <gwenhywfar/debug.h>

#include <qmessagebox.h>
#include <qstring.h>
#include <qlabel.h>





MakeKeys::MakeKeys(Wizard *w,
                   QWidget* parent, const char* name, WFlags fl)
:MakeKeysUi(parent, name, fl)
,_wizard(w)
,_result(false) {

}



MakeKeys::~MakeKeys() {
}



bool MakeKeys::getResult() const {
  return _result;
}



void MakeKeys::slotMakeKeys() {
  WizardInfo *wInfo;
  QBanking *qb;
  AH_BANK *b;
  AH_USER *u;
  AH_CUSTOMER *cu;
  AH_MEDIUM *m;
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

  makeKeysLabel->setText(checking);

  _result=true;
  makeKeysLabel->setText(success);
}



