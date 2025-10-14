/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2025 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "cbox_queue.h"

#include "aqhbci/admjobs/jobacknowledge_l.h"
#include "aqhbci/admjobs/jobtan_l.h"

#include "aqhbci/applayer/cbox_send.h"
#include "aqhbci/applayer/cbox_recv.h"
#include "aqhbci/applayer/cbox_dialog.h"
#include "aqhbci/applayer/cbox_voptan.h"
#include "aqhbci/applayer/cbox_vophbci.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/gui.h>

#include <assert.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static AH_DIALOG *_openDialog(AH_OUTBOX_CBOX *cbox, uint32_t jFlags);
static int _sendMatchingNonDialogJobs(AH_OUTBOX_CBOX *cbox, AH_JOB_LIST *jobList, uint32_t jFlags, uint32_t jMask);
static int _sendMatchingFromJobList(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB_LIST *jobList, uint32_t jFlags, uint32_t jMask);
static int _sendJob(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *j);
static int _sendDialogJobs(AH_OUTBOX_CBOX *cbox);
static int _sendDialogJob(AH_OUTBOX_CBOX *cbox, AH_JOB *j);
static AH_JOB *_possiblyCreateAckJob(AH_JOB *j);
static int _jobIsDone(AH_JOB *j);


/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */




int AH_OutboxCBox_SendAndRecvBox(AH_OUTBOX_CBOX *cbox)
{
  int rv;
  AH_JOB_LIST *todoJobs;

  todoJobs=AH_OutboxCBox_GetTodoJobs(cbox);

  /* dialog queues */
  rv=_sendDialogJobs(cbox);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error performing dialog queues (%d)", rv);
    return rv;
  }

  /* non-dialog queues: unsigned, uncrypted */
  rv=_sendMatchingNonDialogJobs(cbox, todoJobs,
                                0,
                                AH_JOB_FLAGS_DLGJOB | AH_JOB_FLAGS_SIGN | AH_JOB_FLAGS_CRYPT);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error performing queues (-S, -C: %d)", rv);
    return rv;
  }


  /* non-dialog queues: unsigned, crypted */
  rv=_sendMatchingNonDialogJobs(cbox, todoJobs,
                                AH_JOB_FLAGS_CRYPT,
                                AH_JOB_FLAGS_DLGJOB | AH_JOB_FLAGS_SIGN | AH_JOB_FLAGS_CRYPT);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error performing queues (-S, +C: %d)", rv);
    return rv;
  }

  /* non-dialog queues: signed, uncrypted */
  rv=_sendMatchingNonDialogJobs(cbox, todoJobs,
                                AH_JOB_FLAGS_SIGN,
                                AH_JOB_FLAGS_DLGJOB | AH_JOB_FLAGS_SIGN | AH_JOB_FLAGS_CRYPT);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error performing queues (+S, -C: %d)", rv);
    return rv;
  }

  /* non-dialog queues: signed, crypted */
  rv=_sendMatchingNonDialogJobs(cbox, todoJobs,
                                AH_JOB_FLAGS_SIGN | AH_JOB_FLAGS_CRYPT,
                                AH_JOB_FLAGS_DLGJOB | AH_JOB_FLAGS_SIGN | AH_JOB_FLAGS_CRYPT);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error performing queues (+S, +C: %d)", rv);
    return rv;
  }

  return 0;
}



AH_DIALOG *_openDialog(AH_OUTBOX_CBOX *cbox, uint32_t jFlags)
{
  AB_PROVIDER *provider;
  AB_USER *user;
  AH_DIALOG *dlg;
  int rv;
  const char *sCustomer;

  user=AH_OutboxCBox_GetUser(cbox);
  provider=AH_OutboxCBox_GetProvider(cbox);
  sCustomer=AB_User_GetCustomerId(user);

  dlg=AH_Dialog_new(user, provider);
  rv=AH_Dialog_Connect(dlg);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not begin a dialog for customer \"%s\" (%d)", sCustomer?sCustomer:"", rv);
    AH_Dialog_free(dlg);
    return NULL;
  }

  /* open dialog */
  rv=AH_OutboxCBox_OpenDialog(cbox, dlg, jFlags);
  if (rv==0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Dialog open.");
    return dlg;
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not open dialog (%d)", rv);
    AH_Dialog_Disconnect(dlg);
    AH_Dialog_free(dlg);
    return NULL;
  }
}



