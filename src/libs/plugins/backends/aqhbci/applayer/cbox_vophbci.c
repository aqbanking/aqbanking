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


#include "aqhbci/applayer/cbox_vophbci.h"

#include "aqhbci/applayer/cbox_send.h"
#include "aqhbci/applayer/cbox_recv.h"
#include "aqhbci/applayer/cbox_queue.h"
#include "aqhbci/admjobs/jobtan_l.h"
#include "aqhbci/admjobs/jobvpp.h"
#include "aqhbci/admjobs/jobvpa.h"

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

static int _handleStage1(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *vppJob, AH_JOB *workJob);
static int _handleStage2(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *vppJob, AH_JOB *workJob);
static int _sendAndRecvQueue(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOBQUEUE *jobQueue);
static int _repeatJobUntilNoAttachPoint(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *j);
static AH_JOBQUEUE *_createQueueForStage1(AB_USER *user, AH_JOB *vppJob, AH_JOB *workJob);
static AH_JOBQUEUE *_createQueueForStage2(AB_USER *user, AH_JOB *vppJob, AH_JOB *vpaJob, AH_JOB *workJob);

static AH_JOB *_createVppJob(AB_PROVIDER *provider, AB_USER *user, const AH_JOB *workJob);
static AH_JOB *_createVpaJob(AB_PROVIDER *provider, AB_USER *user, const AH_JOB *vppJob, const AH_JOB *workJob);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */

int AH_OutboxCBox_SendAndReceiveJobWithVpp(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *workJob)
{
  AB_PROVIDER *provider;
  AB_USER *user;
  AH_JOB *vppJob;
  int rv;

  DBG_ERROR(AQHBCI_LOGDOMAIN, "Called function with VPP handling");

  provider=AH_OutboxCBox_GetProvider(cbox);
  user=AH_OutboxCBox_GetUser(cbox);

  vppJob=_createVppJob(provider, user, workJob);
  if (vppJob==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No VPP job created");
  }

  DBG_ERROR(AQHBCI_LOGDOMAIN, "Handling stage 1 for job %s", AH_Job_GetName(workJob));
  rv=_handleStage1(cbox, dlg, vppJob, workJob);
  if (rv<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_free(vppJob);
    return GWEN_ERROR_GENERIC;
  }

  if (vppJob) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Handling stage 2 for job %s", AH_Job_GetName(workJob));
    rv=_handleStage2(cbox, dlg, vppJob, workJob);
    if (rv<0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      AH_Job_free(vppJob);
      return GWEN_ERROR_GENERIC;
    }
  }

  AH_Job_free(vppJob);
  return 0;
}



int _handleStage1(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *vppJob, AH_JOB *workJob)
{
  AH_OUTBOX *outbox;
  AB_USER *user;
  AH_JOBQUEUE *jobQueue;
  int rv;

  outbox=AH_OutboxCBox_GetOutbox(cbox);
  user=AH_OutboxCBox_GetUser(cbox);

  jobQueue=_createQueueForStage1(user, vppJob, workJob);
  if (jobQueue==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "here");
    return GWEN_ERROR_GENERIC;
  }

  /* send HKVPP and transaction job, receive response and dispatch it to those jobs */
  rv=_sendAndRecvQueue(cbox, dlg, jobQueue);
  AH_JobQueue_free(jobQueue);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  if (vppJob) {
    AH_Job_Process(vppJob, AH_Outbox_GetImExContext(outbox));
    /* repeat sending HKVPP as long as the bank sends an attach point */
    if (AH_Job_GetFlags(vppJob) & AH_JOB_FLAGS_HASATTACHPOINT) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "VOP not finished, waiting...");
      rv=_repeatJobUntilNoAttachPoint(cbox, dlg, vppJob);
      if (rv) {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
    }
  }

  if (vppJob) {
    const char *s;

    s=AH_Job_VPP_GetVopMsg(vppJob);
    DBG_ERROR(AQHBCI_LOGDOMAIN, "VPP message: %s", s);

    /*
     * TODO: show result of VPP, force user interaction or abort if questionable match
     */

  }

  return 0;
}



int _handleStage2(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *vppJob, AH_JOB *workJob)
{
  AB_USER *user;
  AB_PROVIDER *provider;
  int rv;
  AH_JOBQUEUE *jobQueue;
  AH_JOB *vpaJob=NULL;
  int needVpa;

  provider=AH_OutboxCBox_GetProvider(cbox);
  user=AH_OutboxCBox_GetUser(cbox);
  needVpa=vppJob?(AH_Job_HasResultWithCode(vppJob, 3091)?0:1):0;

  /* possibly create VPA job */
  if (needVpa && vppJob) {
    vpaJob=_createVpaJob(provider, user, vppJob, workJob);
    if (vpaJob==NULL) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "here");
      return GWEN_ERROR_GENERIC;
    }
  }

  jobQueue=_createQueueForStage2(user, vppJob, vpaJob, workJob);
  rv=_sendAndRecvQueue(cbox, dlg, jobQueue);
  if (rv<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "here");
    AH_JobQueue_free(jobQueue);
    return rv;
  }

  AH_JobQueue_free(jobQueue);

  return 0;
}



