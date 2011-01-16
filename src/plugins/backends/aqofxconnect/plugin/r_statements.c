/***************************************************************************
 begin       : Wed Jan 09 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/





int AO_Provider__AddBankStatementReq(AB_PROVIDER *pro, AB_JOB *j,
				     GWEN_BUFFER *buf) {
  const char *s;
  AB_ACCOUNT *a;
  AB_USER *u;
  int rv;

  a=AB_Job_GetAccount(j);
  assert(a);
  u=AB_Account_GetFirstUser(a);
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
  if (AB_Job_GetType(j)==AB_Job_TypeGetTransactions) {
    const GWEN_TIME *ti;

    ti=AB_JobGetTransactions_GetFromTime(j);
    if (ti) {
      GWEN_Buffer_AppendString(buf, "<DTSTART>");
      if (AO_User_GetFlags(u) & AO_USER_FLAGS_SEND_SHORT_DATE)
	GWEN_Time_toString(ti, "YYYYMMDDhhmmss", buf);
      else
	GWEN_Time_toString(ti, "YYYYMMDDhhmmss.000", buf);
    }

    ti=AB_JobGetTransactions_GetToTime(j);
    if (ti) {
      GWEN_Buffer_AppendString(buf, "<DTEND>");
      if (AO_User_GetFlags(u) & AO_USER_FLAGS_SEND_SHORT_DATE)
	GWEN_Time_toString(ti, "YYYYMMDDhhmmss", buf);
      else
	GWEN_Time_toString(ti, "YYYYMMDDhhmmss.000", buf);
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



int AO_Provider__AddCreditCardStatementReq(AB_PROVIDER *pro, AB_JOB *j,
					   GWEN_BUFFER *buf) {
  const char *s;
  AB_ACCOUNT *a;
  AB_USER *u;
  int rv;

  a=AB_Job_GetAccount(j);
  assert(a);
  u=AB_Account_GetFirstUser(a);
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
  if (AB_Job_GetType(j)==AB_Job_TypeGetTransactions) {
    const GWEN_TIME *ti;

    ti=AB_JobGetTransactions_GetFromTime(j);
    if (ti) {
      GWEN_Buffer_AppendString(buf, "<DTSTART>");
      GWEN_Time_toString(ti, "YYYYMMDDhhmmss", buf);
    }

    ti=AB_JobGetTransactions_GetToTime(j);
    if (ti) {
      GWEN_Buffer_AppendString(buf, "<DTEND>");
      GWEN_Time_toString(ti, "YYYYMMDDhhmmss", buf);
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



int AO_Provider__AddInvStatementReq(AB_PROVIDER *pro, AB_JOB *j,
				    GWEN_BUFFER *buf) {
  const char *s;
  AB_ACCOUNT *a;
  AB_USER *u;
  int rv;

  a=AB_Job_GetAccount(j);
  assert(a);
  u=AB_Account_GetFirstUser(a);
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
  if (AB_Job_GetType(j)==AB_Job_TypeGetTransactions) {
    const GWEN_TIME *ti;

    ti=AB_JobGetTransactions_GetFromTime(j);
    if (ti) {
      GWEN_Buffer_AppendString(buf, "<DTSTART>");
      GWEN_Time_toString(ti, "YYYYMMDD", buf);
    }

    ti=AB_JobGetTransactions_GetToTime(j);
    if (ti) {
      GWEN_Buffer_AppendString(buf, "<DTEND>");
      GWEN_Time_toString(ti, "YYYYMMDD", buf);
    }
    GWEN_Buffer_AppendString(buf, "<INCLUDE>Y");
  }
  else {
    GWEN_Buffer_AppendString(buf, "<INCLUDE>N");
  }
  GWEN_Buffer_AppendString(buf, "</INCTRAN>");

  GWEN_Buffer_AppendString(buf, "<INCOO>Y");

  GWEN_Buffer_AppendString(buf, "<INCPOS>");
  if (AB_Job_GetType(j)==AB_Job_TypeGetTransactions) {
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



int AO_Provider__AddStatementRequest(AB_PROVIDER *pro, AB_JOB *j,
				     GWEN_BUFFER *buf) {
  AB_ACCOUNT *a;
  int rv;

  a=AB_Job_GetAccount(j);
  assert(a);

  switch(AB_Account_GetAccountType(a)) {
  case AB_AccountType_CreditCard:
    rv=AO_Provider__AddCreditCardStatementReq(pro, j, buf);
    break;

  case AB_AccountType_Investment:
    rv=AO_Provider__AddInvStatementReq(pro, j, buf);
    break;

  case AB_AccountType_Checking:
  case AB_AccountType_Savings:
  case AB_AccountType_Bank:
  case AB_AccountType_Cash:
  case AB_AccountType_Unknown:
  default:
    rv=AO_Provider__AddBankStatementReq(pro, j, buf);
    break;
  }

  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}






