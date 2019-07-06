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


#include "parser_xml.h"

#include <gwenhywfar/debug.h>




/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static void readGroupsAndSegs(AQFINTS_SEGMENT_LIST *segmentList,
                              AQFINTS_ELEMENT *groupTree,
                              GWEN_XMLNODE *xmlSource);

static void readGroups(AQFINTS_ELEMENT *groupTree, GWEN_XMLNODE *xmlSource);
static void readSegments(AQFINTS_SEGMENT_LIST *segmentList, GWEN_XMLNODE *xmlSource);

static void readElementWithChildren(AQFINTS_ELEMENT *el, GWEN_XMLNODE *xmlSource);
static void readElement(AQFINTS_ELEMENT *el, GWEN_XMLNODE *xmlSource);
static void readChildElements(AQFINTS_ELEMENT *el, GWEN_XMLNODE *xmlSource);

static void readSegment(AQFINTS_SEGMENT *segment, GWEN_XMLNODE *xmlSource);
static void readSegmentWithChildren(AQFINTS_SEGMENT *segment, GWEN_XMLNODE *xmlSource);
static void readSegmentChildren(AQFINTS_SEGMENT *segment, GWEN_XMLNODE *xmlSource);

static void writeSegmentDefinitions(const AQFINTS_SEGMENT_LIST *segmentList, GWEN_XMLNODE *xmlDest);
static void writeSegmentWithElements(const AQFINTS_SEGMENT *segment, GWEN_XMLNODE *xmlDest);
static void writeElementTree(const AQFINTS_ELEMENT *el, GWEN_XMLNODE *xmlDest);

static void writeSegment(const AQFINTS_SEGMENT *segment, GWEN_XMLNODE *xmlDest);
static void writeElement(const AQFINTS_ELEMENT *el, GWEN_XMLNODE *xmlDest);




/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */




int AQFINTS_Parser_Xml_ReadFile(AQFINTS_SEGMENT_LIST *segmentList,
                                AQFINTS_ELEMENT *groupTree,
                                const char *filename)
{
  GWEN_XMLNODE *xmlNodeFile;
  int rv;

  xmlNodeFile=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "fintsFile");
  rv=GWEN_XML_ReadFile(xmlNodeFile, filename, GWEN_XML_FLAGS_HANDLE_COMMENTS | GWEN_XML_FLAGS_HANDLE_HEADERS);
  if (rv<0) {
    DBG_ERROR(0, "Error reading XML file \"%s\" (%d)", filename, rv);
    GWEN_XMLNode_free(xmlNodeFile);
    return rv;
  }
  else {
    GWEN_XMLNODE *xmlNodeFints;

    xmlNodeFints=GWEN_XMLNode_FindFirstTag(xmlNodeFile, "FinTS", NULL, NULL);
    if (xmlNodeFints) {
      readGroupsAndSegs(segmentList, groupTree, xmlNodeFints);
    }
    else {
      GWEN_XMLNode_free(xmlNodeFile);
      return GWEN_ERROR_BAD_DATA;
    }
  }
  GWEN_XMLNode_free(xmlNodeFile);
  return 0;
}



int AQFINTS_Parser_Xml_WriteSegmentDefinitionFile(const AQFINTS_SEGMENT_LIST *segmentList, const char *filename)
{
  GWEN_XMLNODE *xmlFile;
  GWEN_XMLNODE *xmlFinTS;
  GWEN_XMLNODE *xmlSegs;
  int rv;

  xmlFile=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "ROOT");
  xmlFinTS=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "FinTS");
  GWEN_XMLNode_AddChild(xmlFile, xmlFinTS);
  xmlSegs=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "SEGs");
  GWEN_XMLNode_AddChild(xmlFinTS, xmlSegs);

  writeSegmentDefinitions(segmentList, xmlSegs);

  rv=GWEN_XMLNode_WriteFile(xmlFile, filename, GWEN_XML_FLAGS_INDENT | GWEN_XML_FLAGS_HANDLE_COMMENTS);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    GWEN_XMLNode_free(xmlFile);
    return rv;
  }

  GWEN_XMLNode_free(xmlFile);
  return 0;
}



void writeSegmentDefinitions(const AQFINTS_SEGMENT_LIST *segmentList, GWEN_XMLNODE *xmlDest)
{
  const AQFINTS_SEGMENT *segment;

  segment=AQFINTS_Segment_List_First(segmentList);
  while(segment) {
    GWEN_XMLNODE *xmlNode;

    xmlNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "SEGdef");
    writeSegmentWithElements(segment, xmlNode);
    GWEN_XMLNode_AddChild(xmlDest, xmlNode);

    segment=AQFINTS_Segment_List_Next(segment);
  }
}





