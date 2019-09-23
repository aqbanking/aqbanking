/***************************************************************************
    begin       : Sat Dec 01 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


/* included from provider.c */

AB_TRANSACTION *APY_Provider_FindJobById(AB_TRANSACTION_LIST2 *jl, uint32_t jid)
{
  AB_TRANSACTION_LIST2_ITERATOR *jit;

  jit=AB_Transaction_List2_First(jl);
  if (jit) {
    AB_TRANSACTION *j;

    j=AB_Transaction_List2Iterator_Data(jit);
    assert(j);
    while (j) {
      if (AB_Transaction_GetUniqueId(j)==jid) {
        AB_Transaction_List2Iterator_free(jit);
        return j;
      }
      j=AB_Transaction_List2Iterator_Next(jit);
    } /* while */
    AB_Transaction_List2Iterator_free(jit);
  }

  return 0;
}


int APY_Provider__AddJobToList2(AB_PROVIDER *pro, AB_TRANSACTION *j, AB_TRANSACTION_LIST2 *jobList)
{
  APY_PROVIDER *dp;
  uint32_t aid=0;
  int doAdd=1;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, APY_PROVIDER, pro);
  assert(dp);

  aid=AB_Transaction_GetUniqueAccountId(j);
  assert(aid);

  switch (AB_Transaction_GetCommand(j)) {
  case AB_Transaction_CommandGetBalance:
  case AB_Transaction_CommandGetTransactions:
    break;

  case AB_Transaction_CommandTransfer:
  case AB_Transaction_CommandDebitNote:
  default:
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "Job not supported (%d)", AB_Transaction_GetCommand(j));
    return GWEN_ERROR_NOT_SUPPORTED;
  } /* switch */


  if (AB_Transaction_GetCommand(j)==AB_Transaction_CommandGetBalance) {
    AB_TRANSACTION_LIST2_ITERATOR *jit;

    /* check whether a getBalance job already exists. If it does then
     * we don't have to send this job again, once is enough. */
    jit=AB_Transaction_List2_First(jobList);
    if (jit) {
      AB_TRANSACTION *uj;

      uj=AB_Transaction_List2Iterator_Data(jit);
      assert(uj);
      while (uj) {
        AB_TRANSACTION_COMMAND jt;

        jt=AB_Transaction_GetCommand(uj);
	//if (jt==AB_Transaction_CommandGetBalance ||
	//  jt==AB_Transaction_CommandGetTransactions) {
        if (jt==AB_Transaction_CommandGetBalance) {
          if (AB_Transaction_GetUniqueAccountId(uj)==aid) {
            /* let new job refer to the only already in queue */
            AB_Transaction_SetRefUniqueId(j, AB_Transaction_GetUniqueId(uj));
            doAdd=0;
            break;
          }
        }
        uj=AB_Transaction_List2Iterator_Next(jit);
      } /* while */
      AB_Transaction_List2Iterator_free(jit);
    }
  }
  else if (AB_Transaction_GetCommand(j)==AB_Transaction_CommandGetTransactions) {
    AB_TRANSACTION_LIST2_ITERATOR *jit;
    const GWEN_DATE *dtnew;

    /* check whether a getTransactions job already exists. If it does then
     * we don't have to send this job again, once is enough. */
    dtnew=AB_Transaction_GetFirstDate(j);
    jit=AB_Transaction_List2_First(jobList);
    if (jit) {
      AB_TRANSACTION *uj;

      uj=AB_Transaction_List2Iterator_Data(jit);
      assert(uj);
      while (uj) {
        AB_TRANSACTION_COMMAND jt;

        jt=AB_Transaction_GetCommand(uj);
        if (jt==AB_Transaction_CommandGetTransactions) {
          if (AB_Transaction_GetUniqueAccountId(uj)==aid) {
            if (dtnew) {
              const GWEN_DATE *dtcurr;

              dtcurr=AB_Transaction_GetFirstDate(uj);
              if (dtcurr) {
                /* current job has a time */
                if (GWEN_Date_Diff(dtcurr, dtnew)>0) {
                  /* new time is before that of current job, replace */
                  AB_Transaction_SetRefUniqueId(uj, AB_Transaction_GetUniqueId(j));
                  AB_Transaction_List2_Erase(jobList, jit);
                  doAdd=1;
                  break;
                }
              }
              else {
                /* current job has no time, so replace by job with time */
                AB_Transaction_SetRefUniqueId(uj, AB_Transaction_GetUniqueId(j));
                AB_Transaction_List2_Erase(jobList, jit);
                doAdd=1;
                break;
              }
            }
            else {
              /* new job has no time, so don't add it */
              AB_Transaction_SetRefUniqueId(j, AB_Transaction_GetUniqueId(uj));
              doAdd=0;
              break;
            }
          } /* if same account */
        } /* if GetTransactions */
        uj=AB_Transaction_List2Iterator_Next(jit);
      } /* while */
      AB_Transaction_List2Iterator_free(jit);
    }
  }

  if (doAdd) {
    /* only add to queue if needed */
    AB_Transaction_SetStatus(j, AB_Transaction_StatusEnqueued);
    AB_Transaction_List2_PushBack(jobList, j);
  }

  return 0;
}




