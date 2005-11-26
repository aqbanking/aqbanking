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


#ifndef AQHBCI_A_MKKEYS_H
#define AQHBCI_A_MKKEYS_H

#include "waction.h"
#include "winfo.h"

class MakeKeys;


class ActionMakeKeys: public WizardAction {
private:
  MakeKeys *_makeKeysDialog;
public:
  ActionMakeKeys(Wizard *w);
  virtual ~ActionMakeKeys();

  virtual bool apply();

};


#endif
