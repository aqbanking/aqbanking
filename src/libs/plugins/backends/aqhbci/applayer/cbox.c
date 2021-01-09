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



void AH_OutboxCBox_SetTodoQueues(AH_OUTBOX_CBOX *cbox, AH_JOBQUEUE_LIST *nl)
{
  assert(cbox);
  if (cbox->todoQueues)
    AH_JobQueue_List_free(cbox->todoQueues);
  cbox->todoQueues=nl;
}



void AH_OutboxCBox_AddTodoJob(AH_OUTBOX_CBOX *cbox, AH_JOB *j)
{
  assert(cbox);
  assert(j);

  AH_Job_SetStatus(j, AH_JobStatusToDo);
  AH_Job_List_Add(j, cbox->todoJobs);
}



AH_JOB_LIST *AH_OutboxCBox_TakeFinishedJobs(AH_OUTBOX_CBOX *cbox)
{
  AH_JOB_LIST *jl;

  assert(cbox);
  jl=cbox->finishedJobs;
  cbox->finishedJobs=AH_Job_List_new();
  return jl;
}



