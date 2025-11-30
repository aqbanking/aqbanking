/***************************************************************************
 begin       : Sat Oct 26 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "logfile.h"

#include "aqfints.h"
#include "parser/parser.h"
#include "parser/parser_dump.h"
#include "banking/provider_l.h"

#include <gwenhywfar/text.h>
#include <gwenhywfar/syncio_file.h>
#include <gwenhywfar/args.h>



static int _readLogFile(const char *fname, GWEN_DB_NODE *db);
static int parseMessages(AB_PROVIDER *pro, GWEN_DB_NODE *dbMessages, int doListSegments, GWEN_SYNCIO *sioDb);
static int dumpSegmentListToDb(AQFINTS_SEGMENT_LIST *segmentList, GWEN_DB_NODE *dbHeader, GWEN_SYNCIO *sioDb);
static int listSegments(AQFINTS_SEGMENT_LIST *segmentList, GWEN_DB_NODE *dbHeader, GWEN_BUFFER *bufSegmentList);




int AF_Control_LogFile(AB_PROVIDER *pro, GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  int rv;
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbMessages;
  GWEN_SYNCIO *sioDb=NULL;
  const char *inFile;
  const char *dbOutFile;
  int doListSegments=0;
  const GWEN_ARGS args[]= {
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "infile",                     /* name */
      1,                            /* minnum */
      1,                            /* maxnum */
      "i",                          /* short option */
      "infile",                     /* long option */
      "Specify input file",         /* short description */
      "Specify input file"          /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,           /* type */
      "dboutfile",                  /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "d",                          /* short option */
      "dbfile",                     /* long option */
      "Specify DB output file",     /* short description */
      "Specify DB output file"      /* long description */
    },
    {
      0, /* flags */
      GWEN_ArgsType_Int,           /* type */
      "listSegments",                 /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "l",                          /* short option */
      "list-segments",                   /* long option */
      "List segments",    /* short description */
      "List segments"     /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, /* flags */
      GWEN_ArgsType_Int,            /* type */
      "help",                       /* name */
      0,                            /* minnum */
      0,                            /* maxnum */
      "h",                          /* short option */
      "help",                       /* long option */
      "Show this help screen",      /* short description */
      "Show this help screen"       /* long description */
    }
  };

  db=GWEN_DB_GetGroup(dbArgs, GWEN_DB_FLAGS_DEFAULT, "local");
  rv=AB_Cmd_Handle_Args(argc, argv, args, db);
  if (rv==GWEN_ARGS_RESULT_ERROR) {
    return 1;
  }
  else if (rv==GWEN_ARGS_RESULT_HELP) {
    return 0;
  }

  dbOutFile=GWEN_DB_GetCharValue(db, "dbOutFile", 0, NULL);
  inFile=GWEN_DB_GetCharValue(db, "inFile", 0, NULL);
  assert(inFile);

  doListSegments=GWEN_DB_GetIntValue(db, "listSegments", 0, 0);

  dbMessages=GWEN_DB_Group_new("Messages");
  rv=_readLogFile(inFile, dbMessages);
  if (rv<0) {
    DBG_ERROR(0, "Error reading message (%d)", rv);
    GWEN_DB_Group_free(dbMessages);
    return 2;
  }

  if (dbOutFile) {
    sioDb=GWEN_SyncIo_File_new(dbOutFile, GWEN_SyncIo_File_CreationMode_CreateAlways);
    GWEN_SyncIo_AddFlags(sioDb,
                         GWEN_SYNCIO_FILE_FLAGS_READ |
                         GWEN_SYNCIO_FILE_FLAGS_WRITE |
                         GWEN_SYNCIO_FILE_FLAGS_UREAD |
                         GWEN_SYNCIO_FILE_FLAGS_UWRITE |
                         GWEN_SYNCIO_FILE_FLAGS_APPEND);
    rv=GWEN_SyncIo_Connect(sioDb);
    if (rv<0) {
      DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
      GWEN_SyncIo_free(sioDb);
      return 2;
    }
  }

  rv=parseMessages(pro, dbMessages, doListSegments, sioDb);
  if (rv<0) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "Error parsing messages (%d)", rv);
    GWEN_DB_Group_free(dbMessages);
    GWEN_SyncIo_free(sioDb);
    return 2;
  }


  /* close output layer */
  if (dbOutFile) {
    rv=GWEN_SyncIo_Disconnect(sioDb);
    if (rv<0) {
      DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
      GWEN_SyncIo_free(sioDb);
      return 2;
    }
    GWEN_SyncIo_free(sioDb);
  }

  return 0;
}



