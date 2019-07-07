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




void AQFINTS_Parser_DumpElementTree(AQFINTS_ELEMENT *element, int indent)
{

  int i;
  const char *s;
  AQFINTS_ELEMENT *childElement;

  for (i=0; i<indent; i++)
    fprintf(stderr, " ");

  switch(AQFINTS_Element_GetElementType(element)) {
  case AQFINTS_ElementType_Root:  fprintf(stderr, "ROOT "); break;
  case AQFINTS_ElementType_Group: fprintf(stderr, "GROUP"); break;
  case AQFINTS_ElementType_Deg:   fprintf(stderr, "DEG  "); break;
  case AQFINTS_ElementType_De:    fprintf(stderr, "DE   "); break;
  default:                        fprintf(stderr, "(UNK)"); break;
  }

  s=AQFINTS_Element_GetName(element);
  if (s && *s)
    fprintf(stderr, " name=\"%s\"", s?s:"(empty)");

  if (AQFINTS_Element_GetElementType(element)==AQFINTS_ElementType_De) {
    s=AQFINTS_Element_GetType(element);
    if (s && *s)
      fprintf(stderr, " type=\"%s\"", s?s:"(empty)");
  }

  s=AQFINTS_Element_GetId(element);
  if (s && *s)
    fprintf(stderr, " id=\"%s\"", s?s:"(empty)");

  i=AQFINTS_Element_GetVersion(element);
  if (i!=0)
    fprintf(stderr, " version=%d", i);

  s=AQFINTS_Element_GetType(element);
  if (s && *s)
    fprintf(stderr, " type=\"%s\"", s?s:"(empty)");

  i=AQFINTS_Element_GetMinNum(element);
  if (i!=1)
    fprintf(stderr, " minnum=%d", i);

  i=AQFINTS_Element_GetMaxNum(element);
  if (i!=1)
    fprintf(stderr, " maxnum=%d", i);

  i=AQFINTS_Element_GetMinSize(element);
  if (i!=0)
    fprintf(stderr, " minsize=%d", i);

  i=AQFINTS_Element_GetMaxSize(element);
  if (i!=-1)
    fprintf(stderr, " maxsize=%d", i);

  if (AQFINTS_Element_GetDataLength(element)) {
    if (AQFINTS_Element_GetFlags(element) & AQFINTS_ELEMENT_FLAGS_ISBIN) {
      const uint8_t *ptr;
      uint32_t len;

      fprintf(stderr, " binary data: ");
      ptr=AQFINTS_Element_GetDataPointer(element);
      len=AQFINTS_Element_GetDataLength(element);
      GWEN_Text_DumpString((const char*) ptr, len, indent+4);
    }
    else {
      s=(const char*) AQFINTS_Element_GetDataPointer(element);
      fprintf(stderr, " data=\"%s\"", s?s:"(empty)");
    }
  }
  else {
    if (AQFINTS_Element_GetElementType(element)==AQFINTS_ElementType_De)
      fprintf(stderr, " (nodata)");
  }

  fprintf(stderr, "\n");

  childElement=AQFINTS_Element_Tree2_GetFirstChild(element);
  while(childElement) {
    AQFINTS_Parser_DumpElementTree(childElement, indent+2);
    childElement=AQFINTS_Element_Tree2_GetNext(childElement);
  }
}



void AQFINTS_Parser_DumpSegment(AQFINTS_SEGMENT *segment, int indent)
{

  int i;
  const char *s;
  AQFINTS_ELEMENT *elementTree;

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

  i=AQFINTS_Segment_GetStartPos(segment);
  if (i!=0)
    fprintf(stderr, " startpos=%d", i);

  i=AQFINTS_Segment_GetSize(segment);
  if (i!=0)
    fprintf(stderr, " size=%d", i);

  fprintf(stderr, "\n");

  elementTree=AQFINTS_Segment_GetElements(segment);
  if (elementTree)
    AQFINTS_Parser_DumpElementTree(elementTree, indent+2);
}



void AQFINTS_Parser_DumpSegmentList(AQFINTS_SEGMENT_LIST *segmentList, int indent)
{
  AQFINTS_SEGMENT *segment;

  segment=AQFINTS_Segment_List_First(segmentList);
  while(segment) {
    AQFINTS_Parser_DumpSegment(segment, indent);
    segment=AQFINTS_Segment_List_Next(segment);
  }
}




