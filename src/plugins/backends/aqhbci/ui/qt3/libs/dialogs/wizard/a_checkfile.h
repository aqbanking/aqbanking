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


#ifndef AQHBCI_A_CHECKFILE_H
#define AQHBCI_A_CHECKFILE_H

#include "waction.h"
#include "winfo.h"

class ActionWidget;


class ActionCheckFile: public WizardAction {
  Q_OBJECT
private:
  ActionWidget *_realDialog;

public:
  ActionCheckFile(Wizard *w);
  virtual ~ActionCheckFile();

  virtual void enter();
  virtual void leave(bool backward);
  virtual bool apply();
  virtual bool undo();

public slots:
  void slotButtonClicked();

};


#endif
