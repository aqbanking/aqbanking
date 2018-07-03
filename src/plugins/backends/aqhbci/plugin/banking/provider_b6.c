/***************************************************************************
    begin       : Tue Jun 03 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


/*
 * This file is included by provider.c
 */





int AH_Provider_ReadAccount(AB_PROVIDER *pro, uint32_t uid, int doLock, int doUnlock, AB_ACCOUNT *account) {
  int rv;
  GWEN_DB_NODE *db=NULL;

  rv=AB_Banking6_Read_AccountConfig(AB_Provider_GetBanking(pro), uid, doLock, doUnlock, &db);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=AH_Account_ReadDb(account, db);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AH_Provider_WriteAccount(AB_PROVIDER *pro, uint32_t uid, int doLock, int doUnlock, const AB_ACCOUNT *account) {
  int rv;
  GWEN_DB_NODE *db;

  db=GWEN_DB_Group_new("account");
  rv=AH_Account_toDb(account, db);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=AB_Banking6_Write_AccountConfig(AB_Provider_GetBanking(pro), uid, doLock, doUnlock, db);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(db);
    return rv;
  }
  GWEN_DB_Group_free(db);

  return 0;
}



int AH_Provider_ReadUser(AB_PROVIDER *pro, uint32_t uid, int doLock, int doUnlock, AB_USER *user) {
  int rv;
  GWEN_DB_NODE *db=NULL;

  rv=AB_Banking6_Read_UserConfig(AB_Provider_GetBanking(pro), uid, doLock, doUnlock, &db);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=AH_User_ReadDb(user, db);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AH_Provider_WriteUser(AB_PROVIDER *pro, uint32_t uid, int doLock, int doUnlock, const AB_USER *user) {
  int rv;
  GWEN_DB_NODE *db;

  db=GWEN_DB_Group_new("user");
  rv=AH_User_toDb(user, db);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=AB_Banking6_Write_UserConfig(AB_Provider_GetBanking(pro), uid, doLock, doUnlock, db);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(db);
    return rv;
  }
  GWEN_DB_Group_free(db);

  return 0;
}



int AH_Provider__SortProviderQueueIntoUserQueueList(AB_PROVIDER *pro, AB_PROVIDERQUEUE *pq, AB_USERQUEUE_LIST *uql) {
  AH_PROVIDER *hp;
  AB_ACCOUNTQUEUE_LIST *aql;
  AB_ACCOUNTQUEUE *aq;
  int rv;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  aql=AB_ProviderQueue_GetAccountQueueList(pq);
  if (aql==NULL) {
    return GWEN_ERROR_NO_DATA;
  }

  while( (aq=AB_AccountQueue_List_First(aql)) ) {
    uint32_t uid;
    AB_ACCOUNT *a;
    const char *s;
    AB_USERQUEUE *uq=NULL;

    uid=AB_AccountQueue_GetAccountId(aq);
    a=AH_Account_new(AB_Provider_GetBanking(pro), pro);
    rv=AH_Provider_ReadAccount(pro, uid, 1, 1, a);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    AB_AccountQueue_SetAccount(aq, a);

    /* determine user */
    s=AB_Account_GetFirstUserId(a);
    if (!(s && *s)) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "No first user in account %lu, SNH!", (unsigned long int) uid);
      return GWEN_ERROR_INTERNAL;
    }
    else {
      unsigned long int id;

      if (1!=sscanf(s, "%lu", &id)) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Invalid id of first account user (%s), SNH!", s);
        AB_UserQueue_List_free(uql);
        return GWEN_ERROR_INTERNAL;
      }

      uq=AB_UserQueue_List_GetByUserId(uql, id);
      if (uq==NULL) {
        AB_USER *u;

        u=AH_User_new(AB_Provider_GetBanking(pro), pro);

        rv=AH_Provider_ReadUser(pro, id, 1, 1, u);
        if (rv<0) {
          DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
          return rv;
        }
        AB_UserQueue_SetUser(uq, u);

        AB_UserQueue_List_Add(uq, uql);
      }
    }

    AB_AccountQueue_List_Del(aq);
    AB_UserQueue_AddAccountQueue(uq, aq);
  }

  return 0;
}



