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


#ifndef AQBANKING_JOBGETTRANSACTIONS_H
#define AQBANKING_JOBGETTRANSACTIONS_H


#include <aqbanking/job.h>
#include <aqbanking/accstatus.h>

#ifdef __cplusplus
extern "C" {
#endif


AQBANKING_API
AB_JOB *AB_JobGetTransactions_new(AB_ACCOUNT *a,
				  GWEN_TIME *fromTime,
				  GWEN_TIME *toTime);

const GWEN_TIME *AB_JobGetTransactions_GetFromTime(const AB_JOB *j);
const GWEN_TIME *AB_JobGetTransactions_GetToTime(const AB_JOB *j);



#ifdef __cplusplus
}
#endif


#endif /* AQBANKING_JOBGETTRANSACTIONS_H */

