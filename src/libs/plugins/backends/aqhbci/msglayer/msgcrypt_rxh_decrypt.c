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

#include "aqbanking/i18n_l.h"
#include "aqbanking/banking_be.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/cryptkeysym.h>
#include <gwenhywfar/padd.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static GWEN_CRYPT_KEY *_rxhDecrypt_ExtractMessageKey(AH_MSG *hmsg, int rxhProtocol, GWEN_DB_NODE *grHead);
static GWEN_BUFFER *_rxhDecrypt_GetDecryptedMessage(GWEN_CRYPT_KEY *sk, int rxhProtocol, const uint8_t *pSource,
                                                    uint32_t lSource);
static GWEN_CRYPT_TOKEN *_getAndOpenCryptToken(AH_HBCI *h, AB_USER *u, uint32_t gid);
static const GWEN_CRYPT_TOKEN_KEYINFO *_getDecipherKeyInfo(AB_USER *u, GWEN_CRYPT_TOKEN *ct, uint32_t gid);
static uint32_t _prepareEncryptedKeyBuffer(const GWEN_CRYPT_TOKEN_KEYINFO *ki,
					   const uint8_t *srcPtr, uint32_t srcLen, uint8_t *destPtr, uint32_t destLen);
static const uint8_t *_decryptAndUnpaddKey(GWEN_CRYPT_TOKEN *ct, uint32_t keyId, int decKeySize,
					   const uint8_t *srcPtr, uint32_t srcLen,
					   uint8_t *destPtr, uint32_t destLen,
					   uint32_t gid);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AH_Msg_DecryptRxh(AH_MSG *hmsg, GWEN_DB_NODE *gr)
{
  AH_DIALOG *dlg;
  AH_HBCI *h;
  GWEN_BUFFER *mbuf;
  uint32_t l;
  const uint8_t *p;
  AB_USER *u;
  GWEN_CRYPT_KEY *sk=NULL;
  GWEN_DB_NODE *nhead=NULL;
  GWEN_DB_NODE *ndata=NULL;
  const char *crypterId;
  RXH_PARAMETER *rxh_parameter;

  assert(hmsg);
  dlg=AH_Msg_GetDialog(hmsg);
  h=AH_Dialog_GetHbci(dlg);
  assert(h);
  u=AH_Dialog_GetDialogOwner(dlg);

  /* get correct parameters */
  rxh_parameter=AH_MsgRxh_GetParameters(AH_User_GetCryptMode(u), AH_User_GetRdhType(u));
  if (rxh_parameter==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return GWEN_ERROR_GENERIC;
  }

  /* get encrypted session key */
  nhead=GWEN_DB_GetGroup(gr, GWEN_DB_FLAGS_DEFAULT | GWEN_PATH_FLAGS_NAMEMUSTEXIST, "CryptHead");
  if (!nhead) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No crypt head");
    return GWEN_ERROR_BAD_DATA;
  }

  sk=_rxhDecrypt_ExtractMessageKey(hmsg, rxh_parameter->protocol, nhead);
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
  mbuf=_rxhDecrypt_GetDecryptedMessage(sk, rxh_parameter->protocol, p, l);
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
  AH_Msg_ExchangeBufferWithOrigBuffer(hmsg);
  GWEN_Buffer_Rewind(mbuf);
  AH_Msg_SetBuffer(hmsg, mbuf);

  return 0;
}



GWEN_CRYPT_KEY *_rxhDecrypt_ExtractMessageKey(AH_MSG *hmsg, int rxhProtocol, GWEN_DB_NODE *grCryptHead)
{
  AH_DIALOG *dlg;
  AH_HBCI *h;
  uint32_t l;
  const uint8_t *p;
  AB_USER *u;
  //  uint32_t uFlags;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_KEYINFO *ki;
  uint32_t keyId;
  uint32_t gid=0;

  assert(hmsg);
  dlg=AH_Msg_GetDialog(hmsg);
  h=AH_Dialog_GetHbci(dlg);
  assert(h);
  u=AH_Dialog_GetDialogOwner(dlg);

  ct=_getAndOpenCryptToken(h, u, gid);
  if (ct==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return NULL;
  }

  ki=_getDecipherKeyInfo(u, ct, gid);
  if (ki==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return NULL;
  }
  keyId=GWEN_Crypt_Token_KeyInfo_GetId(ki);

  /* get encrypted session key */
  p=GWEN_DB_GetBinValue(grCryptHead, "CryptAlgo/MsgKey", 0, 0, 0, &l);
  if (p && l) {
    uint8_t encKey[AH_MSGRXH_MAXKEYBUF+64];
    uint8_t decKey[AH_MSGRXH_MAXKEYBUF+64];
    int ksize;
    uint8_t decKeySize;
    GWEN_CRYPT_KEY *sk=NULL;

    switch (rxhProtocol) {
      case AH_CryptMode_Rdh: decKeySize=16; break;
      case AH_CryptMode_Rah: decKeySize=32; break;
      default: return NULL;
    }

    ksize=_prepareEncryptedKeyBuffer(ki, p, l, encKey, sizeof(encKey));
    p=_decryptAndUnpaddKey(ct, keyId, decKeySize, encKey, ksize, decKey, sizeof(decKey), gid);
    if (p==NULL) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here");
      return NULL;
    }

    switch (rxhProtocol) {
      case AH_CryptMode_Rdh: sk=GWEN_Crypt_KeyDes3K_fromData(GWEN_Crypt_CryptMode_Cbc, 24, p, 16); break;
      case AH_CryptMode_Rah: sk=GWEN_Crypt_KeyAes256_fromData(GWEN_Crypt_CryptMode_Cbc, 32, p, 32); break;
      default: sk=NULL; break;
    }
    if (sk==NULL) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create DES key from data");
      return NULL;
    }
    return sk;
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Missing message key");
    return NULL;
  }
}



