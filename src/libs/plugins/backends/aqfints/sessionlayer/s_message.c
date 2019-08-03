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


#include "./session.h"

#include "msglayer/parser/parser.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>


/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int createMessageHead(AQFINTS_SESSION *sess, int msgNum, int refMsgNum,
                             int sizeOfMessageWithoutHead,
                             GWEN_BUFFER *destBuffer);
static int createMessageTail(AQFINTS_SESSION *sess, int msgNum, int segNum, GWEN_BUFFER *destBuffer);




/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AQFINTS_Session_WrapMessageHeadAndTail(AQFINTS_SESSION *sess,
                                           int msgNum, int refMsgNum, int lastSegNum,
                                           GWEN_BUFFER *msgBuffer)
{
  GWEN_BUFFER *headBuf;
  int rv;
  int msgSizeWithoutHead;

  /* append msg tail */
  rv=createMessageTail(sess, msgNum, lastSegNum+1, msgBuffer);
  if (rv<0) {
    DBG_ERROR(0, "here (%d)", rv);
    return rv;
  }

  /* take msg tail into account */
  msgSizeWithoutHead=GWEN_Buffer_GetUsedBytes(msgBuffer);

  headBuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=createMessageHead(sess, msgNum, refMsgNum, msgSizeWithoutHead, headBuf);
  if (rv<0) {
    DBG_ERROR(0, "here (%d)", rv);
    GWEN_Buffer_free(headBuf);
    return rv;
  }

  GWEN_Buffer_Rewind(msgBuffer);
  GWEN_Buffer_InsertBytes(msgBuffer, GWEN_Buffer_GetStart(headBuf), GWEN_Buffer_GetUsedBytes(headBuf));
  GWEN_Buffer_SetPos(msgBuffer, GWEN_Buffer_GetUsedBytes(msgBuffer));
  GWEN_Buffer_free(headBuf);
  return 0;
}




int createMessageHead(AQFINTS_SESSION *sess, int msgNum, int refMsgNum,
                      int sizeOfMessageWithoutHead,
                      GWEN_BUFFER *destBuffer)
{
  AQFINTS_PARSER *parser;
  int hbciVersion;
  AQFINTS_SEGMENT *defSegment;
  AQFINTS_SEGMENT *segment;
  GWEN_DB_NODE *dbSegment;
  const char *dialogId;
  GWEN_BUFFER *tmpBuffer;
  int rv;

  parser=AQFINTS_Session_GetParser(sess);
  hbciVersion=AQFINTS_Session_GetHbciVersion(sess);

  dialogId=AQFINTS_Session_GetDialogId(sess);

  /* HNHBK */
  defSegment=AQFINTS_Parser_FindSegmentHighestVersionForProto(parser, "HNHBK", hbciVersion);
  if (defSegment==NULL) {
    DBG_ERROR(0, "No matching definition segment found for HNHBK (proto=%d)", hbciVersion);
    return GWEN_ERROR_INTERNAL;
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
  if (refMsgNum)
    GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "msgref", refMsgNum);

  /* create temporary version to determine the full message size */
  tmpBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  rv=AQFINTS_Session_WriteSegment(sess, segment, tmpBuffer);
  if (rv<0) {
    DBG_ERROR(0, "here (%d)", rv);
    GWEN_Buffer_free(tmpBuffer);
    AQFINTS_Segment_free(segment);
    return rv;
  }

  /* reset segment */
  AQFINTS_Segment_SetElements(segment, NULL);

  /* finally write the message header */
  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "size", sizeOfMessageWithoutHead+GWEN_Buffer_GetUsedBytes(tmpBuffer));
  rv=AQFINTS_Session_WriteSegment(sess, segment, destBuffer);
  if (rv<0) {
    DBG_ERROR(0, "here (%d)", rv);
    GWEN_Buffer_free(tmpBuffer);
    AQFINTS_Segment_free(segment);
    return rv;
  }
  GWEN_Buffer_free(tmpBuffer);
  AQFINTS_Segment_free(segment);

  return 0;
}



int createMessageTail(AQFINTS_SESSION *sess, int msgNum, int segNum, GWEN_BUFFER *destBuffer)
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
    DBG_ERROR(0, "No matching definition segment found for HNHBS (proto=%d)", hbciVersion);
    return GWEN_ERROR_INTERNAL;
  }
  segment=AQFINTS_Segment_new();
  AQFINTS_Segment_copy(segment, defSegment);
  AQFINTS_Segment_SetSegmentNumber(segment, segNum);

  dbSegment=GWEN_DB_Group_new("msgTail");
  AQFINTS_Segment_SetDbData(segment, dbSegment);

  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "msgnum", msgNum);

  rv=AQFINTS_Session_WriteSegment(sess, segment, destBuffer);
  if (rv<0) {
    DBG_ERROR(0, "here (%d)", rv);
    AQFINTS_Segment_free(segment);
    return rv;
  }
  AQFINTS_Segment_free(segment);

  return 0;
}



