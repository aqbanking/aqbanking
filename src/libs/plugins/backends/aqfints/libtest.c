/***************************************************************************
 begin       : Sun Jul 28 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#include "msglayer/parser/parser.h"
#include "servicelayer/upd/upd_read.h"

#include <gwenhywfar/buffer.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>

#include <errno.h>




static int readFile(const char *fname, GWEN_BUFFER *dbuf)
{
  FILE *f;

  f=fopen(fname, "rb");
  if (f) {
    while (!feof(f)) {
      uint32_t l;
      ssize_t s;
      char *p;

      GWEN_Buffer_AllocRoom(dbuf, 1024);
      l=GWEN_Buffer_GetMaxUnsegmentedWrite(dbuf);
      p=GWEN_Buffer_GetPosPointer(dbuf);
      s=fread(p, 1, l, f);
      if (s==0)
        break;
      if (s==(ssize_t)-1) {
        DBG_ERROR(0,
                  "fread(%s): %s",
                  fname, strerror(errno));
        fclose(f);
        return GWEN_ERROR_IO;
      }

      GWEN_Buffer_IncrementPos(dbuf, s);
      GWEN_Buffer_AdjustUsedBytes(dbuf);
    }

    fclose(f);
    return 0;
  }
  else {
    DBG_ERROR(0,
              "fopen(%s): %s",
              fname, strerror(errno));
    return GWEN_ERROR_IO;
  }
}




int test_parser()
{
  AQFINTS_PARSER *parser;
  int rv;
  GWEN_BUFFER *mbuf;
  GWEN_DB_NODE *dbData;

  parser=AQFINTS_Parser_new();
  AQFINTS_Parser_AddPath(parser, ".");
  rv=AQFINTS_Parser_ReadFiles(parser);
  if (rv<0) {
    fprintf(stderr, "Error reading files.\n");
    return 2;
  }

  mbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=readFile("/home/martin/testdata/upd.test", mbuf);
  if (rv<0) {
    DBG_ERROR(0, "here (%d)", rv);
    return 2;
  }

  dbData=GWEN_DB_Group_new("testdata");
  rv=AQFINTS_Parser_ReadIntoDb(parser,
			       (const uint8_t*) GWEN_Buffer_GetStart(mbuf),
			       GWEN_Buffer_GetUsedBytes(mbuf),
			       dbData);
  if (rv<0) {
    //AQFINTS_Parser_DumpDefinitions(parser, 2);
    return 2;
  }

  GWEN_DB_Dump(dbData, 2);

  fprintf(stderr, "Success.\n");
  return 0;
}



int test_upd()
{
  AQFINTS_PARSER *parser;
  int rv;
  GWEN_BUFFER *mbuf;
  AQFINTS_SEGMENT_LIST *segmentList;
  AQFINTS_USERDATA_LIST *userDataList;
  AQFINTS_ACCOUNTDATA_LIST *accountDataList;

  parser=AQFINTS_Parser_new();
  AQFINTS_Parser_AddPath(parser, ".");
  rv=AQFINTS_Parser_ReadFiles(parser);
  if (rv<0) {
    fprintf(stderr, "Error reading files.\n");
    return 2;
  }

  mbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=readFile("/home/martin/testdata/upd.test", mbuf);
  if (rv<0) {
    DBG_ERROR(0, "here (%d)", rv);
    return 2;
  }

  segmentList=AQFINTS_Segment_List_new();
  rv=AQFINTS_Parser_ReadIntoSegmentList(parser, segmentList,
                                        (const uint8_t*) GWEN_Buffer_GetStart(mbuf),
                                        GWEN_Buffer_GetUsedBytes(mbuf));
  if (rv<0) {
    fprintf(stderr, "Error reading HBCI message into segment list (%d)\n", rv);
    //AQFINTS_Parser_DumpDefinitions(parser, 2);
    return 2;
  }

  rv=AQFINTS_Parser_ReadSegmentListToDb(parser, segmentList);
  if (rv<0) {
    fprintf(stderr, "Error reading DB data for segment list (%d)\n", rv);
    //AQFINTS_Parser_DumpDefinitions(parser, 2);
    return 2;
  }

  userDataList=AQFINTS_Upd_SampleUserDataFromSegmentList(segmentList, 0);
  if (userDataList==NULL)
    fprintf(stderr, "Empty userData list\n");
  else
    fprintf(stderr, "Got a userData list\n");
  accountDataList=AQFINTS_Upd_SampleAccountDataFromSegmentList(segmentList, 0);
  if (accountDataList==NULL)
    fprintf(stderr, "Empty accountData list\n");
  else
    fprintf(stderr, "Got a accountData list\n");


  fprintf(stderr, "Success.\n");
  return 0;
}



int test_upd2()
{
  AQFINTS_PARSER *parser;
  int rv;
  GWEN_BUFFER *mbuf;
  AQFINTS_SEGMENT_LIST *segmentList;
  AQFINTS_USERDATA_LIST *userDataList;

  parser=AQFINTS_Parser_new();
  AQFINTS_Parser_AddPath(parser, ".");
  rv=AQFINTS_Parser_ReadFiles(parser);
  if (rv<0) {
    fprintf(stderr, "Error reading files.\n");
    return 2;
  }

  mbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=readFile("/home/martin/testdata/upd.test", mbuf);
  if (rv<0) {
    DBG_ERROR(0, "here (%d)", rv);
    return 2;
  }

  segmentList=AQFINTS_Segment_List_new();
  rv=AQFINTS_Parser_ReadIntoSegmentList(parser, segmentList,
                                        (const uint8_t*) GWEN_Buffer_GetStart(mbuf),
                                        GWEN_Buffer_GetUsedBytes(mbuf));
  if (rv<0) {
    fprintf(stderr, "Error reading HBCI message into segment list (%d)\n", rv);
    //AQFINTS_Parser_DumpDefinitions(parser, 2);
    return 2;
  }

  rv=AQFINTS_Parser_ReadSegmentListToDb(parser, segmentList);
  if (rv<0) {
    fprintf(stderr, "Error reading DB data for segment list (%d)\n", rv);
    //AQFINTS_Parser_DumpDefinitions(parser, 2);
    return 2;
  }

  userDataList=AQFINTS_Upd_SampleUpdFromSegmentList(segmentList, 0);
  if (userDataList==NULL)
    fprintf(stderr, "Empty userData list\n");
  else
    fprintf(stderr, "Got a userData list\n");

  fprintf(stderr, "Success.\n");
  return 0;
}





int main(int argc, char **argv) {
  //test_parser();
  //test_upd();
  test_upd2();
  return 0;
}

