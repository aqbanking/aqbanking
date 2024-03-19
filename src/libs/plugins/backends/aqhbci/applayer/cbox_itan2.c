/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "aqhbci/applayer/cbox_itan2.h"

#include "aqhbci/applayer/cbox_send.h"
#include "aqhbci/applayer/cbox_recv.h"
#include "aqhbci/applayer/cbox_queue.h"
#include "aqhbci/admjobs/jobtan_l.h"

#include "aqbanking/i18n_l.h"

#include <gwenhywfar/mdigest.h>
#include <gwenhywfar/gui.h>

#include <ctype.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


static int _sendTanAndReceiveResponseProc2(AH_OUTBOX_CBOX *cbox,
                                           AH_DIALOG *dlg,
                                           AH_JOBQUEUE *jobQueueNeedingTan,
                                           AH_JOB *tanJobFromFirstStage);
static int _sendTanAndReceiveResponseProcS(AH_OUTBOX_CBOX *cbox,
					   AH_DIALOG *dlg,
					   AH_JOBQUEUE *jobQueueNeedingTan,
					   AH_JOB *tanJobFromFirstStage);
static int _sendTanQueue2AndDispatchResponse(AH_OUTBOX_CBOX *cbox,
					     AH_DIALOG *dlg,
					     AH_JOBQUEUE *jobQueueNeedingTan,
					     const AH_JOB *tanJobFromFirstStage,
					     AH_JOBQUEUE *jobQueue2);

static int _inputTanForQueueWithChallenges(AH_OUTBOX_CBOX *cbox,
					   AH_DIALOG *dlg,
					   const char *challenge,
					   const char *challengeHhd,
					   AH_JOBQUEUE *jobQueue2);
static int _letUserConfirmApproval(AH_OUTBOX_CBOX *cbox, const char *challenge);
static AH_JOB *_createTanJobStage2(AB_PROVIDER *provider,
				   AH_DIALOG *dlg,
				   const AH_JOB *jobNeedingTan,
				   const AH_JOB *tanJobFromFirstStage);
static AH_JOB *_createTanJobDecoupledStageS(AB_PROVIDER *provider,
					    AH_DIALOG *dlg,
					    const AH_JOB *jobNeedingTan,
					    const AH_JOB *tanJobFromFirstStage);
static AH_MSG *_encodeTanJobStage2(AH_DIALOG *dlg, AH_JOBQUEUE *jobQueue2, AH_JOB *jobNeedingTan);
static int _setupTanJobStage2OrS(AH_JOB *tanJob2, const AH_JOB *jobNeedingTan, const AH_JOB *tanJobFromFirstStage);
static void _dispatchJobSegResultsToQueue(AH_JOB *job, AH_JOBQUEUE *qJob);
static void _dispatchJobMsgResultsToQueue(AH_JOB *job, AH_JOBQUEUE *qJob);




/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */


