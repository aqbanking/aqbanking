/***************************************************************************
    begin       : Tue Nov 25 2008
    copyright   : (C) 2020 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#include <gwenhywfar/text.h>

#define AH_MSGRXH_MAXKEYBUF 4096

typedef enum {
  AH_Opmode_None=0,
  AH_Opmode_Cbc=2,
  AH_Opmode_Iso9796_1=16,
  AH_Opmode_Iso9796_2=17,
  AH_Opmode_Rsa_Pkcs1_v1_5=18,
  AH_Opmode_Rsa_Pss=19,
  AH_Opmode_Retail_MAC=999
} AH_OPMODE;

typedef enum {
  AH_HashAlg_None=0,
  AH_HashAlg_Sha1=1,
  AH_HashAlg_Sha256=3,
  AH_HashAlg_Sha256Sha256=6,
  AH_HashAlg_Ripmed160=999
} AH_HASH_ALG;

typedef enum {
  AH_SignAlg_DES=1,
  AH_SignAlg_RSA=10
} AH_SIGN_ALG;

typedef enum {
  AH_CryptAlg_2_Key_Triple_Des=13,
  AH_CryptAlg_AES256=14
} AH_CRYPT_ALG;

typedef enum {
  AH_UsageSign_None=0,
  AH_UsageSign_OwnerSigning=6
} AH_USAGE_SIGN;

typedef struct {
  AH_CRYPT_MODE protocol;
  uint8_t       protocolVersion;
  AH_SIGN_ALG   signAlgo;         /* Signaturalgorithmus, kodiert */
  AH_OPMODE     opmodSignS;       /* Operationsmodus bei Signatur (Signierschluessel) */
  AH_OPMODE     opmodSignD;       /* Operationsmodus bei Signatur (Signaturschluessel) */
  AH_USAGE_SIGN usageSign;        /* Verwendung des Signaturalgorithmus */
  AH_HASH_ALG   hashAlgS;         /* Hashalgorithmus, kodiert (Signierschluessel) */
  AH_HASH_ALG   hashAlgD;         /* Hashalgorithmus, kodiert (Signaturschluessel) */
  AH_CRYPT_ALG  cryptAlg;         /* Verschluesselungsalgorithmus, kodiert */
  AH_OPMODE     opmodCrypt;       /* Operationsmodus bei Verschluesselung */
} RXH_PARAMETER;

RXH_PARAMETER  rdh1_parameter= {
  AH_CryptMode_Rdh,
  1,
  AH_SignAlg_RSA,
  AH_Opmode_Iso9796_1,
  AH_Opmode_None,
  AH_UsageSign_OwnerSigning,
  AH_HashAlg_Ripmed160,
  AH_HashAlg_None,
  AH_CryptAlg_2_Key_Triple_Des,
  AH_Opmode_Cbc
};

RXH_PARAMETER  rdh2_parameter= {
  AH_CryptMode_Rdh,
  2,
  AH_SignAlg_RSA,
  AH_Opmode_Iso9796_2,
  AH_Opmode_None,
  AH_UsageSign_OwnerSigning,
  AH_HashAlg_Ripmed160,
  AH_HashAlg_None,
  AH_CryptAlg_2_Key_Triple_Des,
  AH_Opmode_Cbc
};

RXH_PARAMETER  rdh3_parameter= {
  AH_CryptMode_Rdh,
  3,
  AH_SignAlg_RSA,
  AH_Opmode_Rsa_Pkcs1_v1_5,
  AH_Opmode_Iso9796_2,
  AH_UsageSign_OwnerSigning,
  AH_HashAlg_Sha1,
  AH_HashAlg_Ripmed160,
  AH_CryptAlg_2_Key_Triple_Des,
  AH_Opmode_Rsa_Pkcs1_v1_5
};

RXH_PARAMETER  rdh5_parameter= {
  AH_CryptMode_Rdh,
  5,
  AH_SignAlg_RSA,
  AH_Opmode_Rsa_Pkcs1_v1_5,
  AH_Opmode_None,
  AH_UsageSign_OwnerSigning,
  AH_HashAlg_Sha1,
  AH_HashAlg_None,
  AH_CryptAlg_2_Key_Triple_Des,
  AH_Opmode_Rsa_Pkcs1_v1_5
};

RXH_PARAMETER  rdh6_parameter= {
  AH_CryptMode_Rdh,
  6,
  AH_SignAlg_RSA,
  AH_Opmode_Rsa_Pkcs1_v1_5,
  AH_Opmode_Rsa_Pkcs1_v1_5,
  AH_UsageSign_OwnerSigning,
  AH_HashAlg_Sha256,
  AH_HashAlg_Sha256,
  AH_CryptAlg_2_Key_Triple_Des,
  AH_Opmode_Rsa_Pkcs1_v1_5
};

RXH_PARAMETER  rdh7_parameter= {
  AH_CryptMode_Rdh,
  7,
  AH_SignAlg_RSA,
  AH_Opmode_Rsa_Pss,
  AH_Opmode_Rsa_Pss,
  AH_UsageSign_OwnerSigning,
  AH_HashAlg_Sha256Sha256,
  AH_HashAlg_Sha256,
  AH_CryptAlg_2_Key_Triple_Des,
  AH_Opmode_Rsa_Pkcs1_v1_5
};

RXH_PARAMETER  rdh8_parameter= {
  AH_CryptMode_Rdh,
  8,
  AH_SignAlg_RSA,
  AH_Opmode_Rsa_Pkcs1_v1_5,
  AH_Opmode_None,
  AH_UsageSign_OwnerSigning,
  AH_HashAlg_Sha256,
  AH_HashAlg_None,
  AH_CryptAlg_2_Key_Triple_Des,
  AH_Opmode_Rsa_Pkcs1_v1_5
};

RXH_PARAMETER  rdh9_parameter= {
  AH_CryptMode_Rdh,
  9,
  AH_SignAlg_RSA,
  AH_Opmode_Rsa_Pss,
  AH_Opmode_None,
  AH_UsageSign_OwnerSigning,
  AH_HashAlg_Sha256Sha256,
  AH_HashAlg_None,
  AH_CryptAlg_2_Key_Triple_Des,
  AH_Opmode_Rsa_Pkcs1_v1_5
};

RXH_PARAMETER  rdh10_parameter= {
  AH_CryptMode_Rdh,
  10,
  AH_SignAlg_RSA,
  AH_Opmode_Rsa_Pss,
  AH_Opmode_None,
  AH_UsageSign_OwnerSigning,
  AH_HashAlg_Sha256Sha256,
  AH_HashAlg_None,
  AH_CryptAlg_2_Key_Triple_Des,
  AH_Opmode_Cbc
};

RXH_PARAMETER *rdh_parameter[11]= {
  NULL, /* 0 */
  &rdh1_parameter, /* 1 */
  &rdh2_parameter, /* 2 */
  &rdh3_parameter, /* 3 */
  NULL, /* 4 */
  &rdh5_parameter, /* 5 */
  &rdh6_parameter, /* 6 */
  &rdh7_parameter, /* 7 */
  &rdh8_parameter, /* 8 */
  &rdh9_parameter, /* 9 */
  &rdh10_parameter /* 10 */
};

