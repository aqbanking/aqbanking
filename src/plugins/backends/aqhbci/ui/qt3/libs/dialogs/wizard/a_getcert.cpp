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
#include <aqhbci/provider.h>

#include <qbanking/qbanking.h>

#include <gwenhywfar/debug.h>

#include <assert.h>

#include <qlabel.h>



ActionGetCert::ActionGetCert(Wizard *w)
:WizardAction(w, "GetCert", QWidget::tr("Retrieve Server Certificate")) {
  QLabel *tl;

  tl=new QLabel(this, "GetCertText");
  tl->setText(tr("<qt>"
		 "When you click <i>next</i> below we will attempt to "
		 "retrieve the server's SSL certificate."
		 "</qt>"));
  addWidget(tl);
}



ActionGetCert::~ActionGetCert() {
}



bool ActionGetCert::apply() {
  WizardInfo *wInfo;
  AB_PROVIDER *pro;
  AB_USER *u;
  int rv;

  wInfo=getWizard()->getWizardInfo();
  assert(wInfo);
  pro=wInfo->getProvider();
  assert(pro);
  u=wInfo->getUser();
  assert(u);

  rv=AH_Provider_GetCert(pro, u, 1, 1, 1, 0);
  if (rv) {
    DBG_ERROR(0, "Could not get certificate (%d)", rv);
    return false;
  }

  return true;
}






