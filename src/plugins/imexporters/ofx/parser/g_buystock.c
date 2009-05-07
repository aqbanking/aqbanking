/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Fri Apr 17 2009
 copyright   : (C) 2009 by Stephen R. Besch (C) 2008 by Martin Preuss
 email       : sbesch@buffalo.edu martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "g_buystock_p.h"
#include "ofxxmlctx_l.h"
#include "i18n_l.h"

#include "g_generic_l.h"
#include "g_ignore_l.h"
#include "g_invbuy_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/gui.h>

GWEN_INHERIT(AIO_OFX_GROUP, AIO_OFX_GROUP_BUYSTOCK)

const char * BUYSTOCK_DataTags[ibuystocklastdatum]={"BUYTYPE", "SELLTYPE"};
const char * BUYSTOCK_GroupTags[ibuystocklastgroup]={"INVBUY","INVSELL"};



AIO_OFX_GROUP *AIO_OfxGroup_BUYSTOCK_new(const char *groupName,
					 AIO_OFX_GROUP *parent,
					 GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_GROUP *g;
  AIO_OFX_GROUP_BUYSTOCK *xg;

  /* create base group */
  g=AIO_OfxGroup_Generic_new(groupName, parent, ctx);
  assert(g);

  GWEN_NEW_OBJECT(AIO_OFX_GROUP_BUYSTOCK, xg);
  assert(xg);
  GWEN_INHERIT_SETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BUYSTOCK, g, xg, AIO_OfxGroup_BUYSTOCK_FreeData);
  xg->transaction=AB_Transaction_new();

  /*We know that the transaction type will be AB_Transaction_TypeTransaction, so set this now.*/
  AB_Transaction_SetType(xg->transaction, AB_Transaction_TypeTransaction);

  /* set virtual functions */
  AIO_OfxGroup_SetStartTagFn(g, AIO_OfxGroup_BUYSTOCK_StartTag);
  AIO_OfxGroup_SetAddDataFn(g, AIO_OfxGroup_BUYSTOCK_AddData);
  AIO_OfxGroup_SetEndSubGroupFn(g, AIO_OfxGroup_BUYSTOCK_EndSubGroup);
  return g;
}



GWENHYWFAR_CB
void AIO_OfxGroup_BUYSTOCK_FreeData(void *bp, void *p) {
  AIO_OFX_GROUP_BUYSTOCK *xg;

  xg=(AIO_OFX_GROUP_BUYSTOCK*)p;
  assert(xg);
  free(xg->currentElement);
  AB_Transaction_free(xg->transaction);
  GWEN_FREE_OBJECT(xg);
}



/*Note that we allow the superior group to "take" the transaction - i.e., the taker is now responsible for
 destroying the object!!*/
AB_TRANSACTION *AIO_OfxGroup_BUYSTOCK_TakeTransaction(const AIO_OFX_GROUP *g){
  AIO_OFX_GROUP_BUYSTOCK *xg;
  AB_TRANSACTION *t;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BUYSTOCK, g);
  assert(xg);

  t=xg->transaction;
  xg->transaction=NULL;
  return t;
}



/*BUYSTOCK object can represent either the <BUYSTOCK> or <SELLSTOCK> groups. Thus is has 1 datum
 (<BUYTYPE> or <SELLTYPE>) and one group (<INVBUY> or <INVSELL>). In a sense this is redundant data.
 Well formed data should always place either "buy" or "sell" in the data of <BUYTYPE> and <SELLTYPE>,
 respectively. So I test for this and set the sub-group accordingly.*/