int _sendMatchingNonDialogJobs(AH_OUTBOX_CBOX *cbox, AH_JOB_LIST *jobList, uint32_t jFlags, uint32_t jMask)
{
  AH_JOB *j;

  j=AH_Job_List_First(jobList);
  while (j) {
    if (!((AH_Job_GetFlags(j)^jFlags) & jMask))
      break;
    j=AH_Job_List_Next(j);
  }
  if (j) {
    AH_DIALOG *dlg;
    int rv;

    /* we have at least one matching job */
    dlg=_openDialog(cbox, jFlags);
    if (dlg==NULL) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here");
      return GWEN_ERROR_GENERIC;
    }

    rv=_sendMatchingFromJobList(cbox, dlg, jobList, jFlags, jMask);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      AH_Dialog_Disconnect(dlg);
      AH_Dialog_free(dlg);
      return rv;
    }

    /* close dialog */
    rv=AH_OutboxCBox_CloseDialog(cbox, dlg, jFlags);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Could not close dialog, ignoring");
      /*AH_HBCI_EndDialog(cbox->hbci, dlg);
       return rv;*/
    }

    DBG_INFO(AQHBCI_LOGDOMAIN, "Closing connection");
    AH_Dialog_Disconnect(dlg);
    AH_Dialog_free(dlg);
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No matching jobs for flags %08x & %08x", jFlags, jMask);
  }

  return 0;
}



int _sendDialogJobs(AH_OUTBOX_CBOX *cbox)
{
  AH_JOB_LIST *finishedJobs;
  AH_JOB *j;
  AH_JOB_LIST *jobList;

  finishedJobs=AH_OutboxCBox_GetFinishedJobs(cbox);
  jobList=AH_OutboxCBox_GetTodoJobs(cbox);

  j=AH_Job_List_First(jobList);
  while (j) {
    AH_JOB *jNext;

    jNext=AH_Job_List_Next(j);
    if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_DLGJOB) {
      int rv;

      AH_Job_List_Del(j);
      rv=_sendDialogJob(cbox, j);
      AH_Job_List_Add(j, finishedJobs);
      if (rv<0) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }
    j=jNext;
  } /* while */

  return 0;
}



int _sendDialogJob(AH_OUTBOX_CBOX *cbox, AH_JOB *j)
{
  AB_USER *user;
  AB_PROVIDER *provider;
  AH_DIALOG *dlg;
  int rv;
  uint32_t jFlags;

  user=AH_OutboxCBox_GetUser(cbox);
  provider=AH_OutboxCBox_GetProvider(cbox);

  jFlags=AH_Job_GetFlags(j);

  /* open connection */
  dlg=AH_Dialog_new(user, provider);
  rv=AH_Dialog_Connect(dlg);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not begin a dialog for customer \"%s\" (%d)", AB_User_GetCustomerId(user), rv);
    /* finish all queues */
    AH_Dialog_free(dlg);
    return rv;
  }

  rv=AH_OutboxCBox_OpenDialogWithJob(cbox, dlg, j);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Dialog_Disconnect(dlg);
    AH_Dialog_free(dlg);
    return rv;
  }

  /* close dialog */
  rv=AH_OutboxCBox_CloseDialog(cbox, dlg, jFlags);
  if (rv) {
    AH_Dialog_Disconnect(dlg);
    AH_Dialog_free(dlg);
    return rv;
  }

  /* close connection */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Closing connection");
  AH_Dialog_Disconnect(dlg);
  AH_Dialog_free(dlg);

  return 0;

}



