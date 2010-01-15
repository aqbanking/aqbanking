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


#ifndef AQHBCI_A_SELECTFILE_H
#define AQHBCI_A_SELECTFILE_H

#include "waction.h"
#include "winfo.h"
#include "selectfile.ui.h"

#include <qstring.h>


class ActionSelectFile: public WizardAction {
  Q_OBJECT
private:
  bool _mustExist;
  Ui_SelectFileUi _realDialog;
public:
  ActionSelectFile(Wizard *w, bool mustExist,
                   const QString &title,
                   const QString &descr);
  virtual ~ActionSelectFile();

  virtual void enter();
  virtual bool apply();

public slots:
  void slotFileButtonClicked();
  void slotFileNameChanged(const QString &qs);

};


#endif
