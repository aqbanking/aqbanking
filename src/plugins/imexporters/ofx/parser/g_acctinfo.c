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

#include "g_acctinfo_p.h"
#include "ofxxmlctx_l.h"
#include "i18n_l.h"

#include "g_generic_l.h"
#include "g_ignore_l.h"
#include "g_bankacc_l.h"
#include "g_bankacctinfo_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/gui.h>



GWEN_INHERIT(AIO_OFX_GROUP, AIO_OFX_GROUP_ACCTINFO)




AIO_OFX_GROUP *AIO_OfxGroup_ACCTINFO_new(const char *groupName,
					 AIO_OFX_GROUP *parent,
					 GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_GROUP *g;
  AIO_OFX_GROUP_ACCTINFO *xg;

  /* create base group */
  g=AIO_OfxGroup_Generic_new(groupName, parent, ctx);
  assert(g);

  GWEN_NEW_OBJECT(AIO_OFX_GROUP_ACCTINFO, xg);
  assert(xg);
  GWEN_INHERIT_SETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_ACCTINFO, g, xg,
                       AIO_OfxGroup_ACCTINFO_FreeData);

  /* set virtual functions */
  AIO_OfxGroup_SetStartTagFn(g, AIO_OfxGroup_ACCTINFO_StartTag);
  AIO_OfxGroup_SetAddDataFn(g, AIO_OfxGroup_ACCTINFO_AddData);
  AIO_OfxGroup_SetEndSubGroupFn(g, AIO_OfxGroup_ACCTINFO_EndSubGroup);

  return g;
}



GWENHYWFAR_CB
void AIO_OfxGroup_ACCTINFO_FreeData(void *bp, void *p) {
  AIO_OFX_GROUP_ACCTINFO *xg;

  xg=(AIO_OFX_GROUP_ACCTINFO*)p;
  assert(xg);
  free(xg->description);
  free(xg->bankId);
  free(xg->accId);
  free(xg->accType);
  free(xg->currentElement);
  GWEN_FREE_OBJECT(xg);
}



const char *AIO_OfxGroup_ACCTINFO_GetBankId(const AIO_OFX_GROUP *g) {
  AIO_OFX_GROUP_ACCTINFO *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_ACCTINFO, g);
  assert(xg);

  return xg->bankId;
}



const char *AIO_OfxGroup_ACCTINFO_GetAccId(const AIO_OFX_GROUP *g) {
  AIO_OFX_GROUP_ACCTINFO *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_ACCTINFO, g);
  assert(xg);

  return xg->accId;
}



const char *AIO_OfxGroup_ACCTINFO_GetAccType(const AIO_OFX_GROUP *g) {
  AIO_OFX_GROUP_ACCTINFO *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_ACCTINFO, g);
  assert(xg);

  return xg->accType;
}



const char *AIO_OfxGroup_ACCTINFO_GetAccDescr(const AIO_OFX_GROUP *g) {
  AIO_OFX_GROUP_ACCTINFO *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_ACCTINFO, g);
  assert(xg);

  return xg->description;
}



int AIO_OfxGroup_ACCTINFO_StartTag(AIO_OFX_GROUP *g,
				   const char *tagName) {
  AIO_OFX_GROUP_ACCTINFO *xg;
  GWEN_XML_CONTEXT *ctx;
  AIO_OFX_GROUP *gNew=NULL;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_ACCTINFO, g);
  assert(xg);

  ctx=AIO_OfxGroup_GetXmlContext(g);

  free(xg->currentElement);
  xg->currentElement=NULL;

  if (strcasecmp(tagName, "DESC")==0) {
    xg->currentElement=strdup(tagName);
  }
  else if (strcasecmp(tagName, "BANKACCTINFO")==0 ||
	   strcasecmp(tagName, "CCACCTINFO")==0 ||
	   strcasecmp(tagName, "BPACCTINFO")==0 ||
	   strcasecmp(tagName, "INVACCTINFO")==0) {
    gNew=AIO_OfxGroup_BANKACCTINFO_new(tagName, g, ctx);
  }
  else if (strcasecmp(tagName, "DESC")==0) {
    xg->currentElement=strdup(tagName);
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



int AIO_OfxGroup_ACCTINFO_AddData(AIO_OFX_GROUP *g, const char *data) {
  AIO_OFX_GROUP_ACCTINFO *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_ACCTINFO, g);
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
      if (strcasecmp(xg->currentElement, "DESC")==0) {
	free(xg->description);
        xg->description=strdup(s);
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



int AIO_OfxGroup_ACCTINFO_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg) {
  AIO_OFX_GROUP_ACCTINFO *xg;
  const char *s;
  GWEN_XML_CONTEXT *ctx;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_ACCTINFO, g);
  assert(xg);

  ctx=AIO_OfxGroup_GetXmlContext(g);
  assert(ctx);

  s=AIO_OfxGroup_GetGroupName(sg);
  if (strcasecmp(s, "BANKACCTINFO")==0 ||
      strcasecmp(s, "CCACCTINFO")==0 ||
      strcasecmp(s, "BPACCTINFO")==0 ||
      strcasecmp(s, "INVACCTINFO")==0) {
    const char *s;

    s=AIO_OfxGroup_BANKACCTINFO_GetBankId(sg);
    free(xg->bankId);
    if (s) xg->bankId=strdup(s);
    else xg->bankId=NULL;

    s=AIO_OfxGroup_BANKACCTINFO_GetAccId(sg);
    free(xg->accId);
    if (s) xg->accId=strdup(s);
    else xg->accId=NULL;

    s=AIO_OfxGroup_BANKACCTINFO_GetAccType(sg);
    free(xg->accType);
    if (s) xg->accType=strdup(s);
    else xg->accType=NULL;
  }

  return 0;
}








