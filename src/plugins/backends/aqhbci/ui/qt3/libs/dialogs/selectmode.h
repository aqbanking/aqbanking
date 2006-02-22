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


#ifndef AQHBCI_SELECTMODE_H
#define AQHBCI_SELECTMODE_H

#include "selectmode.ui.h"



class QBanking;


class SelectMode : public SelectModeUi {
public:
  typedef enum {
    ModeUnknown=0,
    ModeImportCard,
    ModeInitCard,
    ModeImportFile,
    ModeCreateFile,
    ModePinTan
  } Mode;

private:
  Mode _mode;

public:
  SelectMode(QWidget* parent=0, const char* name=0,
             bool modal=FALSE, WFlags fl=0);
  virtual ~SelectMode();

  void accept();

  Mode getMode() const;

  static Mode selectMode(QWidget* parent=0);
};






#endif // AQHBCI_SELECTMODE_H

