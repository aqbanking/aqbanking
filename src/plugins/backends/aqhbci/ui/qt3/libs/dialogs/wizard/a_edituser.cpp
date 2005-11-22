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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "a_edituser.h"
#include "wizard.h"
#include "editctuser.h"

#include <qlayout.h>



ActionEditUser::ActionEditUser(Wizard *w)
:WizardAction(w, "EditUser", QWidget::tr("Edit user settings")) {
  _editUserDialog=new EditCtUser(w->getBanking(),
                                 w->getWizardInfo(),
                                 this,
                                 "EditCtUser");
  addWidget(_editUserDialog);
  _editUserDialog->show();
}



ActionEditUser::~ActionEditUser() {
}



bool ActionEditUser::apply() {
  return _editUserDialog->apply();
}



bool ActionEditUser::undo() {
  return _editUserDialog->undo();
}








