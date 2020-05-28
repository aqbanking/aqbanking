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

#include <gwenhywfar/text.h>



static void appendElementInfo(AQFINTS_ELEMENT *element, GWEN_BUFFER *pbuf, int indent);
static void appendString(const char *name, const char *s, GWEN_BUFFER *pbuf);
static void appendInt(const char *name, int value, int emptyValue, GWEN_BUFFER *pbuf);
static void addPathElementDumpElementTree(AQFINTS_ELEMENT *element, GWEN_BUFFER *pbuf, int indent);
static void dumpPath(AQFINTS_ELEMENT *element, GWEN_BUFFER *pbuf, int indent);
static void dumpElementTreeToBuffer(AQFINTS_ELEMENT *element, GWEN_BUFFER *pbuf, int indent);




void AQFINTS_Parser_DumpElementTree(AQFINTS_ELEMENT *element, int indent)
{

  GWEN_BUFFER *pbuf;

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  dumpElementTreeToBuffer(element, pbuf, indent);

  fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(pbuf));
  GWEN_Buffer_free(pbuf);
}



void dumpElementTreeToBuffer(AQFINTS_ELEMENT *element, GWEN_BUFFER *pbuf, int indent)
{

  AQFINTS_ELEMENT *childElement;

  appendElementInfo(element, pbuf, indent);
  GWEN_Buffer_AppendString(pbuf, "\n");
  childElement=AQFINTS_Element_Tree2_GetFirstChild(element);
  while (childElement) {
    dumpElementTreeToBuffer(childElement, pbuf, indent+2);
    childElement=AQFINTS_Element_Tree2_GetNext(childElement);
  }
}



void AQFINTS_Parser_DumpSegment(AQFINTS_SEGMENT *segment, int indent)
{

  int i;
  const char *s;
  AQFINTS_ELEMENT *elementTree;
  uint32_t rtflags;
  GWEN_DB_NODE *db;

  for (i=0; i<indent; i++)
    fprintf(stderr, " ");

  fprintf(stderr, "SEG  ");

  s=AQFINTS_Segment_GetId(segment);
  if (s && *s)
    fprintf(stderr, " id=\"%s\"", s?s:"(empty)");

  s=AQFINTS_Segment_GetCode(segment);
  if (s && *s)
    fprintf(stderr, " code=\"%s\"", s?s:"(empty)");

  i=AQFINTS_Segment_GetSegmentVersion(segment);
  if (i!=0)
    fprintf(stderr, " segver=%d", i);

  i=AQFINTS_Segment_GetProtocolVersion(segment);
  if (i!=0)
    fprintf(stderr, " protover=%d", i);

  rtflags=AQFINTS_Segment_GetRuntimeFlags(segment);
  if (rtflags & AQFINTS_SEGMENT_RTFLAGS_PARSED)
    fprintf(stderr, " parsed");
  if (rtflags & AQFINTS_SEGMENT_RTFLAGS_SIGNED)
    fprintf(stderr, " signed");
  if (rtflags & AQFINTS_SEGMENT_RTFLAGS_ENCRYPTED)
    fprintf(stderr, " encrypted");
  if (rtflags & AQFINTS_SEGMENT_RTFLAGS_HANDLED)
    fprintf(stderr, " handled");

  fprintf(stderr, "\n");

  db=AQFINTS_Segment_GetDbData(segment);
  if (db) {
    for (i=0; i<indent+2; i++)
      fprintf(stderr, " ");
    fprintf(stderr, "DbData:\n");
    GWEN_DB_Dump(db, indent+4);
  }

  if (1) {
    uint8_t *ptr;
    uint32_t len;

    ptr=AQFINTS_Segment_GetDataPointer(segment);
    len=AQFINTS_Segment_GetDataLength(segment);
    if (ptr && len) {
      for (i=0; i<indent+2; i++)
        fprintf(stderr, " ");
      fprintf(stderr, "Buffer:\n");
      GWEN_Text_DumpString((const char*) ptr, len, indent+4);
    }
  }

  elementTree=AQFINTS_Segment_GetElements(segment);
  if (elementTree)
    AQFINTS_Parser_DumpElementTree(elementTree, indent+2);
}



void AQFINTS_Parser_DumpSegmentList(AQFINTS_SEGMENT_LIST *segmentList, int indent)
{
  AQFINTS_SEGMENT *segment;

  segment=AQFINTS_Segment_List_First(segmentList);
  while (segment) {
    AQFINTS_Parser_DumpSegment(segment, indent);
    segment=AQFINTS_Segment_List_Next(segment);
  }
}



