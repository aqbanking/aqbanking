/***************************************************************************
 begin       : Fri Apr 02 2004
 copyright   : (C) 2004-2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "swift_tag_p.h"
#include "swift_l.h"


#include <aqbanking/error.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>

#include <ctype.h>


GWEN_LIST_FUNCTIONS(AHB_SWIFT_TAG, AHB_SWIFT_Tag);
GWEN_LIST_FUNCTIONS(AHB_SWIFT_SUBTAG, AHB_SWIFT_SubTag);



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */



static const char *_findStartOfSubTag(const char *sptr);




/* Create a tag object from a tag ID and the content of the tag. Example:
   Given the following line inside a SWIFT data block:

     :28C:7/1

   You'd call AHB_SWIFT_Tag_new like this:

     AHB_SWIFT_Tag_new("28C", "7/1")

   @return a new AHB_SWIFT_TAG
 */
AHB_SWIFT_TAG *AHB_SWIFT_Tag_new(const char *id,
                                 const char *content)
{
  AHB_SWIFT_TAG *tg;

  assert(id);
  assert(content);
  GWEN_NEW_OBJECT(AHB_SWIFT_TAG, tg);
  GWEN_LIST_INIT(AHB_SWIFT_TAG, tg);
  tg->id=strdup(id);
  tg->content=strdup(content);

  return tg;
}



void AHB_SWIFT_Tag_free(AHB_SWIFT_TAG *tg)
{
  if (tg) {
    GWEN_LIST_FINI(AHB_SWIFT_TAG, tg);
    free(tg->id);
    free(tg->content);
    GWEN_FREE_OBJECT(tg);
  }
}



const char *AHB_SWIFT_Tag_GetId(const AHB_SWIFT_TAG *tg)
{
  assert(tg);
  return tg->id;
}



const char *AHB_SWIFT_Tag_GetData(const AHB_SWIFT_TAG *tg)
{
  assert(tg);
  return tg->content;
}





AHB_SWIFT_SUBTAG *AHB_SWIFT_SubTag_new(int id, const char *content, int clen)
{
  AHB_SWIFT_SUBTAG *stg;

  assert(content);
  GWEN_NEW_OBJECT(AHB_SWIFT_SUBTAG, stg);
  GWEN_LIST_INIT(AHB_SWIFT_SUBTAG, stg);
  stg->id=id;
  if (clen==-1)
    clen=strlen(content);
  stg->content=(char *)malloc(clen+1);
  memmove(stg->content, content, clen);
  stg->content[clen]=0;
  return stg;
}



void AHB_SWIFT_SubTag_free(AHB_SWIFT_SUBTAG *stg)
{
  if (stg) {
    GWEN_LIST_FINI(AHB_SWIFT_SUBTAG, stg);
    free(stg->content);
    GWEN_FREE_OBJECT(stg);
  }
}



int AHB_SWIFT_SubTag_GetId(const AHB_SWIFT_SUBTAG *stg)
{
  assert(stg);
  return stg->id;
}



const char *AHB_SWIFT_SubTag_GetData(const AHB_SWIFT_SUBTAG *stg)
{
  assert(stg);
  return stg->content;
}



AHB_SWIFT_SUBTAG *AHB_SWIFT_FindSubTagById(const AHB_SWIFT_SUBTAG_LIST *stlist, int id)
{
  AHB_SWIFT_SUBTAG *stg;

  stg=AHB_SWIFT_SubTag_List_First(stlist);
  while (stg) {
    if (stg->id==id)
      break;
    stg=AHB_SWIFT_SubTag_List_Next(stg);
  }

  return stg;
}



void AHB_SWIFT_SubTag_Condense(AHB_SWIFT_SUBTAG *stg, int keepMultipleBlanks)
{
  assert(stg);
  AHB_SWIFT_Condense(stg->content, keepMultipleBlanks);
}



const char *_findStartOfSubTag(const char *sptr)
{
  while (*sptr) {
    if (*sptr=='?') {
      const char *t;

      t=sptr;
      t++;
      if (*t==0x0a)
        t++;
      if (*t && isdigit(*t)) {
        t++;
        if (*t==0x0a)
          t++;
        if (*t && isdigit(*t)) {
          return sptr;
        }
      }
    }
    sptr++;
  }

  return NULL;
}



int AHB_SWIFT_GetNextSubTag(const char **sptr, AHB_SWIFT_SUBTAG **tptr)
{
  const char *s;
  int id=0;
  /*int nextId=0;*/
  const char *content=NULL;
  const char *startOfSubTag;
  AHB_SWIFT_SUBTAG *stg;

  s=*sptr;
  startOfSubTag=_findStartOfSubTag(s);
  if (startOfSubTag) {
    const char *t;
    const char *startOfNextSubTag;

    t=startOfSubTag;
    t++; /* skip '?' */
    if (*t==0x0a)
      t++;
    if (*t && isdigit(*t)) {
      id=(*(t++)-'0')*10;
      if (*t==0x0a)
        t++;
      if (*t && isdigit(*t)) {
        id+=*(t++)-'0';
        s=t;
      }
    }
    content=s;

    /* create subtag */
    startOfNextSubTag=_findStartOfSubTag(s);
    if (startOfNextSubTag)
      stg=AHB_SWIFT_SubTag_new(id, content, startOfNextSubTag-content);
    else
      /* rest of line */
      stg=AHB_SWIFT_SubTag_new(id, content, -1);
    /* update return pointers */
    *tptr=stg;
    *sptr=startOfNextSubTag;
    return 0;
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "No subtag found");
    return GWEN_ERROR_NO_DATA;
  }
}



int AHB_SWIFT_ParseSubTags(const char *s, AHB_SWIFT_SUBTAG_LIST *stlist, int keepMultipleBlanks)
{
  while (s && *s) {
    int rv;
    AHB_SWIFT_SUBTAG *stg=NULL;

    rv=AHB_SWIFT_GetNextSubTag(&s, &stg);
    if (rv) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    AHB_SWIFT_SubTag_Condense(stg, keepMultipleBlanks);
    /*DBG_ERROR(GWEN_LOGDOMAIN, "Adding subtag %d: [%s]", AHB_SWIFT_SubTag_GetId(stg), AHB_SWIFT_SubTag_GetData(stg));*/
    AHB_SWIFT_SubTag_List_Add(stg, stlist);
  }

  return 0;
}
