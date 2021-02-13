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


#include "cbox_queue.h"

#include "aqhbci/admjobs/jobtan_l.h"

#include "aqhbci/applayer/cbox_send.h"
#include "aqhbci/applayer/cbox_recv.h"
#include "aqhbci/applayer/cbox_dialog.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/gui.h>

#include <assert.h>


/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static AH_JOBQUEUE *_createNextQueueFromTodoList(AB_USER *user, AH_JOB_LIST *jl, uint32_t jqFlags,
                                                 AH_JOB_LIST *finishedJobs);
static int _performQueue(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOBQUEUE *jq);
static int _performNonDialogQueues(AH_OUTBOX_CBOX *cbox, AH_JOBQUEUE_LIST *jql);
static int _performDialogQueue(AH_OUTBOX_CBOX *cbox, AH_JOBQUEUE *jq);
static void _extractMatchingQueues(AH_JOBQUEUE_LIST *jql,
                                   AH_JOBQUEUE_LIST *jqlWanted,
                                   AH_JOBQUEUE_LIST *jqlRest,
                                   uint32_t jqflags,
                                   uint32_t jqmask);
static void _handleQueueListError(AH_OUTBOX_CBOX *cbox, AH_JOBQUEUE_LIST *jql, const char *logStr);

static int _sendAndRecvDialogQueues(AH_OUTBOX_CBOX *cbox);
static int _sendAndRecvSelected(AH_OUTBOX_CBOX *cbox, uint32_t jqflags, uint32_t jqmask);
static void _handleQueueError(AH_OUTBOX_CBOX *cbox, AH_JOBQUEUE *jq, const char *logStr);

/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */




int AH_OutboxCBox_SendAndRecvBox(AH_OUTBOX_CBOX *cbox)
{
  int rv;

  /* dialog queues */
  rv=_sendAndRecvDialogQueues(cbox);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error performing dialog queues (%d)", rv);
    return rv;
  }

  /* non-dialog queues: unsigned, uncrypted */
  rv=_sendAndRecvSelected(cbox,
                          0,
                          AH_JOBQUEUE_FLAGS_ISDIALOG |
                          AH_JOBQUEUE_FLAGS_SIGN |
                          AH_JOBQUEUE_FLAGS_CRYPT);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error performing queues (-S, -C: %d)", rv);
    return rv;
  }

  /* non-dialog queues: unsigned, crypted */
  rv=_sendAndRecvSelected(cbox,
                          AH_JOBQUEUE_FLAGS_CRYPT,
                          AH_JOBQUEUE_FLAGS_ISDIALOG |
                          AH_JOBQUEUE_FLAGS_SIGN |
                          AH_JOBQUEUE_FLAGS_CRYPT);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error performing queues (-S, +C: %d)", rv);
    return rv;
  }

  /* non-dialog queues: signed, uncrypted */
  rv=_sendAndRecvSelected(cbox,
                          AH_JOBQUEUE_FLAGS_SIGN,
                          AH_JOBQUEUE_FLAGS_ISDIALOG |
                          AH_JOBQUEUE_FLAGS_SIGN |
                          AH_JOBQUEUE_FLAGS_CRYPT);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error performing queues (+S, -C: %d)", rv);
    return rv;
  }

  /* non-dialog queues: signed, crypted */
  rv=_sendAndRecvSelected(cbox,
                          AH_JOBQUEUE_FLAGS_SIGN |
                          AH_JOBQUEUE_FLAGS_CRYPT,
                          AH_JOBQUEUE_FLAGS_ISDIALOG |
                          AH_JOBQUEUE_FLAGS_SIGN |
                          AH_JOBQUEUE_FLAGS_CRYPT);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error performing queues (+S, +C: %d)", rv);
    return rv;
  }

  return 0;
}



/* NOTE: frees jq! */
int _performQueue(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOBQUEUE *jq)
{
  int rv;
  AB_USER *user;
  AH_JOB_LIST *finishedJobs;

  user=AH_OutboxCBox_GetUser(cbox);
  finishedJobs=AH_OutboxCBox_GetFinishedJobs(cbox);

  for (;;) {
    AH_JOBQUEUE *jqTodo;
    AH_JOB_LIST *jl;

    jl=AH_JobQueue_TakeJobList(jq);
    assert(jl);

    jqTodo=_createNextQueueFromTodoList(user, jl, AH_JobQueue_GetFlags(jq), finishedJobs);
    AH_Job_List_free(jl);
    AH_JobQueue_free(jq);
    if (jqTodo==NULL) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "No more jobs left");
      break;
    }
    jq=jqTodo;

    /* jq now contains all jobs to be executed */
    rv=AH_OutboxCBox_SendAndRecvQueue(cbox, dlg, jq);
    if (rv) {
      _handleQueueError(cbox, jq, "Error performing queue");
      AH_JobQueue_free(jq);
      return rv;
    } /* if error */
  } /* for */

  return 0;
}



