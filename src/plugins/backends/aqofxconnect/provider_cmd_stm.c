/***************************************************************************
    begin       : Thu Nov 29 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


/* included by provider.c */



int AO_Provider__AddBankStatementReq(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *j, GWEN_BUFFER *buf) {
  const char *s;
  int rv;

  assert(a);
  assert(u);

  GWEN_Buffer_AppendString(buf, "<STMTRQ>");
  GWEN_Buffer_AppendString(buf, "<BANKACCTFROM>");
  if (!(AO_User_GetFlags(u) & AO_USER_FLAGS_EMPTY_BANKID)) {
    /* only copy bank code if not forbidden by user */
    s=AB_Account_GetBankCode(a);
    if (s) {
      GWEN_Buffer_AppendString(buf, "<BANKID>");
      GWEN_Buffer_AppendString(buf, s);
    }
  }

  s=AB_Account_GetAccountNumber(a);
  if (s) {
    GWEN_Buffer_AppendString(buf, "<ACCTID>");
    GWEN_Buffer_AppendString(buf, s);
  }

  /* add account type */
  GWEN_Buffer_AppendString(buf, "<ACCTTYPE>");
  switch(AB_Account_GetAccountType(a)) {
  case AB_AccountType_Checking:
    GWEN_Buffer_AppendString(buf, "CHECKING");
    break;
  case AB_AccountType_Savings:
    GWEN_Buffer_AppendString(buf, "SAVINGS");
    break;
  case AB_AccountType_Bank:
    GWEN_Buffer_AppendString(buf, "CREDITLINE");
    break;
  case AB_AccountType_MoneyMarket:
    GWEN_Buffer_AppendString(buf, "MONEYMRKT");
    break;
  case AB_AccountType_CreditCard:
  case AB_AccountType_Investment:
  case AB_AccountType_Cash:
  case AB_AccountType_Unknown:
  default:
    GWEN_Buffer_AppendString(buf, "CHECKING");
    break;
  }
  GWEN_Buffer_AppendString(buf, "</BANKACCTFROM>");

  /* add INCTRAN element */
  GWEN_Buffer_AppendString(buf, "<INCTRAN>");
  if (AB_Transaction_GetCommand(j)==AB_Transaction_CommandGetTransactions) {
    const GWEN_DATE *da;

    da=AB_Transaction_GetFirstDate(j);
    if (da) {
      GWEN_Buffer_AppendString(buf, "<DTSTART>");
      if (AO_User_GetFlags(u) & AO_USER_FLAGS_SEND_SHORT_DATE)
	GWEN_Date_toStringWithTemplate(da, "YYYYMMDD000000", buf);
      else
	GWEN_Date_toStringWithTemplate(da, "YYYYMMDD000000.000", buf);
    }

    da=AB_Transaction_GetLastDate(j);
    if (da) {
      GWEN_Buffer_AppendString(buf, "<DTEND>");
      if (AO_User_GetFlags(u) & AO_USER_FLAGS_SEND_SHORT_DATE)
        GWEN_Date_toStringWithTemplate(da, "YYYYMMDD000000", buf);
      else
        GWEN_Date_toStringWithTemplate(da, "YYYYMMDD000000.000", buf);
    }
    GWEN_Buffer_AppendString(buf, "<INCLUDE>Y");
  }
  else {
    GWEN_Buffer_AppendString(buf, "<INCLUDE>N");
  }
  GWEN_Buffer_AppendString(buf, "</INCTRAN>");

  GWEN_Buffer_AppendString(buf, "</STMTRQ>");

  /* wrap into request */
  rv=AO_Provider__WrapRequest(pro, u, "BANK", "STMT", buf);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AO_Provider__AddCreditCardStatementReq(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *j, GWEN_BUFFER *buf) {
  const char *s;
  int rv;

  assert(a);
  assert(u);

  GWEN_Buffer_AppendString(buf, "<CCSTMTRQ>");
  GWEN_Buffer_AppendString(buf, "<CCACCTFROM>");
  s=AB_Account_GetAccountNumber(a);
  if (s) {
    GWEN_Buffer_AppendString(buf, "<ACCTID>");
    GWEN_Buffer_AppendString(buf, s);
  }
  GWEN_Buffer_AppendString(buf, "</CCACCTFROM>");

  /* add INCTRAN element */
  GWEN_Buffer_AppendString(buf, "<INCTRAN>");
  if (AB_Transaction_GetCommand(j)==AB_Transaction_CommandGetTransactions) {
    const GWEN_DATE *da;

    da=AB_Transaction_GetFirstDate(j);
    if (da) {
      GWEN_Buffer_AppendString(buf, "<DTSTART>");
      GWEN_Date_toStringWithTemplate(da, "YYYYMMDD000000", buf);
    }

    da=AB_Transaction_GetLastDate(j);
    if (da) {
      GWEN_Buffer_AppendString(buf, "<DTEND>");
      GWEN_Date_toStringWithTemplate(da, "YYYYMMDD000000", buf);
    }
    GWEN_Buffer_AppendString(buf, "<INCLUDE>Y");
  }
  else {
    GWEN_Buffer_AppendString(buf, "<INCLUDE>N");
  }
  GWEN_Buffer_AppendString(buf, "</INCTRAN>");

  GWEN_Buffer_AppendString(buf, "</CCSTMTRQ>");

  /* wrap into request */
  rv=AO_Provider__WrapRequest(pro, u, "CREDITCARD", "CCSTMT", buf);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AO_Provider__AddInvStatementReq(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *j, GWEN_BUFFER *buf) {
  const char *s;
  int rv;

  assert(a);
  assert(u);

  GWEN_Buffer_AppendString(buf, "<INVSTMTRQ>");
  GWEN_Buffer_AppendString(buf, "<INVACCTFROM>");
  s=AO_User_GetBrokerId(u);
  if (s) {
    GWEN_Buffer_AppendString(buf, "<BROKERID>");
    GWEN_Buffer_AppendString(buf, s);
  }
  s=AB_Account_GetAccountNumber(a);
  if (s) {
    GWEN_Buffer_AppendString(buf, "<ACCTID>");
    GWEN_Buffer_AppendString(buf, s);
  }
  GWEN_Buffer_AppendString(buf, "</INVACCTFROM>");

  /* add INCTRAN element */
  GWEN_Buffer_AppendString(buf, "<INCTRAN>");
  if (AB_Transaction_GetCommand(j)==AB_Transaction_CommandGetTransactions) {
    const GWEN_DATE *da;

    da=AB_Transaction_GetFirstDate(j);
    if (da) {
      GWEN_Buffer_AppendString(buf, "<DTSTART>");
      GWEN_Date_toStringWithTemplate(da, "YYYYMMDD", buf);
    }

    da=AB_Transaction_GetLastDate(j);
    if (da) {
      GWEN_Buffer_AppendString(buf, "<DTEND>");
      GWEN_Date_toStringWithTemplate(da, "YYYYMMDD", buf);
    }
    GWEN_Buffer_AppendString(buf, "<INCLUDE>Y");
  }
  else {
    GWEN_Buffer_AppendString(buf, "<INCLUDE>N");
  }
  GWEN_Buffer_AppendString(buf, "</INCTRAN>");

  GWEN_Buffer_AppendString(buf, "<INCOO>Y");

  GWEN_Buffer_AppendString(buf, "<INCPOS>");
  if (AB_Transaction_GetCommand(j)==AB_Transaction_CommandGetTransactions) {
    GWEN_TIME *ti;

    ti=GWEN_CurrentTime();
    if (ti) {
      GWEN_Buffer_AppendString(buf, "<DTASOF>");
      GWEN_Time_toString(ti, "YYYYMMDDhhmmss.000", buf);
    }
    GWEN_Time_free(ti);
    GWEN_Buffer_AppendString(buf, "<INCLUDE>Y");
  }
  GWEN_Buffer_AppendString(buf, "</INCPOS>");

  GWEN_Buffer_AppendString(buf, "<INCBAL>Y");

  GWEN_Buffer_AppendString(buf, "</INVSTMTRQ>");

  /* wrap into request */
  rv=AO_Provider__WrapRequest(pro, u, "INVSTMT", "INVSTMT", buf);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AO_Provider__AddStatementRequest(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *j, GWEN_BUFFER *buf) {
  int rv;

  assert(a);

  switch(AB_Account_GetAccountType(a)) {
  case AB_AccountType_CreditCard:
    rv=AO_Provider__AddCreditCardStatementReq(pro, u, a, j, buf);
    break;

  case AB_AccountType_Investment:
    rv=AO_Provider__AddInvStatementReq(pro, u, a, j, buf);
    break;

  case AB_AccountType_Checking:
  case AB_AccountType_Savings:
  case AB_AccountType_Bank:
  case AB_AccountType_Cash:
  case AB_AccountType_Unknown:
  default:
    rv=AO_Provider__AddBankStatementReq(pro, u, a, j, buf);
    break;
  }

  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AO_Provider_RequestStatements(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *j, AB_IMEXPORTER_CONTEXT *ictx) {
  AO_PROVIDER *dp;
  GWEN_BUFFER *reqbuf;
  GWEN_BUFFER *rbuf=0;
  int rv;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  /* get all data for the context */
  assert(a);
  assert(u);

  reqbuf=GWEN_Buffer_new(0, 2048, 0, 1);
  GWEN_Buffer_ReserveBytes(reqbuf, 1024);

  /* add actual request */
  rv=AO_Provider__AddStatementRequest(pro, u, a, j, reqbuf);
  if (rv<0) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Error adding request element (%d)", rv);
    GWEN_Buffer_free(reqbuf);
    return rv;
  }

  /* wrap message (adds headers etc) */
  rv=AO_Provider__WrapMessage(pro, u, reqbuf);
  if (rv<0) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
	      "Error adding request element (%d)", rv);
    GWEN_Buffer_free(reqbuf);
    return rv;
  }

  /* exchange messages (might also return HTTP code!) */
  rv=AO_Provider_SendAndReceive(pro, u,
				(const uint8_t*)GWEN_Buffer_GetStart(reqbuf),
				GWEN_Buffer_GetUsedBytes(reqbuf),
				&rbuf);
  if (rv) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
	      "Error exchanging getStatements-request (%d)", rv);
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Error,
			 I18N("Error parsing server response"));
    GWEN_Buffer_free(rbuf);
    GWEN_Buffer_free(reqbuf);
    return rv;
  }
  else {
    AB_IMEXPORTER *importer;
    GWEN_DB_NODE *dbProfile;

    /* parse response */
    GWEN_Buffer_free(reqbuf);
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Info, I18N("Parsing response"));

    /* prepare import */
    importer=AB_Banking_GetImExporter(AB_Provider_GetBanking(pro), "ofx");
    if (!importer) {
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "OFX import module not found");
      GWEN_Buffer_free(rbuf);
      return GWEN_ERROR_NOT_FOUND;
    }

    GWEN_Buffer_Rewind(rbuf);
    dbProfile=GWEN_DB_Group_new("profile");
    /* actually import */
    rv=AB_ImExporter_ImportBuffer(importer, ictx, rbuf, dbProfile);
    GWEN_DB_Group_free(dbProfile);
    GWEN_Buffer_free(rbuf);
    if (rv<0) {
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Error importing server response (%d)", rv);
      GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, I18N("Error parsing response"));
      return rv;
    }
  }

  return 0;
}



