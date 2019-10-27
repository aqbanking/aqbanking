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


#include "./session.h"

#include "servicelayer/bpd/bpd_read.h"

#include <gwenhywfar/debug.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static AQFINTS_MESSAGE *createMessage(AQFINTS_SESSION *sess, const char *bankCode);
static int mkGetAnonBpdMessage(AQFINTS_SESSION *sess, const char *bankCode, GWEN_BUFFER *destBuffer);
static AQFINTS_BPD *extractBpd(AQFINTS_SESSION *sess, const uint8_t *ptrBuffer, uint32_t lenBuffer);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AQFINTS_Session_GetAnonBpd(AQFINTS_SESSION *sess, const char *bankCode, AQFINTS_BPD **pBpd)
{
  GWEN_BUFFER *destBuffer;
  int rv;
  AQFINTS_BPD *bpd;

  destBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  rv=mkGetAnonBpdMessage(sess, bankCode, destBuffer);
  if (rv<0) {
    DBG_ERROR(0, "here (%d)", rv);
    GWEN_Buffer_free(destBuffer);
    return rv;
  }

  rv=AQFINTS_Session_Connect(sess);
  if (rv<0) {
    DBG_ERROR(0, "here (%d)", rv);
    GWEN_Buffer_free(destBuffer);
    return rv;
  }

  rv=AQFINTS_Session_SendMessage(sess,
                                 GWEN_Buffer_GetStart(destBuffer),
                                 GWEN_Buffer_GetUsedBytes(destBuffer));
  if (rv<0) {
    DBG_ERROR(0, "here (%d)", rv);
    AQFINTS_Session_Disconnect(sess);
    GWEN_Buffer_free(destBuffer);
    return rv;
  }

  GWEN_Buffer_Reset(destBuffer);

  rv=AQFINTS_Session_ReceiveMessage(sess, destBuffer);
  if (rv<0) {
    DBG_ERROR(0, "here (%d)", rv);
    AQFINTS_Session_Disconnect(sess);
    GWEN_Buffer_free(destBuffer);
    return rv;
  }

  AQFINTS_Session_Disconnect(sess);

  bpd=extractBpd(sess,
                 (const uint8_t *) GWEN_Buffer_GetStart(destBuffer),
                 GWEN_Buffer_GetUsedBytes(destBuffer));
  if (bpd==NULL) {
    DBG_ERROR(0, "No BPD extracted");
    GWEN_Buffer_free(destBuffer);
    return rv;
  }

  GWEN_Buffer_free(destBuffer);

  *pBpd=bpd;
  return 0;
}



AQFINTS_BPD *extractBpd(AQFINTS_SESSION *sess, const uint8_t *ptrBuffer, uint32_t lenBuffer)
{
  AQFINTS_SEGMENT_LIST *segmentList;
  AQFINTS_PARSER *parser;
  AQFINTS_BPD *bpd;
  int rv;

  parser=AQFINTS_Session_GetParser(sess);
  segmentList=AQFINTS_Segment_List_new();
  rv=AQFINTS_Parser_ReadIntoSegmentList(parser, segmentList, ptrBuffer, lenBuffer);
  if (rv<0) {
    DBG_ERROR(0, "here (%d)", rv);
    AQFINTS_Segment_List_free(segmentList);
    return NULL;
  }

  rv=AQFINTS_Parser_ReadSegmentListToDb(parser, segmentList);
  if (rv<0) {
    DBG_ERROR(0, "here (%d)", rv);
    AQFINTS_Segment_List_free(segmentList);
    return NULL;
  }

  bpd=AQFINTS_Session_ExtractBpdFromSegmentList(sess, segmentList);
  if (bpd==NULL) {
    DBG_ERROR(0, "Empty BPD");
    AQFINTS_Segment_List_free(segmentList);
    return NULL;
  }

  AQFINTS_Segment_List_free(segmentList);
  return bpd;
}





