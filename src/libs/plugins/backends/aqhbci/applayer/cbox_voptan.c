/***************************************************************************
    begin       : Fri Oct 3 2025
    copyright   : (C) 2025 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "aqhbci/applayer/cbox_voptan.h"
#include "aqhbci/applayer/cbox_vopmsg.h"

#include "aqhbci/applayer/cbox_send.h"
#include "aqhbci/applayer/cbox_recv.h"
#include "aqhbci/applayer/cbox_queue.h"
#include "aqhbci/admjobs/jobtan_l.h"
#include "aqhbci/admjobs/jobvpp.h"
#include "aqhbci/admjobs/jobvpa.h"
#include "aqhbci/ajobs/accountjob_l.h"

#include "aqbanking/i18n_l.h"

#include <gwenhywfar/mdigest.h>
#include <gwenhywfar/gui.h>

#include <unistd.h>



/*
 * Simple case: MATCH
 *  User                     Bank
 *  HKVPP+HKxxx+HKTAN(no TAN)    ->
 *                               <-  HIVPP
 *  HKVPA+HKTAN(with TAN)        ->
 *                       <-  HITAN
 *
 * Special case: CLOSEMATCH or NOMATCH
 * Need to send HKTAN(proc 4) again after receiving HIRMS 3945
 *  User                             Bank
 * -----------------------------------------------
 *  HKVPP+HKxxx+HKTAN(no TAN)    ->
 *                               <-  HIVPP, 3945
 *  HKVPA+HKxxx+HKTAN(no TAN)    ->
 *                               <-  HITAN
 *  HKTAN(with TAN)              ->
 *                               <-  HITAN
 *
 * (This whole process sucks royally!)
 */


/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int _handleStage1(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *tanJob1, AH_JOB *vppJob, AH_JOB *workJob);
static int _handleStage2(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *tanJob1, AH_JOB *vppJob, AH_JOB *workJob);
static int _sendTanAndReceiveResponseProc2(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *tanJob1, AH_JOB *vppJob, AH_JOB *workJob);
static int _sendHKVPAandHKXXXandHKTAN(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *vppJob, AH_JOB *workJob);
static int _sendHKTANwithTAN(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *tanJob1, AH_JOB *workJob);
static int _sendTanAndReceiveResponseProcS(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *tanJob1, AH_JOB *vppJob, AH_JOB *workJob);
static int _sendAndRecvQueue(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOBQUEUE *jobQueue);
static int _repeatJobUntilNoAttachPoint(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *j);
static AH_JOBQUEUE *_createQueueForStage1(AB_USER *user, AH_JOB *tanJob, AH_JOB *vppJob, AH_JOB *workJob);
static AH_JOBQUEUE *_createQueueForStage2a(AB_USER *user, AH_JOB *tanJob2, AH_JOB *vppJob, AH_JOB *vpaJob, AH_JOB *workJob);
static AH_JOBQUEUE *_createQueueForStage2b(AB_USER *user, AH_JOB *tanJob1, AH_JOB *vpaJob, AH_JOB *workJob);
static AH_JOBQUEUE *_createQueueForStage2c(AB_USER *user, AH_JOB *tanJob1, const AH_JOB *workJob);
static AH_JOBQUEUE *_createQueueForStageS(AB_USER *user, AH_JOB *tanJobS, AH_JOB *workJob);
static int _setupTanJobStage2OrS(AH_JOB *tanJob2, const AH_JOB *workJob, const AH_JOB *tanJob1);
static AH_JOB *_createTanJobStage1(AB_PROVIDER *provider, AB_USER *user, int jobVersion, const AH_JOB *workJob);
static AH_JOB *_createTanJobStage2(AB_PROVIDER *provider, AH_DIALOG *dlg, const AH_JOB *workJob, const AH_JOB *tanJob1);
static AH_JOB *_createTanJobDecoupledStageS(AB_PROVIDER *provider, AH_DIALOG *dlg, const AH_JOB *workJob, const AH_JOB *tanJob1);
static AH_JOB *_createVppJob(AB_PROVIDER *provider, AB_USER *user, const AH_JOB *workJob);
static AH_JOB *_createVpaJob(AB_PROVIDER *provider, AB_USER *user, const AH_JOB *vppJob, const AH_JOB *workJob);
static void _copySegResultsToJob(const AH_JOB *srcJob, AH_JOB *destJob);
static void _copyMsgResultsToJob(const AH_JOB *srcJob, AH_JOB *destJob);
static void _copyResultsListToList(const AH_RESULT_LIST *srcList, AH_RESULT_LIST *destList, AH_JOB *destJob);
static int _inputTanForQueueWithChallenges(AH_OUTBOX_CBOX *cbox,
					   AH_DIALOG *dlg,
					   const char *challenge,
					   const char *challengeHhd,
					   AH_JOBQUEUE *jobQueue);
static int _letUserConfirmApproval(AH_OUTBOX_CBOX *cbox, AH_JOB *workJob, const char *challenge);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */

