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

#include "g_bal_p.h"
#include "ofxxmlctx_l.h"
#include "i18n_l.h"

#include "g_generic_l.h"
#include "g_ignore_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/gui.h>



GWEN_INHERIT(AIO_OFX_GROUP, AIO_OFX_GROUP_BAL)




AIO_OFX_GROUP *AIO_OfxGroup_BAL_new(const char *groupName,
					AIO_OFX_GROUP *parent,
					GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_GROUP *g;
  AIO_OFX_GROUP_BAL *xg;

  /* create base group */
  g=AIO_OfxGroup_Generic_new(groupName, parent, ctx);
  assert(g);

  GWEN_NEW_OBJECT(AIO_OFX_GROUP_BAL, xg);
  assert(xg);
  GWEN_INHERIT_SETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BAL, g, xg,
                       AIO_OfxGroup_BAL_FreeData);

  /* set virtual functions */
  AIO_OfxGroup_SetStartTagFn(g, AIO_OfxGroup_BAL_StartTag);
  AIO_OfxGroup_SetAddDataFn(g, AIO_OfxGroup_BAL_AddData);

  return g;
}



GWENHYWFAR_CB
void AIO_OfxGroup_BAL_FreeData(void *bp, void *p) {
  AIO_OFX_GROUP_BAL *xg;

  xg=(AIO_OFX_GROUP_BAL*)p;
  assert(xg);
  free(xg->currentElement);
  GWEN_Time_free(xg->date);
  AB_Value_free(xg->value);
  GWEN_FREE_OBJECT(xg);
}



const AB_VALUE *AIO_OfxGroup_BAL_GetValue(const AIO_OFX_GROUP *g) {
  AIO_OFX_GROUP_BAL *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BAL, g);
  assert(xg);

  return xg->value;
}



void AIO_OfxGroup_BAL_SetValue(AIO_OFX_GROUP *g, const AB_VALUE *v) {
  AIO_OFX_GROUP_BAL *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BAL, g);
  assert(xg);

  AB_Value_free(xg->value);
  if (v) xg->value=AB_Value_dup(v);
  else xg->value=NULL;
}



const GWEN_TIME *AIO_OfxGroup_BAL_GetDate(const AIO_OFX_GROUP *g) {
  AIO_OFX_GROUP_BAL *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BAL, g);
  assert(xg);

  return xg->date;
}



void AIO_OfxGroup_BAL_SetDate(AIO_OFX_GROUP *g, const GWEN_TIME *ti) {
  AIO_OFX_GROUP_BAL *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BAL, g);
  assert(xg);

  GWEN_Time_free(xg->date);
  if (ti) xg->date=GWEN_Time_dup(ti);
  else xg->date=NULL;
}




int AIO_OfxGroup_BAL_StartTag(AIO_OFX_GROUP *g,
				 const char *tagName) {
  AIO_OFX_GROUP_BAL *xg;
  //GWEN_XML_CONTEXT *ctx;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BAL, g);
  assert(xg);

  //ctx=AIO_OfxGroup_GetXmlContext(g);

  if (strcasecmp(tagName, "BALAMT")==0 ||
      strcasecmp(tagName, "DTASOF")==0) {
    free(xg->currentElement);
    xg->currentElement=strdup(tagName);
  }
  else {
    DBG_WARN(AQBANKING_LOGDOMAIN,
	     "Ignoring tag [%s]", tagName);
  }

  return 0;
}



int AIO_OfxGroup_BAL_AddData(AIO_OFX_GROUP *g, const char *data) {
  AIO_OFX_GROUP_BAL *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BAL, g);
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
      if (strcasecmp(xg->currentElement, "BALAMT")==0) {
	AB_VALUE *v;

	v=AB_Value_fromString(s);
	if (v==NULL) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Invalid data for BALAMT: [%s]", s);
	  GWEN_Buffer_free(buf);
	  return GWEN_ERROR_BAD_DATA;
	}
	AB_Value_free(xg->value);
	xg->value=v;
      }
      else if (strcasecmp(xg->currentElement, "DTASOF")==0) {
	GWEN_TIME *ti;

	ti=GWEN_Time_fromString(s, "YYYYMMDD");
	if (ti==NULL) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Invalid data for DTASOF: [%s]", s);
	  GWEN_Buffer_free(buf);
	  return GWEN_ERROR_BAD_DATA;
	}
	GWEN_Time_free(xg->date);
        xg->date=ti;
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





