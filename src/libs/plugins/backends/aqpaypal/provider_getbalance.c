/***************************************************************************
    begin       : Sat Dec 01 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


/* included from provider.c */




int APY_Provider_ExecGetBal(AB_PROVIDER *pro,
                            AB_IMEXPORTER_ACCOUNTINFO *ai,
                            AB_USER *u,
                            AB_TRANSACTION *j)
{
  GWEN_HTTP_SESSION *sess;
  GWEN_BUFFER *tbuf;
  const char *s;
  int vmajor;
  int vminor;
  int rv;
  GWEN_DB_NODE *dbResponse;
  GWEN_DB_NODE *dbCurr;

  sess=AB_HttpSession_new(pro, u, APY_User_GetServerUrl(u), "https", 443);
  if (sess==NULL) {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Could not create http session for user [%s]", AB_User_GetUserId(u));
    AB_Transaction_SetStatus(j, AB_Transaction_StatusError);
    return GWEN_ERROR_GENERIC;
  }

  vmajor=APY_User_GetHttpVMajor(u);
  vminor=APY_User_GetHttpVMinor(u);
  if (vmajor==0 && vminor==0) {
    vmajor=1;
    vminor=0;
  }
  GWEN_HttpSession_SetHttpVMajor(sess, vmajor);
  GWEN_HttpSession_SetHttpVMinor(sess, vminor);
  GWEN_HttpSession_SetHttpContentType(sess, "application/x-www-form-urlencoded");

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  GWEN_Buffer_AppendString(tbuf, "user=");
  s=APY_User_GetApiUserId(u);
  if (s && *s)
    GWEN_Text_EscapeToBuffer(s, tbuf);
  else {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Missing user id");
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    AB_Transaction_SetStatus(j, AB_Transaction_StatusError);
    return GWEN_ERROR_INVALID;
  }

  GWEN_Buffer_AppendString(tbuf, "&pwd=");
  s=APY_User_GetApiPassword(u);
  if (s && *s)
    GWEN_Text_EscapeToBuffer(s, tbuf);
  else {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Missing API password");
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    AB_Transaction_SetStatus(j, AB_Transaction_StatusError);
    return GWEN_ERROR_INVALID;
  }

  GWEN_Buffer_AppendString(tbuf, "&signature=");
  s=APY_User_GetApiSignature(u);
  if (s && *s)
    GWEN_Text_EscapeToBuffer(s, tbuf);
  else {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Missing API signature");
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    AB_Transaction_SetStatus(j, AB_Transaction_StatusError);
    return GWEN_ERROR_INVALID;
  }

  GWEN_Buffer_AppendString(tbuf, "&version=");
  GWEN_Text_EscapeToBuffer(AQPAYPAL_API_VER, tbuf);

  GWEN_Buffer_AppendString(tbuf, "&method=GetBalance");

  /* init session */
  rv=GWEN_HttpSession_Init(sess);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    AB_Transaction_SetStatus(j, AB_Transaction_StatusError);
    return rv;
  }

  if (getenv("AQPAYPAL_LOG_COMM")) {
    int len;
    FILE *f;

    len=GWEN_Buffer_GetUsedBytes(tbuf);

    f=fopen("paypal.log", "a+");
    if (f) {
      fprintf(f, "\n============================================\n");
      fprintf(f, "Sending (GetBal):\n");
      if (len>0) {
        if (1!=fwrite(GWEN_Buffer_GetStart(tbuf), GWEN_Buffer_GetUsedBytes(tbuf), 1, f)) {
          DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d: %s)", errno, strerror(errno));
          fclose(f);
        }
        else {
          if (fclose(f)) {
            DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d: %s)", errno, strerror(errno));
          }
        }
      }
      else {
        fprintf(f, "Empty data.\n");
        if (fclose(f)) {
          DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d: %s)", errno, strerror(errno));
        }
      }
    }
  }


  /* send request */
  rv=GWEN_HttpSession_SendPacket(sess, "POST",
                                 (const uint8_t *) GWEN_Buffer_GetStart(tbuf),
                                 GWEN_Buffer_GetUsedBytes(tbuf));
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_HttpSession_Fini(sess);
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    AB_Transaction_SetStatus(j, AB_Transaction_StatusError);
    return rv;
  }
  AB_Transaction_SetStatus(j, AB_Transaction_StatusSending);

  /* get response */
  GWEN_Buffer_Reset(tbuf);
  rv=GWEN_HttpSession_RecvPacket(sess, tbuf);
  if (rv<0 || rv<200 || rv>299) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_HttpSession_Fini(sess);
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    AB_Transaction_SetStatus(j, AB_Transaction_StatusError);
    return rv;
  }

  if (getenv("AQPAYPAL_LOG_COMM")) {
    int len;
    FILE *f;

    len=GWEN_Buffer_GetUsedBytes(tbuf);
    f=fopen("paypal.log", "a+");
    if (f) {
      fprintf(f, "\n============================================\n");
      fprintf(f, "Received (GetBal):\n");
      if (len>0) {
        if (1!=fwrite(GWEN_Buffer_GetStart(tbuf), GWEN_Buffer_GetUsedBytes(tbuf), 1, f)) {
          DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d: %s)", errno, strerror(errno));
          fclose(f);
        }
        else {
          if (fclose(f)) {
            DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d: %s)", errno, strerror(errno));
          }
        }
      }
      else {
        fprintf(f, "Empty data.\n");
        if (fclose(f)) {
          DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d: %s)", errno, strerror(errno));
        }
      }
    }
  }


  /* deinit (ignore result because it isn't important) */
  GWEN_HttpSession_Fini(sess);
  GWEN_HttpSession_free(sess);

  /* parse response */
  dbResponse=GWEN_DB_Group_new("response");
  rv=APY_Provider_ParseResponse(pro, GWEN_Buffer_GetStart(tbuf), dbResponse);
