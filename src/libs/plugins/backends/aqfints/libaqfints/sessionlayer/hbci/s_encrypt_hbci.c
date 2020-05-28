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

#include "sessionlayer/hbci/s_encrypt_hbci.h"
#include "parser/parser.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/cryptkeysym.h>
#include <gwenhywfar/padd.h>

#include <time.h>




/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int _encrypt(AQFINTS_SESSION *sess,
                    GWEN_BUFFER *bufInRawData,
                    AQFINTS_KEYDESCR *keyDescr,
                    const AQFINTS_CRYPTPARAMS *cryptParams,
                    GWEN_BUFFER *bufOutEncryptedData,
                    GWEN_BUFFER *bufOutEncryptedKey);
static int _encryptDes(AQFINTS_SESSION *sess,
                       GWEN_BUFFER *bufInRawData,
                       AQFINTS_KEYDESCR *keyDescr,
                       const AQFINTS_CRYPTPARAMS *cryptParams,
                       GWEN_BUFFER *bufOutEncryptedData,
                       GWEN_BUFFER *bufOutEncryptedKey);
static int _encryptAes(AQFINTS_SESSION *sess,
                       GWEN_BUFFER *bufInRawData,
                       AQFINTS_KEYDESCR *keyDescr,
                       const AQFINTS_CRYPTPARAMS *cryptParams,
                       GWEN_BUFFER *bufOutEncryptedData,
                       GWEN_BUFFER *bufOutEncryptedKey);
static int _encryptMessageAndKey(AQFINTS_SESSION *sess,
                                 AQFINTS_KEYDESCR *keyDescr,
                                 const AQFINTS_CRYPTPARAMS *cryptParams,
                                 GWEN_CRYPT_KEY *keyMessage,
                                 uint8_t messageKeySize,
                                 GWEN_BUFFER *bufInRawData,
                                 GWEN_BUFFER *bufRawKeyData,
                                 GWEN_BUFFER *bufOutEncryptedData,
                                 GWEN_BUFFER *bufOutEncryptedKey);


static AQFINTS_SEGMENT *_createCryptHead(AQFINTS_SESSION *sess,
                                         const AQFINTS_CRYPTPARAMS *cryptParams,
                                         const AQFINTS_KEYDESCR *keyDescr,
                                         const uint8_t *ptrEncryptedKey,
                                         uint32_t lenEncryptedKey);
static AQFINTS_SEGMENT *_createCryptData(AQFINTS_SESSION *sess,
                                         const AQFINTS_KEYDESCR *keyDescr,
                                         const uint8_t *ptrEncryptedData,
                                         uint32_t lenEncryptedData);

