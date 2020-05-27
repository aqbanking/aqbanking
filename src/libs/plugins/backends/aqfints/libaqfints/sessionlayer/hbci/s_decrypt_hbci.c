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

#include "sessionlayer/hbci/s_decrypt_hbci.h"
#include "parser/parser.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/cryptkeysym.h>
#include <gwenhywfar/padd.h>


#define AQFINTS_DECRYPT_HBCI_MAXKEYBUF 4096



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


static GWEN_CRYPT_KEY *_getDecryptedMessageKey(AQFINTS_SESSION *sess,
                                               const AQFINTS_CRYPTPARAMS *cryptParams,
                                               const AQFINTS_KEYDESCR *keyDescr,
                                               const uint8_t *ptr,
                                               uint32_t len);

static GWEN_BUFFER *_getDecryptedMessage(GWEN_CRYPT_KEY *sk,
                                         const AQFINTS_CRYPTPARAMS *cryptParams,
                                         const uint8_t *pSource,
                                         uint32_t lSource);




/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */


int AQFINTS_Session_DecryptSegmentHbci(AQFINTS_SESSION *sess,
                                       AQFINTS_SEGMENT *segCryptHead,
                                       AQFINTS_SEGMENT *segCryptData,
                                       const AQFINTS_CRYPTPARAMS *cryptParams,
                                       const AQFINTS_KEYDESCR *keyDescr,
                                       AQFINTS_SEGMENT_LIST *segmentList)
{
  GWEN_DB_NODE *dbCryptHead;
  GWEN_DB_NODE *dbCryptData;
  unsigned int len=0;
  const uint8_t *ptr;
  GWEN_CRYPT_KEY *key;
  GWEN_BUFFER *bufDecodedMessage;

  dbCryptHead=AQFINTS_Segment_GetDbData(segCryptHead);
  assert(dbCryptHead);
  dbCryptData=AQFINTS_Segment_GetDbData(segCryptData);
  assert(dbCryptData);

  ptr=(const uint8_t *) GWEN_DB_GetBinValue(dbCryptHead, "CryptAlgo/MsgKey", 0, 0, 0, &len);
  if (ptr==NULL || len<1) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "No message key in segment");
    return GWEN_ERROR_BAD_DATA;
  }

  key=_getDecryptedMessageKey(sess, cryptParams, keyDescr, ptr, len);
  if (key==NULL) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here");
    return GWEN_ERROR_BAD_DATA;
  }

  ptr=(const uint8_t *) GWEN_DB_GetBinValue(dbCryptData, "CryptData", 0, NULL, 0, &len);
  if (ptr==NULL || len<1) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "No CryptData in segment");
    GWEN_Crypt_Key_free(key);
    return GWEN_ERROR_BAD_DATA;
  }

  bufDecodedMessage=_getDecryptedMessage(key, cryptParams, ptr, len);
  if (bufDecodedMessage==NULL) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "Error decrypting CryptData");
    GWEN_Crypt_Key_free(key);
    return GWEN_ERROR_BAD_DATA;
  }
  else {
    AQFINTS_PARSER *parser;
    AQFINTS_SEGMENT_LIST *newSegmentList;
    AQFINTS_SEGMENT *segment;
    int rv;

    parser=AQFINTS_Session_GetParser(sess);
    newSegmentList=AQFINTS_Segment_List_new();
    rv=AQFINTS_Parser_ReadIntoSegmentList(parser, newSegmentList,
                                          (const uint8_t*) GWEN_Buffer_GetStart(bufDecodedMessage),
                                          GWEN_Buffer_GetUsedBytes(bufDecodedMessage));
    if (rv<0) {
      DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(bufDecodedMessage);
      GWEN_Crypt_Key_free(key);
      AQFINTS_Segment_List_free(newSegmentList);
      return rv;
    }

    rv=AQFINTS_Parser_ReadSegmentListToDb(parser, newSegmentList);
    if (rv<0) {
      DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(bufDecodedMessage);
      GWEN_Crypt_Key_free(key);
      AQFINTS_Segment_List_free(newSegmentList);
      return rv;
    }

    /* move new segments to given segment list, delete new segment list */
    while ((segment=AQFINTS_Segment_List_First(newSegmentList))) {
      AQFINTS_Segment_AddRuntimeFlags(segment, AQFINTS_SEGMENT_RTFLAGS_ENCRYPTED);
      AQFINTS_Segment_List_Del(segment);
      AQFINTS_Segment_List_Add(segment, segmentList);
    }
    AQFINTS_Segment_List_free(newSegmentList);

    GWEN_Buffer_free(bufDecodedMessage);
    GWEN_Crypt_Key_free(key);

    /* done */
    return 0;
  }
}



