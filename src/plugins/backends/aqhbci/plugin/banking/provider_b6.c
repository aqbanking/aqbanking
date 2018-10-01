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



int AH_Provider_GetAccount(AB_PROVIDER *pro, uint32_t uid, int doLock, int doUnlock, AB_ACCOUNT **pAccount) {
  int rv;
  AB_ACCOUNT *a;

  a=AH_Account_new(AB_Provider_GetBanking(pro), pro);
  assert(a);
  rv=AH_Provider_ReadAccount(pro, uid, doLock, doUnlock, a);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_Account_free(a);
    return rv;
  }
  *pAccount=a;

  return 0;
}



int AH_Provider_ReadAccounts(AB_PROVIDER *pro, AB_ACCOUNT_LIST *accountList) {
  int rv;
  GWEN_DB_NODE *dbAll=NULL;
  GWEN_DB_NODE *db;

  /* read all config groups for accounts which have a unique id and which belong to AqHBCI */
  rv=AB_Banking6_ReadConfigGroups(AB_Provider_GetBanking(pro), AB_CFG_GROUP_ACCOUNTS, "uniqueId", "provider", "AQHBCI", &dbAll);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  db=GWEN_DB_GetFirstGroup(dbAll);
  while(db) {
    AB_ACCOUNT *a=NULL;

    a=AH_Account_new(AB_Provider_GetBanking(pro), pro);
    rv=AH_Account_ReadDb(a, db);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error reading account (%d), ignoring", rv);
      AB_Account_free(a);
    }
    else
      AB_Account_List_Add(a, accountList);

    /* next */
    db=GWEN_DB_GetNextGroup(db);
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



int AH_Provider_AddAccount(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a, int withAccountSpec) {
  uint32_t uid;
  int rv;

  uid=AB_Banking_GetNamedUniqueId(AB_Provider_GetBanking(pro), "account", 1); /* startAtStdUniqueId=1 */
  AB_Account_SetUniqueId(a, uid);
  rv=AH_Provider_WriteAccount(pro, uid, 1, 1, a); /* lock, unlock */
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=AH_Provider_WriteAccountSpecForAccount(pro, u, a);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AH_Provider_DeleteAccount(AB_PROVIDER *pro, uint32_t uid) {
  int rv1;
  int rv2;

  rv1=AB_Banking6_Delete_AccountConfig(AB_Provider_GetBanking(pro), uid);
  if (rv1<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv1);
  }

  rv2=AB_Banking6_DeleteAccountSpec(AB_Provider_GetBanking(pro), uid);
  if (rv2<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv2);
  }

  if (rv1>0)
    return rv1;
  if (rv2>0)
    return rv2;
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



int AH_Provider_GetUser(AB_PROVIDER *pro, uint32_t uid, int doLock, int doUnlock, AB_USER **pUser) {
  int rv;
  AB_USER *u;

  u=AH_User_new(AB_Provider_GetBanking(pro), pro);
  assert(u);
  rv=AH_Provider_ReadUser(pro, uid, doLock, doUnlock, u);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_User_free(u);
    return rv;
  }

  *pUser=u;

  return 0;
}



int AH_Provider_ReadUsers(AB_PROVIDER *pro, AB_USER_LIST *userList) {
  int rv;
  GWEN_DB_NODE *dbAll=NULL;
  GWEN_DB_NODE *db;

  /* read all config groups for users which have a unique id and which belong to AqHBCI */
  rv=AB_Banking6_ReadConfigGroups(AB_Provider_GetBanking(pro), AB_CFG_GROUP_USERS, "uniqueId", "backendName", "AQHBCI", &dbAll);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  db=GWEN_DB_GetFirstGroup(dbAll);
  while(db) {
    AB_USER *u=NULL;

    u=AH_User_new(AB_Provider_GetBanking(pro), pro);
    rv=AH_User_ReadDb(u, db);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error reading user (%d), ignoring", rv);
      AB_User_free(u);
    }
    else
      AB_User_List_Add(u, userList);

    /* next */
    db=GWEN_DB_GetNextGroup(db);
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



int AH_Provider_AddUser(AB_PROVIDER *pro, AB_USER *u) {
  uint32_t uid;
  int rv;

  uid=AB_Banking_GetNamedUniqueId(AB_Provider_GetBanking(pro), "user", 1); /* startAtStdUniqueId=1 */
  AB_User_SetUniqueId(u, uid);
  rv=AH_Provider_WriteUser(pro, uid, 1, 1, u); /* lock, unlock */
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  return 0;
}



int AH_Provider_DeleteUser(AB_PROVIDER *pro, uint32_t uid) {
  int rv;
  AB_ACCOUNT_LIST *al;

  al=AB_Account_List_new();
  rv=AH_Provider_ReadAccounts(pro, al);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_Account_List_free(al);
    return rv;
  }
  else {
    AB_ACCOUNT *a;
    int cnt=0;

    a=AB_Account_List_First(al);
    while(a) {
      if (AB_Account_HasUserId(a, uid)) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Account %lu still uses this user", (unsigned long int) AB_Account_GetUniqueId(a));
        cnt++;
      }
      a=AB_Account_List_Next(a);
    }
    if (cnt>0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "%d accounts using this user",cnt);
      AB_Account_List_free(al);
      return GWEN_ERROR_INVALID;
    }
  }
  AB_Account_List_free(al);

  rv=AB_Banking6_Delete_UserConfig(AB_Provider_GetBanking(pro), uid);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  return 0;
}