int AH_OutboxCBox_SendAndReceiveJobWithTanAndVpp(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *workJob)
{
  AB_PROVIDER *provider;
  AB_USER *user;
  AH_JOB *tanJob1;
  AH_JOB *vppJob;
  int rv;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Called function with TAN and VPP handling");

  provider=AH_OutboxCBox_GetProvider(cbox);
  user=AH_OutboxCBox_GetUser(cbox);

  vppJob=_createVppJob(provider, user, workJob);
  tanJob1=_createTanJobStage1(provider, user, AH_Dialog_GetTanJobVersion(dlg), workJob);
  if (tanJob1==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create TAN job, aborting");
    AH_Job_free(vppJob);
    return GWEN_ERROR_GENERIC;
  }

  DBG_INFO(AQHBCI_LOGDOMAIN, "Handling stage 1 for job %s", AH_Job_GetName(workJob));
  rv=_handleStage1(cbox, dlg, tanJob1, vppJob, workJob);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_free(tanJob1);
    AH_Job_free(vppJob);
    return GWEN_ERROR_GENERIC;
  }

  DBG_INFO(AQHBCI_LOGDOMAIN, "Handling stage 2 for job %s", AH_Job_GetName(workJob));
  rv=_handleStage2(cbox, dlg, tanJob1, vppJob, workJob);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_free(tanJob1);
    AH_Job_free(vppJob);
    return GWEN_ERROR_GENERIC;
  }

  if (vppJob) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Applying VOP results to transfers");
    AH_OutboxCBox_ApplyVopResultsToTransfers(workJob, AH_Job_VPP_GetResultList(vppJob));
  }

  AH_Job_free(tanJob1);
  AH_Job_free(vppJob);
  return 0;
}



int _handleStage1(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *tanJob1, AH_JOB *vppJob, AH_JOB *workJob)
{
  AH_OUTBOX *outbox;
  AB_USER *user;
  AH_JOBQUEUE *jobQueue;
  int rv;

  outbox=AH_OutboxCBox_GetOutbox(cbox);
  user=AH_OutboxCBox_GetUser(cbox);

  jobQueue=_createQueueForStage1(user, tanJob1, vppJob, workJob);
  if (jobQueue==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return GWEN_ERROR_GENERIC;
  }

  /* send HKVPP, transaction job and HKTAN, receive response and dispatch it to those jobs */
  rv=_sendAndRecvQueue(cbox, dlg, jobQueue);
  AH_JobQueue_free(jobQueue);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  if (vppJob)
    AH_Job_Process(vppJob, AH_Outbox_GetImExContext(outbox));
  AH_Job_Process(tanJob1, AH_Outbox_GetImExContext(outbox));

  if (vppJob) {
    /* repeat sending HKVPP as long as the bank sends an attach point */
    if (AH_Job_GetFlags(vppJob) & AH_JOB_FLAGS_HASATTACHPOINT) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "VOP not finished, waiting...");
      GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice, I18N("VOP pending, waiting..."));
      rv=_repeatJobUntilNoAttachPoint(cbox, dlg, vppJob);
      if (rv) {
	DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
    }
  }

  if (vppJob) {
    const char *s;

    s=AH_Job_VPP_GetVopMsg(vppJob);
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "VPP message: %s", s?s:"<no msg>");

    if (AH_Job_HasResultWithCode(vppJob, 25)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Result of VOP: Names match.");
      GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice, I18N("Result of VOP: Names match."));
    }
    else if (AH_Job_HasResultWithCode(vppJob, 9210)) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Result of VOP: Transaction rejected (e.g. non-existent IBAN).");
      GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, I18N("Result of VOP: Transaction rejected (e.g. non-existent IBAN)."));
      return GWEN_ERROR_GENERIC;
    }
    else if (AH_Job_HasResultWithCode(vppJob, 3090)) {
      const AH_VOP_RESULT_LIST *resultList;

      resultList=AH_Job_VPP_GetResultList(vppJob);
      if ((s && *s) || (resultList && AH_VopResult_List_GetCount(resultList))) {
        /* only show dialog if either bank message or result list available */
        DBG_INFO(AQHBCI_LOGDOMAIN, "Let user accept or reject VOP result");
        rv=AH_OutboxCBox_LetUserConfirmVopResult(cbox, workJob, vppJob, s);
        if (rv<0) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Aborted by user (%d)", rv);
          GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, I18N("Aborted by user."));
          return rv;
        }
      }
    }
  }

  return 0;
}



