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


#include "a_getkeys.h"
#include "wizard.h"
#include "getkeys.h"

#include <qbanking/qbanking.h>

#include <aqhbci/outbox.h>
#include <aqhbci/adminjobs.h>

#include <gwenhywfar/debug.h>

#include <qmessagebox.h>

#include <assert.h>



ActionGetKeys::ActionGetKeys(Wizard *w)
:WizardAction(w, "GetKeys", QWidget::tr("Retrieve Server Keys")) {
  _getKeysDialog=new GetKeys(w, this, "GetKeys");
  addWidget(_getKeysDialog);
  _getKeysDialog->show();
}



ActionGetKeys::~ActionGetKeys() {
}



bool ActionGetKeys::apply() {
  return _getKeysDialog->getResult();
}






