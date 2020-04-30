/***************************************************************************
 begin       : Fri Jun 28 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "parser_normalize.h"
#include "parser.h"

#include <gwenhywfar/debug.h>


/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


static AQFINTS_ELEMENT *findGroupInTree(AQFINTS_ELEMENT *groupTree, const char *id, int version);

static void normalizeSequence(AQFINTS_ELEMENT *elementTree);
static void normalizeSegment(AQFINTS_SEGMENT *segment);
static void resolveGroups(AQFINTS_ELEMENT *elementTree, AQFINTS_ELEMENT *groupTree);
static void segmentResolveGroups(AQFINTS_SEGMENT *segment, AQFINTS_ELEMENT *groupTree);

static void removeTrailingEmptyDegChildren(AQFINTS_ELEMENT *elementTree);
static void removeTrailingEmptyDeChildren(AQFINTS_ELEMENT *elementTree);




/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



void AQFINTS_Parser_SegmentList_ResolveGroups(AQFINTS_SEGMENT_LIST *segmentList, AQFINTS_ELEMENT *groupTree)
{
  AQFINTS_SEGMENT *segment;

  segment=AQFINTS_Segment_List_First(segmentList);
  while (segment) {
    segmentResolveGroups(segment, groupTree);
    segment=AQFINTS_Segment_List_Next(segment);
  }
}



void AQFINTS_Parser_SegmentList_Normalize(AQFINTS_SEGMENT_LIST *segmentList)
{
  AQFINTS_SEGMENT *segment;

  segment=AQFINTS_Segment_List_First(segmentList);
  while (segment) {
    normalizeSegment(segment);
    segment=AQFINTS_Segment_List_Next(segment);
  }
}



void AQFINTS_Parser_Segment_RemoveTrailingEmptyElements(AQFINTS_SEGMENT *segment)
{
  AQFINTS_ELEMENT *elementTree;

  elementTree=AQFINTS_Segment_GetElements(segment);
  if (elementTree) {
    AQFINTS_ELEMENT *element;

    /* shorten content of DEG's */
    element=AQFINTS_Element_Tree2_GetFirstChild(elementTree);
    while (element) {
      removeTrailingEmptyDeChildren(element);
      element=AQFINTS_Element_Tree2_GetNext(element);
    }
    /* shorten the list of DEG */
    removeTrailingEmptyDegChildren(elementTree);
  }
}



AQFINTS_ELEMENT *findGroupInTree(AQFINTS_ELEMENT *groupTree, const char *id, int version)
{
  AQFINTS_ELEMENT *group;

  group=AQFINTS_Element_Tree2_GetFirstChild(groupTree);
  while (group) {
    if (version==0 || version==AQFINTS_Element_GetVersion(group)) {
      if (!(id && *id))
        return group;
      else {
        const char *s;

        s=AQFINTS_Element_GetId(group);
        if (s && *s && strcasecmp(s, id)==0)
          return group;
      }
    }
    group=AQFINTS_Element_Tree2_GetNext(group);
  }

  return NULL;
}




void normalizeSegment(AQFINTS_SEGMENT *segment)
{
  AQFINTS_ELEMENT *elementTree;

  elementTree=AQFINTS_Segment_GetElements(segment);
  if (elementTree) {
    normalizeSequence(elementTree);
  }
}



void normalizeSequence(AQFINTS_ELEMENT *elementTree)
{
  AQFINTS_ELEMENT *element;

  /* check top-level elements: if DE, prepend a DEG before it */
  element=AQFINTS_Element_Tree2_GetFirstChild(elementTree);
  while (element) {
    AQFINTS_ELEMENT *nextElement;

    nextElement=AQFINTS_Element_Tree2_GetNext(element);

    if (AQFINTS_Element_GetElementType(element)==AQFINTS_ElementType_Group) {
      normalizeSequence(element);
    }
    else if (AQFINTS_Element_GetElementType(element)==AQFINTS_ElementType_De) {
      AQFINTS_ELEMENT *elementDeg;

      elementDeg=AQFINTS_Element_new();
      AQFINTS_Element_SetElementType(elementDeg, AQFINTS_ElementType_Deg);
      AQFINTS_Element_SetVersion(elementDeg, AQFINTS_Element_GetVersion(element));
      AQFINTS_Element_SetMinNum(elementDeg, AQFINTS_Element_GetMinNum(element));
      AQFINTS_Element_SetMaxNum(elementDeg, AQFINTS_Element_GetMaxNum(element));
      AQFINTS_Element_Tree2_Replace(element, elementDeg);
      AQFINTS_Element_Tree2_AddChild(elementDeg, element);
    }

    element=nextElement;
  }
}



