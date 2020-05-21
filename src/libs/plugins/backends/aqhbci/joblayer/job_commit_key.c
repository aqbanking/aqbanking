/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "job_commit_key.h"
#include "aqhbci/banking/user_l.h"
#include "aqhbci/banking/account_l.h"

#include "aqbanking/i18n_l.h"

#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int _verifiyInitialKey(AB_USER *user, AH_HBCI *h, GWEN_CRYPT_KEY *key, uint16_t sentModl, const char *keyName);




/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



void AH_Job_Commit_Key(AH_JOB *j, GWEN_DB_NODE *dbRd)
{
  const char *keytype;
  AH_HBCI *hbci;
  AB_USER *user;

  user=AH_Job_GetUser(j);
  hbci=AH_Job_GetHbci(j);

  keytype=GWEN_DB_GetCharValue(dbRd, "keyname/keytype",  0, NULL);
  if (keytype && *keytype) {
    GWEN_CRYPT_KEY *bpk;
    uint8_t *expp, *modp;
    unsigned int expl, modl;
    int keynum, keyver;
    uint16_t sentModulusLength;
    uint16_t nbits;
    int keySize;
    int verified=0;
    GWEN_CRYPT_KEY *bpsk;

    /* process received keys */
    keynum=GWEN_DB_GetIntValue(dbRd, "keyname/keynum",  0, -1);
    keyver=GWEN_DB_GetIntValue(dbRd, "keyname/keyversion",  0, -1);
    modp=(uint8_t *)GWEN_DB_GetBinValue(dbRd, "key/modulus",  0, NULL, 0, &modl);
    sentModulusLength=modl;
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Got Key with modulus length %d.", modl);
    /* skip zero bytes if any */
    while (modl && *modp==0) {
      modp++;
      modl--;
    }
    /* calc real length in bits for information purposes */
    nbits=modl*8;
    if (modl) {
      uint8_t b=*modp;
      int i;
      uint8_t mask=0x80;

      for (i=0; i<8; i++) {
        if (b & mask)
          break;
        nbits--;
        mask>>=1;
      }
    }

    /* calculate key size in bytes */
    if (modl<=96) /* could only be for RDH1, in this case we have to pad to 768 bits */
      keySize=96;
    else {
      keySize=modl;
    }
    DBG_INFO(AQHBCI_LOGDOMAIN, "Key has real modulus length %d bytes (%d bits) after skipping leading zero bits.", modl,
             nbits);
    expp=(uint8_t *)GWEN_DB_GetBinValue(dbRd, "key/exponent", 0, NULL, 0, &expl);
    bpk=GWEN_Crypt_KeyRsa_fromModExp(keySize, modp, modl, expp, expl);
    GWEN_Crypt_Key_SetKeyNumber(bpk, keynum);
    GWEN_Crypt_Key_SetKeyVersion(bpk, keyver);

    /* check if it was already verified and saved at the signature verification stage
     * (this is implemented for RDH7 and RDH9 only at the moment) */
    bpsk=AH_User_GetBankPubSignKey(user);
    if (bpsk) {
      int hasVerifiedFlag = GWEN_Crypt_KeyRsa_GetFlags(bpsk) & GWEN_CRYPT_KEYRSA_FLAGS_ISVERIFIED ;
      if (hasVerifiedFlag == GWEN_CRYPT_KEYRSA_FLAGS_ISVERIFIED)
        verified=1;
    }

    /* commit the new key */
    if (strcasecmp(keytype, "S")==0) {
      if (verified == 0) {
        verified=_verifiyInitialKey(user, hbci, bpk, sentModulusLength, "sign");
      }

      if (verified == 1) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Imported sign key.");
        GWEN_Crypt_KeyRsa_AddFlags(bpk, GWEN_CRYPT_KEYRSA_FLAGS_ISVERIFIED);
        AH_User_SetBankPubSignKey(user, bpk);
      }
      else {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Crypt key not imported.");
      }
    }
    else if (strcasecmp(keytype, "V")==0) {
      if (verified == 0) {
        verified = _verifiyInitialKey(user, hbci, bpk, sentModulusLength, "crypt");
      }

      if (verified == 1) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Imported crypt key.");
        GWEN_Crypt_KeyRsa_AddFlags(bpk, GWEN_CRYPT_KEYRSA_FLAGS_ISVERIFIED);
        AH_User_SetBankPubCryptKey(user, bpk);
      }
      else {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Crypt key not imported.");
      }
    }
    else {
      char hashString[1024];
      int expPadBytes=keySize-expl;
      uint8_t *mdPtr;
      unsigned int mdSize;
      /* pad exponent to length of modulus */
      GWEN_BUFFER *keyBuffer;
      GWEN_MDIGEST *md;
      uint16_t i;

      keyBuffer=GWEN_Buffer_new(NULL, 2*keySize, 0, 0);
      GWEN_Buffer_FillWithBytes(keyBuffer, 0x0, expPadBytes);
      GWEN_Buffer_AppendBytes(keyBuffer, (const char *)expp, expl);
      GWEN_Buffer_AppendBytes(keyBuffer, (const char *)modp, keySize);
      /*SHA256*/
      md=GWEN_MDigest_Sha256_new();
      GWEN_MDigest_Begin(md);
      GWEN_MDigest_Update(md, (uint8_t *)GWEN_Buffer_GetStart(keyBuffer), 2*keySize);
      GWEN_MDigest_End(md);
      mdPtr=GWEN_MDigest_GetDigestPtr(md);
      mdSize=GWEN_MDigest_GetDigestSize(md);
      memset(hashString, 0, 1024);
      for (i=0; i<mdSize; i++)
        sprintf(hashString+3*i, "%02x ", *(mdPtr+i));
      GWEN_MDigest_free(md);
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Received unknown server key: type=%s, num=%d, version=%d, hash=%s", keytype, keynum,
                keyver, hashString);
      GWEN_Gui_ProgressLog2(0,
                            GWEN_LoggerLevel_Warning,
                            I18N("Received unknown server key: type=%s, num=%d, version=%d, hash=%s"),
                            keytype, keynum, keyver, hashString);
    }
    if (bpk)
      GWEN_Crypt_Key_free(bpk);
  }
}



