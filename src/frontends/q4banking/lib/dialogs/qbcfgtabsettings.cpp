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
#include "qbanking.h"
#include "qbcfgtabsettings.h"
#include "qbcfgtabpageusers.h"
#include "qbcfgtabpageaccounts.h"
#include "qbcfgtabpagebackends.h"

// AqBanking includes
#include <aqbanking/banking.h>
#include <aqbanking/account.h>

// Gwenhywfar includes
#include <gwenhywfar/types.h>
#include <gwenhywfar/debug.h>





QBCfgTabSettings::QBCfgTabSettings(QBanking *qb,
                                   QWidget *parent,
                                   const char *name,
                                   Qt::WFlags f)
:QBCfgTab(qb, parent, name, f) {
  setHelpContext("QBCfgTabSettings");
  setDescription(tr("This dialog allows adjusting the settings of "
                    "<b>AqBanking</b>."));
  setAllowApply(false);
  resize(720, 400);
}



QBCfgTabSettings::~QBCfgTabSettings() {
}



void QBCfgTabSettings::addAccountsPage() {
  QBCfgTabPageAccounts *p;

  p=new QBCfgTabPageAccounts(getBanking(), this);
  connect(p, SIGNAL(signalUpdate()),
	  this, SLOT(slotUpdate()));
  connect(this, SIGNAL(signalUpdate()),
	  p, SLOT(slotUpdate()));
  addPage(p);
}



void QBCfgTabSettings::addUsersPage() {
  QBCfgTabPageUsers *p;

  p=new QBCfgTabPageUsers(getBanking(), this);
  connect(p, SIGNAL(signalUpdate()),
	  this, SLOT(slotUpdate()));
  connect(this, SIGNAL(signalUpdate()),
	  p, SLOT(slotUpdate()));
  addPage(p);
}



void QBCfgTabSettings::addBackendsPage() {
  QBCfgTabPageBackends *p;

  p=new QBCfgTabPageBackends(getBanking(), this);
  connect(p, SIGNAL(signalUpdate()),
	  this, SLOT(slotUpdate()));
  connect(this, SIGNAL(signalUpdate()),
	  p, SLOT(slotUpdate()));
  addPage(p);
}






void QBCfgTabSettings::slotUpdate() {
  DBG_INFO(AQBANKING_LOGDOMAIN, "Updating all tabs");
  emit signalUpdate();
}



bool QBCfgTabSettings::toGui() {
  GWEN_DB_NODE *dbConfig=NULL;
  int rv;

  rv=getBanking()->loadSharedSubConfig("qbanking",
				       "settings/gui/generic",
				       &dbConfig,
				       0);
  if (rv==0) {
    int w, h;

    assert(dbConfig);

    w=GWEN_DB_GetIntValue(dbConfig, "width", 0, -1);
    h=GWEN_DB_GetIntValue(dbConfig, "height", 0, -1);
    if (w>100 && h>100)
      resize(w, h);
    GWEN_DB_Group_free(dbConfig);

    return QBCfgTab::toGui();
  }
  else {
    DBG_INFO(0, "here (%d)", rv);
    return false;
  }
}



bool QBCfgTabSettings::fromGui() {
  GWEN_DB_NODE *dbConfig;
  int rv;

  dbConfig=GWEN_DB_Group_new("config");
  assert(dbConfig);

  GWEN_DB_SetIntValue(dbConfig, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "gui/width", width());
  GWEN_DB_SetIntValue(dbConfig, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "gui/height", height());
  if (!QBCfgTab::fromGui()) {
    DBG_INFO(0, "here");
    GWEN_DB_Group_free(dbConfig);
    return false;
  }

  rv=getBanking()->saveSharedSubConfig("qbanking",
				       "settings/gui/generic",
				       dbConfig,
				       0);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    GWEN_DB_Group_free(dbConfig);
    return false;
  }
  GWEN_DB_Group_free(dbConfig);
  return true;
}




#include "qbcfgtabsettings.moc"


