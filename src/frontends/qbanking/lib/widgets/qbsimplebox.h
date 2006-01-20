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

#ifndef QBANKING_SIMPLEBOX_H
#define QBANKING_SIMPLEBOX_H


#include "qbsimplebox.ui.h"
#include <gwenhywfar/types.h>
#include <aqbanking/banking.h>



class QBSimpleBox: public QBSimpleBoxUi {
  Q_OBJECT
private:
  GWEN_TYPE_UINT32 _id;

public:
  QBSimpleBox(GWEN_TYPE_UINT32 id,
              const QString& title,
              const QString& text,
              QWidget* parent=0, const char* name=0, WFlags fl=0);
  ~QBSimpleBox();

  GWEN_TYPE_UINT32 getId();
};







#endif /* QBANKING_SIMPLEBOX_H */