int _verifiyInitialKey(AB_USER *user, AH_HBCI *hbci, GWEN_CRYPT_KEY *key, uint16_t sentModl, const char *keyName)
{

  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  int rv;

  /* get crypt token of signer */
  rv=AB_Banking_GetCryptToken(AH_HBCI_GetBankingApi(hbci),
                              AH_User_GetTokenType(user),
                              AH_User_GetTokenName(user),
                              &ct);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Could not get crypt token for user \"%s\" (%d)",
             AB_User_GetUserId(user), rv);
    return rv;
  }

  /* open CryptToken if necessary */
  if (!GWEN_Crypt_Token_IsOpen(ct)) {
    GWEN_Crypt_Token_AddModes(ct, GWEN_CRYPT_TOKEN_MODE_DIRECT_SIGN);
    rv=GWEN_Crypt_Token_Open(ct, 0, 0);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Could not open crypt token for user \"%s\" (%d)",
               AB_User_GetUserId(user), rv);
      return rv;
    }
  }

  /* get context and key info */
  ctx=GWEN_Crypt_Token_GetContext(ct, AH_User_GetTokenContextId(user), 0);
  if (ctx==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Context %d not found on crypt token [%s:%s]",
             AH_User_GetTokenContextId(user),
             GWEN_Crypt_Token_GetTypeName(ct),
             GWEN_Crypt_Token_GetTokenName(ct));
    return GWEN_ERROR_NOT_FOUND;
  }

  return AH_User_VerifyInitialKey(ct, ctx, user, key, sentModl, keyName);

}






