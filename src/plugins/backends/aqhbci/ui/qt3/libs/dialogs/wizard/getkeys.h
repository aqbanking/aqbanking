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


#ifndef AQHBCI_GETKEYS_H
#define AQHBCI_GETKEYS_H

#include "getkeys.ui.h"


class Wizard;



class GetKeys : public GetKeysUi {
  Q_OBJECT
private:
  Wizard *_wizard;
  bool _result;
public:
  GetKeys(Wizard *w,
          QWidget* parent = 0, const char* name = 0, WFlags fl = 0);
  ~GetKeys();

  bool getResult() const;

public slots:
  void slotGetKeys();

};



#endif

