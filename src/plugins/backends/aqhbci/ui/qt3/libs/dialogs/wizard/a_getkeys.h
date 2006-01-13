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


#ifndef AQHBCI_A_GETKEYS_H
#define AQHBCI_A_GETKEYS_H

#include "waction.h"
#include "winfo.h"

class ActionWidget;


class ActionGetKeys: public WizardAction {
  Q_OBJECT
private:
  ActionWidget *_realDialog;
public:
  ActionGetKeys(Wizard *w);
  virtual ~ActionGetKeys();

  virtual void enter();
  virtual bool apply();

public slots:
  void slotButtonClicked();

};


#endif
