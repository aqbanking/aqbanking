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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "editaccount.h"

#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qtimer.h>

#include <gwenhywfar/debug.h>

#ifdef WIN32
# define strcasecmp stricmp
#endif


EditAccount::EditAccount(AH_HBCI *h,
                         AH_ACCOUNT *a,
                         QWidget* parent, const char* name,
                         bool modal, WFlags fl)
:EditAccountUi(parent, name, modal, fl)
,_hbci(h)
,_bank(0)
,_customer(0)
,_account(a)
,_banks(0)
,_customers(0) {

  QObject::connect((QObject*)(bankCombo),
                   SIGNAL(activated(int)),
                   this,
                   SLOT(slotBankChanged(int)));
  QObject::connect((QObject*)(customerIdCombo),
                   SIGNAL(activated(int)),
                   this,
                   SLOT(slotCustomerChanged(int)));

  QTimer::singleShot(0, this, SLOT(adjustSize()));
}



EditAccount::~EditAccount(){
}



void EditAccount::slotBankChanged(int i){
  AH_BANK_LIST2_ITERATOR *it;
  int j;
  AH_BANK *b;
  const char *s;
  AH_CUSTOMER_LIST2_ITERATOR *cit;
  AH_CUSTOMER *cu;
  const GWEN_STRINGLIST *acs;
  int found;
  const char *cid;

  if (!_banks)
    return;

  _customer=0;
  _bank=0;

  customerIdCombo->clear();
  customerIdCombo->insertItem(tr("Select a customer"), -1);

  if (i==0)
    return;

  it=AH_Bank_List2_First(_banks);
  assert(it);
  b=AH_Bank_List2Iterator_Data(it);
  assert(b);
  j=i;
  while(b && --j)
    b=AH_Bank_List2Iterator_Next(it);
  if (!b) {
    DBG_ERROR(0, "Internal error: combo box entry not found");
    abort();
  }
  _bank=b;
  AH_Bank_List2Iterator_free(it);
  bankCodeEdit->setText(AH_Bank_GetBankId(b));

  AH_Customer_List2_free(_customers);
  _customers=AH_Bank_GetCustomers(b, "*", "*");
  if (!_customers) {
    DBG_INFO(0, "No customers");
    return;
  }
  cit=AH_Customer_List2_First(_customers);
  assert(cit);
  cu=AH_Customer_List2Iterator_Data(cit);
  assert(cu);

  acs=AH_Account_GetCustomers(_account);
  cid=0;
  if (acs) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(acs);
    if (se)
      cid=GWEN_StringListEntry_Data(se);
  }

  found=0;
  i=0;
  while(cu) {
    const char *ccid;

    i++;
    ccid=AH_Customer_GetCustomerId(cu);
    s=AH_Customer_GetFullName(cu);
    if (!s)
      s=ccid;
    else
      if (!*s)
        s=ccid;
    customerIdCombo->insertItem(s, -1);
    if (cid)
      if (strcasecmp(cid, ccid)==0) {
        found=i;
      }
    cu=AH_Customer_List2Iterator_Next(cit);
  }
  AH_Customer_List2Iterator_free(cit);

  if (found) {
    customerIdCombo->setCurrentItem(found);
    slotCustomerChanged(found);
  }


}



void EditAccount::slotCustomerChanged(int i){
  int j;
  AH_CUSTOMER_LIST2_ITERATOR *cit;
  AH_CUSTOMER *cu;

  if (!_customers)
    return;

  cit=AH_Customer_List2_First(_customers);
  if (!cit)
    return;

  cu=AH_Customer_List2Iterator_Data(cit);
  assert(cu);

  j=i;
  while(cu && --j)
    cu=AH_Customer_List2Iterator_Next(cit);
  AH_Customer_List2Iterator_free(cit);
  if (!cu) {
    DBG_ERROR(0, "Internal error: Could not find customer in combo box");
    return;
  }
  _customer=cu;
}




bool EditAccount::init() {
  AH_BANK_LIST2_ITERATOR *it;
  AH_BANK *b;
  int found;
  int i;
  const char *s;

  _banks=AH_HBCI_GetBanks(_hbci, 0, "*");
  if (!_banks) {
    QMessageBox::critical(this,
                          tr("No Bank List"),
                          tr("<qt>"
                             "<p>"
                             "There are no banks set up."
                             "</p>"
                             "<p>"
                             "Banks are created when users are added. "
                             "Please add a user and come back later."
                             "</p>"
                             "</qt>"
                            ),
                          tr("Dismiss"),0,0,0);
    return false;
  }

  it=AH_Bank_List2_First(_banks);
  assert(it);
  b=AH_Bank_List2Iterator_Data(it);
  assert(b);
  bankCombo->insertItem(tr("Select a bank"), -1);
  found=0;
  i=0;
  while(b) {
    i++;
    s=AH_Bank_GetBankName(b);
    if (!s)
      s=AH_Bank_GetBankId(b);
    bankCombo->insertItem(s, -1);
    if (b==AH_Account_GetBank(_account))
      found=i;
    b=AH_Bank_List2Iterator_Next(it);
  }
  AH_Bank_List2Iterator_free(it);

  if (found) {
    bankCombo->setCurrentItem(found);
    slotBankChanged(found);
  }

  s=AH_Account_GetBankId(_account);
  if (s)
    bankCodeEdit->setText(s);

  s=AH_Account_GetAccountId(_account);
  if (s)
    accountIdEdit->setText(s);

  s=AH_Account_GetOwnerName(_account);
  if (s)
    ownerNameEdit->setText(s);

  s=AH_Account_GetAccountName(_account);
  if (s)
    accountNameEdit->setText(s);

  return true;
}



void EditAccount::accept(){

  if (!_bank ||
      !_customer ||
      bankCodeEdit->text().isEmpty() ||
      accountIdEdit->text().isEmpty() ||
      ownerNameEdit->text().isEmpty() ||
      bankCombo->currentItem()==0 ||
      customerIdCombo->currentItem()==0) {
    QMessageBox::critical(this,
                          tr("No Bank List"),
                          tr("<qt>"
                             "<p>"
                             "Your input is incomplete."
                             "</p>"
                             "<p>"
                             "Please fill out all required fields or "
                             "abort the dialog."
                             "</p>"
                             "</qt>"
                            ),
                          tr("Dismiss"),0,0,0);
    return;
  }

  AH_Account_SetBank(_account, _bank);
  AH_Account_SetBankId(_account, bankCodeEdit->text().utf8());
  AH_Account_SetAccountId(_account, accountIdEdit->text().utf8());
  if (!accountNameEdit->text().isEmpty())
    AH_Account_SetAccountName(_account, accountNameEdit->text().utf8());
  AH_Account_SetOwnerName(_account, ownerNameEdit->text().utf8());
  AH_Account_ClearCustomers(_account);
  AH_Account_AddCustomer(_account, AH_Customer_GetCustomerId(_customer));

  QDialog::accept();
}








