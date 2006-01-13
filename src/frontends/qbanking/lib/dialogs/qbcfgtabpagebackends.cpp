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


#include "qbcfgtabpagebackends.h"
#include "qbcfgtabpagebackends.ui.h"
#include "qbplugindescrlist.h"

#include <qbanking/qbanking.h>

#include <qlineedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qlayout.h>

#include <gwenhywfar/debug.h>



QBCfgTabPageBackends::QBCfgTabPageBackends(QBanking *qb,
                                           QWidget *parent,
                                           const char *name,
                                           WFlags f)
:QBCfgTabPage(qb, tr("Backends"), parent, name, f){
  _realPage=new QBCfgTabPageBackendsUi(this);
  addWidget(_realPage);
  _realPage->show();

  setHelpSubject("QBCfgTabPageBackends");
  setDescription(tr("This page allows you to enable, disable and setup"
                    " banking backends for AqBanking."));

  QObject::connect(_realPage->backendEnableButton, SIGNAL(clicked()),
                   this, SLOT(slotBackendEnable()));
  QObject::connect(_realPage->backendDisableButton, SIGNAL(clicked()),
                   this, SLOT(slotBackendDisable()));

  QObject::connect(_realPage->backendSetupButton, SIGNAL(clicked()),
                   this, SLOT(slotBackendSetup()));

}



QBCfgTabPageBackends::~QBCfgTabPageBackends() {
}



void QBCfgTabPageBackends::_backendRescan(){
  _realPage->backendList->clear();
  _realPage->backendList->addPluginDescrs(getBanking()->getProviderDescrs());
}



bool QBCfgTabPageBackends::toGui() {
  GWEN_DB_NODE *dbSettings;
  int i, j;

  dbSettings=getBanking()->getSharedData("qbanking");
  assert(dbSettings);
  dbSettings=GWEN_DB_GetGroup(dbSettings, GWEN_DB_FLAGS_DEFAULT,
                              "settings");
  assert(dbSettings);

  /* setup provider list view */
  _realPage->backendList->setResizeMode(QListView::NoColumn);
  for (i=0; i<_realPage->backendList->columns(); i++) {
    _realPage->backendList->setColumnWidthMode(i, QListView::Manual);
    j=GWEN_DB_GetIntValue(dbSettings, "gui/backendList/columns", i, -1);
    if (j!=-1)
      _realPage->backendList->setColumnWidth(i, j);
  } /* for */
  _realPage->backendList->setSelectionMode(QListView::Single);

  _backendRescan();
  return true;
}



bool QBCfgTabPageBackends::fromGui() {
  GWEN_DB_NODE *dbSettings;
  int i, j;

  dbSettings=getBanking()->getSharedData("qbanking");
  assert(dbSettings);
  dbSettings=GWEN_DB_GetGroup(dbSettings, GWEN_DB_FLAGS_DEFAULT,
                              "settings");
  assert(dbSettings);

  /* save user list view settings */
  GWEN_DB_DeleteVar(dbSettings, "gui/backendList/columns");
  for (i=0; i<_realPage->backendList->columns(); i++) {
    j=_realPage->backendList->columnWidth(i);
    GWEN_DB_SetIntValue(dbSettings, GWEN_DB_FLAGS_DEFAULT,
                        "gui/backendList/columns", j);
  } /* for */

  return true;
}



void QBCfgTabPageBackends::slotBackendEnable(){
  GWEN_PLUGIN_DESCRIPTION *pd;

  pd=_realPage->backendList->getCurrentPluginDescr();
  if (!pd) {
    fprintf(stderr, "No provider selected.\n");
  }
  else {
    int rv;

    if (GWEN_PluginDescription_IsActive(pd)) {
      fprintf(stderr, "Provider already active.\n");
      return;
    }

    rv=getBanking()->activateProvider(GWEN_PluginDescription_GetName(pd));
    _backendRescan();
    if (rv) {
      QMessageBox::critical(this,
                            tr("Backend Error"),
                            tr("Could not activate this backend.\n"),
			    QMessageBox::Ok,QMessageBox::NoButton);
    }
    else {
      QMessageBox::information(this,
                               tr("Backend Activated"),
                               tr("This backend has successfully been "
                                  "activated.\n"),
                               QMessageBox::Ok);
    }
  }
}



void QBCfgTabPageBackends::slotBackendDisable(){
  GWEN_PLUGIN_DESCRIPTION *pd;

  pd=_realPage->backendList->getCurrentPluginDescr();
  if (!pd) {
    fprintf(stderr, "No provider selected.\n");
  }
  else {
    int rv;
    int r;

    if (!GWEN_PluginDescription_IsActive(pd)) {
      fprintf(stderr, "Provider already inactive.\n");
      return;
    }

    r=QMessageBox::warning(this,
                           tr("Disable Backend"),
                           tr("This would remove all accounts currently "
                              "supported by that backend.\n"
                              "\n"
                              "Do you still want me to disable it?"),
                           QMessageBox::Yes, QMessageBox::No);
    if (r!=0 && r!=QMessageBox::Yes)
      return;

    rv=getBanking()->deactivateProvider(GWEN_PluginDescription_GetName(pd));
    _backendRescan();
    if (rv) {
      QMessageBox::critical(this,
                            tr("Backend Error"),
                            tr("Could not deactivate this backend.\n"),
                            QMessageBox::Ok,QMessageBox::NoButton);
    }
  }
}



void QBCfgTabPageBackends::slotBackendSetup(){
}



void QBCfgTabPageBackends::updateView() {
  _backendRescan();
}




