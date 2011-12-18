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

#include "g_invtran_p.h"
#include "ofxxmlctx_l.h"
#include "i18n_l.h"

#include "g_generic_l.h"
#include "g_ignore_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/gui.h>

const char * INVTRAN_DataTags[iinvtranlastdatum]={"FITID", "DTTRADE", "MEMO"};

GWEN_INHERIT(AIO_OFX_GROUP, AIO_OFX_GROUP_INVTRAN)



AIO_OFX_GROUP *AIO_OfxGroup_INVTRAN_new(const char *groupName, AIO_OFX_GROUP *parent, GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_GROUP *g;
  AIO_OFX_GROUP_INVTRAN *xg;

  /* create base group */
  g=AIO_OfxGroup_Generic_new(groupName, parent, ctx);
  assert(g);
  GWEN_NEW_OBJECT(AIO_OFX_GROUP_INVTRAN, xg);
  assert(xg);
  GWEN_INHERIT_SETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVTRAN, g, xg, AIO_OfxGroup_INVTRAN_FreeData);

  /* set virtual functions */
  AIO_OfxGroup_SetStartTagFn(g, AIO_OfxGroup_INVTRAN_StartTag);
  AIO_OfxGroup_SetAddDataFn(g, AIO_OfxGroup_INVTRAN_AddData);
  return g;
}



GWENHYWFAR_CB
void AIO_OfxGroup_INVTRAN_FreeData(void *bp, void *p) {
  AIO_OFX_GROUP_INVTRAN *xg;
  int i;

  /*Fetch the group's data object and verify that it is still valid before freeing its data items.*/
  xg=(AIO_OFX_GROUP_INVTRAN*)p;
  assert(xg);

  free(xg->currentElement);
  for (i=0; i<iinvtranlastdatum; i++)
    free(xg->datum[i]);
  GWEN_FREE_OBJECT(xg);
}



/*Here to get a datum by index. Just return a reference. The caller must duplicate the string if he
 wants a permanent copy. We will free the string when the object is destroyed.*/

char *AIO_OfxGroup_INVTRAN_GetDatum(const AIO_OFX_GROUP *g, int index) {
  AIO_OFX_GROUP_INVTRAN *xg;

  if (index < 0 || index > iinvtranlastdatum)
    return NULL;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVTRAN, g);
  assert(xg);
  return xg->datum[index];
}



/*Here to detect the start tags of the 3 data tags in this group.*/
int AIO_OfxGroup_INVTRAN_StartTag(AIO_OFX_GROUP *g, const char *tagName) {
  AIO_OFX_GROUP_INVTRAN *xg;
  //GWEN_XML_CONTEXT *ctx;
  int it;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVTRAN, g);
  assert(xg);

  //ctx=AIO_OfxGroup_GetXmlContext(g);

  free(xg->currentElement);
  xg->currentElement=NULL;

  it=AIO_OfxGroup_INVTRAN_SortTag(tagName, INVTRAN_DataTags, iinvtranlastdatum);

  /*When we find a data tag, simply set this as the current element so that the AddData routine will know
   what to do with the resulting data.*/
  if (it<0) {
    DBG_WARN(AQBANKING_LOGDOMAIN, "Ignoring tag [%s]", tagName);
  }
  else
    xg->currentElement=strdup(tagName);
  return 0;
}



/*Here when a we have some data to process. Simply examine the currentElement to see
 what the data represents. Then stuff the resulting string into the sub-data structure.*/
int AIO_OfxGroup_INVTRAN_AddData(AIO_OFX_GROUP *g, const char *data) {
  AIO_OFX_GROUP_INVTRAN *xg;
  GWEN_BUFFER *buf;
  int rv;
  const char *s;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVTRAN, g);
  assert(xg);

  if (!xg->currentElement)
    return 0;

  buf=GWEN_Buffer_new(0, strlen(data), 0, 1);
  rv=AIO_OfxXmlCtx_SanitizeData(AIO_OfxGroup_GetXmlContext(g), data, buf);
  if (rv<0){
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(buf);
    return rv;
  }

  s=GWEN_Buffer_GetStart(buf);
  if (*s) {
    int ce;

    DBG_INFO(AQBANKING_LOGDOMAIN, "AddData: %s=[%s]", xg->currentElement, s);
    ce=AIO_OfxGroup_INVTRAN_SortTag(xg->currentElement, INVTRAN_DataTags, iinvtranlastdatum);
    if (ce>=0) {
      free(xg->datum[ce]);
      xg->datum[ce]=strdup(s);
    }
    else {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Ignoring data for unknown element [%s]", xg->currentElement);
    }
  }
  GWEN_Buffer_free(buf);
  return 0;
}



/*Routine that converts string tag into index number. Return -1 if no tag found.*/
int AIO_OfxGroup_INVTRAN_SortTag(const char * s, const char ** sTags, int max){
  int i;

  for (i=0; i<max; i++)
    if (strcasecmp(s, sTags[i])==0)
      return i;
  return -1;
}