int _sendMatchingFromJobList(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB_LIST *jobList, uint32_t jFlags, uint32_t jMask)
{
  AH_JOB_LIST *finishedJobs;
  AH_JOB *j;

  finishedJobs=AH_OutboxCBox_GetFinishedJobs(cbox);

  j=AH_Job_List_First(jobList);
  while (j) {
    AH_JOB *jNext;

    jNext=AH_Job_List_Next(j);
    if (!((AH_Job_GetFlags(j)^jFlags)  & jMask)) {
      int rv;

      AH_Job_List_Del(j);
      rv=_sendJob(cbox, dlg, j);
      AH_Job_List_Add(j, finishedJobs);
      if (rv<0) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }
    j=jNext;
  } /* while */

  return 0;
}



int _sendJob(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *j)
{
  int rv;
  AH_JOB *jAck;

  DBG_ERROR(AQHBCI_LOGDOMAIN, "Sending job %s", AH_Job_GetName(j));
  rv=AH_Job_Prepare(j);
  if (rv<0 && rv!=GWEN_ERROR_NOT_SUPPORTED) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_SetStatus(j, AH_JobStatusError);
    AB_Banking_LogMsgForJobId(AH_Job_GetBankingApi(j), AH_Job_GetId(j), "Error on AH_Job_Prepare(): %d", rv);
    return rv;
  }

  rv=AH_OutboxCBox_SendAndReceiveJob(cbox, dlg, j);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* handle attach points and multi-msg jobs */
  while(!_jobIsDone(j)) {
    rv=AH_OutboxCBox_SendAndReceiveJobWithTanAndVpp(cbox, dlg, j);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  /* handle ACK jobs */
  jAck=_possiblyCreateAckJob(j);
  if (jAck) {
    rv=AH_Job_Prepare(jAck);
    if (rv<0 && rv!=GWEN_ERROR_NOT_SUPPORTED) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      AB_Banking_LogMsgForJobId(AH_Job_GetBankingApi(j), AH_Job_GetId(j), "Error on AH_Job_Prepare() for ACK job: %d", rv);
      AH_Job_free(jAck);
      return rv;
    }

    rv=AH_OutboxCBox_SendAndReceiveJobWithTanAndVpp(cbox, dlg, jAck);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      AH_Job_free(jAck);
      return rv;
    }
  }

  return 0;
}



int AH_OutboxCBox_SendAndReceiveJob(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *j)
{
  AB_USER *user;
  int rv;

  user=AH_OutboxCBox_GetUser(cbox);
  if (AH_User_GetCryptMode(user)==AH_CryptMode_Pintan) {
    /* PIN/TAN mode */
    DBG_ERROR(AQHBCI_LOGDOMAIN, "PIN/TAN mode");
    if ((AH_Job_GetFlags(j) & AH_JOB_FLAGS_NEEDTAN) && AH_Dialog_GetItanProcessType(dlg)!=0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "TAN mode");
      rv=AH_OutboxCBox_SendAndReceiveJobWithTanAndVpp(cbox, dlg, j);
      if (rv<0) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      }
      return rv;
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "noTAN mode");
      rv=AH_OutboxCBox_SendAndReceiveJobWithVpp(cbox, dlg, j);
      if (rv<0) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      }
      return rv;
    }
  }
  else {
    /* HBCI mode (not PIN/TAN) */
    DBG_ERROR(AQHBCI_LOGDOMAIN, "HBCI mode");
    rv=AH_OutboxCBox_SendAndReceiveJobWithVpp(cbox, dlg, j);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    }
    return rv;
  }
}



int AH_OutboxCBox_SendAndReceiveJobNoTan(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *j)
{
  AB_USER *user;
  AH_JOBQUEUE *jobQueue;
  int rv;

  user=AH_OutboxCBox_GetUser(cbox);
  jobQueue=AH_JobQueue_new(user);

  AH_Job_Attach(j);
  rv=AH_JobQueue_AddJob(jobQueue, j);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_JobQueue_free(jobQueue);
    return rv;
  }

  rv=AH_OutboxCBox_SendQueue(cbox, dlg, jobQueue);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error sending queue");
    AH_JobQueue_free(jobQueue);
    return rv;
  }

  AH_JobQueue_SetJobStatusOnMatch(jobQueue, AH_JobStatusEncoded, AH_JobStatusSent);

  rv=AH_OutboxCBox_RecvQueue(cbox, dlg, jobQueue);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error receiving queue response");
    AH_JobQueue_free(jobQueue);
    return rv;
  }
  AH_JobQueue_free(jobQueue);

  return 0;
}



