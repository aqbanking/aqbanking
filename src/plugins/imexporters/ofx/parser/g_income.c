/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Fri Apr 17 2009
 copyright   : (C) 2009 by Stephen R. Besch (C) 2008 by Martin Preuss
 email       : sbesch@buffalo.edu martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#warning "Is this file really used??"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "g_income_p.h"
#include "ofxxmlctx_l.h"
#include "i18n_l.h"

#include "g_generic_l.h"
#include "g_ignore_l.h"
#include "g_invtran_l.h"
#include "g_invbuy_l.h"
#include "g_secid_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/gui.h>

const char * INCOME_DataTags[iincomelastdatum]={"TOTAL", "INCOMETYPE", "SUBACCTSEC", "SUBACCTFUND"};
const char * INCOME_GroupTags[iincomelastgroup]={"SECID","INVTRAN"};


GWEN_INHERIT(AIO_OFX_GROUP, AIO_OFX_GROUP_INCOME)



AIO_OFX_GROUP *AIO_OfxGroup_INCOME_new(const char *groupName,
				       AIO_OFX_GROUP *parent,
				       GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_GROUP *g;
  AIO_OFX_GROUP_INCOME *xg;

  /* create base group */
  g=AIO_OfxGroup_Generic_new(groupName, parent, ctx);
  assert(g);

  GWEN_NEW_OBJECT(AIO_OFX_GROUP_INCOME, xg);
  assert(xg);
  GWEN_INHERIT_SETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INCOME, g, xg, AIO_OfxGroup_INCOME_FreeData);
  xg->transaction=AB_Transaction_new();

  /*We know that the transaction type will be AB_Transaction_TypeTransaction, so set this now.*/
  AB_Transaction_SetType(xg->transaction, AB_Transaction_TypeTransaction);

  /* set virtual functions */
  AIO_OfxGroup_SetStartTagFn(g, AIO_OfxGroup_INCOME_StartTag);
  AIO_OfxGroup_SetAddDataFn(g, AIO_OfxGroup_INCOME_AddData);
  AIO_OfxGroup_SetEndSubGroupFn(g, AIO_OfxGroup_INCOME_EndSubGroup);
  return g;
}



GWENHYWFAR_CB
void AIO_OfxGroup_INCOME_FreeData(void *bp, void *p) {
  AIO_OFX_GROUP_INCOME *xg;

  xg=(AIO_OFX_GROUP_INCOME*)p;
  assert(xg);
  free(xg->currentElement);
  AB_Transaction_free(xg->transaction);
  GWEN_FREE_OBJECT(xg);
}



/*Note that we allow the superior group to "take" the transaction - i.e., the taker is now responsible for
 destroying the object!!*/
AB_TRANSACTION *AIO_OfxGroup_INCOME_TakeTransaction(const AIO_OFX_GROUP *g){
  AIO_OFX_GROUP_INCOME *xg;
  AB_TRANSACTION *t;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INCOME, g);
  assert(xg);

  t=xg->transaction;
  xg->transaction=NULL;
  return t;
}



/*INCOME object has 4 datums: "TOTAL", "INCOMETYPE", "SUBACCTSEC", "SUBACCTFUND". We'll treat the
 SUBACCTFUND just like for BUYSTOCK, also ignoring the SUBACCTSEC tag. INCOMETYPE can be any one of
 "DIV", "LONG" or "SHORT". Stocks should never have Long or Short, since brokerages don't compute
 this for you. However, Mutual funds will have these. There are 2 groups: SECID & INVTRAN. We'll handle
 these like data groups, putting the transaction data directly into the transaction structure. */

int AIO_OfxGroup_INCOME_StartTag(AIO_OFX_GROUP *g, const char *tagName) {
  AIO_OFX_GROUP_INCOME *xg;
  GWEN_XML_CONTEXT *ctx;
  AIO_OFX_GROUP *gNew=NULL;
  int ct;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INCOME, g);
  assert(xg);

  ctx=AIO_OfxGroup_GetXmlContext(g);
  ct=AIO_OfxGroup_INCOME_SortTag(tagName, INCOME_DataTags, iincomelastdatum);

  if(ct<0) {
    ct=AIO_OfxGroup_INCOME_SortTag(tagName, INCOME_GroupTags, iincomelastgroup);
    if (ct<0) {
      DBG_WARN(AQBANKING_LOGDOMAIN, "Ignoring group [%s]", tagName);
      gNew=AIO_OfxGroup_Ignore_new(tagName, g, ctx);
    }
    else {
      if (ct==iincomesecid)
	gNew=AIO_OfxGroup_SECID_new(tagName, g, ctx);
      else if (ct==iincomeinvtran)
	gNew=AIO_OfxGroup_INVTRAN_new(tagName, g, ctx);
    }
    if (gNew) {
      AIO_OfxXmlCtx_SetCurrentGroup(ctx, gNew);
      GWEN_XmlCtx_IncDepth(ctx);
    }
  }
  else {
    free(xg->currentElement);
    xg->currentElement=strdup(tagName);
  }
  return 0;
}



/*Here when the data for TOTAL, INCOMETYPE, SUBACCTSEC, SUBACCTFUND arrives. Simply stuff it into the correct part of the
 AB_TRANSACTION record.*/

