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


#ifndef AQHBCI_W_PINTAN_NEW_H
#define AQHBCI_W_PINTAN_NEW_H

#include "wizard.h"


class WizardPinTanNew: public Wizard {
private:

public:
  WizardPinTanNew(QBanking *qb,
                  WizardInfo *wInfo,
                  QWidget* parent=0, const char* name=0,
                  bool modal=FALSE);
  virtual ~WizardPinTanNew();

  virtual int exec();

};



#endif

