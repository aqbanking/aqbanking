/***************************************************************************
    begin       : Sat Dec 15 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/




static void _import_052_001_02_read_account_spec(AB_IMEXPORTER *ie,
                                                 GWEN_XMLNODE *xmlNode,
                                                 AB_ACCOUNT_SPEC *accountSpec) {
  const char *s;

  s=GWEN_XMLNode_GetCharValueByPath(xmlNode, "Id/IBAN", NULL);
  if (s && *s)
    AB_AccountSpec_SetIban(accountSpec, s);

  s=GWEN_XMLNode_GetCharValueByPath(xmlNode, "Ccy", NULL);
  if (s && *s)
    AB_AccountSpec_SetCurrency(accountSpec, s);

  s=GWEN_XMLNode_GetCharValueByPath(xmlNode, "Ownr/Nm", NULL);
  if (s && *s)
    AB_AccountSpec_SetOwnerName(accountSpec, s);

  s=GWEN_XMLNode_GetCharValueByPath(xmlNode, "Svcr/FinInstnId/BIC", NULL);
  if (s && *s)
    AB_AccountSpec_SetBic(accountSpec, s);

}



static int _import_052_001_02_read_balance(AB_IMEXPORTER *ie,
                                           GWEN_XMLNODE *xmlNode,
                                           AB_IMEXPORTER_ACCOUNTINFO *accountInfo) {
  const char *s;
  GWEN_XMLNODE *n;
  AB_BALANCE *balance=NULL;

  balance=AB_Balance_new();

  /* read amount */
  n=GWEN_XMLNode_GetNodeByXPath(xmlNode, "Amt", GWEN_PATH_FLAGS_NAMEMUSTEXIST);
  if (n) {
    const char *currency;
    AB_VALUE *val=NULL;

    currency=GWEN_XMLNode_GetProperty(n, "Ccy", "EUR");

    s=GWEN_XMLNode_GetCharValue(xmlNode, "Amt", NULL);
    if (!(s && *s)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "No data in <BAL/Amt>: [%s]", s);
      AB_Balance_free(balance);
      return GWEN_ERROR_BAD_DATA;
    }

    val=AB_Value_fromString(s);
    if (val==NULL) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid amount in <BAL>: [%s]", s);
      AB_Balance_free(balance);
      return GWEN_ERROR_BAD_DATA;
    }
    AB_Value_SetCurrency(val, currency);

    s=GWEN_XMLNode_GetCharValue(xmlNode, "CdtDbtInd", NULL);
    if (!(s && *s)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing <CdtDbtInd> in <BAL>");
      AB_Value_free(val);
      AB_Balance_free(balance);
      return GWEN_ERROR_BAD_DATA;
    }
    if (strcasecmp(s, "CRDT")==0)
      AB_Value_Negate(val);

    AB_Balance_SetValue(balance, val);
    AB_Value_free(val);
  }

  /* read date */
  s=GWEN_XMLNode_GetCharValueByPath(xmlNode, "Dt/Dt", NULL);
  if (!(s && *s))
    s=GWEN_XMLNode_GetCharValueByPath(xmlNode, "Dt/DtTm", NULL);
  if (s && *s) {
    GWEN_DATE *dt;

    dt=GWEN_Date_fromStringWithTemplate(s, "YYYY-MM-DD");
    if (dt==NULL) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid date in <BAL>: [%s]", s);
      AB_Balance_free(balance);
      return GWEN_ERROR_BAD_DATA;
    }
    AB_Balance_SetDate(balance, dt);
    GWEN_Date_free(dt);
  }

  /* determine the type of balance, add if acceptable */
  s=GWEN_XMLNode_GetCharValueByPath(xmlNode, "Tp/CdOrPrtry/Cd", NULL);
  if (s && *s) {
    if (strcasecmp(s, "CLBD")==0 ||  /* CLBD: Closing Booked Balance */
        strcasecmp(s, "PRCD")==0) {  /* PRCD: Previously Closed Booked Balance */
      AB_Balance_SetType(balance, AB_Balance_TypeBooked);
      AB_ImExporterAccountInfo_AddBalance(accountInfo, balance);
    }

    else if (strcasecmp(s, "CLAV")==0) {
      /* CLAV: Closing Available Balance */
      AB_Balance_SetType(balance, AB_Balance_TypeDisposable);
      AB_ImExporterAccountInfo_AddBalance(accountInfo, balance);
    }
    else {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Unknown balance type [%s] in <BAL>, ignoring", s);
      AB_Balance_free(balance);
    }
  }

  return 0;
}