int _handleStage2(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *tanJob1, AH_JOB *vppJob, AH_JOB *workJob)
{
  int rv;
  int needVpa;

//  needVpa=(AH_Job_HasResultWithCode(workJob, 3091) || AH_Job_HasResultWithCode(tanJob1, 3091))?0:1;
  needVpa=vppJob?(AH_Job_HasResultWithCode(vppJob, 3091)?00:1):0;

  /* handle HKTAN */
  if (AH_Job_HasResultWithCode(tanJob1, 3945) ||           /* "Freigabe kann nicht erteilt werden" */
      (vppJob?AH_Job_HasResultWithCode(vppJob, 3090):0)) { /* "Ergebnis Namensabgleich pruefen" */
    DBG_INFO(AQHBCI_LOGDOMAIN, "TAN job has result 3945 or VPP job has result 3090");
    rv=_sendHKVPAandHKXXXandHKTAN(cbox, dlg, vppJob, workJob);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  else if (AH_Job_HasResultWithCode(tanJob1, 3076)) { /* SCA not needed */
    DBG_INFO(AQHBCI_LOGDOMAIN, "No TAN needed");
    // TODO: send HKVPA if needed (shouldn't be necessary)
  }
  else if (AH_Job_HasResultWithCode(tanJob1, 3955)) { /* decoupled */
    DBG_INFO(AQHBCI_LOGDOMAIN, "Decoupled (3955), waiting for user to approve transaction externally");
    do { /* loop until positive status response received */
      rv=_sendTanAndReceiveResponseProcS(cbox, dlg, tanJob1, needVpa?vppJob:NULL, workJob);
    } while (rv==1);
    if (rv!=0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error exchanging TAN status message (%d)", rv);
      return rv;
    }
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Job has no 3076 result, getting TAN");
    rv=_sendTanAndReceiveResponseProc2(cbox, dlg, tanJob1, needVpa?vppJob:NULL, workJob);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}



int _sendTanAndReceiveResponseProc2(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *tanJob1, AH_JOB *vppJob, AH_JOB *workJob)
{
  AH_OUTBOX *outbox;
  AB_USER *user;
  AB_PROVIDER *provider;
  int rv;
  AH_JOBQUEUE *jobQueue;
  AH_JOB *tanJob2;
  AH_JOB *vpaJob=NULL;

  outbox=AH_OutboxCBox_GetOutbox(cbox);
  provider=AH_OutboxCBox_GetProvider(cbox);
  user=AH_OutboxCBox_GetUser(cbox);

  /* possibly create VPA job */
  if (vppJob) {
    vpaJob=_createVpaJob(provider, user, vppJob, workJob);
    if (vpaJob==NULL) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here");
      return GWEN_ERROR_GENERIC;
    }
  }

  /* prepare HKTAN (process type 2) */
  tanJob2=_createTanJobStage2(provider, dlg, workJob, tanJob1);
  if (tanJob2==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return GWEN_ERROR_GENERIC;
  }

  jobQueue=_createQueueForStage2a(user, tanJob2, vppJob, vpaJob, workJob);
  AH_JobQueue_AddFlags(jobQueue, AH_JOBQUEUE_FLAGS_NEEDTAN);
  rv=_inputTanForQueueWithChallenges(cbox,
				     dlg,
				     AH_Job_Tan_GetChallenge(tanJob1),
				     AH_Job_Tan_GetHhdChallenge(tanJob1),
				     jobQueue); /* sets usedTan in jobQueue */
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_free(tanJob2);
    AH_JobQueue_free(jobQueue);
    return rv;
  }

  rv=_sendAndRecvQueue(cbox, dlg, jobQueue);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_free(tanJob2);
    AH_JobQueue_free(jobQueue);
    return rv;
  }

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing job \"%s\"", AH_Job_GetName(tanJob2));
  rv=AH_Job_Process(tanJob2, AH_Outbox_GetImExContext(outbox));
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_free(tanJob2);
    AH_JobQueue_free(jobQueue);
    return rv;
  }

  /* dispatch results from tanJob2 to workjob */
  _copySegResultsToJob(tanJob2, workJob);    /* TODO: don't copy result, just set status */
  _copyMsgResultsToJob(tanJob2, workJob);

  /* store used TAN in original job (if any) */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Storing TAN in job [%s]", AH_Job_GetName(workJob));
  AH_Job_SetUsedTan(workJob, AH_JobQueue_GetUsedTan(jobQueue));

  AH_JobQueue_free(jobQueue);

  return 0;
}



/**
 * This step is necessary if the bank responds with "3945 - Freigabe kann nicht erteilt werden".
 * In this case we need to create and send a new HKTAN (challenge request) and respond to THAT with a TAN.
 */
