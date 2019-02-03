/***************************************************************************
    begin       : Wed Dec 26 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



int EBC_Provider_SendCommands(AB_PROVIDER *pro, AB_PROVIDERQUEUE *pq, AB_IMEXPORTER_CONTEXT *ctx)
{
  EBC_PROVIDER *hp;
  AB_USERQUEUE_LIST *uql;
  AB_USERQUEUE *uq;
  int rv;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(hp);

  /* sort into user queue list */
  uql=AB_UserQueue_List_new();
  rv=AB_Provider_SortProviderQueueIntoUserQueueList(pro, pq, uql);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    AB_Provider_FreeUsersAndAccountsFromUserQueueList(pro, uql);
    AB_UserQueue_List_free(uql);
    return rv;
  }

  uq=AB_UserQueue_List_First(uql);
  while (uq) {
    int rv;

    rv=EBC_Provider__SendUserQueue(pro, uq, ctx);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    }
    uq=AB_UserQueue_List_Next(uq);
  }

  /* release accounts and users we loaded */
  AB_Provider_FreeUsersAndAccountsFromUserQueueList(pro, uql);

  return 0;
}



int EBC_Provider__SendUserQueue(AB_PROVIDER *pro, AB_USERQUEUE *uq, AB_IMEXPORTER_CONTEXT *ctx)
{
  AB_ACCOUNTQUEUE_LIST *aql;
  AB_USER *u;
  GWEN_HTTP_SESSION *sess;
  int rv;

  assert(u);
  u=AB_UserQueue_GetUser(uq);
  assert(u);
  DBG_ERROR(0, "Handling user \"%s\"", AB_User_GetUserId(u));

  sess=EBC_Dialog_new(pro, u);
  rv=GWEN_HttpSession_Init(sess);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not open session");
    GWEN_HttpSession_free(sess);
    return rv;
  }

  aql=AB_UserQueue_GetAccountQueueList(uq);
  if (aql) {
    AB_ACCOUNTQUEUE *aq;
    int rv;

    GWEN_Gui_ProgressLog2(0, GWEN_LoggerLevel_Info, "Locking customer \"%lu\"", (unsigned long int) AB_User_GetUniqueId(u));
    rv=AB_Provider_BeginExclUseUser(pro, u);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "Could not lock user [%lu] (%d)", (unsigned long int) AB_User_GetUniqueId(u), rv);
      GWEN_Gui_ProgressLog2(0, GWEN_LoggerLevel_Error, I18N("Could not lock user \"%lu\""),
                            (unsigned long int) AB_User_GetUniqueId(u));
      AB_Provider_EndExclUseUser(pro, u, 1);  /* abandon */
      GWEN_HttpSession_Fini(sess);
      GWEN_HttpSession_free(sess);
      return rv;
    }

    aq=AB_AccountQueue_List_First(aql);
    while (aq) {
      int rv;

      rv=EBC_Provider__SendAccountQueue(pro, u, aq, sess, ctx);
      if (rv<0) {
        DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      }

      aq=AB_AccountQueue_List_Next(aq);
    } /* while aq */

    GWEN_Gui_ProgressLog2(0, GWEN_LoggerLevel_Info, I18N("Unlocking customer \"%lu\""),
                          (unsigned long int) AB_User_GetUniqueId(u));
    rv=AB_Provider_EndExclUseUser(pro, u, 0);
    if (rv<0) {
      GWEN_Gui_ProgressLog2(0, GWEN_LoggerLevel_Error, I18N("Could not unlock user \"%lu\""),
                            (unsigned long int) AB_User_GetUniqueId(u));
      AB_Provider_EndExclUseUser(pro, u, 1); /* abandon */
      GWEN_HttpSession_Fini(sess);
      GWEN_HttpSession_free(sess);
      return rv;
    }
  }

  /* close and destroy session */
  GWEN_HttpSession_Fini(sess);
  GWEN_HttpSession_free(sess);

  return 0;
}



int EBC_Provider__SendAccountQueue(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNTQUEUE *aq,
                                   GWEN_HTTP_SESSION *sess, AB_IMEXPORTER_CONTEXT *ctx)
{
  AB_JOBQUEUE_LIST *jql;
  AB_ACCOUNT *a;

  a=AB_AccountQueue_GetAccount(aq);
  assert(a);

  EBC_Provider_SortTransactionsIntoJobQueues(pro, aq);

  jql=AB_AccountQueue_GetJobQueueList(aq);
  if (jql) {
    AB_JOBQUEUE *jq;

    jq=AB_JobQueue_List_First(jql);
    while (jq) {
      AB_TRANSACTION_COMMAND cmd;
      int rv;

      cmd=AB_JobQueue_GetJobType(jq);
      switch (cmd) {
      case AB_Transaction_CommandGetTransactions:
        rv=EBC_Provider_ExecContext_STA(pro, ctx, u, a, sess, jq);
        break;
      case AB_Transaction_CommandTransfer:
      case AB_Transaction_CommandDebitNote:
      default:
        rv=GWEN_ERROR_NOT_SUPPORTED;
        break;
      }

      if (rv<0) {
        DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      }
      jq=AB_JobQueue_List_Next(jq);
    }
  }

  return 0;
}





