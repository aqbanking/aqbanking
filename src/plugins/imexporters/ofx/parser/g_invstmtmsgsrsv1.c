/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 comments    :Stephen R. Besch
 email       :sbesch@acsu.buffalo.edu

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "g_invstmtmsgsrsv1_p.h"
#include "ofxxmlctx_l.h"

#include "g_generic_l.h"
#include "g_ignore_l.h"

#include "g_invstmttrnrs_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>



/*This code parallels the code in BANKMSGSRSV1. First order of business is the object constructor.
 After setting up the base group object, all we need to do is watch for the arrival of the INVSTMTTRNRS
 tag, so we set up a virtual function to intercept future tags.*/

AIO_OFX_GROUP *AIO_OfxGroup_INVSTMTMSGSRSV1_new(const char *groupName,
						AIO_OFX_GROUP *parent,
						GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_GROUP *g;

  /* create base group */
  g=AIO_OfxGroup_Generic_new(groupName, parent, ctx);
  assert(g);

  /* set virtual functions */
  AIO_OfxGroup_SetStartTagFn(g, AIO_OfxGroup_INVSTMTMSGSRSV1_StartTag);

  return g;
}



/*The goal here is to filter all the tags in this group until we find a the INVSTMTTRNRS tag. At that
 point we create a new group (the INVSTMTTRNRS group) and let it handle all the tags for that group.*/
int AIO_OfxGroup_INVSTMTMSGSRSV1_StartTag(AIO_OFX_GROUP *g,
					  const char *tagName) {
  AIO_OFX_GROUP *gNew=NULL;
  GWEN_XML_CONTEXT *ctx;

  assert(g);                                                /*Make sure that the parent group exists*/

  ctx=AIO_OfxGroup_GetXmlContext(g);                        /*If it does, then get the context from it*/

  if (strcasecmp(tagName, "INVSTMTTRNRS")==0) {
    gNew=AIO_OfxGroup_INVSTMTTRNRS_new(tagName, g, ctx);    /*We've found the tag, so create a new group*/
  }
  else {
    DBG_WARN(AQBANKING_LOGDOMAIN,
	     "Ignoring group [%s]", tagName);
    gNew=AIO_OfxGroup_Ignore_new(tagName, g, ctx);          /*All other groups are ignored!*/
  }

  /*OK, so we have a new group - even if it's going to be just the ignore group. So we set that
   new group into the context and bump the depth counter.*/
  if (gNew) {
    AIO_OfxXmlCtx_SetCurrentGroup(ctx, gNew);
    GWEN_XmlCtx_IncDepth(ctx);
  }

  return 0;
}


