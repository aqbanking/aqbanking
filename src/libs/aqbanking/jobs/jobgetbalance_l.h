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


#ifndef AQBANKING_JOBGETBALANCE_L_H
#define AQBANKING_JOBGETBALANCE_L_H


#include <aqbanking/jobgetbalance.h>

#ifdef __cplusplus
extern "C" {
#endif


AB_JOB *AB_JobGetBalance_fromDb(AB_ACCOUNT *a, GWEN_DB_NODE *db);
int AB_JobGetBalance_toDb(const AB_JOB *j, GWEN_DB_NODE *db);

void AB_JobGetBalance_SetAccountStatus(AB_JOB *j,
                                       const AB_ACCOUNT_STATUS *as);


#ifdef __cplusplus
}
#endif


#endif /* AQBANKING_JOBGETBALANCE_L_H */

