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


#ifndef AQHBCI_A_EDITUSER_H
#define AQHBCI_A_EDITUSER_H

#include "waction.h"
#include "winfo.h"

class EditCtUser;


class ActionEditUser: public WizardAction {
private:
  EditCtUser *_editUserDialog;
public:
  ActionEditUser(Wizard *w);
  virtual ~ActionEditUser();

  virtual bool apply();
  virtual bool undo();

};


#endif
