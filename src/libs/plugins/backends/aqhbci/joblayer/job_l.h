/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOB_L_H
#define AH_JOB_L_H

typedef struct AH_JOB AH_JOB;

#define AH_JOB_FLAGS_ACKNOWLEDGE          0x00000200
#define AH_JOB_FLAGS_IGNOREACCOUNTS       0x00000400
#define AH_JOB_FLAGS_SIGNSEQONE           0x00000800
#define AH_JOB_FLAGS_IGNORE_ERROR         0x00001000
#define AH_JOB_FLAGS_NOITAN               0x00002000
#define AH_JOB_FLAGS_TANUSED              0x00004000
#define AH_JOB_FLAGS_NOSYSID              0x00008000
#define AH_JOB_FLAGS_NEEDCRYPT            0x00010000
#define AH_JOB_FLAGS_NEEDSIGN             0x00020000
#define AH_JOB_FLAGS_ATTACHABLE           0x00040000
#define AH_JOB_FLAGS_SINGLE               0x00080000
#define AH_JOB_FLAGS_DLGJOB               0x00100000
#define AH_JOB_FLAGS_CRYPT                0x00200000
#define AH_JOB_FLAGS_SIGN                 0x00400000
#define AH_JOB_FLAGS_MULTIMSG             0x00800000
#define AH_JOB_FLAGS_HASATTACHPOINT       0x01000000
#define AH_JOB_FLAGS_HASMOREMSGS          0x02000000
#define AH_JOB_FLAGS_HASWARNINGS          0x04000000
#define AH_JOB_FLAGS_HASERRORS            0x08000000
#define AH_JOB_FLAGS_PROCESSED            0x10000000
#define AH_JOB_FLAGS_COMMITTED            0x20000000
#define AH_JOB_FLAGS_NEEDTAN              0x40000000
#define AH_JOB_FLAGS_OUTBOX               0x80000000

#define AH_JOB_TANVER_1_4 0x14
#define AH_JOB_TANVER_1_3 0x13


#include <gwenhywfar/misc.h>
#include <gwenhywfar/list2.h>
#include <gwenhywfar/inherit.h>

GWEN_LIST_FUNCTION_DEFS(AH_JOB, AH_Job);
GWEN_INHERIT_FUNCTION_DEFS(AH_JOB);
GWEN_LIST2_FUNCTION_DEFS(AH_JOB, AH_Job);
void AH_Job_List2_FreeAll(AH_JOB_LIST2 *jl);

#include "aqhbci/msglayer/hbci_l.h"
#include "aqhbci/joblayer/result_l.h"
#include <aqhbci/aqhbci.h>

#include <aqbanking/backendsupport/user.h>
#include <aqbanking/types/message.h>
#include <aqbanking/banking_be.h>

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


/** @name Prototypes For Virtual Functions
 *
 */
/*@{*/
typedef int (*AH_JOB_PROCESS_FN)(AH_JOB *j,
                                 AB_IMEXPORTER_CONTEXT *ctx);
typedef int (*AH_JOB_COMMIT_FN)(AH_JOB *j, int doLock);
typedef int (*AH_JOB_PREPARE_FN)(AH_JOB *j);

typedef int (*AH_JOB_ADDCHALLENGEPARAMS_FN)(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod);

typedef int (*AH_JOB_GETLIMITS_FN)(AH_JOB *j, AB_TRANSACTION_LIMITS **pLimits);
typedef int (*AH_JOB_HANDLECOMMAND_FN)(AH_JOB *j, const AB_TRANSACTION *t);
typedef int (*AH_JOB_HANDLERESULTS_FN)(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);


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
                   AB_PROVIDER *pro,
                   AB_USER *u,
                   AB_ACCOUNT *acc,
                   int jobVersion);
void AH_Job_free(AH_JOB *j);
void AH_Job_Attach(AH_JOB *j);
/*@}*/


/** @name Informational Functions
 *
 */
/*@{*/
const char *AH_Job_GetName(const AH_JOB *j);
const char *AH_Job_GetDescription(const AH_JOB *j);

const char *AH_Job_GetCode(const AH_JOB *j);
void AH_Job_SetCode(AH_JOB *j, const char *s);

const char *AH_Job_GetResponseName(const AH_JOB *j);
void AH_Job_SetResponseName(AH_JOB *j, const char *s);

int AH_Job_GetMinSignatures(const AH_JOB *j);
int AH_Job_GetSecurityProfile(const AH_JOB *j);
int AH_Job_GetSecurityClass(const AH_JOB *j);
int AH_Job_GetJobsPerMsg(const AH_JOB *j);

