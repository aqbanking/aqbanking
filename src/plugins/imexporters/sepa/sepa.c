/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "sepa_p.h"
#include "i18n_l.h"

#include <aqbanking/banking.h>
#include <aqbanking/banking_be.h>
#include <aqbanking/accstatus.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/inherit.h>

#include <stdlib.h>
#include <ctype.h>



GWEN_LIST_FUNCTIONS(AH_IMEXPORTER_SEPA_PMTINF, AH_ImExporter_Sepa_PmtInf)
GWEN_INHERIT(AB_IMEXPORTER, AH_IMEXPORTER_SEPA);



static AH_IMEXPORTER_SEPA_PMTINF*
AH_ImExporter_Sepa_PmtInf_new() {
  AH_IMEXPORTER_SEPA_PMTINF *pmtinf;

  GWEN_NEW_OBJECT(AH_IMEXPORTER_SEPA_PMTINF, pmtinf)
  GWEN_LIST_INIT(AH_IMEXPORTER_SEPA_PMTINF, pmtinf)
  pmtinf->value=AB_Value_new();
  pmtinf->transactions=AB_Transaction_List2_new();

  return pmtinf;
}



static void AH_ImExporter_Sepa_PmtInf_free(AH_IMEXPORTER_SEPA_PMTINF *pmtinf) {
  if (pmtinf) {
    free(pmtinf->ctrlsum);
    AB_Value_free(pmtinf->value);
    AB_Transaction_List2_free(pmtinf->transactions);
    GWEN_LIST_FINI(AH_IMEXPORTER_SEPA_PMTINF, pmtinf)
    GWEN_FREE_OBJECT(pmtinf)
  }
}



GWEN_PLUGIN *imexporter_sepa_factory(GWEN_PLUGIN_MANAGER *pm,
				     const char *name,
				     const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=AB_Plugin_ImExporter_new(pm, name, fileName);
  assert(pl);

  AB_Plugin_ImExporter_SetFactoryFn(pl, AB_Plugin_ImExporterSEPA_Factory);

  return pl;
}



AB_IMEXPORTER *AB_Plugin_ImExporterSEPA_Factory(GWEN_PLUGIN *pl,
						AB_BANKING *ab){
  AB_IMEXPORTER *ie;
  AH_IMEXPORTER_SEPA *ieh;

  ie=AB_ImExporter_new(ab, "sepa");
  GWEN_NEW_OBJECT(AH_IMEXPORTER_SEPA, ieh);
  GWEN_INHERIT_SETDATA(AB_IMEXPORTER, AH_IMEXPORTER_SEPA, ie, ieh,
		       AH_ImExporterSEPA_FreeData);
  AB_ImExporter_SetImportFn(ie, AH_ImExporterSEPA_Import);
  AB_ImExporter_SetExportFn(ie, AH_ImExporterSEPA_Export);
  AB_ImExporter_SetCheckFileFn(ie, AH_ImExporterSEPA_CheckFile);
  return ie;
}



void GWENHYWFAR_CB AH_ImExporterSEPA_FreeData(void *bp, void *p){
  AH_IMEXPORTER_SEPA *ieh;

  ieh=(AH_IMEXPORTER_SEPA*)p;
  GWEN_FREE_OBJECT(ieh);
}



int AH_ImExporterSEPA_Import(AB_IMEXPORTER *ie,
			     AB_IMEXPORTER_CONTEXT *ctx,
			     GWEN_SYNCIO *sio,
			     GWEN_DB_NODE *params){
  AH_IMEXPORTER_SEPA *ieh;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_SEPA, ie);
  assert(ieh);


  return GWEN_ERROR_NOT_SUPPORTED;
}



