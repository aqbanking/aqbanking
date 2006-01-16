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


#include "a_useriniletter.h"
#include "wizard.h"
#include "iniletter.h"

#include <aqhbci/provider.h>

#include <qbanking/qbanking.h>

#include <qpushbutton.h>
#include <qtextview.h>

#include <gwenhywfar/debug.h>



ActionUserIniLetter::ActionUserIniLetter(Wizard *w)
:WizardAction(w, "UserIniLetter", QWidget::tr("User's Iniletter"))
,_key(0) {
  _iniLetterDialog=new IniLetter(true,
                                 this,
                                 "IniLetterDialog");
  addWidget(_iniLetterDialog);
  _iniLetterDialog->show();

  connect(_iniLetterDialog->printButton, SIGNAL(clicked()),
          this, SLOT(slotPrint()));
}



ActionUserIniLetter::~ActionUserIniLetter() {
  if (_key)
    GWEN_CryptKey_free(_key);
}



void ActionUserIniLetter::enter() {
  Wizard *w;
  WizardInfo *wi;
  AB_USER *u;
  AH_MEDIUM *m;
  int rv;
  GWEN_CRYPTKEY *key;
  AB_PROVIDER *pro;
  const char *s;
  QString userName;
  QString userId;
  QString appName;

  w=getWizard();
  wi=w->getWizardInfo();
  u=wi->getUser();
  m=wi->getMedium();
  pro=AH_HBCI_GetProvider(wi->getHbci());
  assert(pro);

  /* mount medium (if necessary) */
  if (!AH_Medium_IsMounted(m)) {
    rv=AH_Medium_Mount(m);
    if (rv) {
      DBG_ERROR(0, "Could not mount medium (%d)", rv);
      setNextEnabled(false);
      return;
    }
  }

  /* select context of the user */
  rv=AH_Medium_SelectContext(m, AH_User_GetContextIdx(u));
  if (rv) {
    DBG_ERROR(0, "Could not select context (%d)", rv);
    setNextEnabled(false);
    return;
  }

  /* get key */
  key=AH_Medium_GetLocalPubSignKey(m);
  assert(key);

  s=AB_User_GetUserName(u);
  if (s)
    userName=QString::fromUtf8(s);
  s=AB_User_GetUserId(u);
  if (s)
    userId=QString::fromUtf8(s);

  s=AH_Provider_GetProductName(pro);
  if (s)
    appName=QString::fromUtf8(s);

  if (!_iniLetterDialog->init(userName,
                              userId,
                              appName,
                              key)) {
    DBG_ERROR(0, "Could not init dialog");
    GWEN_CryptKey_free(key);
    setNextEnabled(false);
    return;
  }
  _key=key;
  setNextEnabled(true);
}



void ActionUserIniLetter::slotPrint() {
  int rv;

  rv=getWizard()->getBanking()->print(tr("User's Ini Letter"),
                                      QString("USER::INILETTER"),
                                      tr("This page contains the user's "
                                         "iniletter."),
                                      _iniLetterDialog->iniBrowser->text());
  if (rv) {
    DBG_ERROR(0, "Could not print iniletter (%d)", rv);
  }
}













