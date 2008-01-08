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

#include "g_generic_p.h"
#include "ofxxmlctx_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>




AIO_OFX_GROUP *AIO_OfxGroup_Generic_new(const char *groupName,
					AIO_OFX_GROUP *parent,
					GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_GROUP *g;

  /* create base group */
  g=AIO_OfxGroup_new(groupName, parent, ctx);
  assert(g);

  /* set virtual functions */
  AIO_OfxGroup_SetEndTagFn(g, AIO_OfxGroup_Generic_EndTag);
  AIO_OfxGroup_SetAddDataFn(g, AIO_OfxGroup_Generic_AddData);
  AIO_OfxGroup_SetEndSubGroupFn(g, AIO_OfxGroup_Generic_EndSubGroup);

  return g;
}



int AIO_OfxGroup_Generic_EndTag(AIO_OFX_GROUP *g, const char *tagName) {
  assert(g);

  if (strcasecmp(AIO_OfxGroup_GetGroupName(g), tagName)!=0) {
    DBG_INFO(AQBANKING_LOGDOMAIN,
	     "Tag [%s] does not close [%s]",
	     tagName, AIO_OfxGroup_GetGroupName(g));
    return GWEN_ERROR_BAD_DATA;
  }

  /* always end this tag */
  return 1;
}



int AIO_OfxGroup_Generic_AddData(AIO_OFX_GROUP *g, const char *data) {
  assert(g);

  /* just ignore the data */
  return 0;
}



int AIO_OfxGroup_Generic_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg){
  assert(g);

  /* just ignore the end of sub group */
  return 0;
}



