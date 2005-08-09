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


#include "wizard.h"
#include <aqhbci/outbox.h>
#include <aqhbci/adminjobs.h>

#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qwizard.h>
#include <qcombobox.h>
#include <qtextbrowser.h>

#include <qlineedit.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qpalette.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qprinter.h>
#include <qsimplerichtext.h>
#include <qtextview.h>
#include <qlabel.h>

#include <gwenhywfar/debug.h>




bool Wizard::initVerifyKeysPage() {
  QObject::connect((QObject*)(serverKeysOkButton),
                   SIGNAL(clicked()),
                   this,
                   SLOT(slotKeysOk()));

  QObject::connect((QObject*)(serverKeysBadButton),
                   SIGNAL(clicked()),
                   this,
                   SLOT(slotKeysNotOk()));
  return true;
}







bool Wizard::enterVerifyKeysPage(QWidget *p){
  std::string hash;
  std::string modulus;
  std::string exponent;
  QString result;
  GWEN_CRYPTKEY *key;

  setBackEnabled(verifyKeysPage, false);
  setNextEnabled(verifyKeysPage, false);

  if (!AH_Medium_IsMounted(_medium)) {
    if (AH_Medium_Mount(_medium)) {
      DBG_ERROR(0, "Could not mount medium");
      return false;
    }
  }

  if (AH_Medium_SelectContext(_medium, AH_User_GetContextIdx(_user))) {
    DBG_ERROR(0, "Could not select user");
    QMessageBox::critical(0,
			  tr("Medium Error"),
			  tr("Could not select user context on medium.\n"
			     "Please check the logs."
			    ),
			  tr("Dismiss"),0,0,0);
    return false;
  }



  key=AH_Medium_GetPubSignKey(_medium);
  if (!key) {
    DBG_WARN(0, "No sign key");
    key=AH_Medium_GetPubCryptKey(_medium);
  }
  if (!key) {
    DBG_ERROR(0, "Neither public sign nor crypt key found.");
    return false;
  }

  modulus=_getIniLetterModulus(key);
  exponent=_getIniLetterExponent(key);
  hash=_getIniLetterHash(key);

  GWEN_CryptKey_free(key);

  result+="<qt>";
  result+="<h2>"+tr("Exponent")+"</h2>";
  if (exponent.length()<192) {
    DBG_ERROR(0, "Bad exponent");
    return false;
  }
  result+="<font face=fixed>";
  result+=_dumpHexString(exponent).c_str();
  result+="</font>";
  result+="<br>";

  result+="<h2>"+tr("Modulus")+"</h2>";
  if (exponent.length()<192) {
    DBG_ERROR(0, "Bad modulus");
    return false;
  }
  result+="<font face=fixed>";
  result+=_dumpHexString(modulus).c_str();
  result+="</font>";
  result+="<br>";

  result+="<h2>"+tr("Hash")+"</h2>";
  result+="<font face=fixed>";
  result+=_dumpHexString(hash, 40).c_str();
  result+="</font>";

  result+="<br>";

  result+="</qt>";

  serverIniBrowser->setText(result);
  return true;
}



bool Wizard::doVerifyKeysPage(QWidget *p){
  return true;
}



bool Wizard::undoVerifyKeysPage(QWidget *p){
  return true;
}



void Wizard::slotKeysOk(){
  setNextEnabled(verifyKeysPage, true);
  serverKeysOkButton->setEnabled(false);
  serverKeysBadButton->setEnabled(false);
}



void Wizard::slotKeysNotOk(){
  setNextEnabled(verifyKeysPage, false);
  //serverKeysOkButton->setEnabled(false);
  //serverKeysBadButton->setEnabled(false); 
  // dont deactivate this here -- maybe the user only pressed the wrong button.
  QMessageBox::critical(0,
			tr("Wrong Server Keys"),
			tr("You said the fingerprint of the server's cryptographic keys are wrong. In this case, you need to contact your bank and ask whether their server keys have changed. You should take some notes of the key fingerprint that is displayed right now. Then you need to abort this user setup for now."),
			tr("Dismiss"),0,0,0);
}


















