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


#ifndef AQOFX_CFGMODULEYN_H
#define AQOFX_CFGMODULEYN_H

#include <qbanking/qbcfgmodule.h>

#define CFGMODULEYN_NAME "aqyellownet"


class CfgModuleYn : public QBCfgModule {
private:
public:
  CfgModuleYn(QBanking *qb, const QString &name);
  virtual ~CfgModuleYn();

  virtual QBCfgTabPageUser *getEditUserPage(AB_USER *u, QWidget *parent=0);
  virtual QBCfgTabPageAccount *getEditAccountPage(AB_ACCOUNT *a,
                                                  QWidget *parent=0);

};



#endif // AQOFX_CFGMODULEOFX_H



