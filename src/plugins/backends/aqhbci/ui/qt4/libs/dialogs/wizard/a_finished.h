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


#ifndef AQHBCI_A_FINISHED_H
#define AQHBCI_A_FINISHED_H

#include "waction.h"
#include "winfo.h"



class ActionFinished: public WizardAction {
public:
  ActionFinished(Wizard *w);
  virtual ~ActionFinished();
};


#endif