AH_JOBQUEUE *_createNextQueueFromTodoList(AB_USER *user, AH_JOB_LIST *jl, uint32_t jqFlags, AH_JOB_LIST *finishedJobs)
{
  AH_JOB *j;
  AH_JOBQUEUE *jqTodo;

  jqTodo=AH_JobQueue_new(user);
  /* copy some flags */
  jqFlags&=~(AH_JOBQUEUE_FLAGS_CRYPT |
             AH_JOBQUEUE_FLAGS_SIGN |
             AH_JOBQUEUE_FLAGS_NOSYSID |
             AH_JOBQUEUE_FLAGS_NOITAN);
  AH_JobQueue_SetFlags(jqTodo, (jqFlags&AH_JOBQUEUE_FLAGS_COPYMASK));

  while ((j=AH_Job_List_First(jl))) {
    AH_Job_List_Del(j);

    if (AH_Job_GetStatus(j)==AH_JobStatusAnswered) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Job status \"answered\", checking whether it needs to be re-enqueued");
      /* prepare job for next message
       * (if attachpoint or multi-message job)
       */
      AH_Job_PrepareNextMessage(j);
      if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_HASMOREMSGS) {
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "Requeueing job");
        /* we shall redo this job */
        if (AH_JobQueue_AddJob(jqTodo, j)!=AH_JobQueueAddResultOk) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Job could not be re-added to queue, SNH!");
          AH_Job_Log(j, GWEN_LoggerLevel_Error, "Could not re-enqueue job");
          AH_Job_SetStatus(j, AH_JobStatusError);
        }
        else {
          AH_Job_Log(j, GWEN_LoggerLevel_Info, "Job re-enqueued (multi-message job)");
          j=NULL; /* mark that this job has been dealt with */
        }
      } /* if more messages */
      else {
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "Job has no messages left, not re-enqueing");
      }
    } /* if status "answered" */

    else if (AH_Job_GetStatus(j)==AH_JobStatusEnqueued) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Job status \"enqueued\", trying to re-enqueue (TODO: Is this really used?)");
      if (AH_JobQueue_AddJob(jqTodo, j)!=AH_JobQueueAddResultOk) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Job could not be re-added to queue, SNH!");
        AH_Job_SetStatus(j, AH_JobStatusError);
        AH_Job_Log(j, GWEN_LoggerLevel_Error, "Could not enqueue job");
      }
      else {
        AH_Job_Log(j, GWEN_LoggerLevel_Info, "Job enqueued (2)");
        j=NULL; /* mark that this job has been dealt with */
      }
    } /* if status "enqueued" */

    else {
      DBG_WARN(AQHBCI_LOGDOMAIN, "Bad status \"%s\" (%d)",
               AH_Job_StatusName(AH_Job_GetStatus(j)),
               AH_Job_GetStatus(j));
      if (GWEN_Logger_GetLevel(0)>=GWEN_LoggerLevel_Debug)
        AH_Job_Dump(j, stderr, 4);
    }

    if (j) {
      /* move job to finished list if we still have the job */
      AH_Job_List_Add(j, finishedJobs);
    }
  } /* while */

  if (AH_JobQueue_GetCount(jqTodo)==0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No jobs enqueued.");
    AH_JobQueue_free(jqTodo);
    return NULL;
  }

  return jqTodo;
}