static GWEN_BUFFER *_getSegmentListData(AQFINTS_SEGMENT_LIST *segmentList);
static int _prepareCryptSeg(AQFINTS_SESSION *sess,
                            const AQFINTS_CRYPTPARAMS *cryptParams,
                            const AQFINTS_KEYDESCR *keyDescr,
                            GWEN_DB_NODE *cfg);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AQFINTS_Session_EncryptMessageHbci(AQFINTS_SESSION *sess, AQFINTS_MESSAGE *message)
{
  AQFINTS_SEGMENT *segCryptHead;
  AQFINTS_SEGMENT *segCryptData;
  GWEN_BUFFER *bufDataToEncrypt;
  GWEN_BUFFER *bufEncryptedData;
  GWEN_BUFFER *bufEncryptedKey;
  AQFINTS_KEYDESCR *keyDescr;
  const char *securityProfileName;
  int securityProfileVersion;
  const AQFINTS_CRYPTPARAMS *cryptParams;
  AQFINTS_SEGMENT_LIST *segmentList;
  int rv;

  /* get and update key descriptor */
  keyDescr=AQFINTS_Message_GetCrypter(message);
  if (keyDescr==NULL) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "No crypter set");
    return GWEN_ERROR_GENERIC;
  }

  rv=AQFINTS_Session_FilloutKeyname(sess, keyDescr, AQFINTS_SESSION_CRYPTOP_ENCRYPT);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* get crypt params */
  securityProfileName=AQFINTS_KeyDescr_GetSecurityProfileName(keyDescr);
  securityProfileVersion=AQFINTS_KeyDescr_GetSecurityProfileVersion(keyDescr);

  /* hack for hibiscus */
  if (securityProfileVersion==0) {
    if (securityProfileName && strcasecmp(securityProfileName, "RDH")==0)
      securityProfileVersion=10;
  }

  cryptParams=AQFINTS_CryptParams_GetParamsForSecurityProfile(securityProfileName, securityProfileVersion);
  if (cryptParams==NULL) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "No crypt params for [%s:%d]", securityProfileName?securityProfileName:"<empty>", securityProfileVersion);
    return GWEN_ERROR_INVALID;
  }

  /* encrypt message and key */
  segmentList=AQFINTS_Message_GetSegmentList(message);
  bufDataToEncrypt=_getSegmentListData(segmentList);
  GWEN_Buffer_Rewind(bufDataToEncrypt);


  AQFINTS_Session_LogMessage(sess,
                             (const uint8_t*) GWEN_Buffer_GetStart(bufDataToEncrypt),
                             GWEN_Buffer_GetUsedBytes(bufDataToEncrypt),
                             0, 0); /* !rec, !crypt */


  bufEncryptedData=GWEN_Buffer_new(0, 1024, 0, 1); /* size doesn't matter here, will be adjusted in called fn */
  bufEncryptedKey=GWEN_Buffer_new(0, 512, 0, 1);

  rv=_encrypt(sess, bufDataToEncrypt, keyDescr, cryptParams, bufEncryptedData, bufEncryptedKey);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(bufEncryptedKey);
    GWEN_Buffer_free(bufEncryptedData);
    GWEN_Buffer_free(bufDataToEncrypt);
    return rv;
  }
  GWEN_Buffer_free(bufDataToEncrypt);


  /* create crypt head segment */
  segCryptHead=_createCryptHead(sess,
                                cryptParams,
                                keyDescr,
                                (const uint8_t*) GWEN_Buffer_GetStart(bufEncryptedKey),
                                GWEN_Buffer_GetUsedBytes(bufEncryptedKey));
  if (segCryptHead==NULL) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here");
    GWEN_Buffer_free(bufEncryptedKey);
    GWEN_Buffer_free(bufEncryptedData);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_Buffer_free(bufEncryptedKey);


  /* create crypt data segment */
  segCryptData=_createCryptData(sess,
                                keyDescr,
                                (const uint8_t*) GWEN_Buffer_GetStart(bufEncryptedData),
                                GWEN_Buffer_GetUsedBytes(bufEncryptedData));
  if (segCryptData==NULL) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here");
    AQFINTS_Segment_free(segCryptHead);
    GWEN_Buffer_free(bufEncryptedData);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_Buffer_free(bufEncryptedData);

  /* replace segment list in message with HNVSK and HNVSD */
  AQFINTS_Segment_List_Clear(segmentList);
  AQFINTS_Segment_List_Add(segCryptHead, segmentList);
  AQFINTS_Segment_List_Add(segCryptData, segmentList);

  DBG_INFO(AQFINTS_LOGDOMAIN, "Message encrypted.");
  return 0;
}



