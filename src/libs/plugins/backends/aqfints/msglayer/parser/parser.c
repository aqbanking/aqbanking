/***************************************************************************
 begin       : Sun Jun 23 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "parser_p.h"

#include "parser_xml.h"
#include "parser_hbci.h"
#include "parser_normalize.h"
#include "parser_dbread.h"


#include <gwenhywfar/debug.h>
#include <gwenhywfar/stringlist.h>
#include <gwenhywfar/directory.h>




AQFINTS_PARSER *AQFINTS_Parser_new()
{
  AQFINTS_PARSER *parser;

  GWEN_NEW_OBJECT(AQFINTS_PARSER, parser);

  parser->jobDefList=AQFINTS_JobDef_List_new();
  parser->segmentList=AQFINTS_Segment_List_new();
  parser->pathList=GWEN_StringList_new();

  return parser;
}



void AQFINTS_Parser_free(AQFINTS_PARSER *parser)
{
  if (parser) {
    GWEN_StringList_free(parser->pathList);
    AQFINTS_Segment_List_free(parser->segmentList);
    AQFINTS_JobDef_List_free(parser->jobDefList);

    GWEN_FREE_OBJECT(parser);
  }
}



void AQFINTS_Parser_AddPath(AQFINTS_PARSER *parser, const char *path)
{
  assert(parser);
  assert(path);
  GWEN_StringList_AppendString(parser->pathList, path, 0, 1);
}



int AQFINTS_Parser_ReadFiles(AQFINTS_PARSER *parser)
{
  GWEN_STRINGLIST *slFiles;
  GWEN_STRINGLISTENTRY *slEntry;
  AQFINTS_ELEMENT *groupTree;
  int filesLoaded=0;

  slFiles=GWEN_StringList_new();
  groupTree=AQFINTS_Element_new();

  /* sample file names */
  slEntry=GWEN_StringList_FirstEntry(parser->pathList);
  while(slEntry) {
    const char *s;

    s=GWEN_StringListEntry_Data(slEntry);
    if (s && *s) {
      int rv;

      rv=GWEN_Directory_GetMatchingFilesRecursively(s, slFiles, "*.fints");
      if (rv<0) {
        DBG_ERROR(0, "Error reading file names from \"%s\", ignoring", s);
      }
    }
    slEntry=GWEN_StringListEntry_Next(slEntry);
  }

  /* check whether we have files to load */
  if (GWEN_StringList_Count(slFiles)<1) {
    DBG_ERROR(0, "No files found to load");
    GWEN_StringList_free(slFiles);
    AQFINTS_Element_free(groupTree);
    return GWEN_ERROR_GENERIC;
  }

  /* load files */
  slEntry=GWEN_StringList_FirstEntry(slFiles);
  while(slEntry) {
    const char *s;

    s=GWEN_StringListEntry_Data(slEntry);
    if (s && *s) {
      int rv;

      rv=AQFINTS_Parser_Xml_ReadFile(parser->jobDefList, parser->segmentList, groupTree, s);
      if (rv<0) {
        DBG_ERROR(0, "Error reading file \"%s\" (%d), ignoring", s, rv);
      }
      else
        filesLoaded++;
    }
    slEntry=GWEN_StringListEntry_Next(slEntry);
  }
  if (filesLoaded<1) {
    DBG_ERROR(0, "No files loaded");
    GWEN_StringList_free(slFiles);
    AQFINTS_Element_free(groupTree);
    return GWEN_ERROR_GENERIC;
  }

  /* post-process files */
  AQFINTS_Parser_SegmentList_ResolveGroups(parser->segmentList, groupTree);
  AQFINTS_Parser_SegmentList_Normalize(parser->segmentList);

  /* cleanup */
  GWEN_StringList_free(slFiles);
  AQFINTS_Element_free(groupTree);
  return 0;
}




AQFINTS_SEGMENT *AQFINTS_Parser_FindSegmentByCode(const AQFINTS_PARSER *parser, const char *id, int segmentVersion, int protocolVersion)
{
  AQFINTS_SEGMENT *segment;

  segment=AQFINTS_Segment_List_First(parser->segmentList);
  while(segment) {
    if ((segmentVersion==0 || segmentVersion==AQFINTS_Segment_GetSegmentVersion(segment)) &&
        (protocolVersion==0 || protocolVersion==AQFINTS_Segment_GetProtocolVersion(segment))){
      if (!(id && *id))
        return segment;
      else {
        const char *s;

        s=AQFINTS_Segment_GetCode(segment);
        if (s && *s && strcasecmp(s, id)==0)
          return segment;
      }
    }
    segment=AQFINTS_Segment_List_Next(segment);
  }

  return NULL;
}



AQFINTS_SEGMENT *AQFINTS_Parser_FindSegmentById(const AQFINTS_PARSER *parser, const char *id, int segmentVersion, int protocolVersion)
{
  AQFINTS_SEGMENT *segment;

  segment=AQFINTS_Segment_List_First(parser->segmentList);
  while(segment) {
    if ((segmentVersion==0 || segmentVersion==AQFINTS_Segment_GetSegmentVersion(segment)) &&
        (protocolVersion==0 || protocolVersion==AQFINTS_Segment_GetProtocolVersion(segment))){
      if (!(id && *id))
        return segment;
      else {
        const char *s;

        s=AQFINTS_Segment_GetId(segment);
        if (s && *s && strcasecmp(s, id)==0)
          return segment;
      }
    }
    segment=AQFINTS_Segment_List_Next(segment);
  }

  return NULL;
}



