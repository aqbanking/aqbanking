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


#ifndef AQBANKING_JOB_H
#define AQBANKING_JOB_H

#include <gwenhywfar/misc2.h>
#include <aqbanking/error.h> /* for AQBANKING_API */

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup AB_JOB AB_JOB (Online Banking Tasks)
 * @ingroup AB_C_INTERFACE
 *
 * This group represents online banking tasks such as retrieving the balance,
 * downloading transaction statements, issue transfers etc.
 */
/*@{*/

typedef struct AB_JOB AB_JOB;

GWEN_LIST2_FUNCTION_LIB_DEFS(AB_JOB, AB_Job, AQBANKING_API)
/* Do not terminate this line with semicolon because they are
   macros, not functions, and ISO C89 does not allow a semicolon
   there. */

AQBANKING_API
void AB_Job_List2_FreeAll(AB_JOB_LIST2 *jl);



typedef enum {
  AB_Job_StatusNew=0,
  AB_Job_StatusEnqueued,
  AB_Job_StatusSent,
  AB_Job_StatusAnswered,
  AB_Job_StatusError,
  AB_Job_StatusUnknown=999
} AB_JOB_STATUS;


typedef enum {
  AB_Job_TypeUnknown=0,
  AB_Job_TypeGetBalance,
  AB_Job_TypeGetTransactions,
  AB_Job_TypeTransfer,
  AB_Job_TypeDebitNote
} AB_JOB_TYPE;



#ifdef __cplusplus
}
#endif


#include <aqbanking/account.h>


#ifdef __cplusplus
extern "C" {
#endif

/** @name Constructing, Destroying, Attaching
 *
 * Actually this group does not contain a constructor since you never
 * create an AB_JOB directly. You rather create a derived job (e.g. by using
 * @ref AB_JobGetBalance_new).
 */
/*@{*/
AQBANKING_API
void AB_Job_free(AB_JOB *j);
AQBANKING_API
void AB_Job_Attach(AB_JOB *j);
/*@}*/


/** @name Informational Functions
 *
 */
/*@{*/

/**
 * Every created job gets an unique id. This allows any application to
 * identify a specific job.
 */
AQBANKING_API
GWEN_TYPE_UINT32 AB_Job_GetJobId(const AB_JOB *j);

/**
 * Returns the name of the application which created this job.
 */
AQBANKING_API
const char *AB_Job_GetCreatedBy(const AB_JOB *j);

/**
 * Not all jobs have to be supported by every backend. The application needs
 * to know whether a job actually @b is supported, and this is done by calling
 * this function. It returns the error code (see @ref AB_ERROR) returned
 * by the backend when asked to check for this job.
 */
AQBANKING_API
int AB_Job_CheckAvailability(AB_JOB *j);

/**
 * Returns the status of this job.
 */
AQBANKING_API
AB_JOB_STATUS AB_Job_GetStatus(const AB_JOB *j);

/**
 * Returns the job type.
 */
AQBANKING_API
AB_JOB_TYPE AB_Job_GetType(const AB_JOB *j);

/**
 * Every job is linked to a single account to operate on.
 */
AQBANKING_API
AB_ACCOUNT *AB_Job_GetAccount(const AB_JOB *j);

/**
 * Returns a text result provided by the backend upon execution of this
 * job. This should only be presented to the user when there is no other
 * way to determine the result (e.g. no log etc).
 */
AQBANKING_API
const char *AB_Job_GetResultText(const AB_JOB *j);
/*@}*/


/** @name Helper Functions
 *
 */
/*@{*/
/**
 * Transforms the given status code into a string.
 */
AQBANKING_API
const char *AB_Job_Status2Char(AB_JOB_STATUS i);
/**
 * Transforms the given string into a job status code.
 */
AQBANKING_API
AB_JOB_STATUS AB_Job_Char2Status(const char *s);

/**
 * Transforms the given job type into a string.
 */
AQBANKING_API
const char *AB_Job_Type2Char(AB_JOB_TYPE i);
/**
 * Transforms the given string into a job type.
 */
AQBANKING_API
AB_JOB_TYPE AB_Job_Char2Type(const char *s);
/*@}*/


/*@}*/ /* defgroup */


#ifdef __cplusplus
}
#endif


#endif /* AQBANKING_JOB_H */