int _encrypt(AQFINTS_SESSION *sess,
             GWEN_BUFFER *bufInRawData,
             AQFINTS_KEYDESCR *keyDescr,
             const AQFINTS_CRYPTPARAMS *cryptParams,
             GWEN_BUFFER *bufOutEncryptedData,
             GWEN_BUFFER *bufOutEncryptedKey)
{
  AQFINTS_CRYPTPARAMS_CRYPTALGO cryptAlgo;
  int rv;

  cryptAlgo=AQFINTS_CryptParams_GetCryptAlgo(cryptParams);
  switch(cryptAlgo) {
  case AQFINTS_CryptParams_CryptAlgoTwoKeyTripleDes:
    rv=_encryptDes(sess,
                   bufInRawData,
                   keyDescr,
                   cryptParams,
                   bufOutEncryptedData,
                   bufOutEncryptedKey);
    break;
  case AQFINTS_CryptParams_CryptAlgoAes256:
    rv=_encryptAes(sess,
                   bufInRawData,
                   keyDescr,
                   cryptParams,
                   bufOutEncryptedData,
                   bufOutEncryptedKey);
    break;
  default:
    DBG_ERROR(AQFINTS_LOGDOMAIN, "Unexpected crypt algo \"%d\"", cryptAlgo);
    return GWEN_ERROR_INVALID;
  }
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int _encryptDes(AQFINTS_SESSION *sess,
                GWEN_BUFFER *bufInRawData,
                AQFINTS_KEYDESCR *keyDescr,
                const AQFINTS_CRYPTPARAMS *cryptParams,
                GWEN_BUFFER *bufOutEncryptedData,
                GWEN_BUFFER *bufOutEncryptedKey)
{
  int rv;
  uint8_t messageKeySize;
  int cryptKeySizeInBytes;
  GWEN_CRYPT_KEY *keyMessage;
  GWEN_BUFFER *bufRawKeyData;

  keyMessage=GWEN_Crypt_KeyDes3K_Generate(GWEN_Crypt_CryptMode_Cbc, 24, 2);
  messageKeySize=16;

  /* padd message */
  rv=GWEN_Padd_PaddWithAnsiX9_23(bufInRawData);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Crypt_Key_free(keyMessage);
    return rv;
  }

  cryptKeySizeInBytes=AQFINTS_KeyDescr_GetKeySizeInBytes(keyDescr);
  if (cryptKeySizeInBytes==0) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "Key size in bytes not set (backend error)");
    GWEN_Crypt_Key_free(keyMessage);
    return GWEN_ERROR_GENERIC;
  }
  bufRawKeyData=GWEN_Buffer_new(0, cryptKeySizeInBytes, 0, 1);
  GWEN_Buffer_AppendBytes(bufRawKeyData, (const char*) GWEN_Crypt_KeyDes3K_GetKeyDataPtr(keyMessage), messageKeySize);
  GWEN_Buffer_Rewind(bufRawKeyData);

  rv=_encryptMessageAndKey(sess,
                           keyDescr,
                           cryptParams,
                           keyMessage,
                           messageKeySize,
                           bufInRawData,
                           bufRawKeyData,
                           bufOutEncryptedData,
                           bufOutEncryptedKey);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(bufRawKeyData);
    GWEN_Crypt_Key_free(keyMessage);
    return rv;
  }
  GWEN_Buffer_free(bufRawKeyData);
  GWEN_Crypt_Key_free(keyMessage);

  return 0;
}