int mkGetAnonBpdMessage(AQFINTS_SESSION *sess, const char *bankCode, GWEN_BUFFER *destBuffer)
{
  AQFINTS_MESSAGE *message;
  AQFINTS_SEGMENT_LIST *segmentList;
  AQFINTS_SEGMENT *segment;
  GWEN_BUFFER *msgBuf;
  int rv;

  message=createMessage(sess, bankCode);
  if (message==NULL) {
    DBG_ERROR(0, "No message created");
    return GWEN_ERROR_INVALID;
  }

  segmentList=AQFINTS_Message_GetSegmentList(message);
  msgBuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_ReserveBytes(msgBuf, 128); /* reserve space to insert MSG head later */

  rv=AQFINTS_Session_WriteSegmentList(sess, segmentList, 2, 0, msgBuf);
  if (rv<0) {
    DBG_ERROR(0, "here (%d)", rv);
    GWEN_Buffer_free(msgBuf);
    AQFINTS_Message_free(message);
    return rv;
  }

  segment=AQFINTS_Segment_List_Last(segmentList);
  assert(segment);

  rv=AQFINTS_Session_WrapMessageHeadAndTail(sess,
                                            AQFINTS_Message_GetMessageNumber(message),
                                            AQFINTS_Message_GetRefMessageNumber(message),
                                            AQFINTS_Segment_GetSegmentNumber(segment),
                                            msgBuf);
  if (rv<0) {
    DBG_ERROR(0, "here (%d)", rv);
    GWEN_Buffer_free(msgBuf);
    AQFINTS_Message_free(message);
    return rv;
  }
  GWEN_Buffer_AppendBuffer(destBuffer, msgBuf);
  GWEN_Buffer_free(msgBuf);
  AQFINTS_Message_free(message);
  return 0;
}






AQFINTS_MESSAGE *createMessage(AQFINTS_SESSION *sess, const char *bankCode)
{
  AQFINTS_PARSER *parser;
  int hbciVersion;
  AQFINTS_MESSAGE *message;
  AQFINTS_SEGMENT *defSegment;
  AQFINTS_SEGMENT *segment;
  GWEN_DB_NODE *dbSegment;

  parser=AQFINTS_Session_GetParser(sess);
  hbciVersion=AQFINTS_Session_GetHbciVersion(sess);

  message=AQFINTS_Message_new();
  AQFINTS_Message_SetMessageNumber(message, AQFINTS_Session_GetLastMessageNumSent(sess)+1);

  /* ident */
  defSegment=AQFINTS_Parser_FindSegmentHighestVersionForProto(parser, "HKIDN", hbciVersion);
  if (defSegment==NULL) {
    DBG_ERROR(0, "No matching definition segment found for HKIDN (proto=%d)", hbciVersion);
    return NULL;
  }
  segment=AQFINTS_Segment_new();
  AQFINTS_Segment_copy(segment, defSegment);
  dbSegment=GWEN_DB_Group_new("ident");
  AQFINTS_Segment_SetDbData(segment, dbSegment);
  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "country", 280);
  GWEN_DB_SetCharValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "bankCode", bankCode);
  GWEN_DB_SetCharValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "customerId", "9999999999");
  GWEN_DB_SetCharValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "systemId", "0");
  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "status", 0);
  AQFINTS_Message_AddSegment(message, segment);

  /* prepare */
  defSegment=AQFINTS_Parser_FindSegmentHighestVersionForProto(parser, "HKVVB", hbciVersion);
  if (defSegment==NULL) {
    DBG_ERROR(0, "No matching definition segment found for HKVVB (proto=%d)", hbciVersion);
    return NULL;
  }
  segment=AQFINTS_Segment_new();
  AQFINTS_Segment_copy(segment, defSegment);
  dbSegment=GWEN_DB_Group_new("ident");
  AQFINTS_Segment_SetDbData(segment, dbSegment);
  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "bpdVersion", 0);
  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "updVersion", 0);
  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "lang", 1);
  GWEN_DB_SetCharValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "product", "AqBanking");
  GWEN_DB_SetCharValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "version", "5.99");
  AQFINTS_Message_AddSegment(message, segment);

  return message;
}




