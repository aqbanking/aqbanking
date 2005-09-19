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
#include <aqbanking/banking.h>



UserListViewItem::UserListViewItem(UserListView *parent, AO_USER *user)
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
                                   AO_USER *user)
:QListViewItem(parent, after)
,_user(user){
  assert(user);
  _populate();
}



UserListViewItem::~UserListViewItem(){
}



AO_USER *UserListViewItem::getUser(){
  return _user;
}


void UserListViewItem::_populate() {
  int i;
  AB_BANKING *ab;
  AB_PROVIDER *pro;
  AO_BANK *b;
  const char *s;
  QString qs;
  const AB_COUNTRY *ci;

  assert(_user);
  b=AO_User_GetBank(_user);
  assert(b);
  pro=AO_Bank_GetProvider(b);
  assert(pro);
  ab=AB_Provider_GetBanking(pro);
  assert(ab);
  i=0;

  // country
  s=AO_Bank_GetCountry(b);
  if (!s || !*s)
    s="us";
  ci=AB_Banking_FindCountryByName(ab, s);
  if (ci)
    s=AB_Country_GetLocalName(ci);
  if (!s)
    s="";
  setText(i++, QString::fromUtf8(s));

  // bank name/code
  s=AO_Bank_GetBankName(b);
  if (!s)
    s=AO_Bank_GetBankId(b);
  if (!s)
    s="";
  setText(i++, QString::fromUtf8(s));

  // user name/id
  s=AO_User_GetUserId(_user);
  if (!s)
    s="";
  setText(i++, QString::fromUtf8(s));
}



UserListView::UserListView(QWidget *parent, const char *name)
:QListView(parent, name){
  setAllColumnsShowFocus(true);
  setShowSortIndicator(true);
  addColumn(QWidget::tr("Country"),-1);
  addColumn(QWidget::tr("Institute"),-1);
  addColumn(QWidget::tr("User"),-1);
}



UserListView::~UserListView(){
}



void UserListView::addUser(AO_USER *user){
  UserListViewItem *entry;

  entry=new UserListViewItem(this, user);
}



void UserListView::addUsers(AO_USER_LIST *users){
  AO_USER *u;

  fprintf(stderr, "Adding users...\n");
  u=AO_User_List_First(users);
  while(u) {
    UserListViewItem *entry;

    fprintf(stderr, "Adding user...\n");
    entry=new UserListViewItem(this, u);
    u=AO_User_List_Next(u);
  }
}



AO_USER *UserListView::getCurrentUser() {
  UserListViewItem *entry;

  entry=dynamic_cast<UserListViewItem*>(currentItem());
  if (!entry) {
    fprintf(stderr,"No item selected in list.\n");
    return 0;
  }
  return entry->getUser();
}

























