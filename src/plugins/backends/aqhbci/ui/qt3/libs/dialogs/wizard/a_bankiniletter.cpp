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


#include "a_bankiniletter.h"
#include "wizard.h"
#include "iniletter.h"

#include <qbanking/qbanking.h>

#include <qpushbutton.h>
#include <qtextview.h>

#include <gwenhywfar/debug.h>



ActionBankIniLetter::ActionBankIniLetter(Wizard *w)
:WizardAction(w, "BankIniLetter", QWidget::tr("Verify Bank Key"))
,_key(0) {
  _iniLetterDialog=new IniLetter(false,
                                 this,
                                 "IniLetterDialog");
  addWidget(_iniLetterDialog);
  _iniLetterDialog->show();

  connect(_iniLetterDialog->goodHashButton, SIGNAL(clicked()),
          this, SLOT(slotGoodHash()));
  connect(_iniLetterDialog->badHashButton, SIGNAL(clicked()),
          this, SLOT(slotBadHash()));
  connect(_iniLetterDialog->printButton, SIGNAL(clicked()),
          this, SLOT(slotPrint()));
}



ActionBankIniLetter::~ActionBankIniLetter() {
  if (_key)
    GWEN_CryptKey_free(_key);
}



void ActionBankIniLetter::enter() {
  Wizard *w;
  WizardInfo *wi;
  AB_USER *u;
  AH_MEDIUM *m;
  int rv;
  GWEN_CRYPTKEY *key;

  setNextEnabled(false);
  w=getWizard();
  wi=w->getWizardInfo();
  u=wi->getUser();
  m=wi->getMedium();

  /* mount medium (if necessary) */
  if (!AH_Medium_IsMounted(m)) {
    rv=AH_Medium_Mount(m);
    if (rv) {
      DBG_ERROR(0, "Could not mount medium (%d)", rv);
      return;
    }
  }

  /* select context of the user */
  rv=AH_Medium_SelectContext(m, AH_User_GetContextIdx(u));
  if (rv) {
    DBG_ERROR(0, "Could not select context (%d)", rv);
    return;
  }

  /* get key */
  key=AH_Medium_GetPubSignKey(m);
  if (!key)
    key=AH_Medium_GetPubCryptKey(m);
  assert(key);

  if (!_iniLetterDialog->init(QString::fromUtf8(wi->getBankId().c_str()),
                              key)) {
    DBG_ERROR(0, "Could not init dialog");
    GWEN_CryptKey_free(key);
    return;
  }
  _key=key;
}



void ActionBankIniLetter::slotGoodHash() {
  setNextEnabled(true);
}



void ActionBankIniLetter::slotBadHash() {
}



void ActionBankIniLetter::slotPrint() {
  int rv;

  rv=getWizard()->getBanking()->print(tr("Bank's Ini Letter"),
                                      "BANK::INILETTER",
                                      tr("This page contains the bank's "
                                         "iniletter."),
                                      _iniLetterDialog->iniBrowser->text());
  if (rv) {
    DBG_ERROR(0, "Could not print iniletter (%d)", rv);
  }
}