int AIO_OfxGroup_INCOME_AddData(AIO_OFX_GROUP *g, const char *data) {
  AIO_OFX_GROUP_INCOME *xg;
  int ce;
  GWEN_BUFFER *buf;
  int rv;
  const char *s;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INCOME, g);
  assert(xg);

  if (!xg->currentElement)
    return 0;      /*No element - just ignore these*/

  ce=AIO_OfxGroup_INCOME_SortTag(xg->currentElement, INCOME_DataTags, iincomelastdatum);
  if (ce<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Ignoring data for unknown elements [%s]", xg->currentElement);
    return 0;       /*Unknown data tag.*/
  }

  buf=GWEN_Buffer_new(0, strlen(data), 0, 1);
  rv=AIO_OfxXmlCtx_SanitizeData(AIO_OfxGroup_GetXmlContext(g), data, buf);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(buf);
    return rv;
  }

  /*We now have the XML string in the buffer. Set the pointer into s and let's get buzy coping with
   the data.*/

  s=GWEN_Buffer_GetStart(buf);
  if (*s) {
    AB_VALUE *v;

    rv=0;
    switch (ce) {
      case iincometotal:
	v=AB_Value_fromString(s);
	if (v==NULL) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid data for %s: [%s]", xg->currentElement, s);
	  rv=(int)GWEN_ERROR_BAD_DATA;
/* --or-- v=AB_Value_fromString("0"); */
	}
	else {
	  AB_Transaction_SetValue(xg->transaction, v);
	  AB_Value_free(v);
	}
	break;

      case iincometype: {
	AB_TRANSACTION_SUBTYPE SubType;

	DBG_INFO(AQBANKING_LOGDOMAIN, "TransactionSubType: %s", s);
	if (strcasecmp(s,"DIV")==0) SubType=AB_Transaction_SubTypeDividend;
	else if (strcasecmp(s,"SHORT")==0 ||
		 strcasecmp(s,"LONG")==0) SubType=AB_Transaction_SubTypeSell;
	else SubType=AB_Transaction_SubTypeUnknown;

	AB_Transaction_SetSubType(xg->transaction,SubType);
	break;
      }

      case iincomesubacctsec:
        break;

      case iincomesubacctfund:
        AB_Transaction_SetLocalSuffix(xg->transaction, s);
        break;
    }
  }
  GWEN_Buffer_free(buf);
  return rv;
}



/*Come here on </SECID>, </INVTRAN> This is where most of the work is done. We have to map all the various
 OFX items into the Transaction Structure. Here is the working model of the translation table:

  Type    ID                Setting Call

  char *uniqueid;           AQBANKING_API void AB_Transaction_SetUnitId(AB_TRANSACTION *el, const char *d);
  char *uniqueidtype;       AQBANKING_API void AB_Transaction_SetUnitIdNameSpace(AB_TRANSACTION *el, const char *d);

  char *fitid;              AQBANKING_API void AB_Transaction_SetFiId(AB_TRANSACTION *el, const char *d);
  char *dttrade;            AQBANKING_API void AB_Transaction_SetValutaDate(AB_TRANSACTION *el, const GWEN_TIME *d);
  char *memo;               AQBANKING_API void AB_Transaction_SetTransactionText(AB_TRANSACTION *el, const char *d);

 Note that stmtrn uses AB_Transaction_AddPurpose for memo. GWEN_TIME value is converted using GWEN_Time_fromString(s, "YYYYMMDD")*/

int AIO_OfxGroup_INCOME_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg) {

  AIO_OFX_GROUP_INCOME *xg;
  GWEN_XML_CONTEXT *ctx;
  int eg;
  int rv;

  /*First connect to the data list. Throw a hissy if either the group object or the inherited group object is invalid*/

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INCOME, g);
  assert(xg);

  /*Fetch a copy of this groups context - and it better be valid too.*/
  ctx=AIO_OfxGroup_GetXmlContext(g);
  assert(ctx);

  eg=AIO_OfxGroup_INCOME_SortTag(AIO_OfxGroup_GetGroupName(sg), INCOME_GroupTags, iincomelastgroup);

  if (eg<0)
    return 0;         /*Not a recognised group*/

  rv=0;
  switch (eg) {
    case iincomesecid:
      AB_Transaction_SetUnitId(xg->transaction, AIO_OfxGroup_SECID_GetUniqueId(sg));
      AB_Transaction_SetUnitIdNameSpace(xg->transaction, AIO_OfxGroup_SECID_GetNameSpace(sg));
      break;

    case iincomeinvtran: {
      GWEN_TIME *ti;

      AB_Transaction_SetFiId(xg->transaction, AIO_OfxGroup_INVTRAN_GetDatum(sg, itranfitid));
      /*-or-      AB_Transaction_SetTransactionText(xg->transaction, AIO_OfxGroup_INVTRAN_GetDatum(sg, itranmemo));*/
      AB_Transaction_AddPurpose(xg->transaction, AIO_OfxGroup_INVTRAN_GetDatum(sg, itranmemo), 1);

      /*Finally, do the date. This is a bit of a guess. The ValutaDate description is the more correct one:
       * "The date when the transaction was really executed".  But the Date is used in stmtrn. It's description is
       less convincing: "The date when the transaction was booked (but sometimes it is unused)". Mostly, I suspect
       that it depends upon how the backend uses the data. I'll have to see what happens when I run it from GnuCash.*/

      ti=GWEN_Time_fromString(AIO_OfxGroup_INVTRAN_GetDatum(sg, itrandttrade), "YYYYMMDD");
      if (ti==NULL) {
	DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid data for DTTRADE: [%s]", AIO_OfxGroup_INVTRAN_GetDatum(sg, itrandttrade));
	rv=GWEN_ERROR_BAD_DATA;
      }
      else {
	AB_Transaction_SetValutaDate(xg->transaction, ti);
	GWEN_Time_free(ti);
      }
      break;
    }
  }
  return rv;
}



int AIO_OfxGroup_INCOME_SortTag(const char * s, const char ** sTags, int max){
  int i;

  for (i=0; i<max; i++)
    if (strcasecmp(s, sTags[i])==0) return i;
  return -1;
};



