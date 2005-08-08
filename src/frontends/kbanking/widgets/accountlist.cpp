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
:KListViewItem(parent)
,_account(acc){
  assert(acc);
  _populate();
}



AccountListViewItem::AccountListViewItem(const AccountListViewItem &item)
:KListViewItem(item)
,_account(0){

  if (item._account) {
    _account=item._account;
  }
}


AccountListViewItem::AccountListViewItem(AccountListView *parent,
                                         KListViewItem *after,
                                         AB_ACCOUNT *acc)
:KListViewItem(parent, after)
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
  QString tmp;
  int i;

  assert(_account);

  i=0;

  fprintf(stderr, "Populating...\n");

  // bank code
  setText(i++, AB_Account_GetBankCode(_account));

  // bank name
  tmp=AB_Account_GetBankName(_account);
  if (tmp.isEmpty())
    tmp="(unnamed)";
  setText(i++,tmp);

  // account id
  setText(i++,AB_Account_GetAccountNumber(_account));

  // account name
  tmp=AB_Account_GetAccountName(_account);
  if (tmp.isEmpty())
    tmp="(unnamed)";
  setText(i++, tmp);

  tmp=AB_Account_GetOwnerName(_account);
  if (tmp.isEmpty())
    tmp="";
  setText(i++, tmp);

  tmp=AB_Provider_GetName(AB_Account_GetProvider(_account));
  if (tmp.isEmpty())
    tmp="(unknown)";
  setText(i++, tmp);

}









AccountListView::AccountListView(QWidget *parent, const char *name)
:KListView(parent, name){
  setAllColumnsShowFocus(true);
  setShowSortIndicator(true);
  addColumn(QWidget::tr("Institute Code"),-1);
  addColumn(QWidget::tr("Institute Name"),-1);
  addColumn(QWidget::tr("Account Number"),-1);
  addColumn(QWidget::tr("Account Name"),-1);
  addColumn(QWidget::tr("Owner"),-1);
  addColumn(QWidget::tr("Backend"),-1);
}



AccountListView::~AccountListView(){
}



void AccountListView::addAccount(AB_ACCOUNT *acc){
  AccountListViewItem *entry;

  entry=new AccountListViewItem(this, acc);
}



void AccountListView::addAccounts(const std::list<AB_ACCOUNT*> &accs){
  std::list<AB_ACCOUNT*>::const_iterator it;

  fprintf(stderr, "Adding accounts...\n");
  for (it=accs.begin(); it!=accs.end(); it++) {
    AccountListViewItem *entry;

    fprintf(stderr, "Adding account...\n");
    entry=new AccountListViewItem(this, *it);
  } /* for */
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



std::list<AB_ACCOUNT*> AccountListView::getSelectedAccounts(){
  std::list<AB_ACCOUNT*> accs;
  AccountListViewItem *entry;

  // Create an iterator and give the listview as argument
  QListViewItemIterator it(this);
  // iterate through all items of the listview
  for (;it.current();++it) {
    if (it.current()->isSelected()) {
      entry=dynamic_cast<AccountListViewItem*>(it.current());
      if (entry)
        accs.push_back(entry->getAccount());
    }
  } // for

  return accs;
}



