RXH_PARAMETER  rah7_parameter= {AH_CryptMode_Rah,
                                7,
                                AH_SignAlg_RSA,
                                AH_Opmode_Rsa_Pss,
                                AH_Opmode_Rsa_Pss,
                                AH_UsageSign_OwnerSigning,
                                AH_HashAlg_Sha256Sha256,
                                AH_HashAlg_Sha256,
                                AH_CryptAlg_AES256,
                                AH_Opmode_Rsa_Pkcs1_v1_5
                               };

RXH_PARAMETER  rah9_parameter= {AH_CryptMode_Rah,
                                9,
                                AH_SignAlg_RSA,
                                AH_Opmode_Rsa_Pss,
                                AH_Opmode_None,
                                AH_UsageSign_OwnerSigning,
                                AH_HashAlg_Sha256Sha256,
                                AH_HashAlg_None,
                                AH_CryptAlg_AES256,
                                AH_Opmode_Rsa_Pkcs1_v1_5
                               };

RXH_PARAMETER  rah10_parameter= {AH_CryptMode_Rah,
                                 10,
                                 AH_SignAlg_RSA,
                                 AH_Opmode_Rsa_Pss,
                                 AH_Opmode_None,
                                 AH_UsageSign_OwnerSigning,
                                 AH_HashAlg_Sha256Sha256,
                                 AH_HashAlg_None,
                                 AH_CryptAlg_AES256,
                                 AH_Opmode_Cbc
                                };

RXH_PARAMETER *rah_parameter[11]= {
  NULL, /* 0 */
  NULL, /* 1 */
  NULL, /* 2 */
  NULL, /* 3 */
  NULL, /* 4 */
  NULL, /* 5 */
  NULL, /* 6 */
  &rah7_parameter, /* 7 */
  NULL, /* 8 */
  &rah9_parameter, /* 9 */
  &rah10_parameter /* 10 */

};

static
GWEN_CRYPT_KEY *AH_MsgRxh_VerifyInitialSignKey(GWEN_CRYPT_TOKEN *ct,
                                               const GWEN_CRYPT_TOKEN_CONTEXT *ctx,
                                               AB_USER *user,
                                               GWEN_DB_NODE *gr)
{

  GWEN_DB_NODE *dbCurr;
  int haveKey=0;
  int verified;
  GWEN_CRYPT_KEY *bpk = NULL;

  /* search for "GetKeyResponse" */
  haveKey=0;
  dbCurr=GWEN_DB_GetFirstGroup(gr);
  while (dbCurr) {
    GWEN_DB_NODE *dbKeyResponse;
    const char *s;

    if (strcasecmp(GWEN_DB_GroupName(dbCurr), "GetKeyResponse")==0) {
      unsigned int bs;
      const uint8_t *p;
      dbKeyResponse=dbCurr;
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Got this key response:");
      if (GWEN_Logger_GetLevel(AQHBCI_LOGDOMAIN)>=GWEN_LoggerLevel_Debug)
        GWEN_DB_Dump(dbKeyResponse, 2);

      p=GWEN_DB_GetBinValue(dbKeyResponse, "key/modulus", 0, 0, 0, &bs);

      if (!p || !bs) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "No modulus");
        return NULL;
      }
      else {
        /* :TODO: if no key hash is on the card, check if a certificate was sent with the
         * key and verify that, if not, ask the user for the INI-Letter
         */
        const uint8_t *exponent;
        unsigned int expLen;
        int msgKeyNum;
        int msgKeyVer;
        uint16_t sentModulusLength;
        int keySize;

        exponent=GWEN_DB_GetBinValue(dbKeyResponse, "key/exponent", 0, 0, 0, &expLen);
        sentModulusLength=bs;
        /* skip zero bytes if any */
        while (bs && *p==0) {
          p++;
          bs--;
        }

        /* calculate key size in bytes */
        if (bs<=96)
          keySize=96;
        else {
          keySize=bs;
        }

        s=GWEN_DB_GetCharValue(dbKeyResponse, "keyname/keytype", 0, "V");
        msgKeyNum=GWEN_DB_GetIntValue(dbKeyResponse, "keyname/keynum", 0, 0);
        msgKeyVer=GWEN_DB_GetIntValue(dbKeyResponse, "keyname/keyversion", 0, 0);



        if (strcasecmp(s, "S")==0) {
          bpk=GWEN_Crypt_KeyRsa_fromModExp(keySize, p, bs, exponent, expLen);
          GWEN_Crypt_Key_SetKeyNumber(bpk, msgKeyNum);
          GWEN_Crypt_Key_SetKeyVersion(bpk, msgKeyVer);
          verified=AH_User_VerifyInitialKey(ct, ctx, user, bpk, sentModulusLength, "sign");
          if (verified==1) {
            GWEN_Crypt_KeyRsa_AddFlags(bpk, GWEN_CRYPT_KEYRSA_FLAGS_ISVERIFIED);
            AH_User_SetBankPubSignKey(user, bpk);
            /* reload */
            bpk=AH_User_GetBankPubSignKey(user);
          }
          else {
            return NULL;
          }

        }
      }
      haveKey++;
    } /* if we have one */
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  } /* while */


  return bpk;
}

static int AH_MsgRxh_PrepareCryptoSeg(AH_MSG *hmsg,
                                      AB_USER *u,
                                      RXH_PARAMETER *rxh_parameter,
                                      int keyNum,
                                      int keyVer,
                                      const GWEN_CRYPT_TOKEN_KEYINFO *ki,
                                      GWEN_DB_NODE *cfg,
                                      int crypt,
                                      int createCtrlRef)
{
  char sdate[9];
  char stime[7];
  char ctrlref[15];
  struct tm *lt;
  time_t tt;
  const char *userId;
  const char *peerId;
  int secProfile;
  assert(hmsg);
  assert(u);
  assert(cfg);

  userId=AB_User_GetUserId(u);
  secProfile = AH_Msg_GetSecurityProfile(hmsg);
  assert(userId);
  assert(*userId);
  peerId=AH_User_GetPeerId(u);
  if (!peerId || *peerId==0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No PeerId in user, using user id");
    peerId=userId;
  }

  tt=time(0);
  lt=localtime(&tt);

  if (createCtrlRef) {
    /* create control reference */
    if (!strftime(ctrlref, sizeof(ctrlref), "%Y%m%d%H%M%S", lt)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "CtrlRef string too long");
      return GWEN_ERROR_INTERNAL;
    }

    GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "ctrlref", ctrlref);
  }

  /* create date */
  if (!strftime(sdate, sizeof(sdate), "%Y%m%d", lt)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Date string too long");
    return GWEN_ERROR_INTERNAL;
  }
  /* create time */
  if (!strftime(stime, sizeof(stime), "%H%M%S", lt)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Date string too long");
    return GWEN_ERROR_INTERNAL;
  }

  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecDetails/dir", 1);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecStamp/date", sdate);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecStamp/time", stime);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/bankcode", AB_User_GetBankCode(u));
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/userid", crypt?peerId:userId);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/keytype", crypt?"V":(secProfile>2?"D":"S"));
  if (crypt) {
    GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/keynum", keyNum);
    GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/keyversion", keyVer);
  }
  else {
    GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/keynum", keyNum);
    GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/keyversion", keyVer);
  }
  switch (rxh_parameter->protocol) {
  case AH_CryptMode_Rdh:
    GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "secProfile/code", "RDH");
    break;
  case AH_CryptMode_Rah:
    GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "secProfile/code", "RAH");
    break;
  default:
    return GWEN_ERROR_INTERNAL;
  }
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "secProfile/version", rxh_parameter->protocolVersion);
  if (crypt) {
    GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "function", 4);        /* crypt */
    GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "cryptAlgo/algo", rxh_parameter->cryptAlg);
    GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "cryptAlgo/mode", rxh_parameter->opmodCrypt);
  }
  else {
    GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "signAlgo/algo", rxh_parameter->signAlgo);
    if (secProfile > 2) {
      GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "function", 1);        /* sign with digital signature key */
      assert(rxh_parameter->opmodSignD > 0);
      assert(rxh_parameter->hashAlgD > 0);
      GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "signAlgo/mode", rxh_parameter->opmodSignD);
      GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "hashAlgo/algo", rxh_parameter->hashAlgD);
    }
    else {
      GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "function", 2);        /* sign with signature key */
      GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "signAlgo/mode", rxh_parameter->opmodSignS);
      GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "hashAlgo/algo", rxh_parameter->hashAlgS);
    }
    if (secProfile > 1) {
      /* add certificate TODO: we need to get the type of certificate from outside */
      int certLen = GWEN_Crypt_Token_KeyInfo_GetCertificateLen(ki);
      const uint8_t *certData = GWEN_Crypt_Token_KeyInfo_GetCertificateData(ki);
      GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "cert/type", 3); /* X.509 */
      GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT, "cert/cert", certData, certLen);
    }
  }

  return 0;
}

