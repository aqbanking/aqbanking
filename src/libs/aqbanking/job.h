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
#include <gwenhywfar/gwentime.h>
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

/** This function frees all jobs contained in the given list. */
AQBANKING_API
void AB_Job_List2_FreeAll(AB_JOB_LIST2 *jl);


/** The status of a job. */
typedef enum {
  /** Job is new and not yet enqueued. */
  AB_Job_StatusNew=0,
  /** job has been updated by the backend and is still not yet enqueued. */
  AB_Job_StatusUpdated,
  /** Job has been enqueued, i.e. it has not yet been sent, but will
      be sent at the next AB_BANKING_ExecuteQueue(). These jobs are
      stored in the "todo" directory. */
  AB_Job_StatusEnqueued,
  /** Job has been sent, but there is not yet any response. */
  AB_Job_StatusSent,
  /** Job has been sent, and an answer has been received, so the Job
      has been successfully sent to the bank. However, the answer to
      this job said that the job is still pending at the bank server.
      This status is most likely used with transfer orders which are
      accepted by the bank server but checked (and possibly rejected)
      later. These jobs are stored in the "pending" directory.*/
  AB_Job_StatusPending,
  /** Job has been sent, a response has been received, and everything
      has been sucessfully executed. These jobs are stored in the
      "finished" directory. */
  AB_Job_StatusFinished,
  /** There was an error in jobs' execution. FIXME: Does this mean the
      job is enqueued, sent, pending, or none of these? How can the
      App be sure that the job isn't accidentally enqueued again? */
  AB_Job_StatusError,
  /** Jobs was enqueued but then deferred i.e. removed from the queue,
      and nothing will happen anymore with this job. */
  AB_Job_StatusDeferred,
  /** Unknown status */
  AB_Job_StatusUnknown=999
} AB_JOB_STATUS;


/** The type of the job, which also corresponds to its subclass of AB_JOB. */
typedef enum {
  /** unknown job */
  AB_Job_TypeUnknown=0,
  /** retrieve the balance of an online account */
  AB_Job_TypeGetBalance,
  /** retrieve transaction statements for an online account */
  AB_Job_TypeGetTransactions,
  /** issue a transfer */
  AB_Job_TypeTransfer,
  /** issue a debit note (Lastschrift) */
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
 * identify a specific job. However, unique ids are assigned when they get
 * enqueued (i.e. by calling @ref AB_Banking_EnqueueJob).
 */
AQBANKING_API
GWEN_TYPE_UINT32 AB_Job_GetJobId(const AB_JOB *j);

/**
 * Returns the name of the application which created this job.
 */
AQBANKING_API
const char *AB_Job_GetCreatedBy(const AB_JOB *j);


/**
 * Returns a GWEN_DB_NODE which can be used to store/retrieve data for
 * the currently running application. The group returned MUST NOT be
 * freed !
 * AqBanking is able to separate and store the data for every application.
 */
AQBANKING_API 
GWEN_DB_NODE *AB_Job_GetAppData(AB_JOB *j);


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
 * Returns the time when the status of this job changed last.
 */
AQBANKING_API
const GWEN_TIME *AB_Job_GetLastStatusChange(const AB_JOB *j);

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




