/***************************************************************************
    begin       : Sat Dec 01 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


/* included from provider.c */



int APY_Provider_UpdateTrans(AB_PROVIDER *pro,
                             AB_USER *u,
                             AB_TRANSACTION *t)
{
  GWEN_HTTP_SESSION *sess;
  GWEN_BUFFER *tbuf;
  const char *s;
  int vmajor;
  int vminor;
  int rv;
  GWEN_DB_NODE *dbResponse;
  GWEN_DB_NODE *dbT;

  sess=AB_HttpSession_new(pro, u, APY_User_GetServerUrl(u), "https", 443);
  if (sess==NULL) {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Could not create http session for user [%s]",
              AB_User_GetUserId(u));
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
    return GWEN_ERROR_INVALID;
  }

  GWEN_Buffer_AppendString(tbuf, "&version=");
  GWEN_Text_EscapeToBuffer(AQPAYPAL_API_VER, tbuf);
  GWEN_Buffer_AppendString(tbuf, "&method=getTransactionDetails");

  GWEN_Buffer_AppendString(tbuf, "&transactionId=");
  s=AB_Transaction_GetFiId(t);
  if (s && *s)
    GWEN_Text_EscapeToBuffer(s, tbuf);
  else {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Missing transaction id");
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    return GWEN_ERROR_INVALID;
  }

  /* init session */
  rv=GWEN_HttpSession_Init(sess);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    return rv;
  }

  if (getenv("AQPAYPAL_LOG_COMM")) {
    int len;
    FILE *f;

    len=GWEN_Buffer_GetUsedBytes(tbuf);

    f=fopen("paypal.log", "a+");
    if (f) {
      fprintf(f, "\n============================================\n");
      fprintf(f, "Sending (UpdateTrans):\n");
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
    return rv;
  }

  /* get response */
  GWEN_Buffer_Reset(tbuf);
  rv=GWEN_HttpSession_RecvPacket(sess, tbuf);
  if (rv<0 || rv<200 || rv>299) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_HttpSession_Fini(sess);
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    return rv;
  }

  if (getenv("AQPAYPAL_LOG_COMM")) {
    int len;
    FILE *f;

    len=GWEN_Buffer_GetUsedBytes(tbuf);

    f=fopen("paypal.log", "a+");
    if (f) {
      fprintf(f, "\n============================================\n");
      fprintf(f, "Received (UpdateTrans):\n");
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

#if 0
  if (getenv("AQPAYPAL_LOG_COMM")) {
    static int debugCounter=0;
    char namebuf[64];

    snprintf(namebuf, sizeof(namebuf)-1, "paypal-%02x.db", debugCounter++);
    GWEN_DB_WriteFile(dbResponse, namebuf, GWEN_DB_FLAGS_DEFAULT);
  }
#endif

  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(dbResponse);
    GWEN_Buffer_free(tbuf);
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
      return GWEN_ERROR_BAD_DATA;
    }
  }
  else {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "No ACK response from server");
    GWEN_DB_Group_free(dbResponse);
    GWEN_Buffer_free(tbuf);
    return GWEN_ERROR_BAD_DATA;
  }


  /* parse response */
  s=GWEN_DB_GetCharValue(dbResponse, "TRANSACTIONTYPE", 0, NULL);
  if (s && *s)
    AB_Transaction_SetTransactionText(t, s);
  /* address */
  s=GWEN_DB_GetCharValue(dbResponse, "SHIPTOSTREET", 0, NULL);
  if (s && *s)
    AB_Transaction_SetRemoteAddrStreet(t, s);
  s=GWEN_DB_GetCharValue(dbResponse, "SHIPTOCITY", 0, NULL);
  if (s && *s)
    AB_Transaction_SetRemoteAddrCity(t, s);
  s=GWEN_DB_GetCharValue(dbResponse, "SHIPTOZIP", 0, NULL);
  if (s && *s)
    AB_Transaction_SetRemoteAddrZipcode(t, s);

  s=GWEN_DB_GetCharValue(dbResponse, "PAYMENTSTATUS", 0, NULL);
  if (s && *s) {
    if (strcasecmp(s, "Completed")==0)
      AB_Transaction_SetStatus(t, AB_Transaction_StatusAccepted);
    else if (strcasecmp(s, "Denied")==0 ||
             strcasecmp(s, "Failed")==0 ||
             strcasecmp(s, "Expired")==0 ||
             strcasecmp(s, "Voided")==0)
      AB_Transaction_SetStatus(t, AB_Transaction_StatusRejected);
    else if (strcasecmp(s, "Pending")==0 ||
             strcasecmp(s, "Processed")==0)
      AB_Transaction_SetStatus(t, AB_Transaction_StatusPending);
    else if (strcasecmp(s, "Refunded")==0 ||
             strcasecmp(s, "Reversed")==0)
      AB_Transaction_SetStatus(t, AB_Transaction_StatusRevoked);
    else {
      DBG_INFO(AQPAYPAL_LOGDOMAIN, "Unknown payment status (%s)", s);
    }
  }

  s=GWEN_DB_GetCharValue(dbResponse, "BUYERID", 0, NULL);
  if (s && *s)
    AB_Transaction_SetBankReference(t, s);

  dbT=GWEN_DB_GetFirstGroup(dbResponse);
  while (dbT) {
    GWEN_BUFFER *pbuf;

    pbuf=GWEN_Buffer_new(0, 256, 0, 1);
    s=GWEN_DB_GetCharValue(dbT, "L_QTY", 0, NULL);
    if (s && *s) {
      GWEN_Buffer_AppendString(pbuf, s);
      GWEN_Buffer_AppendString(pbuf, "x");
    }
    s=GWEN_DB_GetCharValue(dbT, "L_NAME", 0, NULL);
    if (s && *s) {
      GWEN_Buffer_AppendString(pbuf, s);
      s=GWEN_DB_GetCharValue(dbT, "L_NUMBER", 0, NULL);
      if (s && *s) {
        GWEN_Buffer_AppendString(pbuf, "(");
        GWEN_Buffer_AppendString(pbuf, s);
        GWEN_Buffer_AppendString(pbuf, ")");
      }
    }
    else {
      s=GWEN_DB_GetCharValue(dbT, "L_NUMBER", 0, NULL);
      if (s && *s)
        GWEN_Buffer_AppendString(pbuf, s);
    }

    s=GWEN_DB_GetCharValue(dbT, "L_AMT", 0, NULL);
    if (s && *s) {
      GWEN_Buffer_AppendString(pbuf, "[");
      GWEN_Buffer_AppendString(pbuf, s);
      s=GWEN_DB_GetCharValue(dbT, "L_CURRENCYCODE", 0, NULL);
      if (s && *s) {
        GWEN_Buffer_AppendString(pbuf, " ");
        GWEN_Buffer_AppendString(pbuf, s);
      }
      GWEN_Buffer_AppendString(pbuf, "]");

    }

    AB_Transaction_AddPurposeLine(t, GWEN_Buffer_GetStart(pbuf));
    GWEN_Buffer_free(pbuf);

    dbT=GWEN_DB_GetNextGroup(dbT);
  }

  GWEN_DB_Group_free(dbResponse);
  GWEN_Buffer_free(tbuf);


  return 0;
}



