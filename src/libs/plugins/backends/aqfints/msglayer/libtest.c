/***************************************************************************
 begin       : Fri Jun 28 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#include "parser_xml.h"
#include "parser_dump.h"
#include "parser_normalize.h"



int test_loadFile(const char *filename)
{
  int rv;
  AQFINTS_SEGMENT_LIST *segmentList;
  AQFINTS_ELEMENT *groupTree;


  segmentList=AQFINTS_Segment_List_new();
  groupTree=AQFINTS_Element_new();
  AQFINTS_Element_SetElementType(groupTree, AQFINTS_ElementType_Root);

  rv=AQFINTS_Parser_Xml_ReadFile(segmentList, groupTree, filename);
  if (rv<0) {
    fprintf(stderr, "Error reading file.\n");
    AQFINTS_Element_Tree2_free(groupTree);
    AQFINTS_Segment_List_free(segmentList);
    return 2;
  }

  fprintf(stderr, "Groups:\n");
  AQFINTS_Parser_DumpElementTree(groupTree, 2);

  fprintf(stderr, "Segments:\n");
  AQFINTS_Parser_DumpSegmentList(segmentList, 2);

  AQFINTS_Parser_SegmentList_ResolveGroups(segmentList, groupTree);

  fprintf(stderr, "Segments after resolving groups:\n");
  AQFINTS_Parser_DumpSegmentList(segmentList, 2);

  AQFINTS_Parser_SegmentList_Normalize(segmentList);

  fprintf(stderr, "Segments after normalizing:\n");
  AQFINTS_Parser_DumpSegmentList(segmentList, 2);

  AQFINTS_Element_Tree2_free(groupTree);
  AQFINTS_Segment_List_free(segmentList);

  fprintf(stderr, "Success.\n");
  return 0;
}







int main(int args, char **argv)
{
  test_loadFile("example.xml");

  return 0;
}


