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


#ifndef AQHBCI_EDITCTUSER_H
#define AQHBCI_EDITCTUSER_H

#include "editctuser.ui.h"
#include "winfo.h"


class QBanking;


class EditCtUser: public EditCtUserUi {
  Q_OBJECT
private:
  QBanking *_app;
  WizardInfo *_wInfo;
  AB_BANKINFO *_bankInfo;
  bool _dataIsOk;

  QString _getServerAddr() const;
  AH_CRYPT_MODE _getCryptMode(AH_MEDIUM *m, int idx);
  void _fromContext(int i);

  bool _checkStringSanity(const char *s);

public:
  EditCtUser(QBanking *qb,
             WizardInfo *wi,
             QWidget* parent=0,
             const char* name=0,
             bool modal=FALSE,
             WFlags fl=0);
  ~EditCtUser();

  void init();

  bool apply();
  bool undo();

public slots:
  void slotBankCodeLostFocus();
  void slotBankCodeChanged(const QString&);
  void slotBankCodeClicked();
  void slotContextActivated(int i);

};


#endif