void EBC_Provider_SortTransactionsIntoJobQueues(AB_PROVIDER *pro, AB_ACCOUNTQUEUE *aq)
{
  AB_TRANSACTION_LIST2 *tl;

  tl=AB_AccountQueue_GetTransactionList(aq);
  if (tl) {
    AB_TRANSACTION_LIST2_ITERATOR *it;

    it=AB_Transaction_List2_First(tl);
    if (it) {
      AB_TRANSACTION *t;

      t=AB_Transaction_List2Iterator_Data(it);
      while (t) {
        AB_JOBQUEUE *jq;
        AB_TRANSACTION_COMMAND cmd;

        cmd=AB_Transaction_GetCommand(t);
        jq=AB_AccountQueue_FindJobQueue(aq, cmd);
        if (jq==NULL) {
          jq=AB_JobQueue_new();
          AB_JobQueue_SetJobType(jq, cmd);
          AB_AccountQueue_AddJobQueue(aq, jq);
        }
        if (cmd==AB_Transaction_CommandGetTransactions) {
          AB_TRANSACTION *tFirst;

          tFirst=AB_JobQueue_GetFirstTransaction(jq);
          if (tFirst) {
            /* don't add, just set reference id */
            AB_Transaction_SetRefUniqueId(t, AB_Transaction_GetUniqueId(tFirst));
          }
          else {
            /* add */
            AB_JobQueue_AddTransaction(jq, t);
          }
        }
        else
          AB_JobQueue_AddTransaction(jq, t);

        t=AB_Transaction_List2Iterator_Next(it);
      }
      AB_Transaction_List2Iterator_free(it);
    }
  }
}



int EBC_Provider_ExecContext_STA(AB_PROVIDER *pro,
                                 AB_IMEXPORTER_CONTEXT *ctx,
                                 GWEN_UNUSED AB_USER *u,
                                 GWEN_UNUSED AB_ACCOUNT *a,
                                 GWEN_HTTP_SESSION *sess,
                                 AB_JOBQUEUE *jq)
{
  EBC_PROVIDER *dp;
  AB_TRANSACTION_LIST2 *jl;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  jl=AB_JobQueue_GetTransactionList(jq);
  if (jl) {
    AB_TRANSACTION_LIST2_ITERATOR *jit;

    jit=AB_Transaction_List2_First(jl);
    if (jit) {
      AB_TRANSACTION *uj;

      uj=AB_Transaction_List2Iterator_Data(jit);
      assert(uj);
      while (uj) {
        int rv;

        /* exchange STA request */
        rv=EBC_Provider_XchgStaRequest(pro, sess,
                                       AB_Transaction_GetFirstDate(uj),
                                       AB_Transaction_GetLastDate(uj),
                                       ctx);
        if (rv==0) {
          AB_Transaction_SetStatus(uj, AB_Transaction_StatusAccepted);
        }
        else {
          if (rv==GWEN_ERROR_NO_DATA)
            AB_Transaction_SetStatus(uj, AB_Transaction_StatusAccepted);
          else {
            AB_Transaction_SetStatus(uj, AB_Transaction_StatusRejected);
            if (rv==GWEN_ERROR_USER_ABORTED) {
              DBG_INFO(AQEBICS_LOGDOMAIN, "User aborted");
              AB_Transaction_List2Iterator_free(jit);
              return rv;
            }
          }
        }
        uj=AB_Transaction_List2Iterator_Next(jit);
      } /* while */
      AB_Transaction_List2Iterator_free(jit);
    }
  } /* if jl */

  return 0;
}


