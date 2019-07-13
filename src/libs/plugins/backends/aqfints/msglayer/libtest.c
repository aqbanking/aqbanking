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
#include "parser_hbci.h"
#include "parser_db.h"



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




int test_readHbci(void)
{
  const char *testData=
    "HNSHK:2:4+PIN:2+942+20190625002302+1+1+1::3333333333333333333333333333+1+1:20190625:002302+1:999:1+6:10:16+280:49999924:1111111111111111111:S:1:1'"
    "HKIDN:3:2+280:49999924+1111111111111111111+2222222222222222222222222222+1'"
    "HKVVB:4:3+4+0+1+AQHBCI+5.99'"
    "HNSHA:5:2+20190625002302++444444444'"
    "TEST1:6:1+testdata1::@12@123456789012'";
  int rv;
  AQFINTS_SEGMENT_LIST *segmentList;

  segmentList=AQFINTS_Segment_List_new();

  rv=AQFINTS_Parser_Hbci_ReadBuffer(segmentList, (const uint8_t*) testData, strlen(testData));
  if (rv<0) {
    fprintf(stderr, "Error reading HBCI data.\n");
    AQFINTS_Segment_List_free(segmentList);
    return 2;
  }

  fprintf(stderr, "Segments:\n");
  AQFINTS_Parser_DumpSegmentList(segmentList, 2);

  fprintf(stderr, "Success.\n");
  return 0;
}



int test_saveFile1(const char *filenameIn, const char *filenameOut)
{
  int rv;
  AQFINTS_SEGMENT_LIST *segmentList;
  AQFINTS_ELEMENT *groupTree;

  segmentList=AQFINTS_Segment_List_new();
  groupTree=AQFINTS_Element_new();
  AQFINTS_Element_SetElementType(groupTree, AQFINTS_ElementType_Root);

  rv=AQFINTS_Parser_Xml_ReadFile(segmentList, groupTree, filenameIn);
  if (rv<0) {
    fprintf(stderr, "Error reading file.\n");
    AQFINTS_Element_Tree2_free(groupTree);
    AQFINTS_Segment_List_free(segmentList);
    return 2;
  }

  AQFINTS_Parser_SegmentList_ResolveGroups(segmentList, groupTree);
  AQFINTS_Parser_SegmentList_Normalize(segmentList);

  rv=AQFINTS_Parser_Xml_WriteSegmentDefinitionFile(segmentList, filenameOut);
  if (rv<0) {
    fprintf(stderr, "Error writing file (%d).\n", rv);
    AQFINTS_Element_Tree2_free(groupTree);
    AQFINTS_Segment_List_free(segmentList);
    return 2;
  }

  AQFINTS_Element_Tree2_free(groupTree);
  AQFINTS_Segment_List_free(segmentList);

  fprintf(stderr, "Success.\n");
  return 0;
}



int test_saveFile2(const char *filenameOut)
{
  const char *testData=
    "HNSHK:2:4+PIN:2+942+20190625002302+1+1+1::3333333333333333333333333333+1+1:20190625:002302+1:999:1+6:10:16+280:49999924:1111111111111111111:S:1:1'"
    "HKIDN:3:2+280:49999924+1111111111111111111+2222222222222222222222222222+1'"
    "HKVVB:4:3+4+0+1+AQHBCI+5.99'"
    "HNSHA:5:2+20190625002302++444444444'"
    "TEST1:6:1+testdata1::@12@123456789012'";
  int rv;
  AQFINTS_SEGMENT_LIST *segmentList;

  segmentList=AQFINTS_Segment_List_new();

  rv=AQFINTS_Parser_Hbci_ReadBuffer(segmentList, (const uint8_t*) testData, strlen(testData));
  if (rv<0) {
    fprintf(stderr, "Error reading HBCI data.\n");
    AQFINTS_Segment_List_free(segmentList);
    return 2;
  }

  rv=AQFINTS_Parser_Xml_WriteSegmentDefinitionFile(segmentList, filenameOut);
  if (rv<0) {
    fprintf(stderr, "Error writing file (%d).\n", rv);
    AQFINTS_Segment_List_free(segmentList);
    return 2;
  }
  AQFINTS_Segment_List_free(segmentList);

  fprintf(stderr, "Success.\n");
  return 0;
}



