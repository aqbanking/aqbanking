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


#ifndef AQHBCI_EDITACCOUNT_H
#define AQHBCI_EDITACCOUNT_H

#include "editaccount.ui.h"
#include <aqhbci/hbci.h>
#include <aqhbci/bank.h>
#include <aqhbci/customer.h>


class EditAccount : public EditAccountUi {
  Q_OBJECT
private:
  AH_HBCI *_hbci;
  AH_BANK *_bank;
  AH_CUSTOMER *_customer;
  AH_ACCOUNT *_account;
  AH_BANK_LIST2 *_banks;
  AH_CUSTOMER_LIST2 *_customers;
public:
  EditAccount(AH_HBCI *h,
              AH_ACCOUNT *a,
              QWidget* parent=0, const char* name=0,
              bool modal=FALSE, WFlags fl=0);
  virtual ~EditAccount();

  bool init();

public slots:
  void slotBankChanged(int i);
  void slotCustomerChanged(int i);
  void accept();


};






#endif // AQHBCI_EDITACCOUNT_H

