/***************************************************************************
 begin       : Sun Aug 04 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "s_pintan_sign.h"

#include "aqfints/sessionlayer/session.h"
#include "aqfints/msglayer/keyname.h"

#include <gwenhywfar/debug.h>

#include <time.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


static int createCtrlRef(char *ptrBuf, size_t lenBuf);
static int createSigHead(AQFINTS_SESSION *sess,
                         int segNum,
                         const AQFINTS_KEYNAME *keyName,
                         const char *ctrlRef,
                         GWEN_BUFFER *destBuffer);
static int createSigTail(AQFINTS_SESSION *sess,
                         int segNum,
                         const AQFINTS_KEYNAME *keyName,
                         const char *ctrlRef,
                         GWEN_BUFFER *destBuffer,
                         const char *pin);

static int prepareSignSeg(AQFINTS_SESSION *sess,
                          const AQFINTS_KEYNAME *keyName,
                          const char *ctrlRef,
                          GWEN_DB_NODE *cfg);




/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AQFINTS_SessionPinTan_WWrapSignatures(AQFINTS_SESSION *sess,
                                          AQFINTS_KEYNAME_LIST *keyNameList,
                                          int firstSegNum,
                                          int lastSegNum,
                                          GWEN_BUFFER *msgBuffer)
{
  AQFINTS_KEYNAME *keyName;
  GWEN_BUFFER *sigHeadBuffer;
  GWEN_BUFFER *sigTailBuffer;

  DBG_INFO(0, "Wrapping signatures");
  sigHeadBuffer=GWEN_Buffer_new(0, 512, 0, 1);
  GWEN_Buffer_ReserveBytes(sigHeadBuffer, 256);
  sigTailBuffer=GWEN_Buffer_new(0, 256, 0, 1);

  keyName=AQFINTS_KeyName_List_First(keyNameList);
  while (keyName) {
    const char *sUserId;
    int rv;

    sUserId=AQFINTS_KeyName_GetUserId(keyName);

    DBG_INFO(0, "User [%s]: Filling out keyname", sUserId?sUserId:"<empty>");
    rv=AQFINTS_Session_FilloutKeyname(sess, keyName);
    if (rv<0) {
      DBG_INFO(0, "here (%d)", rv);
      GWEN_Buffer_free(sigTailBuffer);
      GWEN_Buffer_free(sigHeadBuffer);
      return rv;
    }

    /* TODO: determine pin and tan */
    DBG_INFO(0, "User [%s]: Adding signature head and tail", sUserId?sUserId:"<empty>");
    rv=wrapSignature(sess, keyName, firstSegNum, lastSegNum, msgBuffer, sigHeadBuffer, sigTailBuffer);
    if (rv<0) {
      DBG_INFO(0, "here (%d)", rv);
      GWEN_Buffer_free(sigTailBuffer);
      GWEN_Buffer_free(sigHeadBuffer);
      return rv;
    }

    /* next */
    firstSegNum--;
    lastSegNum++;
    keyName=AQFINTS_KeyName_List_Next(keyName);
  }

  /* combine message with signature heads and tails */
  DBG_INFO(0, "Combining message with signature head and tail");
  GWEN_Buffer_Rewind(msgBuffer);
  GWEN_Buffer_InsertBuffer(msgBuffer, sigHeadBuffer);
  GWEN_Buffer_SetPos(msgBuffer, GWEN_Buffer_GetUsedBytes(msgBuffer));
  GWEN_Buffer_AppendBuffer(msgBuffer, sigTailBuffer);

  GWEN_Buffer_free(sigTailBuffer);
  GWEN_Buffer_free(sigHeadBuffer);
  return 0;
}





int wrapSignature(AQFINTS_SESSION *sess,
                  const AQFINTS_KEYNAME *keyName,
                  int firstSegNum,
                  int lastSegNum,
                  GWEN_BUFFER *msgBuffer,
                  GWEN_BUFFER *sigHeadBuffer,
                  GWEN_BUFFER *sigTailBuffer)
{
  int rv;
  char ctrlref[15];
  GWEN_BUFFER *tmpBuffer;

  rv=createCtrlRef(ctrlref, sizeof(ctrlref));
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    return rv;
  }

  /* signature head */
  tmpBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  rv=createSigHead(sess, firstSegNum-1, keyName, ctrlref, tmpBuffer);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    GWEN_Buffer_free(tmpBuffer);
    return rv;
  }
  GWEN_Buffer_Rewind(sigHeadBuffer);
  GWEN_Buffer_InsertBuffer(sigHeadBuffer, tmpBuffer); /* insert! */
  GWEN_Buffer_free(tmpBuffer);

  /* signature tail */
  tmpBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  rv=createSigTail(sess, lastSegNum+1, keyName, ctrlref, tmpBuffer, NULL); /* TODO: add pin */
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    GWEN_Buffer_free(tmpBuffer);
    return rv;
  }
  GWEN_Buffer_AppendBuffer(sigHeadBuffer, tmpBuffer); /* append! */
  GWEN_Buffer_free(tmpBuffer);

  return 0;
}



int createCtrlRef(char *ptrBuf, size_t lenBuf)
{

  struct tm *lt;
  time_t tt;

  tt=time(0);
  lt=localtime(&tt);

  if (!strftime(ptrBuf, lenBuf, "%Y%m%d%H%M%S", lt)) {
    DBG_INFO(0, "CtrlRef string too long");
    return GWEN_ERROR_INTERNAL;
  }

  return 0;
}