int _performNonDialogQueues(AH_OUTBOX_CBOX *cbox, AH_JOBQUEUE_LIST *jql)
{
  AB_USER *user;
  AB_PROVIDER *provider;
  AH_DIALOG *dlg;
  AH_JOBQUEUE *jq;
  int rv=0;
  int i;
  uint32_t jqflags;

  user=AH_OutboxCBox_GetUser(cbox);
  provider=AH_OutboxCBox_GetProvider(cbox);

  if (AH_JobQueue_List_GetCount(jql)==0) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "No queues to handle, doing nothing");
    AH_JobQueue_List_free(jql);
    return 0;
  }

  for (i=0; i<2; i++) {
    dlg=AH_Dialog_new(user, provider);
    rv=AH_Dialog_Connect(dlg);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Could not begin a dialog for customer \"%s\" (%d)",
               AB_User_GetCustomerId(user), rv);
      /* finish all queues */
      _handleQueueListError(cbox, jql, "Could not begin dialog");
      AH_Dialog_free(dlg);
      return rv;
    }

    jq=AH_JobQueue_List_First(jql);
    jqflags=AH_JobQueue_GetFlags(jq);

    /* open dialog */
    rv=AH_OutboxCBox_OpenDialog(cbox, dlg, jqflags);
    if (rv==0)
      break;
    else if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Could not open dialog");
      AH_Dialog_Disconnect(dlg);
      /* finish all queues */
      _handleQueueListError(cbox, jql, "Could not open dialog");
      AH_Dialog_free(dlg);
      return rv;
    }
    else if (rv==1) {
      AH_Dialog_Disconnect(dlg);
      AH_Dialog_free(dlg);
      GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Info, I18N("Retrying to open dialog"));
    }
  }
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not open dialog");
    AH_Dialog_Disconnect(dlg);
    /* finish all queues */
    _handleQueueListError(cbox, jql, "Could not open dialog");
    AH_Dialog_free(dlg);
    return rv;
  }

  /* handle queues */
  rv=0;
  while ((jq=AH_JobQueue_List_First(jql))) {
    AH_JobQueue_List_Del(jq);
    rv=_performQueue(cbox, dlg, jq); /* frees jq */
    if (rv)
      break;
  } /* while */

  if (rv) {
    /* finish all remaining queues */
    _handleQueueListError(cbox, jql, "Could not send ");
    AH_Dialog_Disconnect(dlg);
    AH_Dialog_free(dlg);
    return rv;
  }

  /* close dialog */
  rv=AH_OutboxCBox_CloseDialog(cbox, dlg, jqflags);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not close dialog, ignoring");
    /*AH_HBCI_EndDialog(cbox->hbci, dlg);
     return rv;*/
  }

  DBG_INFO(AQHBCI_LOGDOMAIN, "Closing connection");
  AH_Dialog_Disconnect(dlg);
  AH_Dialog_free(dlg);

  AH_JobQueue_List_free(jql);
  return 0;
}



int _performDialogQueue(AH_OUTBOX_CBOX *cbox, AH_JOBQUEUE *jq)
{
  AB_USER *user;
  AB_PROVIDER *provider;
  AH_DIALOG *dlg;
  int rv;
  uint32_t jqFlags;

  user=AH_OutboxCBox_GetUser(cbox);
  provider=AH_OutboxCBox_GetProvider(cbox);

  jqFlags=AH_JobQueue_GetFlags(jq);

  /* open connection */
  dlg=AH_Dialog_new(user, provider);
  rv=AH_Dialog_Connect(dlg);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Could not begin a dialog for customer \"%s\" (%d)",
             AB_User_GetCustomerId(user), rv);
    /* finish all queues */
    _handleQueueError(cbox, jq, "Could not begin dialog");
    AH_Dialog_free(dlg);
    return rv;
  }

#ifdef EXTREME_DEBUGGING
  DBG_ERROR(AQHBCI_LOGDOMAIN, "Handling this job queue:");
  AH_JobQueue_Dump(jq, stderr, 2);
#endif

  if (AH_User_GetCryptMode(user)==AH_CryptMode_Pintan) {
    if (jqFlags & AH_JOBQUEUE_FLAGS_NOITAN) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Not using PSD2 code: Job queue has flag NOITAN set (using single step).");
      AH_Dialog_SetItanMethod(dlg, 999);
      AH_Dialog_SetItanProcessType(dlg, 1);
      AH_Dialog_SetTanJobVersion(dlg, 0);
    }
    else {
      int selectedTanVersion;

      /* select iTAN mode */
      DBG_INFO(AQHBCI_LOGDOMAIN, "Job queue doesn't have flag NOITAN");
      rv=AH_OutboxCBox_SelectItanMode(cbox, dlg);
      if (rv) {
        AH_Dialog_Disconnect(dlg);
        AH_Dialog_free(dlg);
        return rv;
      }

      selectedTanVersion=AH_User_GetSelectedTanMethod(user)/1000;
      if (selectedTanVersion>=6) {
        AH_JOB *jTan;

        DBG_INFO(AQHBCI_LOGDOMAIN, "User-selected TAN job version is 6 or newer (%d)", selectedTanVersion);

        /* check for PSD2: HKTAN version 6 available? if so -> use that */
        jTan=AH_Job_Tan_new(provider, user, 4, 6);
        if (jTan) {
          AH_Job_free(jTan);
          DBG_INFO(AQHBCI_LOGDOMAIN, "TAN job version 6 is available");
          DBG_NOTICE(AQHBCI_LOGDOMAIN, "Using PSD2 code for dialog job");
          AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_NEEDTAN);
          AH_Dialog_AddFlags(dlg, AH_DIALOG_FLAGS_SCA);
        }
        else {
          DBG_NOTICE(AQHBCI_LOGDOMAIN, "Not using PSD2 code: HKTAN version 6 not supported by the bank");
        }
      }
      else {
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "Not using PSD2 code: User selected HKTAN version lesser than 6.");
      }
    }
  }

  /* handle queue */
  rv=_performQueue(cbox, dlg, jq);
  if (rv) {
    AH_Dialog_Disconnect(dlg);
    AH_Dialog_free(dlg);
    return rv;
  }

  /* close dialog */
