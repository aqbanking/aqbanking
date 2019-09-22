/***************************************************************************
    begin       : Tue Jun 03 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


/*
 * This file is included by provider.c
 */




int AH_Provider_CheckCryptToken(AB_PROVIDER *pro,
                                GWEN_CRYPT_TOKEN_DEVICE devt,
                                GWEN_BUFFER *typeName,
                                GWEN_BUFFER *tokenName)
{
  GWEN_PLUGIN_MANAGER *pm;
  int rv;

  /* get crypt token */
  pm=GWEN_PluginManager_FindPluginManager(GWEN_CRYPT_TOKEN_PLUGIN_TYPENAME);
  if (pm==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "CryptToken plugin manager not found");
    return GWEN_ERROR_NOT_FOUND;
  }

  /* try to determine the type and name */
  rv=GWEN_Crypt_Token_PluginManager_CheckToken(pm, devt, typeName, tokenName, 0);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AH_Provider_CreateKeys(AB_PROVIDER *pro,
                           AB_USER *u,
                           int nounmount)
{
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  uint32_t keyId;
  GWEN_CRYPT_CRYPTALGO *algo;
  int rv;
  AH_HBCI *h;
  const GWEN_CRYPT_TOKEN_KEYINFO *oki;
  GWEN_CRYPT_TOKEN_KEYINFO *ki;
  int rdhType;
  int maxServerKeySizeInBits=0;

  h=AH_Provider_GetHbci(pro);
  assert(h);

  /* check crypt mode */
  if ((AH_User_GetCryptMode(u)!=AH_CryptMode_Rdh) && (AH_User_GetCryptMode(u)!=AH_CryptMode_Rah)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Key generation not supported with this token");
    return GWEN_ERROR_INVALID;
  }

  rdhType=AH_User_GetRdhType(u);
  if (rdhType==0)
    rdhType=1;

  /* get token */
  rv=AB_Banking_GetCryptToken(AH_HBCI_GetBankingApi(h),
                              AH_User_GetTokenType(u),
                              AH_User_GetTokenName(u),
                              &ct);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Error getting the user's crypt token (%d)", rv);
    return rv;
  }

  /* we always use 65537 as public exponent */
  GWEN_Crypt_Token_AddModes(ct, GWEN_CRYPT_TOKEN_MODE_EXP_65537);

  /* create algo */
  algo=GWEN_Crypt_CryptAlgo_new(GWEN_Crypt_CryptAlgoId_Rsa,
                                GWEN_Crypt_CryptMode_None);

  /* open token for admin */
  if (!GWEN_Crypt_Token_IsOpen(ct)) {
    rv=GWEN_Crypt_Token_Open(ct, 1, 0);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
                "Error opening crypt token (%d)", rv);
      GWEN_Crypt_CryptAlgo_free(algo);
      return rv;
    }
  }

  /* get context */
  ctx=GWEN_Crypt_Token_GetContext(ct, AH_User_GetTokenContextId(u), 0);
  if (ctx==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Could not get context %d", AH_User_GetTokenContextId(u));
    GWEN_Crypt_CryptAlgo_free(algo);
    return GWEN_ERROR_INVALID;
  }

  // XXX RAH?
  if ((AH_User_GetCryptMode(u)==AH_CryptMode_Rdh) && (rdhType==10)) {
    /* the specs say that for RDH-10 we must not create keys longer than the server's
     * sign key (or, if absent, the server's encipher key) */
    uint32_t skeyId;
    const GWEN_CRYPT_TOKEN_KEYINFO *ski;

    skeyId=GWEN_Crypt_Token_Context_GetVerifyKeyId(ctx);
    if (skeyId==0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
                "No verify key id specified (internal error)");
      GWEN_Crypt_CryptAlgo_free(algo);
      return GWEN_ERROR_INVALID;
    }

    ski=GWEN_Crypt_Token_GetKeyInfo(ct, skeyId,
                                    GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS,
                                    0);
    /* the fact that a key info exists does not mean that it contains key data (modulus and exp),
     * so we need to check for key data explicitly */
    if (!(ski && GWEN_Crypt_Token_KeyInfo_GetModulusData(ski) && GWEN_Crypt_Token_KeyInfo_GetModulusLen(ski))) {
      GWEN_Gui_ProgressLog(0,
                           GWEN_LoggerLevel_Notice,
                           I18N("Server has no sign key, using encipher key"));
      skeyId=GWEN_Crypt_Token_Context_GetEncipherKeyId(ctx);
      if (skeyId==0) {
        DBG_ERROR(AQHBCI_LOGDOMAIN,
                  "No encipher key id specified (internal error)");
        GWEN_Crypt_CryptAlgo_free(algo);
        return GWEN_ERROR_INVALID;
      }
      ski=GWEN_Crypt_Token_GetKeyInfo(ct, skeyId,
                                      GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS,
                                      0);
    }

    if (ski) {
      const uint8_t *modPtr;
      uint32_t modLen;

      modPtr=GWEN_Crypt_Token_KeyInfo_GetModulusData(ski);
      modLen=GWEN_Crypt_Token_KeyInfo_GetModulusLen(ski);

      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Server key has a modulus size of %d bytes", modLen);
      GWEN_Gui_ProgressLog2(0,
                            GWEN_LoggerLevel_Notice,
                            I18N("Server key has a modulus size of %d bytes"), modLen);
      if (modPtr && modLen) {
        /* The specs require us to use a key no longer than that of the server,
         * so for our key we use the largest multiple of 8 smaller or equal to
         * the length of the server key in order to keep everyone happy.
         */
        maxServerKeySizeInBits=modLen*8;
        while (modLen && *modPtr==0) {
          maxServerKeySizeInBits-=8;
          modLen--;
          modPtr++;
        }
        if (modLen && (*modPtr&0x80)==0)
          maxServerKeySizeInBits-=8;
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "Max Server Keysize in bits: %d",
                   maxServerKeySizeInBits);
      }
      else {
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "Key info for key %d has no modulus data, using default key size (2048 bits)",
                   (int) skeyId);
      }
    }
    else {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "No key info found for key %d", (int) skeyId);
    }
  }

  if (AH_User_GetCryptMode(u)==AH_CryptMode_Rdh) {
    switch (rdhType) {
    case 1:
      GWEN_Crypt_CryptAlgo_SetChunkSize(algo, 96);
      GWEN_Crypt_CryptAlgo_SetKeySizeInBits(algo, 768);
      break;
    case 2:
      GWEN_Crypt_CryptAlgo_SetChunkSize(algo, 256);
      GWEN_Crypt_CryptAlgo_SetKeySizeInBits(algo, 2048);
      break;
    case 3:
      GWEN_Crypt_CryptAlgo_SetChunkSize(algo, 256);
      GWEN_Crypt_CryptAlgo_SetKeySizeInBits(algo, 2048);
      break;
    case 5:
      GWEN_Crypt_CryptAlgo_SetChunkSize(algo, 256);
      GWEN_Crypt_CryptAlgo_SetKeySizeInBits(algo, 2048);
      break;
    case 7:
      GWEN_Crypt_CryptAlgo_SetChunkSize(algo, 256);
      GWEN_Crypt_CryptAlgo_SetKeySizeInBits(algo, 2048);
      break;
    case 8:
      GWEN_Crypt_CryptAlgo_SetChunkSize(algo, 256);
      GWEN_Crypt_CryptAlgo_SetKeySizeInBits(algo, 2048);
      break;
    case 9:
      GWEN_Crypt_CryptAlgo_SetChunkSize(algo, 256);
      GWEN_Crypt_CryptAlgo_SetKeySizeInBits(algo, 2048);
      break;
    case 10:
      if (maxServerKeySizeInBits) {
        int n=maxServerKeySizeInBits/8;

        assert(maxServerKeySizeInBits%8==0);
        GWEN_Crypt_CryptAlgo_SetChunkSize(algo, n);
        GWEN_Crypt_CryptAlgo_SetKeySizeInBits(algo, maxServerKeySizeInBits);
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "Creating keys of size: %d bytes, %d bits", n, maxServerKeySizeInBits);
      }
      else {
        GWEN_Crypt_CryptAlgo_SetChunkSize(algo, 256);
        GWEN_Crypt_CryptAlgo_SetKeySizeInBits(algo, 2048);
      }
      break;
    default:
      DBG_ERROR(AQHBCI_LOGDOMAIN, "RDH %d not supported", AH_User_GetRdhType(u));
      return GWEN_ERROR_INVALID;
    }
  }
  if (AH_User_GetCryptMode(u)==AH_CryptMode_Rah) {
    switch (rdhType) {
    case 7:
      GWEN_Crypt_CryptAlgo_SetChunkSize(algo, 256);
      GWEN_Crypt_CryptAlgo_SetKeySizeInBits(algo, 2048);
      break;
    case 9:
      GWEN_Crypt_CryptAlgo_SetChunkSize(algo, 256);
      GWEN_Crypt_CryptAlgo_SetKeySizeInBits(algo, 2048);
      break;
    case 10:
      GWEN_Crypt_CryptAlgo_SetChunkSize(algo, 256);
      GWEN_Crypt_CryptAlgo_SetKeySizeInBits(algo, 2048);
      break;
    default:
      DBG_ERROR(AQHBCI_LOGDOMAIN, "RAH %d not supported", AH_User_GetRdhType(u));
      return GWEN_ERROR_INVALID;
    }
  }

  GWEN_Gui_ProgressLog2(0,
                        GWEN_LoggerLevel_Notice,
                        I18N("Creating keys with %d bits (%d bytes), please wait..."),
                        GWEN_Crypt_CryptAlgo_GetKeySizeInBits(algo),
                        GWEN_Crypt_CryptAlgo_GetChunkSize(algo));

  /* get cipher key id */
  keyId=GWEN_Crypt_Token_Context_GetDecipherKeyId(ctx);
  if (keyId==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "No decipher key id specified (internal error)");
    GWEN_Crypt_CryptAlgo_free(algo);
    return GWEN_ERROR_INVALID;
  }

  /* generate cipher key */
  rv=GWEN_Crypt_Token_GenerateKey(ct, keyId, algo, 0);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Error generating key (%d)", rv);
    GWEN_Gui_ProgressLog2(0,
                          GWEN_LoggerLevel_Error,
                          I18N("Error creating cipher key (%d)"), rv);
    GWEN_Crypt_CryptAlgo_free(algo);
    return rv;
  }

  /* set key number/version */
  oki=GWEN_Crypt_Token_GetKeyInfo(ct, keyId,
                                  GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
                                  GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER,
                                  0);
  if (oki==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Could not get KeyInfo for newly created key %d", keyId);
    GWEN_Crypt_CryptAlgo_free(algo);
    return rv;
  }
  ki=GWEN_Crypt_Token_KeyInfo_dup(oki);
  if (rdhType>1)
    GWEN_Crypt_Token_KeyInfo_SetKeyNumber(ki, rdhType);
  else
    GWEN_Crypt_Token_KeyInfo_SetKeyNumber(ki, 1);
  GWEN_Crypt_Token_KeyInfo_SetKeyVersion(ki, 1);
  GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
                                    GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
                                    GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER);
  rv=GWEN_Crypt_Token_SetKeyInfo(ct, keyId, ki, 0);
  GWEN_Crypt_Token_KeyInfo_free(ki);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Crypt_CryptAlgo_free(algo);
    return rv;
  }

  /* get sign key id */
  keyId=GWEN_Crypt_Token_Context_GetSignKeyId(ctx);
  if (keyId==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "No sign key id specified (internal error)");
    GWEN_Crypt_CryptAlgo_free(algo);
    return GWEN_ERROR_INVALID;
  }

  /* generate sign key */
  rv=GWEN_Crypt_Token_GenerateKey(ct, keyId, algo, 0);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Error generating key (%d)", rv);
    GWEN_Gui_ProgressLog2(0,
                          GWEN_LoggerLevel_Error,
                          I18N("Error creating sign key (%d)"), rv);
    GWEN_Crypt_CryptAlgo_free(algo);
    return rv;
  }

  /* set key number/version */
  oki=GWEN_Crypt_Token_GetKeyInfo(ct, keyId,
                                  GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
                                  GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER,
                                  0);
  if (oki==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Could not get KeyInfo for newly created key %d", keyId);
    GWEN_Crypt_CryptAlgo_free(algo);
    return rv;
  }
  ki=GWEN_Crypt_Token_KeyInfo_dup(oki);
  if (rdhType>1)
    GWEN_Crypt_Token_KeyInfo_SetKeyNumber(ki, rdhType);
  else {
    if (AH_User_GetHbciVersion(u)>=300)
      GWEN_Crypt_Token_KeyInfo_SetKeyNumber(ki, 1);
    else
      GWEN_Crypt_Token_KeyInfo_SetKeyNumber(ki, 2);
  }
  GWEN_Crypt_Token_KeyInfo_SetKeyVersion(ki, 1);
  GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
                                    GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
                                    GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER);
  rv=GWEN_Crypt_Token_SetKeyInfo(ct, keyId, ki, 0);
  GWEN_Crypt_Token_KeyInfo_free(ki);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Crypt_CryptAlgo_free(algo);
    return rv;
  }

  /* get auth sign key id */
  keyId=GWEN_Crypt_Token_Context_GetAuthSignKeyId(ctx);
  if (keyId) {
    /* generate auth sign key */
    rv=GWEN_Crypt_Token_GenerateKey(ct, keyId, algo, 0);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
                "Error generating key (%d)", rv);
      GWEN_Gui_ProgressLog2(0,
                            GWEN_LoggerLevel_Error,
                            I18N("Error creating auth key (%d)"), rv);
      GWEN_Crypt_CryptAlgo_free(algo);
      return rv;
    }

    /* set key number/version */
    oki=GWEN_Crypt_Token_GetKeyInfo(ct, keyId,
                                    GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
                                    GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER,
                                    0);
    if (oki==NULL) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
                "Could not get KeyInfo for newly created key %d", keyId);
      GWEN_Crypt_CryptAlgo_free(algo);
      return rv;
    }
    ki=GWEN_Crypt_Token_KeyInfo_dup(oki);
    if (rdhType>1)
      GWEN_Crypt_Token_KeyInfo_SetKeyNumber(ki, rdhType);
    else {
      if (AH_User_GetHbciVersion(u)>=300)
        GWEN_Crypt_Token_KeyInfo_SetKeyNumber(ki, 1);
      else
        GWEN_Crypt_Token_KeyInfo_SetKeyNumber(ki, 3);
    }
    GWEN_Crypt_Token_KeyInfo_SetKeyVersion(ki, 1);
    GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
                                      GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
                                      GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER);
    rv=GWEN_Crypt_Token_SetKeyInfo(ct, keyId, ki, 0);
    GWEN_Crypt_Token_KeyInfo_free(ki);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      GWEN_Crypt_CryptAlgo_free(algo);
      return rv;
    }
  }

  if (!nounmount) {
    /* close token */
    rv=GWEN_Crypt_Token_Close(ct, 0, 0);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
                "Error closing crypt token (%d)", rv);
      GWEN_Crypt_CryptAlgo_free(algo);
      return rv;
    }
  }

  GWEN_Crypt_CryptAlgo_free(algo);
  return 0;
}

