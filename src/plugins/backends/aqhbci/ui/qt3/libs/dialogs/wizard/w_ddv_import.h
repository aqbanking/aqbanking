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


#ifndef AQHBCI_W_DDV_IMPORT_H
#define AQHBCI_W_DDV_IMPORT_H

#include "wizard.h"


class WizardDdvImport: public Wizard {
private:

public:
  WizardDdvImport(QBanking *qb,
                  WizardInfo *wInfo,
                  QWidget* parent=0, const char* name=0,
                  bool modal=FALSE);
  virtual ~WizardDdvImport();

  virtual int exec();

};



#endif

