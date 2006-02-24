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


/** @file job_be.h
 * @short This file is used by provider backends.
 */


#ifndef AQBANKING_JOB_BE_H
#define AQBANKING_JOB_BE_H

#include <aqbanking/job.h>

/** @defgroup G_AB_BE_JOB Online Banking Tasks
 * @ingroup G_AB_BE_INTERFACE
 */
/*@{*/


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

AQBANKING_API
void AB_Job_SetUsedTan(AB_JOB *j, const char *s);

/**
 * Reads a GWEN_TIME object from a DB variable.
 * The expected format of the variable is "YYYYMMDD hh:mm:ss" (where YYYY is
 * the year in 4-digit-notion, MM is the number of the month beginning with
 * 1=January, DD is the day of the month beginning with 1, hh is the hour
 * of the day, mm are the minutes of the hour and ss are the seconds of the
 * minute.
 */
AQBANKING_API
GWEN_TIME *AB_Job_DateFromDb(GWEN_DB_NODE *db, const char *name);
AQBANKING_API
void AB_Job_DateToDb(const GWEN_TIME *ti, GWEN_DB_NODE *db, const char *name);

AQBANKING_API
void AB_Job_DateOnlyToDb(const GWEN_TIME *ti,
                         GWEN_DB_NODE *db,
                         const char *name);

/**
 * Reads a GWEN_TIME object from a DB variable ignoring the time part.
 * The expected format of the variable is "YYYYMMDD".
 */
AQBANKING_API
GWEN_TIME *AB_Job_DateOnlyFromDb(GWEN_DB_NODE *db, const char *name);


/**
 * This function should only be used when copying logs from a backend-private
 * job object (e.g. AqHBCI internally uses its own job types) to an
 * AqBanking job.
 * @param j job to operate on
 * @param txt the text to log (it is expected to have the same format as
 * logs created via @ref AB_Job_Log).
 */
AQBANKING_API
void AB_Job_LogRaw(AB_JOB *j, const char *txt);

/*@}*/


#ifdef __cplusplus
}
#endif

/*@}*/ /* defgroup */


#endif /* AQBANKING_JOB_BE_H */