int AIO_OfxGroup_BUYSTOCK_StartTag(AIO_OFX_GROUP *g, const char *tagName) {
  AIO_OFX_GROUP_BUYSTOCK *xg;
  GWEN_XML_CONTEXT *ctx;
  AIO_OFX_GROUP *gNew=NULL;
  int ct;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BUYSTOCK, g);
  assert(xg);
  ctx=AIO_OfxGroup_GetXmlContext(g);

  ct=AIO_OfxGroup_BUYSTOCK_SortTag(tagName, BUYSTOCK_DataTags, ibuystocklastdatum);
  
  if(ct<0) {
    ct=AIO_OfxGroup_BUYSTOCK_SortTag(tagName, BUYSTOCK_GroupTags, ibuystocklastgroup);
    if (ct<0) {
      DBG_WARN(AQBANKING_LOGDOMAIN, "Ignoring group [%s]", tagName);
      gNew=AIO_OfxGroup_Ignore_new(tagName, g, ctx);
    }
    else
      gNew=AIO_OfxGroup_INVBUY_new(tagName, g, ctx);
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



/*Here when the data for either BUYTYPE or SELLTYPE arrives. We simply stuff it into the correct part of the
 AB_TRANSACTION record.*/

int AIO_OfxGroup_BUYSTOCK_AddData(AIO_OFX_GROUP *g, const char *data) {
  AIO_OFX_GROUP_BUYSTOCK *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BUYSTOCK, g);
  assert(xg);

  if (xg->currentElement) {
    int ce;
    GWEN_BUFFER *buf;
    int rv;
    const char *s;

    ce=AIO_OfxGroup_BUYSTOCK_SortTag(xg->currentElement, BUYSTOCK_DataTags, ibuystocklastdatum);
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
    s=GWEN_Buffer_GetStart(buf);
    if (*s) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "TransactionSubType: %s", s);
      AB_TRANSACTION_SUBTYPE SubType;
      if (strcasecmp(s,"BUY")==0)
	SubType=AB_Transaction_SubTypeBuy;
      else if (strcasecmp(s,"SELL")==0)
	SubType=AB_Transaction_SubTypeSell;
      else
	SubType=AB_Transaction_SubTypeUnknown;
      AB_Transaction_SetSubType(xg->transaction, SubType);
    }
    GWEN_Buffer_free(buf);
  }
  return 0;
}



/*Come here on </INVBUY>, </INVSELL> This is where most of the work is done. We have to map all the various
 OFX items into the Transaction Structure. Here is the working model of the translation table:

  Type    ID                Setting Call
  char *units;              AQBANKING_API void AB_Transaction_SetUnits(AB_TRANSACTION *el, const AB_VALUE *d);
  char *unitprice;          AQBANKING_API void AB_Transaction_SetUnitPrice(AB_TRANSACTION *el, const AB_VALUE *d);
  char *commission;         AQBANKING_API void AB_Transaction_SetCommission(AB_TRANSACTION *el, const AB_VALUE *d);
  char *total;              AQBANKING_API void AB_Transaction_SetValue(AB_TRANSACTION *el, const AB_VALUE *d);
  char *subacctsec;         No equivalent
  char *subacctfund;        Maybe AQBANKING_API void AB_Transaction_SetLocalSuffix(AB_TRANSACTION *el, const char *d);
  char *uniqueid;           AQBANKING_API void AB_Transaction_SetUnitId(AB_TRANSACTION *el, const char *d);
  char *uniqueidtype;       AQBANKING_API void AB_Transaction_SetUnitIdNameSpace(AB_TRANSACTION *el, const char *d);
  char *fitid;              AQBANKING_API void AB_Transaction_SetFiId(AB_TRANSACTION *el, const char *d);
  char *dttrade;            AQBANKING_API void AB_Transaction_SetValutaDate(AB_TRANSACTION *el, const GWEN_TIME *d);
  char *memo;               AQBANKING_API void AB_Transaction_SetTransactionText(AB_TRANSACTION *el, const char *d);

There should be a "FEE" tag and a settlement date in here somewhere, but my brokerage doesn't use them. Maybe the
 subacctsec could use SetBankReference and memo could use SetTransactionText. Note that stmtrn uses
 AB_Transaction_AddPurpose for memo.

 The strings can go in as they are. AB_VALUES are converted from strings using AB_Value_fromString() and
 AB_Value_SetCurrency(), while the GWEN_TIME value is converted using GWEN_Time_fromString(s, "YYYYMMDD")*/

