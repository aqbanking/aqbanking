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


#ifndef AQHBCI_MKKEYS_H
#define AQHBCI_MKKEYS_H

#include "mkkeys.ui.h"


class Wizard;



class MakeKeys : public MakeKeysUi {
  Q_OBJECT
private:
  Wizard *_wizard;
  bool _result;
public:
  MakeKeys(Wizard *w,
          QWidget* parent = 0, const char* name = 0, WFlags fl = 0);
  ~MakeKeys();

  bool getResult() const;

public slots:
  void slotMakeKeys();

};



#endif

