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


#include "cbox_prepare.h"




int AH_OutboxCBox_Prepare(AH_OUTBOX_CBOX *cbox)
{
  AH_JOB_LIST *todoJobs;
  AH_JOB_LIST *finishedJobs;
  AH_JOBQUEUE_LIST *todoQueues;
  AB_USER *user;
  AH_JOB *j;
  unsigned int errors;
  AH_JOBQUEUE *jq;
  int firstJob;

  assert(cbox);

  errors=0;

  todoJobs=AH_OutboxCBox_GetTodoJobs(cbox);
  finishedJobs=AH_OutboxCBox_GetFinishedJobs(cbox);
  todoQueues=AH_OutboxCBox_GetTodoQueues(cbox);
  user=AH_OutboxCBox_GetUser(cbox);

  /* call AH_Job_Prepare() for all jobs */
  j=AH_Job_List_First(todoJobs);
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
        AH_Job_List_Add(j, finishedJobs);
        errors++;
      }
    } /* if status TODO */
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Skip job \"%s\" for its status \"%s\" (%d)",
               AH_Job_GetName(j), AH_Job_StatusName(st), st);
      AH_Job_SetStatus(j, AH_JobStatusError);
      AH_Job_List_Del(j);
      AH_Job_List_Add(j, finishedJobs);
      errors++;
    }

    j=next;
  } /* while */


  /* move all dialog jobs to new queues or to the list of finished jobs */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing dialog jobs");
  j=AH_Job_List_First(todoJobs);
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
        jq=AH_JobQueue_new(user);
        AH_Job_List_Del(j);
        if (AH_JobQueue_AddJob(jq, j)!=AH_JobQueueAddResultOk) {
          /* error adding a single job to the queue */
          DBG_ERROR(AQHBCI_LOGDOMAIN,
                    "Could not add dialog job \"%s\" to queue",
                    AH_Job_GetName(j));
          /* set status to ERROR and move to finished queue */
          AH_Job_SetStatus(j, AH_JobStatusError);
          AH_Job_List_Add(j, finishedJobs);
          AH_JobQueue_free(jq);
          errors++;
        }
        else {
          AH_Job_Log(j, GWEN_LoggerLevel_Info,
                     "Dialog job enqueued");
          /* job added. This is a dialog job */
          AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_ISDIALOG);
          AH_JobQueue_List_Add(jq, todoQueues);
        } /* if added to queue */
      } /* if dialog job */
    } /* if status TODO */
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Skip job \"%s\" for its status \"%s\" (%d)",
               AH_Job_GetName(j), AH_Job_StatusName(st), st);
      AH_Job_List_Add(j, finishedJobs);
    }

    j=next;
  } /* while */

  /* now todoJobs only contains non-dialog jobs with a correct status,
   * append them to new queues as needed */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing non-dialog jobs");
  jq=AH_JobQueue_new(user);
  firstJob=1;
  DBG_INFO(AQHBCI_LOGDOMAIN, "We have %d jobs to handle",
           AH_Job_List_GetCount(todoJobs));
  while (AH_Job_List_GetCount(todoJobs)) {
    int jobsAdded;
    int queueCreated;
    AH_JOB_LIST *retryJobs;

    DBG_INFO(AQHBCI_LOGDOMAIN, "Still some jobs left todo");
    jobsAdded=0;
    queueCreated=0;
    retryJobs=AH_Job_List_new();
    while ((j=AH_Job_List_First(todoJobs))) {
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
          AH_Job_List_Add(j, finishedJobs);
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
            AH_JobQueue_List_Add(jq, todoQueues);
            jq=AH_JobQueue_new(user);
            firstJob=1;
            queueCreated=1;
            /* put job back into queue (same pos, try it again in next loop)*/
            AH_Job_List_Insert(j, todoJobs);
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
            AH_Job_List_Add(j, finishedJobs);
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
        AH_Job_List_Add(j, todoJobs);
        j=jnext;
      }

      /* there are some retry jobs, retry them */
      if (AH_JobQueue_GetCount(jq)!=0) {
        AH_JobQueue_List_Add(jq, todoQueues);
        jq=AH_JobQueue_new(user);
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
      j=AH_Job_List_First(todoJobs);
      while (j) {
        AH_Job_SetStatus(j, AH_JobStatusError);
        AH_Job_List_Del(j);
        AH_Job_List_Add(j, finishedJobs);
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
    AH_JobQueue_List_Add(jq, todoQueues);
  }

  if (errors) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Some errors (%d) occurred", errors);
    return -1;
  }

  return 0;
}