#if 0
  if (AH_User_GetCryptMode(user)==AH_CryptMode_Pintan &&
      (jqFlags & AH_JOBQUEUE_FLAGS_NOITAN)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Changing dialog to anonymous mode");
    AH_Dialog_AddFlags(dlg, AH_DIALOG_FLAGS_ANONYMOUS);
  }
#endif

  rv=AH_OutboxCBox_CloseDialog(cbox, dlg, jqFlags);
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



void _extractMatchingQueues(AH_JOBQUEUE_LIST *jql,
                            AH_JOBQUEUE_LIST *jqlWanted,
                            AH_JOBQUEUE_LIST *jqlRest,
                            uint32_t jqflags,
                            uint32_t jqmask)
{
  AH_JOBQUEUE *jq;

  while ((jq=AH_JobQueue_List_First(jql))) {
    uint32_t flags;

    AH_JobQueue_List_Del(jq);
    flags=AH_JobQueue_GetFlags(jq);
    if ((flags^jqflags)  & jqmask)
      /* no match */
      AH_JobQueue_List_Add(jq, jqlRest);
    else
      AH_JobQueue_List_Add(jq, jqlWanted);
  } /* while */
}



void _handleQueueListError(AH_OUTBOX_CBOX *cbox, AH_JOBQUEUE_LIST *jql, const char *logStr)
{
  AH_JOBQUEUE *jq;

  while ((jq=AH_JobQueue_List_First(jql))) {
    AH_JobQueue_List_Del(jq);
    _handleQueueError(cbox, jq, logStr);
  } /* while */
  AH_JobQueue_List_free(jql);
}



int _sendAndRecvDialogQueues(AH_OUTBOX_CBOX *cbox)
{
  AH_JOBQUEUE_LIST *todoQueues;
  AH_JOBQUEUE_LIST *jqlWanted;
  AH_JOBQUEUE_LIST *jqlRest;
  int rv;

  todoQueues=AH_OutboxCBox_TakeTodoQueues(cbox);

  jqlWanted=AH_JobQueue_List_new();
  jqlRest=AH_JobQueue_List_new();
  _extractMatchingQueues(todoQueues, jqlWanted, jqlRest, AH_JOBQUEUE_FLAGS_ISDIALOG, AH_JOBQUEUE_FLAGS_ISDIALOG);
  AH_JobQueue_List_free(todoQueues); /* is empty now */
  AH_OutboxCBox_SetTodoQueues(cbox, jqlRest);
  todoQueues=jqlRest;

  if (AH_JobQueue_List_GetCount(jqlWanted)) {
    AH_JOBQUEUE *jq;

    /* there are matching queues, handle them */
    while ((jq=AH_JobQueue_List_First(jqlWanted))) {
      AH_JobQueue_List_Del(jq);
      rv=_performDialogQueue(cbox, jq);
      if (rv) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Error performing queue (%d)", rv);
        _handleQueueListError(cbox, jqlWanted, "Could not perform dialog queue");
        //_handleQueueListError(cbox, todoQueues, "Could not perform dialog queue");
        //AH_OutboxCBox_SetTodoQueues(cbox, AH_JobQueue_List_new());
        return rv;
      }
    } /* while */
  }
  AH_JobQueue_List_free(jqlWanted);
  return 0;
}



