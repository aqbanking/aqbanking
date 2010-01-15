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



#include "versionpicker.ui.h"



class VersionPicker: public VersionPickerUi {
  Q_OBJECT

private:
  static const int _versionTable[3];
  int _version;
public:
  VersionPicker(int version,
                QWidget* parent=0,
                const char* name=0,
                bool modal=FALSE,
                WFlags fl=0);
  ~VersionPicker();

  int getVersion() const;
};





