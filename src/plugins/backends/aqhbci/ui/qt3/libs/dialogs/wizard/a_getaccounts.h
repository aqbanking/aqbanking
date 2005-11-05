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


#ifndef AQHBCI_A_GETACCOUNTS_H
#define AQHBCI_A_GETACCOUNTS_H

#include "waction.h"
#include "winfo.h"



class ActionGetAccounts: public WizardAction {
public:
  ActionGetAccounts(Wizard *w);
  virtual ~ActionGetAccounts();

  virtual bool apply();

};


#endif
