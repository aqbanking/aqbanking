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

#include "g_stockinfo_p.h"
#include "ofxxmlctx_l.h"
#include "i18n_l.h"

#include "g_generic_l.h"
#include "g_ignore_l.h"
#include "g_secinfo_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/gui.h>




AIO_OFX_GROUP *AIO_OfxGroup_STOCKINFO_new(const char *groupName,
					   AIO_OFX_GROUP *parent,
					   GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_GROUP *g;

  /* create base group */
  g=AIO_OfxGroup_Generic_new(groupName, parent, ctx);
  assert(g);

  /* set virtual functions */
  AIO_OfxGroup_SetStartTagFn(g, AIO_OfxGroup_STOCKINFO_StartTag);
  AIO_OfxGroup_SetEndSubGroupFn(g, AIO_OfxGroup_STOCKINFO_EndSubGroup);

  return g;
}



int AIO_OfxGroup_STOCKINFO_StartTag(AIO_OFX_GROUP *g,
				    const char *tagName) {
  GWEN_XML_CONTEXT *ctx;
  AIO_OFX_GROUP *gNew=NULL;

  assert(g);

  ctx=AIO_OfxGroup_GetXmlContext(g);

  if (strcasecmp(tagName, "SECINFO")==0) {
    gNew=AIO_OfxGroup_SECINFO_new(tagName, g, ctx);
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



int AIO_OfxGroup_STOCKINFO_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg) {
  const char *s;
  GWEN_XML_CONTEXT *ctx;

  assert(g);

  ctx=AIO_OfxGroup_GetXmlContext(g);
  assert(ctx);

  s=AIO_OfxGroup_GetGroupName(sg);
  if (strcasecmp(s, "SECINFO")==0) {
    AB_SECURITY *sec=NULL;
    const char *uid;
    const char *ns;

    uid=AIO_OfxGroup_SECINFO_GetUniqueId(sg);
    ns=AIO_OfxGroup_SECINFO_GetNameSpace(sg);
    if (uid && ns)
      sec=AB_ImExporterContext_FindSecurity(AIO_OfxXmlCtx_GetIoContext(ctx),
					    ns, uid);
    if (sec==NULL) {
      sec=AB_Security_new();
      AB_Security_SetUniqueId(sec, uid);
      AB_Security_SetNameSpace(sec, ns);
      AB_ImExporterContext_AddSecurity(AIO_OfxXmlCtx_GetIoContext(ctx), sec);
    }

    AB_Security_SetName(sec, AIO_OfxGroup_SECINFO_GetSecurityName(sg));
    AB_Security_SetTickerSymbol(sec, AIO_OfxGroup_SECINFO_GetTicker(sg));
  }

  return 0;
}