int _encryptAes(AQFINTS_SESSION *sess,
                GWEN_BUFFER *bufInRawData,
                AQFINTS_KEYDESCR *keyDescr,
                const AQFINTS_CRYPTPARAMS *cryptParams,
                GWEN_BUFFER *bufOutEncryptedData,
                GWEN_BUFFER *bufOutEncryptedKey)
{
  int rv;
  uint8_t messageKeySize;
  int cryptKeySizeInBytes;
  GWEN_CRYPT_KEY *keyMessage;
  GWEN_BUFFER *bufRawKeyData;

  keyMessage=GWEN_Crypt_KeyAes256_Generate(GWEN_Crypt_CryptMode_Cbc, 32, 2);
  messageKeySize=32;

  /* padd message */
  rv=GWEN_Padd_PaddWithZka(bufInRawData);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* write message key data to buffer */
  cryptKeySizeInBytes=AQFINTS_KeyDescr_GetKeySizeInBytes(keyDescr);
  if (cryptKeySizeInBytes==0) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "Key size in bytes not set (backend error)");
    GWEN_Crypt_Key_free(keyMessage);
    return GWEN_ERROR_GENERIC;
  }
  bufRawKeyData=GWEN_Buffer_new(0, cryptKeySizeInBytes, 0, 1);
  GWEN_Buffer_AppendBytes(bufRawKeyData, (const char*) GWEN_Crypt_KeyAes256_GetKeyDataPtr(keyMessage), messageKeySize);
  GWEN_Buffer_Rewind(bufRawKeyData);

  rv=_encryptMessageAndKey(sess,
                           keyDescr,
                           cryptParams,
                           keyMessage,
                           messageKeySize,
                           bufInRawData,
                           bufRawKeyData,
                           bufOutEncryptedData,
                           bufOutEncryptedKey);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(bufRawKeyData);
    GWEN_Crypt_Key_free(keyMessage);
    return rv;
  }
  GWEN_Buffer_free(bufRawKeyData);
  GWEN_Crypt_Key_free(keyMessage);

  return 0;
}



int _encryptMessageAndKey(AQFINTS_SESSION *sess,
                          AQFINTS_KEYDESCR *keyDescr,
                          const AQFINTS_CRYPTPARAMS *cryptParams,
                          GWEN_CRYPT_KEY *keyMessage,
                          uint8_t messageKeySize,
                          GWEN_BUFFER *bufInRawData,
                          GWEN_BUFFER *bufInRawKeyData,
                          GWEN_BUFFER *bufOutEncryptedData,
                          GWEN_BUFFER *bufOutEncryptedKey)
{
  uint32_t l;
  int rv;
  int cryptKeySizeInBytes;
  AQFINTS_CRYPTPARAMS_OPMODE opMode;
  GWEN_CRYPT_PADDALGO *paddAlgo;

  /* encrypt message with message-key */
  l=GWEN_Buffer_GetUsedBytes(bufInRawData);
  GWEN_Buffer_AllocRoom(bufOutEncryptedData, l);
  l=GWEN_Buffer_GetMaxUnsegmentedWrite(bufOutEncryptedData);
  rv=GWEN_Crypt_Key_Encipher(keyMessage,
                             (uint8_t *)GWEN_Buffer_GetStart(bufInRawData),
                             GWEN_Buffer_GetUsedBytes(bufInRawData),
                             (uint8_t *)GWEN_Buffer_GetPosPointer(bufOutEncryptedData),
                             &l);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "Could not encipher with message key (%d)", rv);
    return rv;
  }
  GWEN_Buffer_IncrementPos(bufOutEncryptedData, l);
  GWEN_Buffer_AdjustUsedBytes(bufOutEncryptedData);

  cryptKeySizeInBytes=AQFINTS_KeyDescr_GetKeySizeInBytes(keyDescr); /* valid size already checked by caller */

  opMode=AQFINTS_CryptParams_GetOpModeCrypt(cryptParams);
  switch(opMode) {
  case AQFINTS_CryptParams_OpModeCbc:
    paddAlgo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_LeftZero);
    break;
  case AQFINTS_CryptParams_OpModeRsa_Pkcs1_v1_5:
    paddAlgo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_Pkcs1_2);
    break;
  default:
    DBG_ERROR(AQFINTS_LOGDOMAIN, "Unexpected op mode %d", opMode);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_Crypt_PaddAlgo_SetPaddSize(paddAlgo, cryptKeySizeInBytes);
  rv=GWEN_Padd_ApplyPaddAlgo(paddAlgo, bufInRawKeyData);
  GWEN_Crypt_PaddAlgo_free(paddAlgo);
  if (rv) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

#if 0
  DBG_ERROR(AQFINTS_LOGDOMAIN, "Padded message key: (cryptKeySizeInBytes=%d)", cryptKeySizeInBytes);
  GWEN_Buffer_Dump(bufInRawKeyData, 2);
