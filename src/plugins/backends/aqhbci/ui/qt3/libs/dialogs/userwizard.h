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

#include <aqhbci/hbci.h>


class UserWizard {
private:
  QBanking *_app;
  AH_HBCI *_hbci;
  QWidget *_parent;

  bool _checkAndCreateMedium(WizardInfo *wInfo,
                             GWEN_CRYPTTOKEN_DEVICE dev);
  bool _handleModePinTan();
  bool _handleModeImportCard();

public:
  UserWizard(QBanking *qb, AH_HBCI *hbci, QWidget *parent=0);
  ~UserWizard();

  bool exec();

};



#endif // AQHBCI_USERWIZARD_H
