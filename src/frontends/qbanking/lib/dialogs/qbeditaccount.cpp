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

#include "qbeditaccount.h"
#include "qbcfgtabpageaccountgen.h"
#include "qbcfgmodule.h"

#include <qbanking/qbanking.h>

#include <gwenhywfar/debug.h>




QBEditAccount::QBEditAccount(QBanking *kb,
                             AB_ACCOUNT *a,
                             QWidget* parent,
                             const char* name,
                             Qt::WFlags fl)
:QBCfgTab(kb, parent, name, fl)
,_account(a) {
  QBCfgModule *mod;
  QBCfgTabPageAccount *uPage;
  const char *backendName;

  setCaption(tr("Account Configuration"));
  setHelpContext("QBEditAccount");
  setDescription(tr("<p>You can now setup this account.</p>"));

  /* add general page */
  uPage=new QBCfgTabPageAccountGeneral(kb, a, this, "GeneralAccountPage");
  addPage(uPage);
  uPage->show();

  /* add application specific page, if any */
  mod=kb->getConfigModule(0);
  if (mod) {
    uPage=mod->getEditAccountPage(a, this);
    if (uPage) {
      addPage(uPage);
      uPage->show();
    }
  }

  /* add backend specific page, if any */
  backendName=AB_Account_GetBackendName(a);
  assert(backendName);
  mod=kb->getConfigModule(backendName);
  if (mod) {
    uPage=mod->getEditAccountPage(a, this);
    if (uPage) {
      addPage(uPage);
      uPage->show();
    }
  }
}



QBEditAccount::~QBEditAccount() {
}



bool QBEditAccount::editAccount(QBanking *kb, AB_ACCOUNT *a, QWidget* parent){
  QBEditAccount ea(kb, a, parent);

  if (!ea.toGui())
    return false;
  if (ea.exec()!=QDialog::Accepted)
    return false;
  if (!ea.fromGui())
    return false;
  return true;
}








