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
#include "selectcontext.h"
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


#include <gwenhywfar/debug.h>



bool Wizard::initInitModePage() {
  QObject::connect((QObject*)(firstInitRadio),
                   SIGNAL(toggled(bool)),
                   this,
                   SLOT(slotFirstInitToggled(bool)));

  return true;
}


// Note: The text field is set up in Wizard::enterPage() in
// wizard.cpp.

bool Wizard::doInitModePage(QWidget *p){
  return true;
}



bool Wizard::undoInitModePage(QWidget *p){
  return true;
}



void Wizard::slotFirstInitToggled(bool on) {
  //setAppropriate(sendKeysPage, on);
  //setAppropriate(iniLetterPage, on);
  _firstInitMode=firstInitRadio->isOn();
  //_adjustToMedium(_medium);
}








