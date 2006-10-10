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


#ifndef AH_JOB_L_H
#define AH_JOB_L_H

typedef struct AH_JOB AH_JOB;


#define AH_JOB_FLAGS_NOITAN         0x00002000
#define AH_JOB_FLAGS_TANUSED        0x00004000
#define AH_JOB_FLAGS_NOSYSID        0x00008000
#define AH_JOB_FLAGS_NEEDCRYPT      0x00010000
#define AH_JOB_FLAGS_NEEDSIGN       0x00020000
#define AH_JOB_FLAGS_ATTACHABLE     0x00040000
#define AH_JOB_FLAGS_SINGLE         0x00080000
#define AH_JOB_FLAGS_DLGJOB         0x00100000
#define AH_JOB_FLAGS_CRYPT          0x00200000
#define AH_JOB_FLAGS_SIGN           0x00400000
#define AH_JOB_FLAGS_MULTIMSG       0x00800000
#define AH_JOB_FLAGS_HASATTACHPOINT 0x01000000
#define AH_JOB_FLAGS_HASMOREMSGS    0x02000000
#define AH_JOB_FLAGS_HASWARNINGS    0x04000000
#define AH_JOB_FLAGS_HASERRORS      0x08000000
#define AH_JOB_FLAGS_PROCESSED      0x10000000
#define AH_JOB_FLAGS_COMMITTED      0x20000000
#define AH_JOB_FLAGS_NEEDTAN        0x40000000
#define AH_JOB_FLAGS_OUTBOX         0x80000000

#include <gwenhywfar/misc.h>
#include <gwenhywfar/list2.h>
#include <gwenhywfar/inherit.h>

GWEN_LIST_FUNCTION_DEFS(AH_JOB, AH_Job);
GWEN_INHERIT_FUNCTION_DEFS(AH_JOB);
GWEN_LIST2_FUNCTION_DEFS(AH_JOB, AH_Job);
void AH_Job_List2_FreeAll(AH_JOB_LIST2 *jl);

#include "hbci_l.h"
#include "result_l.h"
#include <aqhbci/aqhbci.h>

#include <aqbanking/user.h>
#include <aqbanking/job.h>

#include <gwenhywfar/db.h>


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
typedef int (*AH_JOB_PROCESS_FN)(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);
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
AH_JOB *AH_Job_new(const char *name,
                   AB_USER *u,
                   const char *accountId);
void AH_Job_free(AH_JOB *j);
void AH_Job_Attach(AH_JOB *j);
/*@}*/


/** @name Informational Functions
 *
 */
/*@{*/
const char *AH_Job_GetName(const AH_JOB *j);
const char *AH_Job_GetAccountId(const AH_JOB *j);
const char *AH_Job_GetDescription(const AH_JOB *j);

int AH_Job_GetMinSignatures(const AH_JOB *j);
int AH_Job_GetJobsPerMsg(const AH_JOB *j);

AB_USER *AH_Job_GetUser(const AH_JOB *j);

GWEN_DB_NODE *AH_Job_GetParams(const AH_JOB *j);
GWEN_DB_NODE *AH_Job_GetArguments(const AH_JOB *j);
GWEN_DB_NODE *AH_Job_GetResponses(const AH_JOB *j);

unsigned int AH_Job_GetMsgNum(const AH_JOB *j);
const char *AH_Job_GetDialogId(const AH_JOB *j);

AH_JOB_STATUS AH_Job_GetStatus(const AH_JOB *j);
const char *AH_Job_StatusName(AH_JOB_STATUS st);

void AH_Job_AddSigner(AH_JOB *j, const char *s);

int AH_Job_HasWarnings(const AH_JOB *j);
int AH_Job_HasErrors(const AH_JOB *j);

AH_RESULT_LIST *AH_Job_GetSegResults(const AH_JOB *j);
AH_RESULT_LIST *AH_Job_GetMsgResults(const AH_JOB *j);

/*@}*/


/** @name Virtual Functions
 *
 */
/*@{*/
int AH_Job_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);
int AH_Job_Commit(AH_JOB *j);
/** exchanges data between the HBCI job and the banking job
 */
int AH_Job_Exchange(AH_JOB *j, AB_JOB *bj,
                    AH_JOB_EXCHANGE_MODE m);