int APY_Provider__SendJobList(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION_LIST2 *jobList,
			      AB_IMEXPORTER_CONTEXT *ctx, AB_IMEXPORTER_ACCOUNTINFO *ai)
{
  AB_TRANSACTION_LIST2_ITERATOR *jit;

  /* check whether a getBalance job already exists. If it does then
   * we don't have to send this job again, once is enough. */
  jit=AB_Transaction_List2_First(jobList);
  if (jit) {
    AB_TRANSACTION *uj;

    uj=AB_Transaction_List2Iterator_Data(jit);
    assert(uj);
    while (uj) {
      AB_TRANSACTION_COMMAND jt;
      int rv;

      jt=AB_Transaction_GetCommand(uj);
      if (jt==AB_Transaction_CommandGetBalance ||
          jt==AB_Transaction_CommandGetTransactions) {
        AB_Transaction_SetStatus(uj, AB_Transaction_StatusSending);
        //rv=APY_Provider_RequestStatements(pro, u, a, uj, ctx);

        /* request balance */
        if (jt==AB_Transaction_CommandGetBalance)
          rv=APY_Provider_ExecGetBal(pro, ai, u, uj);

        /* request statements */
        if (jt==AB_Transaction_CommandGetTransactions)
          rv=APY_Provider_ExecGetTrans(pro, ai, u, uj);

        if (rv==GWEN_ERROR_USER_ABORTED) {
          DBG_INFO(AQPAYPAL_LOGDOMAIN, "User aborted");
          AB_Transaction_List2Iterator_free(jit);
          AB_Transaction_SetStatus(uj, AB_Transaction_StatusAborted);
          return rv;
        }
        else if (rv==GWEN_ERROR_ABORTED) {
          DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Aborted");
          AB_Transaction_List2Iterator_free(jit);
          AB_Transaction_SetStatus(uj, AB_Transaction_StatusAborted);
          return rv;
        }
        else if (rv<0) {
          DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
          AB_Transaction_SetStatus(uj, AB_Transaction_StatusError);
        }
        else {
          AB_Transaction_SetStatus(uj, AB_Transaction_StatusAccepted);
        }
      }
      else {
        DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Job not supported");
        AB_Transaction_SetStatus(uj, AB_Transaction_StatusError);
      }

      rv=GWEN_Gui_ProgressAdvance(0, GWEN_GUI_PROGRESS_ONE);
      if (rv==GWEN_ERROR_USER_ABORTED) {
        DBG_INFO(AQPAYPAL_LOGDOMAIN, "User aborted");
        AB_Transaction_List2Iterator_free(jit);
        return rv;
      }

      uj=AB_Transaction_List2Iterator_Next(jit);
    } /* while */
    AB_Transaction_List2Iterator_free(jit);
  }

  return 0;
}