void readGroupsAndSegs(AQFINTS_SEGMENT_LIST *segmentList,
                       AQFINTS_ELEMENT *groupTree,
                       GWEN_XMLNODE *xmlSource)
{
  GWEN_XMLNODE *xmlNode;

  xmlNode=GWEN_XMLNode_GetFirstTag(xmlSource);
  while(xmlNode) {
    const char *s;

    s=GWEN_XMLNode_GetData(xmlNode);
    if (s && *s) {
      if (strcasecmp(s, "GROUPs")==0) {
        readGroups(groupTree, xmlNode);
      }
      else if (strcasecmp(s, "SEGs")==0) {
        readSegments(segmentList, xmlNode);
      }
      else {
        DBG_INFO(0, "Ignoring XML element \"%s\"", s);
      }
    }
    xmlNode=GWEN_XMLNode_GetNextTag(xmlNode);
  }
}




void readGroups(AQFINTS_ELEMENT *groupTree, GWEN_XMLNODE *xmlSource)
{
  GWEN_XMLNODE *xmlNode;

  xmlNode=GWEN_XMLNode_GetFirstTag(xmlSource);
  while(xmlNode) {
    const char *s;

    s=GWEN_XMLNode_GetData(xmlNode);
    if (s && *s && strcasecmp(s, "GROUPdef")==0) {
      AQFINTS_ELEMENT *elChild;

      elChild=AQFINTS_Element_new();
      AQFINTS_Element_Tree2_AddChild(groupTree, elChild);
      readElementWithChildren(elChild, xmlNode);
    }
    xmlNode=GWEN_XMLNode_GetNextTag(xmlNode);
  }
}



void readSegments(AQFINTS_SEGMENT_LIST *segmentList, GWEN_XMLNODE *xmlSource)
{
  GWEN_XMLNODE *xmlNode;

  xmlNode=GWEN_XMLNode_GetFirstTag(xmlSource);
  while(xmlNode) {
    const char *s;

    s=GWEN_XMLNode_GetData(xmlNode);
    if (s && *s && strcasecmp(s, "SEGdef")==0) {
      AQFINTS_SEGMENT *segment;

      segment=AQFINTS_Segment_new();
      readSegmentWithChildren(segment, xmlNode);
      AQFINTS_Segment_List_Add(segment, segmentList);
    }
    xmlNode=GWEN_XMLNode_GetNextTag(xmlNode);
  }
}





void readElementWithChildren(AQFINTS_ELEMENT *el, GWEN_XMLNODE *xmlSource)
{

  /* read this element */
  readElement(el, xmlSource);

  /* read child elements for some groups */
  switch(AQFINTS_Element_GetElementType(el)) {
  case AQFINTS_ElementType_Root:
  case AQFINTS_ElementType_Group:
  case AQFINTS_ElementType_Deg:
    readChildElements(el, xmlSource);
    break;
  case AQFINTS_ElementType_De:
  case AQFINTS_ElementType_Unknown:
    break;
  }
}



void readChildElements(AQFINTS_ELEMENT *el, GWEN_XMLNODE *xmlSource)
{
  GWEN_XMLNODE *xmlNode;

  xmlNode=GWEN_XMLNode_GetFirstTag(xmlSource);
  while(xmlNode) {
    AQFINTS_ELEMENT *elChild;

    elChild=AQFINTS_Element_new();
    AQFINTS_Element_Tree2_AddChild(el, elChild);
    readElementWithChildren(elChild, xmlNode);
    xmlNode=GWEN_XMLNode_GetNextTag(xmlNode);
  }
}



void readSegmentWithChildren(AQFINTS_SEGMENT *segment, GWEN_XMLNODE *xmlSource)
{
  readSegment(segment, xmlSource);
  readSegmentChildren(segment, xmlSource);
}



