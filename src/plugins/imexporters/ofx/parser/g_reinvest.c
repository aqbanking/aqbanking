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

#include "g_reinvest_p.h"
#include "ofxxmlctx_l.h"

#include "g_generic_l.h"
#include "g_ignore_l.h"
#include "g_invtran_l.h"
#include "g_secid_l.h"
#include "types/transaction.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/gui.h>



GWEN_INHERIT(AIO_OFX_GROUP, AIO_OFX_GROUP_REINVEST)




AIO_OFX_GROUP *AIO_OfxGroup_REINVEST_new(const char *groupName,
					     AIO_OFX_GROUP *parent,
					     GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_GROUP *g;
  AIO_OFX_GROUP_REINVEST *xg;

  /* create base group */
  g=AIO_OfxGroup_Generic_new(groupName, parent, ctx);
  assert(g);

  GWEN_NEW_OBJECT(AIO_OFX_GROUP_REINVEST, xg);
  assert(xg);
  GWEN_INHERIT_SETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_REINVEST, g, xg,
                       AIO_OfxGroup_REINVEST_FreeData);

  xg->transaction=AB_Transaction_new();

  /* set virtual functions */
  AIO_OfxGroup_SetStartTagFn(g, AIO_OfxGroup_REINVEST_StartTag);
  AIO_OfxGroup_SetAddDataFn(g, AIO_OfxGroup_REINVEST_AddData);
  AIO_OfxGroup_SetEndSubGroupFn(g, AIO_OfxGroup_REINVEST_EndSubGroup);

  return g;
}



GWENHYWFAR_CB
void AIO_OfxGroup_REINVEST_FreeData(void *bp, void *p) {
  AIO_OFX_GROUP_REINVEST *xg;

  xg=(AIO_OFX_GROUP_REINVEST*)p;
  assert(xg);
  AB_Transaction_free(xg->transaction);

  free(xg->currentElement);
  GWEN_FREE_OBJECT(xg);
}



int AIO_OfxGroup_REINVEST_StartTag(AIO_OFX_GROUP *g,
				 const char *tagName) {
  AIO_OFX_GROUP_REINVEST *xg;
  GWEN_XML_CONTEXT *ctx;
  AIO_OFX_GROUP *gNew=NULL;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_REINVEST, g);
  assert(xg);

  ctx=AIO_OfxGroup_GetXmlContext(g);

  if (strcasecmp(tagName, "UNITS")==0 ||
      strcasecmp(tagName, "UNITPRICE")==0 ||
      strcasecmp(tagName, "TOTAL")==0 ||
      strcasecmp(tagName, "SUBACCTSEC")==0 ||
      strcasecmp(tagName, "INCOMETYPE")==0) {
    free(xg->currentElement);
    xg->currentElement=strdup(tagName);
  }
  else if (strcasecmp(tagName, "INVTRAN")==0) {
    gNew=AIO_OfxGroup_INVTRAN_new(tagName, g, ctx);
  }
  else if (strcasecmp(tagName, "SECID")==0) {
   gNew=AIO_OfxGroup_SECID_new(tagName, g, ctx);
  }
  else {
    DBG_WARN(AQBANKING_LOGDOMAIN,
	         "Ignoring tag [%s]", tagName);
    free(xg->currentElement);
    xg->currentElement=strdup(tagName);
  }

  if (gNew) {
    AIO_OfxXmlCtx_SetCurrentGroup(ctx, gNew);
    GWEN_XmlCtx_IncDepth(ctx);
  }

  return 0;
}



int AIO_OfxGroup_REINVEST_AddData(AIO_OFX_GROUP *g, const char *data) {
  AIO_OFX_GROUP_REINVEST *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_REINVEST, g);
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
        AB_Transaction_SetUnits(xg->transaction, v);
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
        AB_Transaction_SetUnitPrice(xg->transaction, v);
        AB_Value_free(v);
      }
      else if (strcasecmp(xg->currentElement, "TOTAL")==0) {
        AB_VALUE *v;

        v=AB_Value_fromString(s);
        if (v==NULL) {
          DBG_ERROR(AQBANKING_LOGDOMAIN,
                    "Invalid data for TOTAL: [%s]", s);
          GWEN_Buffer_free(buf);
          return GWEN_ERROR_BAD_DATA;
        }
        AB_Transaction_SetValue(xg->transaction, v);
        AB_Value_free(v);
      }
      else if (strcasecmp(xg->currentElement, "SUBACCTSEC")==0) {
        /* TODO */
      }
      else if (strcasecmp(xg->currentElement, "INCOMETYPE")==0) {
        /* TODO */
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



int AIO_OfxGroup_REINVEST_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg) {
  AIO_OFX_GROUP_REINVEST *xg;
  const char *s;
  GWEN_XML_CONTEXT *ctx;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_REINVEST, g);
  assert(xg);

  ctx=AIO_OfxGroup_GetXmlContext(g);
  assert(ctx);

  s=AIO_OfxGroup_GetGroupName(sg);
  if (strcasecmp(s, "INVTRAN")==0) {
    AB_TRANSACTION *t;

    t=AIO_OfxGroup_INVTRAN_TakeData(sg);
    if (t) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Adding data");
      AB_Transaction_SetFiId(xg->transaction, AB_Transaction_GetFiId(t));
      AB_Transaction_SetValutaDate(xg->transaction, AB_Transaction_GetValutaDate(t));
      AB_Transaction_SetDate(xg->transaction, AB_Transaction_GetDate(t));
      AB_Transaction_SetPurpose(xg->transaction, AB_Transaction_GetPurpose(t));
    }
  }
  else if (strcasecmp(s, "SECID")==0) {
    AB_TRANSACTION *t;
    
    t=AIO_OfxGroup_SECID_TakeData(sg);
    if (t) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Adding data");
      AB_Transaction_SetUnitId(xg->transaction, AB_Transaction_GetUnitId(t));
      AB_Transaction_SetUnitIdNameSpace(xg->transaction, AB_Transaction_GetUnitIdNameSpace(t));
    }
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN,
             "Ignoring data for unknown element [%s]", s);
  }

  return 0;
}



AB_TRANSACTION *AIO_OfxGroup_REINVEST_TakeTransaction(const AIO_OFX_GROUP *g){
  AIO_OFX_GROUP_REINVEST *xg;
  AB_TRANSACTION *t;
 
  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_REINVEST, g);
  assert(xg);
  
  t=xg->transaction;
  xg->transaction=NULL;
  free(xg->transaction);

  return t;
}

