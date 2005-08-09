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

#ifndef AQOFXCONNECT_KDE_USERLIST_H
#define AQOFXCONNECT_KDE_USERLIST_H


#include <qlistview.h>
#include <aqofxconnect/user.h>

#include <list>

class UserListView;
class UserListViewItem;


class UserListViewItem: public QListViewItem {
private:
  AO_USER *_user;

  void _populate();

public:
  UserListViewItem(UserListView *parent, AO_USER *user);
  UserListViewItem(UserListView *parent,
		      QListViewItem *after,
		      AO_USER *user);
  UserListViewItem(const UserListViewItem &item);

  virtual ~UserListViewItem();

  AO_USER *getUser();
};



class UserListView: public QListView {
private:
public:
  UserListView(QWidget *parent=0, const char *name=0);
  virtual ~UserListView();

  void addUser(AO_USER *user);
  void addUsers(AO_USER_LIST *ul);

  AO_USER *getCurrentUser();

  static QString hbciVersionToString(int version);
};




#endif /* AQOFXCONNECT_KDE_USERLIST_H */