int AH_Msg_SignRxh(AH_MSG *hmsg,
                   GWEN_BUFFER *rawBuf,
                   const char *signer)
{
  AH_HBCI *h;
  GWEN_XMLNODE *node;
  GWEN_DB_NODE *cfg;
  GWEN_BUFFER *sigbuf;
  GWEN_BUFFER *hbuf;
  unsigned int l;
  int rv;
  char ctrlref[15];
  const char *p;
  GWEN_MSGENGINE *e;
  uint32_t uFlags;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  const GWEN_CRYPT_TOKEN_KEYINFO *ki;
  uint32_t keyId;
  uint32_t gid;
  int secProfile;
  RXH_PARAMETER *rxh_parameter;
  int rxhVersion;
  AB_USER *su;

  assert(hmsg);

  su=AH_Msg_GetUser(hmsg, signer);
  if (!su) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Unknown user \"%s\"",
              signer);
    return GWEN_ERROR_NOT_FOUND;
  }

  h=AH_Dialog_GetHbci(hmsg->dialog);
  assert(h);
  e=AH_Dialog_GetMsgEngine(hmsg->dialog);
  assert(e);

  /* get correct parameters */
  rxhVersion = AH_User_GetRdhType(su);
  switch (AH_User_GetCryptMode(su)) {
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
    return AB_ERROR_NOT_INIT;

  }

  GWEN_MsgEngine_SetMode(e, AH_CryptMode_toString(rxh_parameter->protocol));
  //GWEN_MsgEngine_SetMode(e,"rdh");
  secProfile = AH_Msg_GetSecurityProfile(hmsg);
  gid=0;

  uFlags=AH_User_GetFlags(su);



  /* get crypt token of signer */
  rv=AB_Banking_GetCryptToken(AH_HBCI_GetBankingApi(h),
                              AH_User_GetTokenType(su),
                              AH_User_GetTokenName(su),
                              &ct);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Could not get crypt token for user \"%s\" (%d)",
             AB_User_GetUserId(su), rv);
    return rv;
  }

  /* open CryptToken if necessary */
  if (!GWEN_Crypt_Token_IsOpen(ct)) {
    GWEN_Crypt_Token_AddModes(ct, GWEN_CRYPT_TOKEN_MODE_DIRECT_SIGN);
    rv=GWEN_Crypt_Token_Open(ct, 0, gid);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Could not open crypt token for user \"%s\" (%d)",
               AB_User_GetUserId(su), rv);
      return rv;
    }
  }

  /* get context and key info */
  ctx=GWEN_Crypt_Token_GetContext(ct, AH_User_GetTokenContextId(su), gid);
  if (ctx==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Context %d not found on crypt token [%s:%s]",
             AH_User_GetTokenContextId(su),
             GWEN_Crypt_Token_GetTypeName(ct),
             GWEN_Crypt_Token_GetTokenName(ct));
    return GWEN_ERROR_NOT_FOUND;
  }

  if (secProfile > 2) {
    keyId=GWEN_Crypt_Token_Context_GetAuthSignKeyId(ctx);
    DBG_ERROR(AQHBCI_LOGDOMAIN, "AQHBCI does not yet support non-reputation!");
    return AB_ERROR_NOT_INIT;
  }
  else {
    keyId=GWEN_Crypt_Token_Context_GetSignKeyId(ctx);
  }

  ki=GWEN_Crypt_Token_GetKeyInfo(ct, keyId, 0xffffffff, gid);
  if (ki==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Keyinfo %04x not found on crypt token [%s:%s]",
             keyId,
             GWEN_Crypt_Token_GetTypeName(ct),
             GWEN_Crypt_Token_GetTokenName(ct));
    return GWEN_ERROR_NOT_FOUND;
  }

  node=GWEN_MsgEngine_FindNodeByPropertyStrictProto(e, "SEG", "id", 0, "SigHead");
  if (!node) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Segment \"SigHead\" not found");
    return GWEN_ERROR_INTERNAL;
  }

  /* prepare config for segment */
  cfg=GWEN_DB_Group_new("sighead");
  rv=AH_MsgRxh_PrepareCryptoSeg(hmsg, su, rxh_parameter, rxh_parameter->protocolVersion,
                                GWEN_Crypt_Token_KeyInfo_GetKeyVersion(ki), ki, cfg, 0, 1);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(cfg);
    return rv;
  }

  /* set expected signer */
  if (!(uFlags & AH_USER_FLAGS_BANK_DOESNT_SIGN)) {
    const char *remoteId;

    remoteId=AH_User_GetPeerId(su);
    if (!remoteId || *remoteId==0)
      remoteId=AB_User_GetUserId(su);
    assert(remoteId);
    assert(*remoteId);

    DBG_DEBUG(AQHBCI_LOGDOMAIN,
              "Expecting \"%s\" to sign the response",
              remoteId);
    AH_Msg_SetExpectedSigner(hmsg, remoteId);
  }



  /* store system id */
  if (hmsg->noSysId) {
    GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                         "SecDetails/SecId", "0");
  }
  else {
    /* store CID if we use a card */
    const uint8_t *cidData;
    uint32_t cidLen=GWEN_Crypt_Token_Context_GetCidLen(ctx);
    cidData=GWEN_Crypt_Token_Context_GetCidPtr(ctx);
    if (cidLen > 0 && cidData != NULL) {
      GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecDetails/CID", cidData, cidLen);
    }
    p=AH_User_GetSystemId(su);
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

  /* retrieve control reference for sigtail (to be used later) */
  p=GWEN_DB_GetCharValue(cfg, "ctrlref", 0, "");
  if (strlen(p)>=sizeof(ctrlref)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Control reference too long (14 bytes maximum)");
    GWEN_DB_Group_free(cfg);
    return -1;
  }
  strcpy(ctrlref, p);

  /* create SigHead */
  hbuf=GWEN_Buffer_new(0, 128+GWEN_Buffer_GetUsedBytes(rawBuf), 0, 1);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "head/seq", hmsg->firstSegment-1);
  if (AH_Msg_SignSeqOne(hmsg)) {
    GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "signseq", 1);
  }
  else {
    GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "signseq", GWEN_Crypt_Token_KeyInfo_GetSignCounter(ki));
  }

  /* create signature head segment */
  rv=GWEN_MsgEngine_CreateMessageFromNode(e, node, hbuf, cfg);
  GWEN_DB_Group_free(cfg);
  cfg=0;
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not create SigHead");
    GWEN_Buffer_free(hbuf);
    return rv;
  }

  /* remember size of sighead for now */
  l=GWEN_Buffer_GetUsedBytes(hbuf);

  /* add raw data to to-sign data buffer */
  GWEN_Buffer_AppendBuffer(hbuf, rawBuf);

  /* sign message */
  sigbuf=GWEN_Buffer_new(0, 512, 0, 1);
  {
    uint32_t signLen;
    GWEN_CRYPT_PADDALGO *algo;
    GWEN_MDIGEST *md=NULL;
    uint32_t seq;
    AH_HASH_ALG hashAlg;
    AH_OPMODE opMode;
    uint8_t  *digestPtr;
    unsigned int digestSize;
    const char *tokenType = AH_User_GetTokenType(su);
    uint8_t doSHA256inSW = 0;

    if (secProfile > 2) {
      hashAlg = rxh_parameter->hashAlgD;
      opMode= rxh_parameter->opmodSignD;
    }
    else {
      hashAlg = rxh_parameter->hashAlgS;
      opMode= rxh_parameter->opmodSignS;
    }

    /* https://www.aquamaniac.de/rdm/issues/41 */
    if (tokenType && !strcasecmp(tokenType, "ohbci"))
      doSHA256inSW = 1;

    /* hash sighead + data */
    switch (hashAlg) {
    case AH_HashAlg_Sha1:
      md=GWEN_MDigest_Sha1_new();
      break;
    case AH_HashAlg_Sha256:
      break;
    case AH_HashAlg_Sha256Sha256:
      md=GWEN_MDigest_Sha256_new();
      break;
    case AH_HashAlg_Ripmed160:
      md=GWEN_MDigest_Rmd160_new();
      break;
    default:
      md=NULL;
    }
    if (md != NULL) {
      rv=GWEN_MDigest_Begin(md);
      if (rv==0)
        rv=GWEN_MDigest_Update(md,
                               (uint8_t *)GWEN_Buffer_GetStart(hbuf),
                               GWEN_Buffer_GetUsedBytes(hbuf));
      if (rv==0)
        rv=GWEN_MDigest_End(md);
      if (rv<0) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Hash error (%d)", rv);
        GWEN_MDigest_free(md);
        GWEN_Buffer_free(sigbuf);
        GWEN_Buffer_free(hbuf);
        return rv;
      }
      if ((hashAlg == AH_HashAlg_Sha256Sha256) && doSHA256inSW) {
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "%s(): doSHA256inSW (2nd).", __FUNCTION__);
        rv = GWEN_MDigest_Begin(md);
        if (rv == 0) {
          uint8_t h[32];
          memcpy(h, GWEN_MDigest_GetDigestPtr(md), 32);
          rv = GWEN_MDigest_Update(md, h, 32);
          if (rv == 0)
            rv = GWEN_MDigest_End(md);
        }
        if (rv < 0) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Hash error round 2 (%d)", rv);
          GWEN_MDigest_free(md);
          GWEN_Buffer_free(sigbuf);
          GWEN_Buffer_free(hbuf);
          return rv;
        }
      }
      digestPtr=GWEN_MDigest_GetDigestPtr(md);
      digestSize=GWEN_MDigest_GetDigestSize(md);
    }
    else {
      digestPtr=(uint8_t *)GWEN_Buffer_GetStart(hbuf);
      digestSize=GWEN_Buffer_GetUsedBytes(hbuf);
    }

    /* sign hash */
    switch (opMode) {
    case AH_Opmode_Iso9796_1:
      algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_Iso9796_1A4);
      GWEN_Crypt_PaddAlgo_SetPaddSize(algo, GWEN_Crypt_Token_KeyInfo_GetKeySize(ki));
      break;

    case AH_Opmode_Iso9796_2:
      algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_Iso9796_2);
      GWEN_Crypt_PaddAlgo_SetPaddSize(algo, GWEN_Crypt_Token_KeyInfo_GetKeySize(ki));
      break;

    case AH_Opmode_Rsa_Pkcs1_v1_5:
      algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_Pkcs1_2);
      GWEN_Crypt_PaddAlgo_SetPaddSize(algo, GWEN_Crypt_Token_KeyInfo_GetKeySize(ki));
      break;

    case AH_Opmode_Rsa_Pss:
      algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_Pkcs1_Pss_Sha256);
      GWEN_Crypt_PaddAlgo_SetPaddSize(algo, GWEN_Crypt_Token_KeyInfo_GetKeySize(ki));
      break;
    default:
      return GWEN_ERROR_INTERNAL;
    }

    signLen=GWEN_Buffer_GetMaxUnsegmentedWrite(sigbuf);


    rv=GWEN_Crypt_Token_Sign(ct, keyId,
                             algo,
                             digestPtr,
                             digestSize,
                             (uint8_t *)GWEN_Buffer_GetStart(sigbuf),
                             &signLen,
                             &seq,
                             gid);

    GWEN_Crypt_PaddAlgo_free(algo);

    GWEN_MDigest_free(md);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
                "Could not sign data with medium of user \"%s\" (%d)",
                AB_User_GetUserId(su), rv);
      GWEN_Buffer_free(sigbuf);
      GWEN_Buffer_free(hbuf);
      return rv;
    }
    GWEN_Buffer_IncrementPos(sigbuf, signLen);
    GWEN_Buffer_AdjustUsedBytes(sigbuf);
  }
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Signing done");

  /* insert new SigHead at beginning of message buffer */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Inserting signature head");
  GWEN_Buffer_Rewind(hmsg->buffer);
  GWEN_Buffer_InsertBytes(hmsg->buffer, GWEN_Buffer_GetStart(hbuf), l);

  /* create sigtail */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Completing signature tail");
  cfg=GWEN_DB_Group_new("sigtail");
  GWEN_Buffer_Reset(hbuf);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "head/seq", hmsg->lastSegment+1);
  /* store to DB */
  GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "signature",
                      GWEN_Buffer_GetStart(sigbuf),
                      GWEN_Buffer_GetUsedBytes(sigbuf));
  GWEN_Buffer_free(sigbuf);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "ctrlref", ctrlref);

  /* get node */
  node=GWEN_MsgEngine_FindNodeByPropertyStrictProto(e, "SEG", "id", 0, "SigTail");
  if (!node) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Segment \"SigTail\"not found");
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }
  rv=GWEN_MsgEngine_CreateMessageFromNode(e, node, hbuf, cfg);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not create SigTail");
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }

  /* append sigtail */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Appending signature tail");
  if (GWEN_Buffer_AppendBuffer(hmsg->buffer, hbuf)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Appending signature tail: done");

  GWEN_Buffer_free(hbuf);
  GWEN_DB_Group_free(cfg);

  /* adjust segment numbers (for next signature and message tail */
  hmsg->firstSegment--;
  hmsg->lastSegment++;

  return 0;
}

