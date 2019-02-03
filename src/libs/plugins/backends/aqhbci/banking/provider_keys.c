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
  if (AH_User_GetCryptMode(u)!=AH_CryptMode_Rdh) {
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

  if (rdhType==10) {
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


