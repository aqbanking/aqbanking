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


static int _sendAndReceiveTanResponseProc2(AH_OUTBOX_CBOX *cbox,
                                           AH_DIALOG *dlg,
                                           AH_JOBQUEUE *qJob,
                                           AH_JOB *jTan1);

static void _dispatchJobSegResultsToQueue(AH_JOB *job, AH_JOBQUEUE *qJob);
static void _dispatchJobMsgResultsToQueue(AH_JOB *job, AH_JOBQUEUE *qJob);




/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */


int AH_OutboxCBox_SendAndReceiveQueueWithTan2(AH_OUTBOX_CBOX *cbox,
                                              AH_DIALOG *dlg,
                                              AH_JOBQUEUE *qJob)
{
  int rv;
  AB_PROVIDER *provider;
  AH_JOB *job;
  AH_JOB *jTan1;
  AB_USER *u;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Sending job with TAN (process variant 2)");

  provider=AH_OutboxCBox_GetProvider(cbox);

  assert(qJob);
  assert(AH_JobQueue_GetCount(qJob)==1);
  job=AH_JobQueue_GetFirstJob(qJob);
  assert(job);
  u=AH_Job_GetUser(job);
  assert(u);

  /* don't need a TAN for this particular message, just a HKTAN segment */
  AH_JobQueue_SubFlags(qJob, AH_JOBQUEUE_FLAGS_NEEDTAN);

  /* prepare HKTAN (process type 4) */
  jTan1=AH_Job_Tan_new(provider, u, 4, AH_Dialog_GetTanJobVersion(dlg));
  if (!jTan1) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job HKTAN not available");
    return GWEN_ERROR_GENERIC;
  }
  AH_Job_Tan_SetTanMediumId(jTan1, AH_User_GetTanMediumId(u));
  if (AH_Dialog_GetFlags(dlg) &  AH_DIALOG_FLAGS_SCA) {
    /* With tan process "4" only with strong verification */
    AH_Job_Tan_SetSegCode(jTan1, AH_Job_GetCode(job));
  }

  /* copy signers */
  if (AH_Job_GetFlags(job) & AH_JOB_FLAGS_SIGN) {
    rv=AH_Job_AddSigners(jTan1, AH_Job_GetSigners(job));
    if (rv<1) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Signatures needed but no signer given");
      return GWEN_ERROR_INVALID;
    }
  }

  /* add job to queue */
  rv=AH_JobQueue_AddJob(qJob, jTan1);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_free(jTan1);
    return rv;
  }

  /* send original job with HKTAN, receive response and dispatch it to those jobs */
  rv=AH_OutboxCBox_SendAndRecvQueueNoTan(cbox, dlg, qJob);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  if (AH_Job_HasResultWithCode(job, 3076) ||
      AH_Job_HasResultWithCode(jTan1, 3076)) { /* SCA not needed */
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "No TAN needed");
  }
  else {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Job has no 3076 result, getting TAN");
#if 1
    DBG_ERROR(0, "Original job:");
    AH_Job_Dump(job, stderr, 2);

    DBG_ERROR(0, "TAN job:");
    AH_Job_Dump(jTan1, stderr, 2);
#endif

    rv=_sendAndReceiveTanResponseProc2(cbox, dlg, qJob, jTan1);
    if (rv) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}



int AH_OutboxCBox_SendAndReceiveJobWithTan2(AH_OUTBOX_CBOX *cbox,
                                            AH_DIALOG *dlg,
                                            AH_JOB *job)
{
  AB_USER *user;
  AH_JOBQUEUE *qJob;
  int rv;

  user=AH_OutboxCBox_GetUser(cbox);

  qJob=AH_JobQueue_new(user);

  /* add original job to queue */
  AH_Job_Attach(job);
  rv=AH_JobQueue_AddJob(qJob, job);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_JobQueue_free(qJob);
    return rv;
  }

  /* send queue */
  rv=AH_OutboxCBox_SendAndReceiveQueueWithTan2(cbox, dlg, qJob);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_JobQueue_free(qJob);
    return rv;
  }

  AH_JobQueue_free(qJob);
  return rv;
}



