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

#include "g_bankacctinfo_p.h"
#include "ofxxmlctx_l.h"
#include "i18n_l.h"

#include "g_generic_l.h"
#include "g_ignore_l.h"
#include "g_bankacc_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/gui.h>



GWEN_INHERIT(AIO_OFX_GROUP, AIO_OFX_GROUP_BANKACCTINFO)




AIO_OFX_GROUP *AIO_OfxGroup_BANKACCTINFO_new(const char *groupName,
					     AIO_OFX_GROUP *parent,
					     GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_GROUP *g;
  AIO_OFX_GROUP_BANKACCTINFO *xg;

  /* create base group */
  g=AIO_OfxGroup_Generic_new(groupName, parent, ctx);
  assert(g);

  GWEN_NEW_OBJECT(AIO_OFX_GROUP_BANKACCTINFO, xg);
  assert(xg);
  GWEN_INHERIT_SETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BANKACCTINFO, g, xg,
		       AIO_OfxGroup_BANKACCTINFO_FreeData);

  /* set virtual functions */
  AIO_OfxGroup_SetStartTagFn(g, AIO_OfxGroup_BANKACCTINFO_StartTag);
  AIO_OfxGroup_SetAddDataFn(g, AIO_OfxGroup_BANKACCTINFO_AddData);
  AIO_OfxGroup_SetEndSubGroupFn(g, AIO_OfxGroup_BANKACCTINFO_EndSubGroup);

  return g;
}



GWENHYWFAR_CB
void AIO_OfxGroup_BANKACCTINFO_FreeData(void *bp, void *p) {
  AIO_OFX_GROUP_BANKACCTINFO *xg;

  xg=(AIO_OFX_GROUP_BANKACCTINFO*)p;
  assert(xg);
  free(xg->currentElement);
  free(xg->bankId);
  free(xg->accId);
  free(xg->accType);
  GWEN_FREE_OBJECT(xg);
}



const char *AIO_OfxGroup_BANKACCTINFO_GetBankId(const AIO_OFX_GROUP *g) {
  AIO_OFX_GROUP_BANKACCTINFO *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BANKACCTINFO, g);
  assert(xg);

  return xg->bankId;
}



const char *AIO_OfxGroup_BANKACCTINFO_GetAccId(const AIO_OFX_GROUP *g) {
  AIO_OFX_GROUP_BANKACCTINFO *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BANKACCTINFO, g);
  assert(xg);

  return xg->accId;
}



const char *AIO_OfxGroup_BANKACCTINFO_GetAccType(const AIO_OFX_GROUP *g) {
  AIO_OFX_GROUP_BANKACCTINFO *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BANKACCTINFO, g);
  assert(xg);

  return xg->accType;
}



int AIO_OfxGroup_BANKACCTINFO_StartTag(AIO_OFX_GROUP *g,
				       const char *tagName) {
  AIO_OFX_GROUP_BANKACCTINFO *xg;
  GWEN_XML_CONTEXT *ctx;
  AIO_OFX_GROUP *gNew=NULL;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BANKACCTINFO, g);
  assert(xg);

  ctx=AIO_OfxGroup_GetXmlContext(g);

  free(xg->currentElement);
  xg->currentElement=NULL;

  if (strcasecmp(tagName, "USPRODUCTTYPE")==0 ||
      strcasecmp(tagName, "CHECKING")==0 ||
      strcasecmp(tagName, "OPTIONLEVEL")==0 ||
      strcasecmp(tagName, "SUPTXDL")==0 ||
      strcasecmp(tagName, "XFERSRC")==0 ||
      strcasecmp(tagName, "XFERDEST")==0 ||
      strcasecmp(tagName, "INVACCTTYPE")==0 ||
      strcasecmp(tagName, "SVCSTATUS")==0) {
    xg->currentElement=strdup(tagName);
  }
  else if (strcasecmp(tagName, "BANKACCTFROM")==0 ||
	   strcasecmp(tagName, "CCACCTFROM")==0 ||
	   strcasecmp(tagName, "INVACCTFROM")==0) {
    gNew=AIO_OfxGroup_BANKACC_new(tagName, g, ctx);
  }
  else {
    DBG_WARN(AQBANKING_LOGDOMAIN,
	     "Ignoring tag [%s]", tagName);
    xg->currentElement=strdup(tagName);
  }

  if (gNew) {
    AIO_OfxXmlCtx_SetCurrentGroup(ctx, gNew);
    GWEN_XmlCtx_IncDepth(ctx);
  }

  return 0;
}



int AIO_OfxGroup_BANKACCTINFO_AddData(AIO_OFX_GROUP *g, const char *data) {
  AIO_OFX_GROUP_BANKACCTINFO *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BANKACCTINFO, g);
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
      if (strcasecmp(xg->currentElement, "SUPTXDL")==0) {
      }
      else if (strcasecmp(xg->currentElement, "XFERSRC")==0) {
      }
      else if (strcasecmp(xg->currentElement, "XFERDEST")==0) {
      }
      else if (strcasecmp(xg->currentElement, "SVCSTATUS")==0) {
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



int AIO_OfxGroup_BANKACCTINFO_EndSubGroup(AIO_OFX_GROUP *g,
					  AIO_OFX_GROUP *sg) {
  AIO_OFX_GROUP_BANKACCTINFO *xg;
  const char *s;
  GWEN_XML_CONTEXT *ctx;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BANKACCTINFO, g);
  assert(xg);

  ctx=AIO_OfxGroup_GetXmlContext(g);
  assert(ctx);

  s=AIO_OfxGroup_GetGroupName(sg);
  if (strcasecmp(s, "BANKACCTFROM")==0 ||
      strcasecmp(s, "CCACCTFROM")==0 ||
      strcasecmp(s, "INVACCTFROM")==0) {
    const char *s;

    s=AIO_OfxGroup_BANKACC_GetBankId(sg);
    free(xg->bankId);
    if (s) xg->bankId=strdup(s);
    else xg->bankId=NULL;

    s=AIO_OfxGroup_BANKACC_GetAccId(sg);
    free(xg->accId);
    if (s) xg->accId=strdup(s);
    else xg->accId=NULL;

    s=AIO_OfxGroup_BANKACC_GetAccType(sg);
    free(xg->accType);
    if (s) xg->accType=strdup(s);
    else xg->accType=NULL;
  }

  return 0;
}








