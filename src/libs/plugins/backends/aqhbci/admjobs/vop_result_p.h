/***************************************************************************
    begin       : Sat Oct 18 2025
    copyright   : (C) 2025 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_VPP_RESULT_P_H
#define AH_VPP_RESULT_P_H


#include "aqhbci/admjobs/vop_result.h"


struct AH_VOP_RESULT {
  GWEN_LIST_ELEMENT(AH_VOP_RESULT);
  char *localBic;
  char *remoteIban;
  char *remoteName;
  char *altRemoteName;
  int result;
};



#endif