int AH_OutboxCBox_SendAndReceiveQueueWithTan2(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOBQUEUE *jobQueueNeedingTan)
{
  int rv;
  AH_OUTBOX *outbox;
  AB_PROVIDER *provider;
  AH_JOB *jobNeedingTan;
  AH_JOB *tanJobForFirstStage;
  AB_USER *u;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Sending job with TAN (process variant 2)");

  outbox=AH_OutboxCBox_GetOutbox(cbox);
  provider=AH_OutboxCBox_GetProvider(cbox);
  jobNeedingTan=AH_JobQueue_GetFirstJob(jobQueueNeedingTan);
  u=AH_Job_GetUser(jobNeedingTan);

  /* don't need a TAN for this particular message, just a HKTAN segment */
  AH_JobQueue_SubFlags(jobQueueNeedingTan, AH_JOBQUEUE_FLAGS_NEEDTAN);

  /* prepare HKTAN (process type 4) */
  tanJobForFirstStage=AH_Job_Tan_new(provider, u, 4, AH_Dialog_GetTanJobVersion(dlg));
  if (!tanJobForFirstStage) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job HKTAN not available");
    return GWEN_ERROR_GENERIC;
  }
  AH_Job_Tan_SetTanMediumId(tanJobForFirstStage, AH_User_GetTanMediumId(u));
  if (AH_Dialog_GetFlags(dlg) &  AH_DIALOG_FLAGS_SCA) {
    /* With tan process "4" only with strong verification */
    AH_Job_Tan_SetSegCode(tanJobForFirstStage, AH_Job_GetCode(jobNeedingTan));
  }

  /* copy signers */
  if (AH_Job_GetFlags(jobNeedingTan) & AH_JOB_FLAGS_SIGN) {
    rv=AH_Job_AddSigners(tanJobForFirstStage, AH_Job_GetSigners(jobNeedingTan));
    if (rv<1) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Signatures needed but no signer given");
      return GWEN_ERROR_INVALID;
    }
  }

  /* add job to queue */
  rv=AH_JobQueue_AddJob(jobQueueNeedingTan, tanJobForFirstStage);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_free(tanJobForFirstStage);
    return rv;
  }

  /* send original job with HKTAN, receive response and dispatch it to those jobs */
  rv=AH_OutboxCBox_SendAndRecvQueueNoTan(cbox, dlg, jobQueueNeedingTan);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing TAN job from first stage");
  rv=AH_Job_Process(tanJobForFirstStage, AH_Outbox_GetImExContext(outbox));
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  if (AH_Job_HasResultWithCode(jobNeedingTan, 3076) || AH_Job_HasResultWithCode(tanJobForFirstStage, 3076)) { /* SCA not needed */
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "No TAN needed");
  }
  else if (AH_Job_HasResultWithCode(jobNeedingTan, 3955) || AH_Job_HasResultWithCode(tanJobForFirstStage, 3955)) { /* decoupled */
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Decoupled (3955), waiting for user to approve transaction externally");
    do { /* loop until positive status response received */
      rv=_sendTanAndReceiveResponseProcS(cbox, dlg, jobQueueNeedingTan, tanJobForFirstStage);
    } while (rv==1);
    if (rv!=0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Error exchanging TAN status message (%d)", rv);
      return rv;
    }
  }
  else {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Job has no 3076 result, getting TAN");
    rv=_sendTanAndReceiveResponseProc2(cbox, dlg, jobQueueNeedingTan, tanJobForFirstStage);
    if (rv<0) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}



int AH_OutboxCBox_SendAndReceiveJobWithTan2(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *job)
{
  AB_USER *user;
  AH_JOBQUEUE *jobQueueNeedingTan;
  int rv;

  user=AH_OutboxCBox_GetUser(cbox);

  jobQueueNeedingTan=AH_JobQueue_new(user);

  /* add original job to queue */
  AH_Job_Attach(job);
  rv=AH_JobQueue_AddJob(jobQueueNeedingTan, job);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_JobQueue_free(jobQueueNeedingTan);
    return rv;
  }

  /* send queue */
  rv=AH_OutboxCBox_SendAndReceiveQueueWithTan2(cbox, dlg, jobQueueNeedingTan);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_JobQueue_free(jobQueueNeedingTan);
    return rv;
  }

  AH_JobQueue_free(jobQueueNeedingTan);
  return rv;
}



