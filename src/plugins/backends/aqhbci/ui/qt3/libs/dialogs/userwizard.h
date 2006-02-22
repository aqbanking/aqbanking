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


#ifndef AQHBCI_USERWIZARD_H
#define AQHBCI_USERWIZARD_H


class QWidget;
class QBanking;
class WizardInfo;

#include <aqbanking/provider.h>


class UserWizard {
private:
  QBanking *_app;
  AB_PROVIDER *_provider;
  QWidget *_parent;

  bool _checkAndCreateMedium(WizardInfo *wInfo);
  bool _handleModePinTan();
  bool _handleModeImportCard();
  bool _handleModeImportFile();
  bool _handleModeCreateFile();

public:
  UserWizard(QBanking *qb, AB_PROVIDER *pro, QWidget *parent);
  ~UserWizard();

  static bool finishUser(QBanking *qb,
                         AB_PROVIDER *pro,
                         AB_USER *u,
                         QWidget *parent);

  bool exec();

};



#endif // AQHBCI_USERWIZARD_H