#if 0
int EBC_Provider_ExecContext__IZV(AB_PROVIDER *pro,
                                  AB_IMEXPORTER_CONTEXT *ctx,
                                  AB_USER *u,
                                  AB_ACCOUNT *a,
                                  GWEN_HTTP_SESSION *sess,
                                  EBC_CONTEXT *ectx)
{
  EBC_PROVIDER *dp;
  AB_JOB_LIST2_ITERATOR *jit;
  AB_JOB_STATUS js;
  AB_IMEXPORTER_CONTEXT *exCtx;
  AB_IMEXPORTER_ACCOUNTINFO *ai;
  GWEN_BUFFER *bufDtaus;
  GWEN_TIME *currentTime;
  GWEN_BUFFER *logbuf;
  int rv;
  const char *profileName=NULL;
  const char *s;
  const char *rqType;
  uint32_t groupId=0;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  /* prepare CTX log */
  logbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(logbuf, "BEGIN");
  currentTime=GWEN_CurrentTime();
  GWEN_Time_toString(currentTime, I18N("YYYY/MM/DD-hh:mm:ss"), logbuf);
  GWEN_Time_free(currentTime);
  GWEN_Buffer_AppendString(logbuf, "\t");
  GWEN_Buffer_AppendString(logbuf, I18N("National Mass Transfer"));
  GWEN_Buffer_AppendString(logbuf, "\n");
  GWEN_Buffer_AppendString(logbuf, "\t");
  GWEN_Buffer_AppendString(logbuf, I18N("Transfer type: "));
  GWEN_Buffer_AppendString(logbuf, "\t");

  switch (EBC_Context_GetJobType(ectx)) {
  case AB_Job_TypeTransfer:
    if (!profileName)
      profileName="transfer";
    GWEN_Buffer_AppendString(logbuf, I18N("Transfer"));
    break;
  case AB_Job_TypeDebitNote:
    if (!profileName)
      profileName="debitnote";
    GWEN_Buffer_AppendString(logbuf, I18N("Debit Note"));
    break;
  default:
    GWEN_Buffer_AppendString(logbuf, I18N("unknown"));
    break;
  }
  GWEN_Buffer_AppendString(logbuf, "\n");

  GWEN_Buffer_AppendString(logbuf, "\t");
  GWEN_Buffer_AppendString(logbuf, I18N("Account: "));
  GWEN_Buffer_AppendString(logbuf, "\t");
  GWEN_Buffer_AppendString(logbuf, AB_Account_GetBankCode(a));
  GWEN_Buffer_AppendString(logbuf, " / ");
  GWEN_Buffer_AppendString(logbuf, AB_Account_GetAccountNumber(a));
  GWEN_Buffer_AppendString(logbuf, "\n");
  /* add a tab-less line to start a new table */
  GWEN_Buffer_AppendString(logbuf, "Transactions\n");

  DBG_INFO(AQEBICS_LOGDOMAIN, "Sampling transactions from jobs");
  exCtx=AB_ImExporterContext_new();
  ai=AB_ImExporterAccountInfo_new();
  AB_ImExporterAccountInfo_FillFromAccount(ai, a);

  jit=AB_Job_List2_First(EBC_Context_GetJobs(ectx));
  if (jit) {
    AB_JOB *uj;

    uj=AB_Job_List2Iterator_Data(jit);
    assert(uj);
    while (uj) {
      AB_TRANSACTION *t;
      const char *s;
      const AB_VALUE *v;

      switch (EBC_Context_GetJobType(ectx)) {
      case AB_Job_TypeTransfer:
      case AB_Job_TypeDebitNote:
        t=AB_Job_GetTransaction(uj);
        break;
      default:
        t=NULL;
      }
      assert(t);

      if (groupId==0)
        /* take id from first job of the created DTAUS doc */
        groupId=AB_Job_GetJobId(uj);
      AB_Transaction_SetGroupId(t, groupId);

      AB_ImExporterAccountInfo_AddTransaction(ai, AB_Transaction_dup(t));
      s=AB_Transaction_GetRemoteName(t);
      if (!s)
        s=I18N("unknown");
      GWEN_Buffer_AppendString(logbuf, s);
      GWEN_Buffer_AppendString(logbuf, "\t");
      s=AB_Transaction_GetRemoteBankCode(t);
      if (!s)
        s="????????";
      GWEN_Buffer_AppendString(logbuf, s);
      GWEN_Buffer_AppendString(logbuf, "\t");
      s=AB_Transaction_GetRemoteAccountNumber(t);
      if (!s)
        s="??????????";
      GWEN_Buffer_AppendString(logbuf, s);
      GWEN_Buffer_AppendString(logbuf, "\t");
      s=AB_Transaction_GetPurpose(t);
      if (!s)
        s="";
      GWEN_Buffer_AppendString(logbuf, s);
      GWEN_Buffer_AppendString(logbuf, "\t");
      v=AB_Transaction_GetValue(t);
      if (v)
        AB_Value_toHumanReadableString(v, logbuf, 2);
      else
        GWEN_Buffer_AppendString(logbuf, "0,00 EUR");
      GWEN_Buffer_AppendString(logbuf, "\n");

      uj=AB_Job_List2Iterator_Next(jit);
    } /* while */
    AB_Job_List2Iterator_free(jit);
  }
  AB_ImExporterContext_AddAccountInfo(exCtx, ai);

  GWEN_Buffer_AppendString(logbuf, I18N("Results:\n"));

  /* export as DTAUS to bufDtaus */
  bufDtaus=GWEN_Buffer_new(0, 1024, 0, 1);

  DBG_INFO(AQEBICS_LOGDOMAIN, "Exporting transactions to DTAUS[default]");
  rv=AB_Banking_ExportToBuffer(AB_Provider_GetBanking(pro),
                               exCtx,
                               "dtaus",
                               profileName,
                               bufDtaus);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(bufDtaus);
    EBC_Provider_SetJobListStatus(EBC_Context_GetJobs(ectx),
                                  AB_Job_StatusError);
    GWEN_Buffer_AppendString(logbuf, "\t");
    GWEN_Buffer_AppendString(logbuf, I18N("Error while exporting to DTAUS\n"));
    GWEN_Buffer_AppendString(logbuf, "END\n");

    GWEN_Buffer_free(logbuf);
    return rv;
  }

  GWEN_Buffer_AppendString(logbuf, "\t");
  GWEN_Buffer_AppendString(logbuf, I18N("Exporting to DTAUS: ok\n"));

  /* exchange upload request */
  DBG_INFO(AQEBICS_LOGDOMAIN, "Uploading.");
  AB_HttpSession_ClearLog(sess);

  if (EBC_Context_GetJobType(ectx)==AB_Job_TypeDebitNote) {
    if (EBC_User_GetFlags(u) & EBC_USER_FLAGS_USE_IZL)
      rqType="IZL";
    else
      rqType="IZV";
  }
  else
    rqType="IZV";
  rv=EBC_Provider_XchgUploadRequest(pro, sess, u, rqType,
                                    (const uint8_t *)GWEN_Buffer_GetStart(bufDtaus),
                                    GWEN_Buffer_GetUsedBytes(bufDtaus));
  if (rv<0 || rv>=300)
    js=AB_Job_StatusError;
  else
    js=AB_Job_StatusFinished;

  s=AB_HttpSession_GetLog(sess);
  if (s)
    GWEN_Buffer_AppendString(logbuf, s);
  GWEN_Buffer_AppendString(logbuf, "END\n");

  GWEN_Buffer_free(logbuf);

  EBC_Provider_SetJobListStatus(EBC_Context_GetJobs(ectx), js);

  DBG_INFO(AQEBICS_LOGDOMAIN, "Done");
  return 0;
}



