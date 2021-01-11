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


#include "cbox_p.h"
#include "aqhbci/aqhbci_l.h"
#include "aqhbci/msglayer/hbci_l.h"
#include "aqhbci/msglayer/dialog_l.h"
#include "aqhbci/ajobs/accountjob_l.h"
#include "aqhbci/admjobs/jobtan_l.h"
#include "aqhbci/joblayer/job_l.h"
#include "aqhbci/joblayer/jobqueue_l.h"
#include "aqhbci/banking/provider_l.h"

#include "aqhbci/applayer/adminjobs_l.h"
#include "aqhbci/applayer/cbox_send.h"
#include "aqhbci/applayer/cbox_recv.h"
#include "aqhbci/applayer/cbox_dialog.h"

#include <aqbanking/banking_be.h>
#include <aqbanking/backendsupport/imexporter.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/gui.h>

#include <assert.h>


/*#define EXTREME_DEBUGGING */


GWEN_LIST_FUNCTIONS(AH_OUTBOX_CBOX, AH_OutboxCBox);




AH_OUTBOX_CBOX *AH_OutboxCBox_new(AB_PROVIDER *pro, AB_USER *u, AH_OUTBOX *ob)
{
  AH_OUTBOX_CBOX *cbox;

  assert(pro);
  assert(u);
  GWEN_NEW_OBJECT(AH_OUTBOX_CBOX, cbox);
  cbox->usage=1;
  GWEN_LIST_INIT(AH_OUTBOX_CBOX, cbox);
  cbox->user=u;
  cbox->todoQueues=AH_JobQueue_List_new();
  cbox->finishedQueues=AH_JobQueue_List_new();
  cbox->todoJobs=AH_Job_List_new();
  cbox->finishedJobs=AH_Job_List_new();
  cbox->provider=pro;
  cbox->outbox=ob;

  return cbox;
}



void AH_OutboxCBox_free(AH_OUTBOX_CBOX *cbox)
{
  if (cbox) {
    assert(cbox->usage);
    if (--(cbox->usage)==0) {
      GWEN_LIST_FINI(AH_OUTBOX_CBOX, cbox);
      AH_JobQueue_List_free(cbox->todoQueues);
      AH_JobQueue_List_free(cbox->finishedQueues);
      AH_Job_List_free(cbox->todoJobs);
      AH_Job_List_free(cbox->finishedJobs);

      GWEN_FREE_OBJECT(cbox);
    }
  }
}



AH_OUTBOX *AH_OutboxCBox_GetOutbox(const AH_OUTBOX_CBOX *cbox)
{
  assert(cbox);
  return cbox->outbox;
}



AB_PROVIDER *AH_OutboxCBox_GetProvider(const AH_OUTBOX_CBOX *cbox)
{
  assert(cbox);
  return cbox->provider;
}



AB_USER *AH_OutboxCBox_GetUser(const AH_OUTBOX_CBOX *cbox)
{
  assert(cbox);
  return cbox->user;
}



int AH_OutboxCBox_GetIsLocked(const AH_OUTBOX_CBOX *cbox)
{
  assert(cbox);
  return cbox->isLocked;
}



void AH_OutboxCBox_SetIsLocked(AH_OUTBOX_CBOX *cbox, int i)
{
  assert(cbox);
  cbox->isLocked=i;
}



AH_JOB_LIST *AH_OutboxCBox_GetTodoJobs(const AH_OUTBOX_CBOX *cbox)
{
  assert(cbox);
  return cbox->todoJobs;
}



AH_JOB_LIST *AH_OutboxCBox_GetFinishedJobs(const AH_OUTBOX_CBOX *cbox)
{
  assert(cbox);
  return cbox->finishedJobs;
}



AH_JOBQUEUE_LIST *AH_OutboxCBox_GetTodoQueues(const AH_OUTBOX_CBOX *cbox)
{
  assert(cbox);
  return cbox->todoQueues;
}



AH_JOBQUEUE_LIST *AH_OutboxCBox_GetFinishedQueues(const AH_OUTBOX_CBOX *cbox)
{
  assert(cbox);
  return cbox->finishedQueues;
}



void AH_OutboxCBox_AddTodoJob(AH_OUTBOX_CBOX *cbox, AH_JOB *j)
{
  assert(cbox);
  assert(j);

  AH_Job_SetStatus(j, AH_JobStatusToDo);
  AH_Job_List_Add(j, cbox->todoJobs);
}