static int
AH_ImExporterSEPA_Export_Pain_Setup(AB_IMEXPORTER *ie,
				    AB_IMEXPORTER_CONTEXT *ctx,
				    GWEN_XMLNODE *painNode,
				    uint32_t doctype[],
				    AH_IMEXPORTER_SEPA_PMTINF_LIST **pList) {
  GWEN_XMLNODE *n;
  AB_IMEXPORTER_ACCOUNTINFO *ai;
  AB_TRANSACTION *t;
  AH_IMEXPORTER_SEPA_PMTINF_LIST *pl;
  AH_IMEXPORTER_SEPA_PMTINF *pmtinf;
  int tcount=0;
  AB_VALUE *v;
  GWEN_BUFFER *tbuf;
  char *ctrlsum;

  ai=AB_ImExporterContext_GetFirstAccountInfo(ctx);
  if (ai==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "No account info");
    return GWEN_ERROR_NO_DATA;
  }
  else if (AB_ImExporterContext_GetNextAccountInfo(ctx)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Account info for more than one local account");
    return GWEN_ERROR_NOT_SUPPORTED;
  }

  t=AB_ImExporterAccountInfo_GetFirstTransaction(ai);
  if (!t) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "No transactions in ImExporter context");
    return GWEN_ERROR_NO_DATA;
  }

  /* collect matching transactions for storage in a shared PmtInf block */
  pl=AH_ImExporter_Sepa_PmtInf_List_new();
  pmtinf=AH_ImExporter_Sepa_PmtInf_new();
  AH_ImExporter_Sepa_PmtInf_List_Add(pmtinf, pl);
  while(t) {
    const GWEN_TIME *ti;
    int day, month, year;
    uint32_t transDate;
    const char *name=NULL, *iban=NULL, *bic=NULL, *cdtrSchmeId=NULL;
    AB_TRANSACTION_SEQUENCETYPE sequenceType=AB_Transaction_SequenceTypeUnknown;
    const char *s;
    const AB_VALUE *tv;

    tcount++;
    ti=AB_Transaction_GetDate(t);
    if (ti) {
      GWEN_Time_GetBrokenDownDate(ti, &day, &month, &year);
      transDate=(year<<16)+(month<<8)+(day);
    }
    else
      transDate=0;
    s=AB_ImExporterAccountInfo_GetOwner(ai);
    if (!s || !*s) {
      name=AB_Transaction_GetLocalName(t);
      if (!name || !*name) {
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "Missing local name in transaction %d", tcount);
	AH_ImExporter_Sepa_PmtInf_List_free(pl);
	return GWEN_ERROR_BAD_DATA;
      }
    }
    s=AB_ImExporterAccountInfo_GetIban(ai);
    if (!s || !*s) {
      iban=AB_Transaction_GetLocalIban(t);
      if (!iban || !*iban) {
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "Missing local IBAN in transaction %d", tcount);
	AH_ImExporter_Sepa_PmtInf_List_free(pl);
	return GWEN_ERROR_BAD_DATA;
      }
    }
    s=AB_ImExporterAccountInfo_GetBic(ai);
    if (!s || !*s) {
      bic=AB_Transaction_GetLocalBic(t);
      if (!bic || !*bic) {
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "Missing local BIC in transaction %d", tcount);
	AH_ImExporter_Sepa_PmtInf_List_free(pl);
	return GWEN_ERROR_BAD_DATA;
      }
    }
    if (doctype[0]==8) {
      sequenceType=AB_Transaction_GetSequenceType(t);
      if (sequenceType==AB_Transaction_SequenceTypeUnknown) {
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "Missing sequence type in transaction %d", tcount);
	AH_ImExporter_Sepa_PmtInf_List_free(pl);
	return GWEN_ERROR_BAD_DATA;
      }
      cdtrSchmeId=AB_Transaction_GetCreditorSchemeId(t);
      if (!cdtrSchmeId || !*cdtrSchmeId) {
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "Missing creditor scheme id in transaction %d", tcount);
	AH_ImExporter_Sepa_PmtInf_List_free(pl);
	return GWEN_ERROR_BAD_DATA;
      }
    }

    if (pmtinf->tcount) {
      /* specify list of match criteria in one place */
#define TRANSACTION_DOES_NOT_MATCH					\
      (transDate!=pmtinf->transDate ||					\
       (name && strcmp(name, pmtinf->localName)) ||			\
       (iban && strcmp(iban, pmtinf->localIban)) ||			\
       (bic && strcmp(bic, pmtinf->localBic)) ||			\
       (doctype[0]==8 &&						\
	(sequenceType!=pmtinf->sequenceType ||				\
	 (cdtrSchmeId && strcmp(cdtrSchmeId, pmtinf->creditorSchemeId)))))

      /* match against current PmtInf block */
      if (TRANSACTION_DOES_NOT_MATCH) {
	/* search for a fitting PmtInf block */
	pmtinf=AH_ImExporter_Sepa_PmtInf_List_First(pl);
	while(pmtinf && TRANSACTION_DOES_NOT_MATCH)
	  pmtinf=AH_ImExporter_Sepa_PmtInf_List_Next(pmtinf);
#undef TRANSACTION_DOES_NOT_MATCH

	if (!pmtinf) {
	  pmtinf=AH_ImExporter_Sepa_PmtInf_new(t);
	  AH_ImExporter_Sepa_PmtInf_List_Add(pmtinf, pl);
	}
      }
    }

    if (!pmtinf->tcount) {
      /* initialise match data for this PmtInf block */
      pmtinf->localName = name ? name : AB_ImExporterAccountInfo_GetOwner(ai);
      pmtinf->localIban = iban ? iban : AB_ImExporterAccountInfo_GetIban(ai);
      pmtinf->localBic  = bic  ? bic  : AB_ImExporterAccountInfo_GetBic(ai);
      pmtinf->date=ti;
      pmtinf->transDate=transDate;
      if (doctype[0]==8) {
	pmtinf->sequenceType=sequenceType;
	pmtinf->creditorSchemeId=cdtrSchmeId;
      }
    }

    AB_Transaction_List2_PushBack(pmtinf->transactions, t);
    pmtinf->tcount++;
    tv=AB_Transaction_GetValue(t);
    if (tv==NULL) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Missing value in transaction %d", tcount);
      AH_ImExporter_Sepa_PmtInf_List_free(pl);
      return GWEN_ERROR_BAD_DATA;
    }
    AB_Value_AddValue(pmtinf->value, tv);

    t=AB_ImExporterAccountInfo_GetNextTransaction(ai);
  }

  /* construct CtrlSum for PmtInf blocks and GrpHdr */
  v=AB_Value_new();
  tbuf=GWEN_Buffer_new(0, 64, 0, 1);
  pmtinf=AH_ImExporter_Sepa_PmtInf_List_First(pl);
  while(pmtinf) {
    AB_Value_toHumanReadableString2(pmtinf->value, tbuf, 2, 0);
    pmtinf->ctrlsum=strdup(GWEN_Buffer_GetStart(tbuf));
    assert(pmtinf->ctrlsum);
    GWEN_Buffer_Reset(tbuf);
    AB_Value_AddValue(v, pmtinf->value);
    pmtinf=AH_ImExporter_Sepa_PmtInf_List_Next(pmtinf);
  }

  AB_Value_toHumanReadableString2(v, tbuf, 2, 0);
  ctrlsum=strdup(GWEN_Buffer_GetStart(tbuf));
  assert(ctrlsum);
  GWEN_Buffer_free(tbuf);
  AB_Value_free(v);

  /* create GrpHdr */
  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "GrpHdr");
  if (n) {
    GWEN_TIME *ti;
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
    GWEN_Buffer_free(tbuf);

    /* store NbOfTxs */
    GWEN_XMLNode_SetIntValue(n, "NbOfTxs", tcount);
    /* store CtrlSum */
    GWEN_XMLNode_SetCharValue(n, "CtrlSum", ctrlsum);

    /* special treatment for pain.001.001.02 and pain.008.001.01 */
    if (doctype[1]==1 && ((doctype[0]==1 && doctype[2]==2) ||
			  (doctype[0]==8 && doctype[2]==1)))
      GWEN_XMLNode_SetCharValue(n, "Grpg", "GRPD");

    nn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "InitgPty");
    if (nn) {
      GWEN_XMLNode_AddChild(n, nn);
      pmtinf=AH_ImExporter_Sepa_PmtInf_List_First(pl);
      GWEN_XMLNode_SetCharValue(nn, "Nm", pmtinf->localName);
    }
  }
  free(ctrlsum);

  *pList=pl;
  return 0;
}