int _sendTanAndReceiveResponseProc2(AH_OUTBOX_CBOX *cbox,
                                    AH_DIALOG *dlg,
                                    AH_JOBQUEUE *jobQueueNeedingTan,
                                    AH_JOB *tanJobFromFirstStage)
{
  AB_PROVIDER *provider;
  int rv;
  AH_JOB *jobNeedingTan;
  AH_JOBQUEUE *jobQueue2;
  AH_JOB *tanJob2;

  assert(jobQueueNeedingTan);
  assert(tanJobFromFirstStage);

  provider=AH_OutboxCBox_GetProvider(cbox);
  jobNeedingTan=AH_JobQueue_GetFirstJob(jobQueueNeedingTan);

  /* prepare second message (the one with the TAN) */
  jobQueue2=AH_JobQueue_fromQueue(jobQueueNeedingTan);
  AH_JobQueue_SetReferenceQueue(jobQueue2, jobQueueNeedingTan);

  rv=_inputTanForQueueWithChallenges(cbox,
				     dlg,
				     AH_Job_Tan_GetChallenge(tanJobFromFirstStage),
				     AH_Job_Tan_GetHhdChallenge(tanJobFromFirstStage),
				     jobQueue2);
  if (rv<0) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_JobQueue_free(jobQueue2);
    return rv;
  }

  /* prepare HKTAN (process type 2) */
  tanJob2=_createTanJobStage2(provider, dlg, jobNeedingTan, tanJobFromFirstStage);
  if (tanJob2==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    AH_Job_free(tanJob2);
    AH_JobQueue_free(jobQueue2);
    return GWEN_ERROR_GENERIC;
  }

  rv=AH_JobQueue_AddJob(jobQueue2, tanJob2);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_free(tanJob2);
    AH_JobQueue_free(jobQueue2);
    return rv;
  }

  rv=_sendTanQueue2AndDispatchResponse(cbox, dlg, jobQueueNeedingTan, tanJobFromFirstStage, jobQueue2);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    AH_JobQueue_free(jobQueue2);
    return rv;
  }

  /* store used TAN in original job (if any) */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Storing TAN in job [%s]", AH_Job_GetName(jobNeedingTan));
  AH_Job_SetUsedTan(jobNeedingTan, AH_JobQueue_GetUsedTan(jobQueue2));

  AH_JobQueue_free(jobQueue2);

  return 0;
}



int _sendTanAndReceiveResponseProcS(AH_OUTBOX_CBOX *cbox,
                                    AH_DIALOG *dlg,
                                    AH_JOBQUEUE *jobQueueNeedingTan,
                                    AH_JOB *tanJobFromFirstStage)
{
  AB_PROVIDER *provider;
  int rv;
  AH_JOB *jobNeedingTan;
  AH_JOBQUEUE *jobQueue2;
  AH_JOB *tanJob2;

  provider=AH_OutboxCBox_GetProvider(cbox);
  jobNeedingTan=AH_JobQueue_GetFirstJob(jobQueueNeedingTan);

  /* prepare second message (the one with the TAN) */
  jobQueue2=AH_JobQueue_fromQueue(jobQueueNeedingTan);
  AH_JobQueue_SetReferenceQueue(jobQueue2, jobQueueNeedingTan);
  /* we don't need a TAN with this queue, either, because the TAN is conveyed externally via app */
  AH_JobQueue_SubFlags(jobQueue2, AH_JOBQUEUE_FLAGS_NEEDTAN);

  rv=_letUserConfirmApproval(cbox, AH_Job_Tan_GetChallenge(tanJobFromFirstStage));
  if (rv<0) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_JobQueue_free(jobQueue2);
    return rv;
  }

  /* prepare HKTAN (process type 2) */
  tanJob2=_createTanJobDecoupledStageS(provider, dlg, jobNeedingTan, tanJobFromFirstStage);
  if (tanJob2==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    AH_Job_free(tanJob2);
    AH_JobQueue_free(jobQueue2);
    return GWEN_ERROR_GENERIC;
  }
  AH_Job_LogFlags(tanJob2, "TAN job");

  rv=AH_JobQueue_AddJob(jobQueue2, tanJob2);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_free(tanJob2);
    AH_JobQueue_free(jobQueue2);
    return rv;
  }
  AH_Job_LogFlags(tanJob2, "TAN job");

  rv=_sendTanQueue2AndDispatchResponse(cbox, dlg, jobQueueNeedingTan, tanJobFromFirstStage, jobQueue2);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    AH_JobQueue_free(jobQueue2);
    return rv;
  }
  AH_Job_LogFlags(tanJob2, "TAN job");

  if (AH_Job_HasResultWithCode(tanJob2, 3956) || AH_Job_HasResultWithCode(tanJob2, 3956)) { /* decoupled */
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Decoupled (3956), still waiting for user to approve transaction externally");
    AH_JobQueue_free(jobQueue2);
    return 1;
  }

  AH_JobQueue_free(jobQueue2);
  return 0;
}



