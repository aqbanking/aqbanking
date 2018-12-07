/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de
 copyright   : (C) 2013 by Paul Conrady
 email       : c.p.conrady@gmail.com

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "g_invpos_p.h"
#include "ofxxmlctx_l.h"

#include "g_generic_l.h"
#include "g_ignore_l.h"
#include "g_secid_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/gui.h>



GWEN_INHERIT(AIO_OFX_GROUP, AIO_OFX_GROUP_INVPOS)




AIO_OFX_GROUP *AIO_OfxGroup_INVPOS_new(const char *groupName,
				       AIO_OFX_GROUP *parent,
				       GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_GROUP *g;
  AIO_OFX_GROUP_INVPOS *xg;

  /* create base group */
  g=AIO_OfxGroup_Generic_new(groupName, parent, ctx);
  assert(g);

  GWEN_NEW_OBJECT(AIO_OFX_GROUP_INVPOS, xg);
  assert(xg);
  GWEN_INHERIT_SETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVPOS, g, xg,
                       AIO_OfxGroup_INVPOS_FreeData);

  xg->security=AB_Security_new();

  /* set virtual functions */
  AIO_OfxGroup_SetStartTagFn(g, AIO_OfxGroup_INVPOS_StartTag);
  AIO_OfxGroup_SetAddDataFn(g, AIO_OfxGroup_INVPOS_AddData);
  AIO_OfxGroup_SetEndSubGroupFn(g, AIO_OfxGroup_INVPOS_EndSubGroup);

  return g;
}



GWENHYWFAR_CB
void AIO_OfxGroup_INVPOS_FreeData(void *bp, void *p) {
  AIO_OFX_GROUP_INVPOS *xg;

  xg=(AIO_OFX_GROUP_INVPOS*)p;
  assert(xg);
  free(xg->currentElement);
  AB_Security_free(xg->security);

  GWEN_FREE_OBJECT(xg);
}



AB_SECURITY *AIO_OfxGroup_INVPOS_TakeSecurity(const AIO_OFX_GROUP *g){
  AIO_OFX_GROUP_INVPOS *xg;
  AB_SECURITY *sec;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVPOS, g);
  assert(xg);

  sec=xg->security;
  xg->security=NULL;
  return sec;
}



int AIO_OfxGroup_INVPOS_StartTag(AIO_OFX_GROUP *g,
				 const char *tagName) {
  AIO_OFX_GROUP_INVPOS *xg;
  GWEN_XML_CONTEXT *ctx;
  AIO_OFX_GROUP *gNew=NULL;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVPOS, g);
  assert(xg);

  ctx=AIO_OfxGroup_GetXmlContext(g);

  free(xg->currentElement);
  xg->currentElement=NULL;

  if (strcasecmp(tagName, "HELDINACCT")==0 ||
      strcasecmp(tagName, "POSTYPE")==0 ||
      strcasecmp(tagName, "UNITS")==0 ||
      strcasecmp(tagName, "UNITPRICE")==0 ||
      strcasecmp(tagName, "MKTVAL")==0 ||
      strcasecmp(tagName, "DTPRICEASOF")==0 ||
      strcasecmp(tagName, "MEMO")==0) {
    free(xg->currentElement);
    xg->currentElement=strdup(tagName);
  }
  else if (strcasecmp(tagName, "SECID")==0) {
    gNew=AIO_OfxGroup_SECID_new(tagName, g, ctx);
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



int AIO_OfxGroup_INVPOS_AddData(AIO_OFX_GROUP *g, const char *data) {
  AIO_OFX_GROUP_INVPOS *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVPOS, g);
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
      if (strcasecmp(xg->currentElement, "UNITS")==0) {
	AB_VALUE *v;

	v=AB_Value_fromString(s);
	if (v==NULL) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Invalid data for UNITS: [%s]", s);
	  GWEN_Buffer_free(buf);
	  return GWEN_ERROR_BAD_DATA;
	}
	AB_Security_SetUnits(xg->security, v);
	AB_Value_free(v);
      }
      else if (strcasecmp(xg->currentElement, "UNITPRICE")==0) {
	AB_VALUE *v;

	v=AB_Value_fromString(s);
	if (v==NULL) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Invalid data for UNITPRICE: [%s]", s);
	  GWEN_Buffer_free(buf);
	  return GWEN_ERROR_BAD_DATA;
	}
	if (xg->currency)
          AB_Value_SetCurrency(v, xg->currency);
	AB_Security_SetUnitPriceValue(xg->security, v);
	AB_Value_free(v);
      }
      else if (strcasecmp(xg->currentElement, "DTPRICEASOF")==0) {
	GWEN_TIME *ti;

	ti=GWEN_Time_fromString(s, "YYYYMMDD");
	if (ti==NULL) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Invalid data for DTPRICEASOF: [%s]", s);
	  GWEN_Buffer_free(buf);
          return GWEN_ERROR_BAD_DATA;
	}
	AB_Security_SetUnitPriceDate(xg->security, ti);
	GWEN_Time_free(ti);
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



int AIO_OfxGroup_INVPOS_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg) {
  AIO_OFX_GROUP_INVPOS *xg;
  const char *s;
  GWEN_XML_CONTEXT *ctx;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVPOS, g);
  assert(xg);

  ctx=AIO_OfxGroup_GetXmlContext(g);
  assert(ctx);

  s=AIO_OfxGroup_GetGroupName(sg);
  if (strcasecmp(s, "SECID")==0) {
    AB_Security_SetUniqueId(xg->security,
                            AIO_OfxGroup_SECID_GetUniqueId(sg));
    AB_Security_SetNameSpace(xg->security,
                             AIO_OfxGroup_SECID_GetNameSpace(sg));
  }

  return 0;
}








