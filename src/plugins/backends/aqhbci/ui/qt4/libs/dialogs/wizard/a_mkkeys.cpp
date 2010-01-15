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

#include <q4banking/qbanking.h>
#include <aqhbci/provider.h>
#include <aqhbci/user.h>
#include <gwenhywfar/debug.h>

#include <assert.h>



ActionCreateKeys::ActionCreateKeys(Wizard *w)
:WizardAction(w, "CreateKeys", QWidget::tr("Create User Keys")) {
  _realDialog=new ActionWidget
    (tr("<qt>"
        "We will now create your keys."
        "</qt>"),
     tr("<qt>"
	"<font colour=red size=+2>Warning!</font><br>"
	"<p>"
	"Please do not create keys if the bank already has keys from you! "
	"This would be the case if you already used this security medium "
	"before (even with other programs)."
	"</p>"
	"<p>"
	"In such a case you would destroy the keys and thus be unable to "
	"communicate with your bank any further."
	"</p>"
        "<p>"
	"There are two keys to be created:"
	"</p>"
	"<ul>"
        "<li>"
        "<b>Signature key:</b> "
	"This key is used by to sign all messages you send to the server. "
	"This makes sure that the bank only processes requests which have really "
	"been sent by <b>you</b>"
	"</li>"
	"<li>"
	"<b>Crypt key:</b> This key is used by the bank to encrypt messages "
	"prior to sending them to you. This way only <b>you</b> can decrypt the "
	"messages. "
	"</li>"
        "</ul>"
        "When you press the button below the procedure starts. That will "
        "open a window showing the progress in communication with the server."
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
  int rv;

  wi=getWizard()->getWizardInfo();
  assert(wi);
  u=wi->getUser();
  assert(u);

  _realDialog->setStatus(ActionWidget::StatusChecking);

  rv=AH_Provider_CreateKeys(wi->getProvider(),
			    u, 1, 0);
  if (rv) {
    DBG_ERROR(0, "Could not create keys (%d)", rv);
    _realDialog->setStatus(ActionWidget::StatusFailed);
    return;
  }

  _realDialog->setStatus(ActionWidget::StatusSuccess);
  setNextEnabled(true);
}



#include "a_mkkeys.moc"