void AH_OutboxCBox_Finish(AH_OUTBOX_CBOX *cbox)
{
  AH_JOBQUEUE *jq;

  assert(cbox);

  DBG_INFO(AQHBCI_LOGDOMAIN, "Finishing customer box");
  while ((jq=AH_JobQueue_List_First(cbox->finishedQueues))) {
    AH_JOB_LIST *jl;
    AH_JOB *j;

    jl=AH_JobQueue_TakeJobList(jq);
    assert(jl);
    while ((j=AH_Job_List_First(jl))) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Moving job \"%s\" from finished queue to finished jobs",
               AH_Job_GetName(j));
      AH_Job_List_Del(j);
      AH_Job_List_Add(j, cbox->finishedJobs);
    } /* while */
    AH_Job_List_free(jl);
    AH_JobQueue_free(jq);
  } /* while */

  while ((jq=AH_JobQueue_List_First(cbox->todoQueues))) {
    AH_JOB_LIST *jl;
    AH_JOB *j;

    jl=AH_JobQueue_TakeJobList(jq);
    assert(jl);
    while ((j=AH_Job_List_First(jl))) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Moving job \"%s\" from todo queue to finished jobs",
               AH_Job_GetName(j));
      AH_Job_List_Del(j);
      AH_Job_List_Add(j, cbox->finishedJobs);
    } /* while */
    AH_Job_List_free(jl);
    AH_JobQueue_free(jq);
  } /* while */

  if (AH_Job_List_GetCount(cbox->todoJobs)) {
    AH_JOB *j;

    while ((j=AH_Job_List_First(cbox->todoJobs))) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Moving job \"%s\" from todo queue to finished jobs",
               AH_Job_GetName(j));
      AH_Job_List_Del(j);
      AH_Job_List_Add(j, cbox->finishedJobs);
    } /* while */
  }
}



AH_JOB_LIST *AH_OutboxCBox_TakeFinishedJobs(AH_OUTBOX_CBOX *cbox)
{
  AH_JOB_LIST *jl;

  assert(cbox);
  jl=cbox->finishedJobs;
  cbox->finishedJobs=AH_Job_List_new();
  return jl;
}