void AH_Provider__FreeUsersAndAccountsFromUserQueueList(AB_PROVIDER *pro, AB_USERQUEUE_LIST *uql) {
  AH_PROVIDER *hp;
  AB_USERQUEUE *uq;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  uq=AB_UserQueue_List_First(uql);
  while(uql) {
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

    AB_UserQueue_SetUser(uq, NULL);
    AB_User_free(u);

    uq=AB_UserQueue_List_Next(uq);
  }
}



int AH_Provider__AddCommandToOutbox(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *t, AH_OUTBOX *outbox) {
  int rv;
  int cmd;
  AH_JOB *mj=NULL;
  int sigs;
  int jobIsNew=1;

  cmd=AB_Transaction_GetCommand(t);

  /* try to get an existing multi job to add the new one to */
  rv=AH_Provider__GetMultiHbciJob(pro, outbox, u, a, cmd, &mj);
  if (rv==0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Reusing existing multi job");
    jobIsNew=0;
  }
  else {
    if (rv!=GWEN_ERROR_NOT_FOUND) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Error looking for multi job (%d), ignoring", rv);
    }
  }

  /* create new job if necessary */
  if (mj==NULL) {
    rv=AH_Provider__CreateHbciJob(pro, u, a, cmd, &mj);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  assert(mj);

  if (jobIsNew) {
    /* check whether we need to sign the job */
    sigs=AH_Job_GetMinSignatures(mj);
    if (sigs) {
      if (sigs>1) {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Multiple signatures not yet supported");
	GWEN_Gui_ProgressLog(0,
			     GWEN_LoggerLevel_Error,
			     I18N("ERROR: Multiple signatures not "
                                  "yet supported"));
        AH_Job_free(mj);
	return GWEN_ERROR_GENERIC;
      }
      AH_Job_AddSigner(mj, AB_User_GetUserId(u));
    }
  }

  /* store HBCI job, link both jobs */
  if (AH_Job_GetId(mj)==0) {
    int jid;

    jid=AB_Banking_GetNamedUniqueId(AB_Provider_GetBanking(pro), "job", 1);
    assert(jid);
    AH_Job_SetId(mj, jid);
  }

  /* exchange arguments */
  rv=AH_Job_HandleCommand(mj, t);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_free(mj);
    return rv;
  }

  if (jobIsNew) {
    /* add job to outbox */
    AH_Outbox_AddJob(outbox, mj);
    AH_Job_free(mj);
  }

  DBG_INFO(AQHBCI_LOGDOMAIN, "Job successfully added");
  return 0;
}



int AH_Provider__AddCommandsToOutbox(AB_PROVIDER *pro, AB_USERQUEUE_LIST *uql, AB_IMEXPORTER_CONTEXT *ctx, AH_OUTBOX *outbox) {
  AH_PROVIDER *hp;
  AB_USERQUEUE *uq;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  uq=AB_UserQueue_List_First(uql);
  while(uql) {
    AB_ACCOUNTQUEUE_LIST *aql;
    AB_USER *u;

    u=AB_UserQueue_GetUser(uq);
    aql=AB_UserQueue_GetAccountQueueList(uq);
    if (aql) {
      AB_ACCOUNTQUEUE *aq;

      aq=AB_AccountQueue_List_First(aql);
      while(aq) {
        AB_ACCOUNT *a;
        AB_TRANSACTION_LIST2 *tl2;

        a=AB_AccountQueue_GetAccount(aq);

        /* read transactions */
        tl2=AB_AccountQueue_GetTransactionList(aq);
        if (tl2) {
          AB_TRANSACTION_LIST2_ITERATOR *it;

          it=AB_Transaction_List2_First(tl2);
          if (it) {
            AB_TRANSACTION *t;

            t=AB_Transaction_List2Iterator_Data(it);
            while(t) {
              int rv;

              rv=AH_Provider__AddCommandToOutbox(pro, u, a, t, outbox);
              if (rv<0) {
                AB_TRANSACTION *tCopy;

                DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);

                /* error, add a transaction copy with error status to the context */
                tCopy=AB_Transaction_dup(t);
                AB_Transaction_SetStatus(tCopy, AB_Transaction_StatusError);
                AB_ImExporterContext_AddTransaction(ctx, tCopy);
              }
              t=AB_Transaction_List2Iterator_Next(it);
            }

            AB_Transaction_List2Iterator_free(it);
          }
        }

        aq=AB_AccountQueue_List_Next(aq);
      }

      u=AB_UserQueue_GetUser(uq);
      AB_UserQueue_SetUser(uq, NULL);
      AB_User_free(u);
    }

    uq=AB_UserQueue_List_Next(uq);
  }

  return 0;
}




