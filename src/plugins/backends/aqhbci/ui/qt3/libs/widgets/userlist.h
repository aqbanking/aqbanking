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

#ifndef AQHBCI_KDE_USERLIST_H
#define AQHBCI_KDE_USERLIST_H


#include <qlistview.h>
#include <aqhbci/user.h>

#include <list>

class UserListView;
class UserListViewItem;


class UserListViewItem: public QListViewItem {
private:
  AH_USER *_user;

  void _populate();

public:
  UserListViewItem(UserListView *parent, AH_USER *user);
  UserListViewItem(UserListView *parent,
		      QListViewItem *after,
		      AH_USER *user);
  UserListViewItem(const UserListViewItem &item);

  virtual ~UserListViewItem();

  AH_USER *getUser();
};



class UserListView: public QListView {
private:
public:
  UserListView(QWidget *parent=0, const char *name=0);
  virtual ~UserListView();

  void addUser(AH_USER *user);
  void addUsers(AH_USER_LIST2 *accs);

  AH_USER *getCurrentUser();
  AH_USER_LIST2 *getSelectedUsers();

  static QString hbciVersionToString(int version);
};




#endif /* AQHBCI_KDE_USERLIST_H */



