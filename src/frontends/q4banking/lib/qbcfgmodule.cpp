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


#include "qbcfgmodule.h"
#include "qbcfgtabpageuser.h"
#include "qbcfgtabpageaccount.h"




QBCfgModule::QBCfgModule(QBanking *qb, const QString &name)
:_qbanking(qb)
,_name(name)
,_plugin(0)
,_flags(0)
{

}



QBCfgModule::~QBCfgModule() {
}



QBCfgTabPageUser *QBCfgModule::getEditUserPage(AB_USER *u,
                                               QWidget *parent) {
  return 0;
}



QBCfgTabPageAccount *QBCfgModule::getEditAccountPage(AB_ACCOUNT *a,
                                                     QWidget *parent) {
  return 0;
}



QBanking *QBCfgModule::getBanking() {
  return _qbanking;
}



void QBCfgModule::setPlugin(GWEN_PLUGIN *pl) {
  _plugin=pl;
}



GWEN_PLUGIN *QBCfgModule::getPlugin() {
  return _plugin;
}



const QString &QBCfgModule::getName() const {
  return _name;
}



uint32_t QBCfgModule::getFlags() const {
  return _flags;
}



void QBCfgModule::setFlags(uint32_t fl) {
  _flags=fl;
}



int QBCfgModule::createNewUser(QWidget *parent) {
  return GWEN_ERROR_NOT_SUPPORTED;
}



int QBCfgModule::createNewAccount(QWidget *parent) {
  return GWEN_ERROR_NOT_SUPPORTED;
}







