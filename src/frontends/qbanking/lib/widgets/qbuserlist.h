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

#ifndef QBANKING_USERLIST_H
#define QBANKING_USERLIST_H


#include <qlistview.h>
#include <aqhbci/user.h>

#include <list>

class QBUserListView;
class QBUserListViewItem;


class QBUserListViewItem: public QListViewItem {
private:
  AB_USER *_user;

  void _populate();

public:
  QBUserListViewItem(QBUserListView *parent, AB_USER *user);
  QBUserListViewItem(QBUserListView *parent,
		      QListViewItem *after,
		      AB_USER *user);
  QBUserListViewItem(const QBUserListViewItem &item);

  virtual ~QBUserListViewItem();

  AB_USER *getUser();
};



class QBUserListView: public QListView {
private:
public:
  QBUserListView(QWidget *parent=0, const char *name=0);
  virtual ~QBUserListView();

  void addUser(AB_USER *user);
  void addUsers(const std::list<AB_USER*> &users);

  void removeUser(AB_USER *user);

  AB_USER *getCurrentUser();
  std::list<AB_USER*> getSelectedUsers();
  std::list<AB_USER*> getSortedUsers();
  AB_USER_LIST2 *getSortedUsersList2();

};




#endif /* QBANKING_USERLIST_H */



