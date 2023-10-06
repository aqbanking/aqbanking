/***************************************************************************
    begin       : Tue Nov 25 2008
    copyright   : (C) 2020 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "msgcrypt_rxh_decrypt.h"
#include "msgcrypt_rxh_common.h"
#include "message_p.h"

#include "aqhbci/aqhbci_l.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/cryptkeysym.h>
#include <gwenhywfar/padd.h>
#include <gwenhywfar/gui.h>

#include <aqbanking/banking_be.h>


/* ------------------------------------------------------------------------------------------------
 * defines
 * ------------------------------------------------------------------------------------------------
 */

#define AH_MSG_DECRYPTRXH_MAXOFFSET 32



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static GWEN_CRYPT_KEY *_extractMessageKeyFromCryptHead(AH_MSG *hmsg, int rxhProtocol, GWEN_DB_NODE *grHead);
static const GWEN_CRYPT_TOKEN_KEYINFO *_retrieveDecryptKeyInfoFromCryptToken(AB_USER *u, GWEN_CRYPT_TOKEN *ct);
static GWEN_CRYPT_KEY *_decryptMessageKey(int rxhProtocol,
					  GWEN_CRYPT_TOKEN *ct,
					  uint32_t idDecryptUserKey, int sizeDecryptUserKey,
					  const uint8_t *ptrRawEncryptedMsgKey, uint32_t lenRawEncryptedMsgKey);
static GWEN_CRYPT_KEY *_mkMessageKeyFromDecryptedData(int rxhProtocol, uint8_t *ptrDecryptedMsgKey, uint32_t lenDecryptedMsgKey);
static GWEN_BUFFER *_getDecryptedMessage(GWEN_CRYPT_KEY *sk, int rxhProtocol, const uint8_t *pSource, uint32_t lSource);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */

int AH_Msg_DecryptRxh(AH_MSG *hmsg, GWEN_DB_NODE *gr)
{
  AH_HBCI *h;
  GWEN_BUFFER *mbuf;
  uint32_t l;
  const uint8_t *p;
  AB_USER *u;
  GWEN_CRYPT_KEY *sk=NULL;
  GWEN_DB_NODE *nhead=NULL;
  GWEN_DB_NODE *ndata=NULL;
  const char *crypterId;
  const RXH_PARAMETER *rxh_parameter;

  assert(hmsg);
  h=AH_Dialog_GetHbci(hmsg->dialog);
  assert(h);

  u=AH_Dialog_GetDialogOwner(hmsg->dialog);

  rxh_parameter=AH_MsgRxh_GetParameters(AH_User_GetCryptMode(u), AH_User_GetRdhType(u));

  /* get encrypted session key */
  nhead=GWEN_DB_GetGroup(gr, GWEN_DB_FLAGS_DEFAULT | GWEN_PATH_FLAGS_NAMEMUSTEXIST, "CryptHead");
  if (!nhead) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No crypt head");
    return GWEN_ERROR_BAD_DATA;
  }

  sk=_extractMessageKeyFromCryptHead(hmsg, rxh_parameter->protocol, nhead);
  if (sk==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Missing message key");
    return GWEN_ERROR_BAD_DATA;
  }

  /* get encrypted data */
  ndata=GWEN_DB_GetGroup(gr, GWEN_DB_FLAGS_DEFAULT | GWEN_PATH_FLAGS_NAMEMUSTEXIST, "CryptData");
  if (!ndata) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No crypt data");
    GWEN_Crypt_Key_free(sk);
    return GWEN_ERROR_BAD_DATA;
  }
  p=GWEN_DB_GetBinValue(ndata, "CryptData", 0, 0, 0, &l);
  if (!p || !l) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No crypt data");
    GWEN_Crypt_Key_free(sk);
    return GWEN_ERROR_BAD_DATA;
  }

  /* decrypt data */
  mbuf=_getDecryptedMessage(sk, rxh_parameter->protocol, p, l);
  if (mbuf==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not decipher with DES session key.");
    GWEN_Crypt_Key_free(sk);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_Crypt_Key_free(sk);

  /* store crypter id */
  crypterId=GWEN_DB_GetCharValue(nhead, "key/userId", 0, I18N("unknown"));
  AH_Msg_SetCrypterId(hmsg, crypterId);

  /* store new buffer inside message */
  GWEN_Buffer_free(hmsg->origbuffer);
  hmsg->origbuffer=hmsg->buffer;
  GWEN_Buffer_Rewind(mbuf);
  hmsg->buffer=mbuf;

  return 0;
}



