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


#include "wizard.h"
#include <qbanking/qbanking.h>
#include <aqhbci/outbox.h>
#include <aqhbci/adminjobs.h>

#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qwizard.h>
#include <qcombobox.h>
#include <qtextbrowser.h>

#include <qlineedit.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qpalette.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qprinter.h>
#include <qsimplerichtext.h>
#include <qtextview.h>
#include <qlabel.h>

#include <gwenhywfar/debug.h>




bool Wizard::initCreateKeysPage() {
  QObject::connect((QObject*)(createKeysButton),
                   SIGNAL(clicked()),
                   this,
                   SLOT(slotCreateKeys()));
  setNextEnabled(createKeysPage, _hasAllKeys);
  return true;
}




bool Wizard::doCreateKeysPage(QWidget *p){
  return true;
}



bool Wizard::undoCreateKeysPage(QWidget *p){
  return true;
}



void Wizard::slotCreateKeys(){
  GWEN_TYPE_UINT32 bid;
  int rv;

  setNextEnabled(createKeysPage, _hasAllKeys);
  if (_hasAllKeys) {
    rv=QMessageBox::warning(this,
                            tr("Keys already exist"),
                            tr("<qt>"
                               "<p>"
                               "The necessary keys already exist."
                               "</p>"
                               "<p>"
                               "Do you want to overwrite them?"
                               "</p>"
                               "</qt>"
                              ),
                            tr("Yes"), tr("No"),tr("Abort"),0);
    if (rv!=0) {
      return;
    }
  }

  bid=AB_Banking_ShowBox(_app->getCInterface(), 0,
                         tr("Creating Keys").utf8(),
                         tr("Creating keys, please wait...").utf8());

  if (!AH_Medium_IsMounted(_medium)) {
    if (AH_Medium_Mount(_medium)) {
      DBG_ERROR(0, "Could not mount medium");
      createKeysLabel->setText(_ResultMsg_Failed);
      AB_Banking_HideBox(_app->getCInterface(), bid);
      return;
    }
  }

  if (AH_Medium_SelectContext(_medium, AH_User_GetContextIdx(_user))) {
    DBG_ERROR(0, "Could not select user");
    QMessageBox::critical(this,
			  tr("Medium Error"),
			  tr("Could not select user context on medium.\n"
			     "Please check the logs."
			    ),
			  tr("Dismiss"),0,0,0);
    createKeysLabel->setText(_ResultMsg_Failed);
    AB_Banking_HideBox(_app->getCInterface(), bid);
    return;
  }

  if (AH_Medium_CreateKeys(_medium)) {
    DBG_ERROR(0, "Could not create keys");
    createKeysLabel->setText(_ResultMsg_Failed);
    AB_Banking_HideBox(_app->getCInterface(), bid);
    return;
  }

  _hasAllKeys=false;
  setNextEnabled(createKeysPage, true);

  createKeysButton->setEnabled(false);
  createKeysLabel->setText(_ResultMsg_Success);
  AB_Banking_HideBox(_app->getCInterface(), bid);
}




