void APY_Provider__FinishJobs(AB_PROVIDER *pro, AB_TRANSACTION_LIST2 *jobList, AB_IMEXPORTER_CONTEXT *ctx)
{
  AB_TRANSACTION_LIST2_ITERATOR *it;

  it=AB_Transaction_List2_First(jobList);
  if (it) {
    AB_TRANSACTION *t;

    t=AB_Transaction_List2Iterator_Data(it);
    while (t) {
      uint32_t gid;

      gid=AB_Transaction_GetRefUniqueId(t);
      if (gid!=0) {
        AB_TRANSACTION *tOrig;

        tOrig=APY_Provider_FindJobById(jobList, gid);
        if (tOrig) {
          /* copy result */
          AB_Transaction_SetStatus(t, AB_Transaction_GetStatus(tOrig));
        }
      }

      /* copy command to context */
      AB_ImExporterContext_AddTransaction(ctx, AB_Transaction_dup(t));

      t=AB_Transaction_List2Iterator_Next(it);
    }

    AB_Transaction_List2Iterator_free(it);
  }
}



int APY_Provider__SendAccountQueue(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNTQUEUE *aq, AB_IMEXPORTER_CONTEXT *ctx)
{
  AB_ACCOUNT *a;
  AB_TRANSACTION_LIST2 *tl2;
  AB_TRANSACTION_LIST2 *toSend;
  AB_IMEXPORTER_ACCOUNTINFO *ai;

  a=AB_AccountQueue_GetAccount(aq);
  assert(a);
  DBG_ERROR(0, "Handling account \"%lu\"", (unsigned long int)AB_Account_GetUniqueId(a));

  ai=AB_ImExporterContext_GetOrAddAccountInfo(ctx,
                                              AB_Account_GetUniqueId(a),
                                              AB_Account_GetIban(a),
                                              AB_Account_GetBankCode(a),
                                              AB_Account_GetAccountNumber(a),
                                              AB_Transaction_TypeNone);
  if (ai==NULL) {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Could not create account info");
    return GWEN_ERROR_GENERIC;
  }

  toSend=AB_Transaction_List2_new();


  /* read transactions */
  tl2=AB_AccountQueue_GetTransactionList(aq);
  if (tl2) {
    AB_TRANSACTION_LIST2_ITERATOR *it;

    it=AB_Transaction_List2_First(tl2);
    if (it) {
      AB_TRANSACTION *t;

      t=AB_Transaction_List2Iterator_Data(it);
      while (t) {
        int rv;

        /* add job to the list of jobs to send */
        rv=APY_Provider__AddJobToList2(pro, t, toSend);
        if (rv<0) {
          AB_TRANSACTION *tCopy;

          DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);

          /* error, add a transaction copy with error status to the context */
          tCopy=AB_Transaction_dup(t);
          AB_Transaction_SetStatus(tCopy, AB_Transaction_StatusError);
          AB_ImExporterContext_AddTransaction(ctx, tCopy);

          if (rv==GWEN_ERROR_USER_ABORTED) {
            AB_Transaction_List2Iterator_free(it);
            AB_Transaction_List2_free(toSend);
            return rv;
          }
        }

        t=AB_Transaction_List2Iterator_Next(it);
      }

      AB_Transaction_List2Iterator_free(it);
    }
  }

  /* send jobs */
  if (AB_Transaction_List2_GetSize(toSend)) {
    int rv;

    rv=APY_Provider__SendJobList(pro, u, a, toSend, ctx, ai);
    if (rv<0) {
      DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    }
  }

  AB_Transaction_List2_free(toSend);

  return 0;
}



