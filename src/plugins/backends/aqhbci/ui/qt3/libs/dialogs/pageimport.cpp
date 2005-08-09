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

#if !defined(__GNUC__) && defined(WIN32)
// This is MSVC compiler which doesnt have snprintf()
# define snprintf _snprintf
#endif

bool Wizard::initImportPage() {
  bool on;

  setNextEnabled(importPage, true);
  on=importRadio->isOn();
  slotImportToggled(on);

  // page 1
  QObject::connect((QObject*)(importRadio),
                   SIGNAL(toggled(bool)),
                   this,
                   SLOT(slotImportToggled(bool)));
  QObject::connect((QObject*)(createRadio),
                   SIGNAL(toggled(bool)),
                   this,
                   SLOT(slotNewToggled(bool)));
  QObject::connect((QObject*)(pinTanRadio),
                   SIGNAL(toggled(bool)),
                   this,
                   SLOT(slotPinTanToggled(bool)));
  return true;
}



bool Wizard::doImportPage(QWidget *p) {
  if (pinTanRadio->isOn()) {
    char buffer[64];
    time_t currentTime;
    struct tm *currentTimeTm;
    int rv;
  
    currentTime=time(0);
    currentTimeTm=localtime(&currentTime);
    assert(currentTimeTm);
  
    rv=snprintf(buffer,
                sizeof(buffer)-1, "Medium_%04d%02d%02d-%02d%02d%02d",
                currentTimeTm->tm_year+1900,
                currentTimeTm->tm_mon+1,
                currentTimeTm->tm_mday,
                currentTimeTm->tm_hour,
                currentTimeTm->tm_min,
                currentTimeTm->tm_sec);
    assert(rv>0 && rv<(int)(sizeof(buffer)));
    _mediumName=buffer;
    _medium=AH_HBCI_MediumFactory(_hbci,
                                  "PinTan",
                                  0,
                                  _mediumName.c_str());

    if (!_medium) {
      DBG_ERROR(0, "PIN/TAN plugin not installed");
      QMessageBox::critical(0,
                            tr("Medium Error"),
                            tr("Plugin for PIN/TAN does not exist.\n"
                               "Please check the console logs."),
                            tr("Dismiss"),0,0,0);
      return false;
    }
    if (AH_Medium_Create(_medium)) {
      QMessageBox::critical(0,
                            tr("Medium Error"),
                            tr("Could not create the medium.\n"
                               "Please check the console logs."),
                            tr("Dismiss"),0,0,0);
      AH_Medium_free(_medium);
      _medium=0;
      return false;
    }
    AH_HBCI_AddMedium(_hbci, _medium);
    DBG_INFO(0, "New medium created");
    _mediumCreated=true;
  }
  return true;
}



bool Wizard::undoImportPage(QWidget *p) {
  return true;
}



void Wizard::slotImportToggled(bool on) {
  DBG_NOTICE(0, "Current import mode: %s",
             on?"YES":"NO");
  setAppropriate(mediumPage, on);
  setAppropriate(checkCardPage, on);
  setAppropriate(checkFilePage, on);
  setAppropriate(selectFilePage, !on);
  setAppropriate(serverTestPage, on);
  //setAppropriate(serverKeysPage, !on);
  //setAppropriate(verifyKeysPage, !on);
  //setAppropriate(createKeysPage, !on);
  //setAppropriate(sendKeysPage, !on);
  //setAppropriate(systemIdPage, on);
  _importMode=on;
}



void Wizard::slotNewToggled(bool on) {
  setAppropriate(mediumPage, on);
  setAppropriate(checkCardPage, !on);
  setAppropriate(checkFilePage, !on);
  setAppropriate(selectFilePage, on);
  setAppropriate(serverTestPage, on);
  _importMode=!on;
}



void Wizard::slotPinTanToggled(bool on) {
  setAppropriate(mediumPage, !on);
  setAppropriate(checkCardPage, !on);
  setAppropriate(checkFilePage, !on);
  setAppropriate(selectFilePage, !on);
  setAppropriate(serverTestPage, !on);
  _importMode=!on;
}























