/***************************************************************************
    begin       : Tue Nov 25 2008
    copyright   : (C) 2023 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "msgcrypt_rxh_encrypt.h"
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
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static const GWEN_CRYPT_TOKEN_CONTEXT *_getUserContext(AH_MSG *hmsg);
static int _paddMessageAccordingToParams(GWEN_BUFFER *buffer, const RXH_PARAMETER *rParams);
static GWEN_CRYPT_KEY *_genMsgKeyAccordingToParams(const RXH_PARAMETER *rParams);
static GWEN_BUFFER *_encryptMessageIntoReturnedBuffer(GWEN_CRYPT_KEY *sk, const uint8_t *msgPtr, uint32_t msgLen);
static int _encryptMessageKey(GWEN_CRYPT_KEY *ek,
                              const RXH_PARAMETER *rParams,
                              const GWEN_CRYPT_KEY *sk,
                              uint8_t *encKeyBufferPtr, uint32_t encKeyBufferSize);
static GWEN_BUFFER *_sessionKeyToBuffer(const GWEN_CRYPT_KEY *sk, const RXH_PARAMETER *rParams);
static int _paddMessageKey(GWEN_BUFFER *skbuf, const RXH_PARAMETER *rParams, int encryptionKeySize);
static int _writeCtyptHead(AH_MSG *hmsg,
                           const RXH_PARAMETER *rParams,
                           const GWEN_CRYPT_TOKEN_CONTEXT *ctx,
                           GWEN_CRYPT_KEY *ek,
                           const uint8_t *encKey, int encKeyLen,
                           GWEN_BUFFER *hbuf);
static int _writeCryptData(GWEN_MSGENGINE *e, const uint8_t *encryptedMsgPtr, int encryptedMsgLen, GWEN_BUFFER *hbuf);
static GWEN_BUFFER *_composeMessage(AH_MSG *hmsg,
                                    const RXH_PARAMETER *rParams,
                                    const GWEN_CRYPT_TOKEN_CONTEXT *ctx,
                                    GWEN_BUFFER *mbuf,
                                    GWEN_CRYPT_KEY *ek,
                                    const uint8_t *encKey, int encKeyLen);

static void _dumpDesKey(GWEN_CRYPT_KEY *sk, const char *sText);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */

int AH_Msg_EncryptRxh(AH_MSG *hmsg)
{
  GWEN_BUFFER *mbuf;
  GWEN_BUFFER *hbuf;
  int rv;
  GWEN_MSGENGINE *e;
  AB_USER *u;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  GWEN_CRYPT_KEY *sk, *ek;
  uint8_t encKey[AH_MSGRXH_MAXKEYBUF+64];
  int encKeyLen;
  const RXH_PARAMETER *rParams;

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "RXH-encrypting message");
  assert(hmsg);
  u=AH_Dialog_GetDialogOwner(hmsg->dialog);

  rParams=AH_MsgRxh_GetParameters(AH_User_GetCryptMode(u), AH_User_GetRdhType(u));
  if (rParams==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No matching RxH parameters");
    return GWEN_ERROR_GENERIC;
  }

  e=AH_Dialog_GetMsgEngine(hmsg->dialog);
  assert(e);
  GWEN_MsgEngine_SetMode(e, AH_CryptMode_toString(rParams->protocol));

  ctx=_getUserContext(hmsg);
  if (ctx==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error getting user context");
    return GWEN_ERROR_GENERIC;
  }

  ek=AH_User_GetBankPubCryptKey(u);
  if (!ek) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Bank Public Key not available, please download it first, e.g. with aqhbci-tool4 getkeys -u %lu",
              (unsigned long int) AB_User_GetUniqueId(u));
    GWEN_Gui_ProgressLog2(0,
                          GWEN_LoggerLevel_Error,
                          I18N("The public key from the bank is not available, please download it first, e.g. with "
                               "aqhbci-tool4 getkeys -u %lu"),
                          (unsigned long int) AB_User_GetUniqueId(u));
    return GWEN_ERROR_GENERIC;
  }


  rv=_paddMessageAccordingToParams(hmsg->buffer, rParams);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  sk=_genMsgKeyAccordingToParams(rParams);
  if (sk==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not generate message key");
    return GWEN_ERROR_GENERIC;
  }

  mbuf=_encryptMessageIntoReturnedBuffer(sk, (uint8_t *)GWEN_Buffer_GetStart(hmsg->buffer), GWEN_Buffer_GetUsedBytes(hmsg->buffer));
  if (mbuf==NULL) {
    GWEN_Crypt_Key_free(sk);
    return GWEN_ERROR_GENERIC;
  }

  rv=_encryptMessageKey(ek, rParams, sk, encKey, sizeof(encKey));
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(mbuf);
    GWEN_Crypt_Key_free(sk);
  }
  encKeyLen=rv;
  GWEN_Crypt_Key_free(sk);

  hbuf=_composeMessage(hmsg, rParams, ctx, mbuf, ek, encKey, encKeyLen);
  if (hbuf==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    GWEN_Buffer_free(mbuf);
    return GWEN_ERROR_INTERNAL;
  }

  /* replace existing buffer by encrypted one */
  GWEN_Buffer_free(hmsg->buffer);
  hmsg->buffer=hbuf;

  return 0;
}



