

#include <aqbanking/banking_be.h>



int AH_ImExporterSEPA_Export_008_003_02(AB_IMEXPORTER *ie,
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
  const char *s;
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
			   "urn:iso:std:iso:20022:tech:xsd:pain.008.003.02");
  GWEN_XMLNode_SetProperty(documentNode,
			   "xmlns:xsi",
			   "http://www.w3.org/2001/XMLSchema-instance");
  GWEN_XMLNode_SetProperty(documentNode,
			   "xsi:schemaLocation",
			   "urn:iso:std:iso:20022:tech:xsd:pain.008.003.02 pain.008.003.02.xsd");
  GWEN_XMLNode_AddChild(root, documentNode);

  painNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "CstmrDrctDbtInitn");
  GWEN_XMLNode_AddChild(documentNode, painNode);

  /* create GrpHdr */
  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "GrpHdr");
  if (n) {
    GWEN_TIME *ti;
    GWEN_BUFFER *tbuf;
    GWEN_XMLNODE *nn;
    uint32_t uid;
    char numbuf[32];

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
    GWEN_Buffer_Reset(tbuf);

    /* store NbOfTxs */
    GWEN_XMLNode_SetIntValue(n, "NbOfTxs", tcount);

    /* InitgPty */
    nn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "InitgPty");
    if (nn) {
      s=AB_ImExporterAccountInfo_GetOwner(ai);
      if (!s)
        s=AB_Transaction_GetLocalName(t);
      if (!s) {
	DBG_ERROR(AQBANKING_LOGDOMAIN, "No owner");
	AB_Value_free(v);
	return GWEN_ERROR_BAD_DATA;
      }
      GWEN_XMLNode_SetCharValue(nn, "Nm", s);

      GWEN_XMLNode_AddChild(n, nn);
    }


    GWEN_Buffer_free(tbuf);
  }

  /* generate PmtInf */
  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "PmtInf");
  if (n) {
    const GWEN_TIME *tti;
    GWEN_XMLNODE *nn;

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

    GWEN_XMLNode_SetCharValue(n, "PmtMtd", "DD");

    /* store NbOfTxs */
    GWEN_XMLNode_SetIntValue(n, "NbOfTxs", tcount);
    /* CtrlSum */
    if (1) {
        GWEN_BUFFER *tbuf;

      /* store sum */
      tbuf=GWEN_Buffer_new(0, 64, 0, 1);
      AB_Value_toHumanReadableString2(v, tbuf, 2, 0);
      GWEN_XMLNode_SetCharValue(n, "CtrlSum", GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_free(tbuf);
    }

    /* PmtTpInf */
    nn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "PmtTpInf");
    if (nn) {
      GWEN_XMLNODE *nnn;

      nnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "SvcLvl");
      if (nnn) {
	GWEN_XMLNode_SetCharValue(nnn, "Cd", "SEPA");
	GWEN_XMLNode_AddChild(nn, nnn);
      }

      nnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "LclInstrm");
      if (nnn) {
	GWEN_XMLNode_SetCharValue(nnn, "Cd", "CORE");
	GWEN_XMLNode_AddChild(nn, nnn);
      }

      switch(AB_Transaction_GetSequenceType(t)) {
      case AB_Transaction_SequenceTypeUnknown:
      case AB_Transaction_SequenceTypeOnce:
        GWEN_XMLNode_SetCharValue(nn, "SeqTp", "OOFF");
        break;
      case AB_Transaction_SequenceTypeFirst:
        GWEN_XMLNode_SetCharValue(nn, "SeqTp", "FRST");
        break;
      case AB_Transaction_SequenceTypeFollowing:
        GWEN_XMLNode_SetCharValue(nn, "SeqTp", "RCUR");
        break;
      case AB_Transaction_SequenceTypeFinal:
        GWEN_XMLNode_SetCharValue(nn, "SeqTp", "FNAL");
        break;
      }

      GWEN_XMLNode_AddChild(n, nn);
    }

    /* create ReqdColltnDt" */
    tti=AB_Transaction_GetDate(t);
    if (tti) {
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, 64, 0, 1);
      GWEN_Time_toString(tti, "YYYY-MM-DD", tbuf);
      GWEN_XMLNode_SetCharValue(n, "ReqdColltnDt", GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_free(tbuf);
    }
    else {
      GWEN_XMLNode_SetCharValue(n, "ReqdColltnDt", "1999-01-01");
    }

    /* create "Cdtr" */
    nn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "Cdtr");
    if (nn) {
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

    /* create "CdtrAcct" */
    s=AB_ImExporterAccountInfo_GetIban(ai);
    if (!s)
      s=AB_Transaction_GetLocalIban(t);
    if (!s) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "No local IBAN");
      GWEN_XMLNode_free(root);
      return GWEN_ERROR_BAD_DATA;
    }
    GWEN_XMLNode_SetCharValueByPath(n, GWEN_XML_PATH_FLAGS_OVERWRITE_VALUES, "CdtrAcct/Id/IBAN", s);

    /* create "CdtrAgt" */
    s=AB_ImExporterAccountInfo_GetBic(ai);
    if (!s)
      s=AB_Transaction_GetLocalBic(t);
    if (!s) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "No BIC");
      GWEN_XMLNode_free(root);
      return GWEN_ERROR_BAD_DATA;
    }
    GWEN_XMLNode_SetCharValueByPath(n, GWEN_XML_PATH_FLAGS_OVERWRITE_VALUES, "CdtrAgt/FinInstnId/BIC", s);

    GWEN_XMLNode_SetCharValue(n, "ChrgBr", "SLEV");

    /* create "CdtrSchmeId" */
    nn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "CdtrSchmeId");
    if (nn) {
      GWEN_XMLNode_AddChild(n, nn);

      s=AB_Transaction_GetCreditorSchemeId(t);

      if (!s) {
	DBG_ERROR(AQBANKING_LOGDOMAIN, "No CreditorSchemeId");
        GWEN_XMLNode_free(root);
	AB_Value_free(v);
	return GWEN_ERROR_BAD_DATA;
      }
      GWEN_XMLNode_SetCharValueByPath(nn, GWEN_XML_PATH_FLAGS_OVERWRITE_VALUES, "Id/PrvtId/Othr/Id", s);
      GWEN_XMLNode_SetCharValueByPath(nn, GWEN_XML_PATH_FLAGS_OVERWRITE_VALUES, "Id/PrvtId/Othr/SchmeNm/Prtry", "SEPA");
    }


    /* DrctDbtTxInf */
    t=AB_ImExporterAccountInfo_GetFirstTransaction(ai);
    while(t) {
      GWEN_XMLNODE *nn;

      nn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "DrctDbtTxInf");
      if (nn) {
	GWEN_XMLNODE *nnn;
	const AB_VALUE *tv;

	GWEN_XMLNode_AddChild(n, nn);

	/* create "PmtId/EndToEndId" */
	s=AB_Transaction_GetEndToEndReference(t);
	if (!( s && *s))
	  s=AB_Transaction_GetCustomerReference(t);
	if (!s)
	  s="NOTPROVIDED";
        GWEN_XMLNode_SetCharValueByPath(nn, GWEN_XML_PATH_FLAGS_OVERWRITE_VALUES, "PmtId/EndToEndId", s);

	tv=AB_Transaction_GetValue(t);
	if (tv==NULL) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN, "No value in transaction");
          GWEN_XMLNode_free(root);
          AB_Value_free(v);
	  return GWEN_ERROR_BAD_DATA;
	}

	nnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "InstdAmt");
	if (nnn) {
	  GWEN_BUFFER *tbuf;
	  GWEN_XMLNODE *nnnn;

	  GWEN_XMLNode_AddChild(nn, nnn);

	  tbuf=GWEN_Buffer_new(0, 64, 0, 1);
	  AB_Value_toHumanReadableString2(tv, tbuf, 2, 0);
	  s=AB_Value_GetCurrency(tv);
	  if (!s)
	    s="EUR";
	  GWEN_XMLNode_SetProperty(nnn, "Ccy", s);

	  nnnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeData, GWEN_Buffer_GetStart(tbuf));
	  GWEN_XMLNode_AddChild(nnn, nnnn);
	  GWEN_Buffer_free(tbuf);
	}

	/* DrctDbtTx */
	nnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "DrctDbtTx");
	if (nnn) {
          GWEN_XMLNODE *nnnn;

          GWEN_XMLNode_AddChild(nn, nnn);

	  /* add mandate info */
	  nnnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "MndtRltdInf");
	  if (nnnn) {
	    const char *credSchemId;
	    const char *mandateId;
	    const char *origCredSchemId;
	    const char *origMandateId;
	    const char *origCreditorName;
	    const GWEN_DATE *dt;
	    GWEN_BUFFER *tbuf;

            GWEN_XMLNode_AddChild(nnn, nnnn);

	    credSchemId=AB_Transaction_GetCreditorSchemeId(t);
	    if (!credSchemId) {
	      DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing Creditor Scheme Id");
              GWEN_XMLNode_free(root);
              AB_Value_free(v);
	      return GWEN_ERROR_BAD_DATA;
	    }
  
	    dt=AB_Transaction_GetMandateDate(t);
	    if (!dt) {
	      DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing mandate date for direct debit");
              GWEN_XMLNode_free(root);
              AB_Value_free(v);
	      return GWEN_ERROR_BAD_DATA;
	    }
  
	    mandateId=AB_Transaction_GetMandateId(t);
	    if (!mandateId) {
	      DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing mandate id for direct debit");
              GWEN_XMLNode_free(root);
              AB_Value_free(v);
	      return GWEN_ERROR_BAD_DATA;
	    }

            origCredSchemId=AB_Transaction_GetOriginalCreditorSchemeId(t);
	    origMandateId=AB_Transaction_GetOriginalMandateId(t);
	    origCreditorName=AB_Transaction_GetOriginalCreditorName(t);

	    /* MndtId */
	    GWEN_XMLNode_SetCharValue(nnnn, "MndtId", mandateId);

	    /* DtOfSgntr */
	    tbuf=GWEN_Buffer_new(0, 32, 0, 1);
	    rv=GWEN_Date_toStringWithTemplate(dt, "YYYY-MM-DD", tbuf);
	    if (rv<0) {
	      DBG_ERROR(AQBANKING_LOGDOMAIN, "Error converting date to string");
	      GWEN_Buffer_free(tbuf);
              GWEN_XMLNode_free(root);
              AB_Value_free(v);
	      return rv;
	    }
	    GWEN_XMLNode_SetCharValue(nnnn, "DtOfSgntr", GWEN_Buffer_GetStart(tbuf));

	    if ((origCredSchemId && *origCredSchemId) ||
		(origMandateId && *origMandateId) ||
		(origCreditorName && *origCreditorName)) {
	      GWEN_XMLNODE *n5;

	      GWEN_XMLNode_SetCharValue(nnnn, "AmdmntInd", "true");

	      n5=GWEN_XMLNode_GetNodeByXPath(nnnn, "AmdmntInfDtls/OrgnlCdtrSchmeId", 0);
	      if (n5) {

                if (origMandateId && *origMandateId)
                  GWEN_XMLNode_SetCharValue(n5, "OrgnlMndtId", origMandateId);

                if (origCreditorName && *origCreditorName)
                  GWEN_XMLNode_SetCharValue(n5, "Nm", origCreditorName);

                if (origCredSchemId && *origCredSchemId) {
                  GWEN_XMLNode_SetCharValueByPath(n5, GWEN_XML_PATH_FLAGS_OVERWRITE_VALUES, "Id/PrvtId/Othr/Id", origCredSchemId);
                  GWEN_XMLNode_SetCharValueByPath(n5, GWEN_XML_PATH_FLAGS_OVERWRITE_VALUES, "Id/PrvtId/Othr/SchmeNm/Prtry", "SEPA");
		}
	      }
	    }
	    else {
	      GWEN_XMLNode_SetCharValue(nnnn, "AmdmntInd", "false");
	    }
	  }
	}

	/* create "DbtrAgt" */
	nnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "DbtrAgt");
	if (nnn) {
	  GWEN_XMLNODE *nnnn;

	  GWEN_XMLNode_AddChild(nn, nnn);
	  s=AB_Transaction_GetRemoteBic(t);
	  if (!s) {
	    DBG_ERROR(AQBANKING_LOGDOMAIN, "No remote BIC");
            GWEN_XMLNode_free(root);
            AB_Value_free(v);
	    return GWEN_ERROR_BAD_DATA;
	  }

	  nnnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "FinInstnId");
	  if (nnnn) {
	    GWEN_XMLNode_AddChild(nnn, nnnn);
	    GWEN_XMLNode_SetCharValue(nnnn, "BIC", s);
	  }
	}

	/* create "Dbtr" */
	nnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "Dbtr");
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
            AB_Value_free(v);
	    return GWEN_ERROR_BAD_DATA;
	  }
	  GWEN_XMLNode_SetCharValue(nnn, "Nm", s);
	}

	/* create "DbtrAcct" */
	nnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "DbtrAcct");
	if (nnn) {
	  GWEN_XMLNODE *nnnn;

	  GWEN_XMLNode_AddChild(nn, nnn);
	  s=AB_Transaction_GetRemoteIban(t);
	  if (!s) {
	    DBG_ERROR(AQBANKING_LOGDOMAIN, "No remote IBAN");
            GWEN_XMLNode_free(root);
            AB_Value_free(v);
	    return GWEN_ERROR_BAD_DATA;
	  }

	  nnnn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "Id");
	  if (nnnn) {
	    GWEN_XMLNode_AddChild(nnn, nnnn);
	    GWEN_XMLNode_SetCharValue(nnnn, "IBAN", s);
	  }
	}

	/* add "Ultimate Debitor Name", if given */
	s=AB_Transaction_GetMandateDebitorName(t);
	if (s && *s)
          GWEN_XMLNode_SetCharValueByPath(nn, GWEN_XML_PATH_FLAGS_OVERWRITE_VALUES, "UltmtDbtr/Nm", s);

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
            GWEN_XMLNode_free(root);
            AB_Value_free(v);
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
                              GWEN_XML_FLAGS_INDENT |
                              GWEN_XML_FLAGS_SIMPLE |
			      GWEN_XML_FLAGS_HANDLE_HEADERS);

  rv=GWEN_XMLNode_WriteToStream(root, xmlctx, sio);
  if (rv) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_XmlCtx_free(xmlctx);
    GWEN_XMLNode_free(root);
    AB_Value_free(v);
    return rv;
  }
  GWEN_XmlCtx_free(xmlctx);
  GWEN_XMLNode_free(root);
  AB_Value_free(v);

  return 0;
}




