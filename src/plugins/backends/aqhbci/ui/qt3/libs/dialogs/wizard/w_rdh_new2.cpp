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

#include "w_rdh_new2.h"
#include "a_getsysid.h"
#include "a_finished.h"
#include "a_getaccounts.h"

#include <qtimer.h>



WizardRdhNew2::WizardRdhNew2(QBanking *qb,
                             WizardInfo *wInfo,
                             QWidget* parent, const char* name,
                             bool modal)
:Wizard(qb, wInfo,
        tr("Continues to create a new RDH user"),
        parent, name, modal) {
  WizardAction *wa;

  setDescription(tr("<qt>"
                    "This wizard continues to create a user on a <b>RDH</b> "
                    "medium."
                    "</qt>"));

  wa=new ActionGetSysId(this);
  addAction(wa);

  wa=new ActionGetAccounts(this);
  addAction(wa);

  wa=new ActionFinished(this);
  addAction(wa);

  QTimer::singleShot(0, this, SLOT(adjustSize()));
}



WizardRdhNew2::~WizardRdhNew2() {

}



int WizardRdhNew2::exec() {
  int rv;

  rv=Wizard::exec();
  if (rv==QDialog::Accepted) {
    AB_USER *u;

    u=getWizardInfo()->getUser();
    assert(u);
    /* create, set the user status to "pending" */
    AH_User_SetStatus(u, AH_UserStatusEnabled);
  }

  return rv;
}



