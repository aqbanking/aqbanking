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
#include "actionwidget.h"

#include <qpushbutton.h>

#include <qbanking/qbanking.h>
#include <aqhbci/provider.h>
#include <gwenhywfar/debug.h>

#include <assert.h>



ActionGetKeys::ActionGetKeys(Wizard *w)
:WizardAction(w, "GetKeys", QWidget::tr("Retrieve Server Keys")) {
  _realDialog=new ActionWidget
    (tr("<qt>"
        "We will now retrieve the public keys of the bank server."
        "</qt>"),
     tr("<qt>"
        "There are two keys to be retrieved:"
        "<ul>"
        "<li>"
        "<b>Signature key:</b> "
        "This key is used by the server to sign all messages sent to us. "
        "Please note that some institutes do not use a signature key. If "
        "they do not use a signature key, there is no proof of whether a "
        "received message was sent to us by the bank as opposed to someone "
        "else."
        "</li>"
        "<li>"
        "<b>Crypt key:</b> This key is used by this program to encrypt data "
        "prior to sending it to the server. This way the server is the only "
        "subject able to decrypt our messages. "
        "</li>"
        "</ul>"
        "When you press the button below the procedure starts. That will "
        "open a window showing the progress in communication with the server."
        "</qt>"),
     tr("Get Server Keys"),
     this, "GetKeys");
  _realDialog->setStatus(ActionWidget::StatusNone);
  connect(_realDialog->getButton(), SIGNAL(clicked()),
          this, SLOT(slotButtonClicked()));

  addWidget(_realDialog);
  _realDialog->show();
  setNextEnabled(false);
}



ActionGetKeys::~ActionGetKeys() {
}



void ActionGetKeys::enter() {
  setNextEnabled(false);
  _realDialog->setStatus(ActionWidget::StatusNone);
}



bool ActionGetKeys::apply() {
  return _realDialog->getStatus()==ActionWidget::StatusSuccess;
}



void ActionGetKeys::slotButtonClicked() {
  WizardInfo *wInfo;
  QBanking *qb;
  AB_USER *u;
  AB_PROVIDER *pro;
  GWEN_TYPE_UINT32 pid;
  int rv;
  AB_IMEXPORTER_CONTEXT *ctx;

  wInfo=getWizard()->getWizardInfo();
  assert(wInfo);
  u=wInfo->getUser();
  assert(u);
  qb=getWizard()->getBanking();
  assert(qb);
  pro=AH_HBCI_GetProvider(wInfo->getHbci());
  assert(pro);

  _realDialog->setStatus(ActionWidget::StatusChecking);

  pid=qb->progressStart(tr("Getting Server Keys"),
                        tr("<qt>"
                           "Retrieving the public keys of the server."
                           "</qt>"),
                        1);
  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetServerKeys(pro, u, ctx, 1);
  AB_ImExporterContext_free(ctx);
  if (rv) {
    DBG_ERROR(0, "Error getting server keys");
    _realDialog->setStatus(ActionWidget::StatusFailed);
    qb->progressEnd(pid);
    return;
  }

  qb->progressEnd(pid);

  _realDialog->setStatus(ActionWidget::StatusSuccess);
  setNextEnabled(true);
}





