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


#ifndef AQOFX_CFGTABPAGEUSER_H
#define AQOFX_CFGTABPAGEUSER_H


#include <qbanking/qbcfgtabpageuser.h>


class CfgTabPageUserYnUi;


class CfgTabPageUserYn: public QBCfgTabPageUser {
  Q_OBJECT
private:
  CfgTabPageUserYnUi *_realPage;

public:
  CfgTabPageUserYn(QBanking *qb,
                   AB_USER *u,
                   QWidget *parent=0, const char *name=0, WFlags f=0);
  virtual ~CfgTabPageUserYn();

  virtual bool fromGui();
  virtual bool toGui();
  virtual bool checkGui();

public slots:
  void slotServerTest();
  void slotServerChanged(const QString &qs);
};

#endif // AQDTAUS_CFGTABPAGEUSER_H