int AH_Msg_EncryptRxh(AH_MSG *hmsg)
{
  AH_HBCI *h;
  GWEN_XMLNODE *node;
  GWEN_DB_NODE *cfg;
  GWEN_BUFFER *mbuf;
  GWEN_BUFFER *hbuf;
  uint32_t l;
  int rv;
  const char *p;
  GWEN_MSGENGINE *e;
  AB_USER *u;
  const char *peerId;
  //uint32_t uFlags;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  GWEN_CRYPT_KEY *sk, *ek;
  uint8_t encKey[AH_MSGRXH_MAXKEYBUF+64];
  int encKeyLen;
  uint32_t gid;
  uint8_t sessionKeySize;
  RXH_PARAMETER *rxh_parameter;
  int rxhVersion;

  DBG_ERROR(AQHBCI_LOGDOMAIN, "RXH-encrypting message");

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
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Profile RDH%d is not supported!", rxhVersion);
      return AB_ERROR_NOT_INIT;
    }
    break;
  default:
    return GWEN_ERROR_INTERNAL;
  }

  assert(hmsg);
  h=AH_Dialog_GetHbci(hmsg->dialog);
  assert(h);
  e=AH_Dialog_GetMsgEngine(hmsg->dialog);
  assert(e);
  GWEN_MsgEngine_SetMode(e, AH_CryptMode_toString(rxh_parameter->protocol));
  //GWEN_MsgEngine_SetMode(e,"rdh");
  gid=0;


  //  uFlags=AH_User_GetFlags(u);

  peerId=AH_User_GetPeerId(u);
  if (!peerId || *peerId==0)
    peerId=AB_User_GetUserId(u);

  /* get crypt token of signer */
  rv=AB_Banking_GetCryptToken(AH_HBCI_GetBankingApi(h),
                              AH_User_GetTokenType(u),
                              AH_User_GetTokenName(u),
                              &ct);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Could not get crypt token for user \"%s\" (%d)",
             AB_User_GetUserId(u), rv);
    return rv;
  }

  /* open CryptToken if necessary */
  if (!GWEN_Crypt_Token_IsOpen(ct)) {
    GWEN_Crypt_Token_AddModes(ct, GWEN_CRYPT_TOKEN_MODE_DIRECT_SIGN);
    rv=GWEN_Crypt_Token_Open(ct, 0, gid);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Could not open crypt token for user \"%s\" (%d)",
               AB_User_GetUserId(u), rv);
      return rv;
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
    return GWEN_ERROR_NOT_FOUND;
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
    return GWEN_ERROR_INTERNAL;
  }

  switch (rxh_parameter->protocol) {
  case AH_CryptMode_Rdh:
    DBG_INFO(AQHBCI_LOGDOMAIN, "Padding message with ANSI X9.23");
    rv=GWEN_Padd_PaddWithAnsiX9_23(hmsg->buffer);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error padding message with ANSI X9.23 (%d)", rv);
      return rv;
    }

    /* create session key */
    sk=GWEN_Crypt_KeyDes3K_Generate(GWEN_Crypt_CryptMode_Cbc, 24, 2);
    sessionKeySize=16;
    if (sk==NULL) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Could not generate DES key");
      return GWEN_ERROR_INTERNAL;
    }

    if (0) {
      uint8_t *p;
      uint32_t len;

      DBG_ERROR(AQHBCI_LOGDOMAIN, "DES key for message");
      p=GWEN_Crypt_KeyDes3K_GetKeyDataPtr(sk);
      len=GWEN_Crypt_KeyDes3K_GetKeyDataLen(sk);
      GWEN_Text_LogString((const char *)p, len, AQHBCI_LOGDOMAIN, GWEN_LoggerLevel_Error);
    }
    break;

  case AH_CryptMode_Rah:
    DBG_INFO(AQHBCI_LOGDOMAIN, "Padding message with ZKA method");
    rv=GWEN_Padd_PaddWithZka(hmsg->buffer);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Error padding message with ZKA padding (%d)", rv);
      return rv;
    }

    /* create session key */
    sk=GWEN_Crypt_KeyAes256_Generate(GWEN_Crypt_CryptMode_Cbc, 32, 2);
    sessionKeySize=32;
    if (sk==NULL) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Could not generate AES-256 key");
      return GWEN_ERROR_INTERNAL;
    }
    break;

  default:
    DBG_INFO(AQHBCI_LOGDOMAIN, "Protocol not supported!");
    return GWEN_ERROR_INTERNAL;
  }
  /* encrypt message with that session key */
  mbuf=GWEN_Buffer_new(0, GWEN_Buffer_GetUsedBytes(hmsg->buffer), 0, 1);
  l=GWEN_Buffer_GetUsedBytes(hmsg->buffer);
  rv=GWEN_Crypt_Key_Encipher(sk,
                             (uint8_t *)GWEN_Buffer_GetStart(hmsg->buffer),
                             GWEN_Buffer_GetUsedBytes(hmsg->buffer),
                             (uint8_t *)GWEN_Buffer_GetPosPointer(mbuf),
                             &l);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Could not encipher with DES session key (%d)",
             rv);
    GWEN_Buffer_free(mbuf);
    GWEN_Crypt_Key_free(sk);
    return rv;
  }
  GWEN_Buffer_IncrementPos(mbuf, l);
  GWEN_Buffer_AdjustUsedBytes(mbuf);


  /* encrypt session key */
  if (1) {
    uint32_t elen;
    GWEN_BUFFER *skbuf;
    GWEN_CRYPT_PADDALGO *algo;

    skbuf=GWEN_Buffer_new(0, 512, 0, 1);
    switch (rxh_parameter->protocol) {
    case AH_CryptMode_Rdh:
      GWEN_Buffer_InsertBytes(skbuf, (const char *) GWEN_Crypt_KeyDes3K_GetKeyDataPtr(sk), sessionKeySize);
      break;
    case AH_CryptMode_Rah:
      GWEN_Buffer_InsertBytes(skbuf, (const char *) GWEN_Crypt_KeyAes256_GetKeyDataPtr(sk), sessionKeySize);
      break;
    default:
      return GWEN_ERROR_INTERNAL;
    }
    GWEN_Buffer_Rewind(skbuf);

    switch (rxh_parameter->opmodCrypt) {
    case AH_Opmode_Cbc:
      algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_LeftZero);
      break;
    case AH_Opmode_Rsa_Pkcs1_v1_5:
      algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_Pkcs1_2);
      break;
    default:
      return GWEN_ERROR_INTERNAL;
    }
    GWEN_Crypt_PaddAlgo_SetPaddSize(algo, GWEN_Crypt_Key_GetKeySize(ek));
    /* padd according to given algo */
    rv=GWEN_Padd_ApplyPaddAlgo(algo, skbuf);
    GWEN_Crypt_PaddAlgo_free(algo);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(skbuf);
      return rv;
    }


    elen=GWEN_Crypt_Key_GetKeySize(ek);
    rv=GWEN_Crypt_Key_Encipher(ek,
                               (const uint8_t *) GWEN_Buffer_GetStart(skbuf),
                               GWEN_Crypt_Key_GetKeySize(ek), encKey, &elen);

    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(mbuf);
      GWEN_Buffer_free(skbuf);
      GWEN_Crypt_Key_free(sk);
      return rv;
    }
    encKeyLen=elen;
    GWEN_Buffer_free(skbuf);
  }
  GWEN_Crypt_Key_free(sk);

  /* create crypt head */
  node=GWEN_MsgEngine_FindNodeByPropertyStrictProto(e, "SEG", "id", 0, "CryptHead");
  if (!node) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Segment \"CryptHead\" not found");
    GWEN_Buffer_free(mbuf);
    GWEN_Crypt_Key_free(sk);
    return GWEN_ERROR_INTERNAL;
  }

  /* create CryptHead */
  cfg=GWEN_DB_Group_new("crypthead");
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "head/seq", 998);

  rv=AH_MsgRxh_PrepareCryptoSeg(hmsg, u, rxh_parameter, GWEN_Crypt_Key_GetKeyNumber(ek), GWEN_Crypt_Key_GetKeyVersion(ek),
                                NULL, cfg, 1, 0);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(cfg);
    GWEN_Buffer_free(mbuf);
    return rv;
  }

  /* store system id */
  if (hmsg->noSysId) {
    GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                         "SecDetails/SecId", "0");
  }
  else {
    /* store CID if we use a card */
    const uint8_t *cidData;
    uint32_t cidLen=GWEN_Crypt_Token_Context_GetCidLen(ctx);
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

  hbuf=GWEN_Buffer_new(0, 256+GWEN_Buffer_GetUsedBytes(mbuf), 0, 1);
  rv=GWEN_MsgEngine_CreateMessageFromNode(e, node, hbuf, cfg);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not create CryptHead (%d)", rv);
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    GWEN_Buffer_free(mbuf);
    return rv;
  }
  GWEN_DB_Group_free(cfg);

  /* create cryptdata */
  cfg=GWEN_DB_Group_new("cryptdata");
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "head/seq", 999);
  GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT, "cryptdata", GWEN_Buffer_GetStart(mbuf),
                      GWEN_Buffer_GetUsedBytes(mbuf));
  GWEN_Buffer_free(mbuf);

  node=GWEN_MsgEngine_FindNodeByPropertyStrictProto(e, "SEG", "id", 0, "CryptData");
  if (!node) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Segment \"CryptData\"not found");
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }
  rv=GWEN_MsgEngine_CreateMessageFromNode(e, node, hbuf, cfg);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not create CryptData (%d)", rv);
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return rv;
  }

  /* replace existing buffer by encrypted one */
  GWEN_Buffer_free(hmsg->buffer);
  hmsg->buffer=hbuf;
  GWEN_DB_Group_free(cfg);

  return 0;
}



