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


#ifndef AQHBCI_A_WAIT_H
#define AQHBCI_A_WAIT_H

#include "waction.h"
#include "winfo.h"



class ActionWait: public WizardAction {
public:
  ActionWait(Wizard *w);
  virtual ~ActionWait();
};


#endif
