/***************************************************************************
 begin       : Sat Nov 29 2025
 copyright   : (C) 2025 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_DLG_VOP_H
#define AQHBCI_DLG_VOP_H

//#include <aqhbci/aqhbci.h>
#include "aqhbci/admjobs/vop_result.h"

#include <aqbanking/banking.h>

#include <gwenhywfar/dialog.h>


#ifdef __cplusplus
extern "C" {
#endif


GWEN_DIALOG *AH_VopDialog_new(const char *sJobName, const char *sBankName, const char *sUserName,
                              const char *vopMsg,
                              const AH_VOP_RESULT_LIST *resultList);


#ifdef __cplusplus
}
#endif





#endif