int createSigHead(AQFINTS_SESSION *sess,
                  int segNum,
                  const AQFINTS_KEYNAME *keyName,
                  const char *ctrlRef,
                  GWEN_BUFFER *destBuffer)
{
  GWEN_DB_NODE *dbSegment;
  AQFINTS_PARSER *parser;
  int hbciVersion;
  AQFINTS_SEGMENT *defSegment;
  AQFINTS_SEGMENT *segment;
  int rv;

  parser=AQFINTS_Session_GetParser(sess);
  hbciVersion=AQFINTS_Session_GetHbciVersion(sess);

  /* HNSHA */
  defSegment=AQFINTS_Parser_FindSegmentHighestVersionForProto(parser, "HNSHK", hbciVersion);
  if (defSegment==NULL) {
    DBG_ERROR(0, "No matching definition segment found for HNSHK (proto=%d)", hbciVersion);
    return GWEN_ERROR_INTERNAL;
  }

  segment=AQFINTS_Segment_new();
  AQFINTS_Segment_copy(segment, defSegment);
  AQFINTS_Segment_SetSegmentNumber(segment, segNum);
  dbSegment=GWEN_DB_Group_new("sigHead");
  AQFINTS_Segment_SetDbData(segment, dbSegment);

  rv=prepareSignSeg(sess, keyName, ctrlRef, dbSegment);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    return rv;
  }

  rv=AQFINTS_Session_WriteSegment(sess, segment, destBuffer);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    return rv;
  }

  return rv;
}



int createSigTail(AQFINTS_SESSION *sess,
                  int segNum,
                  const AQFINTS_KEYNAME *keyName,
                  const char *ctrlRef,
                  GWEN_BUFFER *destBuffer,
                  const char *pin)
{
  GWEN_DB_NODE *dbSegment;
  AQFINTS_PARSER *parser;
  int hbciVersion;
  AQFINTS_SEGMENT *defSegment;
  AQFINTS_SEGMENT *segment;
  int rv;
  const char *tan;

  parser=AQFINTS_Session_GetParser(sess);
  hbciVersion=AQFINTS_Session_GetHbciVersion(sess);

  tan=AQFINTS_KeyName_GetTan(keyName);

  /* HNSHA */
  defSegment=AQFINTS_Parser_FindSegmentHighestVersionForProto(parser, "HNSHA", hbciVersion);
  if (defSegment==NULL) {
    DBG_ERROR(0, "No matching definition segment found for HNSHA (proto=%d)", hbciVersion);
    return GWEN_ERROR_INTERNAL;
  }

  segment=AQFINTS_Segment_new();
  AQFINTS_Segment_copy(segment, defSegment);
  AQFINTS_Segment_SetSegmentNumber(segment, segNum);
  dbSegment=GWEN_DB_Group_new("sigTail");
  AQFINTS_Segment_SetDbData(segment, dbSegment);

  GWEN_DB_SetCharValue(dbSegment, GWEN_DB_FLAGS_DEFAULT, "ctrlref", ctrlRef);

  if (pin && *pin)
    GWEN_DB_SetCharValue(dbSegment, GWEN_DB_FLAGS_DEFAULT, "pin", pin);
  if (tan && *tan)
    GWEN_DB_SetCharValue(dbSegment, GWEN_DB_FLAGS_DEFAULT, "tan", tan);

  rv=AQFINTS_Session_WriteSegment(sess, segment, destBuffer);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    return rv;
  }

  return rv;
}



int prepareSignSeg(AQFINTS_SESSION *sess,
                   const AQFINTS_KEYNAME *keyName,
                   const char *ctrlRef,
                   GWEN_DB_NODE *cfg)
{
  char sdate[9];
  char stime[7];
  struct tm *lt;
  time_t tt;

  /* some preparations */
  tt=time(0);
  lt=localtime(&tt);

  /* create date */
  if (!strftime(sdate, sizeof(sdate), "%Y%m%d", lt)) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "Date string too long");
    return GWEN_ERROR_INTERNAL;
  }
  /* create time */
  if (!strftime(stime, sizeof(stime), "%H%M%S", lt)) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "Date string too long");
    return GWEN_ERROR_INTERNAL;
  }

  /* store info */
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "function", 2);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "ctrlref", ctrlRef);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "area", 1);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "role", 1);

  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecDetails/dir",
                      AQFINTS_Session_GetIsServer(sess)?2:1); /* 1 client, 2=server */
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecDetails/secId", AQFINTS_KeyName_GetSystemId(keyName));
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "signseq", AQFINTS_KeyName_GetSignatureCounter(keyName));

  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecStamp/date", sdate);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecStamp/time", stime);

  /* hashAlgo */
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "hashAlgo/purpose", 1);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "hashAlgo/algo", 999);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "hashAlgo/pname", 1);

  /* signAlgo */
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "signAlgo/purpose", 6);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "signAlgo/algo", 10);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "signAlgo/mode", 16);

  /* keyname */
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/country", AQFINTS_KeyName_GetCountry(keyName));
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/bankcode", AQFINTS_KeyName_GetBankCode(keyName));
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/userid", AQFINTS_KeyName_GetUserId(keyName));
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/keytype", AQFINTS_KeyName_GetKeyType(keyName));
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/keynum", AQFINTS_KeyName_GetKeyNumber(keyName));
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/keyversion", AQFINTS_KeyName_GetKeyVersion(keyName));

  /* security profile */
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "secProfile/code", AQFINTS_Session_GetSecProfileCode(sess));
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "secProfile/version", AQFINTS_Session_GetSecProfileVersion(sess));

  return 0;
}





