/***************************************************************************
 begin       : Sat Nov 29 2025
 copyright   : (C) 2025 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_DLG_VOP_P_H
#define AQHBCI_DLG_VOP_P_H

#include "./dlg_vop.h"


typedef struct AH_VOP_DIALOG AH_VOP_DIALOG;
struct AH_VOP_DIALOG {
  const AH_VOP_RESULT_LIST *resultList;
  char *vopMsg;
  char *jobName;
  char *bankName;
  char *userName;
};



#endif

