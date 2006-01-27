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


#include "cfgmoduleyn_p.h"
#include "cfgtabpageaccountyn.h"
#include "cfgtabpageuseryn.h"




CfgModuleYn::CfgModuleYn(QBanking *qb, const QString &name)
:QBCfgModule(qb, name) {

}



CfgModuleYn::~CfgModuleYn() {
}



QBCfgTabPageAccount *CfgModuleYn::getEditAccountPage(AB_ACCOUNT *a,
                                                        QWidget *parent) {
  return new CfgTabPageAccountYn(getBanking(), a, parent);
}



QBCfgTabPageUser *CfgModuleYn::getEditUserPage(AB_USER *u,
                                                QWidget *parent) {
  return new CfgTabPageUserYn(getBanking(), u, parent);
}





extern "C" {
  GWEN_PLUGIN*
    qbanking_cfg_module_aqyellownet_factory(GWEN_PLUGIN_MANAGER *pm,
                                            const char *name,
                                            const char *fileName) {
      return GWEN_Plugin_new(pm, name, fileName);
    }


  QBCfgModule *qbanking_cfg_module_aqyellownet_modfactory(QBanking *qb) {
    return new CfgModuleYn(qb, CFGMODULEYN_NAME);
  }
}