// XXX hash-len from algo-id?
#define HASHOUT_MAXLEN  32

struct AH_KEYHASH
{
  uint8_t *hash;
  uint32_t hashLen;
  uint8_t *hashCard;
  uint32_t hashCardLen;
  char *hname;
  uint8_t *e;
  uint32_t eLen;
  uint8_t *m;
  uint32_t mLen;
  int kn;
  int kv;
};

struct AH_KEYHASH *AH_Provider_KeyHash_new()
{
  struct AH_KEYHASH *kh = malloc(sizeof(struct AH_KEYHASH));
  kh->hash = kh->hashCard = kh->e = kh->m = NULL;
  kh->hashLen = kh->hashCardLen = kh->eLen = kh->mLen = 0;
  kh->hname = NULL;
  kh->kn = kh->kv = 0;
  return kh;
}

const uint8_t *AH_Provider_KeyHash_Hash(struct AH_KEYHASH *kh, uint32_t *l)
{
  *l = kh->hashLen;
  return kh->hash;
}

const uint8_t *AH_Provider_KeyHash_HashCard(struct AH_KEYHASH *kh, uint32_t *l)
{
  *l = kh->hashCardLen;
  return kh->hashCard;
}

const uint8_t *AH_Provider_KeyHash_Exponent(struct AH_KEYHASH *kh, uint32_t *l)
{
  *l = kh->eLen;
  return kh->e;
}

