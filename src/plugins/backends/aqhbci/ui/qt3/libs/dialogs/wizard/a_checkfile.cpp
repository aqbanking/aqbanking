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
#include <aqbanking/banking_be.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/ctplugin.h>

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
  GWEN_BUFFER *mediumName;
  std::string s;
  QString txt;
  GWEN_CRYPT_TOKEN *ct;
  bool created;
  uint32_t pid;

  wInfo=getWizard()->getWizardInfo();
  assert(wInfo);
  qb=getWizard()->getBanking();
  assert(qb);
  pro=wInfo->getProvider();
  assert(pro);

  _realDialog->setStatus(ActionWidget::StatusChecking);

  txt=QWidget::trUtf8("<qt>"
                      "Checking type of the key file, please wait..."
		      "</qt>");

  /* try to determine the type and name */
  mtypeName=GWEN_Buffer_new(0, 64, 0, 1);
  mediumName=GWEN_Buffer_new(0, 64, 0, 1);
  s=wInfo->getMediumName();
  if (!s.empty())
    GWEN_Buffer_AppendString(mediumName, s.c_str());

  pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_ALLOW_SUBLEVELS |
			     GWEN_GUI_PROGRESS_SHOW_PROGRESS |
			     GWEN_GUI_PROGRESS_KEEP_OPEN |
			     GWEN_GUI_PROGRESS_SHOW_ABORT,
			     QWidget::tr("Checking Medium").utf8(),
			     txt.utf8(),
			     GWEN_GUI_PROGRESS_NONE,
			     0);

  rv=AB_Banking_CheckCryptToken(AB_Provider_GetBanking(pro),
				GWEN_Crypt_Token_Device_File,
				mtypeName,
				mediumName,
				pid);
  GWEN_Gui_ProgressEnd(pid);
  if (rv) {
    DBG_ERROR(0, "here (%d)", rv);
    GWEN_Buffer_free(mediumName);
    GWEN_Buffer_free(mtypeName);
    _realDialog->setStatus(ActionWidget::StatusFailed);
    return;
  }
  wInfo->setMediumType(GWEN_Buffer_GetStart(mtypeName));
  wInfo->setMediumName(GWEN_Buffer_GetStart(mediumName));
  GWEN_Buffer_free(mediumName);
  GWEN_Buffer_free(mtypeName);

  /* create crypt token */
  rv=AB_Banking_GetCryptToken(AB_Provider_GetBanking(pro),
			      wInfo->getMediumType().c_str(),
			      wInfo->getMediumName().c_str(),
			      &ct);
  if (rv) {
    DBG_ERROR(0, "Error creating CryptToken object (%d)", rv);
    _realDialog->setStatus(ActionWidget::StatusFailed);
    return;
  }

  created=true;

  rv=GWEN_Crypt_Token_Open(ct, 0, 0);
  if (rv) {
    DBG_ERROR(0, "Error mounting medium (%d)", rv);
    _realDialog->setStatus(ActionWidget::StatusFailed);
    if (created)
      AB_Banking_ClearCryptTokenList(AB_Provider_GetBanking(pro), 0);
    return;
  }

  wInfo->setToken(ct);
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
  GWEN_CRYPT_TOKEN *ct;
  AB_PROVIDER *pro;

  wInfo=getWizard()->getWizardInfo();
  assert(wInfo);

  pro=wInfo->getProvider();
  assert(pro);

  ct=wInfo->getToken();
  if (ct) {
    AB_Banking_ClearCryptTokenList(AB_Provider_GetBanking(pro), 0);
    if (wInfo->getFlags() & WIZARDINFO_FLAGS_MEDIUM_CREATED) {
      wInfo->subFlags(WIZARDINFO_FLAGS_MEDIUM_CREATED);
    }
    wInfo->setToken(NULL);
  }

  return true;
}


#include "a_checkfile.moc"


