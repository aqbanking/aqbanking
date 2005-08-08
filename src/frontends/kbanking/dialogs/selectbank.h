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


#ifndef AQHBCI_WIZARD_SELBANK_H
#define AQHBCI_WIZARD_SELBANK_H


class KBanking;

#include "selectbank.ui.h"
#include <aqbanking/bankinfo.h>
#include <string>


class SelectBank: public SelectBankUi {
  Q_OBJECT

private:
  KBanking *_app;
  AB_BANKINFO *_bankInfo;
  std::string _country;
  bool _changed;

  AB_BANKINFO *_getBankInfo();

public:
  SelectBank(KBanking *kb,
             QWidget* parent = 0,
             const char* name = 0,
             bool modal = FALSE,
             WFlags fl = 0);
  ~SelectBank();

  const AB_BANKINFO *selectedBankInfo() const;

  void accept();

  static AB_BANKINFO *selectBank(KBanking *kb,
                                 QWidget* parent=0,
                                 const QString &title="",
                                 const QString &country="de",
                                 const QString &bankCode="",
                                 const QString &swiftCode="",
                                 const QString &bankName="",
                                 const QString &location="");


public slots:
  void slotUpdate();
  void slotChanged(const QString &qs);
  void slotSelectionChanged();
  void slotDoubleClicked(QListViewItem *lv,
                         const QPoint &,
                         int);


};


#endif // AQHBCI_WIZARD_SELBANK_H

