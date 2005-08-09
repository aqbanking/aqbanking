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


#ifndef AQHBCI_EDITCUSTOMER_H
#define AQHBCI_EDITCUSTOMER_H

#include "editcustomer.ui.h"
#include <aqhbci/hbci.h>
#include <aqhbci/bank.h>
#include <aqhbci/customer.h>

class QComboBox;


class EditCustomer : public EditCustomerUi {
  Q_OBJECT
private:
  AH_HBCI *_hbci;
  AH_CUSTOMER *_customer;
  bool _withHttp;
public:
  EditCustomer(AH_HBCI *h,
               AH_CUSTOMER *c,
               QWidget* parent=0, const char* name=0,
               bool modal=FALSE, WFlags fl=0);
  virtual ~EditCustomer();

  bool init();
  bool fini();

public slots:
  void accept();
  void slotGetServerKeys();
  void slotGetSysId();

private:
  void _setComboTextIfPossible(QComboBox *qb, const QString &s);


};






#endif // AQHBCI_EDITCUSTOMER_H