int APY_Provider__SendUserQueue(AB_PROVIDER *pro, AB_USERQUEUE *uq, AB_IMEXPORTER_CONTEXT *ctx)
{
  AB_ACCOUNTQUEUE_LIST *aql;
  AB_USER *u;

  assert(uq);
  u=AB_UserQueue_GetUser(uq);
  assert(u);
  DBG_ERROR(0, "Handling user \"%s\"", AB_User_GetUserId(u));


  aql=AB_UserQueue_GetAccountQueueList(uq);
  if (aql) {
    AB_ACCOUNTQUEUE *aq;
    int rv;
    GWEN_BUFFER *xbuf;

    GWEN_Gui_ProgressLog2(0, GWEN_LoggerLevel_Info, "Locking customer \"%lu\"", (unsigned long int) AB_User_GetUniqueId(u));
    rv=AB_Provider_BeginExclUseUser(pro, u);
    if (rv<0) {
      DBG_INFO(AQPAYPAL_LOGDOMAIN, "Could not lock user [%lu] (%d)", (unsigned long int) AB_User_GetUniqueId(u), rv);
      GWEN_Gui_ProgressLog2(0, GWEN_LoggerLevel_Error, I18N("Could not lock user \"%lu\""),
                            (unsigned long int) AB_User_GetUniqueId(u));
      AB_Provider_EndExclUseUser(pro, u, 1);  /* abandon */
      return rv;
    }

    /* read secrets */
    xbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=APY_Provider_ReadUserApiSecrets(pro, u, xbuf);
    if (rv<0) {
      DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(xbuf);
      return rv;
    }
    else {
      char *t;
      char *t2=NULL;
      GWEN_BUFFER *sbuf1;
      GWEN_BUFFER *sbuf2;
      GWEN_BUFFER *sbuf3;

      t=strchr(GWEN_Buffer_GetStart(xbuf), ':');
      if (t) {
	*(t++)=0;
	t2=strchr(t, ':');
	if (t2) {
	  *(t2++)=0;
	}
      }

      sbuf1=GWEN_Buffer_new(0, 256, 0, 1);
      sbuf2=GWEN_Buffer_new(0, 256, 0, 1);
      sbuf3=GWEN_Buffer_new(0, 256, 0, 1);
      GWEN_Text_UnescapeToBufferTolerant(GWEN_Buffer_GetStart(xbuf), sbuf1);
      if (t) {
	GWEN_Text_UnescapeToBufferTolerant(t, sbuf2);
	t=GWEN_Buffer_GetStart(sbuf2);
	if (t2) {
	  GWEN_Text_UnescapeToBufferTolerant(t2, sbuf3);
	}
      }
      APY_User_SetApiSecrets_l(u, GWEN_Buffer_GetStart(sbuf1), GWEN_Buffer_GetStart(sbuf2), GWEN_Buffer_GetStart(sbuf3));
      GWEN_Buffer_free(xbuf);
      GWEN_Buffer_free(sbuf3);
      GWEN_Buffer_free(sbuf2);
      GWEN_Buffer_free(sbuf1);
    }

    aq=AB_AccountQueue_List_First(aql);
    while (aq) {
      int rv;

      rv=APY_Provider__SendAccountQueue(pro, u, aq, ctx);
      if (rv<0) {
        DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
      }

      aq=AB_AccountQueue_List_Next(aq);
    } /* while aq */

    /* erase secrets */
    APY_User_SetApiSecrets_l(u, NULL, NULL, NULL);

    GWEN_Gui_ProgressLog2(0, GWEN_LoggerLevel_Info, I18N("Unlocking customer \"%lu\""),
                          (unsigned long int) AB_User_GetUniqueId(u));
    rv=AB_Provider_EndExclUseUser(pro, u, 0);
    if (rv<0) {
      GWEN_Gui_ProgressLog2(0, GWEN_LoggerLevel_Error, I18N("Could not unlock user \"%lu\""),
                            (unsigned long int) AB_User_GetUniqueId(u));
      AB_Provider_EndExclUseUser(pro, u, 1); /* abandon */
      return rv;
    }
  }

  return 0;
}


