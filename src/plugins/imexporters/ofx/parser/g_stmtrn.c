/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "g_stmtrn_p.h"
#include "ofxxmlctx_l.h"
#include "i18n_l.h"

#include "g_generic_l.h"
#include "g_ignore_l.h"
#include "g_bankacc_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/gui.h>



GWEN_INHERIT(AIO_OFX_GROUP, AIO_OFX_GROUP_STMTRN)




AIO_OFX_GROUP *AIO_OfxGroup_STMTRN_new(const char *groupName,
				       AIO_OFX_GROUP *parent,
				       GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_GROUP *g;
  AIO_OFX_GROUP_STMTRN *xg;

  /* create base group */
  g=AIO_OfxGroup_Generic_new(groupName, parent, ctx);
  assert(g);

  GWEN_NEW_OBJECT(AIO_OFX_GROUP_STMTRN, xg);
  assert(xg);
  GWEN_INHERIT_SETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_STMTRN, g, xg,
                       AIO_OfxGroup_STMTRN_FreeData);

  xg->transaction=AB_Transaction_new();

  /* set virtual functions */
  AIO_OfxGroup_SetStartTagFn(g, AIO_OfxGroup_STMTRN_StartTag);
  AIO_OfxGroup_SetAddDataFn(g, AIO_OfxGroup_STMTRN_AddData);
  AIO_OfxGroup_SetEndSubGroupFn(g, AIO_OfxGroup_STMTRN_EndSubGroup);

  return g;
}



GWENHYWFAR_CB
void AIO_OfxGroup_STMTRN_FreeData(void *bp, void *p) {
  AIO_OFX_GROUP_STMTRN *xg;

  xg=(AIO_OFX_GROUP_STMTRN*)p;
  assert(xg);
  free(xg->currentElement);
  AB_Transaction_free(xg->transaction);

  GWEN_FREE_OBJECT(xg);
}



AB_TRANSACTION *AIO_OfxGroup_STMTRN_TakeTransaction(const AIO_OFX_GROUP *g){
  AIO_OFX_GROUP_STMTRN *xg;
  AB_TRANSACTION *t;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_STMTRN, g);
  assert(xg);

  t=xg->transaction;
  xg->transaction=NULL;
  return t;
}



int AIO_OfxGroup_STMTRN_StartTag(AIO_OFX_GROUP *g,
				 const char *tagName) {
  AIO_OFX_GROUP_STMTRN *xg;
  GWEN_XML_CONTEXT *ctx;
  AIO_OFX_GROUP *gNew=NULL;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_STMTRN, g);
  assert(xg);

  ctx=AIO_OfxGroup_GetXmlContext(g);

  if (strcasecmp(tagName, "TRNTYPE")==0 ||
      strcasecmp(tagName, "DTPOSTED")==0 ||
      strcasecmp(tagName, "DTUSER")==0 ||
      strcasecmp(tagName, "DTAVAIL")==0 ||
      strcasecmp(tagName, "TRNAMT")==0 ||
      strcasecmp(tagName, "FITID")==0 ||
      strcasecmp(tagName, "CORRECTFITID")==0 ||
      strcasecmp(tagName, "CORRECTATION")==0 ||
      strcasecmp(tagName, "SRVTID")==0 ||
      strcasecmp(tagName, "CHECKNUM")==0 ||
      strcasecmp(tagName, "REFNUM")==0 ||
      strcasecmp(tagName, "SIC")==0 ||
      strcasecmp(tagName, "PAYEEID")==0 ||
      strcasecmp(tagName, "NAME")==0 ||
      strcasecmp(tagName, "MEMO")==0) {
    free(xg->currentElement);
    xg->currentElement=strdup(tagName);
  }
  else if (strcasecmp(tagName, "BANKACCTTO")==0) {
    gNew=AIO_OfxGroup_BANKACC_new(tagName, g, ctx);
  }
  else if (strcasecmp(tagName, "CCACCTTO")==0) {
    /* TODO */
    gNew=AIO_OfxGroup_Ignore_new(tagName, g, ctx);
  }
  else if (strcasecmp(tagName, "PAYEE")==0) {
    /* TODO */
    gNew=AIO_OfxGroup_Ignore_new(tagName, g, ctx);
  }
  else if (strcasecmp(tagName, "CURRENCY")==0) {
    /* TODO */
    gNew=AIO_OfxGroup_Ignore_new(tagName, g, ctx);
  }
  else if (strcasecmp(tagName, "ORIGCURRENCY")==0) {
    /* TODO */
    gNew=AIO_OfxGroup_Ignore_new(tagName, g, ctx);
  }
  else {
    DBG_WARN(AQBANKING_LOGDOMAIN,
	     "Ignoring tag [%s]", tagName);
    /*gNew=AIO_OfxGroup_Ignore_new(tagName, g, ctx);*/
    free(xg->currentElement);
    xg->currentElement=strdup(tagName);
  }

  if (gNew) {
    AIO_OfxXmlCtx_SetCurrentGroup(ctx, gNew);
    GWEN_XmlCtx_IncDepth(ctx);
  }

  return 0;
}



