/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de
 begin       : Fri Apr 17 2009
 copyright   : (C) 2009 by Stephen R. Besch
 email       : sbesch@buffalo.edu
 begin       : Sat May 18 2013
 copyright   : (C) 2013 by Paul Conrady
 email       : c.p.conrady@gmail.com

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "g_invtranlist_p.h"
#include "ofxxmlctx_l.h"

#include "g_generic_l.h"
#include "g_ignore_l.h"
#include "g_buystock_l.h"
#include "g_income_l.h"
#include "g_stmtrn_l.h"
#include "g_buymf_l.h"
#include "g_reinvest_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/gui.h>

GWEN_INHERIT(AIO_OFX_GROUP, AIO_OFX_GROUP_INVTRANLIST)


AIO_OFX_GROUP *AIO_OfxGroup_INVTRANLIST_new(const char *groupName,
					    AIO_OFX_GROUP *parent,
					    GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_GROUP *g;
  AIO_OFX_GROUP_INVTRANLIST *xg;

  /* create base group */
  g=AIO_OfxGroup_Generic_new(groupName, parent, ctx);
  assert(g);

  GWEN_NEW_OBJECT(AIO_OFX_GROUP_INVTRANLIST, xg);
  assert(xg);
  GWEN_INHERIT_SETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVTRANLIST, g, xg,
                       AIO_OfxGroup_INVTRANLIST_FreeData);

  xg->transactionList=AB_Transaction_List2_new();

  /* set virtual functions */
  AIO_OfxGroup_SetStartTagFn(g, AIO_OfxGroup_INVTRANLIST_StartTag);
  AIO_OfxGroup_SetAddDataFn(g, AIO_OfxGroup_INVTRANLIST_AddData);
  AIO_OfxGroup_SetEndSubGroupFn(g, AIO_OfxGroup_INVTRANLIST_EndSubGroup);
  return g;
}



GWENHYWFAR_CB
void AIO_OfxGroup_INVTRANLIST_FreeData(void *bp, void *p) {
  AIO_OFX_GROUP_INVTRANLIST *xg;

  xg=(AIO_OFX_GROUP_INVTRANLIST*)p;
  assert(xg);
  AB_Transaction_List2_freeAll(xg->transactionList);

  free(xg->currentElement);
  GWEN_FREE_OBJECT(xg);
}



AB_TRANSACTION_LIST2* AIO_OfxGroup_INVTRANLIST_TakeTransactionList(const AIO_OFX_GROUP *g) {
  AIO_OFX_GROUP_INVTRANLIST *xg;
  AB_TRANSACTION_LIST2 *tl;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVTRANLIST, g);
  assert(xg);

  tl=xg->transactionList;
  xg->transactionList=NULL;
  return tl;
}



/*The INVTRANLIST has 2 data and at least 4 groups. The BUYSTOCK, SELLSTOCK, and INCOME are similar
enough to be handled using a single subgroup and some steering logic. The INVBANKTRAN is essentially
identical to the Bank equivalent, so we use the STMTTRN group for it.*/

/* The <BUYMF> and <SELLMF> aggregates are similar, therefore each is handled by the <BUYMF>
 * method. The difference between a buy and a sell is indicated by a positive or negative value 
 * in the total and units datum. The <REINVEST> aggregate represents an income event
 * (e.g. dividends) and a buy transaction of the like commodity.
 */

