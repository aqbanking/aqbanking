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


#include "cfgmoduleofx_p.h"
#include "cfgtabpageaccountofx.h"
#include "cfgtabpageuserofx.h"




CfgModuleOfx::CfgModuleOfx(QBanking *qb, const QString &name)
:QBCfgModule(qb, name) {

}



CfgModuleOfx::~CfgModuleOfx() {
}



QBCfgTabPageAccount *CfgModuleOfx::getEditAccountPage(AB_ACCOUNT *a,
                                                        QWidget *parent) {
  return new CfgTabPageAccountOfx(getBanking(), a, parent);
}



QBCfgTabPageUser *CfgModuleOfx::getEditUserPage(AB_USER *u,
                                                QWidget *parent) {
  return new CfgTabPageUserOfx(getBanking(), u, parent);
}





extern "C" {
  GWEN_PLUGIN*
    q4banking_cfg_module_aqofxconnect_factory(GWEN_PLUGIN_MANAGER *pm,
					      const char *name,
					      const char *fileName) {
      return GWEN_Plugin_new(pm, name, fileName);
    }


  QBCfgModule *q4banking_cfg_module_aqofxconnect_modfactory(QBanking *qb) {
    return new CfgModuleOfx(qb, CFGMODULEOFX_NAME);
  }
}