int EBC_Provider_ExecContext_IZV(AB_PROVIDER *pro,
                                 AB_IMEXPORTER_CONTEXT *ctx,
                                 AB_USER *u,
                                 AB_ACCOUNT *a,
                                 GWEN_HTTP_SESSION *sess,
                                 EBC_CONTEXT *ectx)
{
  EBC_PROVIDER *dp;
  AB_JOB_LIST2_ITERATOR *jit;
  int rv;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  DBG_INFO(AQEBICS_LOGDOMAIN, "Executing IZV request");
  rv=EBC_Provider_ExecContext__IZV(pro, ctx, u, a, sess, ectx);

  jit=AB_Job_List2_First(EBC_Context_GetJobs(ectx));
  if (jit) {
    AB_JOB *uj;

    uj=AB_Job_List2Iterator_Data(jit);
    assert(uj);
    while (uj) {
      const AB_TRANSACTION *ot;

      switch (EBC_Context_GetJobType(ectx)) {
      case AB_Job_TypeTransfer:
      case AB_Job_TypeDebitNote:
        ot=AB_Job_GetTransaction(uj);
        break;
      default:
        ot=NULL;
      }

      assert(ot);
      if (ot) {
        AB_TRANSACTION *t;
        AB_TRANSACTION_STATUS tStatus;

        switch (AB_Job_GetStatus(uj)) {
        case AB_Job_StatusFinished:
          tStatus=AB_Transaction_StatusAccepted;
          break;
        case AB_Job_StatusPending:
          tStatus=AB_Transaction_StatusPending;
          break;
        default:
          tStatus=AB_Transaction_StatusRejected;
          break;
        }

        t=AB_Transaction_dup(ot);
        AB_Transaction_SetStatus(t, tStatus);
        AB_Transaction_SetUniqueAccountId(t, AB_Account_GetUniqueId(a));
        if (AB_Transaction_GetType(t)<=AB_Transaction_TypeNone)
          AB_Transaction_SetType(t, AB_Transaction_TypeTransfer);
        AB_ImExporterContext_AddTransaction(ctx, t);
      }

      uj=AB_Job_List2Iterator_Next(jit);
    } /* while */
    AB_Job_List2Iterator_free(jit);
  }

  return rv;
}
#endif



