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


#include "kbaccountlist.h"
#include <assert.h>
#include <qstring.h>



KBAccountListViewItem::KBAccountListViewItem(KBAccountListView *parent,
                                         AB_ACCOUNT *acc)
:KListViewItem(parent)
,_account(acc){
  assert(acc);
  _populate();
}



KBAccountListViewItem::KBAccountListViewItem(const KBAccountListViewItem &item)
:KListViewItem(item)
,_account(0){

  if (item._account) {
    _account=item._account;
  }
}


KBAccountListViewItem::KBAccountListViewItem(KBAccountListView *parent,
                                         KListViewItem *after,
                                         AB_ACCOUNT *acc)
:KListViewItem(parent, after)
,_account(acc){
  assert(acc);
  _populate();
}



KBAccountListViewItem::~KBAccountListViewItem(){
}



AB_ACCOUNT *KBAccountListViewItem::getAccount(){
  return _account;
}


void KBAccountListViewItem::_populate() {
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









KBAccountListView::KBAccountListView(QWidget *parent, const char *name)
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



KBAccountListView::~KBAccountListView(){
}



void KBAccountListView::addAccount(AB_ACCOUNT *acc){
  KBAccountListViewItem *entry;

  entry=new KBAccountListViewItem(this, acc);
}



void KBAccountListView::addAccounts(const std::list<AB_ACCOUNT*> &accs){
  std::list<AB_ACCOUNT*>::const_iterator it;

  fprintf(stderr, "Adding accounts...\n");
  for (it=accs.begin(); it!=accs.end(); it++) {
    KBAccountListViewItem *entry;

    fprintf(stderr, "Adding account...\n");
    entry=new KBAccountListViewItem(this, *it);
  } /* for */
}



AB_ACCOUNT *KBAccountListView::getCurrentAccount() {
  KBAccountListViewItem *entry;

  entry=dynamic_cast<KBAccountListViewItem*>(currentItem());
  if (!entry) {
    fprintf(stderr,"No item selected in list.\n");
    return 0;
  }
  return entry->getAccount();
}



std::list<AB_ACCOUNT*> KBAccountListView::getSelectedAccounts(){
  std::list<AB_ACCOUNT*> accs;
  KBAccountListViewItem *entry;

  // Create an iterator and give the listview as argument
  QListViewItemIterator it(this);
  // iterate through all items of the listview
  for (;it.current();++it) {
    if (it.current()->isSelected()) {
      entry=dynamic_cast<KBAccountListViewItem*>(it.current());
      if (entry)
        accs.push_back(entry->getAccount());
    }
  } // for

  return accs;
}



