int APY_Provider_SendCommands(AB_PROVIDER *pro, AB_PROVIDERQUEUE *pq, AB_IMEXPORTER_CONTEXT *ctx)
{
  APY_PROVIDER *hp;
  AB_USERQUEUE_LIST *uql;
  AB_USERQUEUE *uq;
  int rv;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, APY_PROVIDER, pro);
  assert(hp);

  /* sort into user queue list */
  uql=AB_UserQueue_List_new();
  rv=AB_Provider_SortProviderQueueIntoUserQueueList(pro, pq, uql);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    AB_Provider_FreeUsersAndAccountsFromUserQueueList(pro, uql);
    AB_UserQueue_List_free(uql);
    return rv;
  }

  uq=AB_UserQueue_List_First(uql);
  while (uq) {
    int rv;

    rv=APY_Provider__SendUserQueue(pro, uq, ctx);
    if (rv<0) {
      DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    }
    uq=AB_UserQueue_List_Next(uq);
  }

  /* release accounts and users we loaded */
  AB_Provider_FreeUsersAndAccountsFromUserQueueList(pro, uql);

  return 0;
}



void APY_Provider__AddOrModifyAccount(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *acc)
{
  AB_ACCOUNT *storedAcc=NULL;

  assert(pro);
  assert(acc);

  if (AB_Account_GetUniqueId(acc)) {
    int rv;

    /* account already exists, needs update */
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Account exists, modifying");
    rv=AB_Provider_GetAccount(pro, AB_Account_GetUniqueId(acc), 1, 0, &storedAcc); /* lock, don't unlock */
    if (rv<0) {
      DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Error getting referred account (%d)", rv);
    }
    else {
      const char *s;

      /* account is locked now, apply changes */
      assert(storedAcc);

      s=AB_Account_GetCountry(acc);
      if (s && *s)
        AB_Account_SetCountry(storedAcc, s);

      s=AB_Account_GetBankCode(acc);
      if (s && *s)
        AB_Account_SetBankCode(storedAcc, s);

      s=AB_Account_GetBankName(acc);
      if (s && *s)
        AB_Account_SetBankName(storedAcc, s);

      s=AB_Account_GetAccountNumber(acc);
      if (s && *s)
        AB_Account_SetAccountNumber(storedAcc, s);

      s=AB_Account_GetSubAccountId(acc);
      if (s && *s)
        AB_Account_SetSubAccountId(storedAcc, s);

      s=AB_Account_GetIban(acc);
      if (s && *s)
        AB_Account_SetIban(storedAcc, s);

      s=AB_Account_GetBic(acc);
      if (s && *s)
        AB_Account_SetBic(storedAcc, s);

      s=AB_Account_GetOwnerName(acc);
      if (s && *s)
        AB_Account_SetOwnerName(storedAcc, s);

      s=AB_Account_GetCurrency(acc);
      if (s && *s)
        AB_Account_SetCurrency(storedAcc, s);

      AB_Account_SetAccountType(storedAcc, AB_Account_GetAccountType(acc));

      /* handle users */
      AB_Account_SetUserId(storedAcc, AB_User_GetUniqueId(u));

      /* update and write account spec */
      rv=AB_Provider_WriteAccountSpecForAccount(pro, storedAcc, 0); /* don't lock, account already is locked */
      if (rv<0) {
        DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
      }

      /* unlock account */
      rv=AB_Provider_EndExclUseAccount(pro, storedAcc, 0);
      if (rv<0) {
        DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
        AB_Provider_EndExclUseAccount(pro, acc, 1); /* abort */
      }
    }
  }
  else {
    int rv;

    /* account is new, add it */
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Account is new, adding");
    AB_Account_SetUserId(acc, AB_User_GetUniqueId(u));
    rv=AB_Provider_AddAccount(pro, acc, 1); /* do lock corresponding user */
    if (rv<0) {
      DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Coud not write new account (%d)", rv);
    }
    else {
      DBG_INFO(AQPAYPAL_LOGDOMAIN, "Reading back added account");
      rv=AB_Provider_GetAccount(pro, AB_Account_GetUniqueId(acc), 0, 0, &storedAcc); /* no-lock, no-unlock */
      if (rv<0) {
        DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Error getting referred account (%d)", rv);
      }
    }
  }

  /* done with stored account */
  AB_Account_free(storedAcc);
}



