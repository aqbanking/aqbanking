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


#include "qbuserlist.h"
#include <assert.h>
#include <qstring.h>

#include <gwenhywfar/debug.h>



QBUserListViewItem::QBUserListViewItem(QBUserListView *parent,
                                   AB_USER *user)
:Q3ListViewItem(parent)
,_user(user){
  assert(user);
  _populate();
}



QBUserListViewItem::QBUserListViewItem(const QBUserListViewItem &item)
:Q3ListViewItem(item)
,_user(0){

  if (item._user) {
    _user=item._user;
  }
}


QBUserListViewItem::QBUserListViewItem(QBUserListView *parent,
                                   Q3ListViewItem *after,
                                   AB_USER *user)
:Q3ListViewItem(parent, after)
,_user(user){
  assert(user);
  _populate();
}



QBUserListViewItem::~QBUserListViewItem(){
}



AB_USER *QBUserListViewItem::getUser(){
  return _user;
}


void QBUserListViewItem::_populate() {
  int i;
  const char *s;
  QString qs;

  assert(_user);

  i=0;

  // bank name/code
  s=AB_User_GetBankCode(_user);
  if (!s)
    s="";
  setText(i++, QString::fromUtf8(s));

  // user name/id
  s=AB_User_GetUserId(_user);
  if (!s)
    s="";
  setText(i++, QString::fromUtf8(s));

  // customer id
  s=AB_User_GetCustomerId(_user);
  if (!s)
    s="";
  setText(i++, QString::fromUtf8(s));

  /* backend name */
  s=AB_User_GetBackendName(_user);
  if (!s)
    s="";
  setText(i++, QString::fromUtf8(s));

}



QBUserListView::QBUserListView(QWidget *parent, const char *name)
:Q3ListView(parent, name){
  setAllColumnsShowFocus(true);
  setShowSortIndicator(true);
  addColumn(QWidget::tr("Institute"),-1);
  addColumn(QWidget::tr("User Id"),-1);
  addColumn(QWidget::tr("Customer Id"),-1);
  addColumn(QWidget::tr("Backend"),-1);
}



QBUserListView::~QBUserListView(){
}



void QBUserListView::addUser(AB_USER *user){
  QBUserListViewItem *entry;

  entry=new QBUserListViewItem(this, user);
}



void QBUserListView::addUsers(const std::list<AB_USER*> &users) {
  std::list<AB_USER*>::const_iterator it;

  for (it=users.begin(); it!=users.end(); it++) {
    QBUserListViewItem *entry;

    entry=new QBUserListViewItem(this, *it);
  } /* for */

}



AB_USER *QBUserListView::getCurrentUser() {
  QBUserListViewItem *entry;

  entry=dynamic_cast<QBUserListViewItem*>(currentItem());
  if (!entry) {
    return 0;
  }
  return entry->getUser();
}



std::list<AB_USER*> QBUserListView::getSelectedUsers(){
  std::list<AB_USER*> users;
  QBUserListViewItem *entry;

  // Create an iterator and give the listview as argument
  Q3ListViewItemIterator it(this);
  // iterate through all items of the listview
  for (;it.current();++it) {
    if (it.current()->isSelected()) {
      entry=dynamic_cast<QBUserListViewItem*>(it.current());
      if (entry)
        users.push_back(entry->getUser());
    }
  } // for

  return users;
}



std::list<AB_USER*> QBUserListView::getSortedUsers() {
  std::list<AB_USER*> users;
  QBUserListViewItem *entry;

  // Create an iterator and give the listview as argument
  Q3ListViewItemIterator it(this);
  // iterate through all items of the listview
  for (;it.current();++it) {
    entry=dynamic_cast<QBUserListViewItem*>(it.current());
    if (entry)
      users.push_back(entry->getUser());
  } // for

  return users;
}



AB_USER_LIST2 *QBUserListView::getSortedUsersList2(){
  AB_USER_LIST2 *users;
  QBUserListViewItem *entry;

  users=AB_User_List2_new();
  // Create an iterator and give the listview as argument
  Q3ListViewItemIterator it(this);
  // iterate through all items of the listview
  for (;it.current();++it) {
    entry=dynamic_cast<QBUserListViewItem*>(it.current());
    if (entry)
      AB_User_List2_PushBack(users, entry->getUser());
  } // for

  if (AB_User_List2_GetSize(users)==0) {
    AB_User_List2_free(users);
    return 0;
  }
  return users;
}



void QBUserListView::removeUser(AB_USER *user) {
  // Create an iterator and give the listview as argument
  Q3ListViewItemIterator it(this);
  // iterate through all items of the listview
  for (;it.current();++it) {
    QBUserListViewItem *entry;

    entry=dynamic_cast<QBUserListViewItem*>(it.current());
    if (entry && entry->getUser()==user) {
      delete entry;
      break;
    }
  } // for
}