int AH_OutboxCBox_Prepare(AH_OUTBOX_CBOX *cbox)
{
  AH_JOB *j;
  unsigned int errors;
  AH_JOBQUEUE *jq;
  int firstJob;

  assert(cbox);

  errors=0;

  /* call AH_Job_Prepare() for all jobs */
  j=AH_Job_List_First(cbox->todoJobs);
  while (j) {
    AH_JOB_STATUS st;
    AH_JOB *next;

    next=AH_Job_List_Next(j);
    st=AH_Job_GetStatus(j);
    if (st==AH_JobStatusToDo) {
      int rv=AH_Job_Prepare(j);
      if (rv<0 && rv!=GWEN_ERROR_NOT_SUPPORTED) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
        AH_Job_SetStatus(j, AH_JobStatusError);
        AH_Job_List_Del(j);
        AH_Job_List_Add(j, cbox->finishedJobs);
        errors++;
      }
    } /* if status TODO */
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Skip job \"%s\" for its status \"%s\" (%d)",
               AH_Job_GetName(j), AH_Job_StatusName(st), st);
      AH_Job_SetStatus(j, AH_JobStatusError);
      AH_Job_List_Del(j);
      AH_Job_List_Add(j, cbox->finishedJobs);
      errors++;
    }

    j=next;
  } /* while */


  /* move all dialog jobs to new queues or to the list of finished jobs */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing dialog jobs");
  j=AH_Job_List_First(cbox->todoJobs);
  while (j) {
    AH_JOB_STATUS st;
    AH_JOB *next;

    next=AH_Job_List_Next(j);
    st=AH_Job_GetStatus(j);
    if (st==AH_JobStatusToDo) {
      if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_DLGJOB) {
        /* this is a dialog job, create a new queue for it */
        AH_JOBQUEUE *jq;

        DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing dialog job \"%s\"",
                 AH_Job_GetName(j));
        jq=AH_JobQueue_new(cbox->user);
        AH_Job_List_Del(j);
        if (AH_JobQueue_AddJob(jq, j)!=AH_JobQueueAddResultOk) {
          /* error adding a single job to the queue */
          DBG_ERROR(AQHBCI_LOGDOMAIN,
                    "Could not add dialog job \"%s\" to queue",
                    AH_Job_GetName(j));
          /* set status to ERROR and move to finished queue */
          AH_Job_SetStatus(j, AH_JobStatusError);
          AH_Job_List_Add(j, cbox->finishedJobs);
          AH_JobQueue_free(jq);
          errors++;
        }
        else {
          AH_Job_Log(j, GWEN_LoggerLevel_Info,
                     "Dialog job enqueued");
          /* job added. This is a dialog job */
          AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_ISDIALOG);
          AH_JobQueue_List_Add(jq, cbox->todoQueues);
        } /* if added to queue */
      } /* if dialog job */
    } /* if status TODO */
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Skip job \"%s\" for its status \"%s\" (%d)",
               AH_Job_GetName(j), AH_Job_StatusName(st), st);
      AH_Job_List_Add(j, cbox->finishedJobs);
    }

    j=next;
  } /* while */

  /* now todoJobs only contains non-dialog jobs with a correct status,
   * append them to new queues as needed */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing non-dialog jobs");
  jq=AH_JobQueue_new(cbox->user);
  firstJob=1;
  DBG_INFO(AQHBCI_LOGDOMAIN, "We have %d jobs to handle",
           AH_Job_List_GetCount(cbox->todoJobs));
  while (AH_Job_List_GetCount(cbox->todoJobs)) {
    int jobsAdded;
    int queueCreated;
    AH_JOB_LIST *retryJobs;

    DBG_INFO(AQHBCI_LOGDOMAIN, "Still some jobs left todo");
    jobsAdded=0;
    queueCreated=0;
    retryJobs=AH_Job_List_new();
    while ((j=AH_Job_List_First(cbox->todoJobs))) {
      AH_JOBQUEUE_ADDRESULT res;

      DBG_INFO(AQHBCI_LOGDOMAIN, "Queueing job \"%s\"", AH_Job_GetName(j));
      AH_Job_List_Del(j);
      res=AH_JobQueue_AddJob(jq, j);
      if (res!=AH_JobQueueAddResultOk) {
        DBG_INFO(AQHBCI_LOGDOMAIN,
                 "Could not add job \"%s\" to the current queue",
                 AH_Job_GetName(j));

        if (firstJob) {
          /* error adding a single job to the queue */
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not add single non-dialog job \"%s\" to queue",
                    AH_Job_GetName(j));
          /* set status to ERROR and move to finished queue */
          AH_Job_SetStatus(j, AH_JobStatusError);
          AH_Job_List_Add(j, cbox->finishedJobs);
          AH_Job_Log(j, GWEN_LoggerLevel_Error,
                     "Could not enqueing HBCI-job");
          errors++;
        } /* if first job failed */
        else {
          /* not the first job, check for reason of failure */
          if (res==AH_JobQueueAddResultQueueFull) {
            /* queue is full, so add it to the todo queue list and start
             * a new queue */
            DBG_INFO(AQHBCI_LOGDOMAIN, "Queue full, starting next one");
            AH_JobQueue_List_Add(jq, cbox->todoQueues);
            jq=AH_JobQueue_new(cbox->user);
            firstJob=1;
            queueCreated=1;
            /* put job back into queue (same pos, try it again in next loop)*/
            AH_Job_List_Insert(j, cbox->todoJobs);
            break;
          }
          else if (res==AH_JobQueueAddResultJobLimit) {
            DBG_INFO(AQHBCI_LOGDOMAIN,
                     "Job \"%s\" does not fit into queue, will retry later",
                     AH_Job_GetName(j));
            /* move job to the end of the queue (retry it later) */
            AH_Job_List_Add(j, retryJobs);
          }
          else {
            /* error adding a job to the queue */
            DBG_ERROR(AQHBCI_LOGDOMAIN,
                      "Could not add non-dialog job \"%s\" to queue for "
                      "unknown reason %d",
                      AH_Job_GetName(j), res);
            /* set status to ERROR and move to finished queue */
            AH_Job_SetStatus(j, AH_JobStatusError);
            AH_Job_List_Add(j, cbox->finishedJobs);
            errors++;
          }
        } /* if it wasn't the first job to fail */
      } /* if adding to the queue failed */
      else {
        /* job added successfully */
        DBG_INFO(AQHBCI_LOGDOMAIN, "Job \"%s\" successfully added",
                 AH_Job_GetName(j));
        AH_Job_Log(j, GWEN_LoggerLevel_Info,
                   "HBCI-job enqueued (1)");
        firstJob=0;
        jobsAdded++;
      }
    } /* while */

    /* put back all jobs we dismissed */
    j=AH_Job_List_First(retryJobs);
    if (j) {
      while (j) {
        AH_JOB *jnext;
        jnext=AH_Job_List_Next(j);
        DBG_NOTICE(AQHBCI_LOGDOMAIN,
                   "Moving job \"%s\" back to queue",
                   AH_Job_GetName(j));
        AH_Job_List_Del(j);
        AH_Job_List_Add(j, cbox->todoJobs);
        j=jnext;
      }

      /* there are some retry jobs, retry them */
      if (AH_JobQueue_GetCount(jq)!=0) {
        AH_JobQueue_List_Add(jq, cbox->todoQueues);
        jq=AH_JobQueue_new(cbox->user);
        firstJob=1;
        queueCreated=1;
      }
    }
    AH_Job_List_free(retryJobs);
    retryJobs=NULL;

    /* check whether we could do something in the last loop */
    if (!jobsAdded && !queueCreated) {
      AH_JOB *j;

      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could do nothing in last loop, "
                "discarding remaining jobs");
      j=AH_Job_List_First(cbox->todoJobs);
      while (j) {
        AH_Job_SetStatus(j, AH_JobStatusError);
        AH_Job_List_Del(j);
        AH_Job_List_Add(j, cbox->finishedJobs);
        errors++;
        j=AH_Job_List_Next(j);
      } /* while */

      /* break the loop */
      break;
    } /* if we couldn't do anything */
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Still something to do");
    }
  } /* while still todoJobs */

  /* check whether to free the current queue */
  if (AH_JobQueue_GetCount(jq)==0) {
    /* current queue is empty, free it */
    DBG_INFO(AQHBCI_LOGDOMAIN, "Last queue is empty, deleting it");
    AH_JobQueue_free(jq);
  }
  else {
    /* it is not, so add it to the todo list */
    DBG_INFO(AQHBCI_LOGDOMAIN, "Adding last queue");
    AH_JobQueue_List_Add(jq, cbox->todoQueues);
  }

  if (errors) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Some errors (%d) occurred", errors);
    return -1;
  }

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