GWEN_CRYPT_KEY *_getDecryptedMessageKey(AQFINTS_SESSION *sess,
                                        const AQFINTS_CRYPTPARAMS *cryptParams,
                                        const AQFINTS_KEYDESCR *keyDescr,
                                        const uint8_t *ptr,
                                        uint32_t len)
{
  GWEN_CRYPT_KEY *key;
  uint8_t bufDecKey[AQFINTS_DECRYPT_HBCI_MAXKEYBUF+64];
  uint32_t lenDecKey;
  uint8_t *pKeyStart;
  int rv;
  AQFINTS_CRYPTPARAMS_CRYPTALGO cryptAlgo;
  int decKeySize;

  lenDecKey=sizeof(bufDecKey);
  rv=AQFINTS_Session_DecryptSessionKey(sess, keyDescr, cryptParams, ptr, len, bufDecKey, &lenDecKey);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    return NULL;
  }

  /* get resulting keysize */
  cryptAlgo=AQFINTS_CryptParams_GetCryptAlgo(cryptParams);
  switch(cryptAlgo) {
  case AQFINTS_CryptParams_CryptAlgoTwoKeyTripleDes: decKeySize=16; break;
  case AQFINTS_CryptParams_CryptAlgoAes256:          decKeySize=32; break;
  default:
    DBG_ERROR(AQFINTS_LOGDOMAIN, "Unhandled crypt algo %d (%s)", cryptAlgo, AQFINTS_CryptParams_CryptAlgo_toString(cryptAlgo));
    return NULL;
  }

  /* unpadd */
  pKeyStart=bufDecKey+(lenDecKey-decKeySize);
#if 0
  DBG_ERROR(AQFINTS_LOGDOMAIN,
            "Message key provided in message (padded key size=%d, unpadded keysize=%d, keyPos=%d):",
            lenDecKey, decKeySize, (lenDecKey-decKeySize));
  GWEN_Text_LogString((const char *)bufDecKey, lenDecKey, AQFINTS_LOGDOMAIN, GWEN_LoggerLevel_Error);
#endif

  /* generate key */
  switch(cryptAlgo) {
  case AQFINTS_CryptParams_CryptAlgoTwoKeyTripleDes:
    key=GWEN_Crypt_KeyDes3K_fromData(GWEN_Crypt_CryptMode_Cbc, 24, pKeyStart, 16);
    break;
  case AQFINTS_CryptParams_CryptAlgoAes256:
    key=GWEN_Crypt_KeyAes256_fromData(GWEN_Crypt_CryptMode_Cbc, 32, pKeyStart, 32);
    break;
  default:
    DBG_ERROR(AQFINTS_LOGDOMAIN, "Unhandled crypt algo %d (%s)", cryptAlgo, AQFINTS_CryptParams_CryptAlgo_toString(cryptAlgo));
    return NULL;
  }
  if (key==NULL) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "Invalid key data in message");
    return NULL;
  }

  return key;
}



GWEN_BUFFER *_getDecryptedMessage(GWEN_CRYPT_KEY *sk,
                                  const AQFINTS_CRYPTPARAMS *cryptParams,
                                  const uint8_t *pSource,
                                  uint32_t lSource)
{
  GWEN_BUFFER *mbuf;
  int rv;
  uint32_t lDest;
  AQFINTS_CRYPTPARAMS_CRYPTALGO cryptAlgo;

  /* decipher message with session key */
  lDest=lSource+1024;                        /* maybe the size should be increased even more */
  mbuf=GWEN_Buffer_new(0, lSource+1024, 0, 1);
  lDest=GWEN_Buffer_GetMaxUnsegmentedWrite(mbuf);
  rv=GWEN_Crypt_Key_Decipher(sk,
                             (const uint8_t *)pSource, lSource,
                             (uint8_t *)GWEN_Buffer_GetPosPointer(mbuf),
                             &lDest);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "Could not decipher with message key (%d)", rv);
    GWEN_Buffer_free(mbuf);
    return NULL;
  }
  GWEN_Buffer_IncrementPos(mbuf, lDest);
  GWEN_Buffer_AdjustUsedBytes(mbuf);

  /* unpadd message */
  cryptAlgo=AQFINTS_CryptParams_GetCryptAlgo(cryptParams);
  switch(cryptAlgo) {
  case AQFINTS_CryptParams_CryptAlgoTwoKeyTripleDes:
    DBG_DEBUG(AQFINTS_LOGDOMAIN, "Unpadding with ANSI X9.23");
    rv=GWEN_Padd_UnpaddWithAnsiX9_23(mbuf);
    break;
  case AQFINTS_CryptParams_CryptAlgoAes256:
    DBG_DEBUG(AQFINTS_LOGDOMAIN, "Unpadding with ZKA padding");
    rv=GWEN_Padd_UnpaddWithZka(mbuf);
    break;
  default:
    DBG_ERROR(AQFINTS_LOGDOMAIN, "Unhandled crypt algo %d (%s)", cryptAlgo, AQFINTS_CryptParams_CryptAlgo_toString(cryptAlgo));
    GWEN_Buffer_free(mbuf);
    return NULL;
  }

  if (rv) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "Error unpadding this message (%d)", rv);
    GWEN_Buffer_Dump(mbuf, 2);
    GWEN_Buffer_free(mbuf);
    return NULL;
  }

  return mbuf;
}