int AH_Provider_BeginExclUseAccount(AB_PROVIDER *pro, AB_ACCOUNT *a) {
  int rv;
  uint32_t uid;

  uid=AB_Account_GetUniqueId(a);
  if (uid==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No unique id!");
    return GWEN_ERROR_INVALID;
  }

  rv=AH_Provider_ReadAccount(pro, uid, 1, 0, a);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  return 0;
}



int AH_Provider_EndExclUseAccount(AB_PROVIDER *pro, AB_ACCOUNT *a, int abandon) {
  int rv;
  uint32_t uid;

  uid=AB_Account_GetUniqueId(a);
  if (uid==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No unique id!");
    return GWEN_ERROR_INVALID;
  }

  if (!abandon) {
    rv=AB_Banking6_Unlock_AccountConfig(AB_Provider_GetBanking(pro), uid);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  else {
    rv=AH_Provider_WriteAccount(pro, uid, 0, 1, a);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}



int AH_Provider_BeginExclUseUser(AB_PROVIDER *pro, AB_USER *u) {
  int rv;
  uint32_t uid;

  uid=AB_User_GetUniqueId(u);
  if (uid==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No unique id!");
    return GWEN_ERROR_INVALID;
  }
  rv=AH_Provider_ReadUser(pro, uid, 1, 0, u);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  return 0;
}



int AH_Provider_EndExclUseUser(AB_PROVIDER *pro, AB_USER *u, int abandon) {
  int rv;
  uint32_t uid;

  uid=AB_User_GetUniqueId(u);
  if (uid==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No unique id!");
    return GWEN_ERROR_INVALID;
  }

  if (!abandon) {
    rv=AB_Banking6_Unlock_UserConfig(AB_Provider_GetBanking(pro), uid);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  else {
    rv=AH_Provider_WriteUser(pro, uid, 0, 1, u);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}





int AH_Provider__SortProviderQueueIntoUserQueueList(AB_PROVIDER *pro, AB_PROVIDERQUEUE *pq, AB_USERQUEUE_LIST *uql) {
  AH_PROVIDER *hp;
  AB_ACCOUNTQUEUE_LIST *aql;
  AB_ACCOUNTQUEUE *aq;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

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
    rv=AH_Provider_GetAccount(pro, aid, 1, 1, &a);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    AB_AccountQueue_SetAccount(aq, a);

    /* determine user */
    uid=AB_Account_GetFirstUserIdAsInt(a);
    if (uid==0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "No first user in account %lu, SNH!", (unsigned long int) aid);
      return GWEN_ERROR_INTERNAL;
    }
    else {
      uq=AB_UserQueue_List_GetByUserId(uql, uid);
      if (uq==NULL) {
        AB_USER *u=NULL;

        rv=AH_Provider_GetUser(pro, uid, 1, 1, &u);
        if (rv<0) {
          DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
          return rv;
        }
        uq=AB_UserQueue_new();
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

    AB_UserQueue_SetUser(uq, NULL);
    AB_User_free(u);

    uq=AB_UserQueue_List_Next(uq);
  }
}



int AH_Provider__AddCommandToOutbox(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *t, AH_OUTBOX *outbox) {
  int rv;
  int cmd;
  AH_JOB *mj=NULL;
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
    int sigs;
    
    /* check whether we need to sign the job */
    sigs=AH_Job_GetMinSignatures(mj);
    if (sigs) {
      if (sigs>1) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Multiple signatures not yet supported");
                  GWEN_Gui_ProgressLog(0,
                  GWEN_LoggerLevel_Error,
                  I18N("ERROR: Multiple signatures not yet supported"));
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
  while(uq) {
    AB_ACCOUNTQUEUE_LIST *aql;
    AB_USER *u;

    u=AB_UserQueue_GetUser(uq);
    assert(u);
    DBG_ERROR(0, "Handling user \"%s\"", AB_User_GetUserId(u));
    aql=AB_UserQueue_GetAccountQueueList(uq);
    if (aql) {
      AB_ACCOUNTQUEUE *aq;

      aq=AB_AccountQueue_List_First(aql);
      while(aq) {
        AB_ACCOUNT *a;
        AB_TRANSACTION_LIST2 *tl2;

        a=AB_AccountQueue_GetAccount(aq);
        assert(a);

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
    }

    uq=AB_UserQueue_List_Next(uq);
  }

  return 0;
}




int AH_Provider__SampleResults(AB_PROVIDER *pro, AH_OUTBOX *outbox, AB_IMEXPORTER_CONTEXT *ctx) {
  AH_PROVIDER *hp;
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
    int rv;

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
  outbox=AH_Outbox_new(pro);
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