const GWEN_CRYPT_TOKEN_CONTEXT *_getUserContext(AH_MSG *hmsg)
{
  AH_HBCI *h;
  AB_USER *u;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  int rv;
  uint32_t gid;

  h=AH_Dialog_GetHbci(hmsg->dialog);
  assert(h);
  u=AH_Dialog_GetDialogOwner(hmsg->dialog);
  assert(u);
  gid=0;

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

  /* get context and key info */
  ctx=GWEN_Crypt_Token_GetContext(ct, AH_User_GetTokenContextId(u), gid);
  if (ctx==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Context %d not found on crypt token [%s:%s]",
             AH_User_GetTokenContextId(u),
             GWEN_Crypt_Token_GetTypeName(ct),
             GWEN_Crypt_Token_GetTokenName(ct));
    return NULL;
  }

  return ctx;
}



int _paddMessageAccordingToParams(GWEN_BUFFER *buffer, const RXH_PARAMETER *rParams)
{
  int rv;

  switch (rParams->protocol) {
  case AH_CryptMode_Rdh:
    DBG_INFO(AQHBCI_LOGDOMAIN, "Padding message with ANSI X9.23");
    rv=GWEN_Padd_PaddWithAnsiX9_23(buffer);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error padding message with ANSI X9.23 (%d)", rv);
      return rv;
    }
    break;

  case AH_CryptMode_Rah:
    DBG_INFO(AQHBCI_LOGDOMAIN, "Padding message with ZKA method");
    rv=GWEN_Padd_PaddWithZka(buffer);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error padding message with ZKA padding (%d)", rv);
      return rv;
    }
    break;

  default:
    DBG_INFO(AQHBCI_LOGDOMAIN, "Protocol not supported!");
    return GWEN_ERROR_INTERNAL;
  }

  return 0;
}



GWEN_CRYPT_KEY *_genMsgKeyAccordingToParams(const RXH_PARAMETER *rParams)
{
  GWEN_CRYPT_KEY *sk;

  switch (rParams->protocol) {
  case AH_CryptMode_Rdh:
    sk=GWEN_Crypt_KeyDes3K_Generate(GWEN_Crypt_CryptMode_Cbc, 24, 2);
    if (sk==NULL) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Could not generate DES key");
      return NULL;
    }
    _dumpDesKey(sk, "DES Key for message");
    break;

  case AH_CryptMode_Rah:
    sk=GWEN_Crypt_KeyAes256_Generate(GWEN_Crypt_CryptMode_Cbc, 32, 2);
    if (sk==NULL) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Could not generate AES-256 key");
      return NULL;
    }
    break;

  default:
    DBG_INFO(AQHBCI_LOGDOMAIN, "Protocol not supported!");
    return NULL;
  }

  return sk;
}



GWEN_BUFFER *_encryptMessageIntoReturnedBuffer(GWEN_CRYPT_KEY *sk, const uint8_t *msgPtr, uint32_t msgLen)
{
  GWEN_BUFFER *mbuf;
  uint32_t l;
  int rv;

  mbuf=GWEN_Buffer_new(0, msgLen, 0, 1);
  l=msgLen;
  rv=GWEN_Crypt_Key_Encipher(sk, msgPtr, msgLen, (uint8_t *)GWEN_Buffer_GetPosPointer(mbuf), &l);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not encipher with DES session key (%d)", rv);
    GWEN_Buffer_free(mbuf);
    return NULL;
  }
  GWEN_Buffer_IncrementPos(mbuf, l);
  GWEN_Buffer_AdjustUsedBytes(mbuf);
  return mbuf;
}



