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

#ifndef AQHBCI_KDE_ACCOUNTLIST_H
#define AQHBCI_KDE_ACCOUNTLIST_H


#include <qlistview.h>
#include <aqhbci/account.h>

#include <list>

class AccountListView;
class AccountListViewItem;


class AccountListViewItem: public QListViewItem {
private:
  AH_ACCOUNT *_account;

  void _populate();

public:
  AccountListViewItem(AccountListView *parent, AH_ACCOUNT *acc);
  AccountListViewItem(AccountListView *parent,
		      QListViewItem *after,
		      AH_ACCOUNT *acc);
  AccountListViewItem(const AccountListViewItem &item);

  virtual ~AccountListViewItem();

  AH_ACCOUNT *getAccount();
};



class AccountListView: public QListView {
private:
public:
  AccountListView(QWidget *parent=0, const char *name=0);
  virtual ~AccountListView();

  void addAccount(AH_ACCOUNT *acc);
  void addAccounts(AH_ACCOUNT_LIST2 *accs);

  AH_ACCOUNT *getCurrentAccount();
  AH_ACCOUNT_LIST2 *getSelectedAccounts();

};




#endif /* AQHBCI_KDE_ACCOUNTLIST_H */



