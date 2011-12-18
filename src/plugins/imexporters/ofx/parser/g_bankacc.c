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

#include "g_bankacc_p.h"
#include "ofxxmlctx_l.h"
#include "i18n_l.h"

#include "g_generic_l.h"
#include "g_ignore_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/gui.h>



GWEN_INHERIT(AIO_OFX_GROUP, AIO_OFX_GROUP_BANKACC)




AIO_OFX_GROUP *AIO_OfxGroup_BANKACC_new(const char *groupName,
					AIO_OFX_GROUP *parent,
					GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_GROUP *g;
  AIO_OFX_GROUP_BANKACC *xg;

  /* create base group */
  g=AIO_OfxGroup_Generic_new(groupName, parent, ctx);
  assert(g);

  GWEN_NEW_OBJECT(AIO_OFX_GROUP_BANKACC, xg);
  assert(xg);
  GWEN_INHERIT_SETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BANKACC, g, xg,
                       AIO_OfxGroup_BANKACC_FreeData);

  /* set virtual functions */
  AIO_OfxGroup_SetStartTagFn(g, AIO_OfxGroup_BANKACC_StartTag);
  AIO_OfxGroup_SetAddDataFn(g, AIO_OfxGroup_BANKACC_AddData);

  if (strcasecmp(groupName, "CCACCTFROM")==0 ||
      strcasecmp(groupName, "CCACCTTO")==0)
    xg->accType=strdup("CREDITCARD");
  else if (strcasecmp(groupName, "INVACCTFROM")==0 ||
           strcasecmp(groupName, "INVACCTTO")==0)
    xg->accType=strdup("MONEYMRKT");

  return g;
}



GWENHYWFAR_CB
void AIO_OfxGroup_BANKACC_FreeData(void *bp, void *p) {
  AIO_OFX_GROUP_BANKACC *xg;

  xg=(AIO_OFX_GROUP_BANKACC*)p;
  assert(xg);
  free(xg->currentElement);
  free(xg->bankId);
  free(xg->accId);
  free(xg->accType);
  GWEN_FREE_OBJECT(xg);
}



const char *AIO_OfxGroup_BANKACC_GetBankId(const AIO_OFX_GROUP *g) {
  AIO_OFX_GROUP_BANKACC *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BANKACC, g);
  assert(xg);

  return xg->bankId;
}



void AIO_OfxGroup_BANKACC_SetBankId(AIO_OFX_GROUP *g, const char *s) {
  AIO_OFX_GROUP_BANKACC *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BANKACC, g);
  assert(xg);

  free(xg->bankId);
  if (s) xg->bankId=strdup(s);
  else xg->bankId=NULL;
}



const char *AIO_OfxGroup_BANKACC_GetAccId(const AIO_OFX_GROUP *g) {
  AIO_OFX_GROUP_BANKACC *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BANKACC, g);
  assert(xg);

  return xg->accId;
}



void AIO_OfxGroup_BANKACC_SetAccId(AIO_OFX_GROUP *g, const char *s) {
  AIO_OFX_GROUP_BANKACC *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BANKACC, g);
  assert(xg);

  free(xg->accId);
  if (s) xg->accId=strdup(s);
  else xg->accId=NULL;
}



const char *AIO_OfxGroup_BANKACC_GetAccType(const AIO_OFX_GROUP *g) {
  AIO_OFX_GROUP_BANKACC *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BANKACC, g);
  assert(xg);

  return xg->accType;
}



void AIO_OfxGroup_BANKACC_SetAccType(AIO_OFX_GROUP *g, const char *s) {
  AIO_OFX_GROUP_BANKACC *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BANKACC, g);
  assert(xg);

  free(xg->accType);
  if (s) xg->accType=strdup(s);
  else xg->accType=NULL;
}



int AIO_OfxGroup_BANKACC_StartTag(AIO_OFX_GROUP *g,
				 const char *tagName) {
  AIO_OFX_GROUP_BANKACC *xg;
  //GWEN_XML_CONTEXT *ctx;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BANKACC, g);
  assert(xg);

  //ctx=AIO_OfxGroup_GetXmlContext(g);

  free(xg->currentElement);
  xg->currentElement=NULL;

  if (strcasecmp(tagName, "BANKID")==0 ||
      strcasecmp(tagName, "ACCTID")==0 ||
      strcasecmp(tagName, "ACCTTYPE")==0 ||
      strcasecmp(tagName, "BRANCHID")==0 ||
      strcasecmp(tagName, "ACCTKEY")==0 ||
      strcasecmp(tagName, "BROKERID")==0) {
    xg->currentElement=strdup(tagName);
  }
  else {
    DBG_WARN(AQBANKING_LOGDOMAIN,
	     "Ignoring tag [%s]", tagName);
  }

  return 0;
}



int AIO_OfxGroup_BANKACC_AddData(AIO_OFX_GROUP *g, const char *data) {
  AIO_OFX_GROUP_BANKACC *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_BANKACC, g);
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
      if (strcasecmp(xg->currentElement, "BANKID")==0)
	AIO_OfxGroup_BANKACC_SetBankId(g, GWEN_Buffer_GetStart(buf));
      else if (strcasecmp(xg->currentElement, "BROKERID")==0)
	AIO_OfxGroup_BANKACC_SetBankId(g, GWEN_Buffer_GetStart(buf));
      else if (strcasecmp(xg->currentElement, "ACCTID")==0)
	AIO_OfxGroup_BANKACC_SetAccId(g, GWEN_Buffer_GetStart(buf));
      else if (strcasecmp(xg->currentElement, "ACCTTYPE")==0)
	AIO_OfxGroup_BANKACC_SetAccType(g, GWEN_Buffer_GetStart(buf));
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