int APY_Provider__ProcessImporterContext(AB_PROVIDER *pro, AB_USER *u, AB_IMEXPORTER_CONTEXT *ictx)
{
  AB_IMEXPORTER_ACCOUNTINFO *ai;
  AB_BANKING *ab;

  assert(pro);
  assert(ictx);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  ai=AB_ImExporterContext_GetFirstAccountInfo(ictx);
  if (!ai) {
    DBG_INFO(0, "No accounts");
  }
  while (ai) {
    const char *country;
    const char *bankCode;
    const char *accountNumber;

    country=AB_User_GetCountry(u);
    if (!country)
      country="us";
    bankCode=AB_ImExporterAccountInfo_GetBankCode(ai);
    if (!bankCode || !*bankCode)
      bankCode=AB_User_GetBankCode(u);
    accountNumber=AB_ImExporterAccountInfo_GetAccountNumber(ai);
    if (bankCode && accountNumber) {
      AB_ACCOUNT_SPEC_LIST *accountSpecList=NULL;
      int rv;

      accountSpecList=AB_AccountSpec_List_new();
      rv=AB_Banking_GetAccountSpecList(ab, &accountSpecList);
      if (rv<0) {
        DBG_INFO(AQPAYPAL_LOGDOMAIN, "No account spec list");
      }
      else {
        AB_ACCOUNT *a;
        AB_ACCOUNT_SPEC *as;
        const char *s;

        /* create temporary account */
        a=APY_Account_new(pro);
        assert(a);
        AB_Account_SetCountry(a, country);
        AB_Account_SetBankCode(a, bankCode);
        AB_Account_SetAccountNumber(a, accountNumber);
        AB_Account_SetUserId(a, AB_User_GetUniqueId(u));
        s=AB_ImExporterAccountInfo_GetBankName(ai);
        if (!s)
          s=bankCode;
        AB_Account_SetBankName(a, s);
        AB_Account_SetAccountType(a, AB_ImExporterAccountInfo_GetAccountType(ai));

        as=AB_Provider_FindMatchingAccountSpec(pro, a, accountSpecList);
        if (as) {
          /* account already exists */
          DBG_INFO(AQPAYPAL_LOGDOMAIN, "Account %s at bank %s already exists", accountNumber, bankCode);
          AB_Account_SetUniqueId(a, AB_AccountSpec_GetUniqueId(as));
        }
        else {
          /* account is new, add it */
          DBG_INFO(AQPAYPAL_LOGDOMAIN, "Adding account %s to bank %s", accountNumber, bankCode);
          AB_Account_SetUniqueId(a, 0);
        }

        /* add or modify account */
        APY_Provider__AddOrModifyAccount(pro, u, a);

        /* delete temporary account */
        AB_Account_free(a);
      }
      AB_AccountSpec_List_free(accountSpecList);
    }
    else {
      DBG_WARN(AQPAYPAL_LOGDOMAIN,
               "BankCode or AccountNumber missing (%s/%s)",
               bankCode?bankCode:"<missing>", accountNumber?accountNumber:"<missing>");
    }
    ai=AB_ImExporterAccountInfo_List_Next(ai);
  } /* while accounts */

  return 0;
}


