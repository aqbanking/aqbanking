/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2009 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

/* This file is included by banking.c */



void AB_Banking__fillTransactionFromAccount(AB_TRANSACTION *t, const AB_ACCOUNT *a) {
  const char *s;

  s=AB_Transaction_GetLocalName(t);
  if (!s)
    AB_Transaction_SetLocalName(t, AB_Account_GetOwnerName(a));
  s=AB_Transaction_GetLocalBankCode(t);
  if (!s)
    AB_Transaction_SetLocalBankCode(t, AB_Account_GetBankCode(a));
  s=AB_Transaction_GetLocalAccountNumber(t);
  if (!s)
    AB_Transaction_SetLocalAccountNumber(t, AB_Account_GetAccountNumber(a));
  s=AB_Transaction_GetLocalIban(t);
  if (!s)
    AB_Transaction_SetLocalIban(t, AB_Account_GetIBAN(a));
  s=AB_Transaction_GetLocalBic(t);
  if (!s)
    AB_Transaction_SetLocalBic(t, AB_Account_GetBIC(a));
}



void AB_Banking__fillTransactionRemoteInfo(AB_TRANSACTION *t) {
  const GWEN_STRINGLIST *sl;

  sl=AB_Transaction_GetPurpose(t);
  if (sl) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(sl);
    while(se) {
      const char *s;

      s=GWEN_StringListEntry_Data(se);
      if (-1!=GWEN_Text_ComparePattern(s, "KTO* BLZ*", 0)) {
	char *cpy;
	char *p;
	char *kto;
	char *blz;

	cpy=strdup(s);
	p=cpy;

	/* skip "KTO", position to account number */
	while(*p && !isdigit(*p))
	  p++;
	kto=p;

	/* skip account number */
	while(*p && isdigit(*p))
	  p++;
	/* terminate account number */
	*(p++)=0;

	/* skip "BLZ", position to account number */
	while(*p && !isdigit(*p))
	  p++;
	blz=p;

	/* skip bank code */
	while(*p && isdigit(*p))
	  p++;
	/* terminate bank code */
	*p=0;

	if (*kto && *blz) {
	  AB_Transaction_SetRemoteAccountNumber(t, kto);
	  AB_Transaction_SetRemoteBankCode(t, blz);
	  free(cpy);
	  break;
	}
	else
	  free(cpy);
      }

      se=GWEN_StringListEntry_Next(se);
    }
  }
}




int AB_Banking_FillGapsInImExporterContext(AB_BANKING *ab, AB_IMEXPORTER_CONTEXT *iec) {
  AB_IMEXPORTER_ACCOUNTINFO *iea;
  int notFounds=0;

  assert(iec);
  iea=AB_ImExporterContext_GetFirstAccountInfo(iec);
  while(iea) {
    AB_ACCOUNT *a;

    a=AB_Banking_GetAccountByCodeAndNumber(ab,
					   AB_ImExporterAccountInfo_GetBankCode(iea),
					   AB_ImExporterAccountInfo_GetAccountNumber(iea));
    if (!a)
      a=AB_Banking_GetAccountByIban(ab, AB_ImExporterAccountInfo_GetIban(iea));
    if (a) {
      AB_TRANSACTION *t;

      AB_ImExporterAccountInfo_FillFromAccount(iea, a);

      /* fill transactions */
      t=AB_ImExporterAccountInfo_GetFirstTransaction(iea);
      while(t) {
	AB_Banking__fillTransactionFromAccount(t, a);
	if (AB_Transaction_GetRemoteBankCode(t)==NULL &&
	    AB_Transaction_GetRemoteAccountNumber(t)==NULL)
	  AB_Banking__fillTransactionRemoteInfo(t);
	t=AB_ImExporterAccountInfo_GetNextTransaction(iea);
      }

      /* fill standing orders */
      t=AB_ImExporterAccountInfo_GetFirstStandingOrder(iea);
      while(t) {
	AB_Banking__fillTransactionFromAccount(t, a);
	t=AB_ImExporterAccountInfo_GetNextStandingOrder(iea);
      }

      /* fill transfers */
      t=AB_ImExporterAccountInfo_GetFirstTransfer(iea);
      while(t) {
	AB_Banking__fillTransactionFromAccount(t, a);
	t=AB_ImExporterAccountInfo_GetNextTransfer(iea);
      }

      /* fill dated transfers */
      t=AB_ImExporterAccountInfo_GetFirstDatedTransfer(iea);
      while(t) {
	AB_Banking__fillTransactionFromAccount(t, a);
	t=AB_ImExporterAccountInfo_GetNextDatedTransfer(iea);
      }

      /* fill noted transactions */
      t=AB_ImExporterAccountInfo_GetFirstNotedTransaction(iea);
      while(t) {
	AB_Banking__fillTransactionFromAccount(t, a);
	t=AB_ImExporterAccountInfo_GetNextNotedTransaction(iea);
      }
    }
    else
      notFounds++;

    iea=AB_ImExporterContext_GetNextAccountInfo(iec);
  }

  return (notFounds==0)?0:1;
}



int AB_Banking_ExportToBuffer(AB_BANKING *ab,
			      AB_IMEXPORTER_CONTEXT *ctx,
			      const char *exporterName,
                              const char *profileName,
			      GWEN_BUFFER *buf,
			      uint32_t guiid) {
  AB_IMEXPORTER *ie;
  GWEN_DB_NODE *dbProfile;
  int rv;

  ie=AB_Banking_GetImExporter(ab, exporterName);
  if (ie==NULL) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here");
    return GWEN_ERROR_NO_DATA;
  }

  if (profileName && *profileName)
    dbProfile=AB_Banking_GetImExporterProfiles(ab, profileName);
  else
    dbProfile=GWEN_DB_Group_new("profile");
  if (dbProfile==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Profile [%s] not found",
	      profileName?profileName:"(null)");
    return GWEN_ERROR_NO_DATA;
  }

  rv=AB_ImExporter_ExportToBuffer(ie, ctx, buf, dbProfile, guiid);
  GWEN_DB_Group_free(dbProfile);

  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}