void readSegmentChildren(AQFINTS_SEGMENT *segment, GWEN_XMLNODE *xmlSource)
{
  GWEN_XMLNODE *xmlNode;
  AQFINTS_ELEMENT *elementTree;

  /* get or create element tree */
  elementTree=AQFINTS_Segment_GetElements(segment);
  if (elementTree==NULL) {
    elementTree=AQFINTS_Element_new();
    AQFINTS_Element_SetElementType(elementTree, AQFINTS_ElementType_Root);
    AQFINTS_Segment_SetElements(segment, elementTree);
  }

  xmlNode=GWEN_XMLNode_GetFirstTag(xmlSource);
  while(xmlNode) {
    AQFINTS_ELEMENT *elChild;

    elChild=AQFINTS_Element_new();
    AQFINTS_Element_Tree2_AddChild(elementTree, elChild);
    readElementWithChildren(elChild, xmlNode);
    xmlNode=GWEN_XMLNode_GetNextTag(xmlNode);
  }
}



void writeSegmentWithElements(const AQFINTS_SEGMENT *segment, GWEN_XMLNODE *xmlDest)
{
  const AQFINTS_ELEMENT *elements;

  writeSegment(segment, xmlDest);
  elements=AQFINTS_Segment_GetElements(segment);
  if (elements) {
    const AQFINTS_ELEMENT *el;

    el=AQFINTS_Element_Tree2_GetFirstChild(elements);
    while(el) {
      writeElementTree(el, xmlDest);
      el=AQFINTS_Element_Tree2_GetNext(el);
    }
  }
}



void writeElementTree(const AQFINTS_ELEMENT *el, GWEN_XMLNODE *xmlDest)
{
  AQFINTS_ELEMENT_TYPE elementType;
  const char *s;

  elementType=AQFINTS_Element_GetElementType(el);
  switch(elementType) {
  case AQFINTS_ElementType_Root:   s="root";  break;
  case AQFINTS_ElementType_Group:  s="GROUP"; break;
  case AQFINTS_ElementType_De:     s="DE";    break;
  case AQFINTS_ElementType_Deg:    s="DEG";   break;
  default:                         s=NULL;    break;
  }
  if (s) {
    GWEN_XMLNODE *xmlNode;
    const AQFINTS_ELEMENT *elChild;

    xmlNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, s);
    writeElement(el, xmlNode);

    elChild=AQFINTS_Element_Tree2_GetFirstChild(el);
    while(elChild) {
      writeElementTree(elChild, xmlNode);
      elChild=AQFINTS_Element_Tree2_GetNext(elChild);
    }

    GWEN_XMLNode_AddChild(xmlDest, xmlNode);
  }
}



/* ------------------------------------------------------------------------------------------------
 * basic object reading/writing
 * ------------------------------------------------------------------------------------------------
 */



void readElement(AQFINTS_ELEMENT *el, GWEN_XMLNODE *xmlSource)
{
  const char *s;
  int i;
  uint32_t flags=0;

  assert(el);
  assert(xmlSource);

  s=GWEN_XMLNode_GetData(xmlSource);
  if (s && *s) {
    if (strcasecmp(s, "DE")==0)
      AQFINTS_Element_SetElementType(el, AQFINTS_ElementType_De);
    else if (strcasecmp(s, "DEG")==0)
      AQFINTS_Element_SetElementType(el, AQFINTS_ElementType_Deg);
    else if (strcasecmp(s, "Group")==0 || strcasecmp(s, "GROUPdef")==0)
      AQFINTS_Element_SetElementType(el, AQFINTS_ElementType_Group);
    else {
      DBG_ERROR(0, "Invalid element type \"%s\"", s);
      return;
    }
  }

  s=GWEN_XMLNode_GetProperty(xmlSource, "name", NULL);
  if (s && *s)
    AQFINTS_Element_SetName(el, s);

  s=GWEN_XMLNode_GetProperty(xmlSource, "id", NULL);
  if (s && *s)
    AQFINTS_Element_SetId(el, s);

  s=GWEN_XMLNode_GetProperty(xmlSource, "type", NULL);
  if (s && *s)
    AQFINTS_Element_SetType(el, s);

  i=GWEN_XMLNode_GetIntProperty(xmlSource, "minNum", 1);
  AQFINTS_Element_SetMinNum(el, i);

  i=GWEN_XMLNode_GetIntProperty(xmlSource, "maxNum", 1);
  AQFINTS_Element_SetMaxNum(el, i);

  i=GWEN_XMLNode_GetIntProperty(xmlSource, "minSize", 0);
  AQFINTS_Element_SetMinSize(el, i);

  i=GWEN_XMLNode_GetIntProperty(xmlSource, "maxSize", -1);
  AQFINTS_Element_SetMaxSize(el, i);

  i=GWEN_XMLNode_GetIntProperty(xmlSource, "trustLevel", 0);
  AQFINTS_Element_SetTrustLevel(el, i);

  flags|=(GWEN_XMLNode_GetIntProperty(xmlSource, "leftFill", 0)?AQFINTS_ELEMENT_FLAGS_LEFTFILL:0);
  flags|=(GWEN_XMLNode_GetIntProperty(xmlSource, "rightFill", 0)?AQFINTS_ELEMENT_FLAGS_RIGHTFILL:0);
  flags|=(GWEN_XMLNode_GetIntProperty(xmlSource, "isBin", 0)?AQFINTS_ELEMENT_FLAGS_ISBIN:0);
  AQFINTS_Element_SetFlags(el, flags);
}



