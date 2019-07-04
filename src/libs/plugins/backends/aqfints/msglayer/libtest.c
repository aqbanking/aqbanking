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



int test_loadFile(const char *filename)
{
  int rv;
  AQFINTS_SEGMENT_LIST *segmentList;
  AQFINTS_ELEMENT *groupTree;


  segmentList=AQFINTS_Segment_List_new();
  groupTree=AQFINTS_Element_new();

  rv=AQFINTS_Parser_Xml_ReadFile(segmentList, groupTree, filename);
  if (rv<0) {
    fprintf(stderr, "Error reading file.\n");
    AQFINTS_Element_Tree2_free(groupTree);
    AQFINTS_Segment_List_free(segmentList);
    return 2;
  }

  AQFINTS_Parser_DumpElementTree(groupTree, 2);

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


