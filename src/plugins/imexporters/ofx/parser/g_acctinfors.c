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

#include "g_acctinfors_p.h"
#include "ofxxmlctx_l.h"
#include "i18n_l.h"

#include "g_generic_l.h"
#include "g_ignore_l.h"
#include "g_acctinfo_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/gui.h>




AIO_OFX_GROUP *AIO_OfxGroup_ACCTINFORS_new(const char *groupName,
					   AIO_OFX_GROUP *parent,
					   GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_GROUP *g;

  /* create base group */
  g=AIO_OfxGroup_Generic_new(groupName, parent, ctx);
  assert(g);

  /* set virtual functions */
  AIO_OfxGroup_SetStartTagFn(g, AIO_OfxGroup_ACCTINFORS_StartTag);
  AIO_OfxGroup_SetEndSubGroupFn(g, AIO_OfxGroup_ACCTINFORS_EndSubGroup);

  return g;
}



int AIO_OfxGroup_ACCTINFORS_StartTag(AIO_OFX_GROUP *g,
				     const char *tagName) {
  GWEN_XML_CONTEXT *ctx;
  AIO_OFX_GROUP *gNew=NULL;

  assert(g);

  ctx=AIO_OfxGroup_GetXmlContext(g);

  if (strcasecmp(tagName, "ACCTINFO")==0) {
    gNew=AIO_OfxGroup_ACCTINFO_new(tagName, g, ctx);
  }
  else if (strcasecmp(tagName, "DTACCTUP")==0) {
    /* ignore */
  }
  else if (strcasecmp(tagName, "ESP.XREGION")==0) {
    /* ignore */
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



int AIO_OfxGroup_ACCTINFORS_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg) {
  const char *s;
  GWEN_XML_CONTEXT *ctx;

  assert(g);

  ctx=AIO_OfxGroup_GetXmlContext(g);
  assert(ctx);

  s=AIO_OfxGroup_GetGroupName(sg);
  if (strcasecmp(s, "ACCTINFO")==0) {
    AB_IMEXPORTER_ACCOUNTINFO *ai;
    const char *s;

    DBG_INFO(AQBANKING_LOGDOMAIN,
	     "Importing account %s/%s",
	     AIO_OfxGroup_ACCTINFO_GetBankId(sg),
	     AIO_OfxGroup_ACCTINFO_GetAccId(sg));
    ai=AB_ImExporterAccountInfo_new();
    assert(ai);

    s=AIO_OfxGroup_ACCTINFO_GetBankId(sg);
    if (s)
      AB_ImExporterAccountInfo_SetBankCode(ai, s);
    s=AIO_OfxGroup_ACCTINFO_GetAccId(sg);
    if (s)
      AB_ImExporterAccountInfo_SetAccountNumber(ai, s);
    s=AIO_OfxGroup_ACCTINFO_GetAccDescr(sg);
    if (s)
      AB_ImExporterAccountInfo_SetAccountName(ai, s);

    /* set account type, if known */
    s=AIO_OfxGroup_ACCTINFO_GetAccType(sg);
    if (!s)
      s="BANK"; /* not a real code */
    if (s) {
      AB_ACCOUNT_TYPE t;

      t=AIO_OfxGroup_Generic_AccountTypeFromString(s);
      AB_ImExporterAccountInfo_SetType(ai, t);
    }

    DBG_INFO(AQBANKING_LOGDOMAIN, "Adding account");
    AB_ImExporterContext_AddAccountInfo(AIO_OfxXmlCtx_GetIoContext(ctx), ai);
  }

  return 0;
}








