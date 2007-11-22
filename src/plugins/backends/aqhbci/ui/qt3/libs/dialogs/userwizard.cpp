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

#include "userwizard.h"
#include "selectmode.h"
#include "w_pintan_new.h"
#include "w_ddv_import.h"
#include "w_rdh_import.h"
#include "w_rdh_new.h"
#include "w_rdh_new2.h"

#include <aqhbci/provider.h>
#include <aqhbci/user.h>
#include <qbanking/qbanking.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>

#include <qmessagebox.h>


UserWizard::UserWizard(QBanking *qb, AB_PROVIDER *pro, QWidget *parent)
:_app(qb)
,_provider(pro)
,_parent(parent) {


}



UserWizard::~UserWizard() {
}



bool UserWizard::_handleModePinTan() {
  WizardPinTanNew *w;
  WizardInfo wInfo(_provider);

  /* create medium */
  wInfo.setCryptMode(AH_CryptMode_Pintan);

  /* setup user */
  w=new WizardPinTanNew(_app, &wInfo, _parent, "WizardPinTanNew", TRUE);

  if (w->exec()==QDialog::Accepted) {
    DBG_NOTICE(0, "Accepted");
    wInfo.setUser(NULL);
    wInfo.subFlags(WIZARDINFO_FLAGS_USER_CREATED);
  }
  else {
    DBG_NOTICE(0, "Rejected");
    wInfo.releaseData();
    return false;
  }

  return true;
}



bool UserWizard::_checkAndCreateMedium(WizardInfo *wInfo) {
  int rv;
  GWEN_BUFFER *mtypeName;
  GWEN_BUFFER *mediumName;
  QString txt;
  uint32_t pid;
  GWEN_CRYPT_TOKEN *ct;

  mtypeName=GWEN_Buffer_new(0, 64, 0, 1);
  mediumName=GWEN_Buffer_new(0, 64, 0, 1);

  txt=QWidget::trUtf8("<qt>"
                      "Checking type of the security medium, please wait..."
                      "</qt>");
  pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_ALLOW_SUBLEVELS |
			     GWEN_GUI_PROGRESS_SHOW_PROGRESS |
			     GWEN_GUI_PROGRESS_KEEP_OPEN |
			     GWEN_GUI_PROGRESS_SHOW_ABORT,
			     QWidget::tr("Checking Medium").utf8(),
			     txt.utf8(),
			     GWEN_GUI_PROGRESS_NONE,
			     0);

  rv=AH_Provider_CheckCryptToken(_provider,
				 GWEN_Crypt_Token_Device_Card,
				 mtypeName,
				 mediumName,
				 0);
  GWEN_Gui_ProgressEnd(pid);
  if (rv) {
    DBG_ERROR(0, "here (%d)", rv);
    GWEN_Buffer_free(mediumName);
    GWEN_Buffer_free(mtypeName);
    return false;
  }

  rv=AH_Provider_GetCryptToken(_provider,
			       GWEN_Buffer_GetStart(mtypeName),
			       GWEN_Buffer_GetStart(mediumName),
                               &ct);
  if (rv) {
    DBG_ERROR(0, "here (%d)", rv);
    GWEN_Buffer_free(mediumName);
    GWEN_Buffer_free(mtypeName);
    return false;
  }

  wInfo->setToken(ct);
  wInfo->setMediumType(GWEN_Crypt_Token_GetTypeName(ct));
  wInfo->setMediumName(GWEN_Crypt_Token_GetTokenName(ct));
  wInfo->addFlags(WIZARDINFO_FLAGS_MEDIUM_CREATED);

  GWEN_Buffer_free(mediumName);
  GWEN_Buffer_free(mtypeName);
  return true;
}