int AH_ImExporterSEPA_Export(AB_IMEXPORTER *ie,
			     AB_IMEXPORTER_CONTEXT *ctx,
			     GWEN_SYNCIO *sio,
			     GWEN_DB_NODE *params){
  AH_IMEXPORTER_SEPA *ieh;
  GWEN_XMLNODE *root;
  GWEN_XMLNODE *documentNode;
  GWEN_XMLNODE *topNode;
  GWEN_XMLNODE *n;
  uint32_t doctype[]={0, 0, 0};
  const char *s;
  int rv;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_SEPA, ie);
  assert(ieh);

  s=GWEN_DB_GetCharValue(params, "type", 0, 0);
  if (s) {
    int i, j;
    const char *p;
    char *tail;

    /* Parse strings of the form xxx.yyy.zz */
    p=tail=(char*)s;
    for (i=0; i<3 && *tail; i++) {
      j=strtol(p, &tail, 10);
      if (!isspace(*p) &&
	  ((*tail=='.' && tail-p==3) || (*tail=='\0' && tail-p==2)) &&
	  j>0)
	doctype[i]=j;
      else
	break;
      p=tail+1;
    }
    if (i<3)
      /* Parsing the "type" option failed, record it for later reference */
      doctype[0]=0;
  }

  root=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "?xml");
  if (n) {
    GWEN_XMLNode_AddHeader(root, n);
    GWEN_XMLNode_SetProperty(n, "version", "1.0");
    GWEN_XMLNode_SetProperty(n, "encoding", "UTF-8");
  }

  documentNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "Document");
  s=GWEN_DB_GetCharValue(params, "xmlns", 0, 0);
  if (!s || !*s) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "xmlns not specified in profile \"%s\"",
	      GWEN_DB_GetCharValue(params, "name", 0, 0));
    GWEN_XMLNode_free(root);
    return GWEN_ERROR_INVALID;
  }
  GWEN_XMLNode_SetProperty(documentNode, "xmlns", s);
  GWEN_XMLNode_AddChild(root, documentNode);

  switch(doctype[0]) {
  case 1:
    if (doctype[1]>1 || doctype[2]>2)
      s="CstmrCdtTrfInitn";
    else
      s=strstr(s, "pain");
    topNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, s);
    GWEN_XMLNode_AddChild(documentNode, topNode);
    rv=AH_ImExporterSEPA_Export_Pain_001(ie, ctx, topNode, doctype, params);
    break;
  case 8:
    if (!(doctype[1]==1 && doctype[2]==1))
      s="CstmrDrctDbtInitn";
    else
      s=strstr(s, "pain");
    topNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, s);
    GWEN_XMLNode_AddChild(documentNode, topNode);
    rv=AH_ImExporterSEPA_Export_Pain_008(ie, ctx, topNode, doctype, params);
    break;
  default:
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unknown SEPA type \"%s\"",
	      GWEN_DB_GetCharValue(params, "type", 0, 0));
    GWEN_XMLNode_free(root);
    return GWEN_ERROR_INVALID;
  }

  if (rv==0) {
    GWEN_XML_CONTEXT *xmlctx;

    xmlctx=GWEN_XmlCtxStore_new(root,
				GWEN_XML_FLAGS_INDENT |
				GWEN_XML_FLAGS_SIMPLE |
				GWEN_XML_FLAGS_HANDLE_HEADERS);

    rv=GWEN_XMLNode_WriteToStream(root, xmlctx, sio);
    if (rv)
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_XmlCtx_free(xmlctx);
    GWEN_XMLNode_free(root);
  }

  /* TODO */
  return rv;
}



int AH_ImExporterSEPA_CheckFile(AB_IMEXPORTER *ie, const char *fname){
  AH_IMEXPORTER_SEPA *ieh;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_SEPA, ie);
  assert(ieh);

#if 0
  return AB_ERROR_INDIFFERENT;
#else
  /* TODO */
  return GWEN_ERROR_NOT_IMPLEMENTED;
#endif
}





#include "sepa_pain_001.c"
#include "sepa_pain_008.c"