int test_writeSegments()
{
  const char *testData=
    "HNSHK:2:4+PIN:2+942+20190625002302+1+1+1::3333333333333333333333333333+1+1:20190625:002302+1:999:1+6:10:16+280:49999924:1111111111111111111:S:1:1'"
    "HKIDN:3:2+280:49999924+1111111111111111111+2222222222222222222222222222+1'"
    "HKVVB:4:3+4+0+1+AQHBCI+5.99'"
    "HNSHA:5:2+20190625002302++444444444'"
    "TEST1:6:1+testdata1::@12@123456789012'";
  int rv;
  AQFINTS_SEGMENT_LIST *segmentList;
  GWEN_BUFFER *destBuf;

  segmentList=AQFINTS_Segment_List_new();

  rv=AQFINTS_Parser_Hbci_ReadBuffer(segmentList, (const uint8_t*) testData, strlen(testData));
  if (rv<0) {
    fprintf(stderr, "Error reading HBCI data.\n");
    AQFINTS_Segment_List_free(segmentList);
    return 2;
  }

  destBuf=GWEN_Buffer_new(0, 256, 0, 1);
  AQFINTS_Parser_Hbci_WriteBuffer(segmentList, destBuf);

  GWEN_Buffer_Dump(destBuf, 2);

  fprintf(stderr, "Segments:\n");
  AQFINTS_Parser_DumpSegmentList(segmentList, 2);

  if (strlen(testData)!=GWEN_Buffer_GetUsedBytes(destBuf)) {
    fprintf(stderr, "ERROR: Size differs (orig=%d, returned=%d)\n",
	    (int) strlen(testData), GWEN_Buffer_GetUsedBytes(destBuf));
  }
  if (memcmp(testData, GWEN_Buffer_GetStart(destBuf), GWEN_Buffer_GetUsedBytes(destBuf))) {
    fprintf(stderr, "ERROR: Data differs\n");
  }

  GWEN_Buffer_free(destBuf);
  AQFINTS_Segment_List_free(segmentList);

  fprintf(stderr, "Success.\n");
  return 0;
}



int test_segmentToDb1()
{
  AQFINTS_SEGMENT_LIST *segmentListDef;
  AQFINTS_SEGMENT_LIST *segmentListData;
  AQFINTS_ELEMENT *groupTree;
  GWEN_DB_NODE *dbData;
  int rv;
  const char *defData=
    "<FinTS>"
       "<SEGs>"
         "<SEGdef id=\"Segment1\" code=\"seg1\" segmentVersion=1 protocolVersion=300 >"
           "<DEG name=\"deg1.1\">"
             "<DE name=\"de1.1.1\" type=\"num\"></DE>"
             "<DE name=\"de1.1.2\" type=\"an\"></DE>"
           "</DEG>"
           "<DEG>"
             "<DE name=\"de1.2.1\" type=\"an\"></DE>"
             "<DE name=\"de1.2.2\" type=\"an\"></DE>"
           "</DEG>"
         "</SEGdef>"
      "</SEGs>"
    "</FinTS>";

  const char *elemData=
    "<FinTS>"
      "<SEGs>"
        "<SEGdef id=\"Segment1\" code=\"seg1\" segmentVersion=1 protocolVersion=300 >"
          "<DEG>"
           "<DE>123</DE>"
           "<DE>data 1.1.2</DE>"
          "</DEG>"
          "<DEG>"
           "<DE>data 1.2.1</DE>"
           "<DE>data 1.2.2</DE>"
          "</DEG>"
        "</SEGdef>"
      "</SEGs>"
    "</FinTS>";

  groupTree=AQFINTS_Element_new();
  AQFINTS_Element_SetElementType(groupTree, AQFINTS_ElementType_Root);

  segmentListDef=AQFINTS_Segment_List_new();
  segmentListData=AQFINTS_Segment_List_new();

  rv=AQFINTS_Parser_Xml_ReadBuffer(segmentListDef, groupTree, defData);
  if (rv<0) {
    fprintf(stderr, "Error reading definitions (%d).\n", rv);
    AQFINTS_Element_Tree2_free(groupTree);
    AQFINTS_Segment_List_free(segmentListData);
    AQFINTS_Segment_List_free(segmentListDef);
    return 2;
  }

  rv=AQFINTS_Parser_Xml_ReadBuffer(segmentListData, groupTree, elemData);
  if (rv<0) {
    fprintf(stderr, "Error reading data (%d).\n", rv);
    AQFINTS_Element_Tree2_free(groupTree);
    AQFINTS_Segment_List_free(segmentListData);
    AQFINTS_Segment_List_free(segmentListDef);
    return 2;
  }

  dbData=GWEN_DB_Group_new("data");

  rv=AQFINTS_Parser_Db_ReadSegment(AQFINTS_Segment_List_First(segmentListDef),
                                   AQFINTS_Segment_List_First(segmentListData),
                                   dbData);
  if (rv<0) {
    fprintf(stderr, "Error parsing data.\n");
    GWEN_DB_Group_free(dbData);
    AQFINTS_Element_Tree2_free(groupTree);
    AQFINTS_Segment_List_free(segmentListData);
    AQFINTS_Segment_List_free(segmentListDef);
    return 2;
  }

  GWEN_DB_Dump(dbData, 2);

  GWEN_DB_Group_free(dbData);
  AQFINTS_Element_Tree2_free(groupTree);
  AQFINTS_Segment_List_free(segmentListData);
  AQFINTS_Segment_List_free(segmentListDef);

  fprintf(stderr, "Success.\n");
  return 0;
}