static int _import_052_001_02_read_balances(AB_IMEXPORTER *ie,
                                            GWEN_XMLNODE *xmlNode,
                                            AB_IMEXPORTER_ACCOUNTINFO *accountInfo) {
  GWEN_XMLNODE *n;

  n=GWEN_XMLNode_FindFirstTag(xmlNode, "Bal", NULL, NULL);
  while(n) {
    int rv;

    rv=_import_052_001_02_read_balance(ie, n, accountInfo);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    n=GWEN_XMLNode_FindNextTag(n, "Bal", NULL, NULL);
  } /* while n */

  return 0;
}



static int _import_052_001_02_read_transaction_details(AB_IMEXPORTER *ie,
                                                       GWEN_XMLNODE *xmlNode,
                                                       AB_TRANSACTION *t,
                                                       int isCredit) {
  const char *s;
  GWEN_XMLNODE *n;

  s=GWEN_XMLNode_GetCharValueByPath(xmlNode, "Refs/EndToEndId", NULL);
  if (s && *s)
    AB_Transaction_SetEndToEndReference(t, s);

  /* read names and accounts */
  s=GWEN_XMLNode_GetCharValueByPath(xmlNode, "Refs/MndtId", NULL);
  if (s && *s)
    AB_Transaction_SetMandateId(t, s);

  if (isCredit) {
    s=GWEN_XMLNode_GetCharValueByPath(xmlNode, "RltdPties/Dbtr/Nm", NULL);
    if (s && *s)
      AB_Transaction_SetRemoteName(t, s);
    s=GWEN_XMLNode_GetCharValueByPath(xmlNode, "RltdPties/DbtrAcct/Id/IBAN", NULL);
    if (s && *s)
      AB_Transaction_SetRemoteIban(t, s);

    s=GWEN_XMLNode_GetCharValueByPath(xmlNode, "RltdPties/Cdtr/Nm", NULL);
    if (s && *s)
      AB_Transaction_SetLocalName(t, s);
    s=GWEN_XMLNode_GetCharValueByPath(xmlNode, "RltdPties/CdtrAcct/Id/IBAN", NULL);
    if (s && *s)
      AB_Transaction_SetLocalIban(t, s);
  }
  else {
    s=GWEN_XMLNode_GetCharValueByPath(xmlNode, "RltdPties/Dbtr/Nm", NULL);
    if (s && *s)
      AB_Transaction_SetLocalName(t, s);
    s=GWEN_XMLNode_GetCharValueByPath(xmlNode, "RltdPties/DbtrAcct/Id/IBAN", NULL);
    if (s && *s)
      AB_Transaction_SetLocalIban(t, s);

    s=GWEN_XMLNode_GetCharValueByPath(xmlNode, "RltdPties/Cdtr/Nm", NULL);
    if (s && *s)
      AB_Transaction_SetRemoteName(t, s);

    s=GWEN_XMLNode_GetCharValueByPath(xmlNode, "RltdPties/CdtrAcct/Id/IBAN", NULL);
    if (s && *s)
      AB_Transaction_SetRemoteIban(t, s);

    s=GWEN_XMLNode_GetCharValueByPath(xmlNode, "RltdPties/Cdtr/Id/PrvtId/Othr/Id", NULL);
    if (s && *s)
      AB_Transaction_SetOriginatorId(t, s);
  }

  /* read transaction codes */
  s=GWEN_XMLNode_GetCharValueByPath(xmlNode, "BkTxCd/Domn/Cd", NULL);
  if (s && *s) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendByte(tbuf, '-');

    s=GWEN_XMLNode_GetCharValueByPath(xmlNode, "BkTxCd/Domn/Fmly/Cd", NULL);
    if (s && *s)
      GWEN_Buffer_AppendString(tbuf, s);
    GWEN_Buffer_AppendByte(tbuf, '-');

    s=GWEN_XMLNode_GetCharValueByPath(xmlNode, "BkTxCd/Domn/Fmly/SubFmlyCd", NULL);
    if (s && *s)
      GWEN_Buffer_AppendString(tbuf, s);

    AB_Transaction_SetTransactionKey(t, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }


  /* read purpose */
  n=GWEN_XMLNode_FindFirstTag(xmlNode, "RmtInf", NULL, NULL);
  if(n)
    n=GWEN_XMLNode_FindFirstTag(n, "Ustrd", NULL, NULL);
  while(n) {
    GWEN_XMLNODE *nn;

    nn=GWEN_XMLNode_GetFirstData(n);
    if (nn) {
      s=GWEN_XMLNode_GetData(nn);
      if (s && *s)
        AB_Transaction_AddPurposeLine(t, s);
    }
    n=GWEN_XMLNode_FindNextTag(n, "Ustrd", NULL, NULL);
  } /* while n */

  return 0;
}