GWEN_CRYPT_KEY *_extractMessageKeyFromCryptHead(AH_MSG *hmsg, int rxhProtocol, GWEN_DB_NODE *grCryptHead)
{
  AH_HBCI *h;
  int rv;
  const uint8_t *ptrRawEncryptedMsgKey;
  uint32_t lenRawEncryptedMsgKey;
  AB_USER *u;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_KEYINFO *ki;
  uint32_t idDecryptUserKey;
  int sizeDecryptUserKey;

  assert(hmsg);
  h=AH_Dialog_GetHbci(hmsg->dialog);
  assert(h);
  u=AH_Dialog_GetDialogOwner(hmsg->dialog);

  rv=AB_Banking_GetCryptToken(AH_HBCI_GetBankingApi(h), AH_User_GetTokenType(u), AH_User_GetTokenName(u), &ct);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not get crypt token for user \"%s\" (%d)", AB_User_GetUserId(u), rv);
    return NULL;
  }

  ki=_retrieveDecryptKeyInfoFromCryptToken(u, ct);
  if (ki==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return NULL;
  }
  idDecryptUserKey=GWEN_Crypt_Token_KeyInfo_GetId(ki);
  sizeDecryptUserKey=GWEN_Crypt_Token_KeyInfo_GetKeySize(ki);

  /* get encrypted session key */
  ptrRawEncryptedMsgKey=GWEN_DB_GetBinValue(grCryptHead, "CryptAlgo/MsgKey", 0, 0, 0, &lenRawEncryptedMsgKey);
  if (ptrRawEncryptedMsgKey && lenRawEncryptedMsgKey) {
    GWEN_CRYPT_KEY *sk;

    sk=_decryptMessageKey(rxhProtocol, ct, idDecryptUserKey, sizeDecryptUserKey, ptrRawEncryptedMsgKey, lenRawEncryptedMsgKey);
    if (sk==NULL) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create message key from data");
      return NULL;
    }
    return sk;
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Missing message key in HBCI message");
    return NULL;
  }
}



const GWEN_CRYPT_TOKEN_KEYINFO *_retrieveDecryptKeyInfoFromCryptToken(AB_USER *u, GWEN_CRYPT_TOKEN *ct)
{
  const GWEN_CRYPT_TOKEN_KEYINFO *ki;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  const char *sTokenType;
  const char *sTokenName;
  int rv;
  uint32_t idDecryptUserKey;

  /* get token info for logging */
  sTokenType=GWEN_Crypt_Token_GetTypeName(ct);
  sTokenName=GWEN_Crypt_Token_GetTokenName(ct);

  /* open CryptToken if necessary */
  if (!GWEN_Crypt_Token_IsOpen(ct)) {
    GWEN_Crypt_Token_AddModes(ct, GWEN_CRYPT_TOKEN_MODE_DIRECT_SIGN);
    rv=GWEN_Crypt_Token_Open(ct, 0, 0);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Could not open crypt token for user \"%s\" (%d)", AB_User_GetUserId(u), rv);
      return NULL;
    }
  }

  /* get context and key info */
  ctx=GWEN_Crypt_Token_GetContext(ct, AH_User_GetTokenContextId(u), 0);
  if (ctx==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Context %d not found on crypt token [%s:%s]", AH_User_GetTokenContextId(u), sTokenType,
             sTokenName);
    return NULL;
  }

  idDecryptUserKey=GWEN_Crypt_Token_Context_GetDecipherKeyId(ctx);
  ki=GWEN_Crypt_Token_GetKeyInfo(ct, idDecryptUserKey, 0xffffffff, 0);
  if (ki==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Keyinfo %04x not found on crypt token [%s:%s]", idDecryptUserKey, sTokenType, sTokenName);
    return NULL;
  }
  return ki;
}