int _readLogFile(const char *fname, GWEN_DB_NODE *db)
{
  GWEN_SYNCIO *sio;
  GWEN_FAST_BUFFER *fb;
  int rv;
  GWEN_BUFFER *tbuf = NULL;

  sio=GWEN_SyncIo_File_new(fname, GWEN_SyncIo_File_CreationMode_OpenExisting);
  GWEN_SyncIo_AddFlags(sio, GWEN_SYNCIO_FILE_FLAGS_READ);
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_ERROR(0, "Error opening file [%s] (%d)", fname, rv);
    return rv;
  }

  /* create fast buffer around io layer */
  fb=GWEN_FastBuffer_new(1024, sio);

  for (;;) {
    GWEN_DB_NODE *dbMsg;
    GWEN_DB_NODE *dbHeader;
    unsigned int size;

    /* read header */
    dbMsg=GWEN_DB_Group_new("Message");
    dbHeader=GWEN_DB_GetGroup(dbMsg, GWEN_DB_FLAGS_DEFAULT, "header");

    rv=GWEN_DB_ReadFromFastBuffer(dbHeader, fb,
                                  GWEN_DB_FLAGS_HTTP |
                                  GWEN_DB_FLAGS_UNTIL_EMPTY_LINE);
    if (rv<0) {
      if (rv==GWEN_ERROR_EOF)
        break;
      else {
        GWEN_DB_Group_free(dbMsg);
        GWEN_FastBuffer_free(fb);
        GWEN_SyncIo_Disconnect(sio);
        GWEN_SyncIo_free(sio);
        DBG_ERROR(0, "Error reading header from file [%s] (%d)", fname, rv);
        GWEN_DB_Dump(db, 2);
        return rv;
      }
    }

    /* read body */
    size=GWEN_DB_GetIntValue(dbHeader, "size", 0, 0);
    tbuf=GWEN_Buffer_new(0, 2048, 0, 1);
    while (size) {
      unsigned int lsize;
      uint8_t buffer[1024];

      lsize=size;
      if (lsize>sizeof(buffer))
        lsize=sizeof(buffer);

      GWEN_FASTBUFFER_READFORCED(fb, rv, buffer, lsize);
      if (rv<0) {
        GWEN_DB_Group_free(dbMsg);
        GWEN_FastBuffer_free(fb);
        GWEN_SyncIo_Disconnect(sio);
        GWEN_SyncIo_free(sio);
        DBG_ERROR(0, "Error reading body from file [%s] (%d)", fname, rv);
        return rv;
      }
      GWEN_Buffer_AppendBytes(tbuf, (const char *)buffer, lsize);
      size-=lsize;
    } // while

    GWEN_DB_SetBinValue(dbMsg, GWEN_DB_FLAGS_OVERWRITE_VARS, "body",
                        GWEN_Buffer_GetStart(tbuf),
                        GWEN_Buffer_GetUsedBytes(tbuf));
    GWEN_Buffer_Reset(tbuf);

    GWEN_DB_AddGroup(db, dbMsg);
  }
  GWEN_Buffer_free(tbuf);

  GWEN_FastBuffer_free(fb);
  GWEN_SyncIo_Disconnect(sio);
  GWEN_SyncIo_free(sio);

  return 0;
}