GWEN_CRYPT_TOKEN *_getAndOpenCryptToken(AH_HBCI *h, AB_USER *u, uint32_t gid)
{
  GWEN_CRYPT_TOKEN *ct=NULL;
  int rv;

  /* get crypt token of signer */
  rv=AB_Banking_GetCryptToken(AH_HBCI_GetBankingApi(h), AH_User_GetTokenType(u), AH_User_GetTokenName(u), &ct);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not get crypt token for user \"%s\" (%d)", AB_User_GetUserId(u), rv);
    return NULL;
  }

  /* open CryptToken if necessary */
  if (!GWEN_Crypt_Token_IsOpen(ct)) {
    GWEN_Crypt_Token_AddModes(ct, GWEN_CRYPT_TOKEN_MODE_DIRECT_SIGN);
    rv=GWEN_Crypt_Token_Open(ct, 0, gid);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Could not open crypt token for user \"%s\" (%d)", AB_User_GetUserId(u), rv);
      return NULL;
    }
  }
  return ct;
}



const GWEN_CRYPT_TOKEN_KEYINFO *_getDecipherKeyInfo(AB_USER *u, GWEN_CRYPT_TOKEN *ct, uint32_t gid)
{
  const char *sTokenType;
  const char *sTokenName;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  const GWEN_CRYPT_TOKEN_KEYINFO *ki;
  uint32_t keyId;

  /* get token info for logging */
  sTokenType=GWEN_Crypt_Token_GetTypeName(ct);
  sTokenName=GWEN_Crypt_Token_GetTokenName(ct);

  /* get context and key info */
  ctx=GWEN_Crypt_Token_GetContext(ct, AH_User_GetTokenContextId(u), gid);
  if (ctx==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Context %d not found on crypt token [%s:%s]",
	     AH_User_GetTokenContextId(u), sTokenType, sTokenName);
    return NULL;
  }

  keyId=GWEN_Crypt_Token_Context_GetDecipherKeyId(ctx);
  ki=GWEN_Crypt_Token_GetKeyInfo(ct, keyId, 0xffffffff, gid);
  if (ki==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Keyinfo %04x not found on crypt token [%s:%s]", keyId, sTokenType, sTokenName);
    return NULL;
  }

  return ki;
}



uint32_t _prepareEncryptedKeyBuffer(const GWEN_CRYPT_TOKEN_KEYINFO *ki,
				    const uint8_t *srcPtr, uint32_t srcLen, uint8_t *destPtr, uint32_t destLen)
{
  uint32_t keySize;

  keySize=GWEN_Crypt_Token_KeyInfo_GetKeySize(ki);
  if (keySize<srcLen) {
    DBG_WARN(AQHBCI_LOGDOMAIN, "Keyinfo keysize is smaller than size of transmitted key, adjusting");
    keySize=srcLen;
  }
  assert(keySize<=AH_MSGRXH_MAXKEYBUF);
  
  /* fill encoded key with 0 to the total length of our private key */
  memset(destPtr, 0, destLen);
  memmove(destPtr+(keySize-srcLen), srcPtr, srcLen);
  return keySize;
}



const uint8_t *_decryptAndUnpaddKey(GWEN_CRYPT_TOKEN *ct, uint32_t keyId, int decKeySize,
				    const uint8_t *srcPtr, uint32_t srcLen,
				    uint8_t *destPtr, uint32_t destLen,
				    uint32_t gid)
{
  const uint8_t *p;
  uint32_t decodedLen;
  GWEN_CRYPT_PADDALGO *algo;
  int rv;

  algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_None);
  decodedLen=destLen;
  rv=GWEN_Crypt_Token_Decipher(ct, keyId, algo, srcPtr, srcLen, destPtr, &decodedLen, gid);
  GWEN_Crypt_PaddAlgo_free(algo);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return NULL;
  }
  
  /* unpadd */
  if (decodedLen<decKeySize) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Decrypted data too small for a key (%d < %d)", decodedLen, decKeySize);
    return NULL;
  }
  p=destPtr+(decodedLen-decKeySize);

#if 0
    DBG_ERROR(AQHBCI_LOGDOMAIN,
	      "DES key provided in message (padded key size=%d, unpadded keysize=%d, keyPos=%d):",
	      decodedLen, decKeySize, (decodedLen-decKeySize));
    GWEN_Text_LogString((const char *)destPtr, decodedLen, AQHBCI_LOGDOMAIN, GWEN_LoggerLevel_Error);
#endif

  return p;
}



GWEN_BUFFER *_rxhDecrypt_GetDecryptedMessage(GWEN_CRYPT_KEY *sk, int rxhProtocol, const uint8_t *pSource,
					     uint32_t lSource)
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