#ifdef DEBUG_PAYPAL
  GWEN_DB_WriteFile(dbResponse, "paypal.db", GWEN_DB_FLAGS_DEFAULT);
#endif
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(dbResponse);
    GWEN_Buffer_free(tbuf);
    AB_Transaction_SetStatus(j, AB_Transaction_StatusError);
    return rv;
  }

  /* check result */
  s=GWEN_DB_GetCharValue(dbResponse, "ACK", 0, NULL);
  if (s && *s) {
    if (strcasecmp(s, "Success")==0 ||
        strcasecmp(s, "SuccessWithWarning")==0) {
      DBG_INFO(AQPAYPAL_LOGDOMAIN, "Success");
    }
    else {
      DBG_INFO(AQPAYPAL_LOGDOMAIN, "No positive response from server");
      GWEN_DB_Group_free(dbResponse);
      GWEN_Buffer_free(tbuf);
      AB_Transaction_SetStatus(j, AB_Transaction_StatusError);
      return GWEN_ERROR_BAD_DATA;
    }
  }
  else {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "No ACK response from server");
    GWEN_DB_Group_free(dbResponse);
    GWEN_Buffer_free(tbuf);
    AB_Transaction_SetStatus(j, AB_Transaction_StatusError);
    return GWEN_ERROR_BAD_DATA;
  }

  /* now get the transactions */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponse);
  while (dbCurr) {
    AB_BALANCE *bal;
    GWEN_DATE *t=NULL;
    AB_VALUE *vc;
    const char *p;

    DBG_NOTICE(AQPAYPAL_LOGDOMAIN, "Got a balance");

    /* read and parse value */
    p=GWEN_DB_GetCharValue(dbCurr, "L_AMT", 0, 0);
    if (!p)
      return GWEN_ERROR_BAD_DATA;
    vc=AB_Value_fromString(p);
    if (vc==NULL)
      return GWEN_ERROR_BAD_DATA;

    /* read currency (if any) */
    p=GWEN_DB_GetCharValue(dbCurr, "L_CURRENCYCODE", 0, "EUR");
    if (p)
      AB_Value_SetCurrency(vc, p);

    p=GWEN_DB_GetCharValue(dbResponse, "TIMESTAMP", 0, NULL);
    if (p && *p) {
      /*t=GWEN_Time_fromUtcString(p, "YYYY-MM-DDThh:mm:ssZ");*/
      t=GWEN_Date_fromStringWithTemplate(p, "YYYY-MM-DD");
      if (t==NULL) {
        DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Invalid timespec [%s]", p);
      }
    }
    else {
      DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Missing timespec");
    }

    bal=AB_Balance_new();
    AB_Balance_SetType(bal, AB_Balance_TypeBooked);
    AB_Balance_SetDate(bal, t);
    AB_Balance_SetValue(bal, vc);

    AB_Value_free(vc);
    GWEN_Date_free(t);

    /* add new balance */
    AB_ImExporterAccountInfo_AddBalance(ai, bal);
    break; /* break loop, we found the balance */

    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }

  GWEN_DB_Group_free(dbResponse);
  GWEN_Buffer_free(tbuf);
  AB_Transaction_SetStatus(j, AB_Transaction_StatusAccepted);
  return 0;
}