const uint8_t *AH_Provider_KeyHash_Modulus(struct AH_KEYHASH *kh, uint32_t *l)
{
  *l = kh->mLen;
  return kh->m;
}

void AH_Provider_KeyHash_Info(struct AH_KEYHASH *kh, int *kn, int *kv, const char **hn)
{
  if(kn)
    *kn = kh->kn;
  if(kv)
    *kv = kh->kv;
  if(hn)
  {
    if(!kh->hname)
    {
      kh->hname = malloc(2);
      kh->hname[0] = '?';
      kh->hname[1] = 0;
    }
    *hn = kh->hname;
  }
}

void AH_Provider_KeyHash_free(struct AH_KEYHASH *kh)
{
  free(kh->hash);
  free(kh->hashCard);
  free(kh->hname);
  free(kh);
}

int AH_Provider_GetKeyHash(AB_PROVIDER *pro, AB_USER *u, GWEN_CRYPT_KEY *bk, char keyType, int cryptMode, int rdhType,
                                      const char *tokenType, const char *tokenName, uint32_t tokenCtxId,
                                      const GWEN_CRYPT_TOKEN_CONTEXT *ctx, uint8_t bankKey, struct AH_KEYHASH *res)
{
  AH_HBCI *hbci = AH_Provider_GetHbci(pro);
  GWEN_CRYPT_HASHALGOID hAlgo = GWEN_Crypt_HashAlgoId_Unknown;
  uint32_t eLen = 0, mLen = 0, hLen = 0;
  uint8_t *mData = NULL, *eData = NULL;
  GWEN_BUFFER *hBuff = NULL;
  int rv = 0;

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "%s(): u %p bk %p kt %c cm %s-%d ctx %p, isBK %d.", __FUNCTION__,
              u, bk, keyType, AH_CryptMode_toString(cryptMode), rdhType, ctx, bankKey);

  if(!ctx && u)
  {
    tokenType = AH_User_GetTokenType(u);
    tokenName = AH_User_GetTokenName(u);
    cryptMode = AH_User_GetCryptMode(u);
    rdhType = AH_User_GetRdhType(u);
    tokenCtxId = AH_User_GetTokenContextId(u);
  }

  if(!res)
  {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Result-buffer not set.");
    return GWEN_ERROR_GENERIC;
  }

  if(bankKey)
  {
    if(!bk)
    {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Bank-key not set.");
      return GWEN_ERROR_GENERIC;
    }

    eLen = 3; // XXX
    mLen = GWEN_Crypt_Key_GetKeySize(bk);
    if(eLen && mLen)
    {
      res->kn = GWEN_Crypt_Key_GetKeyNumber(bk);
      res->kv = GWEN_Crypt_Key_GetKeyVersion(bk);
      mData = malloc(mLen);
      eData = malloc(eLen);
      rv = GWEN_Crypt_KeyRsa_GetModulus(bk, mData, &mLen);
      if(rv == 0)
        rv = GWEN_Crypt_KeyRsa_GetExponent(bk, eData, &eLen);
      if(rv != 0)
      {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad key.");
        rv = GWEN_ERROR_BAD_DATA;
      }
    }
  }

  if(!ctx)
  {
    GWEN_CRYPT_TOKEN *ct = NULL;
    const GWEN_CRYPT_TOKEN_KEYINFO *ki = NULL;
    const GWEN_CRYPT_TOKEN_CONTEXT *ctx = NULL;
    rv = AB_Banking_GetCryptToken(AH_HBCI_GetBankingApi(hbci), tokenType, tokenName, &ct);
    if((rv != 0) || !ct)
    {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not get crypt token (%d).", rv);
      rv = GWEN_ERROR_NOT_FOUND;
    }
    if(rv == 0)
    {
      rv = GWEN_Crypt_Token_Open(ct, 1, 0);
      if(rv != 0)
      {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "%s(): Could not open crypt token (%d).", __FUNCTION__, rv);
        rv = GWEN_ERROR_NOT_FOUND;
      }
    }
    if(rv == 0)
    {
      ctx = GWEN_Crypt_Token_GetContext(ct, tokenCtxId, 0);
      if(!ctx)
      {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "%s(): User context %d not found on crypt token.", __FUNCTION__, tokenCtxId);
        rv = GWEN_ERROR_NOT_FOUND;
      }
    }
    if((rv == 0) && ctx)
    {
      uint32_t kf = 0;
      uint32_t kid = 0;
      if(keyType == 'V')
        kid = GWEN_Crypt_Token_Context_GetEncipherKeyId(ctx);
      else if(keyType == 'S')
        kid = GWEN_Crypt_Token_Context_GetSignKeyId(ctx);
      else
      {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "%s(): Key-type '%c' invalid.", __FUNCTION__, keyType);
        rv = GWEN_ERROR_BAD_DATA;
      }
      if(rv == 0)
      {
        if(kid)
          ki = GWEN_Crypt_Token_GetKeyInfo(ct, kid, 0, 0);
        if(ki)
          kf = GWEN_Crypt_Token_KeyInfo_GetFlags(ki);
        if(!ki || !(kf & GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS) || !(kf & GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT))
        {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "%s(): User keys missing (kid %ld, f 0x%04lX).", __FUNCTION__, (long)kid, (long)kf);
          rv = GWEN_ERROR_NOT_FOUND;
        }
      }
    }
    if(!bankKey && (rv == 0) && ki)
    {
      const uint8_t *e = GWEN_Crypt_Token_KeyInfo_GetExponentData(ki);
      const uint8_t *m = GWEN_Crypt_Token_KeyInfo_GetModulusData(ki);
      eLen = GWEN_Crypt_Token_KeyInfo_GetExponentLen(ki);
      mLen = GWEN_Crypt_Token_KeyInfo_GetModulusLen(ki);
      if(!e || !eLen || !m || !mLen)
      {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "%s(): Bad key.", __FUNCTION__);
        rv = GWEN_ERROR_BAD_DATA;
      }
      else
      {
        res->kn = GWEN_Crypt_Token_KeyInfo_GetKeyNumber(ki);
        res->kv = GWEN_Crypt_Token_KeyInfo_GetKeyVersion(ki);
        mData = malloc(mLen);
        eData = malloc(eLen);
        memcpy(mData, m, mLen);
        memcpy(eData, e, eLen);
      }
    }
  }

  if((rv == 0) && ctx)
  {
    hAlgo = GWEN_Crypt_Token_Context_GetKeyHashAlgo(ctx);
    // a hash algo is found, if token is chipcard.
    // if so, that algo was selected regarding EF_NOTEPADs HBCI-Version
    // (RDH-6, 7, 8, 9, 10, RAH-7, 9, 10)
    // for sign-key, try load hash
    if(hAlgo != GWEN_Crypt_HashAlgoId_None)
    {
      const uint8_t *h = NULL;
      uint32_t hl = 0;
      if(keyType == 'S')
      {
        h = GWEN_Crypt_Token_Context_GetKeyHashPtr(ctx);
        hl = GWEN_Crypt_Token_Context_GetKeyHashLen(ctx);
      }
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Hash-algo from card %ld '%s', hash %p %ld.", (long)hAlgo, GWEN_Crypt_HashAlgoId_toString(hAlgo), h, (long)hl);
      switch(hAlgo)
      {
      case GWEN_Crypt_HashAlgoId_Rmd160:
        hLen = 128 * 2;
        break;
      case GWEN_Crypt_HashAlgoId_Sha256:
        hLen = mLen * 2;
        break;
      default:
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Unexpected hash-algo %ld '%s', hLen %ld.", (long)hAlgo, GWEN_Crypt_HashAlgoId_toString(hAlgo), (long)hLen);
        rv = GWEN_ERROR_BAD_DATA;
      }
      if(h && hl)
      {
        uint32_t hNum = GWEN_Crypt_Token_Context_GetKeyHashNum(ctx);
        uint32_t hVer = GWEN_Crypt_Token_Context_GetKeyHashVer(ctx);
        DBG_INFO(AQHBCI_LOGDOMAIN, "Found bank key hash on the zka card! (Hash Algo Identifier: [%d], keyNum: [%d], keyVer: [%d]", hAlgo, hNum, hVer);
        if((hNum == res->kn) && (hVer == res->kv))
        {
          DBG_INFO(AQHBCI_LOGDOMAIN, "Key Number and Version of the Hash match transmitted key.");
          res->hashCardLen = hl;
          res->hashCard = malloc(res->hashCardLen);
          if(!res->hashCard)
          {
            DBG_ERROR(AQHBCI_LOGDOMAIN, "allocate result-buffer (card) failed (%d).", rv);
            rv = GWEN_ERROR_GENERIC;
          }
          else
            memcpy(res->hashCard, h, res->hashCardLen);
        }
      }
    }
  }

  if((rv == 0) && ((hAlgo == GWEN_Crypt_HashAlgoId_None) || (hAlgo == GWEN_Crypt_HashAlgoId_Unknown)))
  {
    switch(cryptMode)
    {
    case AH_CryptMode_Rdh:
      switch(rdhType)
      {
      case 1:
        hLen = 128 * 2;
        hAlgo = GWEN_Crypt_HashAlgoId_Rmd160;
        break;
      case 2:
      case 3:
      case 5:
        hLen = mLen * 2;
        hAlgo = GWEN_Crypt_HashAlgoId_Rmd160;
        break;
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
        hAlgo = GWEN_Crypt_HashAlgoId_Sha256;
        hLen = mLen * 2;
        break;
      default:;
      }
    case AH_CryptMode_Rah:
      switch(rdhType)
      {
      case 7:
      case 9:
      case 10:
        hAlgo = GWEN_Crypt_HashAlgoId_Sha256;
        hLen = mLen * 2;
        break;
      default:;
      }
    default:;
    }
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Hash-algo selected (%s %d) %ld '%s', hLen %ld.",
               AH_CryptMode_toString(cryptMode), rdhType, (long)hAlgo, GWEN_Crypt_HashAlgoId_toString(hAlgo), (long)hLen);
  }

  switch(hAlgo)
  {
  case GWEN_Crypt_HashAlgoId_Rmd160:
    res->hname = malloc(11);
    snprintf(res->hname, 11, "RIPEMD-160");
    break;
  case GWEN_Crypt_HashAlgoId_Sha256:
    res->hname = malloc(7);
    snprintf(res->hname, 7, "SHA256");
    break;
  default:;
  }

  if((rv == 0) && ((mLen > (hLen / 2)) || (eLen > (hLen / 2))))
  {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Invalid size for e, m, or h (%ld, %ld, %ld).", (long)eLen, (long)mLen, (long)hLen);
    rv = GWEN_ERROR_GENERIC;
  }

  if(rv == 0)
  {
    if(hLen && eLen && mLen)
      hBuff = GWEN_Buffer_new(0, hLen, 0, 1);
    if(hBuff)
    {
      GWEN_Buffer_FillWithBytes(hBuff, 0, (hLen / 2) - eLen);
      GWEN_Buffer_AppendBytes(hBuff, (const char*)eData, eLen);
      GWEN_Buffer_FillWithBytes(hBuff, 0, (hLen / 2) - mLen);
      GWEN_Buffer_AppendBytes(hBuff, (const char*)mData, mLen);
      GWEN_Buffer_Rewind(hBuff);
    }
  }

  if((rv == 0) && hBuff)
  {
    uint8_t hOutBuff[HASHOUT_MAXLEN];
    uint32_t hOutLen = 0;
    switch(hAlgo)
    {
    case GWEN_Crypt_HashAlgoId_Rmd160:
      //hName = "RMD-160";
      hOutLen = 20;
      rv = AH_Provider__HashRmd160((const uint8_t*)GWEN_Buffer_GetStart(hBuff),
                                 GWEN_Buffer_GetUsedBytes(hBuff), hOutBuff);
      if(rv != 0)
        DBG_ERROR(AQHBCI_LOGDOMAIN, "AH_Provider__HashRmd160() failed (%d).", rv);
      break;
    case GWEN_Crypt_HashAlgoId_Sha256:
      //hName = "SHA-256";
      hOutLen = 32;
      rv = AH_Provider__HashSha256((const uint8_t*)GWEN_Buffer_GetStart(hBuff),
                                 GWEN_Buffer_GetUsedBytes(hBuff), hOutBuff);
      if(rv != 0)
        DBG_ERROR(AQHBCI_LOGDOMAIN, "AH_Provider__HashSha256() failed (%d).", rv);
      break;
    default:
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Hash algo not specified.");
      rv = GWEN_ERROR_GENERIC;
    }
    if(rv == 0)
    {
      res->hashLen = hOutLen;
      res->hash = malloc(res->hashLen);
      if(!res->hash)
      {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "allocate result-buffer failed (%d).", rv);
        rv = GWEN_ERROR_GENERIC;
      }
      else
        memcpy(res->hash, hOutBuff, hOutLen);
    }
  }
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "bank %d, hLen %ld, eLen %ld, mLen %ld, e %p, m %p, hb %p.",
                    bankKey, (long)hLen, (long)eLen, (long)mLen, eData, mData, hBuff);


  if(hBuff)
    GWEN_Buffer_free(hBuff);
  free(eData);
  free(mData);

  return rv;
}