int parseMessages(AB_PROVIDER *pro, GWEN_DB_NODE *dbMessages, int doListSegments, GWEN_SYNCIO *sioDb)
{
  GWEN_DB_NODE *dbT;
  AQFINTS_PARSER *parser;
  int rv;
  GWEN_BUFFER *bufSegmentList;

  parser=AF_Provider_CreateParser(pro);
  if (parser==NULL) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "Could not create parser");
    return GWEN_ERROR_GENERIC;
  }


  bufSegmentList=GWEN_Buffer_new(0, 256, 0, 1);

  dbT=GWEN_DB_GetFirstGroup(dbMessages);
  while (dbT) {
    const uint8_t *p;
    uint32_t len;
    GWEN_DB_NODE *dbHeader;

    dbHeader=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "header");
    assert(dbHeader);

    p=GWEN_DB_GetBinValue(dbT, "body", 0, NULL, 0, &len);
    if (p && len) {
      AQFINTS_SEGMENT_LIST *segmentList;

      segmentList=AQFINTS_Segment_List_new();
      rv=AQFINTS_Parser_ReadIntoSegmentList(parser, segmentList, p, len);
      if (rv<0) {
        DBG_ERROR(AQFINTS_LOGDOMAIN, "Error reading HBCI message into segment list (%d)", rv);
        //AQFINTS_Parser_DumpDefinitions(parser, 2);
        AQFINTS_Segment_List_free(segmentList);
        AQFINTS_Parser_free(parser);
        return rv;
      }

      rv=AQFINTS_Parser_ReadSegmentListToDb(parser, segmentList);
      if (rv<0) {
        DBG_INFO(AQFINTS_LOGDOMAIN, "Error reading DB data for segment list (%d)", rv);
        //AQFINTS_Parser_DumpDefinitions(parser, 2);
        AQFINTS_Segment_List_free(segmentList);
        AQFINTS_Parser_free(parser);
        return rv;
      }

      if (sioDb) {
        rv=dumpSegmentListToDb(segmentList, dbHeader, sioDb);
        if (rv<0) {
          DBG_ERROR(AQFINTS_LOGDOMAIN, "Error writing DB data for segment list (%d)", rv);
          //AQFINTS_Parser_DumpDefinitions(parser, 2);
          AQFINTS_Segment_List_free(segmentList);
          AQFINTS_Parser_free(parser);
          return rv;
        }
      }

      if (doListSegments) {
        rv=listSegments(segmentList, dbHeader, bufSegmentList);
        if (rv<0) {
          DBG_ERROR(AQFINTS_LOGDOMAIN, "Error listing segment list (%d)", rv);
          //AQFINTS_Parser_DumpDefinitions(parser, 2);
          AQFINTS_Segment_List_free(segmentList);
          AQFINTS_Parser_free(parser);
          return rv;
        }
      }

      /*AQFINTS_Parser_DumpSegmentList(segmentList, 2);*/
      AQFINTS_Segment_List_free(segmentList);
    }

    dbT=GWEN_DB_GetNextGroup(dbT);
  }
  AQFINTS_Parser_free(parser);

  if (doListSegments) {
    fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(bufSegmentList));
  }

  GWEN_Buffer_free(bufSegmentList);
  return 0;
}



