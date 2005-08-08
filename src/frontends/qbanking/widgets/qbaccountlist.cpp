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


#include "qbaccountlist.h"
#include <assert.h>
#include <qstring.h>



QBAccountListViewItem::QBAccountListViewItem(QBAccountListView *parent,
					     AB_ACCOUNT *acc)
:QListViewItem(parent)
,_account(acc){
  assert(acc);
  _populate();
}



QBAccountListViewItem::QBAccountListViewItem(const QBAccountListViewItem &item)
:QListViewItem(item)
,_account(0){

  if (item._account) {
    _account=item._account;
  }
}


QBAccountListViewItem::QBAccountListViewItem(QBAccountListView *parent,
					     QListViewItem *after,
					     AB_ACCOUNT *acc)
:QListViewItem(parent, after)
,_account(acc){
  assert(acc);
  _populate();
}



QBAccountListViewItem::~QBAccountListViewItem(){
}



AB_ACCOUNT *QBAccountListViewItem::getAccount(){
  return _account;
}


void QBAccountListViewItem::_populate() {
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









QBAccountListView::QBAccountListView(QWidget *parent, const char *name)
:QListView(parent, name){
  setAllColumnsShowFocus(true);
  setShowSortIndicator(true);
  addColumn(QWidget::tr("Institute Code"),-1);
  addColumn(QWidget::tr("Institute Name"),-1);
  addColumn(QWidget::tr("Account Number"),-1);
  addColumn(QWidget::tr("Account Name"),-1);
  addColumn(QWidget::tr("Owner"),-1);
  addColumn(QWidget::tr("Backend"),-1);
}



QBAccountListView::~QBAccountListView(){
}



void QBAccountListView::addAccount(AB_ACCOUNT *acc){
  QBAccountListViewItem *entry;

  entry=new QBAccountListViewItem(this, acc);
}



void QBAccountListView::addAccounts(const std::list<AB_ACCOUNT*> &accs){
  std::list<AB_ACCOUNT*>::const_iterator it;

  fprintf(stderr, "Adding accounts...\n");
  for (it=accs.begin(); it!=accs.end(); it++) {
    QBAccountListViewItem *entry;

    fprintf(stderr, "Adding account...\n");
    entry=new QBAccountListViewItem(this, *it);
  } /* for */
}



AB_ACCOUNT *QBAccountListView::getCurrentAccount() {
  QBAccountListViewItem *entry;

  entry=dynamic_cast<QBAccountListViewItem*>(currentItem());
  if (!entry) {
    fprintf(stderr,"No item selected in list.\n");
    return 0;
  }
  return entry->getAccount();
}



std::list<AB_ACCOUNT*> QBAccountListView::getSelectedAccounts(){
  std::list<AB_ACCOUNT*> accs;
  QBAccountListViewItem *entry;

  // Create an iterator and give the listview as argument
  QListViewItemIterator it(this);
  // iterate through all items of the listview
  for (;it.current();++it) {
    if (it.current()->isSelected()) {
      entry=dynamic_cast<QBAccountListViewItem*>(it.current());
      if (entry)
        accs.push_back(entry->getAccount());
    }
  } // for

  return accs;
}



























