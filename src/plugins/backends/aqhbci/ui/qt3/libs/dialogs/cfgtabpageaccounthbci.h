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


#ifndef AQHBCI_CFGTABPAGEACCOUNT_H
#define AQHBCI_CFGTABPAGEACCOUNT_H


#include <qbanking/qbcfgtabpageaccount.h>


class CfgTabPageAccountHbciUi;


class CfgTabPageAccountHbci: public QBCfgTabPageAccount {
  Q_OBJECT
private:
  CfgTabPageAccountHbciUi *_realPage;

public:
  CfgTabPageAccountHbci(QBanking *qb,
                         AB_ACCOUNT *a,
                         QWidget *parent=0, const char *name=0, WFlags f=0);
  virtual ~CfgTabPageAccountHbci();

  virtual bool fromGui();
  virtual bool toGui();
  virtual bool checkGui();

public slots:
  void accept();
  void slotFloppyToggled(bool on);
  void slotMountToggled(bool on);
  void slotFolderLostFocus();
  void slotFolder();

};

#endif // AQHBCI_CFGTABPAGEACCOUNT_H
