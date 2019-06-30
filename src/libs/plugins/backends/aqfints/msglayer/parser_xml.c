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
                              AQFINTS_ELEMENT_TREE *groupTree,
                              GWEN_XMLNODE *xmlSource);

static void readGroups(AQFINTS_ELEMENT_TREE *groupTree, GWEN_XMLNODE *xmlSource);
static void readSegments(AQFINTS_SEGMENT_LIST *segmentList, GWEN_XMLNODE *xmlSource);

static void readElementWithChildren(AQFINTS_ELEMENT *el, GWEN_XMLNODE *xmlSource);
static void readElement(AQFINTS_ELEMENT *el, GWEN_XMLNODE *xmlSource);
static void readChildElements(AQFINTS_ELEMENT *el, GWEN_XMLNODE *xmlSource);

static void readSegment(AQFINTS_SEGMENT *segment, GWEN_XMLNODE *xmlSource);
static void readSegmentWithChildren(AQFINTS_SEGMENT *segment, GWEN_XMLNODE *xmlSource);
static void readSegmentChildren(AQFINTS_SEGMENT *segment, GWEN_XMLNODE *xmlSource);




/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */




int AQFINTS_Parser_Xml_ReadFile(AQFINTS_SEGMENT_LIST *segmentList,
                                AQFINTS_ELEMENT_TREE *groupTree,
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

    DBG_ERROR(0, "Read this file:");
    GWEN_XMLNode_Dump(xmlNodeFile, 2);

    xmlNodeFints=GWEN_XMLNode_FindFirstTag(xmlNodeFile, "FinTS", NULL, NULL);
    if (xmlNodeFints) {
      DBG_ERROR(0, "Reading segments and groups");
      readGroupsAndSegs(segmentList, groupTree, xmlNodeFints);
    }
    else {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing \"FinTS\" in XML file \"%s\"", filename);
      GWEN_XMLNode_free(xmlNodeFile);
      return GWEN_ERROR_BAD_DATA;
    }
  }
  GWEN_XMLNode_free(xmlNodeFile);
  return 0;
}




void readGroupsAndSegs(AQFINTS_SEGMENT_LIST *segmentList,
                       AQFINTS_ELEMENT_TREE *groupTree,
                       GWEN_XMLNODE *xmlSource)
{
  GWEN_XMLNODE *xmlNode;

  xmlNode=GWEN_XMLNode_GetFirstTag(xmlSource);
  while(xmlNode) {
    const char *s;

    s=GWEN_XMLNode_GetData(xmlNode);
    if (s && *s) {
      if (strcasecmp(s, "GROUPs")==0) {
        DBG_ERROR(0, "Reading groups");
        readGroups(groupTree, xmlNode);
      }
      else if (strcasecmp(s, "SEGs")==0) {
        DBG_ERROR(0, "Reading segments");
        readSegments(segmentList, xmlNode);
      }
      else {
        DBG_INFO(0, "Ignoring XML element \"%s\"", s);
      }
    }
    xmlNode=GWEN_XMLNode_GetNextTag(xmlNode);
  }
}




