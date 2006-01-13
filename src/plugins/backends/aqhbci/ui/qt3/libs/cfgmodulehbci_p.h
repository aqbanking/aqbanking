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

#include <qbanking/qbcfgmodule.h>

#include <aqhbci/hbci.h>


#define CFGMODULEHBCI_NAME "aqhbci"


class CfgModuleHbci : public QBCfgModule {
private:
  AH_HBCI *_hbci;
public:
  CfgModuleHbci(QBanking *qb, const QString &name);
  virtual ~CfgModuleHbci();

  virtual QBCfgTabPageUser *getEditUserPage(AB_USER *u, QWidget *parent=0);
  virtual QBCfgTabPageAccount *getEditAccountPage(AB_ACCOUNT *a,
                                                  QWidget *parent=0);

  virtual int createNewUser(QWidget *parent=0);

};



#endif // AQHBCI_CFGMODULEHBCI_H



