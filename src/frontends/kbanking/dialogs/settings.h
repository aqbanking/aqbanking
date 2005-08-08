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

#ifndef AQBANKING_KDE_SETTINGS_H
#define AQBANKING_KDE_SETTINGS_H



#include "settings.ui.h"
#include "kbaccountlist.h"
#include "kbplugindescrlist.h"

#include "kbanking.h"



class KBankingSettings: public KBankingSettingsUi {
  Q_OBJECT
private:
  KBanking *_banking;
  KBAccountListView *_accListView;
  KBPluginDescrListView *_providerListView;

  void _accountRescan();
  void _backendRescan();

public:
  KBankingSettings(KBanking *ab,
                   QWidget* parent = 0,
                   const char* name = 0, WFlags fl = 0);
  ~KBankingSettings();

  int init();
  int fini();

public slots:
  void slotAccountMap();
  void slotBackendEnable();
  void slotBackendDisable();
  void slotBackendSetup();
};










#endif /* AQBANKING_KDE_SETTINGS_H */