int AH_Provider__SampleResults(AB_PROVIDER *pro, AH_OUTBOX *outbox, AB_IMEXPORTER_CONTEXT *ctx) {
  AH_PROVIDER *hp;
  int rv;
  AH_JOB_LIST *mjl;
  AH_JOB *j;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  assert(outbox);

  mjl=AH_Outbox_GetFinishedJobs(outbox);
  assert(mjl);

  j=AH_Job_List_First(mjl);
  while(j) {
    AB_MESSAGE_LIST *ml;

    if (0) {
      const GWEN_STRINGLIST *sl;
      GWEN_STRINGLISTENTRY *se;

      /* exchange logs */
      sl=AH_Job_GetLogs(j);
      assert(sl);
      se=GWEN_StringList_FirstEntry(sl);
      while(se) {
        const char *s;

        s=GWEN_StringListEntry_Data(se);
        assert(s);
        //AB_Job_LogRaw(bj, s);
        se=GWEN_StringListEntry_Next(se);
      }
    }

    /* get remaining results */
    rv=AH_Job_HandleResults(j, ctx);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    }

    /* copy messages from AH_JOB to imexporter context */
    ml=AH_Job_GetMessages(j);
    if (ml) {
      AB_MESSAGE *msg;
  
      msg=AB_Message_List_First(ml);
      while(msg) {
        AB_ImExporterContext_AddMessage(ctx, AB_Message_dup(msg));
        msg=AB_Message_List_Next(msg);
      }
    }
  
    j=AH_Job_List_Next(j);
  } /* while j */

  return 0;
}



int AH_Provider_SendCommands(AB_PROVIDER *pro, AB_PROVIDERQUEUE *pq, AB_IMEXPORTER_CONTEXT *ctx) {
  AH_PROVIDER *hp;
  AB_USERQUEUE_LIST *uql;
  int rv;
  int rv2;
  AH_OUTBOX *outbox;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  /* sort into user queue list */
  uql=AB_UserQueue_List_new();
  rv=AH_Provider__SortProviderQueueIntoUserQueueList(pro, pq, uql);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Provider__FreeUsersAndAccountsFromUserQueueList(pro, uql);
    AB_UserQueue_List_free(uql);
    return rv;
  }

  /* add users to outbox */
  outbox=AH_Outbox_new(hp->hbci);
  rv=AH_Provider__AddCommandsToOutbox(pro, uql, ctx, outbox);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Provider__FreeUsersAndAccountsFromUserQueueList(pro, uql);
    AB_UserQueue_List_free(uql);
    return rv;
  }

  /* actually send commands */
  rv=AH_Outbox_Execute(outbox, ctx, 0, 1, 1);
  if (rv<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error executing outbox (%d).", rv);
    rv=GWEN_ERROR_GENERIC;
  }

  /* gather results */
  rv2=AH_Provider__SampleResults(pro, outbox, ctx);
  if (rv2<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error sampling results (%d)", rv2);
  }

  AH_Outbox_free(outbox);


  /* release accounts and users we loaded */
  AH_Provider__FreeUsersAndAccountsFromUserQueueList(pro, uql);

  /* error code from AH_Outbox_Execute is more important than that from AH_Provider__SampleResults */
  if (rv>=0)
    rv=rv2;

  return rv;
}