int APY_Provider_ExecGetTrans(AB_PROVIDER *pro,
                              AB_IMEXPORTER_ACCOUNTINFO *ai,
                              AB_USER *u,
                              AB_TRANSACTION *j)
{
  GWEN_HTTP_SESSION *sess;
  GWEN_BUFFER *tbuf;
  const char *s;
  const GWEN_DATE *da;
  int vmajor;
  int vminor;
  int rv;
  GWEN_DB_NODE *dbResponse;
  GWEN_DB_NODE *dbT;

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

  GWEN_Buffer_AppendString(tbuf, "&method=transactionSearch");

  da=AB_Transaction_GetFirstDate(j);
  if (da==NULL) {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Missing start date");
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    AB_Transaction_SetStatus(j, AB_Transaction_StatusError);
    return GWEN_ERROR_INVALID;
  }

  GWEN_Buffer_AppendString(tbuf, "&startdate=");
  GWEN_Date_toStringWithTemplate(da, "YYYY-MM-DDT00:00:00Z", tbuf);
  //testing: GWEN_Buffer_AppendString(tbuf, "&enddate=2016-01-01T00:00:00Z");

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
      fprintf(f, "Sending (GetTrans):\n");
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


#if 0
  DBG_ERROR(0, "Would send this: [%s]", GWEN_Buffer_GetStart(tbuf));
  GWEN_HttpSession_Fini(sess);
  GWEN_Buffer_free(tbuf);
  GWEN_HttpSession_free(sess);
  AB_Transaction_SetStatus(j, AB_Transaction_StatusError);
  return GWEN_ERROR_INTERNAL;
#endif

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
      fprintf(f, "Received (GetTrans):\n");
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
  dbT=GWEN_DB_GetFirstGroup(dbResponse);
  while (dbT) {
    AB_TRANSACTION *t;
    int dontKeep;

    dontKeep=0;

    t=AB_Transaction_new();
    s=GWEN_DB_GetCharValue(dbT, "L_TIMESTAMP", 0, NULL);
    if (s && *s) {
      GWEN_DATE *da;

      da=GWEN_Date_fromStringWithTemplate(s, "YYYY-MM-DD");
      if (da) {
        AB_Transaction_SetDate(t, da);
        AB_Transaction_SetValutaDate(t, da);
        GWEN_Date_free(da);
      }
      else {
        DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Invalid timespec [%s]", s);
      }
    }

    s=GWEN_DB_GetCharValue(dbT, "L_TYPE", 0, NULL);
    if (s && *s) {
      // reverse engineered rule which transactions to keep and which not
      if (strcasecmp(s, "Authorization")==0 || strcasecmp(s, "Order")==0)
        dontKeep++;

      /* TODO: maybe handle those types differently? */
      if (strcasecmp(s, "Transfer")==0) {
        AB_Transaction_SetType(t, AB_Transaction_TypeStatement);
        AB_Transaction_SetSubType(t, AB_Transaction_SubTypeStandard);
      }
      else if (strcasecmp(s, "Payment")==0) {
        AB_Transaction_SetType(t, AB_Transaction_TypeStatement);
        AB_Transaction_SetSubType(t, AB_Transaction_SubTypeStandard);
      }
      else {
        AB_Transaction_SetType(t, AB_Transaction_TypeStatement);
        AB_Transaction_SetSubType(t, AB_Transaction_SubTypeStandard);
      }
    }

    s=GWEN_DB_GetCharValue(dbT, "L_NAME", 0, NULL);
    if (s && *s)
      AB_Transaction_SetRemoteName(t, s);

    s=GWEN_DB_GetCharValue(dbT, "L_TRANSACTIONID", 0, NULL);
    if (s && *s)
      AB_Transaction_SetFiId(t, s);

    s=GWEN_DB_GetCharValue(dbT, "L_AMT", 0, NULL);
    if (s && *s) {
      AB_VALUE *v;

      v=AB_Value_fromString(s);
      if (v==NULL) {
        DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Invalid amount [%s]", s);
      }
      else {
        s=GWEN_DB_GetCharValue(dbT, "L_CURRENCYCODE", 0, NULL);
        if (s && *s)
          AB_Value_SetCurrency(v, s);
        AB_Transaction_SetValue(t, v);
        AB_Value_free(v);
      }
    }
    else {
      DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Missing amount");
    }

    s=GWEN_DB_GetCharValue(dbT, "L_FEEAMT", 0, NULL);
    if (s && *s) {
      AB_VALUE *v;

      v=AB_Value_fromString(s);
      if (v==NULL) {
        DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Invalid fee amount [%s]", s);
      }
      else {
        s=GWEN_DB_GetCharValue(dbT, "L_CURRENCYCODE", 0, NULL);
        if (s && *s)
          AB_Value_SetCurrency(v, s);
        AB_Transaction_SetFees(t, v);
        AB_Value_free(v);
      }
    }

    s=GWEN_DB_GetCharValue(dbT, "L_STATUS", 0, NULL);
    if (s && *s) {
      // reverse engineered rule which transactions to keep and which not
      if (strcasecmp(s, "Placed")==0 || strcasecmp(s, "Removed")==0)
        dontKeep++;

      if (strcasecmp(s, "Completed")==0)
        AB_Transaction_SetStatus(t, AB_Transaction_StatusAccepted);
      else
        AB_Transaction_SetStatus(t, AB_Transaction_StatusPending);
    }

    /* get transaction details */
    s=AB_Transaction_GetFiId(t);
    if (s && *s) {
      const char *s2;

      s2=GWEN_DB_GetCharValue(dbT, "L_TYPE", 0, NULL);
      if (s2 && *s2) {
        /* only get details for payments (maybe add other types later) */
        if (strcasecmp(s2, "Payment")==0 ||
            strcasecmp(s2, "Purchase")==0 ||
            strcasecmp(s2, "Donation")==0) {
          DBG_INFO(AQPAYPAL_LOGDOMAIN, "Getting details for transaction [%s]", s);
          rv=APY_Provider_UpdateTrans(pro, u, t);
          if (rv<0) {
            DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
          }
        }
      }
    }

    /* add transaction */
    /* but only if L_TYPE neither Authorization nor Order */
    s=GWEN_DB_GetCharValue(dbT, "L_TYPE", 0, NULL);
    if (s && *s) {
      if (!dontKeep)
        AB_ImExporterAccountInfo_AddTransaction(ai, t);
    }

    dbT=GWEN_DB_GetNextGroup(dbT);
  }

  GWEN_DB_Group_free(dbResponse);
  GWEN_Buffer_free(tbuf);
  AB_Transaction_SetStatus(j, AB_Transaction_StatusAccepted);
  return 0;
}




