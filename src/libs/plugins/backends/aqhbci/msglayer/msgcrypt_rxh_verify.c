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

#include "msgcrypt_rxh_verify.h"
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

#include <aqbanking/banking_be.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int _genHashForSigHeadAndSignedData(AH_MSG *hmsg,
                                           unsigned int signedDataPos, unsigned int signedDataLength,
                                           AH_HASH_ALG hashAlg,
                                           GWEN_DB_NODE *sighead, GWEN_DB_NODE *sigtail,
                                           uint8_t *destHashBuffer32Bytes);
static GWEN_MDIGEST *_getDigestorForAlgo(AH_HASH_ALG hashAlg);
static int _digestSigHeadAndData(GWEN_MDIGEST *md,
                                 const uint8_t *sigHeadPtr, uint32_t sigHeadLen,
                                 const uint8_t *signedDataPtr, uint32_t signedDataLength,
                                 uint8_t *destHashBuffer32Bytes);
static int _verifyAllSignatures(AH_MSG *hmsg,
                                GWEN_DB_NODE *dbParsedMsg,
                                GWEN_LIST *sigheads,
                                GWEN_LIST *sigtails,
                                unsigned int signedDataBeginPos,
                                unsigned int signedDataLength);
static void _addSignerAccordingToVerifyResult(AH_MSG *hmsg, AB_USER *u, const char *signerId, int rv);
static int _verifySignatureAgainstHash(GWEN_CRYPT_KEY *k,
                                       AH_OPMODE opMode,
                                       const uint8_t *pInData,
                                       uint32_t inLen,
                                       const uint8_t *pSignatureData,
                                       uint32_t signatureLen);

static int _verifyInternal(GWEN_CRYPT_KEY *k,
                           GWEN_CRYPT_PADDALGO *a,
                           const uint8_t *pInData,
                           uint32_t inLen,
                           const uint8_t *pSignatureData,
                           uint32_t signatureLen);
static GWEN_CRYPT_KEY *_verifyInitialSignKey(GWEN_CRYPT_TOKEN *ct,
                                             const GWEN_CRYPT_TOKEN_CONTEXT *ctx,
                                             AB_USER *user,
                                             GWEN_DB_NODE *dbParsedMsg);
static GWEN_CRYPT_KEY *_getBankPubSignKey(AH_MSG *hmsg, GWEN_DB_NODE *dbParsedMsg);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */

int AH_Msg_VerifyRxh(AH_MSG *hmsg, GWEN_DB_NODE *dbParsedMsg)
{
  int rv;

  rv=AH_Msg_VerifyWithCallback(hmsg, dbParsedMsg, _verifyAllSignatures);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  return 0;
}



int _verifyAllSignatures(AH_MSG *hmsg,
                         GWEN_DB_NODE *dbParsedMsg,
                         GWEN_LIST *sigheads,
                         GWEN_LIST *sigtails,
                         unsigned int signedDataBeginPos,
                         unsigned int signedDataLength)
{
  int i;
  AB_USER *u;
  GWEN_CRYPT_KEY *bankPubSignKey;
  const RXH_PARAMETER *rParams;

  u=AH_Dialog_GetDialogOwner(hmsg->dialog);
  rParams=AH_MsgRxh_GetParameters(AH_User_GetCryptMode(u), AH_User_GetRdhType(u));
  if (rParams==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No matching RxH parameters");
    return GWEN_ERROR_GENERIC;
  }
  bankPubSignKey=_getBankPubSignKey(hmsg, dbParsedMsg);
  if (bankPubSignKey==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return GWEN_ERROR_GENERIC;
  }

  for (i=0; i< GWEN_List_GetSize(sigtails); i++) {
    GWEN_DB_NODE *sighead;
    GWEN_DB_NODE *sigtail;
    const uint8_t *p;
    uint32_t l;
    int rv;
    uint8_t hash[32];
    const char *signerId;
    int hashLen;

    sigtail=(GWEN_DB_NODE *)GWEN_List_GetBack(sigtails);  /* get signature tail */
    sighead=(GWEN_DB_NODE *)GWEN_List_GetFront(sigheads); /* get corresponding signature head */
    GWEN_List_PopBack(sigtails);
    GWEN_List_PopFront(sigheads);

    signerId=GWEN_DB_GetCharValue(sighead, "key/userid", 0, I18N("unknown"));

    /* some checks */
    if (strcasecmp(GWEN_DB_GetCharValue(sighead, "ctrlref", 0, ""), GWEN_DB_GetCharValue(sigtail, "ctrlref", 0, ""))!=0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Non-matching signature tail");
      return GWEN_ERROR_BAD_DATA;
    }

    /* hash signature head and data */
    rv=_genHashForSigHeadAndSignedData(hmsg, signedDataBeginPos, signedDataLength, rParams->hashAlgS, sighead, sigtail,
                                       hash);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    hashLen=rv;

    /* verify signature */
    p=GWEN_DB_GetBinValue(sigtail, "signature", 0, 0, 0, &l);
    if (p && l) {
      rv=_verifySignatureAgainstHash(bankPubSignKey, rParams->opmodSignS, hash, hashLen, p, l);
      _addSignerAccordingToVerifyResult(hmsg, u, signerId, rv);
    }
    else {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "No signature");
      return GWEN_ERROR_BAD_DATA;
    }

    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Verification done");
  } /* for */

  return 0;
}