int dumpSegmentListToDb(AQFINTS_SEGMENT_LIST *segmentList, GWEN_DB_NODE *dbHeader, GWEN_SYNCIO *sioDb)
{
  GWEN_DB_NODE *dbOut;
  GWEN_BUFFER *xbuf;
  AQFINTS_SEGMENT *segment;
  const char *s;
  int rv;


  dbOut=GWEN_DB_Group_new("Messages");

  segment=AQFINTS_Segment_List_First(segmentList);
  while (segment) {
    GWEN_DB_NODE *dbSegment;

    dbSegment=AQFINTS_Segment_GetDbData(segment);
    if (dbSegment)
      GWEN_DB_AddGroup(dbOut, GWEN_DB_Group_dup(dbSegment));
    segment=AQFINTS_Segment_List_Next(segment);
  }

  xbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(xbuf, "# ========== Message ( ");
  s=GWEN_DB_GetCharValue(dbHeader, "sender", 0, "UNK");
  if (s && *s) {
    GWEN_Buffer_AppendString(xbuf, "sender=");
    GWEN_Buffer_AppendString(xbuf, s);
    GWEN_Buffer_AppendString(xbuf, " ");
  }
  s=GWEN_DB_GetCharValue(dbHeader, "crypt", 0, "UNK");
  if (s && *s) {
    GWEN_Buffer_AppendString(xbuf, "crypt=");
    GWEN_Buffer_AppendString(xbuf, s);
    GWEN_Buffer_AppendString(xbuf, " ");
  }
  GWEN_Buffer_AppendString(xbuf, ") ==========\n");

  rv=GWEN_SyncIo_WriteForced(sioDb, (const uint8_t *) GWEN_Buffer_GetStart(xbuf), GWEN_Buffer_GetUsedBytes(xbuf));
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(xbuf);
    GWEN_DB_Group_free(dbOut);
    return rv;
  }
  GWEN_Buffer_free(xbuf);

  rv=GWEN_DB_WriteToIo(dbOut, sioDb, GWEN_DB_FLAGS_DEFAULT);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(dbOut);
    return rv;
  }

  GWEN_DB_Group_free(dbOut);

  /* append empty line to separate header from data */
  rv=GWEN_SyncIo_WriteForced(sioDb, (const uint8_t *) "\n\n", 1);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int listSegments(AQFINTS_SEGMENT_LIST *segmentList, GWEN_DB_NODE *dbHeader, GWEN_BUFFER *bufSegmentList)
{
  AQFINTS_SEGMENT *segment;
  const char *s;

  GWEN_Buffer_AppendString(bufSegmentList, "# ========== Message ( ");
  s=GWEN_DB_GetCharValue(dbHeader, "sender", 0, "UNK");
  if (s && *s) {
    GWEN_Buffer_AppendString(bufSegmentList, "sender=");
    GWEN_Buffer_AppendString(bufSegmentList, s);
    GWEN_Buffer_AppendString(bufSegmentList, " ");
  }
  s=GWEN_DB_GetCharValue(dbHeader, "crypt", 0, "UNK");
  if (s && *s) {
    GWEN_Buffer_AppendString(bufSegmentList, "crypt=");
    GWEN_Buffer_AppendString(bufSegmentList, s);
    GWEN_Buffer_AppendString(bufSegmentList, " ");
  }
  GWEN_Buffer_AppendString(bufSegmentList, ") ==========\n");

  segment=AQFINTS_Segment_List_First(segmentList);
  while (segment) {
    const char *segId;
    const char *segCode;
    int segNumber;
    int segVersion;
    uint32_t runtimeFlags;
    char lineBuffer[2048];

    segNumber=AQFINTS_Segment_GetSegmentNumber(segment);
    segId=AQFINTS_Segment_GetId(segment);
    segCode=AQFINTS_Segment_GetCode(segment);
    segVersion=AQFINTS_Segment_GetSegmentVersion(segment);
    runtimeFlags=AQFINTS_Segment_GetRuntimeFlags(segment);

    snprintf(lineBuffer, sizeof(lineBuffer)-1,
             "%3d: Code=%s, id=%s, version=%d, parsed=%s\n",
             segNumber,
             segCode?segCode:"<empty>",
             segId?segId:"<empty>",
             segVersion,
             (runtimeFlags & AQFINTS_SEGMENT_RTFLAGS_PARSED)?"yes":"no");
    lineBuffer[sizeof(lineBuffer)-1]=0;
    GWEN_Buffer_AppendString(bufSegmentList, lineBuffer);

    segment=AQFINTS_Segment_List_Next(segment);
  }

  return 0;
}