GWEN_CRYPT_KEY *_decryptMessageKey(int rxhProtocol,
				   GWEN_CRYPT_TOKEN *ct,
				   uint32_t idDecryptUserKey, int sizeDecryptUserKey,
				   const uint8_t *ptrRawEncryptedMsgKey, uint32_t lenRawEncryptedMsgKey)
{
  int rv;
  GWEN_CRYPT_PADDALGO *algo;
  uint8_t bufEncryptedKey[AH_MSGRXH_MAXKEYBUF+64];
  uint8_t bufDecryptedMsgKey[AH_MSGRXH_MAXKEYBUF+64+AH_MSG_DECRYPTRXH_MAXOFFSET];
  uint8_t *ptrDecryptedMsgKey;
  uint32_t lenDecryptedMsgKey;
  GWEN_CRYPT_KEY *sk=NULL;

  if (sizeDecryptUserKey<lenRawEncryptedMsgKey) {
    DBG_WARN(AQHBCI_LOGDOMAIN, "Keyinfo keysize is smaller than size of transmitted key, adjusting");
    sizeDecryptUserKey=lenRawEncryptedMsgKey;
  }
  assert(sizeDecryptUserKey<=AH_MSGRXH_MAXKEYBUF);

  /* fill encoded key with 0 to the total length of our private key */
  memset(bufEncryptedKey, 0, sizeof(bufEncryptedKey));
  memmove(bufEncryptedKey+(sizeDecryptUserKey-lenRawEncryptedMsgKey), ptrRawEncryptedMsgKey, lenRawEncryptedMsgKey);

  memset(bufDecryptedMsgKey, 0, sizeof(bufDecryptedMsgKey));
  ptrDecryptedMsgKey=bufDecryptedMsgKey+AH_MSG_DECRYPTRXH_MAXOFFSET;
  lenDecryptedMsgKey=sizeof(bufDecryptedMsgKey)-AH_MSG_DECRYPTRXH_MAXOFFSET;

  algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_None);
  rv=GWEN_Crypt_Token_Decipher(ct, idDecryptUserKey, algo,
			       bufEncryptedKey, sizeDecryptUserKey,
			       ptrDecryptedMsgKey, &lenDecryptedMsgKey,
			       0);
  GWEN_Crypt_PaddAlgo_free(algo);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return NULL;
  }

  sk=_mkMessageKeyFromDecryptedData(rxhProtocol, ptrDecryptedMsgKey, lenDecryptedMsgKey);
  if (sk==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create message key from data");
    return NULL;
  }

  return sk;
}



GWEN_CRYPT_KEY *_mkMessageKeyFromDecryptedData(int rxhProtocol, uint8_t *ptrDecryptedMsgKey, uint32_t lenDecryptedMsgKey)
{
  uint8_t expectedMsgKeySize;
  GWEN_CRYPT_KEY *sk;

  switch (rxhProtocol) {
  case AH_CryptMode_Rdh:
    expectedMsgKeySize=16;
    break;
  case AH_CryptMode_Rah:
    expectedMsgKeySize=32;
    break;
  default:
    return NULL;
  }

  /* unpadd and generate key */
  if (lenDecryptedMsgKey>expectedMsgKeySize) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Decrypted data larger than keysize (%d > %d), skipping bytes", lenDecryptedMsgKey, expectedMsgKeySize);
    ptrDecryptedMsgKey+=(lenDecryptedMsgKey-expectedMsgKeySize);
    lenDecryptedMsgKey=expectedMsgKeySize;
  }
  else if (lenDecryptedMsgKey<expectedMsgKeySize) {
    int delta;

    delta=expectedMsgKeySize-lenDecryptedMsgKey;
    if (delta>AH_MSG_DECRYPTRXH_MAXOFFSET) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Decrypted data smaller is way too small (%d < %d), aborting", lenDecryptedMsgKey, expectedMsgKeySize);
      return NULL;
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Decrypted data smaller than keysize (%d < %d)", lenDecryptedMsgKey, expectedMsgKeySize);
      ptrDecryptedMsgKey-=delta;
      lenDecryptedMsgKey=expectedMsgKeySize;
    }
  }

