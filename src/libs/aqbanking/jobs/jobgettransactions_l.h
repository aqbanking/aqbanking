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


#ifndef AQBANKING_JOBGETTRANSACTIONS_L_H
#define AQBANKING_JOBGETTRANSACTIONS_L_H


#include <aqbanking/job.h>
#include <aqbanking/accstatus.h>
#include <aqbanking/jobgettransactions.h>

#ifdef __cplusplus
extern "C" {
#endif


int AB_Job_GetTransactions_ReadDb(AB_JOB *j, GWEN_DB_NODE *db);
int AB_Job_GetTransactions_WriteDb(const AB_JOB *j, GWEN_DB_NODE *db);


#ifdef __cplusplus
}
#endif


#endif /* AQBANKING_JOBGETTRANSACTIONS_L_H */