void AH_OutboxCBox_Finish(AH_OUTBOX_CBOX *cbox)
{
  AH_JOB_LIST *finishedJobs;
  AH_JOB_LIST *todoJobs;

  assert(cbox);

  finishedJobs=AH_OutboxCBox_GetFinishedJobs(cbox);
  todoJobs=AH_OutboxCBox_GetTodoJobs(cbox);

  DBG_INFO(AQHBCI_LOGDOMAIN, "Finishing customer box");

  if (AH_Job_List_GetCount(todoJobs)) {
    AH_JOB *j;

    while ((j=AH_Job_List_First(todoJobs))) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Moving job \"%s\" from todo queue to finished jobs", AH_Job_GetName(j));
      AH_Job_List_Del(j);
      AH_Job_List_Add(j, finishedJobs);
    } /* while */
  }
}



AH_JOB *_possiblyCreateAckJob(AH_JOB *j)
{
  AH_JOB *jAck=NULL;
  const char *jobName;

  jobName=AH_Job_GetName(j);
  if (!(jobName && *jobName))
    jobName="<unnamed>";

  if (AH_Job_GetStatus(j)==AH_JobStatusAnswered) {
    const void *ackCode=NULL;
    unsigned int lenAckCode=0;
    GWEN_DB_NODE *dbArgs=AH_Job_GetArguments(j);

    /* Should we send an acknowledgement for the previously executed job? */
    DBG_INFO(AQHBCI_LOGDOMAIN, "Job \"%s\" with status \"answered\", checking whether it needs acknowledgement", jobName);
    ackCode=GWEN_DB_GetBinValue(dbArgs, "_tmpAckCode", 0, 0, 0, &lenAckCode);
    if (ackCode && lenAckCode) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Job \"%s\" received acknowledge code, prepare acknowledge job", jobName);
      jAck=AH_Job_Acknowledge_new(AH_Job_GetProvider(j), AH_Job_GetUser(j), ackCode, lenAckCode);

      /* copy signers to new job */
      if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_SIGN) {
        GWEN_STRINGLISTENTRY *se;

        se=GWEN_StringList_FirstEntry(AH_Job_GetSigners(j));
        while(se) {
          AH_Job_AddSigner(jAck, GWEN_StringListEntry_Data(se));
          se=GWEN_StringListEntry_Next(se);
        } /* while */
      }
      AH_Job_Log(j, GWEN_LoggerLevel_Info, "Acknowledge Job created");

      if (GWEN_DB_DeleteVar(dbArgs, "_tmpAckCode")) {
        DBG_DEBUG(AQHBCI_LOGDOMAIN, "Temporary acknowledge code removed");
      }
    }
    else {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Job \"%s\" didn't receive an acknowledge code, no acknowledge job needed.", jobName);
    }
  }

  return jAck;
}



int _jobIsDone(AH_JOB *j)
{
  const char *jobName;

  jobName=AH_Job_GetName(j);
  if (!(jobName && *jobName))
    jobName="<unnamed>";

  if (AH_Job_GetStatus(j)==AH_JobStatusAnswered) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Job \"%s\" with status \"answered\", checking whether it needs to be re-enqueued", jobName);
    AB_Banking_LogMsgForJobId(AH_Job_GetBankingApi(j), AH_Job_GetId(j), "Job status \"answered\", checking for re-enqueing");
    /* prepare job for next message (if attachpoint or multi-message job) */
    AH_Job_PrepareNextMessage(j);
    if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_HASMOREMSGS) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Job \"%s\" has more messages", jobName);
      return 0;
    } /* if more messages */
    else {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Job \"%s\" has no messages left, not re-enqueueing", jobName);
    }
  } /* if status "answered" */
  return 1;
}