int test_segmentToDb2()
{
  AQFINTS_SEGMENT_LIST *segmentListDef;
  AQFINTS_SEGMENT_LIST *segmentListData;
  AQFINTS_ELEMENT *groupTree;
  GWEN_DB_NODE *dbData;
  int rv;
  const char *defData=
    "<FinTS>"
       "<SEGs>"
         "<SEGdef id=\"Segment1\" code=\"seg1\" segmentVersion=1 protocolVersion=300 >"
           "<DEG name=\"deg1.1\">"
             "<DE name=\"de1.1.1\" type=\"num\"></DE>"
             "<DE name=\"de1.1.2\" type=\"an\"></DE>"
           "</DEG>"
           "<DEG>"
             "<DE name=\"de1.2.1\" type=\"an\"></DE>"
             "<DE name=\"de1.2.2\" type=\"an\" maxnum=99></DE>"
           "</DEG>"
         "</SEGdef>"
      "</SEGs>"
    "</FinTS>";

  const char *elemData=
    "<FinTS>"
      "<SEGs>"
        "<SEGdef id=\"Segment1\" code=\"seg1\" segmentVersion=1 protocolVersion=300 >"
          "<DEG>"
           "<DE>123</DE>"
           "<DE>data 1.1.2</DE>"
          "</DEG>"
          "<DEG>"
           "<DE>data 1.2.1</DE>"
           "<DE>data 1.2.2</DE>"
           "<DE>data 1.2.2b</DE>"
          "</DEG>"
        "</SEGdef>"
      "</SEGs>"
    "</FinTS>";

  groupTree=AQFINTS_Element_new();
  AQFINTS_Element_SetElementType(groupTree, AQFINTS_ElementType_Root);

  segmentListDef=AQFINTS_Segment_List_new();
  segmentListData=AQFINTS_Segment_List_new();

  rv=AQFINTS_Parser_Xml_ReadBuffer(segmentListDef, groupTree, defData);
  if (rv<0) {
    fprintf(stderr, "Error reading definitions (%d).\n", rv);
    AQFINTS_Element_Tree2_free(groupTree);
    AQFINTS_Segment_List_free(segmentListData);
    AQFINTS_Segment_List_free(segmentListDef);
    return 2;
  }

  rv=AQFINTS_Parser_Xml_ReadBuffer(segmentListData, groupTree, elemData);
  if (rv<0) {
    fprintf(stderr, "Error reading data (%d).\n", rv);
    AQFINTS_Element_Tree2_free(groupTree);
    AQFINTS_Segment_List_free(segmentListData);
    AQFINTS_Segment_List_free(segmentListDef);
    return 2;
  }

  dbData=GWEN_DB_Group_new("data");

  rv=AQFINTS_Parser_Db_ReadSegment(AQFINTS_Segment_List_First(segmentListDef),
                                   AQFINTS_Segment_List_First(segmentListData),
                                   dbData);
  if (rv<0) {
    fprintf(stderr, "Error parsing data.\n");
    GWEN_DB_Group_free(dbData);
    AQFINTS_Element_Tree2_free(groupTree);
    AQFINTS_Segment_List_free(segmentListData);
    AQFINTS_Segment_List_free(segmentListDef);
    return 2;
  }

  GWEN_DB_Dump(dbData, 2);

  GWEN_DB_Group_free(dbData);
  AQFINTS_Element_Tree2_free(groupTree);
  AQFINTS_Segment_List_free(segmentListData);
  AQFINTS_Segment_List_free(segmentListDef);

  fprintf(stderr, "Success.\n");
  return 0;
}



