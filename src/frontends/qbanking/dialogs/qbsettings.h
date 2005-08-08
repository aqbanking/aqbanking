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

#ifndef QBANKING_SETTINGS_H
#define QBANKING_SETTINGS_H



#include "qbsettings.ui.h"
#include "qbaccountlist.h"
#include "qbplugindescrlist.h"

#include "qbanking.h"



class QBankingSettings: public QBankingSettingsUi {
  Q_OBJECT
private:
  QBanking *_banking;
  QBAccountListView *_accListView;
  QBPluginDescrListView *_providerListView;

  void _accountRescan();
  void _backendRescan();

public:
  QBankingSettings(QBanking *ab,
                   QWidget* parent = 0,
                   const char* name = 0, WFlags fl = 0);
  ~QBankingSettings();

  int init();
  int fini();

public slots:
  void slotAccountMap();
  void slotBackendEnable();
  void slotBackendDisable();
  void slotBackendSetup();
};










#endif /* QBANKING_SETTINGS_H */