void AH_OutboxCBox_HandleQueueError(AH_OUTBOX_CBOX *cbox, AH_JOBQUEUE *jq, const char *logStr)
{
  AH_JOB *j;
  AH_JOB_LIST *jl;

  jl=AH_JobQueue_TakeJobList(jq);
  assert(jl);

  while ((j=AH_Job_List_First(jl))) {
    AH_Job_List_Del(j);
    if (AH_Job_GetStatus(j)!=AH_JobStatusAnswered) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Setting status of job \"%s\" to ERROR",
               AH_Job_GetName(j));
      AH_Job_SetStatus(j, AH_JobStatusError);
      if (logStr)
        AH_Job_Log(j, GWEN_LoggerLevel_Error, logStr);
    }
    AH_Job_List_Add(j, cbox->finishedJobs);
  }
  AH_Job_List_free(jl);
  AH_JobQueue_free(jq);
}



int AH_OutboxCBox_PerformQueue(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOBQUEUE *jq)
{
  int rv;

  for (;;) {
    AH_JOBQUEUE *jqTodo;
    int jobsTodo;
    uint32_t jqFlags;
    AH_JOB *j;
    AH_JOB_LIST *jl;

    jobsTodo=0;
    jl=AH_JobQueue_TakeJobList(jq);
    assert(jl);
    jqTodo=AH_JobQueue_new(AH_JobQueue_GetUser(jq));
    /* copy some flags */
    jqFlags=AH_JobQueue_GetFlags(jq);
    jqFlags&=~(AH_JOBQUEUE_FLAGS_CRYPT |
               AH_JOBQUEUE_FLAGS_SIGN |
               AH_JOBQUEUE_FLAGS_NOSYSID |
               AH_JOBQUEUE_FLAGS_NOITAN);
    AH_JobQueue_SetFlags(jqTodo, (jqFlags&AH_JOBQUEUE_FLAGS_COPYMASK));

    /* copy todo jobs */
    while ((j=AH_Job_List_First(jl))) {
      AH_Job_List_Del(j);
      if (AH_Job_GetStatus(j)==AH_JobStatusAnswered) {
        DBG_DEBUG(AQHBCI_LOGDOMAIN, "Message finished");
        /* prepare job for next message
         * (if attachpoint or multi-message job)
         */
        AH_Job_PrepareNextMessage(j);
        if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_HASMOREMSGS) {
          DBG_NOTICE(AQHBCI_LOGDOMAIN, "Requeueing job");
          /* we shall redo this job */
          if (AH_JobQueue_AddJob(jqTodo, j)!=
              AH_JobQueueAddResultOk) {
            DBG_ERROR(AQHBCI_LOGDOMAIN,
                      "That's weird, I could not add the job to redo queue");
            AH_Job_Log(j, GWEN_LoggerLevel_Error,
                       "Could not re-enqueue HBCI-job");
            AH_Job_SetStatus(j, AH_JobStatusError);
          }
          else {
            jobsTodo++;
            AH_Job_Log(j, GWEN_LoggerLevel_Info,
                       "HBCI-job re-enqueued (multi-message job)");
            j=0; /* mark that this job has been dealt with */
          }
        } /* if more messages */
        else {
          DBG_NOTICE(AQHBCI_LOGDOMAIN, "Not requeing job");
        }
      } /* if status matches */
      else if (AH_Job_GetStatus(j)==AH_JobStatusEnqueued) {
        if (AH_JobQueue_AddJob(jqTodo, j)!=
            AH_JobQueueAddResultOk) {
          DBG_ERROR(AQHBCI_LOGDOMAIN,
                    "That's weird, I could not add the job to redo queue");
          AH_Job_SetStatus(j, AH_JobStatusError);
          AH_Job_Log(j, GWEN_LoggerLevel_Error,
                     "Could not enqueue HBCI-job");
        }
        else {
          jobsTodo++;
          AH_Job_Log(j, GWEN_LoggerLevel_Info,
                     "HBCI-job enqueued (2)");
          j=0; /* mark that this job has been dealt with */
        }
      }
      else {
        DBG_DEBUG(AQHBCI_LOGDOMAIN, "Bad status \"%s\" (%d)",
                  AH_Job_StatusName(AH_Job_GetStatus(j)),
                  AH_Job_GetStatus(j));
        if (GWEN_Logger_GetLevel(0)>=GWEN_LoggerLevel_Debug)
          AH_Job_Dump(j, stderr, 4);
      }
      if (j) {
        /* move job to finished list if we still have the job */
        AH_Job_List_Add(j, cbox->finishedJobs);
      }
    } /* while */

    AH_Job_List_free(jl);
    AH_JobQueue_free(jq);
    jq=jqTodo;

    if (!jobsTodo)
      break;

    /* jq now contains all jobs to be executed */
    rv=AH_OutboxCBox_SendAndRecvQueue(cbox, dlg, jq);
    if (rv) {
      AH_OutboxCBox_HandleQueueError(cbox, jq,
                                       "Error performing queue");
      return rv;
    } /* if error */
  } /* for */

  AH_JobQueue_free(jq);
  return 0;
}



