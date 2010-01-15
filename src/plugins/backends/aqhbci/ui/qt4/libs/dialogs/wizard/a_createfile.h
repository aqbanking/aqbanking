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


#ifndef AQHBCI_A_CREATEFILE_H
#define AQHBCI_A_CREATEFILE_H

#define AQHBCI_A_CREATEFILE_CT_TYPE "ohbci"


#include "waction.h"
#include "winfo.h"
#include "a_selectfile.h"

#include <qstring.h>



class ActionCreateFile: public ActionSelectFile {
public:
  ActionCreateFile(Wizard *w);
  virtual ~ActionCreateFile();

  virtual bool apply();
  virtual bool undo();
};


#endif