int _sendTanQueue2AndDispatchResponse(AH_OUTBOX_CBOX *cbox,
				      AH_DIALOG *dlg,
				      AH_JOBQUEUE *jobQueueNeedingTan,
				      const AH_JOB *tanJobFromFirstStage,
				      AH_JOBQUEUE *jobQueue2)
{
  AH_OUTBOX *outbox;
  AH_JOB *jobNeedingTan;
  AH_JOB *tanJob2;
  AH_MSG *msg2;
  int rv;

  outbox=AH_OutboxCBox_GetOutbox(cbox);
  tanJob2=AH_JobQueue_GetFirstJob(jobQueue2);
  jobNeedingTan=AH_JobQueue_GetFirstJob(jobQueueNeedingTan);

  msg2=_encodeTanJobStage2(dlg, jobQueue2, jobNeedingTan);
  if (msg2==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return GWEN_ERROR_GENERIC;
  }

  rv=AH_OutboxCBox_SendMessage(cbox, dlg, msg2);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg2);
    return rv;
  }
  AH_Msg_free(msg2);
  AH_JobQueue_SetJobStatusOnMatch(jobQueue2, AH_JobStatusEncoded, AH_JobStatusSent);

  /* receive response */
  rv=AH_OutboxCBox_RecvQueue(cbox, dlg, jobQueue2);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Processing job \"%s\"", AH_Job_GetName(tanJob2));
    rv=AH_Job_Process(tanJob2, AH_Outbox_GetImExContext(outbox));
    if (rv) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    /* dispatch results from tanJob2 to all members of the queue */
    _dispatchJobSegResultsToQueue(tanJob2, jobQueueNeedingTan);
    _dispatchJobMsgResultsToQueue(tanJob2, jobQueueNeedingTan);
  }
  return 0;
}



int _inputTanForQueueWithChallenges(AH_OUTBOX_CBOX *cbox,
				    AH_DIALOG *dlg,
				    const char *challenge,
				    const char *challengeHhd,
				    AH_JOBQUEUE *jobQueue2)
{
  /* ask for TAN */
  if (challenge || challengeHhd) {
    char tanBuffer[64];
    int rv;

    memset(tanBuffer, 0, sizeof(tanBuffer));
    rv=AH_OutboxCBox_InputTanWithChallenge(cbox, dlg, challenge, challengeHhd, tanBuffer, 1, sizeof(tanBuffer)-1);
    if (rv) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    AH_JobQueue_SetUsedTan(jobQueue2, tanBuffer);
    return 0;
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No challenge received");
    return GWEN_ERROR_BAD_DATA;
  }
}



int _letUserConfirmApproval(AH_OUTBOX_CBOX *cbox, const char *challenge)
{
  /* ask for TAN */
  if (challenge) {
    AB_PROVIDER *provider;
    AB_BANKING *ab;
    AB_USER *user;
    GWEN_BUFFER *guiBuf;
    const char *sUserName;
    const char *sBankName=NULL;
    AB_BANKINFO *bankInfo;
    int rv;

    provider=AH_OutboxCBox_GetProvider(cbox);
    ab=AB_Provider_GetBanking(provider);
    user=AH_OutboxCBox_GetUser(cbox);
    sUserName=AB_User_GetUserId(user);

    /* find bank name */
    bankInfo=AB_Banking_GetBankInfo(ab, "de", "*", AB_User_GetBankCode(user));
    if (bankInfo)
      sBankName=AB_BankInfo_GetBankName(bankInfo);
    if (!sBankName)
      sBankName=AB_User_GetBankCode(user);

    guiBuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendArgs(guiBuf, I18N("Please click below after sending TAN for user %s at %s.\n"), sUserName, sBankName);
    GWEN_Buffer_AppendArgs(guiBuf, I18N("Message from bank server regarding this process:\n%s"), challenge);
    AB_BankInfo_free(bankInfo);

    rv=GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_INFO | GWEN_GUI_MSG_FLAGS_CONFIRM_B1 | GWEN_GUI_MSG_FLAGS_SEVERITY_NORMAL,
			   I18N("Decoupled Mode: Confirm TAN"),
			   GWEN_Buffer_GetStart(guiBuf),
			   I18N("Confirm"),
			   I18N("Abort"),
			   NULL,
                           0);
    GWEN_Buffer_free(guiBuf);
    if (rv!=1) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Not confirming TAN (%d)", rv);
      return GWEN_ERROR_USER_ABORTED;
    }
  }

  return 0;
}



