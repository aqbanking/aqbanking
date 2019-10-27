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

#include "s_pintan_crypt.h"

#include "aqfints/sessionlayer/session.h"
#include "aqfints/msglayer/keyname.h"

#include <gwenhywfar/debug.h>

#include <time.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


static int createCryptHead(AQFINTS_SESSION *sess,
                           const AQFINTS_KEYNAME *keyName,
                           GWEN_BUFFER *destBuffer);
static int createCryptData(AQFINTS_SESSION *sess,
                           const AQFINTS_KEYNAME *keyName,
                           const uint8_t *ptrEncryptedData,
                           uint32_t lenEncryptedData,
                           GWEN_BUFFER *destBuffer);
static int prepareCryptSeg(AQFINTS_SESSION *sess, const AQFINTS_KEYNAME *keyName, GWEN_DB_NODE *cfg);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AQFINTS_SessionPinTan_WrapCrypt(AQFINTS_SESSION *sess,
                                    const AQFINTS_KEYNAME *keyName,
                                    const uint8_t *ptrEncryptedData,
                                    uint32_t lenEncryptedData,
                                    GWEN_BUFFER *msgBuffer)
{
  int rv;

  /* crypt head */
  rv=createCryptHead(sess, keyName, msgBuffer);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    return rv;
  }

  /* crypt data */
  rv=createCryptData(sess, keyName, ptrEncryptedData, lenEncryptedData, msgBuffer);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    return rv;
  }

  return 0;
}




int createCryptHead(AQFINTS_SESSION *sess,
                    const AQFINTS_KEYNAME *keyName,
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
  defSegment=AQFINTS_Parser_FindSegmentHighestVersionForProto(parser, "HNVSK", hbciVersion);
  if (defSegment==NULL) {
    DBG_ERROR(0, "No matching definition segment found for HNVSK (proto=%d)", hbciVersion);
    return GWEN_ERROR_INTERNAL;
  }

  segment=AQFINTS_Segment_new();
  AQFINTS_Segment_copy(segment, defSegment);
  AQFINTS_Segment_SetSegmentNumber(segment, 998);
  dbSegment=GWEN_DB_Group_new("cryptHead");
  AQFINTS_Segment_SetDbData(segment, dbSegment);

  rv=prepareCryptSeg(sess, keyName, dbSegment);
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



int createCryptData(AQFINTS_SESSION *sess,
                    const AQFINTS_KEYNAME *keyName,
                    const uint8_t *ptrEncryptedData,
                    uint32_t lenEncryptedData,
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
  defSegment=AQFINTS_Parser_FindSegmentHighestVersionForProto(parser, "HNVSD", hbciVersion);
  if (defSegment==NULL) {
    DBG_ERROR(0, "No matching definition segment found for HNVSD (proto=%d)", hbciVersion);
    return GWEN_ERROR_INTERNAL;
  }

  segment=AQFINTS_Segment_new();
  AQFINTS_Segment_copy(segment, defSegment);
  AQFINTS_Segment_SetSegmentNumber(segment, 999);
  dbSegment=GWEN_DB_Group_new("cryptData");
  AQFINTS_Segment_SetDbData(segment, dbSegment);

  if (ptrEncryptedData && lenEncryptedData)
    GWEN_DB_SetBinValue(dbSegment, GWEN_DB_FLAGS_DEFAULT, "cryptData", ptrEncryptedData, lenEncryptedData);

  rv=AQFINTS_Session_WriteSegment(sess, segment, destBuffer);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    return rv;
  }

  return rv;
}



int prepareCryptSeg(AQFINTS_SESSION *sess, const AQFINTS_KEYNAME *keyName, GWEN_DB_NODE *cfg)
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
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "secProfile/code", AQFINTS_Session_GetSecProfileCode(sess));
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "secProfile/version", AQFINTS_Session_GetSecProfileVersion(sess));

  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "function", 998);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "role", 1);

  /* security details */
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecDetails/dir",
                      AQFINTS_Session_GetIsServer(sess)?2:1); /* 1 client, 2=server */
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecDetails/secId", AQFINTS_KeyName_GetSystemId(keyName));

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
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/country", AQFINTS_KeyName_GetCountry(keyName));
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/bankcode", AQFINTS_KeyName_GetBankCode(keyName));
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/userid", AQFINTS_KeyName_GetUserId(keyName));
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/keytype", AQFINTS_KeyName_GetKeyType(keyName));
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/keynum", AQFINTS_KeyName_GetKeyNumber(keyName));
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/keyversion", AQFINTS_KeyName_GetKeyVersion(keyName));

  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "compress", "0");


  return 0;
}


