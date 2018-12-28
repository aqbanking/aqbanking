/***************************************************************************
    begin       : Fri Feb 29 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#include "msg/zip.h"
#include "msg/keys.h"

#include <gwenhywfar/padd.h>
#include <gwenhywfar/cryptkeysym.h>



int EBC_Provider_ExtractSessionKey(AB_PROVIDER *pro,
				   AB_USER *u,
				   xmlNodePtr node,
				   GWEN_CRYPT_KEY **pKey) {
  const char *s;
  GWEN_BUFFER *d64buf;
  int rv;
  GWEN_CRYPT_KEY *key;
  xmlNodePtr nodeX;
  xmlChar *prop;
  GWEN_BUFFER *keyBuffer;
  GWEN_BUFFER *dkbuf;
  uint32_t l;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  const GWEN_CRYPT_TOKEN_KEYINFO *ki=NULL;
  uint32_t kid;
  GWEN_CRYPT_PADDALGO *algo;
  int ksize;
  int kversion=0;

  /* get pubkey digest node */
  nodeX=EB_Xml_GetNode(node, "EncryptionPubKeyDigest",
		       GWEN_PATH_FLAGS_NAMEMUSTEXIST);
  if (nodeX==NULL) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "No pubkey digest");
    return GWEN_ERROR_BAD_DATA;
  }

  /* check version */
  s=EBC_User_GetCryptVersion(u);
  if (!(s && *s))
    s="E001";
  prop=xmlGetProp(nodeX, BAD_CAST "Version");
  if (prop==NULL) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "No \"Version\" in pubkey digest");
    return GWEN_ERROR_BAD_DATA;
  }
  if (strcasecmp(s, (const char*)prop)!=0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Unexpected crypt version in pubkey digest");
    xmlFree(prop);
    return GWEN_ERROR_BAD_DATA;
  }
  if (strcasecmp((const char*)prop, "E001")==0) {
    xmlFree(prop);
    /* check digest algo */
    prop=xmlGetProp(nodeX, BAD_CAST "Algorithm");
    if (prop==NULL) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "No \"Algorithm\" in pubkey digest");
      return GWEN_ERROR_BAD_DATA;
    }
    if (strcasecmp((const char*)prop,
		   "http://www.w3.org/2000/09/xmldsig#sha1")!=0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "Unexpected digest algo [%s]",
	       prop);
      xmlFree(prop);
      return GWEN_ERROR_BAD_DATA;
    }
    xmlFree(prop);
    kversion=1;
  }
  else if (strcasecmp((const char*)prop, "E002")==0) {
    xmlFree(prop);
    /* check digest algo */
    prop=xmlGetProp(nodeX, BAD_CAST "Algorithm");
    if (prop==NULL) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "No \"Algorithm\" in pubkey digest");
      return GWEN_ERROR_BAD_DATA;
    }
    if (strcasecmp((const char*)prop,
		   "http://www.w3.org/2001/04/xmlenc#sha256")!=0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "Unexpected digest algo [%s]",
	       prop);
      xmlFree(prop);
      return GWEN_ERROR_BAD_DATA;
    }
    xmlFree(prop);
    kversion=2;
  }
  else {
    DBG_INFO(AQEBICS_LOGDOMAIN, "Unexpected crypt version [%s]",
	     prop);
    xmlFree(prop);
    return GWEN_ERROR_BAD_DATA;
  }

  /* get pubkey digest */
  s=EB_Xml_GetCharValue(node, "EncryptionPubKeyDigest", NULL);
  assert(s);
  if (s==NULL) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "No pubkey digest");
    return GWEN_ERROR_BAD_DATA;
  }

  /* get crypt token and context */
  rv=EBC_Provider_MountToken(pro, u, &ct, &ctx);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* get crypt key */
  kid=GWEN_Crypt_Token_Context_GetDecipherKeyId(ctx);
  if (kid)
    ki=GWEN_Crypt_Token_GetKeyInfo(ct, kid,
				   GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
				   GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT,
				   0);
  if (ki==NULL) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Crypt key info not found on crypt token");
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Error,
			 I18N("Crypt key info not found on crypt token"));
    return GWEN_ERROR_NOT_FOUND;
  }
  ksize=GWEN_Crypt_Token_KeyInfo_GetKeySize(ki);

  /* create hash for our own pub crypt key */
  d64buf=GWEN_Buffer_new(0, 256, 0, 1);
  if (kversion==1) {
    EB_Key_Info_BuildHashSha1(ki, d64buf, 1);

    /* compare hashes */
    if (strcasecmp(s, GWEN_Buffer_GetStart(d64buf))!=0) {
      DBG_INFO(AQEBICS_LOGDOMAIN,
	       "Pubkey digest does not match");
      GWEN_Buffer_free(d64buf);
      return GWEN_ERROR_NO_KEY;
    }
    GWEN_Buffer_free(d64buf);

    /* get transaction key */
    s=EB_Xml_GetCharValue(node, "TransactionKey", NULL);
    if (s==NULL) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "No transaction key");
      return GWEN_ERROR_BAD_DATA;
    }

    /* decode transaction key */
    keyBuffer=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_Base64_Decode((const uint8_t*)s, 0, keyBuffer);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "Could not decode transaction key (%d)", rv);
      GWEN_Buffer_free(keyBuffer);
      return rv;
    }

    /* decrypt session key */
    dkbuf=GWEN_Buffer_new(0, ksize+16, 0, 1);

    /* select padd algo */
    ksize=GWEN_Crypt_Token_KeyInfo_GetKeySize(ki);
    algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_Pkcs1_2);
    GWEN_Crypt_PaddAlgo_SetPaddSize(algo, ksize);
    l=GWEN_Buffer_GetMaxUnsegmentedWrite(dkbuf);
    rv=GWEN_Crypt_Token_Decipher(ct, kid,
				 algo,
				 (const uint8_t*)GWEN_Buffer_GetStart(keyBuffer),
				 GWEN_Buffer_GetUsedBytes(keyBuffer),
				 (uint8_t*)GWEN_Buffer_GetStart(dkbuf),
				 &l,
				 0);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(dkbuf);
      GWEN_Buffer_free(keyBuffer);
      return rv;
    }
    GWEN_Buffer_IncrementPos(dkbuf, l);
    GWEN_Buffer_AdjustUsedBytes(dkbuf);
    GWEN_Buffer_free(keyBuffer);

    /* check size of session key */
    if (l!=16) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "keysize!=16 (%d)", (int)l);
      GWEN_Buffer_free(dkbuf);
      return GWEN_ERROR_INTERNAL;
    }

    if (GWEN_Logger_GetLevel(AQEBICS_LOGDOMAIN)>=GWEN_LoggerLevel_Debug) {
      DBG_DEBUG(AQEBICS_LOGDOMAIN,
		"Decrypted session key:");
      GWEN_Buffer_Dump(dkbuf, 2);
    }

    /* create DES key */
    key=GWEN_Crypt_KeyDes3K_fromData(GWEN_Crypt_CryptMode_Cbc, 16,
				     (const uint8_t*)GWEN_Buffer_GetStart(dkbuf),
				     GWEN_Buffer_GetUsedBytes(dkbuf));
    if (key==NULL) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "Could not create DES key from data");
      GWEN_Buffer_free(dkbuf);
      return GWEN_ERROR_INTERNAL;
    }
    GWEN_Buffer_free(dkbuf);

    *pKey=key;
  }
  else if (kversion==2) {
    EB_Key_Info_BuildHashSha256(ki, d64buf, 1);

    /* compare hashes */
    if (strcasecmp(s, GWEN_Buffer_GetStart(d64buf))!=0) {
      DBG_INFO(AQEBICS_LOGDOMAIN,
	       "Pubkey digest does not match");
      GWEN_Buffer_free(d64buf);
      return GWEN_ERROR_NO_KEY;
    }
    GWEN_Buffer_free(d64buf);

    /* get transaction key */
    s=EB_Xml_GetCharValue(node, "TransactionKey", NULL);
    if (s==NULL) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "No transaction key");
      return GWEN_ERROR_BAD_DATA;
    }

    /* decode transaction key */
    keyBuffer=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_Base64_Decode((const uint8_t*)s, 0, keyBuffer);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "Could not decode transaction key (%d)", rv);
      GWEN_Buffer_free(keyBuffer);
      return rv;
    }

    /* decrypt session key */
    dkbuf=GWEN_Buffer_new(0, ksize+16, 0, 1);

    /* select padd algo */
    ksize=GWEN_Crypt_Token_KeyInfo_GetKeySize(ki);
    algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_Pkcs1_2);
    GWEN_Crypt_PaddAlgo_SetPaddSize(algo, ksize);
    l=GWEN_Buffer_GetMaxUnsegmentedWrite(dkbuf);
    rv=GWEN_Crypt_Token_Decipher(ct, kid,
				 algo,
				 (const uint8_t*)GWEN_Buffer_GetStart(keyBuffer),
				 GWEN_Buffer_GetUsedBytes(keyBuffer),
				 (uint8_t*)GWEN_Buffer_GetStart(dkbuf),
				 &l,
				 0);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(dkbuf);
      GWEN_Buffer_free(keyBuffer);
      return rv;
    }
    GWEN_Buffer_IncrementPos(dkbuf, l);
    GWEN_Buffer_AdjustUsedBytes(dkbuf);
    GWEN_Buffer_free(keyBuffer);

    /* check size of session key */
    if (l!=16) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "keysize!=16 (%d)", (int)l);
      GWEN_Buffer_free(dkbuf);
      return GWEN_ERROR_INTERNAL;
    }

    if (GWEN_Logger_GetLevel(AQEBICS_LOGDOMAIN)>=GWEN_LoggerLevel_Debug) {
      DBG_DEBUG(AQEBICS_LOGDOMAIN,
		"Decrypted session key:");
      GWEN_Buffer_Dump(dkbuf, 2);
    }

    /* create DES key */
    key=GWEN_Crypt_KeyAes128_fromData(GWEN_Crypt_CryptMode_Cbc, 16,
				      (const uint8_t*)GWEN_Buffer_GetStart(dkbuf),
				      GWEN_Buffer_GetUsedBytes(dkbuf));
    if (key==NULL) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "Could not create AES-128 key from data");
      GWEN_Buffer_free(dkbuf);
      return GWEN_ERROR_INTERNAL;
    }
    GWEN_Buffer_free(dkbuf);

    *pKey=key;
  }

  return 0;
}