AH_JOB *_createTanJobStage2(AB_PROVIDER *provider, AH_DIALOG *dlg, const AH_JOB *jobNeedingTan, const AH_JOB *tanJobFromFirstStage)
{
  AH_JOB *tanJob2;
  AB_USER *u;
  int rv;

  u=AH_Job_GetUser(jobNeedingTan);

  /* prepare HKTAN (process type 2) */
  tanJob2=AH_Job_Tan_new(provider, u, 2, AH_Dialog_GetTanJobVersion(dlg));
  if (!tanJob2) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job HKTAN not available");
    return NULL;
  }
  AH_Job_LogFlags(tanJob2, "TAN job");

  rv=_setupTanJobStage2OrS(tanJob2, jobNeedingTan, tanJobFromFirstStage);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_free(tanJob2);
    return NULL;
  }

  return tanJob2;
}



AH_JOB *_createTanJobDecoupledStageS(AB_PROVIDER *provider, AH_DIALOG *dlg, const AH_JOB *jobNeedingTan, const AH_JOB *tanJobFromFirstStage)
{
  AH_JOB *tanJob2;
  AB_USER *u;
  int rv;

  u=AH_Job_GetUser(jobNeedingTan);

  /* prepare HKTAN (process type 2) */
  tanJob2=AH_Job_Tan_new(provider, u, 'S', AH_Dialog_GetTanJobVersion(dlg));
  if (!tanJob2) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job HKTAN not available");
    return NULL;
  }
  AH_Job_SubFlags(tanJob2, AH_JOB_FLAGS_NEEDTAN);
  AH_Job_LogFlags(tanJob2, "TAN job");

  rv=_setupTanJobStage2OrS(tanJob2, jobNeedingTan, tanJobFromFirstStage);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_free(tanJob2);
    return NULL;
  }
  AH_Job_LogFlags(tanJob2, "TAN job");

  return tanJob2;
}



int _setupTanJobStage2OrS(AH_JOB *tanJob2, const AH_JOB *jobNeedingTan, const AH_JOB *tanJobFromFirstStage)
{
  AB_USER *u;

  u=AH_Job_GetUser(tanJob2);

  AH_Job_Tan_SetReference(tanJob2, AH_Job_Tan_GetReference(tanJobFromFirstStage));
  AH_Job_Tan_SetTanMediumId(tanJob2, AH_User_GetTanMediumId(u));

  /* copy signers */
  if (AH_Job_GetFlags(jobNeedingTan) & AH_JOB_FLAGS_SIGN) {
    if (AH_Job_AddSigners(tanJob2, AH_Job_GetSigners(jobNeedingTan))<1) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Signatures needed but no signer given");
      AH_Job_free(tanJob2);
      return GWEN_ERROR_GENERIC;
    }
  }
  return 0;
}



