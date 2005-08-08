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

#ifndef AQBANKING_KDE_SIMPLEBOX_H
#define AQBANKING_KDE_SIMPLEBOX_H


#include "simplebox.ui.h"
#include <gwenhywfar/types.h>
#include <aqbanking/banking.h>



class KBSimpleBox: public KBSimpleBoxUi {
  Q_OBJECT
private:
  GWEN_TYPE_UINT32 _id;

public:
  KBSimpleBox(GWEN_TYPE_UINT32 id,
              const QString& title,
              const QString& text,
              QWidget* parent=0, const char* name=0, WFlags fl=0);
  ~KBSimpleBox();

  GWEN_TYPE_UINT32 getId();
};







#endif /* AQBANKING_KDE_SIMPLEBOX_H */

