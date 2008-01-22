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

#include "g_invstmtrs_p.h"
#include "ofxxmlctx_l.h"
#include "i18n_l.h"

#include "g_generic_l.h"
#include "g_ignore_l.h"
#include "g_bankacc_l.h"
#include "g_banktranlist_l.h"
#include "g_bal_l.h"

#include <aqbanking/accstatus.h>

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/gui.h>



GWEN_INHERIT(AIO_OFX_GROUP, AIO_OFX_GROUP_INVSTMTRS)




AIO_OFX_GROUP *AIO_OfxGroup_INVSTMTRS_new(const char *groupName,
					  AIO_OFX_GROUP *parent,
					  GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_GROUP *g;
  AIO_OFX_GROUP_INVSTMTRS *xg;

  /* create base group */
  g=AIO_OfxGroup_Generic_new(groupName, parent, ctx);
  assert(g);

  GWEN_NEW_OBJECT(AIO_OFX_GROUP_INVSTMTRS, xg);
  assert(xg);
  GWEN_INHERIT_SETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVSTMTRS, g, xg,
                       AIO_OfxGroup_INVSTMTRS_FreeData);

  /* set virtual functions */
  AIO_OfxGroup_SetStartTagFn(g, AIO_OfxGroup_INVSTMTRS_StartTag);
  AIO_OfxGroup_SetAddDataFn(g, AIO_OfxGroup_INVSTMTRS_AddData);
  AIO_OfxGroup_SetEndSubGroupFn(g, AIO_OfxGroup_INVSTMTRS_EndSubGroup);

  return g;
}



void AIO_OfxGroup_INVSTMTRS_FreeData(void *bp, void *p) {
  AIO_OFX_GROUP_INVSTMTRS *xg;

  xg=(AIO_OFX_GROUP_INVSTMTRS*)p;
  assert(xg);
  free(xg->currency);
  free(xg->currentElement);
  GWEN_FREE_OBJECT(xg);
}



int AIO_OfxGroup_INVSTMTRS_StartTag(AIO_OFX_GROUP *g,
				 const char *tagName) {
  AIO_OFX_GROUP_INVSTMTRS *xg;
  GWEN_XML_CONTEXT *ctx;
  AIO_OFX_GROUP *gNew=NULL;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVSTMTRS, g);
  assert(xg);

  ctx=AIO_OfxGroup_GetXmlContext(g);

  free(xg->currentElement);
  xg->currentElement=NULL;

  if (strcasecmp(tagName, "CURDEF")==0) {
    xg->currentElement=strdup(tagName);
  }
  else if (strcasecmp(tagName, "INVACCTFROM")==0) {
    gNew=AIO_OfxGroup_BANKACC_new(tagName, g, ctx);
  }
  else {
    DBG_WARN(AQBANKING_LOGDOMAIN,
	     "Ignoring group [%s]", tagName);
    gNew=AIO_OfxGroup_Ignore_new(tagName, g, ctx);
  }

  if (gNew) {
    AIO_OfxXmlCtx_SetCurrentGroup(ctx, gNew);
    GWEN_XmlCtx_IncDepth(ctx);
  }

  return 0;
}



int AIO_OfxGroup_INVSTMTRS_AddData(AIO_OFX_GROUP *g, const char *data) {
  AIO_OFX_GROUP_INVSTMTRS *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVSTMTRS, g);
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
      if (strcasecmp(xg->currentElement, "CURDEF")==0) {
	free(xg->currency);
        xg->currency=strdup(s);
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



int AIO_OfxGroup_INVSTMTRS_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg) {
  AIO_OFX_GROUP_INVSTMTRS *xg;
  const char *s;
  GWEN_XML_CONTEXT *ctx;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVSTMTRS, g);
  assert(xg);

  ctx=AIO_OfxGroup_GetXmlContext(g);
  assert(ctx);

  s=AIO_OfxGroup_GetGroupName(sg);
  if (strcasecmp(s, "INVACCTFROM")==0) {
    AB_IMEXPORTER_ACCOUNTINFO *ai;
    const char *s;

    DBG_INFO(AQBANKING_LOGDOMAIN,
	     "Importing account %s/%s",
	     AIO_OfxGroup_BANKACC_GetBankId(sg),
	     AIO_OfxGroup_BANKACC_GetAccId(sg));
    ai=AB_ImExporterAccountInfo_new();
    assert(ai);

    s=AIO_OfxGroup_BANKACC_GetBankId(sg);
    if (s)
      AB_ImExporterAccountInfo_SetBankCode(ai, s);
    s=AIO_OfxGroup_BANKACC_GetAccId(sg);
    if (s)
      AB_ImExporterAccountInfo_SetAccountNumber(ai, s);

    /* set currency */
    if (xg->currency)
      AB_ImExporterAccountInfo_SetCurrency(ai, xg->currency);

    /* set account type, if known */
    s=AIO_OfxGroup_BANKACC_GetAccType(sg);
    if (!s)
      s="BANK"; /* not a real code */
    if (s) {
      AB_ACCOUNT_TYPE t;

      t=AIO_OfxGroup_Generic_AccountTypeFromString(s);
      AB_ImExporterAccountInfo_SetType(ai, t);
    }

    DBG_INFO(AQBANKING_LOGDOMAIN, "Adding account");
    AB_ImExporterContext_AddAccountInfo(AIO_OfxXmlCtx_GetIoContext(ctx), ai);
    xg->accountInfo=ai;
  }
  else if (strcasecmp(s, "BANKTRANLIST")==0) {
    /* TODO */
#if 0
    AB_TRANSACTION_LIST2 *tl;

    tl=AIO_OfxGroup_BANKTRANLIST_TakeTransactionList(sg);
    if (tl) {
      AB_TRANSACTION_LIST2_ITERATOR *it;

      it=AB_Transaction_List2_First(tl);
      if (it) {
	AB_TRANSACTION *t;

	t=AB_Transaction_List2Iterator_Data(it);
	while(t) {
	  DBG_INFO(AQBANKING_LOGDOMAIN, "Importing transaction");
          /* set currency if missing */
	  if (xg->currency) {
	    const AB_VALUE *v;

	    v=AB_Transaction_GetValue(t);
	    if (v && AB_Value_GetCurrency(v)==NULL) {
	      AB_VALUE *v2;

	      v2=AB_Value_dup(v);
	      AB_Value_SetCurrency(v2, xg->currency);
	      AB_Transaction_SetValue(t, v2);
              AB_Value_free(v2);
	    }
	  }
	  AB_ImExporterAccountInfo_AddTransaction(xg->accountInfo, t);
	  t=AB_Transaction_List2Iterator_Next(it);
	}
	AB_Transaction_List2Iterator_free(it);
      }
      /* don't call AB_Transaction_List2_freeAll(), because the transactions
       * from the list have been taken over by the AccountInfo object */
      AB_Transaction_List2_free(tl);
    }
#endif
  }

  return 0;
}








