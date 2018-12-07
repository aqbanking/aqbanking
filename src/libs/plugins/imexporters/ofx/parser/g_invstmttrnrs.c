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

#include "g_invstmttrnrs_p.h"
#include "ofxxmlctx_l.h"

#include "g_generic_l.h"
#include "g_ignore_l.h"

#include "g_status_l.h"
#include "g_invstmtrs_l.h"

#include "i18n_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>



/*This code parallels the code in g_stmttrnrs. Since there are no data items that we are interested in,
 we only need to deal with sub-group creation. Hence there is only a virtual function for filtering
 start tags.*/

AIO_OFX_GROUP *AIO_OfxGroup_INVSTMTTRNRS_new(const char *groupName,
					     AIO_OFX_GROUP *parent,
					     GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_GROUP *g;

  /* create base group */
  g=AIO_OfxGroup_Generic_new(groupName, parent, ctx);
  assert(g);

  /* set virtual functions */
  AIO_OfxGroup_SetStartTagFn(g, AIO_OfxGroup_INVSTMTTRNRS_StartTag);

  return g;
}



/*There are 4 data items and subgroups here. We are only interested in the STATUS and INVSTMTRS
 groups. The TRNUID and CLTCOOKIE datums are ignored.*/

int AIO_OfxGroup_INVSTMTTRNRS_StartTag(AIO_OFX_GROUP *g,
				       const char *tagName) {
  AIO_OFX_GROUP *gNew=NULL;
  GWEN_XML_CONTEXT *ctx;

  assert(g);
  ctx=AIO_OfxGroup_GetXmlContext(g);

  /*If this is a STATUS subgroup, define it*/
  if (strcasecmp(tagName, "STATUS")==0) {
    gNew=AIO_OfxGroup_STATUS_new(tagName, g, ctx,
				 I18N("Status for investment transaction statement request"));
  }
  /*Or, if it's the TRNUID or CLTCOOKIE data, just ignore them. These are really easy since
   no subgroup Ignore trap is needed.*/
  else if (strcasecmp(tagName, "TRNUID")==0 ||
	   strcasecmp(tagName, "CLTCOOKIE")==0) {
    /* some tags, just ignore them here */
  }
  /*If this is the Investment Statement Request, define it's subgroup*/
  else if (strcasecmp(tagName, "INVSTMTRS")==0) {
    gNew=AIO_OfxGroup_INVSTMTRS_new(tagName, g, ctx);
  }
/*All other sub-groups pass on to the ignore trap.*/
  else {
    DBG_WARN(AQBANKING_LOGDOMAIN,
	     "Ignoring group [%s]", tagName);
    gNew=AIO_OfxGroup_Ignore_new(tagName, g, ctx);
  }

/*If we really made up a new group, put it in to the context. Do nothing if this was
 a data tag.*/

  if (gNew) {
    AIO_OfxXmlCtx_SetCurrentGroup(ctx, gNew);
    GWEN_XmlCtx_IncDepth(ctx);
  }

  return 0;
}



