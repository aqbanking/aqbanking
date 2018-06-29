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

#include "g_posstock_p.h"
#include "ofxxmlctx_l.h"

#include "g_generic_l.h"
#include "g_ignore_l.h"

#include "g_invpos_l.h"

#include "i18n_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>





AIO_OFX_GROUP *AIO_OfxGroup_POSSTOCK_new(const char *groupName,
					   AIO_OFX_GROUP *parent,
					   GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_GROUP *g;

  /* create base group */
  g=AIO_OfxGroup_Generic_new(groupName, parent, ctx);
  assert(g);

  /* set virtual functions */
  AIO_OfxGroup_SetStartTagFn(g, AIO_OfxGroup_POSSTOCK_StartTag);
  AIO_OfxGroup_SetEndSubGroupFn(g, AIO_OfxGroup_POSSTOCK_EndSubGroup);

  return g;
}



int AIO_OfxGroup_POSSTOCK_StartTag(AIO_OFX_GROUP *g,
				   const char *tagName) {
  AIO_OFX_GROUP *gNew=NULL;
  GWEN_XML_CONTEXT *ctx;

  assert(g);

  ctx=AIO_OfxGroup_GetXmlContext(g);

  if (strcasecmp(tagName, "INVPOS")==0) {
    gNew=AIO_OfxGroup_INVPOS_new(tagName, g, ctx);
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



int AIO_OfxGroup_POSSTOCK_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg) {
  const char *s;
  GWEN_XML_CONTEXT *ctx;

  ctx=AIO_OfxGroup_GetXmlContext(g);

  s=AIO_OfxGroup_GetGroupName(sg);
  if (strcasecmp(s, "INVPOS")==0) {
    AB_SECURITY *sec;

    sec=AIO_OfxGroup_INVPOS_TakeSecurity(sg);
    if (sec) {
      AB_IMEXPORTER_CONTEXT *ioCtx;

      ioCtx=AIO_OfxXmlCtx_GetIoContext(ctx);
      DBG_INFO(AQBANKING_LOGDOMAIN, "Adding security");
      AB_ImExporterContext_AddSecurity(ioCtx, sec);
    }
  }

  return 0;
}



