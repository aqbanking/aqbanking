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


#ifndef AQHBCI_A_GETCERT_H
#define AQHBCI_A_GETCERT_H

#include "waction.h"
#include "winfo.h"



class ActionGetCert: public WizardAction {
public:
  ActionGetCert(Wizard *w);
  virtual ~ActionGetCert();

  virtual bool apply();

};


#endif