#if 0
  DBG_ERROR(AQHBCI_LOGDOMAIN,
	    "DES key provided in message (padded key size=%d, unpadded keysize=%d, keyPos=%d):",
	    lenDecryptedMsgKey, expectedMsgKeySize, (lenDecryptedMsgKey-expectedMsgKeySize));
  GWEN_Text_LogString((const char *)ptrDecryptedMsgKey, lenDecryptedMsgKey, AQHBCI_LOGDOMAIN, GWEN_LoggerLevel_Error);
#endif

  switch (rxhProtocol) {
  case AH_CryptMode_Rdh:
    sk=GWEN_Crypt_KeyDes3K_fromData(GWEN_Crypt_CryptMode_Cbc, 24, ptrDecryptedMsgKey, 16);
    break;
  case AH_CryptMode_Rah:
    sk=GWEN_Crypt_KeyAes256_fromData(GWEN_Crypt_CryptMode_Cbc, 32, ptrDecryptedMsgKey, 32);
    break;
  default:
    return NULL;
  }
  if (sk==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create message key from data");
    return NULL;
  }

  return sk;
}



GWEN_BUFFER *_getDecryptedMessage(GWEN_CRYPT_KEY *sk, int rxhProtocol, const uint8_t *pSource, uint32_t lSource)
{
  GWEN_BUFFER *mbuf;
  int rv;
  uint32_t lDest;


  /* decipher message with session key */
  lDest=lSource+1024;                        /* maybe the size should be increased even more */
  mbuf=GWEN_Buffer_new(0, lDest, 0, 1);
  rv=GWEN_Crypt_Key_Decipher(sk,
                             (const uint8_t *)pSource, lSource,
                             (uint8_t *)GWEN_Buffer_GetPosPointer(mbuf),
                             &lDest);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not decipher with DES session key (%d)", rv);
    GWEN_Buffer_free(mbuf);
    return NULL;
  }
  GWEN_Buffer_IncrementPos(mbuf, lDest);
  GWEN_Buffer_AdjustUsedBytes(mbuf);

  /* unpadd message */
  switch (rxhProtocol) {
  case AH_CryptMode_Rdh:
    DBG_INFO(AQHBCI_LOGDOMAIN, "Unpadding with ANSI X9.23");
    rv=GWEN_Padd_UnpaddWithAnsiX9_23(mbuf);
    break;
  case AH_CryptMode_Rah:
    DBG_INFO(AQHBCI_LOGDOMAIN, "Unpadding with ZKA padding");
    rv=GWEN_Padd_UnpaddWithZka(mbuf);
    break;
  default:
    return NULL;
  }
  if (rv) {
    uint8_t *keyData;
    uint32_t  keyLen;

    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error unpadding message (%d), details follow", rv);
    keyData=GWEN_Crypt_KeySym_GetKeyDataPtr(sk);
    keyLen=GWEN_Crypt_KeySym_GetKeyDataLen(sk);
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Key data (rxhProtocol: %d):", rxhProtocol);
    GWEN_Text_LogString((const char*) keyData, keyLen, AQHBCI_LOGDOMAIN, GWEN_LoggerLevel_Error);

    DBG_ERROR(AQHBCI_LOGDOMAIN, "Encrypted data:");
    GWEN_Text_LogString((const char*) pSource, lSource, AQHBCI_LOGDOMAIN, GWEN_LoggerLevel_Error);

    DBG_ERROR(AQHBCI_LOGDOMAIN, "Decrypted data (after unpadding):");
    GWEN_Text_LogString(GWEN_Buffer_GetStart(mbuf),
                        GWEN_Buffer_GetUsedBytes(mbuf),
                        AQHBCI_LOGDOMAIN, GWEN_LoggerLevel_Error);
    GWEN_Buffer_free(mbuf);
    return NULL;
  }

  return mbuf;
}



