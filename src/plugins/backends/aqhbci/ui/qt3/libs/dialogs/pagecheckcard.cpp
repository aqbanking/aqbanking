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




bool Wizard::initCheckCardPage() {
  QObject::connect((checkCardButton),
                   SIGNAL(clicked()),
                   this,
                   SLOT(slotCheckCard()));
  return true;
}



void Wizard::slotCheckCard(){
  GWEN_BUFFER *typeBuf;
  GWEN_BUFFER *subtypeBuf;
  GWEN_BUFFER *nameBuf;

  _mediumName.erase();
  typeBuf=GWEN_Buffer_new(0, 64, 0, 1);
  subtypeBuf=GWEN_Buffer_new(0, 64, 0, 1);
  nameBuf=GWEN_Buffer_new(0, 64, 0, 1);

  if (AH_HBCI_CheckMedium(_hbci,
                          GWEN_CryptToken_Device_Card,
                          typeBuf,
                          subtypeBuf,
                          nameBuf)) {
    GWEN_Buffer_free(nameBuf);
    GWEN_Buffer_free(typeBuf);
    GWEN_Buffer_free(typeBuf);
    checkCardLabel->setText(_ResultMsg_NotSupported);

  }
  else {
    checkCardLabel->setText(_ResultMsg_Supported);
    setNextEnabled(checkCardPage, true);
    checkCardButton->setEnabled(false);
    _mediumTypeName=GWEN_Buffer_GetStart(typeBuf);
    _mediumSubTypeName=GWEN_Buffer_GetStart(subtypeBuf);
    _mediumName=GWEN_Buffer_GetStart(nameBuf);
    GWEN_Buffer_free(nameBuf);
    GWEN_Buffer_free(subtypeBuf);
    GWEN_Buffer_free(typeBuf);
  }
}



bool Wizard::doCheckCardPage(QWidget *p){
  _medium=AH_HBCI_SelectMedium(_hbci,
                               _mediumTypeName.c_str(),
                               _mediumSubTypeName.c_str(),
                               _mediumName.c_str());
  if (!_medium) {
    DBG_ERROR(0, "Medium \"%s:%s\" does not exist",
              _mediumTypeName.c_str(),
              _mediumName.c_str());
    QMessageBox::critical(this,
                          tr("Medium Error"),
                          tr("Medium does not exist.\n"
                             "Please check the console logs."),
                          QMessageBox::Ok,QMessageBox::NoButton);
    return false;
  }

  if (!AH_Medium_IsMounted(_medium)) {
    GWEN_TYPE_UINT32 pid;

    pid=AB_Banking_ProgressStart(_app->getCInterface(),
                                 "Mounting Medium",
                                 "We are now mounting this card... ",
                                 1);

    if (AH_Medium_Mount(_medium)) {
      QMessageBox::critical(this,
                            tr("Medium Error"),
                            tr("Could not mount the medium.\n"
                               "Please check the console logs."),
                            QMessageBox::Ok,QMessageBox::NoButton);
      _medium=0;
      AB_Banking_ProgressEnd(_app->getCInterface(), pid);
      return false;
    }
    AB_Banking_ProgressAdvance(_app->getCInterface(),
                               pid, 1);
    AB_Banking_ProgressEnd(_app->getCInterface(), pid);
  }
  _mediumCreated=false;

  return doSelectCheckFileCardPage(p);
}



bool Wizard::undoCheckCardPage(QWidget *p){
  return undoSelectCheckFileCardPage(p);
}






