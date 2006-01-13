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


#ifndef QBANKING_WCB_SIMPLE_H
#define QBANKING_WCB_SIMPLE_H

#include "qbwcb_progress.h"

class QBProgress;


class QBSimpleCallback: public QBProgressCallback {
public:
  QBSimpleCallback(const char *id);
  virtual ~QBSimpleCallback();
  virtual QBWaitCallback *instantiate();
};



#endif

