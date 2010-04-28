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


#include "a_getsysid.h"
#include "wizard.h"
#include "actionwidget.h"

#include <q4banking/qbanking.h>
#include <aqhbci/provider.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>

#include <qpushbutton.h>
#include <qmessagebox.h>

#include <assert.h>



ActionGetSysId::ActionGetSysId(Wizard *w)
:WizardAction(w, "GetSysId", QWidget::tr("Retrieve System Id")) {
  _realDialog=new ActionWidget
    (tr("<qt>"
        "We will now retrieve a system id for this application."
        "</qt>"),
     tr("<qt>"
        "<p>"
        "The system id is assigned to each library/application system "
        "(like AqBanking). This id is used by the bank to distinguish "
        "between signature counters used by different programs."
        "</p>"
        "<p>"
        "This allows you to access your accounts through multiple applications."
        "</p>"
        "</qt>"),
     tr("Get System Id"),
     this, "GetSysId");
  _realDialog->setStatus(ActionWidget::StatusNone);
  connect(_realDialog->getButton(), SIGNAL(clicked()),
          this, SLOT(slotButtonClicked()));

  addWidget(_realDialog);
  _realDialog->show();
}



ActionGetSysId::~ActionGetSysId() {
}



void ActionGetSysId::enter() {
  setNextEnabled(false);
  _realDialog->setStatus(ActionWidget::StatusNone);
}



bool ActionGetSysId::apply() {
  return true;
}



void ActionGetSysId::slotButtonClicked() {
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

  DBG_ERROR(0, "Retrieving system id");
  pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_ALLOW_SUBLEVELS |
			     GWEN_GUI_PROGRESS_SHOW_PROGRESS |
			     GWEN_GUI_PROGRESS_KEEP_OPEN |
			     GWEN_GUI_PROGRESS_SHOW_ABORT,
			     tr("Retrieving System Id").utf8(),
			     NULL,
			     GWEN_GUI_PROGRESS_NONE,
			     0);

  ctx=AB_ImExporterContext_new();
  rv=AH_Provider_GetSysId(pro, u, ctx, 1, 1, 1);
  GWEN_Gui_ProgressEnd(pid);
  AB_ImExporterContext_free(ctx);
  if (rv) {
    DBG_ERROR(0, "Error getting sysid (%d)", rv);
    _realDialog->setStatus(ActionWidget::StatusFailed);
    return;
  }

  _realDialog->setStatus(ActionWidget::StatusSuccess);
  setNextEnabled(true);
}



#include "a_getsysid.moc"