void _addSignerAccordingToVerifyResult(AH_MSG *hmsg, AB_USER *u, const char *signerId, int rv)
{
  if (rv) {
    if (rv==GWEN_ERROR_NO_KEY) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Unable to verify signature of user \"%s\" (no key)", signerId);
      GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, I18N("Unable to verify signature (no key)"));
    }
    else {
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, 32, 0, 1);
      if (rv==GWEN_ERROR_VERIFY) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Invalid signature of user \"%s\"", signerId);
        GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, I18N("Invalid signature!!!"));
        GWEN_Buffer_AppendString(tbuf, "!");
      }
      else {
        GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, I18N("Could not verify signature"));
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not verify data with medium of user \"%s\" (%d)", AB_User_GetUserId(u), rv);
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



int _genHashForSigHeadAndSignedData(AH_MSG *hmsg,
                                    unsigned int signedDataPos, unsigned int signedDataLength,
                                    AH_HASH_ALG hashAlg,
                                    GWEN_DB_NODE *sighead, GWEN_DB_NODE *sigtail,
                                    uint8_t *destHashBuffer32Bytes)
{
  GWEN_MDIGEST *md;
  const uint8_t *signedDataPtr;
  const uint8_t *sigHeadPtr;
  uint32_t sigHeadLen;
  int rv;
  int hashLen;

  /* hash sighead + data */
  sigHeadPtr=(const uint8_t *)GWEN_Buffer_GetStart(hmsg->buffer);
  signedDataPtr=sigHeadPtr+signedDataPos;
  sigHeadPtr+=GWEN_DB_GetIntValue(sighead, "segment/pos", 0, 0);
  sigHeadLen=GWEN_DB_GetIntValue(sighead, "segment/length", 0, 0);

  md=_getDigestorForAlgo(hashAlg);
  if (md==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Invalid hash algo %d", hashAlg);
    return GWEN_ERROR_INVALID;
  }

  /* first round */
  rv=_digestSigHeadAndData(md, sigHeadPtr, sigHeadLen, signedDataPtr, signedDataLength, destHashBuffer32Bytes);
  if (rv<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Hash error (%d)", rv);
    GWEN_MDigest_free(md);
    return rv;
  }

  /* possible second round */
  if (hashAlg==AH_HashAlg_Sha256Sha256) {
    uint8_t tempHashBuffer[32];

    memmove(tempHashBuffer, destHashBuffer32Bytes, GWEN_MDigest_GetDigestSize(md));
    rv=GWEN_MDigest_Digest(md, tempHashBuffer, GWEN_MDigest_GetDigestSize(md), destHashBuffer32Bytes, 32);
    if (rv<0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Hash error (%d)", rv);
      GWEN_MDigest_free(md);
      return rv;
    }
  }
  hashLen=GWEN_MDigest_GetDigestSize(md);
  GWEN_MDigest_free(md);

  return hashLen;
}



GWEN_MDIGEST *_getDigestorForAlgo(AH_HASH_ALG hashAlg)
{
  switch (hashAlg) {
  case AH_HashAlg_Sha1:
    return GWEN_MDigest_Sha1_new();
  case AH_HashAlg_Sha256:
  case AH_HashAlg_Sha256Sha256:
    return GWEN_MDigest_Sha256_new();
  case AH_HashAlg_Ripmed160:
    return GWEN_MDigest_Rmd160_new();
  default:
    return NULL;
  }
}