static int _import_052_001_02_read_transaction(AB_IMEXPORTER *ie,
					       GWEN_XMLNODE *xmlNode,
                                               AB_IMEXPORTER_ACCOUNTINFO *accountInfo) {
  const char *s;
  GWEN_XMLNODE *n;
  AB_TRANSACTION *t;
  int isCredit=0;

  t=AB_Transaction_new();
  AB_Transaction_SetType(t, AB_Transaction_TypeStatement);

  /* read credit/debit mark */
  s=GWEN_XMLNode_GetCharValueByPath(xmlNode, "CdtDbtInd", NULL);
  if (s && *s) {
    if (strcasecmp(s, "DBIT")==0) {
      isCredit=0;
    }
    else if (strcasecmp(s, "CRDT")==0) {
      isCredit=1;
    }
    else {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid CdtDbtInd in <Ntry>: [%s]", s);
      AB_Transaction_free(t);
      return GWEN_ERROR_BAD_DATA;
    }
  }

  /* read amount */
  n=GWEN_XMLNode_GetNodeByXPath(xmlNode, "Amt", GWEN_PATH_FLAGS_NAMEMUSTEXIST);
  if (n) {
    const char *currency;
    AB_VALUE *val=NULL;

    currency=GWEN_XMLNode_GetProperty(n, "Ccy", "EUR");

    s=GWEN_XMLNode_GetCharValue(xmlNode, "Amt", NULL);
    if (!(s && *s)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "No currency in <Ntry/Amt>: [%s]", s);
      AB_Transaction_free(t);
      return GWEN_ERROR_BAD_DATA;
    }

    val=AB_Value_fromString(s);
    if (val==NULL) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid amount in <Ntry>: [%s]", s);
      AB_Transaction_free(t);
      return GWEN_ERROR_BAD_DATA;
    }

    AB_Value_SetCurrency(val, currency);
    if (!isCredit)
      AB_Value_Negate(val);

    AB_Transaction_SetValue(t, val);
    AB_Value_free(val);
  }

  /* read booked date */
  s=GWEN_XMLNode_GetCharValueByPath(xmlNode, "BookgDt/Dt", NULL);
  if (s && *s) {
    GWEN_DATE *dt;

    dt=GWEN_Date_fromStringWithTemplate(s, "YYYY-MM-DD");
    if (dt==NULL) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid booking date in <Ntry>: [%s]", s);
      AB_Transaction_free(t);
      return GWEN_ERROR_BAD_DATA;
    }
    AB_Transaction_SetDate(t, dt);
    GWEN_Date_free(dt);
  }

  /* read valuta date */
  s=GWEN_XMLNode_GetCharValueByPath(xmlNode, "ValDt/Dt", NULL);
  if (s && *s) {
    GWEN_DATE *dt;

    dt=GWEN_Date_fromStringWithTemplate(s, "YYYY-MM-DD");
    if (dt==NULL) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid valuta date in <Ntry>: [%s]", s);
      AB_Transaction_free(t);
      return GWEN_ERROR_BAD_DATA;
    }
    AB_Transaction_SetValutaDate(t, dt);
    GWEN_Date_free(dt);
  }

  /* read bank reference */
  s=GWEN_XMLNode_GetCharValueByPath(xmlNode, "NtryRef", NULL);
  if (s && *s)
    AB_Transaction_SetBankReference(t, s);

  s=GWEN_XMLNode_GetCharValueByPath(xmlNode, "AddtlNtryInf", NULL);
  if (s && *s)
    AB_Transaction_SetTransactionText(t, s);
  

  /* read transaction details */
  n=GWEN_XMLNode_GetNodeByXPath(xmlNode, "NtryDtls/TxDtls", GWEN_PATH_FLAGS_NAMEMUSTEXIST);
  if (n) {
    int rv;

    rv=_import_052_001_02_read_transaction_details(ie, n, t, isCredit);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  /* check transaction */
  if (!(AB_Transaction_GetValue(t) && (AB_Transaction_GetDate(t) || AB_Transaction_GetValutaDate(t)))) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Incomplete transaction received");
    AB_Transaction_free(t);
    return GWEN_ERROR_BAD_DATA;
  }
  AB_ImExporterAccountInfo_AddTransaction(accountInfo, t);

  return 0;
}