/**
 * Check whether the results for this job contain warning 3920. In this
 * case the result must be commited.
 * @return 0 if there is no such result, !=0 otherwise
 */
int AH_Job_HasItanResult(AH_JOB *j);


/**
 * You can use this from the Commit function of the inheriting class to
 * additionally let the job do some basic stuff (like saving UPD, BPD,
 * messages etc).
 */
int AH_Job_CommitSystemData(AH_JOB *j);


/**
 * You can use this from the Process function of the inheriting class to
 * additionally let the job do some basic stuff (like catching UPD, BPD,
 * messages etc).
 */
int AH_Job_DefaultProcessHandler(AH_JOB *j);

/**
 * You can use this from the Commit function of the inheriting class.
 * It calls @ref AH_Job_CommitSystemData.
 */
int AH_Job_DefaultCommitHandler(AH_JOB *j);
/*@}*/


/** @name Setters For Virtual Functions
 *
 */
/*@{*/
void AH_Job_SetProcessFn(AH_JOB *j, AH_JOB_PROCESS_FN f);
void AH_Job_SetCommitFn(AH_JOB *j, AH_JOB_COMMIT_FN f);
void AH_Job_SetExchangeFn(AH_JOB *j, AH_JOB_EXCHANGE_FN f);
void AH_Job_SetNextMsgFn(AH_JOB *j, AH_JOB_NEXTMSG_FN f);
/*@}*/


void AH_Job_Dump(const AH_JOB *j, FILE *f, unsigned int insert);

const GWEN_STRINGLIST *AH_Job_GetLogs(const AH_JOB *j);


GWEN_TYPE_UINT32 AH_Job_GetFirstSegment(const AH_JOB *j);
void AH_Job_SetFirstSegment(AH_JOB *j, GWEN_TYPE_UINT32 i);

GWEN_TYPE_UINT32 AH_Job_GetLastSegment(const AH_JOB *j);
void AH_Job_SetLastSegment(AH_JOB *j, GWEN_TYPE_UINT32 i);

int AH_Job_HasSegment(const AH_JOB *j, int seg);

/**
 * Takes over ownership of the given group.
 */
void AH_Job_AddResponse(AH_JOB *j, GWEN_DB_NODE *db);

void AH_Job_SetStatus(AH_JOB *j, AH_JOB_STATUS st);

GWEN_XMLNODE *AH_Job_GetXmlNode(const AH_JOB *j);


const GWEN_STRINGLIST *AH_Job_GetSigners(const AH_JOB *j);

int AH_Job_PrepareNextMessage(AH_JOB *j);

void AH_Job_SetMsgNum(AH_JOB *j, unsigned int i);
void AH_Job_SetDialogId(AH_JOB *j, const char *s);

GWEN_TYPE_UINT32 AH_Job_GetFlags(const AH_JOB *j);
void AH_Job_SetFlags(AH_JOB *j, GWEN_TYPE_UINT32 f);
void AH_Job_AddFlags(AH_JOB *j, GWEN_TYPE_UINT32 f);
void AH_Job_SubFlags(AH_JOB *j, GWEN_TYPE_UINT32 f);


GWEN_TYPE_UINT32 AH_Job_GetId(const AH_JOB *j);
void AH_Job_SetId(AH_JOB *j, GWEN_TYPE_UINT32 i);

AH_HBCI *AH_Job_GetHbci(const AH_JOB *j);
AB_BANKING *AH_Job_GetBankingApi(const AH_JOB *j);


const char *AH_Job_GetExpectedSigner(const AH_JOB *j);
void AH_Job_SetExpectedSigner(AH_JOB *j, const char *s);
const char *AH_Job_GetExpectedCrypter(const AH_JOB *j);
void AH_Job_SetExpectedCrypter(AH_JOB *j, const char *s);

int AH_Job_CheckEncryption(AH_JOB *j, GWEN_DB_NODE *dbRsp);
int AH_Job_CheckSignature(AH_JOB *j, GWEN_DB_NODE *dbRsp);

const char *AH_Job_GetUsedTan(const AH_JOB *j);
void AH_Job_SetUsedTan(AH_JOB *j, const char *s);

void AH_Job_Log(AH_JOB *j, AB_BANKING_LOGLEVEL ll, const char *txt);


#endif /* AH_JOB_L_H */




