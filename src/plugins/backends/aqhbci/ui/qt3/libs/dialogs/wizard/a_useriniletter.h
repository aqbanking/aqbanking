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


#ifndef AQHBCI_A_USER_INILETTER_H
#define AQHBCI_A_USER_INILETTER_H

#include "waction.h"
#include "winfo.h"

#include <gwenhywfar/crypt.h>


class IniLetter;


class ActionUserIniLetter: public WizardAction {
  Q_OBJECT
private:
  IniLetter *_iniLetterDialog;
  GWEN_CRYPTKEY *_key;
public:
  ActionUserIniLetter(Wizard *w);
  virtual ~ActionUserIniLetter();

  virtual void enter();

public slots:
  void slotPrint();
};


#endif
