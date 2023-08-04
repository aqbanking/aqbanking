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

#include "msgcrypt_rxh_common.h"
#include "msgcrypt.h"
#include "message_p.h"

#include "aqhbci/aqhbci_l.h"
#include "aqhbci/banking/user_l.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/cryptkeysym.h>
#include <gwenhywfar/padd.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/text.h>

#include <aqbanking/banking_be.h>


#define AH_MSGRXH_MAXKEYBUF 4096


/* ------------------------------------------------------------------------------------------------
 * static variables
 * ------------------------------------------------------------------------------------------------
 */

static const RXH_PARAMETER  rdh1_parameter= {
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

static const RXH_PARAMETER  rdh2_parameter= {
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

static const RXH_PARAMETER  rdh3_parameter= {
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

static const RXH_PARAMETER  rdh5_parameter= {
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

static const RXH_PARAMETER  rdh6_parameter= {
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

static const RXH_PARAMETER  rdh7_parameter= {
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

static const RXH_PARAMETER  rdh8_parameter= {
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

static const RXH_PARAMETER  rdh9_parameter= {
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

static const RXH_PARAMETER  rdh10_parameter= {
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

static const RXH_PARAMETER *rdh_parameter[11]= {
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

static const RXH_PARAMETER  rah7_parameter= {
  AH_CryptMode_Rah,
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

static const RXH_PARAMETER  rah9_parameter= {
  AH_CryptMode_Rah,
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

static const RXH_PARAMETER  rah10_parameter= {
  AH_CryptMode_Rah,
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

static const RXH_PARAMETER *rah_parameter[11]= {
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



/* ------------------------------------------------------------------------------------------------
 * code
 * ------------------------------------------------------------------------------------------------
 */

const RXH_PARAMETER *AH_MsgRxh_GetParameters(AH_CRYPT_MODE cryptMode, int rxhVersion)
{
  const RXH_PARAMETER *rParams=NULL;

  switch (cryptMode) {
  case AH_CryptMode_Rdh:
    if (rxhVersion<11)
      rParams=rdh_parameter[rxhVersion];
    if (rParams == NULL) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Profile RDH%d is not supported!", rxhVersion);
      return NULL;
    }
    break;
  case AH_CryptMode_Rah:
    if (rxhVersion<11)
      rParams=rah_parameter[rxhVersion];
    if (rParams == NULL) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Profile RDH%d is not supported!", rxhVersion);
      return NULL;
    }
    break;
  default:
    return NULL;
  }

  return rParams;
}



GWEN_CRYPT_TOKEN *AH_MsgRxh_GetOpenCryptToken(AH_MSG *hmsg)
{
  AH_HBCI *h;
  AB_USER *u;
  GWEN_CRYPT_TOKEN *ct;
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

  return ct;
}



const GWEN_CRYPT_TOKEN_CONTEXT *AH_MsgRxh_GetUserContext(AH_MSG *hmsg)
{
  AH_HBCI *h;
  AB_USER *u;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  uint32_t gid;

  h=AH_Dialog_GetHbci(hmsg->dialog);
  assert(h);
  u=AH_Dialog_GetDialogOwner(hmsg->dialog);
  assert(u);
  gid=0;

  ct=AH_MsgRxh_GetOpenCryptToken(hmsg);
  if (ct==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return NULL;
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



int AH_MsgRxh_PrepareCryptoSeg(AH_MSG *hmsg,
                               AB_USER *u,
                               const RXH_PARAMETER *rxh_parameter,
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
  const RXH_PARAMETER *rxh_parameter;
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




