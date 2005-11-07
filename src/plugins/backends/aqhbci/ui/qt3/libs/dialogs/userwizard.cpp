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

#include <aqhbci/hbci.h>
#include <aqhbci/medium.h>
#include <qbanking/qbanking.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/waitcallback.h>




UserWizard::UserWizard(QBanking *qb, AH_HBCI *hbci, QWidget *parent)
:_app(qb)
,_hbci(hbci)
,_parent(parent) {


}



UserWizard::~UserWizard() {
}



bool UserWizard::_handleModePinTan() {
  WizardPinTanNew *w;
  WizardInfo wInfo(_hbci);
  AH_MEDIUM *m;
  int rv;
  GWEN_TIME *ti;
  GWEN_BUFFER *bufName;

  /* create medium */
  bufName=GWEN_Buffer_new(0, 128, 0, 1);
  GWEN_Buffer_AppendString(bufName, "PINTAN-");
  ti=GWEN_CurrentTime();
  assert(ti);
  GWEN_Time_toString(ti, "YYYYMMDD-hhmmss", bufName);
  GWEN_Time_free(ti);
  m=AH_HBCI_MediumFactory(_hbci,
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
  w=new WizardPinTanNew(_app, &wInfo, 0, "WizardPinTanNew", TRUE);
  if (w->exec()==QDialog::Accepted) {
    DBG_NOTICE(0, "Accepted");
    /* unmount medium */
    rv=AH_Medium_Unmount(m, 1);
    if (rv) {
      DBG_ERROR(0, "Could not unmount medium (%d)", rv);
      wInfo.releaseData();
      return false;
    }

    AH_HBCI_AddMedium(_hbci, m);
    wInfo.setMedium(0);
    wInfo.subFlags(WIZARDINFO_FLAGS_MEDIUM_CREATED);
  }
  else {
    DBG_NOTICE(0, "Rejected");
    wInfo.releaseData();
    return false;
  }

  wInfo.releaseData();
  return true;
}



bool UserWizard::_checkAndCreateMedium(WizardInfo *wInfo,
                                       GWEN_CRYPTTOKEN_DEVICE dev) {
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
  rv=AH_HBCI_CheckMedium(_hbci, dev,
                         mtypeName, msubTypeName, mediumName);
  GWEN_WaitCallback_Leave();
  if (rv) {
    DBG_ERROR(0, "here (%d)", rv);
    GWEN_Buffer_free(mediumName);
    GWEN_Buffer_free(msubTypeName);
    GWEN_Buffer_free(mtypeName);
    return false;
  }

  m=AH_HBCI_FindMedium(_hbci,
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

  m=AH_HBCI_MediumFactory(_hbci,
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
  WizardDdvImport *w;
  WizardInfo wInfo(_hbci);
  AH_MEDIUM *m;
  int rv;

  /* create medium */
  if (!_checkAndCreateMedium(&wInfo, GWEN_CryptToken_Device_Card))
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

  /* setup user */
  w=new WizardDdvImport(_app, &wInfo, 0, "WizardDdvImport", TRUE);
  if (w->exec()==QDialog::Accepted) {
    DBG_NOTICE(0, "Accepted");
    /* unmount medium */
    rv=AH_Medium_Unmount(m, 1);
    if (rv) {
      DBG_ERROR(0, "Could not unmount medium (%d)", rv);
      wInfo.releaseData();
      return false;
    }

    AH_HBCI_AddMedium(_hbci, m);
    wInfo.setMedium(0);
    wInfo.subFlags(WIZARDINFO_FLAGS_MEDIUM_CREATED);
  }
  else {
    DBG_NOTICE(0, "Rejected");
    wInfo.releaseData();
    return false;
  }

  wInfo.releaseData();
  return true;


}



bool UserWizard::exec() {
  int mode;

  mode=SelectMode::selectMode(_parent);
  if (mode==SelectMode::ModePinTan) {
    return _handleModePinTan();
  }
  else if (mode==SelectMode::ModeImportCard) {
    return _handleModeImportCard();
  }

  return false;
}