int _sendHKVPAandHKXXXandHKTAN(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *vppJob, AH_JOB *workJob)
{
  AH_OUTBOX *outbox;
  AB_USER *user;
  AB_PROVIDER *provider;
  int rv;
  AH_JOBQUEUE *jobQueue;
  AH_JOB *tanJob1;
  AH_JOB *vpaJob=NULL;

  outbox=AH_OutboxCBox_GetOutbox(cbox);
  provider=AH_OutboxCBox_GetProvider(cbox);
  user=AH_OutboxCBox_GetUser(cbox);

  /* possibly create VPA job */
  if (vppJob) {
    vpaJob=_createVpaJob(provider, user, vppJob, workJob);
    if (vpaJob==NULL) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here");
      return GWEN_ERROR_GENERIC;
    }
  }

  /* prepare HKTAN again (stage1 is correct here, that's  process type 4 to request a challenge again) */
  tanJob1=_createTanJobStage1(provider, user, AH_Dialog_GetTanJobVersion(dlg), workJob);
  if (tanJob1==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return GWEN_ERROR_GENERIC;
  }

  jobQueue=_createQueueForStage2b(user, tanJob1, vpaJob, workJob);

  rv=_sendAndRecvQueue(cbox, dlg, jobQueue);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    AH_Job_free(tanJob1);
    AH_JobQueue_free(jobQueue);
    return rv;
  }

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing job \"%s\"", AH_Job_GetName(tanJob1));
  rv=AH_Job_Process(tanJob1, AH_Outbox_GetImExContext(outbox));
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_free(tanJob1);
    AH_JobQueue_free(jobQueue);
    return rv;
  }
  AH_JobQueue_free(jobQueue);

  /* only send TAN challenge request if bank says it wants it! */
  if (AH_Job_HasResultWithCode(tanJob1, 3076)) { /* SCA not needed */
    DBG_INFO(AQHBCI_LOGDOMAIN, "No TAN needed, stage 2 done.");
  }
  else {
    if (AH_Job_HasResultWithCode(tanJob1, 3955)) { /* decoupled */
      DBG_INFO(AQHBCI_LOGDOMAIN, "Decoupled (3955), waiting for user to approve transaction externally");
      do { /* loop until positive status response received */
	DBG_INFO(AQHBCI_LOGDOMAIN, "Waiting for user approval of the transfer");
	rv=_sendTanAndReceiveResponseProcS(cbox, dlg, tanJob1, NULL, workJob);
      } while (rv==1);
      if (rv!=0) {
	DBG_INFO(AQHBCI_LOGDOMAIN, "Error exchanging TAN status message (%d)", rv);
	return rv;
      }
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Sending TAN challenge response");
      rv=_sendHKTANwithTAN(cbox, dlg, tanJob1, workJob);
      if (rv) {
	DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
	AH_Job_free(tanJob1);
	return rv;
      }
    }
  }
  AH_Job_free(tanJob1);

  return 0;
}



int _sendHKTANwithTAN(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *tanJob1, AH_JOB *workJob)
{
  AH_OUTBOX *outbox;
  AB_USER *user;
  AB_PROVIDER *provider;
  int rv;
  AH_JOBQUEUE *jobQueue;
  AH_JOB *tanJob2;

  outbox=AH_OutboxCBox_GetOutbox(cbox);
  provider=AH_OutboxCBox_GetProvider(cbox);
  user=AH_OutboxCBox_GetUser(cbox);

  /* prepare HKTAN (process type 2) */
  tanJob2=_createTanJobStage2(provider, dlg, workJob, tanJob1);
  if (tanJob2==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return GWEN_ERROR_GENERIC;
  }

  jobQueue=_createQueueForStage2c(user, tanJob2, workJob);
  AH_JobQueue_AddFlags(jobQueue, AH_JOBQUEUE_FLAGS_NEEDTAN);
  rv=_inputTanForQueueWithChallenges(cbox,
				     dlg,
				     AH_Job_Tan_GetChallenge(tanJob1),
				     AH_Job_Tan_GetHhdChallenge(tanJob1),
				     jobQueue); /* sets usedTan in jobQueue */
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_free(tanJob2);
    AH_JobQueue_free(jobQueue);
    return rv;
  }

  rv=_sendAndRecvQueue(cbox, dlg, jobQueue);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_free(tanJob2);
    AH_JobQueue_free(jobQueue);
    return rv;
  }

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing job \"%s\"", AH_Job_GetName(tanJob2));
  rv=AH_Job_Process(tanJob2, AH_Outbox_GetImExContext(outbox));
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_free(tanJob2);
    AH_JobQueue_free(jobQueue);
    return rv;
  }

  // TODO: handle decoupled TAN!

  /* dispatch results from tanJob2 to workjob */
  _copySegResultsToJob(tanJob2, workJob);
  _copyMsgResultsToJob(tanJob2, workJob);

  /* store used TAN in original job (if any) */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Storing TAN in job [%s]", AH_Job_GetName(workJob));
  AH_Job_SetUsedTan(workJob, AH_JobQueue_GetUsedTan(jobQueue));

  AH_JobQueue_free(jobQueue);

  return 0;
}



int _sendTanAndReceiveResponseProcS(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *tanJob1, AH_JOB *vppJob, AH_JOB *workJob)
{
  AH_OUTBOX *outbox;
  AB_PROVIDER *provider;
  int rv;
  AH_JOBQUEUE *jobQueue;
  AH_JOB *tanJob2;
  AB_USER *user;

  outbox=AH_OutboxCBox_GetOutbox(cbox);
  provider=AH_OutboxCBox_GetProvider(cbox);
  user=AH_OutboxCBox_GetUser(cbox);

  rv=_letUserConfirmApproval(cbox, workJob, AH_Job_Tan_GetChallenge(tanJob1));
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* prepare HKTAN (process type 2) */
  tanJob2=_createTanJobDecoupledStageS(provider, dlg, workJob, tanJob1);
  if (tanJob2==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    AH_Job_free(tanJob2);
    return GWEN_ERROR_GENERIC;
  }

  /* prepare second message (the one with the TAN) */
  jobQueue=_createQueueForStageS(user, tanJob2, workJob);

  rv=_sendAndRecvQueue(cbox, dlg, jobQueue);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_JobQueue_free(jobQueue);
    return rv;
  }
  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing job \"%s\"", AH_Job_GetName(tanJob2));
  rv=AH_Job_Process(tanJob2, AH_Outbox_GetImExContext(outbox));
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* dispatch results from tanJob2 to workjob */
  _copySegResultsToJob(tanJob2, workJob);
  _copyMsgResultsToJob(tanJob2, workJob);

  if (AH_Job_HasResultWithCode(tanJob2, 3956)) { /* decoupled */
    DBG_INFO(AQHBCI_LOGDOMAIN, "Decoupled (3956), still waiting for user to approve transaction externally");
    AH_JobQueue_free(jobQueue);
    return 1;
  }

  AH_JobQueue_free(jobQueue);
  return 0;
}



