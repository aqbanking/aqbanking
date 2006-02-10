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

#ifndef AH_JOB_H
#define AH_JOB_H

#include <aqhbci/aqhbci.h>

#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif
typedef struct AH_JOB AH_JOB;

#include <gwenhywfar/misc.h>
#include <gwenhywfar/list2.h>
#include <gwenhywfar/inherit.h>

GWEN_INHERIT_FUNCTION_DEFS(AH_JOB);
GWEN_LIST2_FUNCTION_DEFS(AH_JOB, AH_Job);
void AH_Job_List2_FreeAll(AH_JOB_LIST2 *jl);
#ifdef __cplusplus
}
#endif



#include <aqhbci/aqhbci.h>
#include <aqhbci/hbci.h>
#include <aqhbci/result.h>
#include <aqbanking/user.h>
#include <aqbanking/job.h>
#include <gwenhywfar/db.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  AH_JobStatusUnknown=0,
  AH_JobStatusToDo,
  AH_JobStatusEnqueued,
  AH_JobStatusEncoded,
  AH_JobStatusSent,
  AH_JobStatusAnswered,
  AH_JobStatusError,

  AH_JobStatusAll=255
} AH_JOB_STATUS;


typedef enum {
  AH_Job_ExchangeModeParams=0,
  AH_Job_ExchangeModeArgs,
  AH_Job_ExchangeModeResults
} AH_JOB_EXCHANGE_MODE;


/** @name Prototypes For Virtual Functions
 *
 */
/*@{*/
typedef int (*AH_JOB_PROCESS_FN)(AH_JOB *j);
typedef int (*AH_JOB_COMMIT_FN)(AH_JOB *j);
typedef int (*AH_JOB_EXCHANGE_FN)(AH_JOB *j, AB_JOB *bj,
                                  AH_JOB_EXCHANGE_MODE m);
/**
 * This function is called on multi-message jobs and should return:
 * <ul>
 *   <li>0 if it is sure that no message is to follow</li>
 *   <li>1 if there might be more message (will be checked by AqHBCI)</li>
 *   <li>any other value (indicating an error)</li>
 * </ul>
 */
typedef int (*AH_JOB_NEXTMSG_FN)(AH_JOB *j);

/*@}*/



/** @name Constructors, Destructors
 *
 */
/*@{*/
AQHBCI_API
void AH_Job_free(AH_JOB *j);
AQHBCI_API
void AH_Job_Attach(AH_JOB *j);

/*@}*/


/** @name Informational Functions
 *
 */
/*@{*/
AQHBCI_API
const char *AH_Job_GetName(const AH_JOB *j);
AQHBCI_API
const char *AH_Job_GetAccountId(const AH_JOB *j);
AQHBCI_API
const char *AH_Job_GetDescription(const AH_JOB *j);

AQHBCI_API
int AH_Job_GetMinSignatures(const AH_JOB *j);
AQHBCI_API
int AH_Job_GetJobsPerMsg(const AH_JOB *j);

AQHBCI_API
AB_USER *AH_Job_GetUser(const AH_JOB *j);

AQHBCI_API
GWEN_DB_NODE *AH_Job_GetParams(const AH_JOB *j);
AQHBCI_API
GWEN_DB_NODE *AH_Job_GetArguments(const AH_JOB *j);
AQHBCI_API
GWEN_DB_NODE *AH_Job_GetResponses(const AH_JOB *j);

AQHBCI_API
unsigned int AH_Job_GetMsgNum(const AH_JOB *j);
AQHBCI_API
const char *AH_Job_GetDialogId(const AH_JOB *j);

AQHBCI_API
AH_JOB_STATUS AH_Job_GetStatus(const AH_JOB *j);
AQHBCI_API
const char *AH_Job_StatusName(AH_JOB_STATUS st);

AQHBCI_API
void AH_Job_AddSigner(AH_JOB *j, const char *s);

AQHBCI_API
int AH_Job_HasWarnings(const AH_JOB *j);
AQHBCI_API
int AH_Job_HasErrors(const AH_JOB *j);

AQHBCI_API
AH_RESULT_LIST *AH_Job_GetSegResults(const AH_JOB *j);
AQHBCI_API
AH_RESULT_LIST *AH_Job_GetMsgResults(const AH_JOB *j);

/*@}*/


/** @name Virtual Functions
 *
 */
/*@{*/
AQHBCI_API
int AH_Job_Process(AH_JOB *j);
AQHBCI_API
int AH_Job_Commit(AH_JOB *j);
/** exchanges data between the HBCI job and the banking job
 */
AQHBCI_API
int AH_Job_Exchange(AH_JOB *j, AB_JOB *bj, AH_JOB_EXCHANGE_MODE m);


/**
 * You can use this from the Commit function of the inheriting class to
 * additionally let the job do some basic stuff (like saving UPD, BPD,
 * messages etc).
 */
AQHBCI_API
int AH_Job_CommitSystemData(AH_JOB *j);


/**
 * You can use this from the Process function of the inheriting class to
 * additionally let the job do some basic stuff (like catching UPD, BPD,
 * messages etc).
 */
AQHBCI_API
int AH_Job_DefaultProcessHandler(AH_JOB *j);

/**
 * You can use this from the Commit function of the inheriting class.
 * It calls @ref AH_Job_CommitSystemData.
 */
AQHBCI_API
int AH_Job_DefaultCommitHandler(AH_JOB *j);
/*@}*/



/** @name Setters For Virtual Functions
 *
 */
/*@{*/
AQHBCI_API
void AH_Job_SetProcessFn(AH_JOB *j, AH_JOB_PROCESS_FN f);
AQHBCI_API
void AH_Job_SetCommitFn(AH_JOB *j, AH_JOB_COMMIT_FN f);
AQHBCI_API
void AH_Job_SetExchangeFn(AH_JOB *j, AH_JOB_EXCHANGE_FN f);
AQHBCI_API
void AH_Job_SetNextMsgFn(AH_JOB *j, AH_JOB_NEXTMSG_FN f);
/*@}*/


AQHBCI_API
void AH_Job_Dump(const AH_JOB *j, FILE *f, unsigned int insert);

AQHBCI_API
const GWEN_STRINGLIST *AH_Job_GetLogs(const AH_JOB *j);


#ifdef __cplusplus
}
#endif


#endif /* AH_JOB_H */




