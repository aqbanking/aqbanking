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

class KBAccountListView;
class KBAccountListViewItem;


class KBAccountListViewItem: public KListViewItem {
private:
  AB_ACCOUNT *_account;

  void _populate();

public:
  KBAccountListViewItem(KBAccountListView *parent, AB_ACCOUNT *acc);
  KBAccountListViewItem(KBAccountListView *parent,
		      KListViewItem *after,
		      AB_ACCOUNT *acc);
  KBAccountListViewItem(const KBAccountListViewItem &item);

  virtual ~KBAccountListViewItem();

  AB_ACCOUNT *getAccount();
};



class KBAccountListView: public KListView {
private:
public:
  KBAccountListView(QWidget *parent=0, const char *name=0);
  virtual ~KBAccountListView();

  void addAccount(AB_ACCOUNT *acc);
  void addAccounts(const std::list<AB_ACCOUNT*> &accs);

  AB_ACCOUNT *getCurrentAccount();
  std::list<AB_ACCOUNT*> getSelectedAccounts();

};




#endif /* AQHBCI_KDE_ACCOUNTLIST_H */



