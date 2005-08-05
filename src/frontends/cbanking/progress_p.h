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


#ifndef CBANKING_PROGRESS_H
#define CBANKING_PREGRESS_H

#define CBANKING_PROGRESS_CHAR_ABORT 27

#include "progress_l.h"


struct CBANKING_PROGRESS {
  GWEN_LIST_ELEMENT(CBANKING_PROGRESS)
  AB_BANKING *banking;
  GWEN_TYPE_UINT32 id;
  char *title;
  char *text;
  GWEN_TYPE_UINT32 total;
  GWEN_TYPE_UINT32 current;
  GWEN_BUFFER *logBuf;
  int aborted;
};




#endif

