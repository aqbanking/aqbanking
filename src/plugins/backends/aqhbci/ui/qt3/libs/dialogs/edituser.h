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


#ifndef AQHBCI_EDITUSER_H
#define AQHBCI_EDITUSER_H

#include "edituser.ui.h"
#include <aqhbci/hbci.h>
#include <aqhbci/bank.h>
#include <aqhbci/customer.h>

class QComboBox;


class EditUser : public EditUserUi {
  Q_OBJECT
private:
  AH_HBCI *_hbci;
  AH_USER *_user;

public:
  EditUser(AH_HBCI *h,
	   AH_USER *u,
	   QWidget* parent=0, const char* name=0,
	   bool modal=FALSE, WFlags fl=0);
  virtual ~EditUser();

  bool init();
  bool fini();

public slots:
  void slotStatusChanged(int i);
  void slotEditCustomer();
  void accept();

private:
  void _setComboTextIfPossible(QComboBox *qb, const QString &s);

protected:
  void updateLists();

};






#endif // AQHBCI_EDITUSER_H

