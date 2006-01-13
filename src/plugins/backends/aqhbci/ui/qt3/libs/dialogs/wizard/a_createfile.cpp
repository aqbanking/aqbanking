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
#include <gwenhywfar/debug.h>

#include <unistd.h>




ActionCreateFile::ActionCreateFile(Wizard *w)
:ActionSelectFile(w, false,
                  QWidget::tr("Create Key File"),
                  QWidget::tr("<qt>"
                              "<p>"
                              "Please select a name for the keyfile."
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
  AH_HBCI *h;
  AH_MEDIUM *m;
  int rv;

  if (!ActionSelectFile::apply())
    return false;

  fname=getWizard()->getWizardInfo()->getMediumName();
  if (fname.empty())
    return false;

  wInfo=getWizard()->getWizardInfo();
  assert(wInfo);
  h=wInfo->getHbci();
  assert(h);

  m=AH_HBCI_FindMedium(h, AQHBCI_A_CREATEFILE_CT_TYPE, fname.c_str());
  if (m) {
    DBG_ERROR(0, "Medium is already listed");
    return false;
  }

  m=AH_HBCI_MediumFactory(h,
                          AQHBCI_A_CREATEFILE_CT_TYPE, 0,
                          fname.c_str());
  assert(m);

  rv=AH_Medium_Create(m);
  if (rv) {
    DBG_ERROR(0, "Could not create medium (%d)", rv);
    AH_Medium_free(m);
    return false;
  }

  wInfo->setMedium(m);
  wInfo->addFlags(WIZARDINFO_FLAGS_MEDIUM_CREATED |
                  WIZARDINFO_FLAGS_MEDIUM_FILE_CREATED);

  return true;
}



bool ActionCreateFile::undo() {
  WizardInfo *wInfo;
  AH_HBCI *h;
  AH_MEDIUM *m;
  std::string fname;

  wInfo=getWizard()->getWizardInfo();
  assert(wInfo);
  h=wInfo->getHbci();
  assert(h);

  fname=getWizard()->getWizardInfo()->getMediumName();
  if (fname.empty())
    return true;

  m=wInfo->getMedium();
  if (m) {
    if (wInfo->getFlags() & WIZARDINFO_FLAGS_MEDIUM_CREATED) {
      AH_Medium_free(m);
      wInfo->subFlags(WIZARDINFO_FLAGS_MEDIUM_CREATED);
    }
    wInfo->setMedium(0);
    unlink(fname.c_str());
  }

  return true;
}
















