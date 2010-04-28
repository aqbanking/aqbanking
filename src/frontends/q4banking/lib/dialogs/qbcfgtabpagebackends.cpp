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
#include "qbcfgtabpagebackends.h"
#include "qbplugindescrlist.h"
#include "qbanking.h"

// Gwenhywfar includes
#include <gwenhywfar/debug.h>

// QT includes
#include <qlineedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qlayout.h>




QBCfgTabPageBackends::QBCfgTabPageBackends(QBanking *qb,
                                           QWidget *parent,
                                           const char *name,
                                           Qt::WFlags f)
:QBCfgTabPage(qb, tr("Backends"), parent, name, f){
  _realPage.setupUi(this);

  setHelpSubject("QBCfgTabPageBackends");
  setDescription(tr("This page allows you to enable, disable and setup"
                    " banking backends for AqBanking."));

}



QBCfgTabPageBackends::~QBCfgTabPageBackends() {
}



void QBCfgTabPageBackends::_backendRescan(){
  _realPage.backendList->clear();
  _realPage.backendList->addPluginDescrs(getBanking()->getProviderDescrs());
}



bool QBCfgTabPageBackends::toGui() {
  GWEN_DB_NODE *dbConfig=NULL;
  int rv;

  rv=getBanking()->loadSharedSubConfig("qbanking",
				       "settings/gui/backendList",
				       &dbConfig);
  if (rv==0) {
    int i, j;

    assert(dbConfig);

    /* setup backend list view */
    _realPage.backendList->setResizeMode(Q3ListView::NoColumn);
    for (i=0; i<_realPage.backendList->columns(); i++) {
      _realPage.backendList->setColumnWidthMode(i, Q3ListView::Manual);
      j=GWEN_DB_GetIntValue(dbConfig, "columns", i, -1);
      if (j!=-1)
	_realPage.backendList->setColumnWidth(i, j);
    } /* for */
    _realPage.backendList->setSelectionMode(Q3ListView::Single);

    _backendRescan();
    GWEN_DB_Group_free(dbConfig);
    return true;
  }
  else {
    DBG_INFO(0, "here (%d)", rv);
    return false;
  }
}



bool QBCfgTabPageBackends::fromGui() {
  GWEN_DB_NODE *dbConfig;
  int i, j;
  int rv;

  dbConfig=GWEN_DB_Group_new("config");
  assert(dbConfig);

  /* save account list view settings */
  for (i=0; i<_realPage.backendList->columns(); i++) {
    j=_realPage.backendList->columnWidth(i);
    GWEN_DB_SetIntValue(dbConfig, GWEN_DB_FLAGS_DEFAULT,
			"columns", j);
  } /* for */

  rv=getBanking()->saveSharedSubConfig("qbanking",
				       "settings/gui/backendList",
				       dbConfig);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    GWEN_DB_Group_free(dbConfig);
    return false;
  }
  GWEN_DB_Group_free(dbConfig);
  return true;
}




void QBCfgTabPageBackends::updateView() {
  _backendRescan();
}



void QBCfgTabPageBackends::slotUpdate() {
  DBG_INFO(AQBANKING_LOGDOMAIN, "updating backend view");
  updateView();
}


#include "qbcfgtabpagebackends.moc"