int _sendAndRecvQueue(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOBQUEUE *jobQueue)
{
  int rv;

  rv=AH_OutboxCBox_SendQueue(cbox, dlg, jobQueue);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  AH_JobQueue_SetJobStatusOnMatch(jobQueue, AH_JobStatusEncoded, AH_JobStatusSent);

  rv=AH_OutboxCBox_RecvQueue(cbox, dlg, jobQueue);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int _repeatJobUntilNoAttachPoint(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *j)
{
  AH_OUTBOX *outbox;
  AB_USER *user;

  outbox=AH_OutboxCBox_GetOutbox(cbox);
  user=AH_Job_GetUser(j);

  /* repeat sending job as long as the bank sends an attach point */
  while(AH_Job_GetFlags(j) & AH_JOB_FLAGS_HASATTACHPOINT) {
    AH_JOBQUEUE *jobQueue;
    int rv;

    DBG_INFO(NULL, "Job has attach point, waiting and sending again");
    sleep(2); /* TODO: use select from GUI! */
    jobQueue=AH_JobQueue_new(user);
    AH_JobQueue_SubFlags(jobQueue, AH_JOBQUEUE_FLAGS_NEEDTAN);
    AH_Job_Attach(j);
    rv=AH_JobQueue_AddJob(jobQueue, j);
    if (rv!=AH_JobQueueAddResultOk) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      AH_JobQueue_free(jobQueue);
      return rv;
    }

    rv=_sendAndRecvQueue(cbox, dlg, jobQueue);
    AH_JobQueue_free(jobQueue);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    AH_Job_Process(j, AH_Outbox_GetImExContext(outbox));
  }

  return 0;
}



void _copySegResultsToJob(const AH_JOB *srcJob, AH_JOB *destJob)
{
  /* dispatch results from job to all members of the queue */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Copying segment results from job %s to job %s", AH_Job_GetName(srcJob), AH_Job_GetName(destJob));
  _copyResultsListToList(AH_Job_GetSegResults(srcJob), AH_Job_GetSegResults(destJob), destJob);
}



void _copyMsgResultsToJob(const AH_JOB *srcJob, AH_JOB *destJob)
{
  /* dispatch results from job to all members of the queue */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Copying message results from job %s to job %s", AH_Job_GetName(srcJob), AH_Job_GetName(destJob));
  _copyResultsListToList(AH_Job_GetMsgResults(srcJob), AH_Job_GetMsgResults(destJob), destJob);

}



void _copyResultsListToList(const AH_RESULT_LIST *srcList, AH_RESULT_LIST *destList, AH_JOB *destJob)
{
  if (srcList && destList) {
    AH_RESULT *origRes;

    origRes=AH_Result_List_First(srcList);
    if (origRes) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "We have segment results in source job, setting status of all sent jobs to ANSWERED");
      AH_Job_SetJobStatusOnMatch(destJob, AH_JobStatusSent, AH_JobStatusAnswered);
    }
    while (origRes) {
      AH_RESULT *nr;

      nr=AH_Result_dup(origRes);
      DBG_INFO(AQHBCI_LOGDOMAIN, "Adding result %d to job %s", AH_Result_GetCode(origRes), AH_Job_GetName(destJob));
      AH_Result_List_Add(nr, destList);
      origRes=AH_Result_List_Next(origRes);
    } /* while origRes */
  } /* if srcList */
}



AH_JOBQUEUE *_createQueueForStage1(AB_USER *user, AH_JOB *tanJob, AH_JOB *vppJob, AH_JOB *workJob)
{
  AH_JOBQUEUE *jobQueue;
  int rv;

  jobQueue=AH_JobQueue_new(user);
  AH_JobQueue_SubFlags(jobQueue, AH_JOBQUEUE_FLAGS_NEEDTAN);

  if (vppJob) {
    AH_Job_Attach(vppJob);
    rv=AH_JobQueue_AddJob(jobQueue, vppJob);
    if (rv!=AH_JobQueueAddResultOk) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      AH_JobQueue_free(jobQueue);
      return NULL;
    }
  }

  AH_Job_Attach(workJob);
  rv=AH_JobQueue_AddJob(jobQueue, workJob);
  if (rv!=AH_JobQueueAddResultOk) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_JobQueue_free(jobQueue);
    return NULL;
  }

  AH_Job_Attach(tanJob);
  rv=AH_JobQueue_AddJob(jobQueue, tanJob);
  if (rv!=AH_JobQueueAddResultOk) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_JobQueue_free(jobQueue);
    return NULL;
  }

  AH_JobQueue_SubFlags(jobQueue, AH_JOBQUEUE_FLAGS_NEEDTAN);

  return jobQueue;
}