int _sendAndRecvQueue(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOBQUEUE *jobQueue)
{
  int rv;

  rv=AH_OutboxCBox_SendQueue(cbox, dlg, jobQueue);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  AH_JobQueue_SetJobStatusOnMatch(jobQueue, AH_JobStatusEncoded, AH_JobStatusSent);

  rv=AH_OutboxCBox_RecvQueue(cbox, dlg, jobQueue);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
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

    DBG_ERROR(NULL, "Job has attach point, waiting and sending again");
    sleep(2); /* TODO: use select from GUI! */
    jobQueue=AH_JobQueue_new(user);
    AH_JobQueue_SubFlags(jobQueue, AH_JOBQUEUE_FLAGS_NEEDTAN);
    AH_Job_Attach(j);
    rv=AH_JobQueue_AddJob(jobQueue, j);
    if (rv!=AH_JobQueueAddResultOk) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      AH_JobQueue_free(jobQueue);
      return rv;
    }

    rv=_sendAndRecvQueue(cbox, dlg, jobQueue);
    AH_JobQueue_free(jobQueue);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    AH_Job_Process(j, AH_Outbox_GetImExContext(outbox));
  }

  return 0;
}



AH_JOBQUEUE *_createQueueForStage1(AB_USER *user, AH_JOB *vppJob, AH_JOB *workJob)
{
  AH_JOBQUEUE *jobQueue;
  int rv;

  jobQueue=AH_JobQueue_new(user);
  AH_JobQueue_SubFlags(jobQueue, AH_JOBQUEUE_FLAGS_NEEDTAN);

  if (vppJob) {
    AH_Job_Attach(vppJob);
    rv=AH_JobQueue_AddJob(jobQueue, vppJob);
    if (rv!=AH_JobQueueAddResultOk) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      AH_JobQueue_free(jobQueue);
      return NULL;
    }
  }

  AH_Job_Attach(workJob);
  rv=AH_JobQueue_AddJob(jobQueue, workJob);
  if (rv!=AH_JobQueueAddResultOk) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_JobQueue_free(jobQueue);
    return NULL;
  }

  AH_JobQueue_SubFlags(jobQueue, AH_JOBQUEUE_FLAGS_NEEDTAN);
  AH_JobQueue_AddFlags(jobQueue, (AH_Job_GetFlags(workJob) & AH_JOB_FLAGS_CRYPT)?AH_JOBQUEUE_FLAGS_CRYPT:0);
  AH_JobQueue_AddFlags(jobQueue, (AH_Job_GetFlags(workJob) & AH_JOB_FLAGS_SIGN)?AH_JOBQUEUE_FLAGS_SIGN:0);

  return jobQueue;
}



AH_JOBQUEUE *_createQueueForStage2(AB_USER *user, AH_JOB *vppJob, AH_JOB *vpaJob, AH_JOB *workJob)
{
  AH_JOBQUEUE *jobQueue;
  int rv;

  /* prepare second message (the one with the TAN) */
  jobQueue=AH_JobQueue_new(user);

  if (vpaJob) {
    AH_Job_Attach(vpaJob);
    rv=AH_JobQueue_AddJob(jobQueue, vpaJob);
    if (rv!=AH_JobQueueAddResultOk) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "here (%d)", rv);
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
	DBG_ERROR(AQHBCI_LOGDOMAIN, "here (%d)", rv);
	AH_JobQueue_free(jobQueue);
	return NULL;
      }
    }
  }

  AH_JobQueue_SubFlags(jobQueue, AH_JOBQUEUE_FLAGS_NEEDTAN);
  AH_JobQueue_AddFlags(jobQueue, (AH_Job_GetFlags(workJob) & AH_JOB_FLAGS_CRYPT)?AH_JOBQUEUE_FLAGS_CRYPT:0);
  AH_JobQueue_AddFlags(jobQueue, (AH_Job_GetFlags(workJob) & AH_JOB_FLAGS_SIGN)?AH_JOBQUEUE_FLAGS_SIGN:0);

  return jobQueue;
}



AH_JOB *_createVppJob(AB_PROVIDER *provider, AB_USER *user, const AH_JOB *workJob)
{
  AH_JOB *vppJob;

  vppJob=AH_Job_VPP_new(provider, user, 0);
  if (vppJob) {
    const char *jobCode;

    jobCode=AH_Job_GetCode(workJob);
    if (jobCode && *jobCode && AH_Job_VPP_IsNeededForCode(vppJob, jobCode)) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "VOP needed for job %s", AH_Job_GetCode(workJob));
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
      DBG_ERROR(AQHBCI_LOGDOMAIN, "VOP not needed for job %s", AH_Job_GetCode(workJob));
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



