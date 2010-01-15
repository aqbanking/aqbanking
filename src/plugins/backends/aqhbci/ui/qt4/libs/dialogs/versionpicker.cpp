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



#include "versionpicker.h"
#include <qcombobox.h>


const int VersionPicker::_versionTable[3]={201,210, 220};



VersionPicker::VersionPicker(int version,
                             QWidget* parent,
                             const char* name,
                             bool modal,
                             WFlags fl)
:VersionPickerUi(parent, name, modal, fl), _version(version) {
  int i;

  // preselect version
  for (i=0; i<3; i++)
    if (version==_versionTable[i]) {
      versionBox->setCurrentItem(i);
      break;
    }
}



VersionPicker::~VersionPicker(){
}



int VersionPicker::getVersion() const{
  int i;

  i=versionBox->currentItem();
  if (i>=0 && i<3)
    return _versionTable[i];

  return -1;
}



