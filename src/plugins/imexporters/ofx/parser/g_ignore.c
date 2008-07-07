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

#include "g_ignore_p.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>



GWEN_INHERIT(AIO_OFX_GROUP, AIO_OFX_GROUP_IGNORE)




AIO_OFX_GROUP *AIO_OfxGroup_Ignore_new(const char *groupName,
				       AIO_OFX_GROUP *parent,
				       GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_GROUP *g;
  AIO_OFX_GROUP_IGNORE *xg;

  /* create base group */
  g=AIO_OfxGroup_new(groupName, parent, ctx);
  assert(g);

  /* create extension, assign to base group */
  GWEN_NEW_OBJECT(AIO_OFX_GROUP_IGNORE, xg);
  assert(xg);
  GWEN_INHERIT_SETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_IGNORE, g, xg,
                       AIO_OfxGroup_Ignore_FreeData);
  xg->openTags=GWEN_StringList_new();

  /* set virtual functions */
  AIO_OfxGroup_SetStartTagFn(g, AIO_OfxGroup_Ignore_StartTag);
  AIO_OfxGroup_SetEndTagFn(g, AIO_OfxGroup_Ignore_EndTag);
  AIO_OfxGroup_SetAddDataFn(g, AIO_OfxGroup_Ignore_AddData);

  return g;
}



GWENHYWFAR_CB
void AIO_OfxGroup_Ignore_FreeData(void *bp, void *p) {
  AIO_OFX_GROUP_IGNORE *xg;

  xg=(AIO_OFX_GROUP_IGNORE*)p;
  GWEN_StringList_free(xg->openTags);
  GWEN_FREE_OBJECT(xg);
}



int AIO_OfxGroup_Ignore_StartTag(AIO_OFX_GROUP *g, const char *tagName) {
  AIO_OFX_GROUP_IGNORE *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_IGNORE, g);
  assert(xg);

  /* just insert the new tag into list */
  GWEN_StringList_InsertString(xg->openTags, tagName, 0, 0);

  return 0;
}



int AIO_OfxGroup_Ignore_EndTag(AIO_OFX_GROUP *g, const char *tagName) {
  AIO_OFX_GROUP_IGNORE *xg;

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_IGNORE, g);
  assert(xg);

  if (strcasecmp(AIO_OfxGroup_GetGroupName(g), tagName)==0)
    /* ending this tag */
    return 1;

  /* should be a subtag */
  for (;;) {
    const char *s;

    s=GWEN_StringList_FirstString(xg->openTags);
    if (!s)
      /* empty and it is not our name, so let tha caller try parents */
      break;

    /* remove 1st open tag in any case */
    GWEN_StringList_RemoveString(xg->openTags, s);
    if (strcasecmp(s, tagName)==0)
      /* it was the one we wanted, stop here */
      break;
    /* otherwise this loop continues to remove all subtags until the
     * matching one is found */
  }

  return 0;
}



int AIO_OfxGroup_Ignore_AddData(AIO_OFX_GROUP *g, const char *data) {
  /* just ignore the data */
  return 0;
}



