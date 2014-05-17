

#include <aqbanking/banking_be.h>



int AH_ImExporterSEPA_Export_Ccm(AB_IMEXPORTER *ie,
				 AB_IMEXPORTER_CONTEXT *ctx,
				 GWEN_SYNCIO *sio,
				 GWEN_DB_NODE *params){
  GWEN_XMLNODE *root;
  GWEN_XMLNODE *documentNode;
  GWEN_XMLNODE *painNode;
  GWEN_XMLNODE *n;
  AB_IMEXPORTER_ACCOUNTINFO *ai;
  const AB_TRANSACTION *t;
  AB_VALUE *v;
  int tcount=0;
  GWEN_XML_CONTEXT *xmlctx;
  int rv;

  ai=AB_ImExporterContext_GetFirstAccountInfo(ctx);
  if (ai==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "No account info");
    return GWEN_ERROR_NO_DATA;
  }

  v=AB_Value_new();
  t=AB_ImExporterAccountInfo_GetFirstTransaction(ai);
  while(t) {
    const AB_VALUE *tv;

    tv=AB_Transaction_GetValue(t);
    if (tv==NULL) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "No value in transaction");
      AB_Value_free(v);
      return GWEN_ERROR_BAD_DATA;
    }
    AB_Value_AddValue(v, tv);
    tcount++;

    t=AB_ImExporterAccountInfo_GetNextTransaction(ai);
  }

  t=AB_ImExporterAccountInfo_GetFirstTransaction(ai);

  root=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "?xml");
  if (n) {
    GWEN_XMLNode_AddHeader(root, n);
    GWEN_XMLNode_SetProperty(n, "version", "1.0");
    GWEN_XMLNode_SetProperty(n, "encoding", "UTF-8");
  }

  documentNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "Document");
  GWEN_XMLNode_SetProperty(documentNode,
			   "xmlns",
			   "urn:sepade:xsd:pain.001.001.02");
  GWEN_XMLNode_SetProperty(documentNode,
			   "xmlns:xsi",
			   "http://www.w3.org/2001/XMLSchema-instance");
  GWEN_XMLNode_SetProperty(documentNode,
			   "xsi:schemaLocation",
                           "urn:sepade:xsd:pain.001.001.02 pain.001.001.02.xsd");
  GWEN_XMLNode_AddChild(root, documentNode);

  painNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "pain.001.001.02");
  GWEN_XMLNode_AddChild(documentNode, painNode);

  /* create GrpHdr */
  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "GrpHdr");
  if (n) {
    GWEN_TIME *ti;
    GWEN_BUFFER *tbuf;
    uint32_t uid;
    char numbuf[32];
    GWEN_XMLNODE *nn;

    GWEN_XMLNode_AddChild(painNode, n);
    ti=GWEN_CurrentTime();

    tbuf=GWEN_Buffer_new(0, 64, 0, 1);

    /* generate MsgId */
    uid=AB_Banking_GetUniqueId(AB_ImExporter_GetBanking(ie));
    GWEN_Time_toUtcString(ti, "YYYYMMDD-hh:mm:ss-", tbuf);
    snprintf(numbuf, sizeof(numbuf)-1, "%08x", uid);
    GWEN_Buffer_AppendString(tbuf, numbuf);
    GWEN_XMLNode_SetCharValue(n, "MsgId", GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_Reset(tbuf);

    /* generate CreDtTm */
    GWEN_Time_toUtcString(ti, "YYYY-MM-DDThh:mm:ssZ", tbuf);
    GWEN_XMLNode_SetCharValue(n, "CreDtTm", GWEN_Buffer_GetStart(tbuf));
    GWEN_Time_free(ti);
    GWEN_Buffer_Reset(tbuf);

    /* store NbOfTxs */
    GWEN_XMLNode_SetIntValue(n, "NbOfTxs", tcount);

    /* store sum */
    AB_Value_toHumanReadableString2(v, tbuf, 2, 0);
    GWEN_XMLNode_SetCharValue(n, "CtrlSum", GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);

    GWEN_XMLNode_SetCharValue(n, "Grpg", "GRPD");

    nn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "InitgPty");
    if (nn) {
      const char *s;

      GWEN_XMLNode_AddChild(n, nn);
      s=AB_ImExporterAccountInfo_GetOwner(ai);
      if (!s)
        s=AB_Transaction_GetLocalName(t);
      if (!s) {
	DBG_ERROR(AQBANKING_LOGDOMAIN, "No owner");
	GWEN_XMLNode_free(root);
	AB_Value_free(v);
	return GWEN_ERROR_BAD_DATA;
      }
      GWEN_XMLNode_SetCharValue(nn, "Nm", s);

    }
  }
  AB_Value_free(v);

  /* generate PmtInf */
  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "PmtInf");
  if (n) {
    const GWEN_TIME *tti;
    GWEN_XMLNODE *nn;

    GWEN_XMLNode_AddChild(painNode, n);
    GWEN_XMLNode_SetCharValue(n, "PmtMtd", "TRF");
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
    tti=AB_Transaction_GetDate(t);
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
      const char *s;

      GWEN_XMLNode_AddChild(n, nn);

      s=AB_ImExporterAccountInfo_GetOwner(ai);
      if (!s)
        s=AB_Transaction_GetLocalName(t);
      if (!s) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "No owner");
        GWEN_XMLNode_free(root);
        return GWEN_ERROR_BAD_DATA;
      }

      GWEN_XMLNode_SetCharValue(nn, "Nm", s);
    }

    /* create "DbtrAcct" */
    nn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "DbtrAcct");
    if (nn) {
      const char *s;
      GWEN_XMLNODE *nnn;

      GWEN_XMLNode_AddChild(n, nn);
      s=AB_ImExporterAccountInfo_GetIban(ai);
      if (!s)
	s=AB_Transaction_GetLocalIban(t);
      if (!s) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "No local IBAN");
	GWEN_XMLNode_free(root);
	return GWEN_ERROR_BAD_DATA;
      }

      nnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "Id");
      if (nnn) {
	GWEN_XMLNode_AddChild(nn, nnn);
	GWEN_XMLNode_SetCharValue(nnn, "IBAN", s);
      }
    }

    /* create "DbtrAgt" */
    nn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "DbtrAgt");
    if (nn) {
      const char *s;
      GWEN_XMLNODE *nnn;

      GWEN_XMLNode_AddChild(n, nn);
      s=AB_ImExporterAccountInfo_GetBic(ai);
      if (!s)
        s=AB_Transaction_GetLocalBic(t);
      if (!s) {
	DBG_ERROR(AQBANKING_LOGDOMAIN, "No BIC");
	GWEN_XMLNode_free(root);
	return GWEN_ERROR_BAD_DATA;
      }

      nnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "FinInstnId");
      if (nnn) {
	GWEN_XMLNode_AddChild(nn, nnn);
	GWEN_XMLNode_SetCharValue(nnn, "BIC", s);
      }
    }

    GWEN_XMLNode_SetCharValue(n, "ChrgBr", "SLEV");


    t=AB_ImExporterAccountInfo_GetFirstTransaction(ai);
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
	  const char *s;

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
	  GWEN_XMLNode_free(root);
	  return GWEN_ERROR_BAD_DATA;
	}

	nnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "Amt");
	if (nnn) {
	  GWEN_XMLNODE *nnnn;

	  GWEN_XMLNode_AddChild(nn, nnn);

	  nnnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "InstdAmt");
	  if (nnnn) {
	    GWEN_BUFFER *tbuf;
	    const char *s;
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

	/* create "DbtrAgt" */
	nnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "CdtrAgt");
	if (nnn) {
	  const char *s;
	  GWEN_XMLNODE *nnnn;

	  GWEN_XMLNode_AddChild(nn, nnn);
	  s=AB_Transaction_GetRemoteBic(t);
	  if (!s) {
	    DBG_ERROR(AQBANKING_LOGDOMAIN, "No remote BIC");
	    GWEN_XMLNode_free(root);
	    return GWEN_ERROR_BAD_DATA;
	  }

	  nnnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "FinInstnId");
	  if (nnnn) {
	    GWEN_XMLNode_AddChild(nnn, nnnn);
	    GWEN_XMLNode_SetCharValue(nnnn, "BIC", s);
	  }
	}

	/* create "Cdtr" */
	nnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "Cdtr");
	if (nnn) {
	  const GWEN_STRINGLIST *sl;
	  const char *s=NULL;

	  GWEN_XMLNode_AddChild(nn, nnn);
	  sl=AB_Transaction_GetRemoteName(t);
	  if (sl)
	    s=GWEN_StringList_FirstString(sl);
	  if (!s) {
	    DBG_ERROR(AQBANKING_LOGDOMAIN, "No remote name");
	    GWEN_XMLNode_free(root);
	    return GWEN_ERROR_BAD_DATA;
	  }
	  GWEN_XMLNode_SetCharValue(nnn, "Nm", s);
	}

	/* create "CdtrAcct" */
	nnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "CdtrAcct");
	if (nnn) {
	  const char *s;
	  GWEN_XMLNODE *nnnn;

	  GWEN_XMLNode_AddChild(nn, nnn);
	  s=AB_Transaction_GetRemoteIban(t);
	  if (!s) {
	    DBG_ERROR(AQBANKING_LOGDOMAIN, "No remote IBAN");
	    GWEN_XMLNode_free(root);
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
	      const char *s;

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
	    GWEN_XMLNode_free(root);
	    return GWEN_ERROR_BAD_DATA;
	  }

	  GWEN_XMLNode_SetCharValue(nnn, "Ustrd", GWEN_Buffer_GetStart(tbuf));

	  GWEN_Buffer_free(tbuf);
	}
      }

      t=AB_ImExporterAccountInfo_GetNextTransaction(ai);
    } /* while t */
  }

  xmlctx=GWEN_XmlCtxStore_new(root,
			      GWEN_XML_FLAGS_DEFAULT |
                              GWEN_XML_FLAGS_SIMPLE |
			      GWEN_XML_FLAGS_HANDLE_HEADERS);

  rv=GWEN_XMLNode_WriteToStream(root, xmlctx, sio);
  if (rv) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_XmlCtx_free(xmlctx);
    GWEN_XMLNode_free(root);
    return rv;
  }
  GWEN_XmlCtx_free(xmlctx);
  GWEN_XMLNode_free(root);

  return 0;
}




