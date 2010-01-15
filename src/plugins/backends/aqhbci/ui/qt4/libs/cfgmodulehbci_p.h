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


#ifndef AQHBCI_CFGMODULEHBCI_H
#define AQHBCI_CFGMODULEHBCI_H

#include <q4banking/qbcfgmodule.h>

#include <aqhbci/provider.h>


#define CFGMODULEHBCI_NAME "aqhbci"


class CfgModuleHbci : public QBCfgModule {
private:
  AB_PROVIDER *_provider;
public:
  CfgModuleHbci(QBanking *qb, const QString &name);
  virtual ~CfgModuleHbci();

  virtual QBCfgTabPageUser *getEditUserPage(AB_USER *u, QWidget *parent=0);
  virtual QBCfgTabPageAccount *getEditAccountPage(AB_ACCOUNT *a,
                                                  QWidget *parent=0);

  virtual int createNewUser(QWidget *parent=0);

};



#endif // AQHBCI_CFGMODULEHBCI_H