int AH_OutboxCBox_PerformNonDialogQueues(AH_OUTBOX_CBOX *cbox, AH_JOBQUEUE_LIST *jql)
{
  AH_DIALOG *dlg;
  AH_JOBQUEUE *jq;
  int rv=0;
  int i;
  uint32_t jqflags;

  if (AH_JobQueue_List_GetCount(jql)==0) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "No queues to handle, doing nothing");
    AH_JobQueue_List_free(jql);
    return 0;
  }

  for (i=0; i<2; i++) {
    dlg=AH_Dialog_new(cbox->user, cbox->provider);
    rv=AH_Dialog_Connect(dlg);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Could not begin a dialog for customer \"%s\" (%d)",
               AB_User_GetCustomerId(cbox->user), rv);
      /* finish all queues */
      AH_OutboxCBox_HandleQueueListError(cbox, jql,
                                           "Could not begin dialog");
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
      AH_OutboxCBox_HandleQueueListError(cbox, jql,
                                           "Could not open dialog");
      AH_Dialog_free(dlg);
      return rv;
    }
    else if (rv==1) {
      AH_Dialog_Disconnect(dlg);
      AH_Dialog_free(dlg);
      GWEN_Gui_ProgressLog(0,
                           GWEN_LoggerLevel_Info,
                           I18N("Retrying to open dialog"));
    }
  }
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not open dialog");
    AH_Dialog_Disconnect(dlg);
    /* finish all queues */
    AH_OutboxCBox_HandleQueueListError(cbox, jql,
                                         "Could not open dialog");
    AH_Dialog_free(dlg);
    return rv;
  }

  /* handle queues */
  rv=0;
  while ((jq=AH_JobQueue_List_First(jql))) {
    AH_JobQueue_List_Del(jq);
    rv=AH_OutboxCBox_PerformQueue(cbox, dlg, jq);
    if (rv)
      break;
  } /* while */

  if (rv) {
    /* finish all remaining queues */
    AH_OutboxCBox_HandleQueueListError(cbox, jql,
                                         "Could not send ");
    AH_Dialog_Disconnect(dlg);
    AH_Dialog_free(dlg);
    return rv;
  }

  /* close dialog */
  rv=AH_OutboxCBox_CloseDialog(cbox, dlg, jqflags);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Could not close dialog, ignoring");
    /*AH_HBCI_EndDialog(cbox->hbci, dlg);
     return rv;*/
  }

  DBG_INFO(AQHBCI_LOGDOMAIN, "Closing connection");
  AH_Dialog_Disconnect(dlg);
  AH_Dialog_free(dlg);

  AH_JobQueue_List_free(jql);
  return 0;
}