AH_MSG *_encodeTanJobStage2(AH_DIALOG *dlg, AH_JOBQUEUE *jobQueue2, AH_JOB *jobNeedingTan)
{
  int rv;
  AH_MSG *msg2;
  AH_JOB *tanJob2;

  tanJob2=AH_JobQueue_GetFirstJob(jobQueue2);

  msg2=AH_Msg_new(dlg);
  AH_Msg_SetNeedTan(msg2, (AH_JobQueue_GetFlags(jobQueue2) & AH_JOBQUEUE_FLAGS_NEEDTAN)?1:0);
  AH_Msg_SetItanMethod(msg2, 0);
  AH_Msg_SetItanHashMode(msg2, 0);
  AH_Msg_SetTan(msg2, AH_JobQueue_GetUsedTan(jobQueue2));

  rv=AH_OutboxCBox_JobToMessage(tanJob2, msg2, 1);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg2);
    return NULL;
  }

  /* encode HKTAN message */
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Encoding queue");
  rv=AH_Msg_EncodeMsg(msg2);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg2);
    return NULL;
  }

  /* store used TAN in original job (if any) */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Storing TAN in job [%s]", AH_Job_GetName(jobNeedingTan));
  AH_Job_SetUsedTan(jobNeedingTan, AH_Msg_GetTan(msg2));

  if (AH_Job_GetStatus(tanJob2)==AH_JobStatusEncoded) {
    AH_Job_SetMsgNum(tanJob2, AH_Msg_GetMsgNum(msg2));
    AH_Job_SetDialogId(tanJob2, AH_Dialog_GetDialogId(dlg));
    /* store expected signer and crypter (if any) */
    AH_Job_SetExpectedSigner(tanJob2, AH_Msg_GetExpectedSigner(msg2));
    AH_Job_SetExpectedCrypter(tanJob2, AH_Msg_GetExpectedCrypter(msg2));
    /* store TAN in TAN job */
    AH_Job_SetUsedTan(tanJob2, AH_Msg_GetTan(msg2));
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "jTAN2 not encoded? (%d)", AH_Job_GetStatus(tanJob2));
  }
  return msg2;
}



void _dispatchJobSegResultsToQueue(AH_JOB *job, AH_JOBQUEUE *qJob)
{
  const AH_JOB_LIST *qjl;

  /* dispatch results from job to all members of the queue */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Dispatching results for HKTAN to queue jobs");
  qjl=AH_JobQueue_GetJobList(qJob);
  if (qjl) {
    AH_RESULT_LIST *rl;

    /* segment results */
    rl=AH_Job_GetSegResults(job);
    if (rl) {
      AH_RESULT *origRes;

      origRes=AH_Result_List_First(rl);
      if (origRes==NULL) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "No segment result in job HKTAN");
      }
      else {
        DBG_INFO(AQHBCI_LOGDOMAIN,
                 "We have segment results in TAN job, setting status of all sent jobs to ANSWERED");
        AH_JobQueue_SetJobStatusOnMatch(qJob, AH_JobStatusSent, AH_JobStatusAnswered);
      }
      while (origRes) {
        AH_JOB *qj;

        qj=AH_Job_List_First(qjl);
        while (qj) {
          if (qj!=job) {
            AH_RESULT *nr;

            nr=AH_Result_dup(origRes);
            DBG_INFO(AQHBCI_LOGDOMAIN, "Adding result %d to job %s", AH_Result_GetCode(origRes), AH_Job_GetName(qj));
            AH_Result_List_Add(nr, AH_Job_GetSegResults(qj));
          }
          else {
            DBG_INFO(AQHBCI_LOGDOMAIN, "Not adding result to the same job");
          }
          qj=AH_Job_List_Next(qj);
        }

        origRes=AH_Result_List_Next(origRes);
      } /* while origRes */
    } /* if rl */
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "No segment results in HKTAN");
    }
  } /* if jql */
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No job list");
  }
}



void _dispatchJobMsgResultsToQueue(AH_JOB *job, AH_JOBQUEUE *qJob)
{
  const AH_JOB_LIST *qjl;

  /* dispatch results from job to all members of the queue */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Dispatching results for HKTAN to queue jobs");
  qjl=AH_JobQueue_GetJobList(qJob);
  if (qjl) {
    AH_RESULT_LIST *rl;

    /* msg results */
    rl=AH_Job_GetMsgResults(job);
    if (rl) {
      AH_RESULT *origRes;

      origRes=AH_Result_List_First(rl);
      while (origRes) {
        AH_JOB *qj;

        qj=AH_Job_List_First(qjl);
        while (qj) {
          AH_RESULT *nr;

          nr=AH_Result_dup(origRes);
          AH_Result_List_Add(nr, AH_Job_GetMsgResults(qj));
          qj=AH_Job_List_Next(qj);
        }

        origRes=AH_Result_List_Next(origRes);
      } /* while origRes */
    } /* if rl */
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "No message results in HKTAN");
    }
  } /* if qjl */
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No job list");
  }
}




