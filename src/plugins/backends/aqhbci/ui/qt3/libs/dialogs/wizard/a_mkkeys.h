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


#ifndef AQHBCI_A_MKKEYS_H
#define AQHBCI_A_MKKEYS_H

#include "waction.h"
#include "winfo.h"

class ActionWidget;


class ActionCreateKeys: public WizardAction {
  Q_OBJECT
private:
  ActionWidget *_realDialog;
public:
  ActionCreateKeys(Wizard *w);
  virtual ~ActionCreateKeys();

  virtual void enter();
  virtual bool apply();

public slots:
  void slotButtonClicked();


};


#endif
