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


#include "qbanking.h"
#include "qbcfgtabsettings.h"
#include "qbcfgtabpageusers.h"
#include "qbcfgtabpageaccounts.h"
#include "qbcfgtabpagebackends.h"

#include <aqbanking/banking.h>
#include <aqbanking/account.h>

#include <gwenhywfar/types.h>





QBCfgTabSettings::QBCfgTabSettings(QBanking *qb,
                                   QWidget *parent,
                                   const char *name,
                                   Qt::WFlags f)
:QBCfgTab(qb, parent, name, f) {
  setHelpContext("QBCfgTabSettings");
  setDescription(tr("This dialog allows adjusting the settings of "
                    "<b>AqBanking</b>."));
  resize(720, 400);
}



QBCfgTabSettings::~QBCfgTabSettings() {
}



void QBCfgTabSettings::addAccountsPage() {
  QBCfgTabPageAccounts *p;

  p=new QBCfgTabPageAccounts(getBanking(), this);
  addPage(p);
}



void QBCfgTabSettings::addUsersPage() {
  QBCfgTabPageUsers *p;

  p=new QBCfgTabPageUsers(getBanking(), this);
  addPage(p);
}



void QBCfgTabSettings::addBackendsPage() {
  QBCfgTabPageBackends *p;

  p=new QBCfgTabPageBackends(getBanking(), this);
  addPage(p);
}






void QBCfgTabSettings::slotUpdate() {
}



bool QBCfgTabSettings::toGui() {
  GWEN_DB_NODE *dbSettings;
  int w, h;

  dbSettings=getBanking()->getSharedData("qbanking");
  assert(dbSettings);
  dbSettings=GWEN_DB_GetGroup(dbSettings, GWEN_DB_FLAGS_DEFAULT,
                              "settings");
  assert(dbSettings);
  w=GWEN_DB_GetIntValue(dbSettings, "gui/width", 0, -1);
  h=GWEN_DB_GetIntValue(dbSettings, "gui/height", 0, -1);
  if (w>100 && h>100)
    resize(w, h);
  return QBCfgTab::toGui();
}



bool QBCfgTabSettings::fromGui() {
  GWEN_DB_NODE *dbSettings;

  dbSettings=getBanking()->getSharedData("qbanking");
  assert(dbSettings);
  dbSettings=GWEN_DB_GetGroup(dbSettings, GWEN_DB_FLAGS_DEFAULT,
                              "settings");
  assert(dbSettings);
  GWEN_DB_SetIntValue(dbSettings, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "gui/width", width());
  GWEN_DB_SetIntValue(dbSettings, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "gui/height", height());
  return QBCfgTab::fromGui();
}