int AIO_OfxGroup_STMTRN_AddData(AIO_OFX_GROUP *g, const char *data) {
  AIO_OFX_GROUP_STMTRN *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_STMTRN, g);
  assert(xg);

  if (xg->currentElement) {
    GWEN_BUFFER *buf;
    int rv;
    const char *s;

    buf=GWEN_Buffer_new(0, strlen(data), 0, 1);
    rv=AIO_OfxXmlCtx_SanitizeData(AIO_OfxGroup_GetXmlContext(g), data, buf);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(buf);
      return rv;
    }
    s=GWEN_Buffer_GetStart(buf);
    if (*s) {
      DBG_INFO(AQBANKING_LOGDOMAIN,
	       "AddData: %s=[%s]", xg->currentElement, s);
      if (strcasecmp(xg->currentElement, "TRNTYPE")==0) {
	AB_TRANSACTION *t;

        t=xg->transaction;
	if (strcasecmp(s, "CREDIT")==0) {
	  AB_Transaction_SetTransactionKey(t, "MSC");
	  AB_Transaction_SetTransactionText(t, I18N("Generic credit"));
	}
	else if (strcasecmp(s, "DEBIT")==0) {
	  AB_Transaction_SetTransactionKey(t, "MSC");
	  AB_Transaction_SetTransactionText(t, I18N("Generic debit"));
	}
	else if (strcasecmp(s, "INT")==0) {
	  AB_Transaction_SetTransactionKey(t, "INT");
	  AB_Transaction_SetTransactionText(t, I18N("Interest earned or paid (Note: Depends on signage of amount)"));
	}
	else if (strcasecmp(s, "DIV")==0) {
	  AB_Transaction_SetTransactionKey(t, "DIV");
	  AB_Transaction_SetTransactionText(t, I18N("Dividend"));
	}
	else if (strcasecmp(s, "FEE")==0) {
	  AB_Transaction_SetTransactionKey(t, "BRF");
	  AB_Transaction_SetTransactionText(t, I18N("FI fee"));
	}
	else if (strcasecmp(s, "SRVCHG")==0) {
	  AB_Transaction_SetTransactionKey(t, "CHG");
	  AB_Transaction_SetTransactionText(t, I18N("Service charge"));
	}
	else if (strcasecmp(s, "DEP")==0) {
	  AB_Transaction_SetTransactionKey(t, "LDP"); /* FIXME: not sure */
	  AB_Transaction_SetTransactionText(t, I18N("Deposit"));
	}
	else if (strcasecmp(s, "ATM")==0) {
	  AB_Transaction_SetTransactionKey(t, "MSC"); /* misc */
	  AB_Transaction_SetTransactionText(t, I18N("ATM debit or credit (Note: Depends on signage of amount)"));
	}
	else if (strcasecmp(s, "POS")==0) {
	  AB_Transaction_SetTransactionKey(t, "MSC"); /* misc */
	  AB_Transaction_SetTransactionText(t, I18N("Point of sale debit or credit (Note: Depends on signage of amount)"));
	}
	else if (strcasecmp(s, "XFER")==0) {
	  AB_Transaction_SetTransactionKey(t, "TRF");
	  AB_Transaction_SetTransactionText(t, I18N("Transfer"));
	}
	else if (strcasecmp(s, "CHECK")==0) {
	  AB_Transaction_SetTransactionKey(t, "CHK");
	  AB_Transaction_SetTransactionText(t, I18N("Check"));
	}
	else if (strcasecmp(s, "PAYMENT")==0) {
	  AB_Transaction_SetTransactionKey(t, "TRF"); /* FIXME: not sure */
	  AB_Transaction_SetTransactionText(t, I18N("Electronic payment"));
	}
	else if (strcasecmp(s, "CASH")==0) {
	  AB_Transaction_SetTransactionKey(t, "MSC"); /* FIXME: not sure */
	  AB_Transaction_SetTransactionText(t, I18N("Cash withdrawal"));
	}
	else if (strcasecmp(s, "DIRECTDEP")==0) {
	  AB_Transaction_SetTransactionKey(t, "LDP"); /* FIXME: not sure */
	  AB_Transaction_SetTransactionText(t, I18N("Direct deposit"));
	}
	else if (strcasecmp(s, "DIRECTDEBIT")==0) {
	  AB_Transaction_SetTransactionKey(t, "MSC"); /* FIXME: not sure */
	  AB_Transaction_SetTransactionText(t, I18N("Merchant initiated debit"));
	}
	else if (strcasecmp(s, "REPEATPMT")==0) {
	  AB_Transaction_SetTransactionKey(t, "STO");
	  AB_Transaction_SetTransactionText(t, I18N("Repeating payment/standing order"));
	}
	else if (strcasecmp(s, "OTHER")==0) {
	  AB_Transaction_SetTransactionKey(t, "MSC");
	  AB_Transaction_SetTransactionText(t, I18N("Other"));
	}
	else {
	  DBG_WARN(AQBANKING_LOGDOMAIN, "Unknown transaction type [%s]", s);
	  AB_Transaction_SetTransactionText(t, I18N("Unknown transaction type"));
	}
      }
      else if (strcasecmp(xg->currentElement, "DTPOSTED")==0) {
	GWEN_TIME *ti;

	ti=GWEN_Time_fromString(s, "YYYYMMDD");
	if (ti==NULL) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Invalid data for DTPOSTED: [%s]", s);
	  GWEN_Buffer_free(buf);
          return GWEN_ERROR_BAD_DATA;
	}
	AB_Transaction_SetValutaDate(xg->transaction, ti);
        GWEN_Time_free(ti);
      }
      else if (strcasecmp(xg->currentElement, "DTUSER")==0) {
	GWEN_TIME *ti;

	ti=GWEN_Time_fromString(s, "YYYYMMDD");
	if (ti==NULL) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Invalid data for DTUSER: [%s]", s);
	  GWEN_Buffer_free(buf);
	  return GWEN_ERROR_BAD_DATA;
	}
	AB_Transaction_SetDate(xg->transaction, ti);
	GWEN_Time_free(ti);
      }
      else if (strcasecmp(xg->currentElement, "DTAVAIL")==0) {
        /* ignore */
      }
      else if (strcasecmp(xg->currentElement, "TRNAMT")==0) {
	AB_VALUE *v;

	v=AB_Value_fromString(s);
	if (v==NULL) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Invalid data for TRNAMT: [%s]", s);
	  GWEN_Buffer_free(buf);
	  return GWEN_ERROR_BAD_DATA;
	}
	if (xg->currency)
          AB_Value_SetCurrency(v, xg->currency);
	AB_Transaction_SetValue(xg->transaction, v);
	AB_Value_free(v);
      }
      else if (strcasecmp(xg->currentElement, "FITID")==0) {
	AB_Transaction_SetFiId(xg->transaction, s);
      }
      else if (strcasecmp(xg->currentElement, "CHECKNUM")==0) {
	AB_Transaction_SetCustomerReference(xg->transaction, s);
      }
      else if (strcasecmp(xg->currentElement, "REFNUM")==0) {
	AB_Transaction_SetCustomerReference(xg->transaction, s);
      }
      else if (strcasecmp(xg->currentElement, "PAYEEID")==0) {
        /* ignore */
      }
      else if (strcasecmp(xg->currentElement, "NAME")==0) {
	AB_Transaction_AddRemoteName(xg->transaction, s, 1);
      }
      else if (strcasecmp(xg->currentElement, "MEMO")==0 ||
	       strcasecmp(xg->currentElement, "MEMO2")==0) {
	AB_Transaction_AddPurpose(xg->transaction, s, 1);
      }
      else if (strcasecmp(xg->currentElement, "SRVRTID")==0 ||
	       strcasecmp(xg->currentElement, "SRVRTID2")==0) {
	AB_Transaction_SetBankReference(xg->transaction, s);
      }
      else {
	DBG_INFO(AQBANKING_LOGDOMAIN,
		 "Ignoring data for unknown element [%s]",
		 xg->currentElement);
      }
    }
    GWEN_Buffer_free(buf);
  }

  return 0;
}



int AIO_OfxGroup_STMTRN_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg) {
  AIO_OFX_GROUP_STMTRN *xg;
  const char *s;
  GWEN_XML_CONTEXT *ctx;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_STMTRN, g);
  assert(xg);

  ctx=AIO_OfxGroup_GetXmlContext(g);
  assert(ctx);

  s=AIO_OfxGroup_GetGroupName(sg);
  if (strcasecmp(s, "PAYEE")==0) {
  }
  else if (strcasecmp(s, "BANKACCTTO")==0) {
  }

  return 0;
}