bool UserWizard::_handleModeImportCard() {
  Wizard *w;
  WizardInfo wInfo(_provider);
  const char *s;

  /* create medium */
  if (!_checkAndCreateMedium(&wInfo))
    return false;

  s=wInfo.getMediumType().c_str();
  if (strcasecmp(s, "ddvcard")==0) {
    wInfo.setCryptMode(AH_CryptMode_Ddv);
    w=new WizardDdvImport(_app, &wInfo, _parent, "WizardDdvImport", TRUE);
  }
  else {
    wInfo.setCryptMode(AH_CryptMode_Rdh);
    w=new WizardRdhImport(_app, &wInfo, false,
			  _parent, "WizardRdhImport", TRUE);
  }

  /* setup user */
  if (w->exec()==QDialog::Accepted) {
    DBG_NOTICE(0, "Accepted");
    /* unmount medium */
    AH_Provider_ClearCryptTokenList(wInfo.getProvider());
    wInfo.setToken(NULL);
    wInfo.subFlags(WIZARDINFO_FLAGS_MEDIUM_CREATED);
    wInfo.setUser(NULL);
    wInfo.subFlags(WIZARDINFO_FLAGS_USER_CREATED);
  }
  else {
    DBG_NOTICE(0, "Rejected");
    wInfo.releaseData();
    return false;
  }

  return true;
}



bool UserWizard::_handleModeImportFile() {
  Wizard *w;
  WizardInfo wInfo(_provider);

  wInfo.setCryptMode(AH_CryptMode_Rdh);
  w=new WizardRdhImport(_app, &wInfo, TRUE,
                        _parent, "WizardRdhImport", TRUE);

  /* setup user */
  if (w->exec()==QDialog::Accepted) {
    DBG_NOTICE(0, "Accepted");
    AH_Provider_ClearCryptTokenList(_provider);

    wInfo.setToken(NULL);
    wInfo.subFlags(WIZARDINFO_FLAGS_MEDIUM_CREATED);
    wInfo.setUser(NULL);
    wInfo.subFlags(WIZARDINFO_FLAGS_USER_CREATED);
  }
  else {
    DBG_NOTICE(0, "Rejected");
    wInfo.releaseData();
    return false;
  }

  return true;
}



bool UserWizard::_handleModeCreateFile() {
  Wizard *w;
  WizardInfo wInfo(_provider);

  wInfo.setCryptMode(AH_CryptMode_Rdh);
  w=new WizardRdhNew(_app, &wInfo, _parent, "WizardRdhImport", TRUE);

  /* setup user */
  if (w->exec()==QDialog::Accepted) {
    DBG_NOTICE(0, "Accepted");
    AH_Provider_ClearCryptTokenList(wInfo.getProvider());
    wInfo.setToken(NULL);
    wInfo.subFlags(WIZARDINFO_FLAGS_MEDIUM_CREATED);
    wInfo.setUser(NULL);
    wInfo.subFlags(WIZARDINFO_FLAGS_USER_CREATED);
  }
  else {
    DBG_NOTICE(0, "Rejected");
    wInfo.releaseData();
    return false;
  }

  return true;
}



bool UserWizard::exec() {
  int mode;

  mode=SelectMode::selectMode(_parent);
  switch(mode) {
  case SelectMode::ModeUnknown:
    DBG_INFO(0, "Mode selection dialog was aborted");
    break;
  case SelectMode::ModeImportCard:
    return _handleModeImportCard();
  case SelectMode::ModeInitCard:
      QMessageBox::information(_parent,
                               "Not yet implemented",
                               "Sorry, this mode is not yet implemented",
                               QMessageBox::Abort);
    break;
  case SelectMode::ModeImportFile:
    return _handleModeImportFile();
  case SelectMode::ModeCreateFile:
    return _handleModeCreateFile();
    break;
  case SelectMode::ModePinTan:
    return _handleModePinTan();
  }

  return false;
}



bool UserWizard::finishUser(QBanking *qb,
                            AB_PROVIDER *pro,
                            AB_USER *u,
                            QWidget *parent) {
  WizardRdhNew2 *w;
  WizardInfo wInfo(pro);
  const char *s;

  wInfo.setUser(u);
  s=AH_User_GetTokenType(u);
  if (s)
    wInfo.setMediumType(s);
  s=AH_User_GetTokenName(u);
  if (s)
    wInfo.setMediumName(s);
  wInfo.setContext(AH_User_GetTokenContextId(u));

  /* setup user */
  w=new WizardRdhNew2(qb, &wInfo, parent, "WizardRdhNew2", TRUE);
  if (w->exec()==QDialog::Accepted) {
    DBG_NOTICE(0, "Accepted");
    /* unmount medium */
    AH_Provider_ClearCryptTokenList(pro);
  }
  else {
    DBG_NOTICE(0, "Rejected");
    wInfo.releaseData();
    return false;
  }

  return true;

}