int _encryptMessageKey(GWEN_CRYPT_KEY *ek,
                       const RXH_PARAMETER *rParams,
                       const GWEN_CRYPT_KEY *sk,
                       uint8_t *encKeyBufferPtr, uint32_t encKeyBufferSize)
{
  uint32_t elen;
  GWEN_BUFFER *skbuf;
  int rv;

  elen=GWEN_Crypt_Key_GetKeySize(ek);
  if (encKeyBufferSize<elen) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "destination buffer too small (%d < %d)", encKeyBufferSize, elen);
    return GWEN_ERROR_GENERIC;
  }

  skbuf=_sessionKeyToBuffer(sk, rParams);
  if (skbuf==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return GWEN_ERROR_GENERIC;
  }

  rv=_paddMessageKey(skbuf, rParams, GWEN_Crypt_Key_GetKeySize(ek));
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=GWEN_Crypt_Key_Encipher(ek, (const uint8_t *) GWEN_Buffer_GetStart(skbuf), elen, encKeyBufferPtr, &elen);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(skbuf);
    return rv;
  }
  GWEN_Buffer_free(skbuf);

  return elen;
}



GWEN_BUFFER *_sessionKeyToBuffer(const GWEN_CRYPT_KEY *sk, const RXH_PARAMETER *rParams)
{
  GWEN_BUFFER *skbuf;

  skbuf=GWEN_Buffer_new(0, 512, 0, 1);
  switch (rParams->protocol) {
  case AH_CryptMode_Rdh:
    GWEN_Buffer_InsertBytes(skbuf, (const char *) GWEN_Crypt_KeyDes3K_GetKeyDataPtr(sk), 16);
    break;
  case AH_CryptMode_Rah:
    GWEN_Buffer_InsertBytes(skbuf, (const char *) GWEN_Crypt_KeyAes256_GetKeyDataPtr(sk), 32);
    break;
  default:
    GWEN_Buffer_free(skbuf);
    return NULL;
  }
  GWEN_Buffer_Rewind(skbuf);
  return skbuf;
}



int _paddMessageKey(GWEN_BUFFER *skbuf, const RXH_PARAMETER *rParams, int encryptionKeySize)
{
  GWEN_CRYPT_PADDALGO *algo;
  int rv;

  switch (rParams->opmodCrypt) {
  case AH_Opmode_Cbc:
    algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_LeftZero);
    break;
  case AH_Opmode_Rsa_Pkcs1_v1_5:
    algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_Pkcs1_2);
    break;
  default:
    return GWEN_ERROR_INTERNAL;
  }

  GWEN_Crypt_PaddAlgo_SetPaddSize(algo, encryptionKeySize);
  /* padd according to given algo */
  rv=GWEN_Padd_ApplyPaddAlgo(algo, skbuf);
  GWEN_Crypt_PaddAlgo_free(algo);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  return 0;
}



void _dumpDesKey(GWEN_CRYPT_KEY *sk, const char *sText)
{
  const uint8_t *p;
  uint32_t len;

  p=GWEN_Crypt_KeyDes3K_GetKeyDataPtr(sk);
  len=GWEN_Crypt_KeyDes3K_GetKeyDataLen(sk);
  DBG_INFO(AQHBCI_LOGDOMAIN, "%s (len=%d)", sText?sText:"KEY", len);
  GWEN_Text_LogString((const char *)p, len, AQHBCI_LOGDOMAIN, GWEN_LoggerLevel_Info);
}