int AH_Job_GetChallengeClass(const AH_JOB *j);
void AH_Job_SetChallengeClass(AH_JOB *j, int i);

AB_USER *AH_Job_GetUser(const AH_JOB *j);

GWEN_DB_NODE *AH_Job_GetParams(const AH_JOB *j);
GWEN_DB_NODE *AH_Job_GetArguments(const AH_JOB *j);
GWEN_DB_NODE *AH_Job_GetResponses(const AH_JOB *j);

unsigned int AH_Job_GetMsgNum(const AH_JOB *j);
const char *AH_Job_GetDialogId(const AH_JOB *j);

AH_JOB_STATUS AH_Job_GetStatus(const AH_JOB *j);
const char *AH_Job_StatusName(AH_JOB_STATUS st);

void AH_Job_AddSigner(AH_JOB *j, const char *s);
int AH_Job_AddSigners(AH_JOB *j, const GWEN_STRINGLIST *sl);

int AH_Job_HasWarnings(const AH_JOB *j);
int AH_Job_HasErrors(const AH_JOB *j);

AH_RESULT_LIST *AH_Job_GetSegResults(const AH_JOB *j);
AH_RESULT_LIST *AH_Job_GetMsgResults(const AH_JOB *j);

AB_MESSAGE_LIST *AH_Job_GetMessages(const AH_JOB *j);

/**
 * Add command to job for later reference.
 */
void AH_Job_AddCommand(AH_JOB *j, AB_TRANSACTION *t);
AB_TRANSACTION_LIST2 *AH_Job_GetCommandList(const AH_JOB *j);

/*@}*/


/** @name Virtual Functions
 *
 */
/*@{*/
int AH_Job_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);
int AH_Job_Commit(AH_JOB *j, int doLock);

/**
 * Check whether the results for this job contain warning 3920. In this
 * case the result must be commited.
 * @return 0 if there is no such result, !=0 otherwise
 */
int AH_Job_HasItanResult(const AH_JOB *j);



int AH_Job_HasResultWithCode(const AH_JOB *j, int code);


/**
 * You can use this from the Commit function of the inheriting class to
 * additionally let the job do some basic stuff (like saving UPD, BPD,
 * messages etc).
 */
int AH_Job_CommitSystemData(AH_JOB *j, int doLock);


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
int AH_Job_DefaultCommitHandler(AH_JOB *j, int doLock);

int AH_Job_Prepare(AH_JOB *j);

/**
 * This function lets the job add its challenge parameters itself.
 * Unfortunately this is needed, because the ZKA decided to make FinTS
 * even more complicated than it already is :-/
 * For some HKTAN versions the list of parameters differs from others.
 */
int AH_Job_AddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod);

/**
 * Get transaction limits for a given job.
 */
int AH_Job_GetLimits(AH_JOB *j, AB_TRANSACTION_LIMITS **pLimits);

/**
 * Handle a given command transaction.
 * The transaction contains the command and parameters for a given job.
 * The job should test the given transaction. If the transaction is not acceptable an
 * error code should be returned.
 * If zero is returned the job should have taken all the data from the given command,
 * e.g. a transfer job will add the transfer represented by the command to its internal list.
 * Don't call AH_Job_AddCommand() within that function, this will be done outside.
 */
int AH_Job_HandleCommand(AH_JOB *j, const AB_TRANSACTION *t);

/**
 * Let the job transfer remaining results to the imExporter context.
 * Some jobs already add their results to the context upon @ref AH_Job_Process.
 * This function is for those jobs which don't.
 */
int AH_Job_HandleResults(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);

/*@}*/


/** @name Setters For Virtual Functions
 *
 */
/*@{*/
void AH_Job_SetProcessFn(AH_JOB *j, AH_JOB_PROCESS_FN f);
void AH_Job_SetCommitFn(AH_JOB *j, AH_JOB_COMMIT_FN f);
void AH_Job_SetNextMsgFn(AH_JOB *j, AH_JOB_NEXTMSG_FN f);
void AH_Job_SetPrepareFn(AH_JOB *j, AH_JOB_PREPARE_FN f);
void AH_Job_SetAddChallengeParamsFn(AH_JOB *j, AH_JOB_ADDCHALLENGEPARAMS_FN f);

void AH_Job_SetGetLimitsFn(AH_JOB *j, AH_JOB_GETLIMITS_FN f);
void AH_Job_SetHandleCommandFn(AH_JOB *j, AH_JOB_HANDLECOMMAND_FN f);
void AH_Job_SetHandleResultsFn(AH_JOB *j, AH_JOB_HANDLERESULTS_FN f);


/*@}*/


