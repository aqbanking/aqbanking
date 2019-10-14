/***************************************************************************
 begin       : Sun Jul 28 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#include "msglayer/parser/parser.h"
#include "msglayer/parser/parser_dump.h"
#include "servicelayer/upd/upd_read.h"
#include "transportlayer/transportssl.h"
#include "sessionlayer/session.h"

#include <gwenhywfar/buffer.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/cgui.h>

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
                               (const uint8_t *) GWEN_Buffer_GetStart(mbuf),
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
                                        (const uint8_t *) GWEN_Buffer_GetStart(mbuf),
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



int test_readHbci(const char *fileName)
{
  AQFINTS_PARSER *parser;
  int rv;
  GWEN_BUFFER *mbuf;
  AQFINTS_SEGMENT_LIST *segmentList;

  parser=AQFINTS_Parser_new();
  AQFINTS_Parser_AddPath(parser, ".");
  rv=AQFINTS_Parser_ReadFiles(parser);
  if (rv<0) {
    fprintf(stderr, "Error reading files.\n");
    return 2;
  }

  mbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=readFile(fileName, mbuf);
  if (rv<0) {
    DBG_ERROR(0, "here (%d)", rv);
    return 2;
  }

  segmentList=AQFINTS_Segment_List_new();
  rv=AQFINTS_Parser_ReadIntoSegmentList(parser, segmentList,
                                        (const uint8_t *) GWEN_Buffer_GetStart(mbuf),
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

  AQFINTS_Parser_DumpSegmentList(segmentList, 2);


  fprintf(stderr, "Success.\n");
  return 0;
}



int test_getBpd()
{
  GWEN_GUI *cgui;
  AQFINTS_PARSER *parser;
  AQFINTS_SESSION *session;
  AQFINTS_TRANSPORT *transport;
  AQFINTS_BPD *bpd;
  int rv;
  GWEN_DB_NODE *dbSettings;
  const char *sUrl;
  const char *sBankCode;
  int httpVersionMajor;
  int httpVersionMinor;
  int hbciVersion;

  cgui=GWEN_Gui_CGui_new();
  GWEN_Gui_SetGui(cgui);

  dbSettings=GWEN_DB_Group_new("settings");
  rv=GWEN_DB_ReadFile(dbSettings, "/home/martin/testdata/user1.cfg", GWEN_DB_FLAGS_DEFAULT);
  if (rv<0) {
    fprintf(stderr, "Error reading config file.\n");
    return 2;
  }

  parser=AQFINTS_Parser_new();
  AQFINTS_Parser_AddPath(parser, ".");
  rv=AQFINTS_Parser_ReadFiles(parser);
  if (rv<0) {
    fprintf(stderr, "Error reading files.\n");
    return 2;
  }

  sUrl=GWEN_DB_GetCharValue(dbSettings, "url", 0, NULL);
  if (sUrl==NULL) {
    fprintf(stderr, "Settings: No url.\n");
    return 2;
  }
  sBankCode=GWEN_DB_GetCharValue(dbSettings, "bankCode", 0, NULL);
  if (sBankCode==NULL) {
    fprintf(stderr, "Settings: No bank code.\n");
    return 2;
  }
  httpVersionMajor=GWEN_DB_GetIntValue(dbSettings, "httpVersionMajor", 0, 1);
  httpVersionMinor=GWEN_DB_GetIntValue(dbSettings, "httpVersionMinor", 0, 1);
  hbciVersion=GWEN_DB_GetIntValue(dbSettings, "hbciVersion", 0, 300);

  transport=AQFINTS_TransportSsl_new(sUrl);
  AQFINTS_TransportSsl_SetVersionMajor(transport, httpVersionMajor);
  AQFINTS_TransportSsl_SetVersionMinor(transport, httpVersionMinor);

  session=AQFINTS_Session_new(parser, transport);
  AQFINTS_Session_SetHbciVersion(session, hbciVersion);

  rv=AQFINTS_Session_GetAnonBpd(session, sBankCode);
  if (rv<0) {
    fprintf(stderr, "Error creating GetBPD request (%d).\n", rv);
    return 2;
  }

  bpd=AQFINTS_Session_GetBpd(session);
  if (bpd) {
    GWEN_DB_NODE *dbBpd;

    dbBpd=GWEN_DB_Group_new("Bpd");
    AQFINTS_Bpd_toDb(bpd, dbBpd);
    fprintf(stderr, "Got these BPD:\n");
    GWEN_DB_Dump(dbBpd, 2);
  }

  fprintf(stderr, "Success.\n");
  return 0;
}






int main(int argc, char **argv)
{
  //test_parser();
  //test_upd();
  //test_getBpd();
  test_readHbci("/tmp/test.hbci");
  return 0;
}

