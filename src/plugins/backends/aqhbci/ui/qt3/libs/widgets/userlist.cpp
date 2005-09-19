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


#include "userlist.h"
#include <assert.h>
#include <qstring.h>

#include <gwenhywfar/debug.h>



UserListViewItem::UserListViewItem(UserListView *parent,
                                   AH_USER *user)
:QListViewItem(parent)
,_user(user){
  assert(user);
  _populate();
}



UserListViewItem::UserListViewItem(const UserListViewItem &item)
:QListViewItem(item)
,_user(0){

  if (item._user) {
    _user=item._user;
  }
}


UserListViewItem::UserListViewItem(UserListView *parent,
                                   QListViewItem *after,
                                   AH_USER *user)
:QListViewItem(parent, after)
,_user(user){
  assert(user);
  _populate();
}



UserListViewItem::~UserListViewItem(){
}



AH_USER *UserListViewItem::getUser(){
  return _user;
}


void UserListViewItem::_populate() {
  int i;
  AH_BANK *b;
  AH_HBCI *h;
  AH_CUSTOMER *cu;
  const char *s;
  QString qs;
  AH_USER_STATUS ust;
  int v;

  assert(_user);
  b=AH_User_GetBank(_user);
  assert(b);
  h=AH_Bank_GetHbci(b);
  assert(h);
  cu=AH_User_FindCustomer(_user, "*");

  i=0;

  // bank name/code
  s=AH_Bank_GetBankName(b);
  if (!s)
    s=AH_Bank_GetBankId(b);
  if (!s)
    s="";
  setText(i++, QString::fromUtf8(s));

  // user name/id
  s=AH_User_GetUserId(_user);
  if (!s)
    s="";
  setText(i++, QString::fromUtf8(s));

  // status
  ust=AH_User_GetStatus(_user);
  switch(ust) {
  case AH_UserStatusNew:
    qs=QWidget::tr("New");
    break;
  case AH_UserStatusEnabled:
    qs=QWidget::tr("Enabled");
    break;
  case AH_UserStatusPending:
    qs=QWidget::tr("Pending");
    break;
  case AH_UserStatusDisabled:
    qs=QWidget::tr("Disabled");
    break;
  default:
    qs=QWidget::tr("unknown");
    break;
  }
  setText(i++, qs);

  if (cu) {
    v=AH_Customer_GetHbciVersion(cu);
    setText(i++, UserListView::hbciVersionToString(v));
  }
  else
    // skip this column
    i++;
}


QString 
UserListView::hbciVersionToString(int v)
{
  QString qs;
  switch(v) {
  case 2: 
    qs="2.0"; break;
  case 201: 
    qs="2.01"; break;
  case 210: 
    qs="2.10"; break;
  case 220: 
    qs="2.20"; break;
  default:  
    qs=QWidget::tr("unknown");
  }
  return qs;
}





UserListView::UserListView(QWidget *parent, const char *name)
:QListView(parent, name){
  setAllColumnsShowFocus(true);
  setShowSortIndicator(true);
  addColumn(QWidget::tr("Institute"),-1);
  addColumn(QWidget::tr("User"),-1);
  addColumn(QWidget::tr("Status"),-1);
  addColumn(QWidget::tr("HBCI Version"),-1);
}



UserListView::~UserListView(){
}



void UserListView::addUser(AH_USER *user){
  UserListViewItem *entry;

  entry=new UserListViewItem(this, user);
}



void UserListView::addUsers(AH_USER_LIST2 *users){
  AH_USER_LIST2_ITERATOR *it;

  fprintf(stderr, "Adding users...\n");
  it=AH_User_List2_First(users);
  if (it) {
    AH_USER *u;

    u=AH_User_List2Iterator_Data(it);
    while(u) {
      UserListViewItem *entry;

      fprintf(stderr, "Adding user...\n");
      entry=new UserListViewItem(this, u);
      u=AH_User_List2Iterator_Next(it);
    }
    AH_User_List2Iterator_free(it);
  }
}



AH_USER *UserListView::getCurrentUser() {
  UserListViewItem *entry;

  entry=dynamic_cast<UserListViewItem*>(currentItem());
  if (!entry) {
    fprintf(stderr,"No item selected in list.\n");
    return 0;
  }
  return entry->getUser();
}



AH_USER_LIST2 *UserListView::getSelectedUsers(){
  AH_USER_LIST2 *users;
  UserListViewItem *entry;

  // Create an iterator and give the listview as argument
  users=AH_User_List2_new();

  QListViewItemIterator it(this);
  // iterate through all items of the listview
  for (;it.current();++it) {
    if (it.current()->isSelected()) {
      entry=dynamic_cast<UserListViewItem*>(it.current());
      if (entry)
        AH_User_List2_PushBack(users, entry->getUser());
    }
  } // for

  if (!AH_User_List2_GetSize(users)) {
    AH_User_List2_free(users);
    return 0;
  }
  return users;
}


