void readGroups(AQFINTS_ELEMENT_TREE *groupTree, GWEN_XMLNODE *xmlSource)
{
  GWEN_XMLNODE *xmlNode;

  xmlNode=GWEN_XMLNode_GetFirstTag(xmlSource);
  while(xmlNode) {
    const char *s;

    s=GWEN_XMLNode_GetData(xmlNode);
    if (s && *s && strcasecmp(s, "GROUPdef")==0) {
      AQFINTS_ELEMENT *elChild;

      elChild=AQFINTS_Element_new();
      AQFINTS_Element_Tree_Add(groupTree, elChild);
      DBG_ERROR(0, "Reading group");
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

  DBG_ERROR(0, "Reading element \"%s\"", GWEN_XMLNode_GetData(xmlSource));

  /* read this element */
  readElement(el, xmlSource);

  DBG_ERROR(0, "Reading child elements");

  /* read child elements for some groups */
  switch(AQFINTS_Element_GetElementType(el)) {
  case AQFINTS_ElementType_Group:
  case AQFINTS_ElementType_Deg:
    readChildElements(el, xmlSource);
    break;
  case AQFINTS_ElementType_De:
  case AQFINTS_ElementType_Unknown:
    break;
  }

  DBG_ERROR(0, "Done.");
}



void readChildElements(AQFINTS_ELEMENT *el, GWEN_XMLNODE *xmlSource)
{
  GWEN_XMLNODE *xmlNode;

  DBG_ERROR(0, "ReadChildElements(\"%s\")", GWEN_XMLNode_GetData(xmlSource));

  xmlNode=GWEN_XMLNode_GetFirstTag(xmlSource);
  while(xmlNode) {
    AQFINTS_ELEMENT *elChild;

    DBG_ERROR(0, "Reading child element \"%s\"", GWEN_XMLNode_GetData(xmlNode));

    elChild=AQFINTS_Element_new();
    AQFINTS_Element_Tree_AddChild(el, elChild);
    readElementWithChildren(elChild, xmlNode);
    xmlNode=GWEN_XMLNode_GetNextTag(xmlNode);
  }
  DBG_ERROR(0, "Done.");
}



void readSegmentWithChildren(AQFINTS_SEGMENT *segment, GWEN_XMLNODE *xmlSource)
{
  readSegment(segment, xmlSource);
  readSegmentChildren(segment, xmlSource);
}



void readSegmentChildren(AQFINTS_SEGMENT *segment, GWEN_XMLNODE *xmlSource)
{
  GWEN_XMLNODE *xmlNode;
  AQFINTS_ELEMENT_TREE *elementTree;

  /* get or create element tree */
  elementTree=AQFINTS_Segment_GetElements(segment);
  if (elementTree==NULL) {
    elementTree=AQFINTS_Element_Tree_new();
    AQFINTS_Segment_SetElements(segment, elementTree);
  }

  xmlNode=GWEN_XMLNode_GetFirstTag(xmlSource);
  while(xmlNode) {
    AQFINTS_ELEMENT *elChild;

    elChild=AQFINTS_Element_new();
    AQFINTS_Element_Tree_Add(elementTree, elChild);
    readElementWithChildren(elChild, xmlNode);
    xmlNode=GWEN_XMLNode_GetNextTag(xmlNode);
  }
}




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

  s=GWEN_XMLNode_GetProperty(xmlSource, "ref", NULL);
  if (s && *s)
    AQFINTS_Element_SetRef(el, s);

  i=GWEN_XMLNode_GetIntProperty(xmlSource, "dbType", AQFINTS_ElementDataType_Unknown);
  AQFINTS_Element_SetDbType(el, i);

  s=GWEN_XMLNode_GetProperty(xmlSource, "type", NULL);
  if (s && *s)
    AQFINTS_Element_SetType(el, s);

  i=GWEN_XMLNode_GetIntProperty(xmlSource, "minNum", 0);
  AQFINTS_Element_SetMinNum(el, i);

  i=GWEN_XMLNode_GetIntProperty(xmlSource, "maxNum", 0);
  AQFINTS_Element_SetMaxNum(el, i);

  i=GWEN_XMLNode_GetIntProperty(xmlSource, "minSize", 0);
  AQFINTS_Element_SetMinSize(el, i);

  i=GWEN_XMLNode_GetIntProperty(xmlSource, "maxSize", 0);
  AQFINTS_Element_SetMaxSize(el, i);

  flags|=(GWEN_XMLNode_GetIntProperty(xmlSource, "leftFill", 0)?AQFINTS_ELEMENT_FLAGS_LEFTFILL:0);
  flags|=(GWEN_XMLNode_GetIntProperty(xmlSource, "rightFill", 0)?AQFINTS_ELEMENT_FLAGS_RIGHTFILL:0);
  AQFINTS_Element_SetFlags(el, flags);
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

