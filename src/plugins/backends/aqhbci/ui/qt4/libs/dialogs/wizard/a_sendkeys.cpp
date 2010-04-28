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


#include "a_sendkeys.h"
#include "wizard.h"
#include "actionwidget.h"

#include <qpushbutton.h>

#include <q4banking/qbanking.h>
#include <aqhbci/provider.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>

#include <assert.h>



ActionSendKeys::ActionSendKeys(Wizard *w)
:WizardAction(w, "SendKeys", QWidget::tr("Send Public Keys")) {
  _realDialog=new ActionWidget
    (tr("<qt>"
        "We will now send your public keys to the bank server."
        "</qt>"),
     tr("<qt>"
        "There are two keys to be sent:"
        "<ul>"
        "<li>"
        "<b>Signature key:</b> "
        "This key is used to sign message sent to the server."
        "</li>"
        "<li>"
        "<b>Crypt key:</b> This key is used by the server to encrypt "
        "its messages. This way only we are able to decrypt messages "
        "received from the server."
        "</li>"
        "</ul>"
        "When you press the button below the procedure starts. That will "
        "open a window showing the progress in communication with the server."
        "</qt>"),
     tr("Send User Keys"),
     this, "SendKeys");
  _realDialog->setStatus(ActionWidget::StatusNone);
  connect(_realDialog->getButton(), SIGNAL(clicked()),
          this, SLOT(slotButtonClicked()));

  addWidget(_realDialog);
  _realDialog->show();
  setNextEnabled(false);
}



ActionSendKeys::~ActionSendKeys() {
}



void ActionSendKeys::enter() {
  setNextEnabled(false);
  _realDialog->setStatus(ActionWidget::StatusNone);
}



bool ActionSendKeys::apply() {
  return _realDialog->getStatus()==ActionWidget::StatusSuccess;
}



void ActionSendKeys::slotButtonClicked() {
  WizardInfo *wInfo;
  QBanking *qb;
  AB_USER *u;
  AB_PROVIDER *pro;
  uint32_t pid;
  int rv;
  AB_IMEXPORTER_CONTEXT *ctx;

  wInfo=getWizard()->getWizardInfo();
  assert(wInfo);
  u=wInfo->getUser();
  assert(u);
  qb=getWizard()->getBanking();
  assert(qb);
  pro=wInfo->getProvider();
  assert(pro);

  _realDialog->setStatus(ActionWidget::StatusChecking);

  pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_ALLOW_SUBLEVELS |
			     GWEN_GUI_PROGRESS_SHOW_PROGRESS |
			     GWEN_GUI_PROGRESS_KEEP_OPEN |
			     GWEN_GUI_PROGRESS_SHOW_ABORT,
			     tr("Sending User Keys").utf8(),
			     NULL,
			     GWEN_GUI_PROGRESS_NONE,
			     0);

  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_SendUserKeys(pro, u, ctx, 1, 1, 1);
  GWEN_Gui_ProgressEnd(pid);
  AB_ImExporterContext_free(ctx);
  if (rv) {
    DBG_ERROR(0, "Error sending user keys");
    _realDialog->setStatus(ActionWidget::StatusFailed);
    return;
  }

  _realDialog->setStatus(ActionWidget::StatusSuccess);
  setNextEnabled(true);
}



#include "a_sendkeys.moc"


