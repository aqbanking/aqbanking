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

#include "a_createfile.h"
#include "wizard.h"

#include <qbanking/qbanking.h>
#include <aqhbci/provider.h>
#include <aqbanking/banking_be.h>
#include <gwenhywfar/debug.h>

#include <unistd.h>




ActionCreateFile::ActionCreateFile(Wizard *w)
:ActionSelectFile(w, false,
                  QWidget::tr("Create Key File"),
                  QWidget::tr("<qt>"
                              "<p>"
                              "Please select a directory and name for the new keyfile."
                              "</p>"
                              "<p>"
                              "If you click <i>next</i> then the keyfile "
                              "will be created."
                              "</p>"
                              "</qt>")){
}



ActionCreateFile::~ActionCreateFile() {
}



bool ActionCreateFile::apply() {
  std::string fname;
  WizardInfo *wInfo;
  AB_PROVIDER *pro;
  GWEN_CRYPT_TOKEN *ct;
  int rv;

  if (!ActionSelectFile::apply())
    return false;

  fname=getWizard()->getWizardInfo()->getMediumName();
  if (fname.empty())
    return false;

  wInfo=getWizard()->getWizardInfo();
  assert(wInfo);
  pro=wInfo->getProvider();
  assert(pro);

  rv=AB_Banking_GetCryptToken(AB_Provider_GetBanking(pro),
			      wInfo->getMediumType().c_str(),
			      wInfo->getMediumName().c_str(),
			      &ct);

  if (rv) {
    DBG_ERROR(0, "Error creating CryptToken object (%d)", rv);
    return false;
  }

  assert(ct);

  rv=GWEN_Crypt_Token_Create(ct, 0);
  if (rv) {
    DBG_ERROR(0, "Error creating CryptToken (%d)", rv);
    AB_Banking_ClearCryptTokenList(AB_Provider_GetBanking(pro), 0);
    return false;
  }

  wInfo->setToken(ct);
  wInfo->addFlags(WIZARDINFO_FLAGS_MEDIUM_CREATED |
		  WIZARDINFO_FLAGS_MEDIUM_FILE_CREATED);

  return true;
}



bool ActionCreateFile::undo() {
  WizardInfo *wInfo;
  AB_PROVIDER *pro;
  std::string fname;
  GWEN_CRYPT_TOKEN *ct;

  wInfo=getWizard()->getWizardInfo();
  assert(wInfo);
  pro=wInfo->getProvider();
  assert(pro);

  fname=getWizard()->getWizardInfo()->getMediumName();
  if (fname.empty())
    return true;

  ct=wInfo->getToken();
  if (ct) {
    if (wInfo->getFlags() & WIZARDINFO_FLAGS_MEDIUM_CREATED) {
      AB_Banking_ClearCryptTokenList(AB_Provider_GetBanking(pro), 0);
      wInfo->subFlags(WIZARDINFO_FLAGS_MEDIUM_CREATED);
    }
    wInfo->setToken(NULL);
    unlink(fname.c_str());
  }

  return true;
}
















