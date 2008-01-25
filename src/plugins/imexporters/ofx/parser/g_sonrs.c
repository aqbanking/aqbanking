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

#include "g_sonrs_p.h"
#include "ofxxmlctx_l.h"

#include "g_generic_l.h"
#include "g_ignore_l.h"
#include "g_status_l.h"

#include "i18n_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>





AIO_OFX_GROUP *AIO_OfxGroup_SONRS_new(const char *groupName,
				      AIO_OFX_GROUP *parent,
				      GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_GROUP *g;

  /* create base group */
  g=AIO_OfxGroup_Generic_new(groupName, parent, ctx);
  assert(g);

  /* set virtual functions */
  AIO_OfxGroup_SetStartTagFn(g, AIO_OfxGroup_SONRS_StartTag);

  return g;
}



int AIO_OfxGroup_SONRS_StartTag(AIO_OFX_GROUP *g,
				const char *tagName) {
  AIO_OFX_GROUP *gNew=NULL;
  GWEN_XML_CONTEXT *ctx;

  assert(g);

  ctx=AIO_OfxGroup_GetXmlContext(g);

  if (strcasecmp(tagName, "STATUS")==0) {
    gNew=AIO_OfxGroup_STATUS_new(tagName, g, ctx,
				 I18N("Status for signon request"));
  }
  else if (strcasecmp(tagName, "DTSERVER")==0 ||
	   strcasecmp(tagName, "LANGUAGE")==0 ||
	   strcasecmp(tagName, "DTPROFUP")==0 ||
	   strcasecmp(tagName, "DTACCTUP")==0 ||
	   strcasecmp(tagName, "SESSCOOKIE")==0) {
    /* some tags, just ignore them here */
  }
  else if (strcasecmp(tagName, "FI")==0) {
    gNew=AIO_OfxGroup_Ignore_new(tagName, g, ctx);
  }
  else if (-1!=GWEN_Text_ComparePattern(tagName, "INTU.*", 0) ||
	   -1!=GWEN_Text_ComparePattern(tagName, "AT.*", 0)) {
    /* simply ignore INTU. stuff */
  }
  else {
    DBG_WARN(AQBANKING_LOGDOMAIN,
	     "Ignoring element [%s]", tagName);
    /*gNew=AIO_OfxGroup_Ignore_new(tagName, g, ctx);*/
  }

  if (gNew) {
    AIO_OfxXmlCtx_SetCurrentGroup(ctx, gNew);
    GWEN_XmlCtx_IncDepth(ctx);
  }

  return 0;
}



