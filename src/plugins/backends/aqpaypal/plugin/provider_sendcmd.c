/***************************************************************************
    begin       : Sat Dec 01 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


/* included from provider.c */



int APY_Provider_ExecJobQueue(AB_PROVIDER *pro,
                              AB_IMEXPORTER_ACCOUNTINFO *ai,
			      AB_USER *u,
			      AB_ACCOUNT *a,
			      AB_JOBQUEUE *jq) {
  APY_PROVIDER *xp;
  AB_JOB_LIST2_ITERATOR *it;
  int errors=0;
  int oks=0;

  assert(pro);
  xp=GWEN_INHERIT_GETDATA(AB_PROVIDER, APY_PROVIDER, pro);
  assert(xp);

  it=AB_Job_List2_First(AB_JobQueue_GetJobList(jq));
  if (it) {
    AB_JOB *j;

    j=AB_Job_List2Iterator_Data(it);
    while(j) {
      int rv=0;

      switch(AB_Job_GetType(j)) {
      case AB_Job_TypeGetTransactions:
	rv=APY_Provider_ExecGetTrans(pro, ai, u, j);
	break;
      case AB_Job_TypeGetBalance:
	rv=APY_Provider_ExecGetBal(pro, ai, u, j);
	break;
      case AB_Job_TypeTransfer:
      case AB_Job_TypeDebitNote:
      default:
	DBG_INFO(AQPAYPAL_LOGDOMAIN,
		 "Job not supported (%d)",
		 AB_Job_GetType(j));
	rv=GWEN_ERROR_NOT_SUPPORTED;
      } /* switch */

      if (rv<0) {
	errors++;
      }
      else
	oks++;

      j=AB_Job_List2Iterator_Next(it);
    }

    AB_Job_List2Iterator_free(it);
  }

  if (errors) {
    if (oks) {
      DBG_WARN(AQPAYPAL_LOGDOMAIN, "Some jobs failed");
    }
    else {
      DBG_ERROR(AQPAYPAL_LOGDOMAIN, "All jobs failed");
      return GWEN_ERROR_GENERIC;
    }
  }

  return 0;
}




int APY_Provider_ExecAccountQueue(AB_PROVIDER *pro,
				  AB_IMEXPORTER_CONTEXT *ctx,
				  AB_USER *u,
				  AB_ACCOUNTQUEUE *aq) {
  APY_PROVIDER *xp;
  AB_JOBQUEUE *jq;
  AB_ACCOUNT *a;
  int errors=0;
  int oks=0;
  AB_IMEXPORTER_ACCOUNTINFO *ai;

  assert(pro);
  xp=GWEN_INHERIT_GETDATA(AB_PROVIDER, APY_PROVIDER, pro);
  assert(xp);

  a=AB_AccountQueue_GetAccount(aq);
  assert(a);

  ai=AB_ImExporterContext_GetOrAddAccountInfo(ctx,
                                              AB_Account_GetUniqueId(a),
                                              AB_Account_GetIBAN(a),
                                              AB_Account_GetBankCode(a),
                                              AB_Account_GetAccountNumber(a),
                                              AB_Transaction_TypeNone);
  if (ai==NULL) {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Could not create account info");
    return GWEN_ERROR_GENERIC;
  }

  jq=AB_JobQueue_List_First(AB_AccountQueue_GetJobQueueList(aq));
  while(jq) {
    int rv;

    rv=APY_Provider_ExecJobQueue(pro, ai, u, a, jq);
    if (rv<0)
      errors++;
    else
      oks++;
    jq=AB_JobQueue_List_Next(jq);
  }

  if (errors) {
    if (oks) {
      DBG_WARN(AQPAYPAL_LOGDOMAIN, "Some jobs failed");
    }
    else {
      DBG_ERROR(AQPAYPAL_LOGDOMAIN, "All jobs failed");
      return GWEN_ERROR_GENERIC;
    }
  }

  return 0;
}



int APY_Provider_ExecUserQueue(AB_PROVIDER *pro,
			       AB_IMEXPORTER_CONTEXT *ctx,
			       AB_USERQUEUE *uq) {
  APY_PROVIDER *xp;
  AB_ACCOUNTQUEUE *aq;
  AB_USER *u;
  int errors=0;
  int oks=0;
  GWEN_BUFFER *xbuf;
  int rv;

  assert(pro);
  xp=GWEN_INHERIT_GETDATA(AB_PROVIDER, APY_PROVIDER, pro);
  assert(xp);

  u=AB_UserQueue_GetUser(uq);
  assert(u);

  /* lock user */
  rv=AB_Banking_BeginExclUseUser(AB_Provider_GetBanking(pro), u);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
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

  aq=AB_AccountQueue_List_First(AB_UserQueue_GetAccountQueueList(uq));
  while(aq) {
    int rv;

    rv=APY_Provider_ExecAccountQueue(pro, ctx, u, aq);
    if (rv<0)
      errors++;
    else
      oks++;
    aq=AB_AccountQueue_List_Next(aq);
  }

  /* erase secrets */
  APY_User_SetApiSecrets_l(u, NULL, NULL, NULL);

  /* unlock user */
  rv=AB_Banking_EndExclUseUser(AB_Provider_GetBanking(pro), u, 0);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    AB_Banking_EndExclUseUser(AB_Provider_GetBanking(pro), u, 1);
    return rv;
  }

  if (errors) {
    if (oks) {
      DBG_WARN(AQPAYPAL_LOGDOMAIN, "Some jobs failed");
    }
    else {
      DBG_ERROR(AQPAYPAL_LOGDOMAIN, "All jobs failed");
      return GWEN_ERROR_GENERIC;
    }
  }

  return 0;
}



int APY_Provider_Execute(AB_PROVIDER *pro, AB_IMEXPORTER_CONTEXT *ctx){
  APY_PROVIDER *xp;
  AB_USERQUEUE *uq;
  int errors=0;
  int oks=0;

  assert(pro);
  xp=GWEN_INHERIT_GETDATA(AB_PROVIDER, APY_PROVIDER, pro);
  assert(xp);

  if (xp->queue==NULL) {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Empty queue");
    return GWEN_ERROR_NOT_FOUND;
  }

  uq=AB_UserQueue_List_First(AB_Queue_GetUserQueueList(xp->queue));
  while(uq) {
    int rv;

    rv=APY_Provider_ExecUserQueue(pro, ctx, uq);
    if (rv<0)
      errors++;
    else
      oks++;
    uq=AB_UserQueue_List_Next(uq);
  }

  if (errors) {
    if (oks) {
      DBG_WARN(AQPAYPAL_LOGDOMAIN, "Some jobs failed");
    }
    else {
      DBG_ERROR(AQPAYPAL_LOGDOMAIN, "All jobs failed");
      return GWEN_ERROR_GENERIC;
    }
  }

  return 0;
}