void segmentResolveGroups(AQFINTS_SEGMENT *segment, AQFINTS_ELEMENT *groupTree)
{
  resolveGroups(AQFINTS_Segment_GetElements(segment), groupTree);
}



void resolveGroups(AQFINTS_ELEMENT *elementTree, AQFINTS_ELEMENT *groupTree)
{
  AQFINTS_ELEMENT *element;

  element=AQFINTS_Element_Tree2_GetFirstChild(elementTree);
  while (element) {
    const char *sGroupType;
    AQFINTS_ELEMENT_TYPE eType;

    eType=AQFINTS_Element_GetElementType(element);
    sGroupType=AQFINTS_Element_GetType(element);
    if ((eType==AQFINTS_ElementType_Group) ||
        (eType==AQFINTS_ElementType_Deg && sGroupType && *sGroupType)) {
      if (sGroupType && *sGroupType) {
        AQFINTS_ELEMENT *groupDefElement;
        int iGroupVersion;

        iGroupVersion=AQFINTS_Element_GetVersion(element);
        DBG_ERROR(0, "Looking for group \"%s:%d\"", sGroupType, iGroupVersion);
        groupDefElement=findGroupInTree(groupTree, sGroupType, iGroupVersion);
        if (groupDefElement==NULL) {
          DBG_ERROR(0, "Group \"%s:%d\" not found", sGroupType, iGroupVersion);
          assert(0);
        }
        else {
          AQFINTS_ELEMENT *groupElement;

          DBG_ERROR(0, "Found group \"%s:%d\"", sGroupType, AQFINTS_Element_GetVersion(groupDefElement));
          groupElement=AQFINTS_Element_Tree2_GetFirstChild(groupDefElement);
          while (groupElement) {
            AQFINTS_ELEMENT *elementCopy;

            elementCopy=AQFINTS_Element_dup(groupElement);
            AQFINTS_Element_Tree2_AddChild(element, elementCopy);

            groupElement=AQFINTS_Element_Tree2_GetNext(groupElement);
          }
        }

      }
    }

    /* recursion */
    resolveGroups(element, groupTree);

    element=AQFINTS_Element_Tree2_GetNext(element);
  }
}




void removeTrailingEmptyDegChildren(AQFINTS_ELEMENT *elementTree)
{
  AQFINTS_ELEMENT *element;

  while ((element=AQFINTS_Element_Tree2_GetLastChild(elementTree))) {
    if (AQFINTS_Element_GetElementType(element)==AQFINTS_ElementType_Deg) {
      if (AQFINTS_Element_Tree2_GetFirstChild(element)==NULL) {
        AQFINTS_Element_Tree2_Unlink(element);
        AQFINTS_Element_free(element);
      }
      else
        break;
    }
    else
      break;
  }
}



void removeTrailingEmptyDeChildren(AQFINTS_ELEMENT *elementTree)
{
  AQFINTS_ELEMENT *element;

  while ((element=AQFINTS_Element_Tree2_GetLastChild(elementTree))) {
    if (AQFINTS_Element_GetElementType(element)==AQFINTS_ElementType_De) {
      if (AQFINTS_Element_GetDataLength(element)==0 ||
          AQFINTS_Element_GetDataPointer(element)==NULL) {
        AQFINTS_Element_Tree2_Unlink(element);
        AQFINTS_Element_free(element);
      }
      else
        break;
    }
    else
      break;
  }
}




