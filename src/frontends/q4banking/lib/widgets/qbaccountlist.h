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

#ifndef Q4BANKING_ACCOUNTLIST_H
#define Q4BANKING_ACCOUNTLIST_H


#include <Qt3Support/q3listview.h>
#include <aqbanking/account.h>

#include <q4banking/qbanking.h>

#include <list>

class QBAccountListView;
class QBAccountListViewItem;


class Q4BANKING_API QBAccountListViewItem: public Q3ListViewItem {
private:
  AB_ACCOUNT *_account;

  void _populate();

public:
  QBAccountListViewItem(QBAccountListView *parent, AB_ACCOUNT *acc);
  QBAccountListViewItem(QBAccountListView *parent,
                        Q3ListViewItem *after,
                        AB_ACCOUNT *acc);
  QBAccountListViewItem(const QBAccountListViewItem &item);

  virtual ~QBAccountListViewItem();

  AB_ACCOUNT *getAccount();

  QString key(int column, bool ascending) const;
};



class Q4BANKING_API QBAccountListView: public Q3ListView {
private:
public:
  QBAccountListView(QWidget *parent=0, const char *name=0);
  virtual ~QBAccountListView();

  void addAccount(AB_ACCOUNT *acc);
  void addAccounts(const std::list<AB_ACCOUNT*> &accs);

  AB_ACCOUNT *getCurrentAccount();
  std::list<AB_ACCOUNT*> getSelectedAccounts();

  std::list<AB_ACCOUNT*> getSortedAccounts();

};




#endif /* QBANKING_ACCOUNTLIST_H */



