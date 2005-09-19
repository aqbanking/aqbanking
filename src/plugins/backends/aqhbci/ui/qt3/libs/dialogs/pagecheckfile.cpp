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




bool Wizard::initCheckFilePage() {
  QObject::connect((QObject*)(checkFileButton),
                   SIGNAL(clicked()),
                   this,
                   SLOT(slotCheckFile()));
  return true;
}



void Wizard::slotCheckFile(){
  GWEN_BUFFER *typeBuf;
  GWEN_BUFFER *subtypeBuf;
  GWEN_BUFFER *nameBuf;
  QString fname;

  fname=fileNameEdit->text();
  typeBuf=GWEN_Buffer_new(0, 64, 0, 1);
  subtypeBuf=GWEN_Buffer_new(0, 64, 0, 1);
  nameBuf=GWEN_Buffer_new(0, fname.length(), 0, 1);
  GWEN_Buffer_AppendString(nameBuf, fname.local8Bit());

  if (AH_HBCI_CheckMedium(_hbci,
                          GWEN_CryptToken_Device_File,
                          typeBuf,
                          subtypeBuf,
			  nameBuf)) {
    GWEN_Buffer_free(nameBuf);
    GWEN_Buffer_free(subtypeBuf);
    GWEN_Buffer_free(typeBuf);
    checkFileLabel->setText(_ResultMsg_NotSupported);

  }
  else {
    checkFileLabel->setText(_ResultMsg_Supported);
    setNextEnabled(checkFilePage, true);
    checkFileButton->setEnabled(false);
    _mediumTypeName=GWEN_Buffer_GetStart(typeBuf);
    _mediumSubTypeName=GWEN_Buffer_GetStart(subtypeBuf);
    GWEN_Buffer_free(nameBuf);
    GWEN_Buffer_free(subtypeBuf);
    GWEN_Buffer_free(typeBuf);
  }
}



bool Wizard::doCheckFilePage(QWidget *p){
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
    if (AH_Medium_Mount(_medium)) {
      QMessageBox::critical(this,
                            tr("Medium Error"),
                            tr("Could not mount the medium.\n"
                               "Please check the console logs."),
                            QMessageBox::Ok,QMessageBox::NoButton);
      _medium=0;
      return false;
    }
  }
  _mediumCreated=false;

  return doSelectCheckFileCardPage(p);
}



bool Wizard::undoCheckFilePage(QWidget *p){
  return undoSelectCheckFileCardPage(p);
}












