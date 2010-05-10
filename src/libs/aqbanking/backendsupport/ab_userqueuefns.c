/***************************************************************************
    begin       : Mon May 10 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/




AB_ACCOUNTQUEUE *AB_UserQueue_FindAccountQueue(const AB_USERQUEUE *uq, AB_ACCOUNT *acc) {
  AB_ACCOUNTQUEUE *aq;

  aq=AB_AccountQueue_List_First(uq->accountQueueList);
  while(aq) {
    if (AB_AccountQueue_GetAccount(aq)==acc)
      return aq;
    aq=AB_AccountQueue_List_Next(aq);
  }

  return NULL;
}



void AB_UserQueue_AddJob(AB_USERQUEUE *uq, AB_JOB *j) {
  AB_ACCOUNT *a;
  AB_ACCOUNTQUEUE *aq;

  a=AB_Job_GetAccount(j);
  assert(a);

  aq=AB_UserQueue_FindAccountQueue(uq, a);
  if (aq==NULL) {
    aq=AB_AccountQueue_new();
    AB_AccountQueue_SetAccount(aq, a);
    AB_AccountQueue_List_Add(aq, uq->accountQueueList);
  }

  AB_AccountQueue_AddJob(aq, j);
}










