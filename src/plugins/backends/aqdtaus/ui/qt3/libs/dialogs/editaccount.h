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


#ifndef AQDTAUS_EDITACCOUNT_H
#define AQDTAUS_EDITACCOUNT_H

#include "editaccount.ui.h"
#include <aqdtaus/account.h>

#include <qbanking/qbanking.h>


class EditAccount : public EditAccountUi {
  Q_OBJECT
private:
  QBanking *_app;
  AB_ACCOUNT *_account;
  bool _isNew;

  void accountToGui(AB_ACCOUNT *a);
  void guiToAccount(AB_ACCOUNT *a);

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
  void slotFloppyToggled(bool on);
  void slotMountToggled(bool on);
  void slotBankCodeLostFocus();
  void slotFolderLostFocus();
  void slotWhatsThis();
  void slotFolder();


};






#endif // AQDTAUS_EDITACCOUNT_H

