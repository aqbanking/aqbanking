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


#include "cfgtabpageuseryn.h"
#include "cfgtabpageuseryn.ui.h"

#include <aqyellownet/user.h>
#include <aqyellownet/provider.h>

#include <qbanking/qbanking.h>
#include <qbanking/qbcfgtab.h>
#include <qbanking/qbcfgtabuser.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/url.h>

#include <qmessagebox.h>
#include <qtimer.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcombobox.h>




CfgTabPageUserYn::CfgTabPageUserYn(QBanking *qb,
                                   AB_USER *u,
                                   QWidget *parent,
                                   const char *name, WFlags f)
:QBCfgTabPageUser(qb, "Yellownet", u, parent, name, f) {
  _realPage=new CfgTabPageUserYnUi(this);

  setHelpSubject("CfgTabPageUserYn");
  setDescription(tr("<p>This page contains "
                    "Yellownet-specific settings.</p>"));

  setUserIdInfo(tr("Yellownet Number"),
                tr("Enter your Yellownet number"));
  setCustomerIdInfo(tr("User Id"),
                    tr("This is only available for customers with "
                       "multiple user ids or with business accounts"));

  addWidget(_realPage);
  _realPage->show();

  QTimer::singleShot(0, this, SLOT(adjustSize()));
}



CfgTabPageUserYn::~CfgTabPageUserYn() {
}



bool CfgTabPageUserYn::fromGui() {
  AB_USER *u;
  AY_USER_LANGUAGE l;

  u=getUser();
  assert(u);

  switch(_realPage->languageCombo->currentItem()) {
  case 1: l=AY_User_LanguageFrench; break;
  case 2: l=AY_User_LanguageItalian; break;
  case 3: l=AY_User_LanguageEnglish; break;
  case 0:
  default:
    l=AY_User_LanguageGerman; break;
  }

  return true;
}



bool CfgTabPageUserYn::toGui() {
  AB_USER *u;

  u=getUser();
  assert(u);

  switch(AY_User_GetLanguage(u)) {
  case AY_User_LanguageFrench:
    _realPage->languageCombo->setCurrentItem(1);
    break;
  case AY_User_LanguageItalian:
    _realPage->languageCombo->setCurrentItem(2);
    break;
  case AY_User_LanguageEnglish:
    _realPage->languageCombo->setCurrentItem(3);
    break;
  case AY_User_LanguageGerman:
  default:
    _realPage->languageCombo->setCurrentItem(0);
    break;
  }

  return true;
}



bool CfgTabPageUserYn::checkGui() {
  return true;
}



