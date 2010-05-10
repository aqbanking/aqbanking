/***************************************************************************
    begin       : Mon May 10 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



AB_JOBQUEUE *AB_AccountQueue_FindJobQueue(const AB_ACCOUNTQUEUE *aq, AB_JOB_TYPE jt) {
  AB_JOBQUEUE *jq;

  jq=AB_JobQueue_List_First(aq->jobQueueList);
  while(jq) {
    if (AB_JobQueue_GetJobType(jq)==jt)
      return jq;
    jq=AB_JobQueue_List_Next(jq);
  }

  return NULL;
}



void AB_AccountQueue_AddJob(AB_ACCOUNTQUEUE *aq, AB_JOB *j) {
  AB_JOBQUEUE *jq;

  jq=AB_AccountQueue_FindJobQueue(aq, AB_Job_GetType(j));
  if (jq==NULL) {
    jq=AB_JobQueue_new();
    AB_JobQueue_List_Add(jq, aq->jobQueueList);
  }

  AB_Job_List2_PushBack(AB_JobQueue_GetJobList(jq), j);
}