int AIO_OfxGroup_BUYSTOCK_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg) {

  AIO_OFX_GROUP_BUYSTOCK *xg;
  GWEN_XML_CONTEXT *ctx;
  GWEN_TIME *ti;

  /*First connect to the data list. Throw a hissy if either the group object or the inherited group object is invalid*/

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BUYSTOCK, g);
  assert(xg);

  /*Fetch a copy of this groups context - and it better be valid too.*/
  ctx=AIO_OfxGroup_GetXmlContext(g);
  assert(ctx);

  /*We need to look at the group name to see if it's recognized. Simply return, ignoring it if we can't use it.*/

  if (0>AIO_OfxGroup_BUYSTOCK_SortTag(AIO_OfxGroup_GetGroupName(sg), BUYSTOCK_GroupTags, ibuystocklastgroup))
    return 0;

  /*Both the INVBUY and INVSELL groups do the same thing.*/

  AIO_OfxGroup_BUYSTOCK_SetABValue(sg, &AB_Transaction_SetUnits,  xg->transaction, iunits);
  AIO_OfxGroup_BUYSTOCK_SetABValue(sg, &AB_Transaction_SetUnitPrice, xg->transaction,  iunitprice);
  AIO_OfxGroup_BUYSTOCK_SetABValue(sg, &AB_Transaction_SetCommission, xg->transaction, icommission);
  AIO_OfxGroup_BUYSTOCK_SetABValue(sg, &AB_Transaction_SetValue, xg->transaction, itotal);

  /*AIO_OfxGroup_INVBUY_TakeDatum(sg, i)  char *subacctsec;         No equivalent*/

  /*Maybe Try this?*/
  AB_Transaction_SetLocalSuffix(xg->transaction, AIO_OfxGroup_INVBUY_GetDatum(sg, isubacctfund));
  AB_Transaction_SetUnitId(xg->transaction, AIO_OfxGroup_INVBUY_GetDatum(sg, iuniqueid));
  AB_Transaction_SetUnitIdNameSpace(xg->transaction, AIO_OfxGroup_INVBUY_GetDatum(sg, iuniqueidtype));
  AB_Transaction_SetFiId(xg->transaction, AIO_OfxGroup_INVBUY_GetDatum(sg, ifitid));
  /*   AB_Transaction_SetTransactionText(xg->transaction, AIO_OfxGroup_INVBUY_GetDatum(sg, imemo)); */
  AB_Transaction_AddPurpose(xg->transaction, AIO_OfxGroup_INVBUY_GetDatum(sg, imemo), 1);

  /*Finally, do the date. This is a bit of a guess. The ValutaDate description is the more correct one:
   * "The date when the transaction was really executed".  But the Date is used in stmtrn. It's description is
   less convincing: "The date when the transaction was booked (but sometimes it is unused)". Mostly, I suspect
   that it depends upon how the backend uses the data. I'll have to see what happens when I run it from GnuCash.*/

  ti=GWEN_Time_fromString(AIO_OfxGroup_INVBUY_GetDatum(sg, idttrade), "YYYYMMDD");
  if (ti==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid data for DTTRADE: [%s]", AIO_OfxGroup_INVBUY_GetDatum(sg, idttrade));
    return GWEN_ERROR_BAD_DATA;
  }
  AB_Transaction_SetValutaDate(xg->transaction, ti);

  /*Just to be safe, if the date is not otherwise set, set it to the DTTRADE value.*/

  if (AB_Transaction_GetDate(xg->transaction)==NULL)
    AB_Transaction_SetDate(xg->transaction,ti);
  GWEN_Time_free(ti);
  return 0;
}



void AIO_OfxGroup_BUYSTOCK_SetABValue(const AIO_OFX_GROUP *sg, AB_Transaction_Set_Value_Func F, AB_TRANSACTION *t, int index){
  AB_VALUE * ab_val;

  ab_val=AB_Value_fromString(AIO_OfxGroup_INVBUY_GetDatum(sg, index));
  if (ab_val==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
             "Invalid data for %s: [%s]",
              AIO_OfxGroup_INVBUY_DatumName(index),
              AIO_OfxGroup_INVBUY_GetDatum(sg, index));
    ab_val=AB_Value_fromString("0");
  }
  F(t, ab_val);
  AB_Value_free(ab_val);  /*All done with it now*/
  return;
}



/*Routine that converts string tag into index number. Return -1 if no tag found.*/

int AIO_OfxGroup_BUYSTOCK_SortTag(const char * s, const char ** sTags, int max) {
  int i;

  for (i=0; i<max; i++)
    if (strcasecmp(s, sTags[i])==0)
      return i;
  return -1;
}