GWEN_BUFFER *_composeMessage(AH_MSG *hmsg,
                             const RXH_PARAMETER *rParams,
                             const GWEN_CRYPT_TOKEN_CONTEXT *ctx,
                             GWEN_BUFFER *mbuf,
                             GWEN_CRYPT_KEY *ek,
                             const uint8_t *encKey, int encKeyLen)
{
  GWEN_BUFFER *hbuf;
  int rv;
  GWEN_MSGENGINE *e;

  e=AH_Dialog_GetMsgEngine(hmsg->dialog);

  hbuf=GWEN_Buffer_new(0, 256+GWEN_Buffer_GetUsedBytes(mbuf), 0, 1);
  rv=_writeCtyptHead(hmsg, rParams, ctx, ek, encKey, encKeyLen, hbuf);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(hbuf);
    return NULL;
  }

  rv=_writeCryptData(e, (const uint8_t*) GWEN_Buffer_GetStart(mbuf), GWEN_Buffer_GetUsedBytes(mbuf), hbuf);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(hbuf);
    return NULL;
  }

  return hbuf;
}



int _writeCtyptHead(AH_MSG *hmsg,
                    const RXH_PARAMETER *rParams,
                    const GWEN_CRYPT_TOKEN_CONTEXT *ctx,
                    GWEN_CRYPT_KEY *ek,
                    const uint8_t *encKey, int encKeyLen,
                    GWEN_BUFFER *hbuf)
{
  GWEN_XMLNODE *node;
  GWEN_DB_NODE *cfg;
  GWEN_MSGENGINE *e;
  AB_USER *u;
  int rv;

  u=AH_Dialog_GetDialogOwner(hmsg->dialog);
  e=AH_Dialog_GetMsgEngine(hmsg->dialog);

  /* create crypt head */
  node=GWEN_MsgEngine_FindNodeByPropertyStrictProto(e, "SEG", "id", 0, "CryptHead");
  if (!node) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Segment \"CryptHead\" not found");
    return GWEN_ERROR_GENERIC;
  }

  /* create CryptHead */
  cfg=GWEN_DB_Group_new("crypthead");
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "head/seq", 998);

  rv=AH_MsgRxh_PrepareCryptoSeg(hmsg, u, rParams,
                                GWEN_Crypt_Key_GetKeyNumber(ek), GWEN_Crypt_Key_GetKeyVersion(ek),
                                NULL, cfg, 1, 0);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(cfg);
    return rv;
  }

  /* store system id */
  if (hmsg->noSysId) {
    GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecDetails/SecId", "0");
  }
  else {
    /* store CID if we use a card */
    const uint8_t *cidData;
    uint32_t cidLen;
    const char *p;

    cidLen=GWEN_Crypt_Token_Context_GetCidLen(ctx);
    cidData=GWEN_Crypt_Token_Context_GetCidPtr(ctx);
    if (cidLen > 0 && cidData != NULL) {
      GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecDetails/CID", cidData, cidLen);
    }

    p=AH_User_GetSystemId(u);
    if (p==NULL) {
      p=GWEN_Crypt_Token_Context_GetSystemId(ctx);
    }
    if (p) {
      GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecDetails/SecId", p);
    }
    else {
      GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecDetails/SecId", "0");
    }
  }

  /* store encrypted message key */
  GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT, "CryptAlgo/MsgKey", encKey, encKeyLen);

  rv=GWEN_MsgEngine_CreateMessageFromNode(e, node, hbuf, cfg);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not create CryptHead (%d)", rv);
    GWEN_DB_Group_free(cfg);
    return rv;
  }
  GWEN_DB_Group_free(cfg);

  return 0;
}



int _writeCryptData(GWEN_MSGENGINE *e, const uint8_t *encryptedMsgPtr, int encryptedMsgLen, GWEN_BUFFER *hbuf)
{
  GWEN_DB_NODE *cfg;
  GWEN_XMLNODE *node;
  int rv;

  /* create cryptdata */
  cfg=GWEN_DB_Group_new("cryptdata");
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "head/seq", 999);
  GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT, "cryptdata", encryptedMsgPtr, encryptedMsgLen);

  node=GWEN_MsgEngine_FindNodeByPropertyStrictProto(e, "SEG", "id", 0, "CryptData");
  if (!node) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Segment \"CryptData\"not found");
    GWEN_DB_Group_free(cfg);
    return GWEN_ERROR_GENERIC;
  }
  rv=GWEN_MsgEngine_CreateMessageFromNode(e, node, hbuf, cfg);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not create CryptData (%d)", rv);
    GWEN_DB_Group_free(cfg);
    return rv;
  }
  GWEN_DB_Group_free(cfg);

  return 0;
}