int AIO_OfxGroup_INVTRANLIST_StartTag(AIO_OFX_GROUP *g,
				      const char *tagName) {
  AIO_OFX_GROUP_INVTRANLIST *xg;
  GWEN_XML_CONTEXT *ctx;
  AIO_OFX_GROUP *gNew=NULL;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVTRANLIST, g);
  assert(xg);

  ctx=AIO_OfxGroup_GetXmlContext(g);

  if (strcasecmp(tagName, "DTSTART")==0 ||
      strcasecmp(tagName, "DTEND")==0) {
    free(xg->currentElement);
    xg->currentElement=strdup(tagName);
  }
  else if (strcasecmp(tagName, "BUYSTOCK")==0 ||
	   strcasecmp(tagName, "SELLSTOCK")==0)
    gNew=AIO_OfxGroup_BUYSTOCK_new(tagName, g, ctx);
  else if (strcasecmp(tagName, "INCOME")==0)
    gNew=AIO_OfxGroup_INCOME_new(tagName, g, ctx);
  else if (strcasecmp(tagName, "INVBANKTRAN")==0)
    gNew=AIO_OfxGroup_STMTRN_new(tagName, g, ctx);
  else if (strcasecmp(tagName, "BUYMF")==0 ||
           strcasecmp(tagName, "SELLMF")==0)
    gNew=AIO_OfxGroup_BUYMF_new(tagName, g, ctx);
  else if (strcasecmp(tagName, "REINVEST")==0)
    gNew=AIO_OfxGroup_REINVEST_new(tagName, g, ctx);
  else {
    DBG_WARN(AQBANKING_LOGDOMAIN, "Ignoring group [%s]", tagName);
    gNew=AIO_OfxGroup_Ignore_new(tagName, g, ctx);
  }

  if (gNew) {
    AIO_OfxXmlCtx_SetCurrentGroup(ctx, gNew);
    GWEN_XmlCtx_IncDepth(ctx);
  }
  return 0;
}



/*Here when the data for either DTSTART or DTEND arrives*/
int AIO_OfxGroup_INVTRANLIST_AddData(AIO_OFX_GROUP *g, const char *data) {
  AIO_OFX_GROUP_INVTRANLIST *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVTRANLIST, g);
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
      DBG_INFO(AQBANKING_LOGDOMAIN, "AddData: %s=[%s]", xg->currentElement, s);
      if (strcasecmp(xg->currentElement, "DTSTART")==0) {
	free(xg->dtstart);
	xg->dtstart=strdup(s);
      }
      else if (strcasecmp(xg->currentElement, "DTEND")==0) {
	free(xg->dtend);
	xg->dtend=strdup(s);
      }
      else {
	DBG_INFO(AQBANKING_LOGDOMAIN, "Ignoring data for unknown elements [%s]", xg->currentElement);
      }
    }
    GWEN_Buffer_free(buf);
  }
  return 0;
}



/* Come here when the </BUYSTOCK>, </SELLSTOCK>, </INCOME>, </INVBANKTRAN>,
 * </BUYMF>, </SELLMF> or </REINVEST> tags are encountered.
 */

int AIO_OfxGroup_INVTRANLIST_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg) {
  AIO_OFX_GROUP_INVTRANLIST *xg;
  const char *s;
  GWEN_XML_CONTEXT *ctx;
  AB_TRANSACTION *t;

  /*First connect to the data list. Throw a hissy if either the group object or the inherited group object is invalid*/

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVTRANLIST, g);
  assert(xg);

  /*Fetch a copy of this groups context - and it better be valid too.*/
  ctx=AIO_OfxGroup_GetXmlContext(g);
  assert(ctx);

  /*We need to look at the group name to see what to do. Then call the appropriate routine to take the transaction
   and push it into the transaction list.*/

  s=AIO_OfxGroup_GetGroupName(sg);
  if (strcasecmp(s, "BUYSTOCK")==0 ||
      strcasecmp(s, "SELLSTOCK")==0)
    t=AIO_OfxGroup_BUYSTOCK_TakeTransaction(sg);
  else if (strcasecmp(s, "INCOME")==0)
    t=AIO_OfxGroup_INCOME_TakeTransaction(sg);
  else if (strcasecmp(s, "INVBANKTRAN")==0)
    t=AIO_OfxGroup_STMTRN_TakeTransaction(sg);
  else if (strcasecmp(s, "BUYMF")==0 ||
      strcasecmp(s, "SELLMF")==0)
    t=AIO_OfxGroup_BUYMF_TakeTransaction(sg);
  else if (strcasecmp(s, "REINVEST")==0)
    t=AIO_OfxGroup_REINVEST_TakeTransaction(sg);
  else
    return 0;

  /*If one of the groups matches, then post a message about adding the new transaction to the list*/
  if (t) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Adding transaction");
    AB_Transaction_List2_PushBack(xg->transactionList, t);
    }
  return 0;
}




