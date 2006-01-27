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


#ifndef AQYN_CFGTABPAGEACCOUNT_H
#define AQYN_CFGTABPAGEACCOUNT_H


#include <qbanking/qbcfgtabpageaccount.h>


class CfgTabPageAccountYnUi;


class CfgTabPageAccountYn: public QBCfgTabPageAccount {
private:
  CfgTabPageAccountYnUi *_realPage;

public:
  CfgTabPageAccountYn(QBanking *qb,
                      AB_ACCOUNT *a,
                      QWidget *parent=0, const char *name=0, WFlags f=0);
  virtual ~CfgTabPageAccountYn();

  virtual bool fromGui();
  virtual bool toGui();
  virtual bool checkGui();

};

#endif // AQYN_CFGTABPAGEACCOUNT_H

