/***************************************************************************
    begin       : Sun Jun 02 2019
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_TAN_MECHANISM_P_H
#define AH_TAN_MECHANISM_P_H


#include "tanmechanism.h"



struct AH_TAN_MECHANISM {
  GWEN_INHERIT_ELEMENT(AH_TAN_MECHANISM)

  AH_TAN_METHOD *tanMethod;
  AH_TAN_MECHANISM_GETTAN_FN getTanFn;

  int tanMethodId;
};






#endif


