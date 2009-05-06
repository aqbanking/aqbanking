/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Fri Apr 17 2009
 copyright   : (C) 2009 by Stephen R. Besch (C) 2008 by Martin Preuss
 email       : sbesch@buffalo.edu martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "g_invbuy_p.h"
#include "ofxxmlctx_l.h"
#include "i18n_l.h"

#include "g_generic_l.h"
#include "g_ignore_l.h"
#include "g_invtran_l.h"
#include "g_secid_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/gui.h>

/*There are 2 group (<INVTRAN> & <SECID>) and several data (<UNITS>, <UNITPRICE>, <COMMISSION>,
 <TOTAL>, <SUBACCTSEC> & <SUBACCTFUND>) tags which we need to handle. The SECID group already
 appears and has 2 data items that are referenced with 2 object methods. The INVTRAN group is
 new and returns 3 data items indexed from an array.*/

char * INVBUY_DataTags[iinvbuylastget+1]={"COMMISSION", "TOTAL", "UNITS", "UNITPRICE", "SUBACCTSEC", "SUBACCTFUND",
                                          "UNIQUEID", "UNIQUEIDTYPE",
                                          "FITID", "DTTRADE", "MEMO", "UNKNOWN"};

char * INVBUY_GroupTags[iinvbuylastgroup]={"SECID","INVTRAN"};

GWEN_INHERIT(AIO_OFX_GROUP, AIO_OFX_GROUP_INVBUY)

AIO_OFX_GROUP *AIO_OfxGroup_INVBUY_new(const char *groupName, AIO_OFX_GROUP *parent, GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_GROUP *g;
  AIO_OFX_GROUP_INVBUY *xg;

  /* create base group */
  g=AIO_OfxGroup_Generic_new(groupName, parent, ctx);
  assert(g);

  GWEN_NEW_OBJECT(AIO_OFX_GROUP_INVBUY, xg);
  assert(xg);
  GWEN_INHERIT_SETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVBUY, g, xg,
                       AIO_OfxGroup_INVBUY_FreeData);

  /* set virtual functions */
  AIO_OfxGroup_SetStartTagFn(g, AIO_OfxGroup_INVBUY_StartTag);
  AIO_OfxGroup_SetAddDataFn(g, AIO_OfxGroup_INVBUY_AddData);
  AIO_OfxGroup_SetEndSubGroupFn(g, AIO_OfxGroup_INVBUY_EndSubGroup);
  return g;
}

GWENHYWFAR_CB
void AIO_OfxGroup_INVBUY_FreeData(void *bp, void *p) {
  AIO_OFX_GROUP_INVBUY *xg;

/*First fetch the group's data object and verify that it is still valid before freeing up its
 data items.*/

  xg=(AIO_OFX_GROUP_INVBUY*)p;
  assert(xg);
  int i;
  for (i=0; i<iinvbuylastget; i++) free(xg->datum[i]);
  GWEN_FREE_OBJECT(xg);
};

/*Here to get the data*/

char *AIO_OfxGroup_INVBUY_GetDatum(const AIO_OFX_GROUP *g, int index) {
  if (index < 0 || index > iinvbuylastget) return NULL;
  AIO_OFX_GROUP_INVBUY *xg;
  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVBUY, g);
  assert(xg);
  return xg->datum[index];
}

char *AIO_OfxGroup_INVBUY_DatumName(int index){
  if (index < 0 || index > iinvbuylastget) return INVBUY_DataTags[iinvbuylastget];
  return INVBUY_DataTags[index];
}

int AIO_OfxGroup_INVBUY_StartTag(AIO_OFX_GROUP *g, const char *tagName) {
  AIO_OFX_GROUP_INVBUY *xg;
  GWEN_XML_CONTEXT *ctx;
  AIO_OFX_GROUP *gNew=NULL;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVBUY, g);
  assert(xg);
  ctx=AIO_OfxGroup_GetXmlContext(g);

  free(xg->currentElement);
  xg->currentElement=NULL;
  int ce;
  ce=AIO_OfxGroup_INVBUY_SortTag(tagName, INVBUY_DataTags, iinvbuylastdatum);
  if (ce<0){
    ce=AIO_OfxGroup_INVBUY_SortTag(tagName, INVBUY_GroupTags, iinvbuylastgroup);
    if (ce==isecid)gNew=AIO_OfxGroup_SECID_new(tagName, g, ctx);
    else if (ce==iinvtran)gNew=AIO_OfxGroup_INVTRAN_new(tagName, g, ctx);
    else {
      DBG_WARN(AQBANKING_LOGDOMAIN, "Ignoring group [%s]", tagName);
      gNew=AIO_OfxGroup_Ignore_new(tagName, g, ctx);
    }
    if (gNew) {
      AIO_OfxXmlCtx_SetCurrentGroup(ctx, gNew);
      GWEN_XmlCtx_IncDepth(ctx);
    }
  }
  else xg->currentElement=strdup(tagName);
  return 0;
}