void AH_Job_Dump(const AH_JOB *j, FILE *f, unsigned int insert);

const GWEN_STRINGLIST *AH_Job_GetLogs(const AH_JOB *j);


uint32_t AH_Job_GetFirstSegment(const AH_JOB *j);
void AH_Job_SetFirstSegment(AH_JOB *j, uint32_t i);

uint32_t AH_Job_GetLastSegment(const AH_JOB *j);
void AH_Job_SetLastSegment(AH_JOB *j, uint32_t i);

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

uint32_t AH_Job_GetFlags(const AH_JOB *j);
void AH_Job_SetFlags(AH_JOB *j, uint32_t f);
void AH_Job_AddFlags(AH_JOB *j, uint32_t f);
void AH_Job_SubFlags(AH_JOB *j, uint32_t f);


uint32_t AH_Job_GetId(const AH_JOB *j);
void AH_Job_SetId(AH_JOB *j, uint32_t i);

AH_HBCI *AH_Job_GetHbci(const AH_JOB *j);
AB_BANKING *AH_Job_GetBankingApi(const AH_JOB *j);


const char *AH_Job_GetExpectedSigner(const AH_JOB *j);
void AH_Job_SetExpectedSigner(AH_JOB *j, const char *s);
const char *AH_Job_GetExpectedCrypter(const AH_JOB *j);
void AH_Job_SetExpectedCrypter(AH_JOB *j, const char *s);

const char *AH_Job_GetUsedTan(const AH_JOB *j);
void AH_Job_SetUsedTan(AH_JOB *j, const char *s);

void AH_Job_Log(AH_JOB *j, GWEN_LOGGER_LEVEL ll, const char *txt);


GWEN_STRINGLIST *AH_Job_GetChallengeParams(const AH_JOB *j);
void AH_Job_AddChallengeParam(AH_JOB *j, const char *s);
void AH_Job_ClearChallengeParams(AH_JOB *j);
int AH_Job_GetChallengeClass(const AH_JOB *j);
void AH_Job_SetChallengeClass(AH_JOB *j, int i);

void AH_Job_ValueToChallengeString(const AB_VALUE *v, GWEN_BUFFER *buf);

/**
 * This function adds all BPD groups of the job with the given name and for
 * which XML descriptions are available. So basically, the user can choose
 * a job version from the returned list.
 */
int AH_Job_SampleBpdVersions(const char *name,
                             AB_USER *u,
                             GWEN_DB_NODE *dbResult);

int AH_Job_GetMaxVersionUpUntil(const char *name, AB_USER *u, int maxVersion);

int AH_Job_GetSegmentVersion(const AH_JOB *j);


int AH_Job_GetTransferCount(AH_JOB *j);
void AH_Job_IncTransferCount(AH_JOB *j);
int AH_Job_GetMaxTransfers(AH_JOB *j);
void AH_Job_SetMaxTransfers(AH_JOB *j, int i);

AB_TRANSACTION_LIST *AH_Job_GetTransferList(const AH_JOB *j);
void AH_Job_AddTransfer(AH_JOB *j, AB_TRANSACTION *t);
AB_TRANSACTION *AH_Job_GetFirstTransfer(const AH_JOB *j);

AB_TRANSACTION_COMMAND AH_Job_GetSupportedCommand(const AH_JOB *j);
void AH_Job_SetSupportedCommand(AH_JOB *j, AB_TRANSACTION_COMMAND tc);


/**
 * This is a simple implementation of the GetLimits-callback which just creates
 * an empty transaction limits object with just the command field set.
 */
int AH_Job_GetLimits_EmptyLimits(AH_JOB *j, AB_TRANSACTION_LIMITS **pLimits);


/**
 * Implementation of AH_Job_HandleCommand which just accepts the command
 * and does nothing else.
 */
int AH_Job_HandleCommand_Accept(AH_JOB *j, const AB_TRANSACTION *t);


/**
 * Implementation of AH_Job_HandleResults which does nothing.
 */
int AH_Job_HandleResults_Empty(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);



void AH_Job_ReadAccountDataSeg(AB_ACCOUNT *acc, GWEN_DB_NODE *dbAccountData);



AB_PROVIDER *AH_Job_GetProvider(const AH_JOB *j);


/* Get job from list by id */
AH_JOB *AH_Job_List_GetById(AH_JOB_LIST *jl, uint32_t id);


void AH_Job_SetStatusOnCommands(AH_JOB *j, AB_TRANSACTION_STATUS status);


char *AH_Job_GenerateIdFromDateTimeAndJobId(const AH_JOB *j, int runningNumber);


#endif /* AH_JOB_L_H */