static
int AH_MsgRxh__Verify_Internal(GWEN_CRYPT_KEY *k,
                               GWEN_CRYPT_PADDALGO *a,
                               const uint8_t *pInData,
                               uint32_t inLen,
                               const uint8_t *pSignatureData,
                               uint32_t signatureLen)
{

  int rv;
  GWEN_CRYPT_PADDALGOID aid;

  aid=GWEN_Crypt_PaddAlgo_GetId(a);

  if (aid==GWEN_Crypt_PaddAlgoId_Iso9796_2 ||
      aid==GWEN_Crypt_PaddAlgoId_Pkcs1_2 ||
      aid==GWEN_Crypt_PaddAlgoId_Pkcs1_Pss_Sha256) {
    GWEN_BUFFER *tbuf;
    uint32_t l;

    /* these algos add random numbers, we must use encrypt fn here and
     * compare the decrypted and unpadded data with the source data */
    tbuf=GWEN_Buffer_new(0, signatureLen+16, 0, 0);
    l=GWEN_Buffer_GetMaxUnsegmentedWrite(tbuf);
    rv=GWEN_Crypt_Key_Encipher(k,
                               pSignatureData, signatureLen,
                               (uint8_t *)GWEN_Buffer_GetStart(tbuf),
                               &l);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(tbuf);
      return rv;
    }
    GWEN_Buffer_IncrementPos(tbuf, l);
    GWEN_Buffer_AdjustUsedBytes(tbuf);

    if (aid==GWEN_Crypt_PaddAlgoId_Pkcs1_Pss_Sha256) {
      int nbits;
      uint8_t *modPtr;
      /* nasty hack, do something better later */
      uint8_t modBuffer[AH_MSGRXH_MAXKEYBUF];
      uint32_t modLen;
      GWEN_MDIGEST *md;

      modPtr=&modBuffer[0];
      modLen=AH_MSGRXH_MAXKEYBUF;
      /* calculate real number of bits */
      rv=GWEN_Crypt_KeyRsa_GetModulus(k, modPtr, &modLen);

      nbits=modLen*8;
      while (modLen && *modPtr==0) {
        nbits-=8;
        modLen--;
        modPtr++;
      }
      if (modLen) {
        uint8_t b=*modPtr;
        int i;
        uint8_t mask=0x80;

        for (i=0; i<8; i++) {
          if (b & mask)
            break;
          nbits--;
          mask>>=1;
        }
      }

      if (nbits==0) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Empty modulus");
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_GENERIC;
      }

      md=GWEN_MDigest_Sha256_new();
      rv=GWEN_Padd_VerifyPkcs1Pss((const uint8_t *) GWEN_Buffer_GetStart(tbuf),
                                  GWEN_Buffer_GetUsedBytes(tbuf),
                                  nbits,
                                  pInData, inLen,
                                  inLen,
                                  md);
      GWEN_MDigest_free(md);
      if (rv<0) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "here (%d)", rv);
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Error verifying this data:");
        GWEN_Buffer_Dump(tbuf, 2);
        GWEN_Buffer_free(tbuf);
        return rv;
      }
    }
    else {
      rv=GWEN_Padd_UnapplyPaddAlgo(a, tbuf);
      if (rv<0) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
        GWEN_Buffer_free(tbuf);
        return rv;
      }
      l=GWEN_Buffer_GetUsedBytes(tbuf);

      if (l!=inLen) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Signature length doesn't match");
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_VERIFY;
      }
      if (memcmp(pInData, GWEN_Buffer_GetStart(tbuf), l)!=0) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Signature doesn't match:");
        GWEN_Buffer_free(tbuf);
        return GWEN_ERROR_VERIFY;
      }
    }
    GWEN_Buffer_free(tbuf);
  }
  else {
    GWEN_BUFFER *srcBuf;

    /* copy to a buffer for padding */
    srcBuf=GWEN_Buffer_new(0, inLen, 0, 0);
    GWEN_Buffer_AppendBytes(srcBuf, (const char *)pInData, inLen);

    /* padd according to given algo */
    rv=GWEN_Padd_ApplyPaddAlgo(a, srcBuf);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(srcBuf);
      return rv;
    }

    /* verify with key */
    rv=GWEN_Crypt_Key_Verify(k,
                             (const uint8_t *)GWEN_Buffer_GetStart(srcBuf),
                             GWEN_Buffer_GetUsedBytes(srcBuf),
                             pSignatureData,
                             signatureLen);
    GWEN_Buffer_free(srcBuf);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }



  return 0;
}