int _sendAndReceiveTanResponseProc2(AH_OUTBOX_CBOX *cbox,
                                    AH_DIALOG *dlg,
                                    AH_JOBQUEUE *qJob,
                                    AH_JOB *jTan1)
{
  AH_OUTBOX *outbox;
  AB_PROVIDER *provider;
  int rv;
  AH_JOB *j;
  AB_USER *u;
  const char *challenge;
  const char *challengeHhd;
  AH_JOBQUEUE *qJob2=NULL;
  AH_MSG *msg2;
  AH_JOB *jTan2;

  assert(qJob);
  assert(jTan1);

  provider=AH_OutboxCBox_GetProvider(cbox);
  outbox=AH_OutboxCBox_GetOutbox(cbox);

  j=AH_JobQueue_GetFirstJob(qJob);
  assert(j);
  u=AH_Job_GetUser(j);
  assert(u);

  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing job \"%s\"", AH_Job_GetName(jTan1));
  rv=AH_Job_Process(jTan1, AH_Outbox_GetImExContext(outbox));
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  challengeHhd=AH_Job_Tan_GetHhdChallenge(jTan1);
  challenge=AH_Job_Tan_GetChallenge(jTan1);

  /* prepare second message (the one with the TAN) */
  qJob2=AH_JobQueue_fromQueue(qJob);
  msg2=AH_Msg_new(dlg);
  AH_Msg_SetNeedTan(msg2, 1);
  AH_Msg_SetItanMethod(msg2, 0);
  AH_Msg_SetItanHashMode(msg2, 0);

  /* ask for TAN */
  if (challenge || challengeHhd) {
    char tanBuffer[64];

    memset(tanBuffer, 0, sizeof(tanBuffer));
    rv=AH_OutboxCBox_InputTanWithChallenge(cbox,
                                           dlg,
                                           challenge,
                                           challengeHhd,
                                           tanBuffer,
                                           1,
                                           sizeof(tanBuffer)-1);
    if (rv) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      AH_Msg_free(msg2);
      return rv;
    }

    /* set TAN in msg 2 */
    AH_Msg_SetTan(msg2, tanBuffer);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No challenge received");
    AH_Msg_free(msg2);
    return GWEN_ERROR_BAD_DATA;
  }


  /* prepare HKTAN (process type 2) */
  jTan2=AH_Job_Tan_new(provider, u, 2, AH_Dialog_GetTanJobVersion(dlg));
  if (!jTan2) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job HKTAN not available");
    AH_Job_free(jTan2);
    AH_Msg_free(msg2);
    return GWEN_ERROR_GENERIC;
  }
  AH_Job_Tan_SetReference(jTan2, AH_Job_Tan_GetReference(jTan1));
  AH_Job_Tan_SetTanMediumId(jTan2, AH_User_GetTanMediumId(u));
  AH_Job_AddFlags(jTan2, AH_JOB_FLAGS_NEEDTAN);

  /* With tan process "4" only with strong verification
   if (AH_Dialog_GetFlags(dlg) & AH_DIALOG_FLAGS_SCA)
   AH_Job_Tan_SetSegCode(jTan2, AH_Job_GetCode(j));
  */

  /* copy signers */
  if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_SIGN) {
    if (AH_Job_AddSigners(jTan2, AH_Job_GetSigners(j))<1) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Signatures needed but no signer given");
      AH_Job_free(jTan2);
      AH_Msg_free(msg2);
      AH_JobQueue_free(qJob2);
      return GWEN_ERROR_INVALID;
    }
  }

  rv=AH_JobQueue_AddJob(qJob2, jTan2);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_free(jTan2);
    AH_Msg_free(msg2);
    AH_JobQueue_free(qJob2);
    return rv;
  }

  rv=AH_OutboxCBox_JobToMessage(jTan2, msg2, 1);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg2);
    AH_JobQueue_free(qJob2);
    return rv;
  }

  /* encode HKTAN message */
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Encoding queue");
  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Info, I18N("Encoding queue"));
  AH_Msg_SetNeedTan(msg2, 1);
  rv=AH_Msg_EncodeMsg(msg2);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg2);
    AH_JobQueue_free(qJob2);
    return rv;
  }

  /* store used TAN in original job (if any) */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Storing TAN in job [%s]", AH_Job_GetName(j));
  AH_Job_SetUsedTan(j, AH_Msg_GetTan(msg2));

  if (AH_Job_GetStatus(jTan2)==AH_JobStatusEncoded) {
    const char *s;

    AH_Job_SetMsgNum(jTan2, AH_Msg_GetMsgNum(msg2));
    AH_Job_SetDialogId(jTan2, AH_Dialog_GetDialogId(dlg));
    /* store expected signer and crypter (if any) */
    s=AH_Msg_GetExpectedSigner(msg2);
    if (s)
      AH_Job_SetExpectedSigner(jTan2, s);
    s=AH_Msg_GetExpectedCrypter(msg2);
    if (s)
      AH_Job_SetExpectedCrypter(jTan2, s);

    /* store TAN in TAN job */
    AH_Job_SetUsedTan(jTan2, AH_Msg_GetTan(msg2));
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "jTAN2 not encoded? (%d)", AH_Job_GetStatus(jTan2));
  }

  /* send message */
  rv=AH_OutboxCBox_SendMessage(cbox, dlg, msg2);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg2);
    AH_JobQueue_free(qJob2);
    return rv;
  }

  /* receive response */
  rv=AH_OutboxCBox_RecvQueue(cbox, dlg, qJob);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg2);
    AH_JobQueue_free(qJob2);
    return rv;
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Processing job \"%s\"", AH_Job_GetName(jTan2));
    rv=AH_Job_Process(jTan2, AH_Outbox_GetImExContext(outbox));
    if (rv) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      AH_Msg_free(msg2);
      AH_JobQueue_free(qJob2);
      return rv;
    }

    /* dispatch results from jTan2 to all members of the queue */
    _dispatchJobSegResultsToQueue(jTan2, qJob);
    _dispatchJobMsgResultsToQueue(jTan2, qJob);
  }


  /* store used TAN in original job (if any) */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Storing TAN in job [%s]", AH_Job_GetName(j));
  AH_Job_SetUsedTan(j, AH_Msg_GetTan(msg2));

  AH_Msg_free(msg2);
  AH_JobQueue_free(qJob2);

  return 0;
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
      while (origRes) {
        AH_JOB *qj;

        qj=AH_Job_List_First(qjl);
        while (qj) {
          if (qj!=job) {
            AH_RESULT *nr;

            nr=AH_Result_dup(origRes);
            DBG_ERROR(AQHBCI_LOGDOMAIN, "Adding result %d to job %s", AH_Result_GetCode(origRes), AH_Job_GetName(qj));
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




