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

#include "g_secinfo_p.h"
#include "ofxxmlctx_l.h"
#include "i18n_l.h"

#include "g_generic_l.h"
#include "g_ignore_l.h"
#include "g_secid_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/gui.h>



GWEN_INHERIT(AIO_OFX_GROUP, AIO_OFX_GROUP_SECINFO)




AIO_OFX_GROUP *AIO_OfxGroup_SECINFO_new(const char *groupName,
					AIO_OFX_GROUP *parent,
					GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_GROUP *g;
  AIO_OFX_GROUP_SECINFO *xg;

  /* create base group */
  g=AIO_OfxGroup_Generic_new(groupName, parent, ctx);
  assert(g);

  GWEN_NEW_OBJECT(AIO_OFX_GROUP_SECINFO, xg);
  assert(xg);
  GWEN_INHERIT_SETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_SECINFO, g, xg,
                       AIO_OfxGroup_SECINFO_FreeData);

  /* set virtual functions */
  AIO_OfxGroup_SetStartTagFn(g, AIO_OfxGroup_SECINFO_StartTag);
  AIO_OfxGroup_SetAddDataFn(g, AIO_OfxGroup_SECINFO_AddData);
  AIO_OfxGroup_SetEndSubGroupFn(g, AIO_OfxGroup_SECINFO_EndSubGroup);

  return g;
}



GWENHYWFAR_CB
void AIO_OfxGroup_SECINFO_FreeData(void *bp, void *p) {
  AIO_OFX_GROUP_SECINFO *xg;

  xg=(AIO_OFX_GROUP_SECINFO*)p;
  assert(xg);
  free(xg->currentElement);
  free(xg->ticker);
  free(xg->secname);
  free(xg->uniqueId);
  free(xg->nameSpace);
  GWEN_FREE_OBJECT(xg);
}



const char *AIO_OfxGroup_SECINFO_GetTicker(const AIO_OFX_GROUP *g) {
  AIO_OFX_GROUP_SECINFO *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_SECINFO, g);
  assert(xg);

  return xg->ticker;
}



void AIO_OfxGroup_SECINFO_SetTicker(AIO_OFX_GROUP *g, const char *s) {
  AIO_OFX_GROUP_SECINFO *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_SECINFO, g);
  assert(xg);

  free(xg->ticker);
  if (s) xg->ticker=strdup(s);
  else xg->ticker=NULL;
}



const char *AIO_OfxGroup_SECINFO_GetSecurityName(const AIO_OFX_GROUP *g) {
  AIO_OFX_GROUP_SECINFO *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_SECINFO, g);
  assert(xg);

  return xg->secname;
}



void AIO_OfxGroup_SECINFO_SetSecurityName(AIO_OFX_GROUP *g, const char *s) {
  AIO_OFX_GROUP_SECINFO *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_SECINFO, g);
  assert(xg);

  free(xg->secname);
  if (s) xg->secname=strdup(s);
  else xg->secname=NULL;
}



const char *AIO_OfxGroup_SECINFO_GetUniqueId(const AIO_OFX_GROUP *g) {
  AIO_OFX_GROUP_SECINFO *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_SECINFO, g);
  assert(xg);

  return xg->uniqueId;
}



void AIO_OfxGroup_SECINFO_SetUniqueId(AIO_OFX_GROUP *g, const char *s) {
  AIO_OFX_GROUP_SECINFO *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_SECINFO, g);
  assert(xg);

  free(xg->uniqueId);
  if (s) xg->uniqueId=strdup(s);
  else xg->uniqueId=NULL;
}



const char *AIO_OfxGroup_SECINFO_GetNameSpace(const AIO_OFX_GROUP *g) {
  AIO_OFX_GROUP_SECINFO *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_SECINFO, g);
  assert(xg);

  return xg->nameSpace;
}



void AIO_OfxGroup_SECINFO_SetNameSpace(AIO_OFX_GROUP *g, const char *s) {
  AIO_OFX_GROUP_SECINFO *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_SECINFO, g);
  assert(xg);

  free(xg->nameSpace);
  if (s) xg->nameSpace=strdup(s);
  else xg->nameSpace=NULL;
}



int AIO_OfxGroup_SECINFO_StartTag(AIO_OFX_GROUP *g,
				  const char *tagName) {
  AIO_OFX_GROUP_SECINFO *xg;
  GWEN_XML_CONTEXT *ctx;
  AIO_OFX_GROUP *gNew=NULL;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_SECINFO, g);
  assert(xg);

  ctx=AIO_OfxGroup_GetXmlContext(g);

  free(xg->currentElement);
  xg->currentElement=NULL;

  if (strcasecmp(tagName, "SECNAME")==0 ||
      strcasecmp(tagName, "TICKER")==0 ||
      strcasecmp(tagName, "FIID")==0 ||
      strcasecmp(tagName, "UNITPRICE")==0 ||
      strcasecmp(tagName, "DTASOF")==0) {
    xg->currentElement=strdup(tagName);
  }
  else if (strcasecmp(tagName, "SECID")==0) {
    gNew=AIO_OfxGroup_SECID_new(tagName, g, ctx);
  }
  else {
    DBG_WARN(AQBANKING_LOGDOMAIN,
	     "Ignoring tag [%s]", tagName);
  }

  if (gNew) {
    AIO_OfxXmlCtx_SetCurrentGroup(ctx, gNew);
    GWEN_XmlCtx_IncDepth(ctx);
  }

  return 0;
}



int AIO_OfxGroup_SECINFO_AddData(AIO_OFX_GROUP *g, const char *data) {
  AIO_OFX_GROUP_SECINFO *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_SECINFO, g);
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
      if (strcasecmp(xg->currentElement, "SECNAME")==0)
	AIO_OfxGroup_SECINFO_SetSecurityName(g, GWEN_Buffer_GetStart(buf));
      else if (strcasecmp(xg->currentElement, "TICKER")==0)
	AIO_OfxGroup_SECINFO_SetTicker(g, GWEN_Buffer_GetStart(buf));
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



int AIO_OfxGroup_SECINFO_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg) {
  AIO_OFX_GROUP_SECINFO *xg;
  const char *s;
  GWEN_XML_CONTEXT *ctx;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_SECINFO, g);
  assert(xg);

  ctx=AIO_OfxGroup_GetXmlContext(g);
  assert(ctx);

  s=AIO_OfxGroup_GetGroupName(sg);
  if (strcasecmp(s, "SECID")==0) {
    AIO_OfxGroup_SECINFO_SetUniqueId(g, AIO_OfxGroup_SECID_GetUniqueId(sg));
    AIO_OfxGroup_SECINFO_SetNameSpace(g, AIO_OfxGroup_SECID_GetNameSpace(sg));
  }

  return 0;
}





