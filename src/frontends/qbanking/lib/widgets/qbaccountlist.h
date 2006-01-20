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

#ifndef QBANKING_ACCOUNTLIST_H
#define QBANKING_ACCOUNTLIST_H


#include <qlistview.h>
#include <aqbanking/account.h>

#include <qbanking/qbanking.h>

#include <list>

class QBAccountListView;
class QBAccountListViewItem;


class QBANKING_API QBAccountListViewItem: public QListViewItem {
private:
  AB_ACCOUNT *_account;

  void _populate();

public:
  QBAccountListViewItem(QBAccountListView *parent, AB_ACCOUNT *acc);
  QBAccountListViewItem(QBAccountListView *parent,
                        QListViewItem *after,
                        AB_ACCOUNT *acc);
  QBAccountListViewItem(const QBAccountListViewItem &item);

  virtual ~QBAccountListViewItem();

  AB_ACCOUNT *getAccount();

  QString key(int column, bool ascending) const;
};



class QBANKING_API QBAccountListView: public QListView {
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



