/***************************************************************************
 begin       : Sun Oct 27 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "session/pintan/s_encrypt_pintan.h"
#include "parser/parser.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>

#include <time.h>




/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static AQFINTS_SEGMENT *_createCryptHead(AQFINTS_SESSION *sess, const AQFINTS_KEYDESCR *keyDescr);
static AQFINTS_SEGMENT *_createCryptData(AQFINTS_SESSION *sess,
                                         const AQFINTS_KEYDESCR *keyDescr,
                                         const uint8_t *ptrEncryptedData,
                                         uint32_t lenEncryptedData);

static GWEN_BUFFER *_getSegmentListData(AQFINTS_SEGMENT_LIST *segmentList);
static int _prepareCryptSeg(AQFINTS_SESSION *sess, const AQFINTS_KEYDESCR *keyDescr, GWEN_DB_NODE *cfg);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AQFINTS_Session_EncryptMessagePinTan(AQFINTS_SESSION *sess, AQFINTS_MESSAGE *message)
{
  AQFINTS_SEGMENT_LIST *segmentList;
  AQFINTS_SEGMENT *segCryptHead;
  AQFINTS_SEGMENT *segCryptData;
  GWEN_BUFFER *bufDataToEncrypt;
  const AQFINTS_KEYDESCR *keyDescr;

  keyDescr=AQFINTS_Message_GetCrypter(message);
  if (keyDescr==NULL) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "No crypter set");
    return GWEN_ERROR_GENERIC;
  }
  segmentList=AQFINTS_Message_GetSegmentList(message);
  bufDataToEncrypt=_getSegmentListData(segmentList);

  segCryptHead=_createCryptHead(sess, keyDescr);
  if (segCryptHead==NULL) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here");
    GWEN_Buffer_free(bufDataToEncrypt);
    return GWEN_ERROR_GENERIC;
  }

  segCryptData=_createCryptData(sess, keyDescr,
                                (const uint8_t*) GWEN_Buffer_GetStart(bufDataToEncrypt),
                                GWEN_Buffer_GetUsedBytes(bufDataToEncrypt));
  if (segCryptData==NULL) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here");
    AQFINTS_Segment_free(segCryptHead);
    GWEN_Buffer_free(bufDataToEncrypt);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_Buffer_free(bufDataToEncrypt);

  /* replace segment list in message with HNVSK and HNVSD */
  AQFINTS_Segment_List_Clear(segmentList);
  AQFINTS_Segment_List_Add(segCryptHead, segmentList);
  AQFINTS_Segment_List_Add(segCryptData, segmentList);

  return 0;
}



GWEN_BUFFER *_getSegmentListData(AQFINTS_SEGMENT_LIST *segmentList)
{
  GWEN_BUFFER *buf;

  buf=GWEN_Buffer_new(0, 1024, 0, 1);
  AQFINTS_Segment_List_SampleBuffers(segmentList, buf);
  return buf;
}



AQFINTS_SEGMENT *_createCryptHead(AQFINTS_SESSION *sess, const AQFINTS_KEYDESCR *keyDescr)
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
  defSegment=AQFINTS_Parser_FindSegmentHighestVersionForProto(parser, "HNVSK", hbciVersion);
  if (defSegment==NULL) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "No matching definition segment found for HNVSK (proto=%d)", hbciVersion);
    return NULL;
  }

  segment=AQFINTS_Segment_new();
  AQFINTS_Segment_copy(segment, defSegment);
  AQFINTS_Segment_SetSegmentNumber(segment, 998);
  dbSegment=GWEN_DB_Group_new("cryptHead");
  AQFINTS_Segment_SetDbData(segment, dbSegment);

  rv=_prepareCryptSeg(sess, keyDescr, dbSegment);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    return NULL;
  }

  rv=AQFINTS_Session_WriteSegment(sess, segment);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    AQFINTS_Segment_free(segment);
    return NULL;
  }

  return segment;
}



AQFINTS_SEGMENT *_createCryptData(AQFINTS_SESSION *sess,
                                  const AQFINTS_KEYDESCR *keyDescr,
                                  const uint8_t *ptrEncryptedData,
                                  uint32_t lenEncryptedData)
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
  defSegment=AQFINTS_Parser_FindSegmentHighestVersionForProto(parser, "HNVSD", hbciVersion);
  if (defSegment==NULL) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "No matching definition segment found for HNVSD (proto=%d)", hbciVersion);
    return NULL;
  }

  segment=AQFINTS_Segment_new();
  AQFINTS_Segment_copy(segment, defSegment);
  AQFINTS_Segment_SetSegmentNumber(segment, 999);
  dbSegment=GWEN_DB_Group_new("cryptData");
  AQFINTS_Segment_SetDbData(segment, dbSegment);

  if (ptrEncryptedData && lenEncryptedData)
    GWEN_DB_SetBinValue(dbSegment, GWEN_DB_FLAGS_DEFAULT, "cryptData", ptrEncryptedData, lenEncryptedData);

  rv=AQFINTS_Session_WriteSegment(sess, segment);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    AQFINTS_Segment_free(segment);
    return NULL;
  }

  return segment;
}




int _prepareCryptSeg(AQFINTS_SESSION *sess, const AQFINTS_KEYDESCR *keyDescr, GWEN_DB_NODE *cfg)
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

  /* security profile */
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "secProfile/code", AQFINTS_KeyDescr_GetSecurityProfileName(keyDescr));
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "secProfile/version", AQFINTS_KeyDescr_GetSecurityProfileVersion(keyDescr));

  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "function", 998);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "role", 1);

  /* security details */
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecDetails/dir",
                      AQFINTS_Session_GetIsServer(sess)?2:1); /* 1 client, 2=server */
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecDetails/secId", AQFINTS_KeyDescr_GetSystemId(keyDescr));

  /* security stamp */
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecStamp/stampCode", 1);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecStamp/date", sdate);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecStamp/time", stime);

  /* cryptAlgo */
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "cryptAlgo/purpose", 2);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "cryptAlgo/mode", 2);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "cryptAlgo/algo", 13);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "cryptAlgo/pname", 1);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "cryptAlgo/keytype", 6);
  GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT, "cryptAlgo/msgKey", "NOKEY", 5);

  /* keyname */
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/country", AQFINTS_KeyDescr_GetCountry(keyDescr));
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/bankcode", AQFINTS_KeyDescr_GetBankCode(keyDescr));
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/userid", AQFINTS_KeyDescr_GetUserId(keyDescr));
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/keytype", AQFINTS_KeyDescr_GetKeyType(keyDescr));
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/keynum", AQFINTS_KeyDescr_GetKeyNumber(keyDescr));
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/keyversion", AQFINTS_KeyDescr_GetKeyVersion(keyDescr));

  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "compress", "0");


  return 0;
}



