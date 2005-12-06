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

#include "w_rdh_new.h"
#include "a_edituser.h"
#include "a_getkeys.h"
#include "a_bank_iniletter.h"
#include "a_wait.h"



WizardRdhNew::WizardRdhNew(QBanking *qb,
                           WizardInfo *wInfo,
                           QWidget* parent, const char* name,
                           bool modal)
:Wizard(qb, wInfo,
        tr("Create a new RDH user"),
        parent, name, modal) {
  WizardAction *wa;

  setDescription(tr("<qt>"
                    "This wizard creates an user on a <b>RDH</b> medium."
                    "</qt>"));
  wa=new ActionEditUser(this);
  addAction(wa);

  wa=new ActionGetKeys(this);
  addAction(wa);

  wa=new ActionBankIniLetter(this);
  addAction(wa);

  wa=new ActionWait(this);
  addAction(wa);
}



WizardRdhNew::~WizardRdhNew() {

}



int WizardRdhNew::exec() {
  int rv;

  rv=Wizard::exec();
  if (rv==QDialog::Accepted) {
    AH_USER *u;

    u=getWizardInfo()->getUser();
    assert(u);
    /* create, set the user status to "pending" */
    AH_User_SetStatus(u, AH_UserStatusPending);
  }

  return rv;
}



