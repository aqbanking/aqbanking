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


#ifndef AQGELDKARTE_CFGMODULEGELDKARTE_H
#define AQGELDKARTE_CFGMODULEGELDKARTE_H

#include <qbanking/qbcfgmodule.h>


#define CFGMODULEGELDKARTE_NAME "aqgeldkarte"


class CfgModuleGeldKarte : public QBCfgModule {
public:
  CfgModuleGeldKarte(QBanking *qb, const QString &name);
  virtual ~CfgModuleGeldKarte();

  virtual QBCfgTabPageAccount *getEditAccountPage(AB_ACCOUNT *a,
                                                  QWidget *parent=0);

};



#endif // AQGELDKARTE_CFGMODULEGELDKARTE_H



