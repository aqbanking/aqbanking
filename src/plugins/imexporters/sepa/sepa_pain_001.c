

#include <aqbanking/banking_be.h>



int AH_ImExporterSEPA_Export_Pain_001(AB_IMEXPORTER *ie,
                                      AB_IMEXPORTER_CONTEXT *ctx,
                                      GWEN_XMLNODE *painNode,
                                      uint32_t doctype[],
                                      GWEN_DB_NODE *params){
  GWEN_XMLNODE *n;
  AH_IMEXPORTER_SEPA_PMTINF_LIST *pl;
  AH_IMEXPORTER_SEPA_PMTINF *pmtinf;
  int post_1_1_2=(doctype[1]>1 || doctype[2]>2);
  const char *s;
  int rv;

  rv=AH_ImExporterSEPA_Export_Pain_Setup(ie, ctx, painNode, doctype, &pl);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "here %d", rv);
    return rv;
  }

  /* generate PmtInf blocks */
  pmtinf=AH_ImExporter_Sepa_PmtInf_List_First(pl);
  while(pmtinf) {
    const GWEN_TIME *tti;
    GWEN_XMLNODE *nn;
    AB_TRANSACTION *t;
    AB_TRANSACTION_LIST2_ITERATOR *it;

    n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "PmtInf");
    GWEN_XMLNode_AddChild(painNode, n);

    /* generate PmtInfId */
    if (1) {
      GWEN_TIME *ti;
      GWEN_BUFFER *tbuf;
      uint32_t uid;
      char numbuf[32];

      ti=GWEN_CurrentTime();
      tbuf=GWEN_Buffer_new(0, 64, 0, 1);

      uid=AB_Banking_GetUniqueId(AB_ImExporter_GetBanking(ie));
      GWEN_Time_toUtcString(ti, "YYYYMMDD-hh:mm:ss-", tbuf);
      snprintf(numbuf, sizeof(numbuf)-1, "%08x", uid);
      GWEN_Buffer_AppendString(tbuf, numbuf);
      GWEN_XMLNode_SetCharValue(n, "PmtInfId", GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_free(tbuf);
      GWEN_Time_free(ti);
    }

    GWEN_XMLNode_SetCharValue(n, "PmtMtd", "TRF");

    if (post_1_1_2) {
      /* store BtchBookg */
      GWEN_XMLNode_SetCharValue(n, "BtchBookg",
				GWEN_DB_GetIntValue(params,
						    "singleBookingWanted", 0, 1)
				? "false"
				: "true");
      /* store NbOfTxs */
      GWEN_XMLNode_SetIntValue(n, "NbOfTxs", pmtinf->tcount);
      /* store CtrlSum */
      GWEN_XMLNode_SetCharValue(n, "CtrlSum", pmtinf->ctrlsum);
    }

    nn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "PmtTpInf");
    if (nn) {
      GWEN_XMLNODE *nnn;

      nnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "SvcLvl");
      if (nnn) {
	GWEN_XMLNode_SetCharValue(nnn, "Cd", "SEPA");
	GWEN_XMLNode_AddChild(nn, nnn);
      }

      GWEN_XMLNode_AddChild(n, nn);
    }

    /* create ReqdExctnDt" */
    tti=pmtinf->date;
    if (tti) {
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, 64, 0, 1);
      GWEN_Time_toString(tti, "YYYY-MM-DD", tbuf);
      GWEN_XMLNode_SetCharValue(n, "ReqdExctnDt", GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_free(tbuf);
    }
    else {
      GWEN_XMLNode_SetCharValue(n, "ReqdExctnDt", "1999-01-01");
    }

    /* create "Dbtr" */
    nn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "Dbtr");
    if (nn) {
      GWEN_XMLNode_AddChild(n, nn);
      GWEN_XMLNode_SetCharValue(nn, "Nm", pmtinf->localName);
    }

    /* create "DbtrAcct" */
    nn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "DbtrAcct");
    if (nn) {
      GWEN_XMLNODE *nnn;

      GWEN_XMLNode_AddChild(n, nn);
      nnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "Id");
      if (nnn) {
	GWEN_XMLNode_AddChild(nn, nnn);
	GWEN_XMLNode_SetCharValue(nnn, "IBAN", pmtinf->localIban);
      }
    }

    /* create "DbtrAgt" */
    nn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "DbtrAgt");
    if (nn) {
      GWEN_XMLNODE *nnn;

      GWEN_XMLNode_AddChild(n, nn);
      nnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "FinInstnId");
      if (nnn) {
	GWEN_XMLNode_AddChild(nn, nnn);
	GWEN_XMLNode_SetCharValue(nnn, "BIC", pmtinf->localBic);
      }
    }

    GWEN_XMLNode_SetCharValue(n, "ChrgBr", "SLEV");


    it=AB_Transaction_List2_First(pmtinf->transactions);
    assert(it);
    t=AB_Transaction_List2Iterator_Data(it);
    while(t) {
      GWEN_XMLNODE *nn;

      nn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "CdtTrfTxInf");
      if (nn) {
	GWEN_XMLNODE *nnn;
	const AB_VALUE *tv;

	GWEN_XMLNode_AddChild(n, nn);

	/* create "PmtId" */
	nnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "PmtId");
	if (nnn) {
	  GWEN_XMLNode_AddChild(nn, nnn);
	  s=AB_Transaction_GetEndToEndReference(t);
	  if (!( s && *s))
	    s=AB_Transaction_GetCustomerReference(t);
	  if (!s)
	    s="NOTPROVIDED";
	  GWEN_XMLNode_SetCharValue(nnn, "EndToEndId", s);
	}

	tv=AB_Transaction_GetValue(t);
	if (tv==NULL) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN, "No value in transaction");
	  AB_Transaction_List2Iterator_free(it);
	  AH_ImExporter_Sepa_PmtInf_List_free(pl);
	  return GWEN_ERROR_BAD_DATA;
	}

	nnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "Amt");
	if (nnn) {
	  GWEN_XMLNODE *nnnn;

	  GWEN_XMLNode_AddChild(nn, nnn);

	  nnnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "InstdAmt");
	  if (nnnn) {
	    GWEN_BUFFER *tbuf;
	    GWEN_XMLNODE *nnnnn;

	    GWEN_XMLNode_AddChild(nnn, nnnn);

	    tbuf=GWEN_Buffer_new(0, 64, 0, 1);
	    AB_Value_toHumanReadableString2(tv, tbuf, 2, 0);
	    s=AB_Value_GetCurrency(tv);
	    if (!s)
	      s="EUR";
	    GWEN_XMLNode_SetProperty(nnnn, "Ccy", s);

	    nnnnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeData, GWEN_Buffer_GetStart(tbuf));
	    GWEN_XMLNode_AddChild(nnnn, nnnnn);
	    GWEN_Buffer_free(tbuf);
	  }
	}

	/* create "CdtrAgt" */
	s=AB_Transaction_GetRemoteBic(t);
	if (s && *s)
	  GWEN_XMLNode_SetCharValueByPath(nn, GWEN_XML_PATH_FLAGS_OVERWRITE_VALUES,
					  "CdtrAgt/FinInstnId/BIC", s);
	else if (doctype[1]<3) { /* BIC not required since 001.003.03 */
	  DBG_ERROR(AQBANKING_LOGDOMAIN, "No remote BIC");
	  AB_Transaction_List2Iterator_free(it);
	  AH_ImExporter_Sepa_PmtInf_List_free(pl);
	  return GWEN_ERROR_BAD_DATA;
	}

	/* create "Cdtr" */
	nnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "Cdtr");
	if (nnn) {
	  const GWEN_STRINGLIST *sl;

	  s=NULL;
	  GWEN_XMLNode_AddChild(nn, nnn);
	  sl=AB_Transaction_GetRemoteName(t);
	  if (sl)
	    s=GWEN_StringList_FirstString(sl);
	  if (!s) {
	    DBG_ERROR(AQBANKING_LOGDOMAIN, "No remote name");
	    AB_Transaction_List2Iterator_free(it);
	    AH_ImExporter_Sepa_PmtInf_List_free(pl);
	    return GWEN_ERROR_BAD_DATA;
	  }
	  GWEN_XMLNode_SetCharValue(nnn, "Nm", s);
	}

	/* create "CdtrAcct" */
	nnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "CdtrAcct");
	if (nnn) {
	  GWEN_XMLNODE *nnnn;

	  GWEN_XMLNode_AddChild(nn, nnn);
	  s=AB_Transaction_GetRemoteIban(t);
	  if (!s) {
	    DBG_ERROR(AQBANKING_LOGDOMAIN, "No remote IBAN");
	    AB_Transaction_List2Iterator_free(it);
	    AH_ImExporter_Sepa_PmtInf_List_free(pl);
	    return GWEN_ERROR_BAD_DATA;
	  }

	  nnnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "Id");
	  if (nnnn) {
	    GWEN_XMLNode_AddChild(nnn, nnnn);
	    GWEN_XMLNode_SetCharValue(nnnn, "IBAN", s);
	  }
	}

	/* create "RmtInf" */
	nnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "RmtInf");
	if (nnn) {
	  const GWEN_STRINGLIST *sl;
	  GWEN_BUFFER *tbuf;

	  GWEN_XMLNode_AddChild(nn, nnn);

	  tbuf=GWEN_Buffer_new(0, 140, 0, 1);
	  sl=AB_Transaction_GetPurpose(t);
	  if (sl) {
	    GWEN_STRINGLISTENTRY *se;

	    se=GWEN_StringList_FirstEntry(sl);
	    while(se) {
	      s=GWEN_StringListEntry_Data(se);
	      assert(s);
	      if (GWEN_Buffer_GetUsedBytes(tbuf))
		GWEN_Buffer_AppendByte(tbuf, ' ');
	      GWEN_Buffer_AppendString(tbuf, s);
	      se=GWEN_StringListEntry_Next(se);
	    }
	    if (GWEN_Buffer_GetUsedBytes(tbuf)>140)
	      GWEN_Buffer_Crop(tbuf, 0, 140);
	  }

	  if (GWEN_Buffer_GetUsedBytes(tbuf)<1) {
	    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing purpose in transaction");
	    GWEN_Buffer_free(tbuf);
	    AB_Transaction_List2Iterator_free(it);
	    AH_ImExporter_Sepa_PmtInf_List_free(pl);
	    return GWEN_ERROR_BAD_DATA;
	  }

	  GWEN_XMLNode_SetCharValue(nnn, "Ustrd", GWEN_Buffer_GetStart(tbuf));

	  GWEN_Buffer_free(tbuf);
	}
      }

      t=AB_Transaction_List2Iterator_Next(it);
    } /* while t */
    AB_Transaction_List2Iterator_free(it);
    pmtinf=AH_ImExporter_Sepa_PmtInf_List_Next(pmtinf);
  } /* while pmtinf  */
  AH_ImExporter_Sepa_PmtInf_List_free(pl);

  return 0;
}