int _sendAndRecvSelected(AH_OUTBOX_CBOX *cbox, uint32_t jqflags, uint32_t jqmask)
{
  AH_JOBQUEUE_LIST *todoQueues;
  AH_JOBQUEUE_LIST *jqlWanted;
  AH_JOBQUEUE_LIST *jqlRest;
  int rv;

  todoQueues=AH_OutboxCBox_TakeTodoQueues(cbox);

  jqlWanted=AH_JobQueue_List_new();
  jqlRest=AH_JobQueue_List_new();
  _extractMatchingQueues(todoQueues, jqlWanted, jqlRest, jqflags, jqmask);
  AH_JobQueue_List_free(todoQueues); /* is empty now */
  AH_OutboxCBox_SetTodoQueues(cbox, jqlRest);
  todoQueues=jqlRest;

  if (AH_JobQueue_List_GetCount(jqlWanted)) {
    /* there are matching queues, handle them */
    rv=_performNonDialogQueues(cbox, jqlWanted);
    if (rv<0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Error performing queue (%d)", rv);
      //_handleQueueListError(cbox, todoQueues, "Error performing selected jobs");
      //AH_OutboxCBox_SetTodoQueues(cbox, AH_JobQueue_List_new());
      return rv;
    }
  } /* if matching queuees */
  else
    AH_JobQueue_List_free(jqlWanted);
  return 0;
}



int AH_OutboxCBox_SendAndRecvQueueNoTan(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOBQUEUE *jq)
{
  int rv;

  rv=AH_OutboxCBox_SendQueue(cbox, dlg, jq);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error sending queue");
    return rv;
  }

  AH_JobQueue_SetJobStatusOnMatch(jq, AH_JobStatusEncoded, AH_JobStatusSent);

  rv=AH_OutboxCBox_RecvQueue(cbox, dlg, jq);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error receiving queue response");
    return rv;
  }

  return 0;
}



int AH_OutboxCBox_SendAndRecvQueue(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOBQUEUE *jq)
{
  int rv;

  if ((AH_JobQueue_GetFlags(jq) & AH_JOBQUEUE_FLAGS_NEEDTAN) &&
      AH_Dialog_GetItanProcessType(dlg)!=0) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "TAN mode");
    rv=AH_OutboxCBox_SendAndReceiveQueueWithTan(cbox, dlg, jq);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  else {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Normal mode");
    rv=AH_OutboxCBox_SendAndRecvQueueNoTan(cbox, dlg, jq);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}



void _handleQueueError(AH_OUTBOX_CBOX *cbox, AH_JOBQUEUE *jq, const char *logStr)
{
  AH_JOB_LIST *finishedJobs;
  AH_JOB *j;
  AH_JOB_LIST *jl;

  finishedJobs=AH_OutboxCBox_GetFinishedJobs(cbox);

  jl=AH_JobQueue_TakeJobList(jq);
  assert(jl);

  while ((j=AH_Job_List_First(jl))) {
    AH_Job_List_Del(j);
    if (AH_Job_GetStatus(j)!=AH_JobStatusAnswered) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Setting status of job \"%s\" to ERROR", AH_Job_GetName(j));
      AH_Job_SetStatus(j, AH_JobStatusError);
      if (logStr)
        AH_Job_Log(j, GWEN_LoggerLevel_Error, logStr);
    }
    AH_Job_List_Add(j, finishedJobs);
  }
  AH_Job_List_free(jl);
  AH_JobQueue_free(jq);
}



void AH_OutboxCBox_Finish(AH_OUTBOX_CBOX *cbox)
{
  AH_JOB_LIST *finishedJobs;
  AH_JOBQUEUE_LIST *todoQueues;
  AH_JOB_LIST *todoJobs;
  AH_JOBQUEUE *jq;

  assert(cbox);

  finishedJobs=AH_OutboxCBox_GetFinishedJobs(cbox);
  todoQueues=AH_OutboxCBox_GetTodoQueues(cbox);
  todoJobs=AH_OutboxCBox_GetTodoJobs(cbox);

  DBG_INFO(AQHBCI_LOGDOMAIN, "Finishing customer box");

  while ((jq=AH_JobQueue_List_First(todoQueues))) {
    AH_JOB_LIST *jl;
    AH_JOB *j;

    jl=AH_JobQueue_TakeJobList(jq);
    assert(jl);
    while ((j=AH_Job_List_First(jl))) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Moving job \"%s\" from todo queue to finished jobs", AH_Job_GetName(j));
      AH_Job_List_Del(j);
      AH_Job_List_Add(j, finishedJobs);
    } /* while */
    AH_Job_List_free(jl);
    AH_JobQueue_free(jq);
  } /* while */

  if (AH_Job_List_GetCount(todoJobs)) {
    AH_JOB *j;

    while ((j=AH_Job_List_First(todoJobs))) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Moving job \"%s\" from todo queue to finished jobs",
               AH_Job_GetName(j));
      AH_Job_List_Del(j);
      AH_Job_List_Add(j, finishedJobs);
    } /* while */
  }
}



