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
#include <aqhbci/user.h>

#include <qbanking/qbanking.h>

#include <qpushbutton.h>
#include <qtextview.h>

#include <gwenhywfar/debug.h>



ActionUserIniLetter::ActionUserIniLetter(Wizard *w)
:WizardAction(w, "UserIniLetter", QWidget::tr("User's Ini Letter")){
  _iniLetterDialog=new IniLetter(true,
                                 w->getWizardInfo()->getProvider(),
				 this,
                                 "IniLetterDialog");
  addWidget(_iniLetterDialog);
  _iniLetterDialog->show();

  connect(_iniLetterDialog->printButton, SIGNAL(clicked()),
          this, SLOT(slotPrint()));
}



ActionUserIniLetter::~ActionUserIniLetter() {
}



void ActionUserIniLetter::enter() {
  Wizard *w;
  WizardInfo *wi;
  AB_USER *u;

  setNextEnabled(false);
  w=getWizard();
  wi=w->getWizardInfo();
  u=wi->getUser();
  assert(u);

  if (!_iniLetterDialog->init(u)) {
    DBG_ERROR(0, "Could not init dialog");
    setNextEnabled(false);
    return;
  }
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




#include "a_useriniletter.moc"