static int _import_052_001_02_read_transactions(AB_IMEXPORTER *ie,
						GWEN_XMLNODE *xmlNode,
						AB_IMEXPORTER_ACCOUNTINFO *accountInfo) {
  GWEN_XMLNODE *n;

  n=GWEN_XMLNode_FindFirstTag(xmlNode, "Ntry", NULL, NULL);
  while(n) {
    int rv;

    rv=_import_052_001_02_read_transaction(ie, n, accountInfo);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    n=GWEN_XMLNode_FindNextTag(n, "Ntry", NULL, NULL);
  } /* while n */

  return 0;
}



static int _import_052_001_02_report(AB_IMEXPORTER *ie,
                                     AB_IMEXPORTER_CONTEXT *ctx,
                                     GWEN_DB_NODE *params,
                                     GWEN_XMLNODE *xmlNode) {
  GWEN_XMLNODE *n;
  AB_IMEXPORTER_ACCOUNTINFO *accountInfo=NULL;
  int rv;

  /* read account, set accountInfo */
  n=GWEN_XMLNode_FindFirstTag(xmlNode, "Acct", NULL, NULL);
  if (n) {
    AB_ACCOUNT_SPEC *accountSpec;

    accountSpec=AB_AccountSpec_new();
    _import_052_001_02_read_account_spec(ie, n, accountSpec);
    accountInfo=AB_ImExporterContext_GetOrAddAccountInfo(ctx,
							 0,
							 AB_AccountSpec_GetIban(accountSpec),
							 AB_AccountSpec_GetBankCode(accountSpec),
							 AB_AccountSpec_GetAccountNumber(accountSpec),
							 AB_AccountType_Unknown);
    assert(accountInfo);
    AB_AccountSpec_free(accountSpec);
  }

  /* read balances */
  rv=_import_052_001_02_read_balances(ie, xmlNode, accountInfo);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* read transactions */
  rv=_import_052_001_02_read_transactions(ie, xmlNode, accountInfo);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}





int AH_ImExporterCAMT_Import_052_001_02(AB_IMEXPORTER *ie,
                                        AB_IMEXPORTER_CONTEXT *ctx,
                                        GWEN_DB_NODE *params,
                                        GWEN_XMLNODE *xmlNode) {
  GWEN_XMLNODE *n;

  n=GWEN_XMLNode_FindFirstTag(xmlNode, "BkToCstmrAcctRpt", NULL, NULL);
  if (n==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "<BkToCstmrAcctRpt> element not found");
    return GWEN_ERROR_BAD_DATA;
  }

  n=GWEN_XMLNode_FindFirstTag(n, "Rpt", NULL, NULL);
  if (n==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "<Rpt> element not found");
    return GWEN_ERROR_BAD_DATA;
  }

  /* now read every report */
  while(n) {
    int rv;

    rv=_import_052_001_02_report(ie, ctx, params, n);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    n=GWEN_XMLNode_FindNextTag(n, "Rpt", NULL, NULL);
  }
  return 0;
}