int test_segmentToDb3()
{
  AQFINTS_SEGMENT_LIST *segmentListDef;
  AQFINTS_SEGMENT_LIST *segmentListData;
  AQFINTS_ELEMENT *groupTree;
  GWEN_DB_NODE *dbData;
  int rv;
  const char *defData=
    "<FinTS>"
       "<SEGs>"
         "<SEGdef id=\"Segment1\" code=\"seg1\" segmentVersion=1 protocolVersion=300 >"
           "<DEG name=\"deg1.1\">"
             "<DE name=\"de1.1.1\" type=\"num\"></DE>"
             "<DE name=\"de1.1.2\" type=\"an\"></DE>"
           "</DEG>"
           "<DEG>"
             "<DE name=\"de1.2.1\" type=\"an\"></DE>"
             "<DE name=\"de1.2.2\" type=\"an\"></DE>"
             "<DE name=\"de1.2.3\" type=\"an\" minnum=0></DE>"
           "</DEG>"
         "</SEGdef>"
      "</SEGs>"
    "</FinTS>";

  const char *elemData=
    "<FinTS>"
      "<SEGs>"
        "<SEGdef id=\"Segment1\" code=\"seg1\" segmentVersion=1 protocolVersion=300 >"
          "<DEG>"
           "<DE>123</DE>"
           "<DE>data 1.1.2</DE>"
          "</DEG>"
          "<DEG>"
           "<DE>data 1.2.1</DE>"
           "<DE>data 1.2.2</DE>"
          "</DEG>"
        "</SEGdef>"
      "</SEGs>"
    "</FinTS>";

  groupTree=AQFINTS_Element_new();
  AQFINTS_Element_SetElementType(groupTree, AQFINTS_ElementType_Root);

  segmentListDef=AQFINTS_Segment_List_new();
  segmentListData=AQFINTS_Segment_List_new();

  rv=AQFINTS_Parser_Xml_ReadBuffer(segmentListDef, groupTree, defData);
  if (rv<0) {
    fprintf(stderr, "Error reading definitions (%d).\n", rv);
    AQFINTS_Element_Tree2_free(groupTree);
    AQFINTS_Segment_List_free(segmentListData);
    AQFINTS_Segment_List_free(segmentListDef);
    return 2;
  }

  rv=AQFINTS_Parser_Xml_ReadBuffer(segmentListData, groupTree, elemData);
  if (rv<0) {
    fprintf(stderr, "Error reading data (%d).\n", rv);
    AQFINTS_Element_Tree2_free(groupTree);
    AQFINTS_Segment_List_free(segmentListData);
    AQFINTS_Segment_List_free(segmentListDef);
    return 2;
  }

  dbData=GWEN_DB_Group_new("data");

  rv=AQFINTS_Parser_Db_ReadSegment(AQFINTS_Segment_List_First(segmentListDef),
                                   AQFINTS_Segment_List_First(segmentListData),
                                   dbData);
  if (rv<0) {
    fprintf(stderr, "Error parsing data.\n");
    GWEN_DB_Group_free(dbData);
    AQFINTS_Element_Tree2_free(groupTree);
    AQFINTS_Segment_List_free(segmentListData);
    AQFINTS_Segment_List_free(segmentListDef);
    return 2;
  }

  GWEN_DB_Dump(dbData, 2);

  GWEN_DB_Group_free(dbData);
  AQFINTS_Element_Tree2_free(groupTree);
  AQFINTS_Segment_List_free(segmentListData);
  AQFINTS_Segment_List_free(segmentListDef);

  fprintf(stderr, "Success.\n");
  return 0;
}




int main(int args, char **argv)
{
  //test_loadFile("example.xml");
  //test_readHbci();
  //test_saveFile1("example.xml", "example.xml.out");
  //test_saveFile2("example.xml.out");
  //test_writeSegments();
  test_segmentToDb3();

  return 0;
}