AH_JOBQUEUE *_createQueueForStageS(AB_USER *user, AH_JOB *tanJobS, AH_JOB *workJob)
{
  AH_JOBQUEUE *jobQueue;
  int rv;

  /* decoupled, only HKTAN needed */

  jobQueue=AH_JobQueue_new(user);
  /* we don't need a TAN with this queue, because the TAN is conveyed externally via cellphone app */
  AH_JobQueue_SubFlags(jobQueue, AH_JOBQUEUE_FLAGS_NEEDTAN);

  AH_Job_Attach(tanJobS);
  rv=AH_JobQueue_AddJob(jobQueue, tanJobS);
  if (rv!=AH_JobQueueAddResultOk) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_JobQueue_free(jobQueue);
    return NULL;
  }

  AH_JobQueue_AddFlags(jobQueue, (AH_Job_GetFlags(workJob) & AH_JOB_FLAGS_CRYPT)?AH_JOBQUEUE_FLAGS_CRYPT:0);
  AH_JobQueue_AddFlags(jobQueue, (AH_Job_GetFlags(workJob) & AH_JOB_FLAGS_SIGN)?AH_JOBQUEUE_FLAGS_SIGN:0);

  return jobQueue;
}



AH_JOBQUEUE *_createQueueForStage2a(AB_USER *user, AH_JOB *tanJob2, AH_JOB *vppJob, AH_JOB *vpaJob, AH_JOB *workJob)
{
  AH_JOBQUEUE *jobQueue;
  int rv;

  /* prepare second message (the one with the TAN) */
  jobQueue=AH_JobQueue_new(user);

  if (vpaJob) {
    AH_Job_Attach(vpaJob);
    rv=AH_JobQueue_AddJob(jobQueue, vpaJob);
    if (rv!=AH_JobQueueAddResultOk) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      AH_JobQueue_free(jobQueue);
      return NULL;
    }
  }

  if (vppJob && workJob) {
    if (!AH_Job_HasResultWithCode(vppJob, 25) && !!AH_Job_HasResultWithCode(vppJob, 20)) {
      /* result "MATCH" not found, need to add workJob again */
      AH_Job_Attach(workJob);
      rv=AH_JobQueue_AddJob(jobQueue, workJob);
      if (rv!=AH_JobQueueAddResultOk) {
	DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
	AH_JobQueue_free(jobQueue);
	return NULL;
      }
    }
  }

  AH_Job_Attach(tanJob2);
  rv=AH_JobQueue_AddJob(jobQueue, tanJob2);
  if (rv!=AH_JobQueueAddResultOk) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_JobQueue_free(jobQueue);
    return NULL;
  }
  AH_JobQueue_AddFlags(jobQueue, (AH_Job_GetFlags(workJob) & AH_JOB_FLAGS_CRYPT)?AH_JOBQUEUE_FLAGS_CRYPT:0);
  AH_JobQueue_AddFlags(jobQueue, (AH_Job_GetFlags(workJob) & AH_JOB_FLAGS_SIGN)?AH_JOBQUEUE_FLAGS_SIGN:0);

  return jobQueue;
}



AH_JOBQUEUE *_createQueueForStage2b(AB_USER *user, AH_JOB *tanJob1, AH_JOB *vpaJob, AH_JOB *workJob)
{
  AH_JOBQUEUE *jobQueue;
  int rv;

  /* prepare second message (with new TAN request process 4) */
  jobQueue=AH_JobQueue_new(user);
  AH_JobQueue_SubFlags(jobQueue, AH_JOBQUEUE_FLAGS_NEEDTAN);

  if (vpaJob) {
    AH_Job_Attach(vpaJob);
    rv=AH_JobQueue_AddJob(jobQueue, vpaJob);
    if (rv!=AH_JobQueueAddResultOk) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      AH_JobQueue_free(jobQueue);
      return NULL;
    }
  }

  AH_Job_Attach(workJob);
  rv=AH_JobQueue_AddJob(jobQueue, workJob);
  if (rv!=AH_JobQueueAddResultOk) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_JobQueue_free(jobQueue);
    return NULL;
  }

  AH_Job_Attach(tanJob1);
  rv=AH_JobQueue_AddJob(jobQueue, tanJob1);
  if (rv!=AH_JobQueueAddResultOk) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_JobQueue_free(jobQueue);
    return NULL;
  }

  AH_JobQueue_SubFlags(jobQueue, AH_JOBQUEUE_FLAGS_NEEDTAN);

  return jobQueue;
}



