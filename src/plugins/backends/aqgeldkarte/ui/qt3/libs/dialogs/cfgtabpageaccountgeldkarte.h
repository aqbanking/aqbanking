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


#ifndef AQGELDKARTE_CFGTABPAGEACCOUNT_H
#define AQGELDKARTE_CFGTABPAGEACCOUNT_H


#include <qbanking/qbcfgtabpageaccount.h>


class CfgTabPageAccountGeldKarteUi;


class CfgTabPageAccountGeldKarte: public QBCfgTabPageAccount {
  Q_OBJECT
private:
  CfgTabPageAccountGeldKarteUi *_realPage;

public:
  CfgTabPageAccountGeldKarte(QBanking *qb,
                             AB_ACCOUNT *a,
                             QWidget *parent=0, const char *name=0,
                             WFlags f=0);
  virtual ~CfgTabPageAccountGeldKarte();

  virtual bool fromGui();
  virtual bool toGui();
  virtual bool checkGui();

public slots:
  void slotReadFromCard();

};

#endif // AQGELDKARTE_CFGTABPAGEACCOUNT_H