int _digestSigHeadAndData(GWEN_MDIGEST *md,
                          const uint8_t *sigHeadPtr, uint32_t sigHeadLen,
                          const uint8_t *signedDataPtr, uint32_t signedDataLength,
                          uint8_t *destHashBuffer32Bytes)
{
  int rv;

  /* first round */
  rv=GWEN_MDigest_Begin(md);
  if (rv==0)
    /* digest signature head */
    rv=GWEN_MDigest_Update(md, sigHeadPtr, sigHeadLen);
  if (rv==0)
    /* digest data */
    rv=GWEN_MDigest_Update(md, signedDataPtr, signedDataLength);
  if (rv==0)
    rv=GWEN_MDigest_End(md);
  if (rv<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Hash error (%d)", rv);
    GWEN_MDigest_free(md);
    return rv;
  }
  memmove(destHashBuffer32Bytes, GWEN_MDigest_GetDigestPtr(md), GWEN_MDigest_GetDigestSize(md));
  return GWEN_MDigest_GetDigestSize(md);
}



int _verifySignatureAgainstHash(GWEN_CRYPT_KEY *k,
                                AH_OPMODE opMode,
                                const uint8_t *pInData,
                                uint32_t inLen,
                                const uint8_t *pSignatureData,
                                uint32_t signatureLen)
{
  GWEN_CRYPT_PADDALGO *algo;
  int rv;

  switch (opMode) {
  case AH_Opmode_Iso9796_1:
    algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_Iso9796_1A4);
    GWEN_Crypt_PaddAlgo_SetPaddSize(algo, GWEN_Crypt_Key_GetKeySize(k));
    break;

  case AH_Opmode_Iso9796_2:
    algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_Iso9796_2);
    GWEN_Crypt_PaddAlgo_SetPaddSize(algo, GWEN_Crypt_Key_GetKeySize(k));
    break;

  case AH_Opmode_Rsa_Pkcs1_v1_5:
    algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_Pkcs1_2);
    GWEN_Crypt_PaddAlgo_SetPaddSize(algo, GWEN_Crypt_Key_GetKeySize(k));
    break;

  case AH_Opmode_Rsa_Pss:
    algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_Pkcs1_Pss_Sha256);
    GWEN_Crypt_PaddAlgo_SetPaddSize(algo, GWEN_Crypt_Key_GetKeySize(k));
    break;
  default:
    return GWEN_ERROR_INTERNAL;
  }

  rv=_verifyInternal(k, algo, pInData, inLen, pSignatureData, signatureLen);
  GWEN_Crypt_PaddAlgo_free(algo);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  return rv;
}




int _verifyInternal(GWEN_CRYPT_KEY *k,
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



GWEN_CRYPT_KEY *_verifyInitialSignKey(GWEN_CRYPT_TOKEN *ct,
                                      const GWEN_CRYPT_TOKEN_CONTEXT *ctx,
                                      AB_USER *user,
                                      GWEN_DB_NODE *dbParsedMsg)
{

  GWEN_DB_NODE *dbCurr;
  int haveKey=0;
  int verified;
  GWEN_CRYPT_KEY *bpk = NULL;

  /* search for "GetKeyResponse" */
  haveKey=0;
  dbCurr=GWEN_DB_GetFirstGroup(dbParsedMsg);
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



GWEN_CRYPT_KEY *_getBankPubSignKey(AH_MSG *hmsg, GWEN_DB_NODE *dbParsedMsg)
{
  AB_USER *u;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  GWEN_CRYPT_KEY *bankPubSignKey;

  u=AH_Dialog_GetDialogOwner(hmsg->dialog);

  ct=AH_MsgRxh_GetOpenCryptToken(hmsg);
  if (ct==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return NULL;
  }

  ctx=GWEN_Crypt_Token_GetContext(ct, AH_User_GetTokenContextId(u), 0);
  if (ctx==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Context %d not found on crypt token [%s:%s]",
             AH_User_GetTokenContextId(u),
             GWEN_Crypt_Token_GetTypeName(ct),
             GWEN_Crypt_Token_GetTokenName(ct));
    return NULL;
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
    bankPubSignKey=_verifyInitialSignKey(ct, ctx, u, dbParsedMsg);
    if (bankPubSignKey==NULL) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "No public bank sign key for user [%s]", AB_User_GetUserName(u));
      return NULL;
    }
  }

  if (GWEN_Crypt_Key_GetKeySize(bankPubSignKey)>AH_MSGRXH_MAXKEYBUF) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Size of banks public crypt key too large (%d > %d)",
              GWEN_Crypt_Key_GetKeySize(bankPubSignKey), AH_MSGRXH_MAXKEYBUF);
    return NULL;
  }

  return bankPubSignKey;
}



