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


#include "cfgmoduledtaus_p.h"
#include "cfgtabpageaccountdtaus.h"



CfgModuleDtaus::CfgModuleDtaus(QBanking *qb, const QString &name)
:QBCfgModule(qb, name) {

}



CfgModuleDtaus::~CfgModuleDtaus() {
}



QBCfgTabPageAccount *CfgModuleDtaus::getEditAccountPage(AB_ACCOUNT *a,
                                                        QWidget *parent) {
  return new CfgTabPageAccountDtaus(getBanking(), a, parent);
}





extern "C" {
  GWEN_PLUGIN *qbanking_cfg_module_aqdtaus_factory(GWEN_PLUGIN_MANAGER *pm,
                                                  const char *name,
                                                  const char *fileName) {
    return GWEN_Plugin_new(pm, name, fileName);
  }


  QBCfgModule *qbanking_cfg_module_aqdtaus_modfactory(QBanking *qb) {
    return new CfgModuleDtaus(qb, CFGMODULEHBCI_NAME);
  }
}




