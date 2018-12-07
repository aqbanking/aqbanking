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

#include "g_banktranlist_p.h"
#include "ofxxmlctx_l.h"
#include "i18n_l.h"

#include "g_generic_l.h"
#include "g_ignore_l.h"
#include "g_bankacc_l.h"
#include "g_stmtrn_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/gui.h>



GWEN_INHERIT(AIO_OFX_GROUP, AIO_OFX_GROUP_BANKTRANLIST)




AIO_OFX_GROUP *AIO_OfxGroup_BANKTRANLIST_new(const char *groupName,
					     AIO_OFX_GROUP *parent,
					     GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_GROUP *g;
  AIO_OFX_GROUP_BANKTRANLIST *xg;

  /* create base group */
  g=AIO_OfxGroup_Generic_new(groupName, parent, ctx);
  assert(g);

  GWEN_NEW_OBJECT(AIO_OFX_GROUP_BANKTRANLIST, xg);
  assert(xg);
  GWEN_INHERIT_SETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BANKTRANLIST, g, xg,
                       AIO_OfxGroup_BANKTRANLIST_FreeData);

  xg->transactionList=AB_Transaction_List2_new();

  /* set virtual functions */
  AIO_OfxGroup_SetStartTagFn(g, AIO_OfxGroup_BANKTRANLIST_StartTag);
  AIO_OfxGroup_SetAddDataFn(g, AIO_OfxGroup_BANKTRANLIST_AddData);
  AIO_OfxGroup_SetEndSubGroupFn(g, AIO_OfxGroup_BANKTRANLIST_EndSubGroup);

  return g;
}



GWENHYWFAR_CB
void AIO_OfxGroup_BANKTRANLIST_FreeData(void *bp, void *p) {
  AIO_OFX_GROUP_BANKTRANLIST *xg;

  xg=(AIO_OFX_GROUP_BANKTRANLIST*)p;
  assert(xg);
  AB_Transaction_List2_freeAll(xg->transactionList);

  free(xg->currentElement);
  GWEN_FREE_OBJECT(xg);
}



AB_TRANSACTION_LIST2*
AIO_OfxGroup_BANKTRANLIST_TakeTransactionList(const AIO_OFX_GROUP *g) {
  AIO_OFX_GROUP_BANKTRANLIST *xg;
  AB_TRANSACTION_LIST2 *tl;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BANKTRANLIST, g);
  assert(xg);

  tl=xg->transactionList;
  xg->transactionList=NULL;
  return tl;
}



int AIO_OfxGroup_BANKTRANLIST_StartTag(AIO_OFX_GROUP *g,
				 const char *tagName) {
  AIO_OFX_GROUP_BANKTRANLIST *xg;
  GWEN_XML_CONTEXT *ctx;
  AIO_OFX_GROUP *gNew=NULL;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BANKTRANLIST, g);
  assert(xg);

  ctx=AIO_OfxGroup_GetXmlContext(g);

  if (strcasecmp(tagName, "DTSTART")==0 ||
      strcasecmp(tagName, "DTEND")==0) {
    free(xg->currentElement);
    xg->currentElement=strdup(tagName);
  }
  else if (strcasecmp(tagName, "STMTTRN")==0) {
    gNew=AIO_OfxGroup_STMTRN_new(tagName, g, ctx);
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



int AIO_OfxGroup_BANKTRANLIST_AddData(AIO_OFX_GROUP *g, const char *data) {
  AIO_OFX_GROUP_BANKTRANLIST *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BANKTRANLIST, g);
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
      if (strcasecmp(xg->currentElement, "DTSTART")==0) {
	free(xg->dtstart);
	xg->dtstart=strdup(s);
      }
      else if (strcasecmp(xg->currentElement, "DTEND")==0) {
	free(xg->dtend);
	xg->dtend=strdup(s);
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



int AIO_OfxGroup_BANKTRANLIST_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg) {
  AIO_OFX_GROUP_BANKTRANLIST *xg;
  const char *s;
  GWEN_XML_CONTEXT *ctx;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BANKTRANLIST, g);
  assert(xg);

  ctx=AIO_OfxGroup_GetXmlContext(g);
  assert(ctx);

  s=AIO_OfxGroup_GetGroupName(sg);
  if (strcasecmp(s, "STMTTRN")==0) {
    AB_TRANSACTION *t;

    t=AIO_OfxGroup_STMTRN_TakeTransaction(sg);
    if (t) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Adding transaction");
      AB_Transaction_List2_PushBack(xg->transactionList, t);
    }
  }

  return 0;
}








