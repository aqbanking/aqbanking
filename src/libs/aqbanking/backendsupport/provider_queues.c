/***************************************************************************
 begin       : Mon Nov 26 2018
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


/*
 * This file is included by provider.c
 */



int AB_Provider_SortProviderQueueIntoUserQueueList(AB_PROVIDER *pro, AB_PROVIDERQUEUE *pq, AB_USERQUEUE_LIST *uql) {
  AB_ACCOUNTQUEUE_LIST *aql;
  AB_ACCOUNTQUEUE *aq;

  assert(pro);

  aql=AB_ProviderQueue_GetAccountQueueList(pq);
  if (aql==NULL) {
    return GWEN_ERROR_NO_DATA;
  }

  while( (aq=AB_AccountQueue_List_First(aql)) ) {
    uint32_t aid;
    uint32_t uid;
    AB_ACCOUNT *a=NULL;
    AB_USERQUEUE *uq=NULL;
    int rv;

    aid=AB_AccountQueue_GetAccountId(aq);
    rv=AB_Provider_GetAccount(pro, aid, 1, 1, &a);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    AB_AccountQueue_SetAccount(aq, a);

    /* determine user */
    uid=AB_Account_GetUserId(a);
    if (uid==0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "No first user in account %lu, SNH!", (unsigned long int) aid);
      return GWEN_ERROR_INTERNAL;
    }
    else {
      uq=AB_UserQueue_List_GetByUserId(uql, uid);
      if (uq==NULL) {
        AB_USER *u=NULL;

        rv=AB_Provider_GetUser(pro, uid, 1, 1, &u);
        if (rv<0) {
          DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
          return rv;
        }
        uq=AB_UserQueue_new();
        AB_UserQueue_SetUser(uq, u);
	AB_UserQueue_SetUserId(uq, uid);

        AB_UserQueue_List_Add(uq, uql);
      }
    }

    AB_AccountQueue_List_Del(aq);
    AB_UserQueue_AddAccountQueue(uq, aq);
  }

  return 0;
}



void AB_Provider_FreeUsersAndAccountsFromUserQueueList(AB_PROVIDER *pro, AB_USERQUEUE_LIST *uql) {
  AB_USERQUEUE *uq;

  assert(pro);

  uq=AB_UserQueue_List_First(uql);
  while(uq) {
    AB_ACCOUNTQUEUE_LIST *aql;
    AB_USER *u;

    u=AB_UserQueue_GetUser(uq);
    aql=AB_UserQueue_GetAccountQueueList(uq);
    if (aql) {
      AB_ACCOUNTQUEUE *aq;

      aq=AB_AccountQueue_List_First(aql);
      while(aq) {
        AB_ACCOUNT *a;

        a=AB_AccountQueue_GetAccount(aq);
        AB_AccountQueue_SetAccount(aq, NULL);
        AB_Account_free(a);
        aq=AB_AccountQueue_List_Next(aq);
      }

    }

    AB_UserQueue_SetUserId(uq, 0);
    AB_UserQueue_SetUser(uq, NULL);
    AB_User_free(u);

    uq=AB_UserQueue_List_Next(uq);
  }
}




