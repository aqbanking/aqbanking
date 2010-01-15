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

// QBanking includes
#include "qbedituser.h"
#include "qbcfgtabpageusergen.h"
#include "qbcfgmodule.h"
#include "qbanking.h"

// QT includes
#include <qmessagebox.h>

// Gwenhywfar includes
#include <gwenhywfar/debug.h>




QBEditUser::QBEditUser(QBanking *kb,
                       AB_USER *u,
                       QWidget* parent,
                       const char* name,
                       Qt::WFlags fl)
:QBCfgTab(kb, parent, name, fl)
,_user(u) {
  QBCfgModule *mod;
  QBCfgTabPageUser *generalPage;
  QBCfgTabPageUser *uPage;
  const char *backendName;

  setCaption(tr("User Configuration"));
  setHelpContext("QBEditUser");
  setDescription(tr("<p>You can now setup this user.</p>"));

  /* add general page */
  uPage=new QBCfgTabPageUserGeneral(kb, u, this, "GeneralUserPage");
  addPage(uPage);
  uPage->show();
  generalPage=uPage;

  /* add application specific page, if any */
  mod=kb->getConfigModule(0);
  if (mod) {
    uPage=mod->getEditUserPage(u, this);
    if (uPage) {
      addPage(uPage);
      uPage->show();
    }
  }

  /* add backend specific page, if any */
  backendName=AB_User_GetBackendName(u);
  assert(backendName);
  mod=kb->getConfigModule(backendName);
  if (mod) {
    uPage=mod->getEditUserPage(u, this);
    if (uPage) {
      generalPage->setUserIdInfo(uPage->getUserIdLabel(), "");
      generalPage->setCustomerIdInfo(uPage->getCustomerIdLabel(), "");

      addPage(uPage);
      uPage->show();
    }
  }
}



QBEditUser::~QBEditUser() {
}



bool QBEditUser::fromGui(bool doLock) {
  int rv;

  if (doLock) {
    rv=getBanking()->beginExclUseUser(_user, 0);
    if (rv<0) {
      DBG_ERROR(0, "Could not lock user");
      QMessageBox::critical(this,
			    tr("Error"),
			    tr("Could not lock user data. "
			       "Maybe this user is still used by another application?"),
			    QMessageBox::Ok,Qt::NoButton);
      return false;
    }
  }

  if (!QBCfgTab::fromGui()) {
    if (doLock)
      getBanking()->endExclUseUser(_user, 1, 0); /* abandon changes */
    return false;
  }

  if (doLock) {
    rv=getBanking()->endExclUseUser(_user, 0, 0);
    if (rv<0) {
      DBG_ERROR(0, "Could not unlock user");
      QMessageBox::critical(this,
			    tr("Internal Error"),
			    tr("Could not unlock user data."),
			    QMessageBox::Ok,Qt::NoButton);
      return false;
    }
  }

  return true;
}



bool QBEditUser::editUser(QBanking *kb, AB_USER *u,
			  bool doLock,
			  QWidget* parent) {
  QBEditUser eu(kb, u, parent);

  if (!eu.toGui())
    return false;
  if (eu.exec()!=QDialog::Accepted)
    return false;
  if (!eu.fromGui(doLock))
    return false;
  return true;
}








