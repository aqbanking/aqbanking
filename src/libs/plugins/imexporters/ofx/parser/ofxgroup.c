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

#include "ofxgroup_p.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>



GWEN_INHERIT_FUNCTIONS(AIO_OFX_GROUP)





AIO_OFX_GROUP *AIO_OfxGroup_new(const char *groupName,
				AIO_OFX_GROUP *parent,
				GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_GROUP *g;

  GWEN_NEW_OBJECT(AIO_OFX_GROUP, g);
  GWEN_INHERIT_INIT(AIO_OFX_GROUP, g);
  g->parent=parent;
  g->xmlContext=ctx;
  if (groupName)
    g->groupName=strdup(groupName);
  if (g->xmlContext==NULL && g->parent)
    g->xmlContext=parent->xmlContext;

  return g;
}



void AIO_OfxGroup_free(AIO_OFX_GROUP *g) {
  if (g) {
    GWEN_INHERIT_FINI(AIO_OFX_GROUP, g);
    free(g->groupName);
    GWEN_FREE_OBJECT(g);
  }
}



AIO_OFX_GROUP *AIO_OfxGroup_GetParent(const AIO_OFX_GROUP *g) {
  assert(g);
  return g->parent;
}



GWEN_XML_CONTEXT *AIO_OfxGroup_GetXmlContext(const AIO_OFX_GROUP *g) {
  assert(g);
  return g->xmlContext;
}



const char *AIO_OfxGroup_GetGroupName(const AIO_OFX_GROUP *g) {
  assert(g);
  return g->groupName;
}








AIO_OFX_GROUP_STARTTAG_FN
AIO_OfxGroup_SetStartTagFn(AIO_OFX_GROUP *g,
			   AIO_OFX_GROUP_STARTTAG_FN f) {
  AIO_OFX_GROUP_STARTTAG_FN oldFn;

  assert(g);
  oldFn=g->startTagFn;
  g->startTagFn=f;
  return oldFn;
}



AIO_OFX_GROUP_ENDTAG_FN
AIO_OfxGroup_SetEndTagFn(AIO_OFX_GROUP *g,
			 AIO_OFX_GROUP_ENDTAG_FN f) {
  AIO_OFX_GROUP_ENDTAG_FN oldFn;

  assert(g);
  oldFn=g->endTagFn;
  g->endTagFn=f;
  return oldFn;
}



AIO_OFX_GROUP_ADDDATA_FN
AIO_OfxGroup_SetAddDataFn(AIO_OFX_GROUP *g,
			  AIO_OFX_GROUP_ADDDATA_FN f) {
  AIO_OFX_GROUP_ADDDATA_FN oldFn;

  assert(g);
  oldFn=g->addDataFn;
  g->addDataFn=f;
  return oldFn;
}



AIO_OFX_GROUP_ENDSUBGROUP_FN
AIO_OfxGroup_SetEndSubGroupFn(AIO_OFX_GROUP *g,
			      AIO_OFX_GROUP_ENDSUBGROUP_FN f) {
  AIO_OFX_GROUP_ENDSUBGROUP_FN oldFn;

  assert(g);
  oldFn=g->endSubGroupFn;
  g->endSubGroupFn=f;
  return oldFn;
}





int AIO_OfxGroup_StartTag(AIO_OFX_GROUP *g,
			  const char *tagName) {
  assert(g);
  if (g->startTagFn)
    return g->startTagFn(g, tagName);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int AIO_OfxGroup_EndTag(AIO_OFX_GROUP *g,
			const char *tagName) {
  assert(g);
  if (g->endTagFn)
    return g->endTagFn(g, tagName);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int AIO_OfxGroup_AddData(AIO_OFX_GROUP *g,
			 const char *data) {
  assert(g);
  if (g->addDataFn)
    return g->addDataFn(g, data);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int AIO_OfxGroup_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg) {
  assert(g);
  if (g->endSubGroupFn)
    return g->endSubGroupFn(g, sg);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}





