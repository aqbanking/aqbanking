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

#include "g_invtran_p.h"
#include "ofxxmlctx_l.h"

#include "g_generic_l.h"
#include "g_ignore_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/gui.h>



GWEN_INHERIT(AIO_OFX_GROUP, AIO_OFX_GROUP_INVTRAN)




AIO_OFX_GROUP *AIO_OfxGroup_INVTRAN_new(const char *groupName,
				       AIO_OFX_GROUP *parent,
				       GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_GROUP *g;
  AIO_OFX_GROUP_INVTRAN *xg;

  /* create base group */
  g=AIO_OfxGroup_Generic_new(groupName, parent, ctx);
  assert(g);

  GWEN_NEW_OBJECT(AIO_OFX_GROUP_INVTRAN, xg);
  assert(xg);
  GWEN_INHERIT_SETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVTRAN, g, xg,
                       AIO_OfxGroup_INVTRAN_FreeData);

  xg->transaction=AB_Transaction_new();

  /* set virtual functions */
  AIO_OfxGroup_SetStartTagFn(g, AIO_OfxGroup_INVTRAN_StartTag);
  AIO_OfxGroup_SetAddDataFn(g, AIO_OfxGroup_INVTRAN_AddData);

  return g;
}



GWENHYWFAR_CB
void AIO_OfxGroup_INVTRAN_FreeData(void *bp, void *p) {
  AIO_OFX_GROUP_INVTRAN *xg;

  xg=(AIO_OFX_GROUP_INVTRAN*)p;
  assert(xg);
  free(xg->currentElement);
  AB_Transaction_free(xg->transaction);

  GWEN_FREE_OBJECT(xg);
}



int AIO_OfxGroup_INVTRAN_StartTag(AIO_OFX_GROUP *g,
				 const char *tagName) {
  AIO_OFX_GROUP_INVTRAN *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVTRAN, g);
  assert(xg);

  if (strcasecmp(tagName, "FITID")==0 ||
      strcasecmp(tagName, "DTTRADE")==0 ||
      strcasecmp(tagName, "DTSETTLE")==0 ||
      strcasecmp(tagName, "MEMO")==0) {
    free(xg->currentElement);
    xg->currentElement=strdup(tagName);
  }
  else {
    DBG_WARN(AQBANKING_LOGDOMAIN,
             "Ignoring tag [%s]", tagName);
    free(xg->currentElement);
    xg->currentElement=strdup(tagName);
  }

  return 0;
}



int AIO_OfxGroup_INVTRAN_AddData(AIO_OFX_GROUP *g, const char *data) {
  AIO_OFX_GROUP_INVTRAN *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVTRAN, g);
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
      if (strcasecmp(xg->currentElement, "FITID")==0) {
        AB_Transaction_SetFiId(xg->transaction, s);
      }  
      else if (strcasecmp(xg->currentElement, "DTTRADE")==0) {
        GWEN_TIME *ti;

        ti=GWEN_Time_fromString(s, "YYYYMMDD");
        if (ti==NULL) {
          DBG_ERROR(AQBANKING_LOGDOMAIN,
                    "Invalid data for DTTRADE: [%s]", s);
          GWEN_Buffer_free(buf);
          return GWEN_ERROR_BAD_DATA;
        }
        AB_Transaction_SetValutaDate(xg->transaction, ti);
        GWEN_Time_free(ti);
      }
      else if (strcasecmp(xg->currentElement, "DTSETTLE")==0) {
        GWEN_TIME *ti;

        ti=GWEN_Time_fromString(s, "YYYYMMDD");
        if (ti==NULL) {
          DBG_ERROR(AQBANKING_LOGDOMAIN,
                    "Invalid data for DTSETTLE: [%s]", s);
          GWEN_Buffer_free(buf);
          return GWEN_ERROR_BAD_DATA;
        }
        AB_Transaction_SetDate(xg->transaction, ti);
        GWEN_Time_free(ti);
      }
      else if (strcasecmp(xg->currentElement, "MEMO")==0) {
        AB_Transaction_AddPurpose(xg->transaction, s, 1);
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


AB_TRANSACTION *AIO_OfxGroup_INVTRAN_TakeData(const AIO_OFX_GROUP *g){
  AIO_OFX_GROUP_INVTRAN *xg;
  AB_TRANSACTION *t;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVTRAN, g);
  assert(xg);

  t=xg->transaction;
  xg->transaction=NULL;
  free(xg->transaction);

  return t;
}
