/***************************************************************************
    begin       : Tue Nov 25 2008
    copyright   : (C) 2020 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static GWEN_CRYPT_KEY *_rxhDecrypt_ExtractMessageKey(AH_MSG *hmsg, int rxhProtocol, GWEN_DB_NODE *grHead);
static GWEN_BUFFER *_rxhDecrypt_GetDecryptedMessage(GWEN_CRYPT_KEY *sk, int rxhProtocol, const uint8_t *pSource,
                                                    uint32_t lSource);



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
  RXH_PARAMETER *rxh_parameter;
  int rxhVersion;

  assert(hmsg);
  h=AH_Dialog_GetHbci(hmsg->dialog);
  assert(h);

  u=AH_Dialog_GetDialogOwner(hmsg->dialog);

  /* get correct parameters */
  rxhVersion = AH_User_GetRdhType(u);
  switch (AH_User_GetCryptMode(u)) {
  case AH_CryptMode_Rdh:
    rxh_parameter=rdh_parameter[rxhVersion];
    if (rxh_parameter == NULL) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Profile RDH%d is not supported!", rxhVersion);
      return AB_ERROR_NOT_INIT;
    }
    break;
  case AH_CryptMode_Rah:
    rxh_parameter=rah_parameter[rxhVersion];
    if (rxh_parameter == NULL) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Profile RAH%d is not supported!", rxhVersion);
      return AB_ERROR_NOT_INIT;
    }
    break;
  default:
    return GWEN_ERROR_INTERNAL;
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
  GWEN_Buffer_free(hmsg->origbuffer);
  hmsg->origbuffer=hmsg->buffer;
  GWEN_Buffer_Rewind(mbuf);
  hmsg->buffer=mbuf;

  return 0;
}



GWEN_CRYPT_KEY *_rxhDecrypt_ExtractMessageKey(AH_MSG *hmsg, int rxhProtocol, GWEN_DB_NODE *grCryptHead)
{
  AH_HBCI *h;
  uint32_t l;
  int rv;
  const uint8_t *p;
  AB_USER *u;
  //  uint32_t uFlags;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  const char *sTokenType;
  const char *sTokenName;
  const GWEN_CRYPT_TOKEN_KEYINFO *ki;
  uint32_t keyId;
  GWEN_CRYPT_KEY *sk=NULL;
  uint8_t decKey[AH_MSGRXH_MAXKEYBUF+64];
  uint32_t gid;
  uint8_t decKeySize;


  u=AH_Dialog_GetDialogOwner(hmsg->dialog);

  assert(hmsg);
  h=AH_Dialog_GetHbci(hmsg->dialog);
  assert(h);
  gid=0;


  /* get crypt token of signer */
  rv=AB_Banking_GetCryptToken(AH_HBCI_GetBankingApi(h),
                              AH_User_GetTokenType(u),
                              AH_User_GetTokenName(u),
                              &ct);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not get crypt token for user \"%s\" (%d)", AB_User_GetUserId(u), rv);
    return NULL;
  }

  /* get token info for logging */
  sTokenType=GWEN_Crypt_Token_GetTypeName(ct);
  sTokenName=GWEN_Crypt_Token_GetTokenName(ct);

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
    DBG_INFO(AQHBCI_LOGDOMAIN, "Context %d not found on crypt token [%s:%s]", AH_User_GetTokenContextId(u), sTokenType,
             sTokenName);
    return NULL;
  }

  keyId=GWEN_Crypt_Token_Context_GetDecipherKeyId(ctx);
  ki=GWEN_Crypt_Token_GetKeyInfo(ct, keyId, 0xffffffff, gid);
  if (ki==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Keyinfo %04x not found on crypt token [%s:%s]", keyId, sTokenType, sTokenName);
    return NULL;
  }

  /* get encrypted session key */
  p=GWEN_DB_GetBinValue(grCryptHead, "CryptAlgo/MsgKey", 0, 0, 0, &l);
  if (p && l) {
    uint32_t elen;
    GWEN_CRYPT_PADDALGO *algo;
    uint8_t encKey[AH_MSGRXH_MAXKEYBUF+64];
    int ksize;

    ksize=GWEN_Crypt_Token_KeyInfo_GetKeySize(ki);
    if (ksize<l) {
      DBG_WARN(AQHBCI_LOGDOMAIN, "Keyinfo keysize is smaller than size of transmitted key, adjusting");
      ksize=l;
    }
    assert(ksize<=AH_MSGRXH_MAXKEYBUF);

    /* fill encoded key with 0 to the total length of our private key */
    memset(encKey, 0, sizeof(encKey));
    memmove(encKey+(ksize-l), p, l);

    algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_None);
    elen=sizeof(decKey);
    rv=GWEN_Crypt_Token_Decipher(ct, keyId, algo, encKey, ksize, decKey, &elen, gid);
    GWEN_Crypt_PaddAlgo_free(algo);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return NULL;
    }

    switch (rxhProtocol) {
    case AH_CryptMode_Rdh:
      decKeySize=16;
      break;
    case AH_CryptMode_Rah:
      decKeySize=32;
      break;
    default:
      return NULL;
    }

    /* unpadd and generate key */
    p=decKey+(elen-decKeySize);

#if 0
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "DES key provided in message (padded key size=%d, unpadded keysize=%d, keyPos=%d):",
              elen, decKeySize, (elen-decKeySize));
    GWEN_Text_LogString((const char *)decKey, elen, AQHBCI_LOGDOMAIN, GWEN_LoggerLevel_Error);
#endif

    switch (rxhProtocol) {
    case AH_CryptMode_Rdh:
      sk=GWEN_Crypt_KeyDes3K_fromData(GWEN_Crypt_CryptMode_Cbc, 24, p, 16);
      break;
    case AH_CryptMode_Rah:
      sk=GWEN_Crypt_KeyAes256_fromData(GWEN_Crypt_CryptMode_Cbc, 32, p, 32);
      break;
    default:
      return NULL;
    }
    if (sk==NULL) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create DES key from data");
      return NULL;
    }
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Missing message key");
    return NULL;
  }

  return sk;
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
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error unpadding this message (%d)", rv);
    GWEN_Buffer_Dump(mbuf, 2);
    GWEN_Buffer_free(mbuf);
    return NULL;
  }

  return mbuf;
}



