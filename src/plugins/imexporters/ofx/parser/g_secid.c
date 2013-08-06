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

#include "g_secid_p.h"
#include "ofxxmlctx_l.h"

#include "g_generic_l.h"
#include "g_ignore_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/gui.h>



GWEN_INHERIT(AIO_OFX_GROUP, AIO_OFX_GROUP_SECID)




AIO_OFX_GROUP *AIO_OfxGroup_SECID_new(const char *groupName,
				      AIO_OFX_GROUP *parent,
				      GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_GROUP *g;
  AIO_OFX_GROUP_SECID *xg;

  /* create base group */
  g=AIO_OfxGroup_Generic_new(groupName, parent, ctx);
  assert(g);

  GWEN_NEW_OBJECT(AIO_OFX_GROUP_SECID, xg);
  assert(xg);
  GWEN_INHERIT_SETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_SECID, g, xg,
                       AIO_OfxGroup_SECID_FreeData);

  /* set virtual functions */
  AIO_OfxGroup_SetStartTagFn(g, AIO_OfxGroup_SECID_StartTag);
  AIO_OfxGroup_SetAddDataFn(g, AIO_OfxGroup_SECID_AddData);

  return g;
}



GWENHYWFAR_CB
void AIO_OfxGroup_SECID_FreeData(void *bp, void *p) {
  AIO_OFX_GROUP_SECID *xg;

  xg=(AIO_OFX_GROUP_SECID*)p;
  assert(xg);
  free(xg->currentElement);
  free(xg->uniqueId);
  free(xg->nameSpace);
  GWEN_FREE_OBJECT(xg);
}



const char *AIO_OfxGroup_SECID_GetUniqueId(const AIO_OFX_GROUP *g) {
  AIO_OFX_GROUP_SECID *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_SECID, g);
  assert(xg);

  return xg->uniqueId;
}



void AIO_OfxGroup_SECID_SetUniqueId(AIO_OFX_GROUP *g, const char *s) {
  AIO_OFX_GROUP_SECID *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_SECID, g);
  assert(xg);

  free(xg->uniqueId);
  if (s) xg->uniqueId=strdup(s);
  else xg->uniqueId=NULL;
}



const char *AIO_OfxGroup_SECID_GetNameSpace(const AIO_OFX_GROUP *g) {
  AIO_OFX_GROUP_SECID *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_SECID, g);
  assert(xg);

  return xg->nameSpace;
}



void AIO_OfxGroup_SECID_SetNameSpace(AIO_OFX_GROUP *g, const char *s) {
  AIO_OFX_GROUP_SECID *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_SECID, g);
  assert(xg);

  free(xg->nameSpace);
  if (s) xg->nameSpace=strdup(s);
  else xg->nameSpace=NULL;
}



int AIO_OfxGroup_SECID_StartTag(AIO_OFX_GROUP *g,
				 const char *tagName) {
  AIO_OFX_GROUP_SECID *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_SECID, g);
  assert(xg);

  free(xg->currentElement);
  xg->currentElement=NULL;

  if (strcasecmp(tagName, "UNIQUEID")==0 ||
      strcasecmp(tagName, "UNIQUEIDTYPE")==0) {
    xg->currentElement=strdup(tagName);
  }
  else {
    DBG_WARN(AQBANKING_LOGDOMAIN,
	     "Ignoring tag [%s]", tagName);
  }

  return 0;
}



int AIO_OfxGroup_SECID_AddData(AIO_OFX_GROUP *g, const char *data) {
  AIO_OFX_GROUP_SECID *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_SECID, g);
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
      if (strcasecmp(xg->currentElement, "UNIQUEID")==0)
	AIO_OfxGroup_SECID_SetUniqueId(g, GWEN_Buffer_GetStart(buf));
      else if (strcasecmp(xg->currentElement, "UNIQUEIDTYPE")==0)
	AIO_OfxGroup_SECID_SetNameSpace(g, GWEN_Buffer_GetStart(buf));
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



AB_TRANSACTION *AIO_OfxGroup_SECID_TakeData(const AIO_OFX_GROUP *g){
  AIO_OFX_GROUP_SECID *xg;
  AB_TRANSACTION *t;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_SECID, g);
  assert(xg);

  t=xg->transaction;
  xg->transaction=NULL;
  free(xg->transaction);

  return t;
}
