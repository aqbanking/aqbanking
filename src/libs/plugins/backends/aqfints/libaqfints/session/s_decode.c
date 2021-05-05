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


#include "libaqfints/session/s_decode.h"
#include "libaqfints/session/session.h"
#include "libaqfints/session/s_decrypt.h"
#include "libaqfints/session/s_verify.h"
#include "libaqfints/session/s_log.h"

#include "libaqfints/parser/parser.h"
#include "libaqfints/parser/parser_dump.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

int _setMessageInfoFromHeadSegment(AQFINTS_MESSAGE *message, AQFINTS_SEGMENT_LIST *segmentList);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



AQFINTS_MESSAGE *AQFINTS_Session_DecodeMessage(AQFINTS_SESSION *sess, const uint8_t *ptrBuffer, uint32_t lenBuffer)
{
  AQFINTS_SEGMENT_LIST *segmentList;
  AQFINTS_PARSER *parser;
  AQFINTS_MESSAGE *message;
  int rv;

  message=AQFINTS_Message_new();

  parser=AQFINTS_Session_GetParser(sess);
  segmentList=AQFINTS_Message_GetSegmentList(message);

  /* parse HBCI message into a segment list */
  DBG_DEBUG(AQFINTS_LOGDOMAIN, "Reading message into segment list");
  rv=AQFINTS_Parser_ReadIntoSegmentList(parser, segmentList, ptrBuffer, lenBuffer);
  if (rv<0) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    AQFINTS_Message_free(message);
    return NULL;
  }

  /* interprete segment list and extract data */
  DBG_DEBUG(AQFINTS_LOGDOMAIN, "Reading segment list into dbs");
  rv=AQFINTS_Parser_ReadSegmentListToDb(parser, segmentList);
  if (rv<0) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    AQFINTS_Message_free(message);
    return NULL;
  }

  AQFINTS_Session_LogMessage(sess, ptrBuffer, lenBuffer, 1, 1); /* rec, crypt */

#if 0
  DBG_ERROR(AQFINTS_LOGDOMAIN, "Received this segment list:");
  AQFINTS_Parser_DumpSegmentList(segmentList, 2);
#endif

  /* read basic message info from message head */
  DBG_INFO(AQFINTS_LOGDOMAIN, "Reading basic message info from db");
  rv=_setMessageInfoFromHeadSegment(message, segmentList);
  if (rv<0) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    AQFINTS_Message_free(message);
    return NULL;
  }


  /* decrypt message, if necessary */
  DBG_INFO(AQFINTS_LOGDOMAIN, "Decrypting message");
  rv=AQFINTS_Session_DecryptMessage(sess, message);
  if (rv<0) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    AQFINTS_Message_free(message);
    return NULL;
  }

  /* verify signatures, if any */
  DBG_INFO(AQFINTS_LOGDOMAIN, "Verifying signatures");
  rv=AQFINTS_Session_VerifyMessage(sess, message);
  if (rv<0) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    AQFINTS_Message_free(message);
    return NULL;
  }

  DBG_INFO(AQFINTS_LOGDOMAIN, "Message decoded.");
  return message;
}



AQFINTS_KEYDESCR *AQFINTS_Session_ReadKeyDescrFromDbHead(GWEN_DB_NODE *dbHead)
{
  GWEN_DB_NODE *dbKey;
  GWEN_DB_NODE *dbSecDetails;
  GWEN_DB_NODE *dbSecProfile;
  AQFINTS_KEYDESCR *keyDescr;
  unsigned int len;
  const uint8_t *ptr;

  dbKey=GWEN_DB_GetGroup(dbHead, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "key");
  if (dbKey==NULL) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "No keyDescr in segment");
    GWEN_DB_Dump(dbHead, 2);
    return NULL;
  }
  dbSecDetails=GWEN_DB_GetGroup(dbHead, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "secDetails");
  if (dbSecDetails==NULL) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "No security details in segment");
    GWEN_DB_Dump(dbHead, 2);
    return NULL;
  }

  dbSecProfile=GWEN_DB_GetGroup(dbHead, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "secProfile");
  if (dbSecProfile==NULL) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "No security profile info in segment");
    GWEN_DB_Dump(dbHead, 2);
    return NULL;
  }

  /* now create key description */
  keyDescr=AQFINTS_KeyDescr_new();

  /* info from "Key" */
  AQFINTS_KeyDescr_SetUserId(keyDescr, GWEN_DB_GetCharValue(dbKey, "userId", 0, NULL));
  AQFINTS_KeyDescr_SetKeyType(keyDescr, GWEN_DB_GetCharValue(dbKey, "keyType", 0, NULL));
  AQFINTS_KeyDescr_SetKeyNumber(keyDescr, GWEN_DB_GetIntValue(dbKey, "keyNum", 0, 0));
  AQFINTS_KeyDescr_SetKeyVersion(keyDescr, GWEN_DB_GetIntValue(dbKey, "keyVersion", 0, 0));

  /* info from "SecDetails" */
  len=0;
  ptr=(const uint8_t *) GWEN_DB_GetBinValue(dbSecDetails, "cid", 0, NULL, 0, &len);
  if (ptr && len>0) {
    uint8_t *ptrCopy;

    ptrCopy=(uint8_t *) malloc(len);
    assert(ptrCopy);
    memmove(ptrCopy, ptr, len);
    AQFINTS_KeyDescr_SetCid(keyDescr, ptrCopy, len);
  }
  AQFINTS_KeyDescr_SetSystemId(keyDescr, GWEN_DB_GetCharValue(dbSecDetails, "secId", 0, NULL));

  /* info from security profile */
  AQFINTS_KeyDescr_SetSecurityProfileName(keyDescr, GWEN_DB_GetCharValue(dbSecProfile, "code", 0, NULL));
  AQFINTS_KeyDescr_SetSecurityProfileVersion(keyDescr, GWEN_DB_GetIntValue(dbSecProfile, "version", 0, 0));

  /* done */
  return keyDescr;
}



int _setMessageInfoFromHeadSegment(AQFINTS_MESSAGE *message, AQFINTS_SEGMENT_LIST *segmentList)
{
  AQFINTS_SEGMENT *segment;
  const char *sCode;
  GWEN_DB_NODE *dbMsgHead;

  segment=AQFINTS_Segment_List_First(segmentList);
  if (segment==NULL) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "No segment in message");
    return GWEN_ERROR_NO_DATA;
  }

  sCode=AQFINTS_Segment_GetCode(segment);
  if (sCode==NULL || *sCode==0 || strcasecmp(sCode, "HNHBK")) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "Message does not start with message head (%s)", sCode);
    return GWEN_ERROR_INVALID;
  }

  dbMsgHead=AQFINTS_Segment_GetDbData(segment);
  if (dbMsgHead==NULL) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "Segment has no DB data");
    return GWEN_ERROR_BAD_DATA;
  }

  AQFINTS_Message_SetMessageNumber(message, GWEN_DB_GetIntValue(dbMsgHead, "msgnum", 0, 0));
  AQFINTS_Message_SetRefMessageNumber(message, GWEN_DB_GetIntValue(dbMsgHead, "msgref", 0, 0));
  AQFINTS_Message_SetDialogId(message, GWEN_DB_GetCharValue(dbMsgHead, "dialogId", 0, NULL));
  AQFINTS_Message_SetHbciVersion(message, GWEN_DB_GetIntValue(dbMsgHead, "hversion", 0, 0));

  return 0;
}





