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

#ifndef AH_RESULT_P_H
#define AH_RESULT_P_H

#include "result_l.h"


struct AH_RESULT {
  GWEN_LIST_ELEMENT(AH_RESULT);
  int code;
  char *text;
  char *ref;
  char *param;
  int isMsgResult;
};




#endif /* AH_RESULT_H */




