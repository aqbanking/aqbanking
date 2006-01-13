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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define GWEN_EXTEND_WAITCALLBACK
#include "qbwcb_fast.h"

#include <gwenhywfar/debug.h>



QBFastCallback::QBFastCallback(const char *id)
:QBProgressCallback(id, QBProgress::ProgressTypeFast) {
}



QBFastCallback::~QBFastCallback(){
}



QBWaitCallback *QBFastCallback::instantiate(){
  return new QBFastCallback(getId());
}



