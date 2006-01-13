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


#include "a_checkfile.h"
#include "wizard.h"
#include "actionwidget.h"

#include <qpushbutton.h>

#include <qbanking/qbanking.h>
#include <aqhbci/provider.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/waitcallback.h>

#include <assert.h>



ActionCheckFile::ActionCheckFile(Wizard *w)
:WizardAction(w, "CheckFile", QWidget::tr("Check Key File")) {
  _realDialog=new ActionWidget
    (tr("<qt>"
        "We have now to check the selected keyfile."
        "</qt>"),
     tr("<qt>"
        "When you click the button below the file you selected will be "
        "checked in order to find out which plugin is needed to support "
        "the keyfile."
        "</qt>"),
     tr("Check Key File"),
     this, "CheckFile");
  _realDialog->setStatus(ActionWidget::StatusNone);
  connect(_realDialog->getButton(), SIGNAL(clicked()),
          this, SLOT(slotButtonClicked()));

  addWidget(_realDialog);
  _realDialog->show();
  setNextEnabled(false);
}



ActionCheckFile::~ActionCheckFile() {
}



void ActionCheckFile::enter() {
  setNextEnabled(false);
  _realDialog->getButton()->setEnabled(true);
  _realDialog->setStatus(ActionWidget::StatusNone);
}



void ActionCheckFile::leave(bool backward) {
  if (backward) {
    undo();
    setNextEnabled(false);
    _realDialog->getButton()->setEnabled(true);
  }
}



void ActionCheckFile::slotButtonClicked() {
  WizardInfo *wInfo;
  QBanking *qb;
  AB_PROVIDER *pro;
  int rv;
  GWEN_BUFFER *mtypeName;
  GWEN_BUFFER *msubTypeName;
  GWEN_BUFFER *mediumName;
  std::string s;
  QString txt;
  AH_MEDIUM *m;
  bool created;

  wInfo=getWizard()->getWizardInfo();
  assert(wInfo);
  qb=getWizard()->getBanking();
  assert(qb);
  pro=AH_HBCI_GetProvider(wInfo->getHbci());
  assert(pro);

  _realDialog->setStatus(ActionWidget::StatusChecking);

  mtypeName=GWEN_Buffer_new(0, 64, 0, 1);
  msubTypeName=GWEN_Buffer_new(0, 64, 0, 1);
  mediumName=GWEN_Buffer_new(0, 64, 0, 1);

  s=wInfo->getMediumName();
  if (!s.empty())
    GWEN_Buffer_AppendString(mediumName, s.c_str());

  txt=QWidget::trUtf8("<qt>"
                      "Checking type of the key file, please wait..."
                      "</qt>");
  GWEN_WaitCallback_EnterWithText(GWEN_WAITCALLBACK_ID_SIMPLE_PROGRESS,
                                  QBanking::QStringToUtf8String(txt).c_str(),
                                  0, GWEN_WAITCALLBACK_FLAGS_IMMEDIATELY);
  rv=AH_HBCI_CheckMedium(wInfo->getHbci(),
                         GWEN_CryptToken_Device_File,
                         mtypeName, msubTypeName, mediumName);
  GWEN_WaitCallback_Leave();
  if (rv) {
    DBG_ERROR(0, "here (%d)", rv);
    GWEN_Buffer_free(mediumName);
    GWEN_Buffer_free(msubTypeName);
    GWEN_Buffer_free(mtypeName);
    _realDialog->setStatus(ActionWidget::StatusFailed);
    return;
  }

  m=AH_HBCI_FindMedium(wInfo->getHbci(),
                       GWEN_Buffer_GetStart(mtypeName),
                       GWEN_Buffer_GetStart(mediumName));
  if (m) {
    DBG_ERROR(0, "Medium is already listed");
    created=false;
    wInfo->setMedium(m);
  }
  else {
    m=AH_HBCI_MediumFactory(wInfo->getHbci(),
                            GWEN_Buffer_GetStart(mtypeName),
                            GWEN_Buffer_GetStart(msubTypeName),
                            GWEN_Buffer_GetStart(mediumName));
    assert(m);
    created=true;
  }

  GWEN_Buffer_free(mediumName);
  GWEN_Buffer_free(msubTypeName);
  GWEN_Buffer_free(mtypeName);

  rv=AH_Medium_Mount(m);
  if (rv) {
    DBG_ERROR(0, "Error mounting medium (%d)", rv);
    _realDialog->setStatus(ActionWidget::StatusFailed);
    if (created)
      AH_Medium_free(m);
    return;
  }

  wInfo->setMedium(m);
  if (created)
    wInfo->addFlags(WIZARDINFO_FLAGS_MEDIUM_CREATED);

  _realDialog->setStatus(ActionWidget::StatusSuccess);
  _realDialog->getButton()->setEnabled(false);
  setNextEnabled(true);
}



bool ActionCheckFile::apply() {
  return (_realDialog->getStatus()==ActionWidget::StatusSuccess);
}



bool ActionCheckFile::undo() {
  WizardInfo *wInfo;
  AH_HBCI *h;
  AH_MEDIUM *m;

  wInfo=getWizard()->getWizardInfo();
  assert(wInfo);
  h=wInfo->getHbci();
  assert(h);

  m=wInfo->getMedium();
  if (m) {
    if (AH_Medium_IsMounted(m))
      AH_Medium_Unmount(m, 1);

    if (wInfo->getFlags() & WIZARDINFO_FLAGS_MEDIUM_CREATED) {
      AH_Medium_free(m);
      wInfo->subFlags(WIZARDINFO_FLAGS_MEDIUM_CREATED);
    }
    wInfo->setMedium(0);
  }

  return true;
}