int EBC_Provider_DecryptData(GWEN_UNUSED AB_PROVIDER *pro,
                             AB_USER *u,
			     GWEN_CRYPT_KEY *skey,
			     const uint8_t *p,
                             uint32_t len,
			     GWEN_BUFFER *msgBuffer) {
  GWEN_BUFFER *tbuf;
  uint32_t l;
  int rv;
  const char *s;

  DBG_DEBUG(AQEBICS_LOGDOMAIN, "Deciphering %d bytes of data", len);
  s=EBC_User_GetCryptVersion(u);
  if (!(s && *s))
    s="E001";
  if (strcasecmp(s, "E001")==0) {
    /* decrypt message with session key */
    tbuf=GWEN_Buffer_new(0, len+16, 0, 1);
    l=GWEN_Buffer_GetMaxUnsegmentedWrite(tbuf);
    /* reset IV */
    GWEN_Crypt_KeyDes3K_SetIV(skey, NULL, 0);
    /* now decrypt */
    rv=GWEN_Crypt_Key_Decipher(skey,
			       p, len,
			       (uint8_t*)GWEN_Buffer_GetPosPointer(tbuf),
			       &l);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN,
	       "Error deciphering %d bytes of data here (%d)",
	       (int)len, rv);
      GWEN_Buffer_free(tbuf);
      return rv;
    }

    GWEN_Buffer_IncrementPos(tbuf, l);
    GWEN_Buffer_AdjustUsedBytes(tbuf);

    /* unpadd message */
    rv=GWEN_Padd_UnpaddWithAnsiX9_23(tbuf);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    /* unzip */
    rv=EB_Zip_Inflate(GWEN_Buffer_GetStart(tbuf), l, msgBuffer);
    if (rv) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not unzip doc (%d)", rv);
      GWEN_Buffer_free(tbuf);
      return rv;
    }
    GWEN_Buffer_free(tbuf);
  }
  else if (strcasecmp(s, "E002")==0) {
    /* decrypt message with session key */
    tbuf=GWEN_Buffer_new(0, len+16, 0, 1);
    l=GWEN_Buffer_GetMaxUnsegmentedWrite(tbuf);
    /* reset IV */
    GWEN_Crypt_KeyAes128_SetIV(skey, NULL, 0);
    /* now decrypt */
    rv=GWEN_Crypt_Key_Decipher(skey,
			       p, len,
			       (uint8_t*)GWEN_Buffer_GetPosPointer(tbuf),
			       &l);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN,
	       "Error deciphering %d bytes of data here (%d)",
	       (int)len, rv);
      GWEN_Buffer_free(tbuf);
      return rv;
    }

    GWEN_Buffer_IncrementPos(tbuf, l);
    GWEN_Buffer_AdjustUsedBytes(tbuf);

    /* unpadd message */
    rv=GWEN_Padd_UnpaddWithAnsiX9_23FromMultipleOf(tbuf, 16);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    /* unzip */
    rv=EB_Zip_Inflate(GWEN_Buffer_GetStart(tbuf), l, msgBuffer);
    if (rv) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not unzip doc (%d)", rv);
      GWEN_Buffer_free(tbuf);
      return rv;
    }
    GWEN_Buffer_free(tbuf);
  }


  return 0;
}




