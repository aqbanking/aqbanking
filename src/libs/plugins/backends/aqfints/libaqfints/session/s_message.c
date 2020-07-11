/***************************************************************************
 begin       : Sat Aug 03 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "s_message.h"
#include "parser/parser.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>


/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static AQFINTS_SEGMENT *createMessageHead(AQFINTS_SESSION *sess,
                                          const char *dialogId,
                                          int msgNum, int refMsgNum,
                                          int sizeOfMessageWithoutHead);
static AQFINTS_SEGMENT *createMessageTail(AQFINTS_SESSION *sess, int msgNum, int segNum);




/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AQFINTS_Session_WrapMessageHeadAndTail(AQFINTS_SESSION *sess,
                                           AQFINTS_SEGMENT_LIST *segmentList,
                                           const char *dialogId,
                                           int msgNum, int refMsgNum, int lastSegNum)
{
  int msgSizeWithoutHead;
  AQFINTS_SEGMENT *segment;

  /* create and append msg tail */
  segment=createMessageTail(sess, msgNum, lastSegNum+1);
  if (segment==NULL) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "here");
    return GWEN_ERROR_INTERNAL;
  }
  AQFINTS_Segment_List_Add(segment, segmentList);

  /* take msg tail into account */
  msgSizeWithoutHead=AQFINTS_Segment_List_SampleSizes(segmentList);

  /* create and insert msg tail */
  segment=createMessageHead(sess, dialogId, msgNum, refMsgNum, msgSizeWithoutHead);
  if (segment==NULL) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "here");
    return GWEN_ERROR_INTERNAL;
  }
  AQFINTS_Segment_List_Insert(segment, segmentList);

  return 0;
}




AQFINTS_SEGMENT *createMessageHead(AQFINTS_SESSION *sess,
                                   const char *dialogId,
                                   int msgNum, int refMsgNum, int sizeOfMessageWithoutHead)
{
  AQFINTS_PARSER *parser;
  int hbciVersion;
  AQFINTS_SEGMENT *defSegment;
  AQFINTS_SEGMENT *segment;
  GWEN_DB_NODE *dbSegment;
  uint32_t segSize;
  int rv;

  parser=AQFINTS_Session_GetParser(sess);
  hbciVersion=AQFINTS_Session_GetHbciVersion(sess);

  /* HNHBK */
  defSegment=AQFINTS_Parser_FindSegmentHighestVersionForProto(parser, "HNHBK", hbciVersion);
  if (defSegment==NULL) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "No matching definition segment found for HNHBK (proto=%d)", hbciVersion);
    return NULL;
  }

  segment=AQFINTS_Segment_new();
  AQFINTS_Segment_copy(segment, defSegment);
  AQFINTS_Segment_SetSegmentNumber(segment, 1);
  dbSegment=GWEN_DB_Group_new("msgHead");
  AQFINTS_Segment_SetDbData(segment, dbSegment);

  /* temp */
  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "size", sizeOfMessageWithoutHead);
  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "hversion", hbciVersion);
  GWEN_DB_SetCharValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "dialogId", dialogId?dialogId:"0");
  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "msgnum", msgNum);
  if (refMsgNum) {
    GWEN_DB_SetCharValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "msgref/dialogId", dialogId?dialogId:"0");
    GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "msgref/msgnum", refMsgNum);
  }

  /* create temporary version to determine the full message size */
  rv=AQFINTS_Session_WriteSegment(sess, segment);
  if (rv<0) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    AQFINTS_Segment_free(segment);
    return NULL;
  }
  segSize=AQFINTS_Segment_GetDataLength(segment);

  /* reset segment */
  AQFINTS_Segment_SetElements(segment, NULL);
  AQFINTS_Segment_SetData(segment, NULL, 0);

  /* finally write the message header */
  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "size", sizeOfMessageWithoutHead+segSize);
  rv=AQFINTS_Session_WriteSegment(sess, segment);
  if (rv<0) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    AQFINTS_Segment_free(segment);
    return NULL;
  }

  return segment;
}



AQFINTS_SEGMENT *createMessageTail(AQFINTS_SESSION *sess, int msgNum, int segNum)
{
  AQFINTS_PARSER *parser;
  int hbciVersion;
  AQFINTS_SEGMENT *defSegment;
  AQFINTS_SEGMENT *segment;
  GWEN_DB_NODE *dbSegment;
  int rv;

  parser=AQFINTS_Session_GetParser(sess);
  hbciVersion=AQFINTS_Session_GetHbciVersion(sess);

  /* HNHBS */
  defSegment=AQFINTS_Parser_FindSegmentHighestVersionForProto(parser, "HNHBS", hbciVersion);
  if (defSegment==NULL) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "No matching definition segment found for HNHBS (proto=%d)", hbciVersion);
    return NULL;
  }

  segment=AQFINTS_Segment_new();
  AQFINTS_Segment_copy(segment, defSegment);
  AQFINTS_Segment_SetSegmentNumber(segment, segNum);

  dbSegment=GWEN_DB_Group_new("msgTail");
  AQFINTS_Segment_SetDbData(segment, dbSegment);

  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "msgnum", msgNum);

  rv=AQFINTS_Session_WriteSegment(sess, segment);
  if (rv<0) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    AQFINTS_Segment_free(segment);
    return NULL;
  }

  return segment;
}



