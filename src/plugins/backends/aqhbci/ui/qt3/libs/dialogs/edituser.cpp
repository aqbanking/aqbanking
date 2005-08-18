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


#include "edituser.h"
#include "editcustomer.h"

#include <qbanking/qbanking.h>

#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qlistview.h>
#include <qtimer.h>

#include <gwenhywfar/debug.h>

#ifdef WIN32
# define strcasecmp stricmp
#endif



EditUser::EditUser(AH_HBCI *h,
                   AH_USER *u,
                   QWidget* parent, const char* name,
                   bool modal, WFlags fl)
:EditUserUi(parent, name, modal, fl)
,_hbci(h)
,_user(u) {
  const char *s;
  const AH_BPD_ADDR *ba;
  AH_MEDIUM *m;
  AH_USER_STATUS ust;
  QString qs;

  s=AH_User_GetUserId(u);
  if (s)
    userIdEdit->setText(QString::fromUtf8(s));

  userStatusCombo->insertItem(tr("New"));
  userStatusCombo->insertItem(tr("Enabled"));
  userStatusCombo->insertItem(tr("Pending"));
  userStatusCombo->insertItem(tr("Disabled"));
  userStatusCombo->insertItem(tr("Unknown"));
  ust=AH_User_GetStatus(u);
  switch(ust) {
  case AH_UserStatusNew:      qs=tr("New"); break;
  case AH_UserStatusEnabled:  qs=tr("Enabled"); break;
  case AH_UserStatusPending:  qs=tr("Pending"); break;
  case AH_UserStatusDisabled: qs=tr("Disabled"); break;
  case AH_UserStatusUnknown:  qs=tr("Unknown"); break;
  }
  _setComboTextIfPossible(userStatusCombo, qs);

  ba=AH_User_GetAddress(u);
  if (ba) {
    s=AH_BpdAddr_GetAddr(ba);
    if (ba)
      serverEdit->setText(QString::fromUtf8(s));
  }

  m=AH_User_GetMedium(u);
  if (m) {
    s=AH_Medium_GetDescriptiveName(m);
    if (s)
      descriptiveEdit->setText(QString::fromUtf8(s));
  }

  updateLists();

  QObject::connect((QObject*)(editButton),
                   SIGNAL(clicked()),
                   this,
                   SLOT(slotEditCustomer()));

  QTimer::singleShot(0, this, SLOT(adjustSize()));
}



EditUser::~EditUser() {
}



void EditUser::updateLists() {
  AH_CUSTOMER_LIST2 *cl;

  customerList->clear();
  cl=AH_User_GetCustomers(_user, "*");
  if (cl) {
    AH_CUSTOMER_LIST2_ITERATOR *cit;
    AH_CUSTOMER *cu;

    cit=AH_Customer_List2_First(cl);
    assert(cit);
    cu=AH_Customer_List2Iterator_Data(cit);
    assert(cu);
    while(cu) {
      const char *cid;

      cid=AH_Customer_GetCustomerId(cu);
      if (cid) {
        const char *fn;
        QListViewItem *lvi;

        fn=AH_Customer_GetFullName(cu);
        if (!fn)
          fn="";

        lvi=new QListViewItem(customerList, cid, fn);
      }
      cu=AH_Customer_List2Iterator_Next(cit);
    }
    AH_Customer_List2Iterator_free(cit);

    AH_Customer_List2_free(cl);
  }

}


bool EditUser::init() {
  return true;
}



bool EditUser::fini() {
  return true;
}




void EditUser::slotStatusChanged(int i) {
}



void EditUser::accept() {
  std::string s;
  QString qs;
  AH_USER_STATUS ust;
  const AH_BPD_ADDR *oldBa;
  AH_BPD_ADDR *newBa;
  AH_MEDIUM *m;

  if (userIdEdit->text().isEmpty() ||
      serverEdit->text().isEmpty()) {
    DBG_ERROR(0, "Error: Empty fields");
    QMessageBox::critical(this,
                          tr("Empty Fields"),
                          tr("<qt>"
                             "<p>"
                             "Please fill out all necessary fields."
                             "</p>"
                             "</qt>"
                            ),
                          tr("Dismiss"),0,0,0);
    return;
  }

  s=QBanking::QStringToUtf8String(userIdEdit->text());
  AH_User_SetUserId(_user, s.c_str());

  switch(userStatusCombo->currentItem()) {
  case 0:  ust=AH_UserStatusNew; break;
  case 1:  ust=AH_UserStatusEnabled; break;
  case 2:  ust=AH_UserStatusPending; break;
  case 3:  ust=AH_UserStatusDisabled; break;
  default: ust=AH_UserStatusUnknown; break;
  }
  AH_User_SetStatus(_user, ust);

  qs=serverEdit->text();
  oldBa=AH_User_GetAddress(_user);
  assert(oldBa);
  newBa=AH_BpdAddr_dup(oldBa);
  s=QBanking::QStringToUtf8String(qs);
  AH_BpdAddr_SetAddr(newBa, s.c_str());

  m=AH_User_GetMedium(_user);
  assert(m);

  s=QBanking::QStringToUtf8String(descriptiveEdit->text());
  AH_Medium_SetDescriptiveName(m, s.c_str());

  EditUserUi::accept();
}



void EditUser::_setComboTextIfPossible(QComboBox *qb,
                                       const QString &s){
  int i;

  for (i=0; i<qb->count(); i++) {
    if (qb->text(i)==s) {
      qb->setCurrentItem(i);
      break;
    }
  }
}



void EditUser::slotEditCustomer() {
  QListViewItem *lvi=customerList->currentItem();
  std::string s;
  AH_CUSTOMER *cu;

  if (lvi==0) {
  }
  s=QBanking::QStringToUtf8String(lvi->text(0));
  cu=AH_User_FindCustomer(_user, s.c_str());
  assert(cu);

  EditCustomer w(_hbci, cu, this, "EditCustomer", true);
  w.show(); // necessary for qt4
  if (w.exec()==QDialog::Accepted)
    updateLists();

}




