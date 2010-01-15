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


#ifndef AQOFX_CFGMODULEOFX_H
#define AQOFX_CFGMODULEOFX_H

#include <q4banking/qbcfgmodule.h>

#define CFGMODULEOFX_NAME "aqofxconnect"


class CfgModuleOfx : public QBCfgModule {
private:
public:
  CfgModuleOfx(QBanking *qb, const QString &name);
  virtual ~CfgModuleOfx();

  virtual QBCfgTabPageUser *getEditUserPage(AB_USER *u, QWidget *parent=0);
  virtual QBCfgTabPageAccount *getEditAccountPage(AB_ACCOUNT *a,
                                                  QWidget *parent=0);

};



#endif // AQOFX_CFGMODULEOFX_H



