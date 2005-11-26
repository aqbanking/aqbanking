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


#include "a_mkkeys.h"
#include "wizard.h"
#include "mkkeys.h"

#include <qbanking/qbanking.h>

#include <aqhbci/outbox.h>
#include <aqhbci/adminjobs.h>

#include <gwenhywfar/debug.h>

#include <qmessagebox.h>

#include <assert.h>



ActionMakeKeys::ActionMakeKeys(Wizard *w)
:WizardAction(w, "MakeKeys", QWidget::tr("Create User keys")) {
  _makeKeysDialog=new MakeKeys(w, this, "MakeKeys");
  addWidget(_makeKeysDialog);
  _makeKeysDialog->show();
}



ActionMakeKeys::~ActionMakeKeys() {
}



bool ActionMakeKeys::apply() {
  return _makeKeysDialog->getResult();
}