/*Arrive here with each new data item. Have a look at the data's type and then take appropriate action*/

int AIO_OfxGroup_INVBUY_AddData(AIO_OFX_GROUP *g, const char *data) {
  AIO_OFX_GROUP_INVBUY *xg;
  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVBUY, g);
  assert(xg);

/*Get the current element so we can examine it. Only proceed if there is in fact a current element*/

  if (!xg->currentElement) return 0;
    GWEN_BUFFER *buf;
    int rv;
    const char *s;

    buf=GWEN_Buffer_new(0, strlen(data), 0, 1);
    rv=AIO_OfxXmlCtx_SanitizeData(AIO_OfxGroup_GetXmlContext(g), data, buf);

/*Error message if sanitize fails*/

    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(buf);
      return rv;
    }

/* COMMISSION, TOTAL, UNITS, UNITPRICE, SUBACCTSEC, SUBACCTFUND"*/

    int ce;
    ce=AIO_OfxGroup_INVBUY_SortTag(xg->currentElement, INVBUY_DataTags, iinvbuylastdatum);
    if (ce<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Ignoring data for unknown element [%s]", xg->currentElement);
      return 0;
    }

/*Get pointer to start of buffer and dereference it into a string.*/

    s=GWEN_Buffer_GetStart(buf);
    if (*s) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "AddData: %s=[%s]", xg->currentElement, s);
      free(xg->datum[ce]);        /*Free up any old stuff*/
      xg->datum[ce]=strdup(s);    /*Copy string datum*/
    }
    GWEN_Buffer_free(buf);
  return 0;
}

/*Now we have to get stuff from sub-groups into the local data structure. We need to do this before
 the sub-group object is destroyed (as soon as we return?)*/

int AIO_OfxGroup_INVBUY_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg) {
  AIO_OFX_GROUP_INVBUY *xg;
  const char *s;
  GWEN_XML_CONTEXT *ctx;
  int cg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVBUY, g);
  assert(xg);
  ctx=AIO_OfxGroup_GetXmlContext(g);
  assert(ctx);

/*First, find out which group is ending here. There are only 2 possibilities as of now.*/

  s=AIO_OfxGroup_GetGroupName(sg);
  cg=AIO_OfxGroup_INVBUY_SortTag(s, INVBUY_GroupTags, iinvbuylastgroup);
  if (cg==isecid){
    free(xg->datum[iuniqueid]);
    free(xg->datum[iuniqueidtype]);
    xg->datum[iuniqueid]=strdup(AIO_OfxGroup_SECID_GetUniqueId(sg));
    xg->datum[iuniqueidtype]=strdup(AIO_OfxGroup_SECID_GetNameSpace(sg));
  }
  else if (cg==iinvtran){
    free(xg->datum[ifitid]);
    free(xg->datum[idttrade]);
    free(xg->datum[imemo]);
    xg->datum[ifitid]=strdup(AIO_OfxGroup_INVTRAN_GetDatum(sg, itranfitid));
    xg->datum[idttrade]=strdup(AIO_OfxGroup_INVTRAN_GetDatum(sg, itrandttrade));
    xg->datum[imemo]=strdup(AIO_OfxGroup_INVTRAN_GetDatum(sg, itranmemo));
  }
  return 0;
}

/*Routine that converts string tag into index number. Return -1 if no tag found.*/

int AIO_OfxGroup_INVBUY_SortTag(const char * s, char ** sTags, int max){
  int i;
  for (i=0; i<max; i++)
    if (strcasecmp(s, sTags[i])==0) return i;
  return -1;
};

