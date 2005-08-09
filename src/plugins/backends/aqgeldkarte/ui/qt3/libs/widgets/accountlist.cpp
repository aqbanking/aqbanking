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


#include "accountlist.h"
#include <assert.h>
#include <qstring.h>



AccountListViewItem::AccountListViewItem(AccountListView *parent,
                                         AB_ACCOUNT *acc)
:QListViewItem(parent)
,_account(acc){
  assert(acc);
  _populate();
}



AccountListViewItem::AccountListViewItem(const AccountListViewItem &item)
:QListViewItem(item)
,_account(0){

  if (item._account) {
    _account=item._account;
  }
}


AccountListViewItem::AccountListViewItem(AccountListView *parent,
                                         QListViewItem *after,
                                         AB_ACCOUNT *acc)
:QListViewItem(parent, after)
,_account(acc){
  assert(acc);
  _populate();
}



AccountListViewItem::~AccountListViewItem(){
}



AB_ACCOUNT *AccountListViewItem::getAccount(){
  return _account;
}


void AccountListViewItem::_populate() {
  int i;
  const char *s;

  assert(_account);

  i=0;

  // bank name/code
  s=AB_Account_GetBankName(_account);
  if (!s)
    AB_Account_GetBankCode(_account);
  if (!s)
    s="";
  setText(i++, QString::fromUtf8(s));

  // account number
  s=AB_Account_GetAccountNumber(_account);
  if (!s)
    s="";
  setText(i++, QString::fromUtf8(s));

  // owner name
  s=AB_Account_GetOwnerName(_account);
  if (!s)
    s="";
  setText(i++, QString::fromUtf8(s));
}









AccountListView::AccountListView(QWidget *parent, const char *name)
:QListView(parent, name){
  setAllColumnsShowFocus(true);
  setShowSortIndicator(true);
  addColumn(QWidget::tr("Institute"),-1);
  addColumn(QWidget::tr("Account"),-1);
  addColumn(QWidget::tr("Owner"),-1);
}



AccountListView::~AccountListView(){
}



void AccountListView::addAccount(AB_ACCOUNT *acc){
  AccountListViewItem *entry;

  entry=new AccountListViewItem(this, acc);
}



void AccountListView::addAccounts(AB_ACCOUNT_LIST2 *accs){
  AB_ACCOUNT_LIST2_ITERATOR *it;

  fprintf(stderr, "Adding accounts...\n");
  it=AB_Account_List2_First(accs);
  if (it) {
    AB_ACCOUNT *a;

    a=AB_Account_List2Iterator_Data(it);
    while(a) {
      AccountListViewItem *entry;

      fprintf(stderr, "Adding account...\n");
      entry=new AccountListViewItem(this, a);
      a=AB_Account_List2Iterator_Next(it);
    }
    AB_Account_List2Iterator_free(it);
  }
}



AB_ACCOUNT *AccountListView::getCurrentAccount() {
  AccountListViewItem *entry;

  entry=dynamic_cast<AccountListViewItem*>(currentItem());
  if (!entry) {
    fprintf(stderr,"No item selected in list.\n");
    return 0;
  }
  return entry->getAccount();
}



AB_ACCOUNT_LIST2 *AccountListView::getSelectedAccounts(){
  AB_ACCOUNT_LIST2 *accs;
  AccountListViewItem *entry;

  // Create an iterator and give the listview as argument
  accs=AB_Account_List2_new();

  QListViewItemIterator it(this);
  // iterate through all items of the listview
  for (;it.current();++it) {
    if (it.current()->isSelected()) {
      entry=dynamic_cast<AccountListViewItem*>(it.current());
      if (entry)
        AB_Account_List2_PushBack(accs, entry->getAccount());
    }
  } // for

  if (!AB_Account_List2_GetSize(accs)) {
    AB_Account_List2_free(accs);
    return 0;
  }
  return accs;
}



