AH_JOBQUEUE *_createQueueForStage2c(AB_USER *user, AH_JOB *tanJob1, const AH_JOB *workJob)
{
  AH_JOBQUEUE *jobQueue;
  int rv;

  /* prepare second message (with TAN process 2) */
  jobQueue=AH_JobQueue_new(user);
  AH_JobQueue_AddFlags(jobQueue, AH_JOBQUEUE_FLAGS_NEEDTAN);

  AH_Job_Attach(tanJob1);
  rv=AH_JobQueue_AddJob(jobQueue, tanJob1);
  if (rv!=AH_JobQueueAddResultOk) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_JobQueue_free(jobQueue);
    return NULL;
  }

  AH_JobQueue_AddFlags(jobQueue, AH_JOBQUEUE_FLAGS_NEEDTAN);

  AH_JobQueue_AddFlags(jobQueue, (AH_Job_GetFlags(workJob) & AH_JOB_FLAGS_CRYPT)?AH_JOBQUEUE_FLAGS_CRYPT:0);
  AH_JobQueue_AddFlags(jobQueue, (AH_Job_GetFlags(workJob) & AH_JOB_FLAGS_SIGN)?AH_JOBQUEUE_FLAGS_SIGN:0);

  return jobQueue;
}



AH_JOB *_createTanJobStage1(AB_PROVIDER *provider, AB_USER *user, int jobVersion, const AH_JOB *workJob)
{
  AH_JOB *tanJob;

  /* prepare HKTAN (process type 4) */
  tanJob=AH_Job_Tan_new(provider, user, 4, jobVersion);
  if (!tanJob) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Job HKTAN not available");
    return NULL;
  }
  AH_Job_Tan_SetSegCode(tanJob, AH_Job_GetCode(workJob));
  AH_Job_SubFlags(tanJob, AH_JOB_FLAGS_NEEDTAN);

  AH_Job_Tan_SetTanMediumId(tanJob, AH_User_GetTanMediumId(user));

  /* copy signers */
  if (AH_Job_GetFlags(workJob) & AH_JOB_FLAGS_SIGN) {
    int rv;

    DBG_ERROR(AQHBCI_LOGDOMAIN, "Signature needed");
    AH_Job_AddFlags(tanJob, AH_JOB_FLAGS_SIGN);
    rv=AH_Job_AddSigners(tanJob, AH_Job_GetSigners(workJob));
    if (rv<1) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Signatures needed but no signer given");
      AH_Job_free(tanJob);
      return NULL;
    }
  }
  if (AH_Job_GetFlags(workJob) & AH_JOB_FLAGS_CRYPT) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Encryption needed");
    AH_Job_AddFlags(tanJob, AH_JOB_FLAGS_CRYPT);
  }

  return tanJob;
}



AH_JOB *_createTanJobStage2(AB_PROVIDER *provider, AH_DIALOG *dlg, const AH_JOB *workJob, const AH_JOB *tanJob1)
{
  AH_JOB *tanJob;
  AB_USER *u;
  int rv;

  u=AH_Job_GetUser(workJob);

  /* prepare HKTAN (process type 2) */
  tanJob=AH_Job_Tan_new(provider, u, 2, AH_Dialog_GetTanJobVersion(dlg));
  if (!tanJob) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Job HKTAN not available");
    return NULL;
  }
  /* this time we need a TAN */
  AH_Job_AddFlags(tanJob, AH_JOB_FLAGS_NEEDTAN);

  rv=_setupTanJobStage2OrS(tanJob, workJob, tanJob1);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_free(tanJob);
    return NULL;
  }

  return tanJob;
}



AH_JOB *_createTanJobDecoupledStageS(AB_PROVIDER *provider, AH_DIALOG *dlg, const AH_JOB *workJob, const AH_JOB *tanJob1)
{
  AH_JOB *tanJob;
  AB_USER *u;
  int rv;

  u=AH_Job_GetUser(workJob);

  /* prepare HKTAN (process type 2) */
  tanJob=AH_Job_Tan_new(provider, u, 'S', AH_Dialog_GetTanJobVersion(dlg));
  if (!tanJob) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job HKTAN not available");
    return NULL;
  }
  AH_Job_SubFlags(tanJob, AH_JOB_FLAGS_NEEDTAN);

  rv=_setupTanJobStage2OrS(tanJob, workJob, tanJob1);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_free(tanJob);
    return NULL;
  }

  return tanJob;
}



AH_JOB *_createVppJob(AB_PROVIDER *provider, AB_USER *user, const AH_JOB *workJob)
{
  AH_JOB *vppJob;
  AB_ACCOUNT *account;

  account=AH_AccountJob_IsAccountJob(workJob)?AH_AccountJob_GetAccount(workJob):NULL;
  vppJob=AH_Job_VPP_new(provider, user, account, 0);
  if (vppJob) {
    const char *jobCode;

    jobCode=AH_Job_GetCode(workJob);
    if (jobCode && *jobCode && AH_Job_VPP_IsNeededForCode(vppJob, jobCode)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "VOP needed for job %s", AH_Job_GetCode(workJob));
      if (AH_Job_GetFlags(workJob) & AH_JOB_FLAGS_SIGN) {
        int rv;

        rv=AH_Job_AddSigners(vppJob, AH_Job_GetSigners(workJob));
        if (rv<1) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Signatures needed but no signer given");
          AH_Job_free(vppJob);
          return NULL;
        }
        AH_Job_AddFlags(vppJob, AH_JOB_FLAGS_SIGN);
      }
      return vppJob;
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "VOP not needed for job %s", AH_Job_GetCode(workJob));
      AH_Job_free(vppJob);
      return NULL;
    }
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "VOP not available");
    return NULL;
  }
}



