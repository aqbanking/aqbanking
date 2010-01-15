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


#ifndef AQHBCI_HBCISETTINGS_H
#define AQHBCI_HBCISETTINGS_H


#include "hbcisettings.ui.h"
#include "accountlist.h"
#include "userlist.h"

#include <aqhbci/hbci.h>
#include <string>
#include <list>


class QBanking;


class HBCISettings: public HBCISettingsUi {
  Q_OBJECT
private:
  AH_HBCI *_hbci;
  QBanking *_app;
  AccountListView *_accList;
  UserListView *_userList;

  bool _updateBPD(AH_CUSTOMER *cu);

public:
  HBCISettings(AH_HBCI *hbci,
               QBanking *kb,
               QWidget* parent=0, const char* name=0,
               bool modal=FALSE, WFlags fl=0);
  virtual ~HBCISettings();


public slots:
  void updateLists();

  void slotNewUser();
  void slotEditUser();
  void slotDelUser();
  void slotCompleteUser();
  void slotChangeVersion();
  void slotUpdateBPD();
  void slotUserSelectionChanged();
  void slotIniLetter();

  void slotNewAccount();
  void slotEditAccount();
  void slotDelAccount();

  void slotHelp();

};





#endif


