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
#include <aqhbci/medium.h>
#include <aqhbci/user.h>
#include <qbanking/qbanking.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/waitcallback.h>

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
  AH_MEDIUM *m;
  int rv;
  GWEN_TIME *ti;
  GWEN_BUFFER *bufName;

  /* create medium */
  wInfo.setCryptMode(AH_CryptMode_Pintan);
  bufName=GWEN_Buffer_new(0, 128, 0, 1);
  GWEN_Buffer_AppendString(bufName, "PINTAN-");
  ti=GWEN_CurrentTime();
  assert(ti);
  GWEN_Time_toString(ti, "YYYYMMDD-hhmmss", bufName);
  GWEN_Time_free(ti);
  m=AH_Provider_MediumFactory(_provider,
                              "pintan", 0,
                              GWEN_Buffer_GetStart(bufName));
  GWEN_Buffer_free(bufName);
  assert(m);

  /* mount medium */
  rv=AH_Medium_Mount(m);
  if (rv) {
    DBG_ERROR(0, "Could not mount medium (%d)", rv);
    return false;
  }

  wInfo.setMedium(m);
  wInfo.addFlags(WIZARDINFO_FLAGS_MEDIUM_CREATED);


  /* setup user */
  w=new WizardPinTanNew(_app, &wInfo, _parent, "WizardPinTanNew", TRUE);

  if (w->exec()==QDialog::Accepted) {
    DBG_NOTICE(0, "Accepted");
    /* unmount medium */
    rv=AH_Medium_Unmount(m, 1);
    if (rv) {
      DBG_ERROR(0, "Could not unmount medium (%d)", rv);
      wInfo.releaseData();
      return false;
    }

    AH_Provider_AddMedium(_provider, m);
    wInfo.setMedium(0);
    wInfo.subFlags(WIZARDINFO_FLAGS_MEDIUM_CREATED);
    AB_Banking_AddUser(_app->getCInterface(), wInfo.getUser());
    wInfo.setUser(0);
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
  GWEN_BUFFER *msubTypeName;
  GWEN_BUFFER *mediumName;
  AH_MEDIUM *m;
  QString txt;

  mtypeName=GWEN_Buffer_new(0, 64, 0, 1);
  msubTypeName=GWEN_Buffer_new(0, 64, 0, 1);
  mediumName=GWEN_Buffer_new(0, 64, 0, 1);

  txt=QWidget::trUtf8("<qt>"
                      "Checking type of the security medium, please wait..."
                      "</qt>");
  GWEN_WaitCallback_EnterWithText(GWEN_WAITCALLBACK_ID_SIMPLE_PROGRESS,
                                  QBanking::QStringToUtf8String(txt).c_str(),
                                  0, GWEN_WAITCALLBACK_FLAGS_IMMEDIATELY);
  rv=AH_Provider_CheckMedium(_provider, GWEN_CryptToken_Device_Card,
                             mtypeName, msubTypeName, mediumName);
  GWEN_WaitCallback_Leave();
  if (rv) {
    DBG_ERROR(0, "here (%d)", rv);
    GWEN_Buffer_free(mediumName);
    GWEN_Buffer_free(msubTypeName);
    GWEN_Buffer_free(mtypeName);
    return false;
  }

  m=AH_Provider_FindMedium(_provider,
                           GWEN_Buffer_GetStart(mtypeName),
                           GWEN_Buffer_GetStart(mediumName));
  if (m) {
    DBG_INFO(0, "Medium is already listed");
    wInfo->setMedium(m);
    wInfo->subFlags(WIZARDINFO_FLAGS_MEDIUM_ADDED |
                    WIZARDINFO_FLAGS_MEDIUM_CREATED);
    GWEN_Buffer_free(mediumName);
    GWEN_Buffer_free(msubTypeName);
    GWEN_Buffer_free(mtypeName);
    return true;
  }

  m=AH_Provider_MediumFactory(_provider,
                              GWEN_Buffer_GetStart(mtypeName),
                              GWEN_Buffer_GetStart(msubTypeName),
                              GWEN_Buffer_GetStart(mediumName));
  assert(m);
  wInfo->setMedium(m);
  wInfo->addFlags(WIZARDINFO_FLAGS_MEDIUM_CREATED);

  GWEN_Buffer_free(mediumName);
  GWEN_Buffer_free(msubTypeName);
  GWEN_Buffer_free(mtypeName);
  return true;
}