void AQFINTS_Parser_DumpContext(AQFINTS_ELEMENT *elementDef,
                                AQFINTS_ELEMENT *elementData,
                                GWEN_DB_NODE *dbData,
                                int indent)
{
  int i;
  GWEN_BUFFER *pbuf;

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);

  if (indent)
    GWEN_Buffer_FillWithBytes(pbuf, ' ', indent);
  GWEN_Buffer_AppendString(pbuf, "____Parser Context____\n");

  if (elementDef) {
    if (indent)
      GWEN_Buffer_FillWithBytes(pbuf, ' ', indent);
    GWEN_Buffer_AppendString(pbuf, "__Definition Path__\n");
    dumpPath(elementDef, pbuf, indent+2);

    if (indent)
      GWEN_Buffer_FillWithBytes(pbuf, ' ', indent);
    GWEN_Buffer_AppendString(pbuf, "__Definition Content__\n");
    dumpElementTreeToBuffer(elementDef, pbuf, indent+2);
  }

  if (elementData) {
    if (indent)
      GWEN_Buffer_FillWithBytes(pbuf, ' ', indent);
    GWEN_Buffer_AppendString(pbuf, "__Data Path__\n");
    dumpPath(elementData, pbuf, indent+2);

    if (indent)
      GWEN_Buffer_FillWithBytes(pbuf, ' ', indent);
    GWEN_Buffer_AppendString(pbuf, "__Data Content__\n");
    dumpElementTreeToBuffer(elementData, pbuf, indent+2);
  }

  fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(pbuf));
  GWEN_Buffer_free(pbuf);

  if (dbData) {
    for (i=0; i<indent; i++)
      fprintf(stderr, " ");
    fprintf(stderr, "Database data:\n");
    GWEN_DB_Dump(dbData, indent+2);
  }
}



void appendElementInfo(AQFINTS_ELEMENT *element, GWEN_BUFFER *pbuf, int indent)
{

  const char *s;

  if (indent)
    GWEN_Buffer_FillWithBytes(pbuf, ' ', indent);

  switch (AQFINTS_Element_GetElementType(element)) {
  case AQFINTS_ElementType_Root:
    s="ROOT ";
    break;
  case AQFINTS_ElementType_Group:
    s="GROUP";
    break;
  case AQFINTS_ElementType_Deg:
    s="DEG  ";
    break;
  case AQFINTS_ElementType_De:
    s="DE   ";
    break;
  default:
    s="(UNK)";
    break;
  }
  GWEN_Buffer_AppendString(pbuf, s);

  appendString("name", AQFINTS_Element_GetName(element), pbuf);
  appendString("type", AQFINTS_Element_GetType(element), pbuf);
  appendString("id", AQFINTS_Element_GetId(element), pbuf);
  appendInt("version", AQFINTS_Element_GetVersion(element), 0, pbuf);
  appendInt("minnum", AQFINTS_Element_GetMinNum(element), 1, pbuf);
  appendInt("maxnum", AQFINTS_Element_GetMaxNum(element), 1, pbuf);
  appendInt("minsize", AQFINTS_Element_GetMinSize(element), 0, pbuf);
  appendInt("maxsize", AQFINTS_Element_GetMaxSize(element), -1, pbuf);


  if (AQFINTS_Element_GetDataLength(element)) {
    if (AQFINTS_Element_GetFlags(element) & AQFINTS_ELEMENT_FLAGS_ISBIN) {
      const uint8_t *ptr;
      uint32_t len;

      GWEN_Buffer_AppendString(pbuf, " binary data: ");
      ptr=AQFINTS_Element_GetDataPointer(element);
      len=AQFINTS_Element_GetDataLength(element);
      GWEN_Text_DumpString2Buffer((const char *) ptr, len, pbuf, indent+4);
    }
    else {
      s=(const char *) AQFINTS_Element_GetDataPointer(element);
      appendString("data", s, pbuf);
    }
  }
  else {
    if (AQFINTS_Element_GetElementType(element)==AQFINTS_ElementType_De)
      GWEN_Buffer_AppendString(pbuf, " (nodata)");
  }
}



void appendString(const char *name, const char *s, GWEN_BUFFER *pbuf)
{
  if (s && *s) {
    GWEN_Buffer_AppendString(pbuf, " ");
    GWEN_Buffer_AppendString(pbuf, name);
    GWEN_Buffer_AppendString(pbuf, "=\"");
    GWEN_Buffer_AppendString(pbuf, s?s:"(empty)");
    GWEN_Buffer_AppendString(pbuf, "\"");
  }
}



void appendInt(const char *name, int value, int emptyValue, GWEN_BUFFER *pbuf)
{
  if (value!=emptyValue) {
    char numbuf[64];

    if (snprintf(numbuf, sizeof(numbuf)-1, "%d", value)<sizeof(numbuf)) {
      GWEN_Buffer_AppendString(pbuf, " ");
      GWEN_Buffer_AppendString(pbuf, name);
      GWEN_Buffer_AppendString(pbuf, "=\"");
      GWEN_Buffer_AppendString(pbuf, numbuf);
      GWEN_Buffer_AppendString(pbuf, "\"");
    }
  }
}



void addPathElementDumpElementTree(AQFINTS_ELEMENT *element, GWEN_BUFFER *pbuf, int indent)
{
  AQFINTS_ELEMENT *parent;

  parent=AQFINTS_Element_Tree2_GetParent(element);
  if (parent)
    addPathElementDumpElementTree(parent, pbuf, indent-2);

  appendElementInfo(element, pbuf, indent);
  GWEN_Buffer_AppendString(pbuf, "\n");
}



void dumpPath(AQFINTS_ELEMENT *element, GWEN_BUFFER *pbuf, int indent)
{
  AQFINTS_ELEMENT *parent;
  int i;
  int rootIndent;

  /* determine indentation for root */
  i=0;
  parent=AQFINTS_Element_Tree2_GetParent(element);
  while (parent) {
    i++;
    parent=AQFINTS_Element_Tree2_GetParent(parent);
  }
  rootIndent=indent+(i*2);

  addPathElementDumpElementTree(element, pbuf, rootIndent);
}





