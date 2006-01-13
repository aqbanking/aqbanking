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


#ifndef AQHBCI_CFGMODULEDTAUS_H
#define AQHBCI_CFGMODULEDTAUS_H

#include <qbanking/qbcfgmodule.h>


#define CFGMODULEHBCI_NAME "aqdtaus"


class CfgModuleDtaus : public QBCfgModule {
private:

public:
  CfgModuleDtaus(QBanking *qb, const QString &name);
  virtual ~CfgModuleDtaus();

  virtual QBCfgTabPageAccount *getEditAccountPage(AB_ACCOUNT *a,
                                                  QWidget *parent=0);

};



#endif // AQHBCI_CFGMODULEDTAUS_H



