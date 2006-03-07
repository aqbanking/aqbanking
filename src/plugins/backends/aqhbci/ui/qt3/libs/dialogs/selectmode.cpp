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

#include "selectmode.h"

#include <qradiobutton.h>
#include <qtimer.h>
#include <gwenhywfar/debug.h>




SelectMode::SelectMode(QWidget* parent, const char* name,
                       bool modal, WFlags fl)
:SelectModeUi(parent, name, modal, fl)
,_mode(ModeUnknown) {

  QTimer::singleShot(0, this, SLOT(adjustSize()));
}


SelectMode::~SelectMode() {
}



void SelectMode::accept() {
  if (importCardRadio->isOn())
    _mode=ModeImportCard;
  if (initCardRadio->isOn())
    _mode=ModeInitCard;
  if (importFileRadio->isOn())
    _mode=ModeImportFile;
  if (createFileRadio->isOn())
    _mode=ModeCreateFile;
  if (pinTanRadio->isOn())
    _mode=ModePinTan;
  SelectModeUi::accept();
}



SelectMode::Mode SelectMode::getMode() const {
  return _mode;
}



SelectMode::Mode SelectMode::selectMode(QWidget* parent) {
  SelectMode w(parent, "SelectMode", TRUE);

  if (w.exec()==QDialog::Accepted) {
    DBG_INFO(0, "Selected %d", w.getMode());
    return w.getMode();
  }
  else {
    DBG_ERROR(0, "Not accepted");
  }

  return ModeUnknown;
}