void writeElement(const AQFINTS_ELEMENT *el, GWEN_XMLNODE *xmlDest)
{
  const char *s;
  int i;
  uint32_t flags;

  assert(el);
  assert(xmlDest);

  s=AQFINTS_Element_GetName(el);
  if (s && *s)
    GWEN_XMLNode_SetProperty(xmlDest, "name", s);

  s=AQFINTS_Element_GetId(el);
  if (s && *s)
    GWEN_XMLNode_SetProperty(xmlDest, "id", s);

  s=AQFINTS_Element_GetType(el);
  if (s && *s)
    GWEN_XMLNode_SetProperty(xmlDest, "type", s);

  i=AQFINTS_Element_GetMinNum(el);
  if (i!=1)
    GWEN_XMLNode_SetIntProperty(xmlDest, "minNum", i);

  i=AQFINTS_Element_GetMaxNum(el);
  if (i!=1)
    GWEN_XMLNode_SetIntProperty(xmlDest, "maxNum", i);

  i=AQFINTS_Element_GetMinSize(el);
  if (i!=0)
    GWEN_XMLNode_SetIntProperty(xmlDest, "minSize", i);

  i=AQFINTS_Element_GetMaxSize(el);
  if (i!=-1)
    GWEN_XMLNode_SetIntProperty(xmlDest, "maxSize", i);

  i=AQFINTS_Element_GetTrustLevel(el);
  if (i!=0)
    GWEN_XMLNode_SetIntProperty(xmlDest, "trustLevel", i);

  flags=AQFINTS_Element_GetFlags(el);
  if (flags & AQFINTS_ELEMENT_FLAGS_LEFTFILL)
    GWEN_XMLNode_SetIntProperty(xmlDest, "leftFill", 1);
  if (flags & AQFINTS_ELEMENT_FLAGS_RIGHTFILL)
    GWEN_XMLNode_SetIntProperty(xmlDest, "rightFill", 1);
  if (flags & AQFINTS_ELEMENT_FLAGS_ISBIN)
    GWEN_XMLNode_SetIntProperty(xmlDest, "isBin", 1);
}



void readSegment(AQFINTS_SEGMENT *segment, GWEN_XMLNODE *xmlSource)
{
  const char *s;
  int i;

  assert(segment);
  assert(xmlSource);

  s=GWEN_XMLNode_GetProperty(xmlSource, "id", NULL);
  if (s && *s)
    AQFINTS_Segment_SetId(segment, s);

  s=GWEN_XMLNode_GetProperty(xmlSource, "code", NULL);
  if (s && *s)
    AQFINTS_Segment_SetCode(segment, s);

  i=GWEN_XMLNode_GetIntProperty(xmlSource, "segmentVersion", -1);
  AQFINTS_Segment_SetSegmentVersion(segment, i);

  i=GWEN_XMLNode_GetIntProperty(xmlSource, "protocolVersion", -1);
  AQFINTS_Segment_SetProtocolVersion(segment, i);
}



void writeSegment(const AQFINTS_SEGMENT *segment, GWEN_XMLNODE *xmlDest)
{
  const char *s;
  int i;

  assert(segment);
  assert(xmlDest);

  s=AQFINTS_Segment_GetId(segment);
  if (s && *s)
    GWEN_XMLNode_SetProperty(xmlDest, "id", s);

  s=AQFINTS_Segment_GetCode(segment);
  if (s && *s)
    GWEN_XMLNode_SetProperty(xmlDest, "code", s);

  i=AQFINTS_Segment_GetSegmentVersion(segment);
  if (i!=-1)
    GWEN_XMLNode_SetIntProperty(xmlDest, "segmentVersion", i);

  i=AQFINTS_Segment_GetProtocolVersion(segment);
  if (i!=-1)
    GWEN_XMLNode_SetIntProperty(xmlDest, "protocolVersion", i);
}