int AH_OutboxCBox_PerformDialogQueue(AH_OUTBOX_CBOX *cbox, AH_JOBQUEUE *jq)
{
  AH_DIALOG *dlg;
  int rv;
  uint32_t jqFlags;

  jqFlags=AH_JobQueue_GetFlags(jq);

  /* open connection */
  dlg=AH_Dialog_new(cbox->user, cbox->provider);
  rv=AH_Dialog_Connect(dlg);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Could not begin a dialog for customer \"%s\" (%d)",
             AB_User_GetCustomerId(cbox->user), rv);
    /* finish all queues */
    AH_OutboxCBox_HandleQueueError(cbox, jq, "Could not begin dialog");
    AH_Dialog_free(dlg);
    return rv;
  }

#ifdef EXTREME_DEBUGGING
  DBG_ERROR(AQHBCI_LOGDOMAIN, "Handling this job queue:");
  AH_JobQueue_Dump(jq, stderr, 2);
#endif

  if (AH_User_GetCryptMode(cbox->user)==AH_CryptMode_Pintan) {
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

      selectedTanVersion=AH_User_GetSelectedTanMethod(cbox->user)/1000;
      if (selectedTanVersion>=6) {
        AH_JOB *jTan;

        DBG_INFO(AQHBCI_LOGDOMAIN, "User-selected TAN job version is 6 or newer (%d)", selectedTanVersion);

        /* check for PSD2: HKTAN version 6 available? if so -> use that */
        jTan=AH_Job_Tan_new(cbox->provider, cbox->user, 4, 6);
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
  rv=AH_OutboxCBox_PerformQueue(cbox, dlg, jq);
  if (rv) {
    AH_Dialog_Disconnect(dlg);
    AH_Dialog_free(dlg);
    return rv;
  }

  /* close dialog */
#if 0
  if (AH_User_GetCryptMode(cbox->user)==AH_CryptMode_Pintan &&
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



void AH_OutboxCBox_ExtractMatchingQueues(AH_JOBQUEUE_LIST *jql,
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



void AH_OutboxCBox_HandleQueueListError(AH_OUTBOX_CBOX *cbox, AH_JOBQUEUE_LIST *jql, const char *logStr)
{
  AH_JOBQUEUE *jq;

  while ((jq=AH_JobQueue_List_First(jql))) {
    AH_JobQueue_List_Del(jq);
    AH_OutboxCBox_HandleQueueError(cbox, jq, logStr);
  } /* while */
  AH_JobQueue_List_free(jql);
}



int AH_OutboxCBox_SendAndRecvDialogQueues(AH_OUTBOX_CBOX *cbox)
{
  AH_JOBQUEUE_LIST *jqlWanted;
  AH_JOBQUEUE_LIST *jqlRest;
  int rv;

  jqlWanted=AH_JobQueue_List_new();
  jqlRest=AH_JobQueue_List_new();
  AH_OutboxCBox_ExtractMatchingQueues(cbox->todoQueues,
                                      jqlWanted,
                                      jqlRest,
                                      AH_JOBQUEUE_FLAGS_ISDIALOG,
                                      AH_JOBQUEUE_FLAGS_ISDIALOG);
  AH_JobQueue_List_free(cbox->todoQueues);
  cbox->todoQueues=jqlRest;
  if (AH_JobQueue_List_GetCount(jqlWanted)) {
    AH_JOBQUEUE *jq;

    /* there are matching queues, handle them */
    while ((jq=AH_JobQueue_List_First(jqlWanted))) {
      AH_JobQueue_List_Del(jq);
      rv=AH_OutboxCBox_PerformDialogQueue(cbox, jq);
      if (rv) {
        DBG_INFO(AQHBCI_LOGDOMAIN,
                 "Error performing queue (%d)", rv);
        AH_OutboxCBox_HandleQueueListError(cbox, jqlWanted,
                                             "Could not perform "
                                             "dialog queue");
        AH_OutboxCBox_HandleQueueListError(cbox, cbox->todoQueues,
                                             "Could not perform "
                                             "dialog queue");
        cbox->todoQueues=AH_JobQueue_List_new();
        return rv;
      }
    } /* while */
  }
  AH_JobQueue_List_free(jqlWanted);
  return 0;
}



int AH_OutboxCBox_SendAndRecvSelected(AH_OUTBOX_CBOX *cbox, uint32_t jqflags, uint32_t jqmask)
{
  AH_JOBQUEUE_LIST *jqlWanted;
  AH_JOBQUEUE_LIST *jqlRest;
  int rv;

  jqlWanted=AH_JobQueue_List_new();
  jqlRest=AH_JobQueue_List_new();
  AH_OutboxCBox_ExtractMatchingQueues(cbox->todoQueues,
                                      jqlWanted,
                                      jqlRest, jqflags, jqmask);
  AH_JobQueue_List_free(cbox->todoQueues);
  cbox->todoQueues=jqlRest;
  if (AH_JobQueue_List_GetCount(jqlWanted)) {
    /* there are matching queues, handle them */
    rv=AH_OutboxCBox_PerformNonDialogQueues(cbox, jqlWanted);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
                "Error performing queue (%d)", rv);
      AH_OutboxCBox_HandleQueueListError(cbox, cbox->todoQueues,
                                           "Error performing "
                                           "selected jobs");
      cbox->todoQueues=AH_JobQueue_List_new();
      return rv;
    }
  } /* if matching queuees */
  else
    AH_JobQueue_List_free(jqlWanted);
  return 0;
}



int AH_OutboxCBox_SendAndRecvBox(AH_OUTBOX_CBOX *cbox)
{
  int rv;

  /* dialog queues */
  rv=AH_OutboxCBox_SendAndRecvDialogQueues(cbox);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error performing dialog queues (%d)", rv);
    return rv;
  }

  /* non-dialog queues: unsigned, uncrypted */
  rv=AH_OutboxCBox_SendAndRecvSelected(cbox,
                                         0,
                                         AH_JOBQUEUE_FLAGS_ISDIALOG |
                                         AH_JOBQUEUE_FLAGS_SIGN |
                                         AH_JOBQUEUE_FLAGS_CRYPT);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error performing queues (-S, -C: %d)", rv);
    return rv;
  }

  /* non-dialog queues: unsigned, crypted */
  rv=AH_OutboxCBox_SendAndRecvSelected(cbox,
                                         AH_JOBQUEUE_FLAGS_CRYPT,
                                         AH_JOBQUEUE_FLAGS_ISDIALOG |
                                         AH_JOBQUEUE_FLAGS_SIGN |
                                         AH_JOBQUEUE_FLAGS_CRYPT);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error performing queues (-S, +C: %d)", rv);
    return rv;
  }

  /* non-dialog queues: signed, uncrypted */
  rv=AH_OutboxCBox_SendAndRecvSelected(cbox,
                                         AH_JOBQUEUE_FLAGS_SIGN,
                                         AH_JOBQUEUE_FLAGS_ISDIALOG |
                                         AH_JOBQUEUE_FLAGS_SIGN |
                                         AH_JOBQUEUE_FLAGS_CRYPT);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error performing queues (+S, -C: %d)", rv);
    return rv;
  }

  /* non-dialog queues: signed, crypted */
  rv=AH_OutboxCBox_SendAndRecvSelected(cbox,
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

