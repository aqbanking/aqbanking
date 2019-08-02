/***************************************************************************
 begin       : Thu Aug 01 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "./session_p.h"
#include "msglayer/parser/parser.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>


GWEN_INHERIT_FUNCTIONS(AQFINTS_SESSION)



AQFINTS_SESSION *AQFINTS_Session_new(AQFINTS_PARSER *parser, AQFINTS_TRANSPORT *trans)
{
  AQFINTS_SESSION *sess;

  GWEN_NEW_OBJECT(AQFINTS_SESSION, sess);
  sess->_refCount=1;
  GWEN_INHERIT_INIT(AQFINTS_SESSION, sess);
  sess->lastMessageNumSent=0;
  sess->lastMessageNumReceived=0;
  sess->hbciVersion=300;

  sess->parser=parser;

  return sess;
}



void AQFINTS_Session_free(AQFINTS_SESSION *sess)
{
  if (sess) {
    assert(sess->_refCount);
    if (sess->_refCount==1) {
      GWEN_INHERIT_FINI(AQFINTS_SESSION, sess)
      sess->_refCount=0;

      if (sess->dialogId)
        free(sess->dialogId);

      GWEN_FREE_OBJECT(sess);
    }
    else
      sess->_refCount--;
  }
}



void AQFINTS_Session_Attach(AQFINTS_SESSION *sess)
{
  assert(sess);
  assert(sess->_refCount);
  sess->_refCount++;
}



int AQFINTS_Session_GetHbciVersion(const AQFINTS_SESSION *sess)
{
  assert(sess);
  return sess->hbciVersion;
}



void AQFINTS_Session_SetHbciVersion(AQFINTS_SESSION *sess, int v)
{
  assert(sess);
  sess->hbciVersion=v;
}



const char *AQFINTS_Session_GetDialogId(const AQFINTS_SESSION *sess)
{
  assert(sess);
  return sess->dialogId;
}



void AQFINTS_Session_SetDialogId(AQFINTS_SESSION *sess, const char *s)
{
  assert(sess);
  if (sess->dialogId)
    free(sess->dialogId);
  if (s)
    sess->dialogId=strdup(s);
  else
    sess->dialogId=NULL;
}



int AQFINTS_Session_GetLastMessageNumSent(const AQFINTS_SESSION *sess)
{
  assert(sess);
  return sess->lastMessageNumSent;
}



int AQFINTS_Session_GetLastMessageNumReceived(const AQFINTS_SESSION *sess)
{
  assert(sess);
  return sess->lastMessageNumReceived;
}



AQFINTS_PARSER *AQFINTS_Session_GetParser(const AQFINTS_SESSION *sess)
{
  assert(sess);
  return sess->parser;
}



void AQFINTS_Session_SetLastMessageNumSent(AQFINTS_SESSION *sess, int p_src)
{
  assert(sess);
  sess->lastMessageNumSent=p_src;
}



void AQFINTS_Session_SetLastMessageNumReceived(AQFINTS_SESSION *sess, int p_src)
{
  assert(sess);
  sess->lastMessageNumReceived=p_src;
}



void AQFINTS_Session_SetParser(AQFINTS_SESSION *sess, AQFINTS_PARSER *p_src)
{
  assert(sess);
  sess->parser=p_src;
}



int AQFINTS_Session_ExchangeMessages(AQFINTS_SESSION *sess, AQFINTS_MESSAGE *messageOut,
                                     AQFINTS_MESSAGE **pMessageIn)
{
  assert(sess);
  if (sess->exchangeMessagesFn)
    return sess->exchangeMessagesFn(sess, messageOut, pMessageIn);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



AQFINTS_SESSION_EXCHANGEMESSAGES_FN AQFINTS_Session_SetExchangeMessagesFn(AQFINTS_SESSION *sess,
                                                                          AQFINTS_SESSION_EXCHANGEMESSAGES_FN fn)
{
  AQFINTS_SESSION_EXCHANGEMESSAGES_FN oldFn;

  assert(sess);
  oldFn=sess->exchangeMessagesFn;
  sess->exchangeMessagesFn=fn;
  return oldFn;
}



int AQFINTS_Session_WriteSegmentList(AQFINTS_SESSION *sess, AQFINTS_SEGMENT_LIST *segmentList,
                                     int firstSegNum, int refSegNum, GWEN_BUFFER *destBuffer)
{
  AQFINTS_SEGMENT *segment;
  int segNum=firstSegNum;

  segment=AQFINTS_Segment_List_First(segmentList);
  while(segment) {
    int rv;

    AQFINTS_Segment_SetSegmentNumber(segment, segNum++);
    AQFINTS_Segment_SetRefSegmentNumber(segment, refSegNum);

    rv=AQFINTS_Session_WriteSegment(sess, segment, destBuffer);
    if (rv<0) {
      DBG_INFO(0, "here (%d)", rv);
      return rv;
    }
    segment=AQFINTS_Segment_List_Next(segment);
  }

  return 0;
}



int AQFINTS_Session_WriteSegment(AQFINTS_SESSION *sess, AQFINTS_SEGMENT *segment, GWEN_BUFFER *destBuffer)
{
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbHead;
  const char *sCode;
  int segVersion;
  int segNum;
  int refSegNum;
  int rv;

  sCode=AQFINTS_Segment_GetCode(segment);
  segVersion=AQFINTS_Segment_GetSegmentVersion(segment);
  segNum=AQFINTS_Segment_GetSegmentNumber(segment);
  refSegNum=AQFINTS_Segment_GetRefSegmentNumber(segment);

  db=AQFINTS_Segment_GetDbData(segment);
  if (db==NULL) {
    DBG_ERROR(0, "Segment has no DB data");
    return GWEN_ERROR_INTERNAL;
  }

  /* prepare segment head */
  dbHead=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "head");
  assert(dbHead);
  GWEN_DB_SetCharValue(dbHead, GWEN_DB_FLAGS_OVERWRITE_VARS, "code", sCode);
  GWEN_DB_SetIntValue(dbHead, GWEN_DB_FLAGS_OVERWRITE_VARS, "seq", segNum);
  GWEN_DB_SetIntValue(dbHead, GWEN_DB_FLAGS_OVERWRITE_VARS, "version", segVersion);
  GWEN_DB_SetIntValue(dbHead, GWEN_DB_FLAGS_OVERWRITE_VARS, "ref", refSegNum);

  rv=AQFINTS_Parser_WriteSegment(sess->parser, segment, destBuffer);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AQFINTS_Session_CreateMessageHead(AQFINTS_SESSION *sess,
                                      int msgNum, int refMsgNum,
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
  dbSegment=GWEN_DB_Group_new("msgHead");
  AQFINTS_Segment_SetDbData(segment, dbSegment);

  /* temp */
  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "size", sizeOfMessageWithoutHead);
  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "hversion", hbciVersion);
  GWEN_DB_SetCharValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "dialogId", dialogId?dialogId:"0");
  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "msgnum", msgNum);
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




