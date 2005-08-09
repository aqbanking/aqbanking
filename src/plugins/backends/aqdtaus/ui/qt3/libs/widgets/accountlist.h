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

#ifndef AQDTAUS_KDE_ACCOUNTLIST_H
#define AQDTAUS_KDE_ACCOUNTLIST_H


#include <qlistview.h>
#include <aqbanking/account.h>

#include <list>

class AccountListView;
class AccountListViewItem;


class AccountListViewItem: public QListViewItem {
private:
  AB_ACCOUNT *_account;

  void _populate();

public:
  AccountListViewItem(AccountListView *parent, AB_ACCOUNT *acc);
  AccountListViewItem(AccountListView *parent,
		      QListViewItem *after,
		      AB_ACCOUNT *acc);
  AccountListViewItem(const AccountListViewItem &item);

  virtual ~AccountListViewItem();

  AB_ACCOUNT *getAccount();
};



class AccountListView: public QListView {
private:
public:
  AccountListView(QWidget *parent=0, const char *name=0);
  ~AccountListView();

  void addAccount(AB_ACCOUNT *acc);
  void addAccounts(AB_ACCOUNT_LIST2 *accs);

  AB_ACCOUNT *getCurrentAccount();
  AB_ACCOUNT_LIST2 *getSelectedAccounts();

};




#endif /* AQDTAUS_KDE_ACCOUNTLIST_H */



