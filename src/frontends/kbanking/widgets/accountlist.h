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


#include <klistview.h>
#include <aqbanking/account.h>

#include <list>

class AccountListView;
class AccountListViewItem;


class AccountListViewItem: public KListViewItem {
private:
  AB_ACCOUNT *_account;

  void _populate();

public:
  AccountListViewItem(AccountListView *parent, AB_ACCOUNT *acc);
  AccountListViewItem(AccountListView *parent,
		      KListViewItem *after,
		      AB_ACCOUNT *acc);
  AccountListViewItem(const AccountListViewItem &item);

  virtual ~AccountListViewItem();

  AB_ACCOUNT *getAccount();
};



class AccountListView: public KListView {
private:
public:
  AccountListView(QWidget *parent=0, const char *name=0);
  virtual ~AccountListView();

  void addAccount(AB_ACCOUNT *acc);
  void addAccounts(const std::list<AB_ACCOUNT*> &accs);

  AB_ACCOUNT *getCurrentAccount();
  std::list<AB_ACCOUNT*> getSelectedAccounts();

};




#endif /* AQHBCI_KDE_ACCOUNTLIST_H */



