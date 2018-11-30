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



int AH_Provider_AddAccount(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a, int withAccountSpec) {
  uint32_t uid;
  int rv;

  /* add account */
  uid=AB_Banking_GetNamedUniqueId(AB_Provider_GetBanking(pro), "account", 1); /* startAtStdUniqueId=1 */
  rv=AB_Provider_AddAccount(pro, a);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  uid=AB_Account_GetUniqueId(a);
  assert(uid);

  /* write account spec */
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

  rv1=AB_Banking_DeleteAccountSpec(AB_Provider_GetBanking(pro), uid);
  if (rv1<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv1);
  }

  rv2=AB_Provider_DeleteAccount(pro, uid);
  if (rv2<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv2);
  }

  if (rv1>0)
    return rv1;
  if (rv2>0)
    return rv2;
  return 0;
}








int AH_Provider_AddUser(AB_PROVIDER *pro, AB_USER *u) {
  uint32_t uid;
  int rv;

  uid=AB_Banking_GetNamedUniqueId(AB_Provider_GetBanking(pro), "user", 1); /* startAtStdUniqueId=1 */
  AB_User_SetUniqueId(u, uid);
  rv=AB_Provider_WriteUser(pro, uid, 1, 1, u); /* lock, unlock */
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
  rv=AB_Provider_ReadAccounts(pro, al);
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
      if (AB_Account_GetUserId(a)==uid) {
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

  rv=AB_Provider_DeleteUser(pro, uid);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  return 0;
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

  /* set job id in command, add command to job */
  AB_Transaction_SetUniqueId(t, AH_Job_GetId(mj));
  AH_Job_AddCommand(mj, t);

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
  rv=AB_Provider_SortProviderQueueIntoUserQueueList(pro, pq, uql);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_Provider_FreeUsersAndAccountsFromUserQueueList(pro, uql);
    AB_UserQueue_List_free(uql);
    return rv;
  }

  /* add users to outbox */
  outbox=AH_Outbox_new(pro);
  rv=AH_Provider__AddCommandsToOutbox(pro, uql, ctx, outbox);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_Provider_FreeUsersAndAccountsFromUserQueueList(pro, uql);
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
  AB_Provider_FreeUsersAndAccountsFromUserQueueList(pro, uql);

  /* error code from AH_Outbox_Execute is more important than that from AH_Provider__SampleResults */
  if (rv>=0)
    rv=rv2;

  return rv;
}



AB_ACCOUNT *AH_Provider_CreateAccountObject(AB_PROVIDER *pro) {
  return AH_Account_new(pro);
}



AB_USER *AH_Provider_CreateUserObject(AB_PROVIDER *pro) {
  return AH_User_new(pro);
}




