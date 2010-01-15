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


#ifndef AQHBCI_W_RDH_NEW2_H
#define AQHBCI_W_RDH_NEW2_H

#include "wizard.h"


class WizardRdhNew2: public Wizard {
private:

public:
  WizardRdhNew2(QBanking *qb,
                WizardInfo *wInfo,
                QWidget* parent=0, const char* name=0,
                bool modal=FALSE);
  virtual ~WizardRdhNew2();

  virtual int exec();

};



#endif

