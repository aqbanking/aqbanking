/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004-2013 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_JOBGETBALANCE_P_H
#define AQBANKING_JOBGETBALANCE_P_H


#include "jobgetbalance_l.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AB_JOBGETBALANCE AB_JOBGETBALANCE;
struct AB_JOBGETBALANCE {
  AB_ACCOUNT_STATUS *accountStatus;
};
static void GWENHYWFAR_CB AB_JobGetBalance_FreeData(void *bp, void *p);


#ifdef __cplusplus
}
#endif


#endif /* AQBANKING_JOBGETBALANCE_P_H */

