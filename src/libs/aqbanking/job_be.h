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


#ifndef AQBANKING_JOB_BE_H
#define AQBANKING_JOB_BE_H

#include <aqbanking/job.h>


#ifdef __cplusplus
extern "C" {
#endif

/** @name Functions To Be Used by Backends
 *
 */
/*@{*/
/**
 * This id can be used by a AB_PROVIDER to map AB_Jobs to whatever the
 * provider uses. This id is not used by AB_Banking itself.
 */
AQBANKING_API
GWEN_TYPE_UINT32 AB_Job_GetIdForProvider(const AB_JOB *j);
AQBANKING_API
void AB_Job_SetIdForProvider(AB_JOB *j, GWEN_TYPE_UINT32 i);

/**
 * Store backend specific data with a job. This data is not specific
 * to an application, it will rather be used with every application (since
 * it doesn't depend on the application but on the backend).
 * @param j pointer to the AB_JOB object
 * @param pro pointer to the backend for which the data is to be returned
 */
AQBANKING_API
GWEN_DB_NODE *AB_Job_GetProviderData(AB_JOB *j, AB_PROVIDER *pro);

AQBANKING_API
void AB_Job_SetResultText(AB_JOB *j, const char *s);
AQBANKING_API
void  AB_Job_SetStatus(AB_JOB *j, AB_JOB_STATUS st);
/*@}*/


#ifdef __cplusplus
}
#endif


#endif /* AQBANKING_JOB_BE_H */