bool UserWizard::_handleModeImportCard() {
  Wizard *w;
  WizardInfo wInfo(_provider);
  AH_MEDIUM *m;
  int rv;
  const char *s;

  /* create medium */
  if (!_checkAndCreateMedium(&wInfo))
    return false;

  m=wInfo.getMedium();
  assert(m);

  /* mount medium */
  rv=AH_Medium_Mount(m);
  if (rv) {
    DBG_ERROR(0, "Could not mount medium (%d)", rv);
    return false;
  }
  wInfo.setMedium(m);
  wInfo.addFlags(WIZARDINFO_FLAGS_MEDIUM_CREATED);

  s=AH_Medium_GetMediumTypeName(m);
  assert(s);
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
    rv=AH_Medium_Unmount(m, 1);
    if (rv) {
      DBG_ERROR(0, "Could not unmount medium (%d)", rv);
      wInfo.releaseData();
      return false;
    }

    DBG_INFO(0, "Adding medium");
    AH_Provider_AddMedium(_provider, m);
    wInfo.setMedium(0);
    wInfo.subFlags(WIZARDINFO_FLAGS_MEDIUM_CREATED);
    AB_Banking_AddUser(_app->getCInterface(), wInfo.getUser());
    wInfo.setUser(0);
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
  int rv;

  wInfo.setCryptMode(AH_CryptMode_Rdh);
  w=new WizardRdhImport(_app, &wInfo, TRUE,
                        _parent, "WizardRdhImport", TRUE);

  /* setup user */
  if (w->exec()==QDialog::Accepted) {
    AH_MEDIUM *m;

    DBG_NOTICE(0, "Accepted");
    m=wInfo.getMedium();
    assert(m);

    /* unmount medium */
    rv=AH_Medium_Unmount(m, 1);
    if (rv) {
      DBG_ERROR(0, "Could not unmount medium (%d)", rv);
      wInfo.releaseData();
      return false;
    }

    DBG_INFO(0, "Adding medium");
    AH_Provider_AddMedium(_provider, m);
    wInfo.setMedium(0);
    wInfo.subFlags(WIZARDINFO_FLAGS_MEDIUM_CREATED);
    AB_Banking_AddUser(_app->getCInterface(), wInfo.getUser());
    wInfo.setUser(0);
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
  int rv;

  wInfo.setCryptMode(AH_CryptMode_Rdh);
  w=new WizardRdhNew(_app, &wInfo, _parent, "WizardRdhImport", TRUE);

  /* setup user */
  if (w->exec()==QDialog::Accepted) {
    AH_MEDIUM *m;

    DBG_NOTICE(0, "Accepted");
    m=wInfo.getMedium();
    assert(m);

    /* unmount medium */
    rv=AH_Medium_Unmount(m, 1);
    if (rv) {
      DBG_ERROR(0, "Could not unmount medium (%d)", rv);
      wInfo.releaseData();
      return false;
    }

    DBG_INFO(0, "Adding medium");
    AH_Provider_AddMedium(_provider, m);
    wInfo.setMedium(0);
    wInfo.subFlags(WIZARDINFO_FLAGS_MEDIUM_CREATED);
    AB_Banking_AddUser(_app->getCInterface(), wInfo.getUser());
    wInfo.setUser(0);
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
  AH_MEDIUM *m;
  int rv;

  m=AH_User_GetMedium(u);
  assert(m);

  wInfo.setUser(u);
  wInfo.setMedium(m);

  /* setup user */
  w=new WizardRdhNew2(qb, &wInfo, parent, "WizardRdhNew2", TRUE);
  if (w->exec()==QDialog::Accepted) {
    DBG_NOTICE(0, "Accepted");
    /* unmount medium */
    rv=AH_Medium_Unmount(m, 1);
    if (rv) {
      DBG_ERROR(0, "Could not unmount medium (%d)", rv);
      wInfo.releaseData();
      return false;
    }
  }
  else {
    DBG_NOTICE(0, "Rejected");
    wInfo.releaseData();
    return false;
  }

  return true;

}








