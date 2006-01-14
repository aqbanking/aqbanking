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


#include "cfgmodulegeldkarte_p.h"
#include "cfgtabpagk.h"

#include <aqgeldkarte/provider.h>

#include <qbanking/qbanking.h>

#include <aqbanking/banking_be.h>



CfgModuleGeldKarte::CfgModuleGeldKarte(QBanking *qb, const QString &name)
:QBCfgModule(qb, name) {
}



CfgModuleGeldKarte::~CfgModuleGeldKarte() {
}



QBCfgTabPageAccount *CfgModuleGeldKarte::getEditAccountPage(AB_ACCOUNT *a,
							    QWidget *parent) {
  return new CfgTabPageAccountGeldKarte(getBanking(), a, parent);
}







extern "C" {
  GWEN_PLUGIN*
    qbanking_cfg_module_aqgeldkarte_factory(GWEN_PLUGIN_MANAGER *pm,
                                            const char *name,
                                            const char *fileName) {
      return GWEN_Plugin_new(pm, name, fileName);
    }


  QBCfgModule *qbanking_cfg_module_aqgeldkarte_modfactory(QBanking *qb) {
    return new CfgModuleGeldKarte(qb, CFGMODULEGELDKARTE_NAME);
  }
}