int AH_Msg_VerifyRxh(AH_MSG *hmsg, GWEN_DB_NODE *gr)
{
  AH_HBCI *h;
  GWEN_LIST *sigheads;
  GWEN_LIST *sigtails;
  GWEN_DB_NODE *n;
  int nonSigHeads;
  int nSigheads;
  unsigned int dataBegin;
  char *dataStart;
  unsigned int dataLength;
  unsigned int i;
  AB_USER *u;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  int ksize;
  int rv;
  uint32_t gid;
  uint32_t hashLen;
  AH_HASH_ALG hashAlg;
  AH_OPMODE opMode;
  uint8_t rxhVersion;
  RXH_PARAMETER *rxh_parameter;
  GWEN_CRYPT_KEY *bankPubSignKey;

  /* get correct parameters */
  u=AH_Dialog_GetDialogOwner(hmsg->dialog);
  assert(u);

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
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Profile RDH%d is not supported!", rxhVersion);
      return AB_ERROR_NOT_INIT;
    }
    break;
  default:
    return GWEN_ERROR_INTERNAL;
  }
  assert(hmsg);
  h=AH_Dialog_GetHbci(hmsg->dialog);
  assert(h);


  gid=0;


  hashAlg = rxh_parameter->hashAlgS;
  opMode= rxh_parameter->opmodSignS;

  /* get crypt token of signer */
  rv=AB_Banking_GetCryptToken(AH_HBCI_GetBankingApi(h),
                              AH_User_GetTokenType(u),
                              AH_User_GetTokenName(u),
                              &ct);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Could not get crypt token for user \"%s\" (%d)",
             AB_User_GetUserId(u), rv);
    return rv;
  }

  /* open CryptToken if necessary */
  if (!GWEN_Crypt_Token_IsOpen(ct)) {
    GWEN_Crypt_Token_AddModes(ct, GWEN_CRYPT_TOKEN_MODE_DIRECT_SIGN);
    rv=GWEN_Crypt_Token_Open(ct, 0, gid);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Could not open crypt token for user \"%s\" (%d)",
               AB_User_GetUserId(u), rv);
      return rv;
    }
  }

  /* get context and key info */
  ctx=GWEN_Crypt_Token_GetContext(ct,
                                  AH_User_GetTokenContextId(u),
                                  gid);
  if (ctx==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Context %d not found on crypt token [%s:%s]",
             AH_User_GetTokenContextId(u),
             GWEN_Crypt_Token_GetTypeName(ct),
             GWEN_Crypt_Token_GetTokenName(ct));
    return GWEN_ERROR_NOT_FOUND;
  }

  /* let's go */
  sigheads=GWEN_List_new();

  /* enumerate signature heads */
  nonSigHeads=0;
  nSigheads=0;
  n=GWEN_DB_GetFirstGroup(gr);
  while (n) {
    if (strcasecmp(GWEN_DB_GroupName(n), "SigHead")==0) {
      /* found a signature head */
      if (nonSigHeads) {
        DBG_ERROR(AQHBCI_LOGDOMAIN,
                  "Found some unsigned parts at the beginning");
        GWEN_List_free(sigheads);
        return GWEN_ERROR_BAD_DATA;
      }
      GWEN_List_PushBack(sigheads, n);
      nSigheads++;
    }
    else if (strcasecmp(GWEN_DB_GroupName(n), "MsgHead")!=0) {
      if (nSigheads)
        break;
      nonSigHeads++;
    }
    n=GWEN_DB_GetNextGroup(n);
  } /* while */

  if (!n) {
    if (nSigheads) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
                "Found Signature heads but no other segments");
      GWEN_List_free(sigheads);
      return GWEN_ERROR_BAD_DATA;
    }
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "No signatures");
    GWEN_List_free(sigheads);
    return 0;
  }

  /* only now we need the verify key */
  /* the public sign key is not on the RDH card, but exchanged in the
   * initial key exchange and resides in the user information
   */
  bankPubSignKey=AH_User_GetBankPubSignKey(u);
  if (bankPubSignKey==NULL) {
    /* this may be the first message with the public keys from the bank server,
     * if its signed, the key is transmitted in the message and my be verified with
     * different methods ([HBCI] B.3.1.3, case A):
     * * the zka card contains the hash in EF_NOTEPAD
     * * a certificate is sent with the message to verify
     * * INI letter
     *
     * check message for "S"-KEy, look up if there is a hash on the chip card
     */
    bankPubSignKey=AH_MsgRxh_VerifyInitialSignKey(ct, ctx, u, gr);

    if (bankPubSignKey==NULL) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "No public bank sign key for user [%s]",
               AB_User_GetUserName(u));
      return GWEN_ERROR_NOT_FOUND;
    }
  }

  ksize=GWEN_Crypt_Key_GetKeySize(bankPubSignKey);
  assert(ksize<=AH_MSGRXH_MAXKEYBUF);

  /* store begin of signed data */
  dataBegin=GWEN_DB_GetIntValue(n, "segment/pos", 0, 0);
  if (!dataBegin) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No position specifications in segment");
    GWEN_List_free(sigheads);
    return GWEN_ERROR_BAD_DATA;
  }

  /* now get first signature tail */
  while (n) {
    if (strcasecmp(GWEN_DB_GroupName(n), "SigTail")==0) {
      unsigned int currpos;

      /* found a signature tail */
      currpos=GWEN_DB_GetIntValue(n, "segment/pos", 0, 0);
      if (!currpos || dataBegin>currpos) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad position specification in Signature tail");
        GWEN_List_free(sigheads);
        return GWEN_ERROR_BAD_DATA;
      }
      dataLength=currpos-dataBegin;
      break;
    }
    n=GWEN_DB_GetNextGroup(n);
  } /* while */

  if (!n) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No signature tail found");
    GWEN_List_free(sigheads);
    return GWEN_ERROR_BAD_DATA;
  }

  sigtails=GWEN_List_new();
  while (n) {
    if (strcasecmp(GWEN_DB_GroupName(n), "SigTail")!=0)
      break;
    GWEN_List_PushBack(sigtails, n);
    n=GWEN_DB_GetNextGroup(n);
  } /* while */

  if (!n) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Message tail expected");
    GWEN_List_free(sigheads);
    GWEN_List_free(sigtails);
    return GWEN_ERROR_BAD_DATA;
  }

  if (strcasecmp(GWEN_DB_GroupName(n), "MsgTail")!=0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Unexpected segment (msg tail expected)");
    GWEN_List_free(sigheads);
    GWEN_List_free(sigtails);
    return GWEN_ERROR_BAD_DATA;
  }

  n=GWEN_DB_GetNextGroup(n);
  if (n) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Unexpected segment (end expected)");
    GWEN_List_free(sigheads);
    GWEN_List_free(sigtails);
    return GWEN_ERROR_BAD_DATA;
  }

  if (GWEN_List_GetSize(sigheads)!=
      GWEN_List_GetSize(sigtails)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Number of signature heads (%d) does not match "
              "number of signature tails (%d)",
              GWEN_List_GetSize(sigheads),
              GWEN_List_GetSize(sigtails));
    GWEN_List_free(sigheads);
    GWEN_List_free(sigtails);
    return GWEN_ERROR_BAD_DATA;
  }

  /* ok, now verify all signatures */
  dataStart=GWEN_Buffer_GetStart(hmsg->buffer)+dataBegin;
  for (i=0; i< GWEN_List_GetSize(sigtails); i++) {
    GWEN_DB_NODE *sighead;
    GWEN_DB_NODE *sigtail;
    const uint8_t *p;
    uint32_t l;
    int rv;
    uint8_t hash[32];
    const char *signerId;

    /* get signature tail */
    sigtail=(GWEN_DB_NODE *)GWEN_List_GetBack(sigtails);

    /* get corresponding signature head */
    sighead=(GWEN_DB_NODE *)GWEN_List_GetFront(sigheads);

    if (!sighead || !sigtail) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
                "No signature head/tail left (internal error)");
      GWEN_List_free(sigheads);
      GWEN_List_free(sigtails);
      return GWEN_ERROR_INTERNAL;
    }

    GWEN_List_PopBack(sigtails);
    GWEN_List_PopFront(sigheads);

    signerId=GWEN_DB_GetCharValue(sighead, "key/userid", 0,
                                  I18N("unknown"));

    /* some checks */
    if (strcasecmp(GWEN_DB_GetCharValue(sighead, "ctrlref", 0, ""),
                   GWEN_DB_GetCharValue(sigtail, "ctrlref", 0, ""))!=0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Non-matching signature tail");
      GWEN_List_free(sigheads);
      GWEN_List_free(sigtails);
      return GWEN_ERROR_BAD_DATA;
    }

    /* hash signature head and data */
    if (1) {
      GWEN_MDIGEST *md;

      /* hash sighead + data */
      p=(const uint8_t *)GWEN_Buffer_GetStart(hmsg->buffer);
      p+=GWEN_DB_GetIntValue(sighead,
                             "segment/pos",
                             0,
                             0);
      l=GWEN_DB_GetIntValue(sighead,
                            "segment/length",
                            0,
                            0);
      switch (hashAlg) {
      case AH_HashAlg_Sha1:
        md=GWEN_MDigest_Sha1_new();
        break;
      case AH_HashAlg_Sha256:
      case AH_HashAlg_Sha256Sha256:
        md=GWEN_MDigest_Sha256_new();
        break;
      case AH_HashAlg_Ripmed160:
        md=GWEN_MDigest_Rmd160_new();
        break;
      default:
        md=NULL;
      }

      /* first round */
      rv=GWEN_MDigest_Begin(md);
      if (rv==0)
        /* digest signature head */
        rv=GWEN_MDigest_Update(md, p, l);
      if (rv==0)
        /* digest data */
        rv=GWEN_MDigest_Update(md, (const uint8_t *)dataStart, dataLength);
      if (rv==0)
        rv=GWEN_MDigest_End(md);
      if (rv<0) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Hash error (%d)", rv);
        GWEN_MDigest_free(md);
        GWEN_List_free(sigheads);
        GWEN_List_free(sigtails);
        return rv;
      }
      memmove(hash,
              GWEN_MDigest_GetDigestPtr(md),
              GWEN_MDigest_GetDigestSize(md));

      /* second round */
      if (hashAlg==AH_HashAlg_Sha256Sha256) {
        uint8_t hash1[32];
        memmove(hash1,
                hash,
                GWEN_MDigest_GetDigestSize(md));
        rv=GWEN_MDigest_Begin(md);
        if (rv==0)
          /* digest signature head */
          rv=GWEN_MDigest_Update(md, hash1, sizeof(hash1));
        if (rv==0)
          rv=GWEN_MDigest_End(md);
        if (rv<0) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Hash error (%d)", rv);
          GWEN_MDigest_free(md);
          GWEN_List_free(sigheads);
          GWEN_List_free(sigtails);
          return rv;
        }
        memmove(hash,
                GWEN_MDigest_GetDigestPtr(md),
                GWEN_MDigest_GetDigestSize(md));
      }
      hashLen=GWEN_MDigest_GetDigestSize(md);
      GWEN_MDigest_free(md);
    }

    /* verify signature */
    p=GWEN_DB_GetBinValue(sigtail, "signature", 0, 0, 0, &l);

    if (p && l) {

      GWEN_CRYPT_PADDALGO *algo;

      switch (opMode) {
      case AH_Opmode_Iso9796_1:
        algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_Iso9796_1A4);
        GWEN_Crypt_PaddAlgo_SetPaddSize(algo, GWEN_Crypt_Key_GetKeySize(bankPubSignKey));
        break;

      case AH_Opmode_Iso9796_2:
        algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_Iso9796_2);
        GWEN_Crypt_PaddAlgo_SetPaddSize(algo, GWEN_Crypt_Key_GetKeySize(bankPubSignKey));
        break;

      case AH_Opmode_Rsa_Pkcs1_v1_5:
        algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_Pkcs1_2);
        GWEN_Crypt_PaddAlgo_SetPaddSize(algo, GWEN_Crypt_Key_GetKeySize(bankPubSignKey));
        break;

      case AH_Opmode_Rsa_Pss:
        algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_Pkcs1_Pss_Sha256);
        GWEN_Crypt_PaddAlgo_SetPaddSize(algo, GWEN_Crypt_Key_GetKeySize(bankPubSignKey));
        break;
      default:
        return GWEN_ERROR_INTERNAL;
      }

      rv=AH_MsgRxh__Verify_Internal(bankPubSignKey, algo,
                                    hash, hashLen, p, l);
      GWEN_Crypt_PaddAlgo_free(algo);

      if (rv) {
        if (rv==GWEN_ERROR_NO_KEY) {
          DBG_ERROR(AQHBCI_LOGDOMAIN,
                    "Unable to verify signature of user \"%s\" (no key)",
                    signerId);
          GWEN_Gui_ProgressLog(gid,
                               GWEN_LoggerLevel_Error,
                               I18N("Unable to verify signature (no key)"));
        }
        else {
          GWEN_BUFFER *tbuf;

          tbuf=GWEN_Buffer_new(0, hashLen, 0, 1);
          if (rv==GWEN_ERROR_VERIFY) {
            DBG_ERROR(AQHBCI_LOGDOMAIN,
                      "Invalid signature of user \"%s\"", signerId);
            GWEN_Gui_ProgressLog(gid,
                                 GWEN_LoggerLevel_Error,
                                 I18N("Invalid signature!!!"));
            GWEN_Buffer_AppendString(tbuf, "!");
          }
          else {
            GWEN_Gui_ProgressLog(gid,
                                 GWEN_LoggerLevel_Error,
                                 I18N("Could not verify signature"));
            DBG_ERROR(AQHBCI_LOGDOMAIN,
                      "Could not verify data with medium of user \"%s\" (%d)",
                      AB_User_GetUserId(u), rv);
            GWEN_Buffer_AppendString(tbuf, "?");
          }

          GWEN_Buffer_AppendString(tbuf, signerId);
          AH_Msg_AddSignerId(hmsg, GWEN_Buffer_GetStart(tbuf));
          GWEN_Buffer_free(tbuf);
        }
      }
      else {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Message signed by \"%s\"", signerId);
        AH_Msg_AddSignerId(hmsg, signerId);
      }
    }
    else {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "No signature");
      GWEN_List_free(sigheads);
      GWEN_List_free(sigtails);
      return GWEN_ERROR_BAD_DATA;
    }

    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Verification done");
  } /* for */

  GWEN_List_free(sigheads);
  GWEN_List_free(sigtails);
  return 0;
}


#include "msgcrypt_rxh_decrypt.c"


