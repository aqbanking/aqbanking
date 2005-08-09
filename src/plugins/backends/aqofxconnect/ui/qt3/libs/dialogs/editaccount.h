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


#ifndef AQOFXCONNECT_EDITACCOUNT_H
#define AQOFXCONNECT_EDITACCOUNT_H

#include "editaccount.ui.h"
#include <aqofxconnect/account.h>

#include <qbanking/qbanking.h>

class QComboBox;


class EditAccount : public EditAccountUi {
  Q_OBJECT
private:
  QBanking *_app;
  AB_ACCOUNT *_account;
  bool _isNew;

  void accountToGui(AB_ACCOUNT *a);
  void guiToAccount(AB_ACCOUNT *a);

  void countriesToCombo(QComboBox *qc, const char *c);
  void usersToCombo(QComboBox *qc,
                    const char *country,
                    const char *bankCode,
                    const char *userId);

public:
  EditAccount(QBanking *app,
              AB_ACCOUNT *a,
              bool isNew,
              QWidget* parent=0, const char* name=0,
              bool modal=FALSE, WFlags fl=0);
  virtual ~EditAccount();

  bool init();

public slots:
  void accept();
  void slotBankCodeLostFocus();
  void slotWhatsThis();


};






#endif // AQOFXCONNECT_EDITACCOUNT_H

