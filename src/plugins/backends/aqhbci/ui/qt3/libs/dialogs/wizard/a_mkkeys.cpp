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
#include "actionwidget.h"

#include <qpushbutton.h>

#include <qbanking/qbanking.h>
#include <aqhbci/provider.h>
#include <gwenhywfar/debug.h>

#include <assert.h>



ActionCreateKeys::ActionCreateKeys(Wizard *w)
:WizardAction(w, "CreateKeys", QWidget::tr("Create User Keys")) {
  _realDialog=new ActionWidget
    (tr("<qt>"
        "We will now create your keys."
        "</qt>"),
     tr("<qt>"
        "</qt>"),
     tr("Create User Keys"),
     this, "CreateKeys");
  _realDialog->setStatus(ActionWidget::StatusNone);
  connect(_realDialog->getButton(), SIGNAL(clicked()),
          this, SLOT(slotButtonClicked()));

  addWidget(_realDialog);
  _realDialog->show();
  setNextEnabled(false);
}



ActionCreateKeys::~ActionCreateKeys() {
}



void ActionCreateKeys::enter() {
  setNextEnabled(false);
  _realDialog->setStatus(ActionWidget::StatusNone);
}



bool ActionCreateKeys::apply() {
  return _realDialog->getStatus()==ActionWidget::StatusSuccess;
}



void ActionCreateKeys::slotButtonClicked() {
  WizardInfo *wi;
  AB_USER *u;
  AH_MEDIUM *m;
  int rv;

  wi=getWizard()->getWizardInfo();
  assert(wi);
  u=wi->getUser();
  assert(u);
  m=wi->getMedium();
  assert(m);

  _realDialog->setStatus(ActionWidget::StatusChecking);

  /* mount medium (if necessary) */
  if (!AH_Medium_IsMounted(m)) {
    rv=AH_Medium_Mount(m);
    if (rv) {
      DBG_ERROR(0, "Could not mount medium (%d)", rv);
      _realDialog->setStatus(ActionWidget::StatusFailed);
      return;
    }
  }

  /* select context of the user */
  rv=AH_Medium_SelectContext(m, AH_User_GetContextIdx(u));
  if (rv) {
    DBG_ERROR(0, "Could not select context (%d)", rv);
    _realDialog->setStatus(ActionWidget::StatusFailed);
    return;
  }

  rv=AH_Medium_CreateKeys(m);
  if (rv) {
    DBG_ERROR(0, "Could not create keys (%d)", rv);
    _realDialog->setStatus(ActionWidget::StatusFailed);
    return;
  }


  _realDialog->setStatus(ActionWidget::StatusSuccess);
  setNextEnabled(true);
}