#endif

  /* encrypt message key */
  GWEN_Buffer_AllocRoom(bufOutEncryptedKey, cryptKeySizeInBytes+16);
  l=GWEN_Buffer_GetMaxUnsegmentedWrite(bufOutEncryptedKey);

  rv=AQFINTS_Session_EncryptSessionKey(sess,
                                       keyDescr,
                                       cryptParams,
                                       (uint8_t *)GWEN_Buffer_GetPosPointer(bufInRawKeyData),
                                       GWEN_Buffer_GetUsedBytes(bufInRawKeyData),
                                       (uint8_t *)GWEN_Buffer_GetPosPointer(bufOutEncryptedKey),
                                       &l);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "Could not encipher with message key (%d)", rv);
    return rv;
  }
  GWEN_Buffer_IncrementPos(bufOutEncryptedKey, l);
  GWEN_Buffer_AdjustUsedBytes(bufOutEncryptedKey);

  return 0;
}



GWEN_BUFFER *_getSegmentListData(AQFINTS_SEGMENT_LIST *segmentList)
{
  GWEN_BUFFER *buf;

  buf=GWEN_Buffer_new(0, 1024, 0, 1);
  AQFINTS_Segment_List_SampleBuffers(segmentList, buf);
  return buf;
}



AQFINTS_SEGMENT *_createCryptHead(AQFINTS_SESSION *sess,
                                  const AQFINTS_CRYPTPARAMS *cryptParams,
                                  const AQFINTS_KEYDESCR *keyDescr,
                                  const uint8_t *ptrEncryptedKey,
                                  uint32_t lenEncryptedKey)
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

  rv=_prepareCryptSeg(sess, cryptParams, keyDescr, dbSegment);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    return NULL;
  }
  GWEN_DB_SetBinValue(dbSegment, GWEN_DB_FLAGS_DEFAULT, "cryptAlgo/msgKey",
                      ptrEncryptedKey, lenEncryptedKey);

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




int _prepareCryptSeg(AQFINTS_SESSION *sess,
                     const AQFINTS_CRYPTPARAMS *cryptParams,
                     const AQFINTS_KEYDESCR *keyDescr,
                     GWEN_DB_NODE *cfg)
{
  char sdate[9];
  char stime[7];
  struct tm *lt;
  time_t tt;
  const char *s;
  const char *securityProfileName;
  int securityProfileVersion;

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
  securityProfileName=AQFINTS_KeyDescr_GetSecurityProfileName(keyDescr);
  securityProfileVersion=AQFINTS_KeyDescr_GetSecurityProfileVersion(keyDescr);

  /* hack for hibiscus */
  if (securityProfileVersion==0) {
    if (securityProfileName && strcasecmp(securityProfileName, "RDH")==0)
      securityProfileVersion=10;
  }

  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "secProfile/code", securityProfileName);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "secProfile/version", securityProfileVersion);

  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "function", 4);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "role", 1);

  /* security details */
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecDetails/id",
                      AQFINTS_Session_GetIsServer(sess)?2:1); /* 1 client, 2=server */
  s=AQFINTS_KeyDescr_GetSystemId(keyDescr);
  if (s && *s)
    GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecDetails/secId", s);

  /* security stamp */
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecStamp/stampCode", 1);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecStamp/date", sdate);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecStamp/time", stime);

  /* cryptAlgo */
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "cryptAlgo/purpose", 2);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "cryptAlgo/mode", AQFINTS_CryptParams_GetOpModeCrypt(cryptParams));
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "cryptAlgo/algo", AQFINTS_CryptParams_GetCryptAlgo(cryptParams));
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "cryptAlgo/keytype", 6);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "cryptAlgo/pname", 1);
  //GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT, "cryptAlgo/msgKey", "NOKEY", 5);

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



