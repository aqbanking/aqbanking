/***************************************************************************
    begin       : Mon May 10 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/




AB_USERQUEUE *AB_Queue_FindUserQueue(const AB_QUEUE *q, AB_USER *u) {
  AB_USERQUEUE *uq;

  uq=AB_UserQueue_List_First(q->userQueueList);
  while(uq) {
    if (AB_UserQueue_GetUser(uq)==u)
      return uq;
    uq=AB_UserQueue_List_Next(uq);
  }

  return NULL;
}



void AB_Queue_AddJob(AB_QUEUE *q, AB_USER *u, AB_JOB *j) {
  AB_USERQUEUE *uq;

  uq=AB_Queue_FindUserQueue(q, u);
  if (uq==NULL) {
    uq=AB_UserQueue_new();
    AB_UserQueue_SetUser(uq, u);
    AB_UserQueue_List_Add(uq, q->userQueueList);
  }

  AB_UserQueue_AddJob(uq, j);
}



AB_JOB *AB_Queue_FindFirstJobLikeThis(AB_QUEUE *q, AB_USER *u, AB_JOB *bj) {
  AB_USERQUEUE *uq=AB_Queue_FindUserQueue(q, u);
  if (uq) {
    AB_ACCOUNTQUEUE *aq=AB_UserQueue_FindAccountQueue(uq, AB_Job_GetAccount(bj));
    if (aq) {
      AB_JOBQUEUE *jq=AB_AccountQueue_FindJobQueue(aq, AB_Job_GetType(bj));
      if (jq)
	return AB_Job_List2_GetFront(AB_JobQueue_GetJobList(jq));
    }
  }

  return NULL;
}