AH_JOB *_createVpaJob(AB_PROVIDER *provider, AB_USER *user, const AH_JOB *vppJob, const AH_JOB *workJob)
{
  AH_JOB *vpaJob;
  const uint8_t *ptrVopId;
  unsigned int lenVopId;

  ptrVopId=AH_Job_VPP_GetPtrVopId(vppJob);
  lenVopId=AH_Job_VPP_GetLenVopId(vppJob);
  if (!(ptrVopId && lenVopId)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No VOP id!");
    return NULL;
  }

  vpaJob=AH_Job_VPA_new(provider, user, 0, ptrVopId, lenVopId);
  if (vpaJob) {
    /* copy signers */
    if (AH_Job_GetFlags(workJob) & AH_JOB_FLAGS_SIGN) {
      int rv;
  
      rv=AH_Job_AddSigners(vpaJob, AH_Job_GetSigners(workJob));
      if (rv<1) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Signatures needed but no signer given");
        AH_Job_free(vpaJob);
        return NULL;
      }
      AH_Job_AddFlags(vpaJob, AH_JOB_FLAGS_SIGN);
    }
  }
  return vpaJob;
}



int _setupTanJobStage2OrS(AH_JOB *tanJob2, const AH_JOB *workJob, const AH_JOB *tanJob1)
{
  AB_USER *u;

  u=AH_Job_GetUser(tanJob2);

  AH_Job_Tan_SetReference(tanJob2, AH_Job_Tan_GetReference(tanJob1));
  AH_Job_Tan_SetTanMediumId(tanJob2, AH_User_GetTanMediumId(u));

  /* copy signers */
  if (AH_Job_GetFlags(workJob) & AH_JOB_FLAGS_SIGN) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Signature needed");
    AH_Job_AddFlags(tanJob2, AH_JOB_FLAGS_SIGN);
    if (AH_Job_AddSigners(tanJob2, AH_Job_GetSigners(workJob))<1) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Signatures needed but no signer given");
      return GWEN_ERROR_GENERIC;
    }
  }
  if (AH_Job_GetFlags(workJob) & AH_JOB_FLAGS_CRYPT) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Encryption needed");
    AH_Job_AddFlags(tanJob2, AH_JOB_FLAGS_CRYPT);
  }
  return 0;
}



int _inputTanForQueueWithChallenges(AH_OUTBOX_CBOX *cbox,
				    AH_DIALOG *dlg,
				    const char *challenge,
				    const char *challengeHhd,
				    AH_JOBQUEUE *jobQueue)
{
  /* ask for TAN */
  if (challenge || challengeHhd) {
    char tanBuffer[64];
    int rv;

    memset(tanBuffer, 0, sizeof(tanBuffer));
    rv=AH_OutboxCBox_InputTanWithChallenge(cbox, dlg, challenge, challengeHhd, tanBuffer, 1, sizeof(tanBuffer)-1);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    AH_JobQueue_SetUsedTan(jobQueue, tanBuffer);
    return 0;
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No challenge received");
    return GWEN_ERROR_BAD_DATA;
  }
}



int _letUserConfirmApproval(AH_OUTBOX_CBOX *cbox, AH_JOB *workJob, const char *challenge)
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
    GWEN_Buffer_AppendArgs(guiBuf,
                           I18N("Your approval for this communication with your bank server is required.\n"
                                "Please use your smart phone app for this\n"
                                "and click \"Approved\" afterwards (%s, user %s at %s).\n"
                                "\n"
                                "Message from bank server regarding this process:\n"
                                "%s\n"),
                           AH_Job_GetCode(workJob),
                           sUserName?sUserName:"<no user id>",
                           sBankName?sBankName:"<no bank name>",
                           challenge);

    GWEN_Buffer_AppendString(guiBuf, "<html>");
    GWEN_Buffer_AppendArgs(guiBuf,
                           I18N("<html>"
                                "<p>Your approval for this communication with your bank server is required.</p>"
                                "<p>Please use your smart phone app for this "
                                "and click <b><i>Approved</i></b> <i>afterwards</i> (%s, user %s at %s).</p>"
                                "<p>Message from bank server regarding this process:</p>"
                                "<p>%s</p>"
                                "</html>"),
                           AH_Job_GetCode(workJob),
                           sUserName?sUserName:"no user id",
                           sBankName?sBankName:"no bank name",
                           challenge?challenge:"no message");

    AB_BankInfo_free(bankInfo);

    rv=GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_INFO | GWEN_GUI_MSG_FLAGS_CONFIRM_B1 | GWEN_GUI_MSG_FLAGS_SEVERITY_NORMAL,
			   I18N("Decoupled Mode: Waiting for Approval"),
			   GWEN_Buffer_GetStart(guiBuf),
			   I18N("Approved"),
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





