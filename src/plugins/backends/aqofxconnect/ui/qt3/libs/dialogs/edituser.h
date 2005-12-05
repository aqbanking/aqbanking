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


#ifndef AQOFXCONNECT_EDITUSER_H
#define AQOFXCONNECT_EDITUSER_H

#include "edituser.ui.h"
#include <aqofxconnect/user.h>

#include <qbanking/qbanking.h>

class QComboBox;


class EditUser : public EditUserUi {
  Q_OBJECT
private:
  QBanking *_app;
  AO_USER *_user;
  bool _isNew;

  void userToGui(AO_USER *u);
  void guiToUser(AO_USER *u);

  void countriesToCombo(QComboBox *qc, const char *c);
  std::string comboToCountry(QComboBox *qc);

public:
  EditUser(QBanking *app,
           AO_USER *u,
           bool isNew,
           QWidget* parent=0, const char* name=0,
           bool modal=FALSE, WFlags fl=0);
  virtual ~EditUser();

  bool init();

public slots:
  void accept();
  void slotBankCodeLostFocus();
  void slotBankCodeClicked();
  void slotWhatsThis();


};






#endif // AQOFXCONNECT_EDITUSER_H

