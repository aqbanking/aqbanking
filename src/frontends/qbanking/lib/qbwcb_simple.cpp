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
#include "qbwcb_simple.h"

#include <gwenhywfar/debug.h>



QBSimpleCallback::QBSimpleCallback(const char *id)
:QBProgressCallback(id, QBProgress::ProgressTypeSimple) {
}



QBSimpleCallback::~QBSimpleCallback(){
}



QBWaitCallback *QBSimpleCallback::instantiate(){
  return new QBSimpleCallback(getId());
}



