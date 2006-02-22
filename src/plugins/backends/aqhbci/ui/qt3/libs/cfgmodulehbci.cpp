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


#include "cfgmodulehbci_p.h"
#include "cfgtabpageuserhbci.h"
#include "cfgtabpageaccounthbci.h"
#include "userwizard.h"

#include <aqhbci/provider.h>

#include <qbanking/qbanking.h>

#include <aqbanking/banking_be.h>



CfgModuleHbci::CfgModuleHbci(QBanking *qb, const QString &name)
:QBCfgModule(qb, name), _provider(0) {
  AB_PROVIDER *pro;

  pro=AB_Banking_GetProvider(qb->getCInterface(), AH_PROVIDER_NAME);
  assert(pro);
  _provider=pro;
  setFlags(QBCFGMODULE_FLAGS_CAN_CREATE_USER);
}



CfgModuleHbci::~CfgModuleHbci() {
}



QBCfgTabPageUser *CfgModuleHbci::getEditUserPage(AB_USER *u,
                                                 QWidget *parent) {
  return new CfgTabPageUserHbci(getBanking(), u, parent);
}



QBCfgTabPageAccount *CfgModuleHbci::getEditAccountPage(AB_ACCOUNT *a,
                                                       QWidget *parent) {
  return new CfgTabPageAccountHbci(getBanking(), a, parent);
}



int CfgModuleHbci::createNewUser(QWidget *parent) {
  UserWizard dlg(getBanking(), _provider, parent);

  if (dlg.exec())
    return 0;
  return AB_ERROR_USER_ABORT;
}





extern "C" {
  GWEN_PLUGIN *qbanking_cfg_module_aqhbci_factory(GWEN_PLUGIN_MANAGER *pm,
                                                  const char *name,
                                                  const char *fileName) {
    return GWEN_Plugin_new(pm, name, fileName);
  }


  QBCfgModule *qbanking_cfg_module_aqhbci_modfactory(QBanking *qb) {
    return new CfgModuleHbci(qb, CFGMODULEHBCI_NAME);
  }
}




