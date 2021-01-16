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


/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int _prepareTodoJobs(AH_JOB_LIST *todoJobs, AH_JOB_LIST *finishedJobs);
static int _prepareDialogJobs(AB_USER *user, AH_JOB_LIST *todoJobs, AH_JOB_LIST *finishedJobs,
                              AH_JOBQUEUE_LIST *todoQueues);
static int _sortTodoJobsIntoQueues(AB_USER *user, AH_JOB_LIST *todoJobs, AH_JOB_LIST *finishedJobs,
                                   AH_JOBQUEUE_LIST *todoQueues);
static void _fillQueueWithTodoJobs(AH_JOB_LIST *todoJobs, AH_JOB_LIST *finishedJobs, AH_JOB_LIST *retryJobs,
                                   AH_JOBQUEUE *jq);
static void _moveJobsAndSetErrorStatus(AH_JOB_LIST *todoJobs, AH_JOB_LIST *finishedJobs);


/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AH_OutboxCBox_Prepare(AH_OUTBOX_CBOX *cbox)
{
  AH_JOB_LIST *todoJobs;
  AH_JOB_LIST *finishedJobs;
  AH_JOBQUEUE_LIST *todoQueues;
  AB_USER *user;
  unsigned int errors=0;

  assert(cbox);

  todoJobs=AH_OutboxCBox_GetTodoJobs(cbox);
  finishedJobs=AH_OutboxCBox_GetFinishedJobs(cbox);
  todoQueues=AH_OutboxCBox_GetTodoQueues(cbox);
  user=AH_OutboxCBox_GetUser(cbox);

  if (_prepareTodoJobs(todoJobs, finishedJobs)<0)
    errors++;

  if (_prepareDialogJobs(user, todoJobs, finishedJobs, todoQueues)<0)
    errors++;

  /* try sorting remaining jobs (all non-dialog) */
  if (_sortTodoJobsIntoQueues(user, todoJobs, finishedJobs, todoQueues)<0)
    errors++;

  /* move remaining todo jobs to finishedJobs with error status */
  _moveJobsAndSetErrorStatus(todoJobs, finishedJobs);

  if (AH_JobQueue_List_GetCount(todoQueues)==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No executable job queues produced");
    return GWEN_ERROR_GENERIC;
  }

  if (errors) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Some errors (%d) occurred", errors);
  }

  return 0;
}



int _prepareTodoJobs(AH_JOB_LIST *todoJobs, AH_JOB_LIST *finishedJobs)
{
  AH_JOB *j;
  unsigned int errors=0;

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

  return (errors==0)?0:GWEN_ERROR_GENERIC;
}



int _prepareDialogJobs(AB_USER *user, AH_JOB_LIST *todoJobs, AH_JOB_LIST *finishedJobs, AH_JOBQUEUE_LIST *todoQueues)
{
  AH_JOB *j;
  unsigned int errors=0;

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

        DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing dialog job \"%s\"", AH_Job_GetName(j));
        jq=AH_JobQueue_new(user);
        AH_Job_List_Del(j);
        if (AH_JobQueue_AddJob(jq, j)!=AH_JobQueueAddResultOk) {
          /* error adding a single job to the queue */
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not add dialog job \"%s\" to queue", AH_Job_GetName(j));
          /* set status to ERROR and move to finished queue */
          AH_Job_SetStatus(j, AH_JobStatusError);
          AH_Job_List_Add(j, finishedJobs);
          AH_JobQueue_free(jq);
          errors++;
        }
        else {
          AH_Job_Log(j, GWEN_LoggerLevel_Info, "Dialog job enqueued");
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

  return (errors==0)?0:GWEN_ERROR_GENERIC;
}



int _sortTodoJobsIntoQueues(AB_USER *user, AH_JOB_LIST *todoJobs, AH_JOB_LIST *finishedJobs,
                            AH_JOBQUEUE_LIST *todoQueues)
{
  DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing non-dialog jobs");
  while (AH_Job_List_GetCount(todoJobs)) {
    AH_JOBQUEUE *jq;
    AH_JOB_LIST *retryJobs;

    retryJobs=AH_Job_List_new();
    jq=AH_JobQueue_new(user);
    _fillQueueWithTodoJobs(todoJobs, finishedJobs, retryJobs, jq);
    AH_Job_List_AddList(todoJobs, retryJobs);
    AH_Job_List_free(retryJobs);

    if (AH_JobQueue_GetCount(jq)==0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Empty queue, so all jobs failed, should not happen.");
      AH_JobQueue_free(jq);
      return GWEN_ERROR_GENERIC;
    }

    AH_JobQueue_List_Add(jq, todoQueues);
  }

  return 0;
}


/**
 * Add jobs from todoJobs to given queue until queue is full.
 * If a job can not be added to the given queue it will be added to the retryJobs list.
 * If an error occurrs when trying to add a job that job will je set to error status and added to the finishedJobs list.
 */
void _fillQueueWithTodoJobs(AH_JOB_LIST *todoJobs, AH_JOB_LIST *finishedJobs, AH_JOB_LIST *retryJobs, AH_JOBQUEUE *jq)
{
  AH_JOB *j;

  while ((j=AH_Job_List_First(todoJobs))) {
    AH_JOBQUEUE_ADDRESULT res;

    DBG_INFO(AQHBCI_LOGDOMAIN, "Queueing job \"%s\"", AH_Job_GetName(j));
    AH_Job_List_Del(j);

    res=AH_JobQueue_AddJob(jq, j);
    if (res!=AH_JobQueueAddResultOk) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Could not add job \"%s\" to the current queue", AH_Job_GetName(j));
      if (AH_JobQueue_GetCount(jq)==0) {
        /* error adding a job to an empty queue */
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not add non-dialog job \"%s\" to empty queue", AH_Job_GetName(j));
        /* set status to ERROR and move to finished queue */
        AH_Job_SetStatus(j, AH_JobStatusError);
        AH_Job_List_Add(j, finishedJobs);
        AH_Job_Log(j, GWEN_LoggerLevel_Error, "Could not enqueue HBCI-job");
      } /* if first job failed */
      else {
        /* not the first job, check for reason of failure */
        if (res==AH_JobQueueAddResultQueueFull) {
          /* queue is full, so add it to the todo queue list and start
           * a new queue */
          DBG_INFO(AQHBCI_LOGDOMAIN, "Queue full");
          AH_Job_List_Add(j, retryJobs);
          break;
        }
        else if (res==AH_JobQueueAddResultJobLimit) {
          DBG_INFO(AQHBCI_LOGDOMAIN, "Job \"%s\" does not fit into queue, will retry later", AH_Job_GetName(j));
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
        }
      } /* if it wasn't the first job to fail */
    } /* if adding to the queue failed */
    else {
      /* job added successfully */
      DBG_INFO(AQHBCI_LOGDOMAIN, "Job \"%s\" successfully added", AH_Job_GetName(j));
      AH_Job_Log(j, GWEN_LoggerLevel_Info, "HBCI-job enqueued (1)");
    }
  }
}



void _moveJobsAndSetErrorStatus(AH_JOB_LIST *todoJobs, AH_JOB_LIST *finishedJobs)
{
  AH_JOB *j;

  j=AH_Job_List_First(todoJobs);
  while (j) {
    AH_JOB *next;

    next=AH_Job_List_Next(j);
    AH_Job_SetStatus(j, AH_JobStatusError);
    AH_Job_List_Del(j);
    AH_Job_List_Add(j, finishedJobs);
    j=next;
  }
}



