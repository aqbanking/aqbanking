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

AQBANKING_API
void AB_Job_SetResultText(AB_JOB *j, const char *s);
AQBANKING_API
void  AB_Job_SetStatus(AB_JOB *j, AB_JOB_STATUS st);
/*@}*/


#ifdef __cplusplus
}
#endif


#endif /* AQBANKING_JOB_BE_H */