AQFINTS_JOBDEF *AQFINTS_Parser_FindJobDefByCode(const AQFINTS_PARSER *parser, const char *id, int jobVersion, int protocolVersion)
{
  AQFINTS_JOBDEF *jobDef;

  jobDef=AQFINTS_JobDef_List_First(parser->jobDefList);
  while(jobDef) {
    if ((jobVersion==0 || jobVersion==AQFINTS_JobDef_GetJobVersion(jobDef)) &&
        (protocolVersion==0 || protocolVersion==AQFINTS_JobDef_GetProtocolVersion(jobDef))){
      if (!(id && *id))
        return jobDef;
      else {
        const char *s;

        s=AQFINTS_JobDef_GetCode(jobDef);
        if (s && *s && strcasecmp(s, id)==0)
          return jobDef;
      }
    }
    jobDef=AQFINTS_JobDef_List_Next(jobDef);
  }

  return NULL;
}



AQFINTS_JOBDEF *AQFINTS_Parser_FindJobDefById(const AQFINTS_PARSER *parser, const char *id, int jobVersion, int protocolVersion)
{
  AQFINTS_JOBDEF *jobDef;

  jobDef=AQFINTS_JobDef_List_First(parser->jobDefList);
  while(jobDef) {
    if ((jobVersion==0 || jobVersion==AQFINTS_JobDef_GetJobVersion(jobDef)) &&
        (protocolVersion==0 || protocolVersion==AQFINTS_JobDef_GetProtocolVersion(jobDef))){
      if (!(id && *id))
        return jobDef;
      else {
        const char *s;

        s=AQFINTS_JobDef_GetId(jobDef);
        if (s && *s && strcasecmp(s, id)==0)
          return jobDef;
      }
    }
    jobDef=AQFINTS_JobDef_List_Next(jobDef);
  }

  return NULL;
}



int AQFINTS_Parser_ReadIntoDb(AQFINTS_PARSER *parser,
                              const uint8_t *ptrBuf,
                              uint32_t lenBuf,
                              GWEN_DB_NODE *db)
{
  AQFINTS_SEGMENT_LIST *segmentList;
  int rv;

  segmentList=AQFINTS_Segment_List_new();

  rv=AQFINTS_Parser_ReadIntoSegmentList(parser, segmentList, ptrBuf, lenBuf);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    AQFINTS_Segment_List_free(segmentList);
    return rv;
  }

  rv=AQFINTS_Parser_ReadSegmentListToDb(parser, segmentList, db);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    AQFINTS_Segment_List_free(segmentList);
    return rv;
  }

  AQFINTS_Segment_List_free(segmentList);
  return 0;
}




int AQFINTS_Parser_ReadIntoSegmentList(AQFINTS_PARSER *parser,
                                       AQFINTS_SEGMENT_LIST *targetSegmentList,
                                       const uint8_t *ptrBuf,
                                       uint32_t lenBuf)
{
  int rv;

  rv=AQFINTS_Parser_Hbci_ReadBuffer(targetSegmentList, ptrBuf, lenBuf);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AQFINTS_Parser_ReadSegmentListToDb(AQFINTS_PARSER *parser,
                                       AQFINTS_SEGMENT_LIST *segmentList,
                                       GWEN_DB_NODE *db)
{
  AQFINTS_SEGMENT *segment;
  int segmentsRead=0;

  segment=AQFINTS_Segment_List_First(segmentList);
  while(segment) {
    const char *sCode;
    int segmentVersion;

    sCode=AQFINTS_Segment_GetCode(segment);
    segmentVersion=AQFINTS_Segment_GetSegmentVersion(segment);
    if (sCode && *sCode && segmentVersion>0) {
      AQFINTS_SEGMENT *defSegment;

      /* TODO: set protocol version somehow */
      defSegment=AQFINTS_Parser_FindSegmentByCode(parser, sCode, segmentVersion, 0);
      if (defSegment) {
        int rv;

        rv=AQFINTS_Parser_Db_ReadSegment(defSegment, segment, db);
        if (rv<0) {
          DBG_ERROR(0, "Error reading segment \"%s\" (version %d) into DB (%d)", sCode, segmentVersion, rv);
          return rv;
        }
        AQFINTS_Segment_AddRuntimeFlags(segment, AQFINTS_SEGMENT_RTFLAGS_PARSED);
        segmentsRead++;
      }
      else {
        DBG_ERROR(0, "Segment \"%s\" (version %d) not found, ignoring", sCode, segmentVersion);
      }
    }
    else {
      DBG_ERROR(0, "Unnamed segment, ignoring");
    }

    segment=AQFINTS_Segment_List_Next(segment);
  }

  if (segmentsRead<1) {
    DBG_ERROR(0, "No segment read into DB");
    return GWEN_ERROR_GENERIC;
  }

  return 0;
}






