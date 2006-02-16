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

#include <gwenhywfar/list2.h>
#include <gwenhywfar/gwentime.h>
#include <aqbanking/error.h> /* for AQBANKING_API */

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup G_AB_JOB Online Banking Tasks
 * @ingroup G_AB_C_INTERFACE
 *
 * This group represents online banking tasks such as retrieving the balance,
 * downloading transaction statements, issue transfers etc.
 */

  /** @defgroup G_AB_JOBS_GETBALANCE Get Balance
   * @ingroup G_AB_JOB
   */

  /** @defgroup G_AB_JOBS_GETTRANSACTIONS Get Transaction Statements
   * @ingroup G_AB_JOB
   */

  /** @defgroup G_AB_JOBS_XFERS Transfer Jobs
   * @ingroup G_AB_JOB
   */
  /** @defgroup G_AB_JOBS_XFER_TRANSFER National Transfer
   * @ingroup G_AB_JOBS_XFERS
   */
  /** @defgroup G_AB_JOBS_XFER_DEBIT Debit Notes
   * @ingroup G_AB_JOBS_XFERS
   */
  /** @defgroup G_AB_JOBS_XFER_EU EU Transfer
   * @ingroup G_AB_JOBS_XFERS
   */
  /** @defgroup G_AB_JOBS_XFER_INTERNAL Internal Transfer
   * @ingroup G_AB_JOBS_XFERS
   */


  /** @defgroup G_AB_JOBS_DATED_TRANSFER Dated Transfers
   * @ingroup G_AB_JOB
   *
   * This is a scheduled transaction managed by the credit institute.
   */
  /** @defgroup G_AB_JOBS_DATED_TRANSFER_MK Create a Dated Transfer
   * @ingroup G_AB_JOBS_DATED_TRANSFER
   */
  /** @defgroup G_AB_JOBS_DATED_TRANSFER_MOD Modify a Dated Transfer
   * @ingroup G_AB_JOBS_DATED_TRANSFER
   */
  /** @defgroup G_AB_JOBS_DATED_TRANSFER_DEL Delete a Dated Transfer
   * @ingroup G_AB_JOBS_DATED_TRANSFER
   */
  /** @defgroup G_AB_JOBS_DATED_TRANSFER_GET Get Dated Transfers
   * @ingroup G_AB_JOBS_DATED_TRANSFER
   */


  /** @defgroup G_AB_JOBS_STO Standing Orders
   * @ingroup G_AB_JOB
   */
  /** @defgroup G_AB_JOBS_STO_MK Create a Standing Order
   * @ingroup G_AB_JOBS_STO
   */
  /** @defgroup G_AB_JOBS_STO_MOD Modify a Standing Order
   * @ingroup G_AB_JOBS_STO
   */
  /** @defgroup G_AB_JOBS_STO_DEL Delete a Standing Order
   * @ingroup G_AB_JOBS_STO
   */
  /** @defgroup G_AB_JOBS_STO_GET Get Standing Orders
   * @ingroup G_AB_JOBS_STO
   */



/** @addtogroup G_AB_JOB
 *
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
  /** There was an error in jobs' execution. These jobs are stored in the
   * "finished" directory. Jobs are never enqueued twice for execution,
   * so if it has this status it will never be sent again.
   */
  AB_Job_StatusError,
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
  AB_Job_TypeDebitNote,
  /** EU transfer (transfer within the EMU zone) */
  AB_Job_TypeEuTransfer,
  /** retrieve list of standing orders for an online account */
  AB_Job_TypeGetStandingOrders,
  /** retrieve list of dated transfers for an online account */
  AB_Job_TypeGetDatedTransfers,
  /* creates a new standing order */
  AB_Job_TypeCreateStandingOrder,
  /* modifies an existing standing order */
  AB_Job_TypeModifyStandingOrder,
  /* deletes an existing standing order */
  AB_Job_TypeDeleteStandingOrder,
  /* creates a new dated transfer */
  AB_Job_TypeCreateDatedTransfer,
  /* modifies an existing dated transfer */
  AB_Job_TypeModifyDatedTransfer,
  /* deletes an existing dated transfer */
  AB_Job_TypeDeleteDatedTransfer,
  /* internal transfer between two accounts at the same bank */
  AB_Job_TypeInternalTransfer
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

AQBANKING_API
const char *AB_Job_GetUsedTan(const AB_JOB *j);
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

/**
 * Transforms the given job type into a localized string which can be
 * presented to the user.
 */
AQBANKING_API
const char *AB_Job_Type2LocalChar(AB_JOB_TYPE i);

/*@}*/



/** @name Logging Functions
 *
 */
/*@{*/
AQBANKING_API
void AB_Job_Log(AB_JOB *j,
		AB_BANKING_LOGLEVEL ll,
                const char *who,
		const char *txt);

AQBANKING_API
GWEN_STRINGLIST *AB_Job_GetLogs(const AB_JOB *j);

/*@}*/


/*@}*/ /* defgroup */


#ifdef __cplusplus
}
#endif


#endif /* AQBANKING_JOB_H */




