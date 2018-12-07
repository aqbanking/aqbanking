/***************************************************************************
 $RCSfile: adminjobs.h,v $
                             -------------------
    cvs         : $Id: adminjobs.h,v 1.3 2006/01/13 13:59:58 cstim Exp $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "keys.h"
#include "xml.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/base64.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/cryptkeyrsa.h>
#include <gwenhywfar/mdigest.h>

#include <ctype.h>




EB_RC EB_Key_toBin(const GWEN_CRYPT_KEY *k,
                   const char *userId,
		   const char *version,
                   int keySize,
		   GWEN_BUFFER *buf) {
  int i;
  char numbuf[32];
  int rv;
  uint8_t kbuf[300];
  uint32_t klen;

  if (strlen(version)!=4) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Invalid version \"%s\"", version);
    return EB_RC_KEYMGMT_UNSUPPORTED_VERSION_SIGNATURE;
  }

  if (!userId || !*userId) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Missing key owner");
    return EB_RC_INVALID_REQUEST;
  }
  GWEN_Buffer_AppendString(buf, version);
  i=(int)strlen(userId);
  if (i>8) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "User id too long");
    return EB_RC_INVALID_REQUEST;
  }
  GWEN_Buffer_AppendString(buf, userId);
  if (i<8)
    GWEN_Buffer_FillWithBytes(buf, ' ', (uint32_t)(8-i));

  /* get exponent */
  klen=sizeof(kbuf);
  rv=GWEN_Crypt_KeyRsa_GetExponent(k, kbuf, &klen);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return EB_RC_INVALID_REQUEST;
  }
  snprintf(numbuf, sizeof(numbuf), "%04d", keySize);
  GWEN_Buffer_AppendString(buf, numbuf);
  if (klen<128)
    GWEN_Buffer_FillWithBytes(buf, 0, 128-klen);
  GWEN_Buffer_AppendBytes(buf, (const char*)kbuf, klen);

  /* get modulus */
  klen=sizeof(kbuf);
  rv=GWEN_Crypt_KeyRsa_GetModulus(k, kbuf, &klen);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return EB_RC_INVALID_REQUEST;
  }
  snprintf(numbuf, sizeof(numbuf), "%04d", keySize);
  GWEN_Buffer_AppendString(buf, numbuf);
  if (klen<128)
    GWEN_Buffer_FillWithBytes(buf, 0, 128-klen);
  GWEN_Buffer_AppendBytes(buf, (const char*)kbuf, klen);


  GWEN_Buffer_FillWithBytes(buf, ' ', 236);

  return 0;
}



EB_RC EB_Key_fromBin(GWEN_CRYPT_KEY **k,
		     const char *version,
		     char *bufUserId,
		     unsigned int lenUserId,
                     const char *p, unsigned int bsize) {
  GWEN_CRYPT_KEY *key;
  char tmpbuf[32];
  const char *t;
  char *d;
  uint32_t nsize;
  int i;
  const uint8_t *mPtr;
  uint32_t mLen;
  const uint8_t *ePtr;
  uint32_t eLen;
  uint32_t keySize;

  if (bsize<512) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Too few bytes (%d)", bsize);
    return EB_RC_INVALID_REQUEST;
  }

  if (strlen(version)!=4) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Invalid version \"%s\"", version);
    return EB_RC_INVALID_REQUEST;
  }

  /* compare version */
  if (strncasecmp(p, version, 4)!=0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Unexpected version [%s]", p);
    return EB_RC_KEYMGMT_UNSUPPORTED_VERSION_SIGNATURE;
  }
  t=p+4;

  /* copy user id */
  d=bufUserId;
  i=0;
  while(t[i] && t[i]!=' ' && (unsigned int)i<(lenUserId-1) && i<8)
    *(d++)=t[i++];
  *d=0;
  t+=8;

  /* get LExponent */
  d=tmpbuf;
  i=0;
  while(isdigit(t[i]) && i<4)
    *(d++)=t[i++];
  *d=0;
  i=0;
  sscanf(tmpbuf, "%d", &i);
  t+=4;
  nsize=(i+7)/8;

  /* get Exponent */
  ePtr=(const uint8_t*)(t+(128-nsize));
  eLen=nsize;
  t+=128;

  /* get LModulus */
  d=tmpbuf;
  i=0;
  while(t[i] && t[i]!=' ' && i<4)
    *(d++)=t[i++];
  *d=0;
  i=0;
  sscanf(tmpbuf, "%d", &i);
  t+=4;

  /* calculate number of bits/bytes */
  nsize=i;
  if (nsize>2048)
    nsize=4096;
  else if (nsize>1024)
    nsize=2048;
  else if (nsize>768)
    nsize=1024;
  else
    nsize=768;
  keySize=(nsize+7)/8;

  /* calculate real size of modulus */
  nsize=(i+7)/8;

  /* get Modulus */
  mPtr=(const uint8_t*)(t+(128-nsize));
  mLen=nsize;
  t+=128;

  /* create key */
  key=GWEN_Crypt_KeyRsa_fromModExp(keySize,
				   mPtr, mLen,
				   ePtr, eLen);
  if (!key) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "Could not create RSA key");
    return EB_RC_INTERNAL_ERROR;
  }
  *k=key;

  return 0;
}



EB_RC EB_Key_toXml(GWEN_CRYPT_KEY *k, xmlNodePtr node) {
  int rv;
  uint8_t kbuf[300];
  uint32_t klen;

  /* get modulus */
  klen=sizeof(kbuf);
  rv=GWEN_Crypt_KeyRsa_GetModulus(k, kbuf, &klen);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return EB_RC_INVALID_REQUEST;
  }
  else {
    GWEN_BUFFER *b64buf;

    b64buf=GWEN_Buffer_new(0, 256, 0, 1);
    if (GWEN_Base64_Encode(kbuf, klen, b64buf, 0)) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not encode data");
      GWEN_Buffer_free(b64buf);
      return EB_RC_INTERNAL_ERROR;
    }
    EB_Xml_SetCharValue(node,
			"PubKeyValue/ds:RSAKeyValue/Modulus",
			GWEN_Buffer_GetStart(b64buf));
    GWEN_Buffer_free(b64buf);
  }


  /* get exponent */
  klen=sizeof(kbuf);
  rv=GWEN_Crypt_KeyRsa_GetExponent(k, kbuf, &klen);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return EB_RC_INVALID_REQUEST;
  }
  else {
    GWEN_BUFFER *b64buf;

    b64buf=GWEN_Buffer_new(0, 256, 0, 1);
    if (GWEN_Base64_Encode(kbuf, klen, b64buf, 0)) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not encode data");
      GWEN_Buffer_free(b64buf);
      return EB_RC_INVALID_REQUEST;
    }
    EB_Xml_SetCharValue(node,
			"PubKeyValue/ds:RSAKeyValue/Exponent",
			GWEN_Buffer_GetStart(b64buf));
    GWEN_Buffer_free(b64buf);
  }

  return 0;
}



EB_RC EB_Key_fromXml(GWEN_CRYPT_KEY **k, xmlNodePtr node) {
  const char *s;
  GWEN_CRYPT_KEY *key;
  uint8_t eBuf[512];
  uint32_t eLen;
  uint8_t mBuf[512];
  uint32_t mLen;

  s=EB_Xml_GetCharValue(node, "PubKeyValue/ds:RSAKeyValue/Modulus", 0);
  if (s) {
    GWEN_BUFFER *b64buf;
    const uint8_t *p;

    b64buf=GWEN_Buffer_new(0, 256, 0, 1);
    if (GWEN_Base64_Decode((const unsigned char*) s, 0, b64buf)) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not decode data");
      GWEN_Buffer_free(b64buf);
      return EB_RC_INVALID_REQUEST;
    }

    if (GWEN_Buffer_GetUsedBytes(b64buf)>sizeof(mBuf)) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Modulus too long");
      GWEN_Buffer_free(b64buf);
      return EB_RC_INVALID_REQUEST;
    }

    /* correctly calculate key length */
    mLen=GWEN_Buffer_GetUsedBytes(b64buf);
    p=(const uint8_t*)GWEN_Buffer_GetStart(b64buf);
    while(mLen && *p==0) {
      mLen--;
      p++;
    }
    DBG_ERROR(0, "Real key size is: %d (from %d)",
	      mLen, GWEN_Buffer_GetUsedBytes(b64buf));
    memmove(mBuf, p, mLen);
    GWEN_Buffer_free(b64buf);
  }
  else {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "No modulus in key data (%s)",
              node->name);
    return EB_RC_INVALID_REQUEST;
  }

  s=EB_Xml_GetCharValue(node, "PubKeyValue/ds:RSAKeyValue/Exponent", 0);
  if (s) {
    GWEN_BUFFER *b64buf;

    b64buf=GWEN_Buffer_new(0, 256, 0, 1);
    if (GWEN_Base64_Decode((const unsigned char*) s, 0, b64buf)) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not decode data");
      GWEN_Buffer_free(b64buf);
      return EB_RC_INVALID_REQUEST;
    }

    if (GWEN_Buffer_GetUsedBytes(b64buf)>sizeof(eBuf)) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Exponent too long");
      GWEN_Buffer_free(b64buf);
      return EB_RC_INVALID_REQUEST;
    }

    memmove(eBuf,
	    GWEN_Buffer_GetStart(b64buf),
	    GWEN_Buffer_GetUsedBytes(b64buf));
    eLen=GWEN_Buffer_GetUsedBytes(b64buf);
    GWEN_Buffer_free(b64buf);
  }
  else {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "No exponent in key data");
    return EB_RC_INVALID_REQUEST;
  }

  /* create key */
  key=GWEN_Crypt_KeyRsa_fromModExp(mLen,
				   mBuf, mLen,
				   eBuf, eLen);
  if (!key) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "Could not create RSA key");
    return EB_RC_INTERNAL_ERROR;
  }
  *k=key;

  return 0;
}



int EB_Key_BuildHashSha1(const GWEN_CRYPT_KEY *k, GWEN_BUFFER *hbuf, int encode64) {
  GWEN_BUFFER *buf1;
  GWEN_BUFFER *buf2;
  char *s;
  int rv;
  uint8_t kbuf[300];
  uint32_t klen;
  GWEN_MDIGEST *md;

  buf1=GWEN_Buffer_new(0, 256, 0, 1);
  buf2=GWEN_Buffer_new(0, 520, 0, 1);

  /* get exponent */
  klen=sizeof(kbuf);
  rv=GWEN_Crypt_KeyRsa_GetExponent(k, kbuf, &klen);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(buf2);
    GWEN_Buffer_free(buf1);
    return EB_RC_INVALID_REQUEST;
  }

  rv=GWEN_Text_ToHexBuffer((const char*)kbuf, klen, buf1, 0, 0, 0);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "No modulus in key");
    GWEN_Buffer_free(buf2);
    GWEN_Buffer_free(buf1);
    return EB_RC_INTERNAL_ERROR;
  }

  s=GWEN_Buffer_GetStart(buf1);
  while(*s=='0')
    s++;
  GWEN_Buffer_AppendString(buf2, s);
  GWEN_Buffer_AppendString(buf2, " ");
  GWEN_Buffer_Reset(buf1);

  /* get modulus */
  klen=sizeof(kbuf);
  rv=GWEN_Crypt_KeyRsa_GetModulus(k, kbuf, &klen);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(buf2);
    GWEN_Buffer_free(buf1);
    return EB_RC_INVALID_REQUEST;
  }

  rv=GWEN_Text_ToHexBuffer((const char*)kbuf, klen, buf1, 0, 0, 0);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "No modulus in key");
    GWEN_Buffer_free(buf2);
    GWEN_Buffer_free(buf1);
    return EB_RC_INTERNAL_ERROR;
  }

  s=GWEN_Buffer_GetStart(buf1);
  while(*s=='0')
    s++;
  GWEN_Buffer_AppendString(buf2, s);

  GWEN_Buffer_free(buf1);

  /* convert to lower case */
  s=GWEN_Buffer_GetStart(buf2);
  while(*s) {
    *s=(char)tolower(*s);
    s++;
  }

  /* hash it */
  md=GWEN_MDigest_Sha1_new();

  /* begin */
  rv=GWEN_MDigest_Begin(md);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(buf2);
    return EB_RC_INTERNAL_ERROR;
  }

  /* update */
  rv=GWEN_MDigest_Update(md,
			 (const uint8_t*) GWEN_Buffer_GetStart(buf2),
			 GWEN_Buffer_GetUsedBytes(buf2));
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(buf2);
    return EB_RC_INTERNAL_ERROR;
  }

  /* end */
  rv=GWEN_MDigest_End(md);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(buf2);
    return EB_RC_INTERNAL_ERROR;
  }

  if (encode64) {
    if (GWEN_Base64_Encode(GWEN_MDigest_GetDigestPtr(md),
			   GWEN_MDigest_GetDigestSize(md),
			   hbuf, 0)) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not encode data");
      GWEN_MDigest_free(md);
      GWEN_Buffer_free(buf2);
      return EB_RC_INTERNAL_ERROR;
    }
  }
  else
    GWEN_Buffer_AppendBytes(hbuf,
			    (const char*)GWEN_MDigest_GetDigestPtr(md),
			    GWEN_MDigest_GetDigestSize(md));
  GWEN_MDigest_free(md);

  /* cleanup */
  GWEN_Buffer_free(buf2);

  return 0;
}



int EB_Key_BuildHashSha256(const GWEN_CRYPT_KEY *k, GWEN_BUFFER *hbuf, int encode64) {
  GWEN_BUFFER *buf1;
  GWEN_BUFFER *buf2;
  char *s;
  int rv;
  uint8_t kbuf[300];
  uint32_t klen;
  GWEN_MDIGEST *md;

  buf1=GWEN_Buffer_new(0, 256, 0, 1);
  buf2=GWEN_Buffer_new(0, 520, 0, 1);

  /* get exponent */
  klen=sizeof(kbuf);
  rv=GWEN_Crypt_KeyRsa_GetExponent(k, kbuf, &klen);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(buf2);
    GWEN_Buffer_free(buf1);
    return EB_RC_INVALID_REQUEST;
  }

  rv=GWEN_Text_ToHexBuffer((const char*)kbuf, klen, buf1, 0, 0, 0);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "No modulus in key");
    GWEN_Buffer_free(buf2);
    GWEN_Buffer_free(buf1);
    return EB_RC_INTERNAL_ERROR;
  }

  s=GWEN_Buffer_GetStart(buf1);
  while(*s=='0')
    s++;
  GWEN_Buffer_AppendString(buf2, s);
  GWEN_Buffer_AppendString(buf2, " ");
  GWEN_Buffer_Reset(buf1);

  /* get modulus */
  klen=sizeof(kbuf);
  rv=GWEN_Crypt_KeyRsa_GetModulus(k, kbuf, &klen);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(buf2);
    GWEN_Buffer_free(buf1);
    return EB_RC_INVALID_REQUEST;
  }

  rv=GWEN_Text_ToHexBuffer((const char*)kbuf, klen, buf1, 0, 0, 0);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "No modulus in key");
    GWEN_Buffer_free(buf2);
    GWEN_Buffer_free(buf1);
    return EB_RC_INTERNAL_ERROR;
  }

  s=GWEN_Buffer_GetStart(buf1);
  while(*s=='0')
    s++;
  GWEN_Buffer_AppendString(buf2, s);

  GWEN_Buffer_free(buf1);

  /* convert to lower case */
  s=GWEN_Buffer_GetStart(buf2);
  while(*s) {
    *s=(char)tolower(*s);
    s++;
  }

  /* hash it */
  md=GWEN_MDigest_Sha256_new();

  /* begin */
  rv=GWEN_MDigest_Begin(md);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(buf2);
    return EB_RC_INTERNAL_ERROR;
  }

  /* update */
  rv=GWEN_MDigest_Update(md,
			 (const uint8_t*) GWEN_Buffer_GetStart(buf2),
			 GWEN_Buffer_GetUsedBytes(buf2));
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(buf2);
    return EB_RC_INTERNAL_ERROR;
  }

  /* end */
  rv=GWEN_MDigest_End(md);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(buf2);
    return EB_RC_INTERNAL_ERROR;
  }

  if (encode64) {
    if (GWEN_Base64_Encode(GWEN_MDigest_GetDigestPtr(md),
			   GWEN_MDigest_GetDigestSize(md),
			   hbuf, 0)) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not encode data");
      GWEN_MDigest_free(md);
      GWEN_Buffer_free(buf2);
      return EB_RC_INTERNAL_ERROR;
    }
  }
  else
    GWEN_Buffer_AppendBytes(hbuf,
			    (const char*)GWEN_MDigest_GetDigestPtr(md),
			    GWEN_MDigest_GetDigestSize(md));
  GWEN_MDigest_free(md);

  /* cleanup */
  GWEN_Buffer_free(buf2);

  return 0;
}



int EB_Key_Info_BuildHashSha1(const GWEN_CRYPT_TOKEN_KEYINFO *ki,
			      GWEN_BUFFER *hbuf,
			      int encode64) {
  GWEN_BUFFER *buf1;
  GWEN_BUFFER *buf2;
  char *s;
  int rv;
  GWEN_MDIGEST *md;
  const uint8_t *p;
  uint32_t len;

  buf1=GWEN_Buffer_new(0, 256, 0, 1);
  buf2=GWEN_Buffer_new(0, 520, 0, 1);

  /* get exponent */
  p=GWEN_Crypt_Token_KeyInfo_GetExponentData(ki);
  len=GWEN_Crypt_Token_KeyInfo_GetExponentLen(ki);
  if (p==NULL || len==0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here");
    GWEN_Buffer_free(buf2);
    GWEN_Buffer_free(buf1);
    return GWEN_ERROR_NO_DATA;
  }

  rv=GWEN_Text_ToHexBuffer((const char*)p, len, buf1, 0, 0, 0);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "No modulus in key");
    GWEN_Buffer_free(buf2);
    GWEN_Buffer_free(buf1);
    return EB_RC_INTERNAL_ERROR;
  }
  s=GWEN_Buffer_GetStart(buf1);
  while(*s=='0')
    s++;
  GWEN_Buffer_AppendString(buf2, s);

  GWEN_Buffer_AppendString(buf2, " ");
  GWEN_Buffer_Reset(buf1);

  /* get modulus */
  p=GWEN_Crypt_Token_KeyInfo_GetModulusData(ki);
  len=GWEN_Crypt_Token_KeyInfo_GetModulusLen(ki);
  if (p==NULL || len==0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here");
    GWEN_Buffer_free(buf2);
    GWEN_Buffer_free(buf1);
    return EB_RC_INVALID_REQUEST;
  }
  rv=GWEN_Text_ToHexBuffer((const char*)p, len, buf1, 0, 0, 0);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "No modulus in key");
    GWEN_Buffer_free(buf2);
    GWEN_Buffer_free(buf1);
    return EB_RC_INTERNAL_ERROR;
  }

  s=GWEN_Buffer_GetStart(buf1);
  while(*s=='0')
    s++;
  GWEN_Buffer_AppendString(buf2, s);

  GWEN_Buffer_free(buf1);

  /* convert to lower case */
  s=GWEN_Buffer_GetStart(buf2);
  while(*s) {
    *s=(char)tolower(*s);
    s++;
  }

  /* hash it */
  md=GWEN_MDigest_Sha1_new();

  /* begin */
  rv=GWEN_MDigest_Begin(md);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(buf2);
    return EB_RC_INTERNAL_ERROR;
  }

  /* update */
  rv=GWEN_MDigest_Update(md,
			 (const uint8_t*) GWEN_Buffer_GetStart(buf2),
			 GWEN_Buffer_GetUsedBytes(buf2));
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(buf2);
    return EB_RC_INTERNAL_ERROR;
  }

  /* end */
  rv=GWEN_MDigest_End(md);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(buf2);
    return EB_RC_INTERNAL_ERROR;
  }

  if (encode64) {
    if (GWEN_Base64_Encode(GWEN_MDigest_GetDigestPtr(md),
			   GWEN_MDigest_GetDigestSize(md),
			   hbuf, 0)) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not encode data");
      GWEN_MDigest_free(md);
      GWEN_Buffer_free(buf2);
      return EB_RC_INTERNAL_ERROR;
    }
  }
  else
    GWEN_Buffer_AppendBytes(hbuf,
			    (const char*)GWEN_MDigest_GetDigestPtr(md),
			    GWEN_MDigest_GetDigestSize(md));
  GWEN_MDigest_free(md);

  /* cleanup */
  GWEN_Buffer_free(buf2);

  return 0;
}



int EB_Key_Info_BuildHashSha256(const GWEN_CRYPT_TOKEN_KEYINFO *ki,
				GWEN_BUFFER *hbuf,
				int encode64) {
  GWEN_BUFFER *buf1;
  GWEN_BUFFER *buf2;
  char *s;
  int rv;
  GWEN_MDIGEST *md;
  const uint8_t *p;
  uint32_t len;

  buf1=GWEN_Buffer_new(0, 256, 0, 1);
  buf2=GWEN_Buffer_new(0, 520, 0, 1);

  /* get exponent */
  p=GWEN_Crypt_Token_KeyInfo_GetExponentData(ki);
  len=GWEN_Crypt_Token_KeyInfo_GetExponentLen(ki);
  if (p==NULL || len==0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here");
    GWEN_Buffer_free(buf2);
    GWEN_Buffer_free(buf1);
    return GWEN_ERROR_NO_DATA;
  }

  rv=GWEN_Text_ToHexBuffer((const char*)p, len, buf1, 0, 0, 0);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "No modulus in key");
    GWEN_Buffer_free(buf2);
    GWEN_Buffer_free(buf1);
    return EB_RC_INTERNAL_ERROR;
  }
  s=GWEN_Buffer_GetStart(buf1);
  while(*s=='0')
    s++;
  GWEN_Buffer_AppendString(buf2, s);

  GWEN_Buffer_AppendString(buf2, " ");
  GWEN_Buffer_Reset(buf1);

  /* get modulus */
  p=GWEN_Crypt_Token_KeyInfo_GetModulusData(ki);
  len=GWEN_Crypt_Token_KeyInfo_GetModulusLen(ki);
  if (p==NULL || len==0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here");
    GWEN_Buffer_free(buf2);
    GWEN_Buffer_free(buf1);
    return EB_RC_INVALID_REQUEST;
  }
  rv=GWEN_Text_ToHexBuffer((const char*)p, len, buf1, 0, 0, 0);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "No modulus in key");
    GWEN_Buffer_free(buf2);
    GWEN_Buffer_free(buf1);
    return EB_RC_INTERNAL_ERROR;
  }

  s=GWEN_Buffer_GetStart(buf1);
  while(*s=='0')
    s++;
  GWEN_Buffer_AppendString(buf2, s);

  GWEN_Buffer_free(buf1);

  /* convert to lower case */
  s=GWEN_Buffer_GetStart(buf2);
  while(*s) {
    *s=(char)tolower(*s);
    s++;
  }

  /* hash it */
  md=GWEN_MDigest_Sha256_new();

  /* begin */
  rv=GWEN_MDigest_Begin(md);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(buf2);
    return EB_RC_INTERNAL_ERROR;
  }

  /* update */
  rv=GWEN_MDigest_Update(md,
			 (const uint8_t*) GWEN_Buffer_GetStart(buf2),
			 GWEN_Buffer_GetUsedBytes(buf2));
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(buf2);
    return EB_RC_INTERNAL_ERROR;
  }

  /* end */
  rv=GWEN_MDigest_End(md);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(buf2);
    return EB_RC_INTERNAL_ERROR;
  }

  if (encode64) {
    if (GWEN_Base64_Encode(GWEN_MDigest_GetDigestPtr(md),
			   GWEN_MDigest_GetDigestSize(md),
			   hbuf, 0)) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not encode data");
      GWEN_MDigest_free(md);
      GWEN_Buffer_free(buf2);
      return EB_RC_INTERNAL_ERROR;
    }
  }
  else
    GWEN_Buffer_AppendBytes(hbuf,
			    (const char*)GWEN_MDigest_GetDigestPtr(md),
			    GWEN_MDigest_GetDigestSize(md));
  GWEN_MDigest_free(md);

  /* cleanup */
  GWEN_Buffer_free(buf2);

  return 0;
}



int EB_Key_Info_BuildSigHash_Rmd160(const GWEN_CRYPT_TOKEN_KEYINFO *ki,
				    GWEN_BUFFER *hbuf) {
  const uint8_t *p;
  uint32_t len;
  GWEN_BUFFER *bbuf;
  GWEN_MDIGEST *md;
  int rv;

  bbuf=GWEN_Buffer_new(0, 256, 0, 1);

  /* get exponent */
  p=GWEN_Crypt_Token_KeyInfo_GetExponentData(ki);
  len=GWEN_Crypt_Token_KeyInfo_GetExponentLen(ki);
  if (p==NULL || len==0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here");
    GWEN_Buffer_free(bbuf);
    return GWEN_ERROR_NO_DATA;
  }
  if (len<128)
    GWEN_Buffer_FillWithBytes(bbuf, 0, 128-len);
  GWEN_Buffer_AppendBytes(bbuf, (const char*)p, len);

  /* get modulus */
  p=GWEN_Crypt_Token_KeyInfo_GetModulusData(ki);
  len=GWEN_Crypt_Token_KeyInfo_GetModulusLen(ki);
  if (p==NULL || len==0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here");
    GWEN_Buffer_free(bbuf);
    return GWEN_ERROR_NO_DATA;
  }
  if (len<128)
    GWEN_Buffer_FillWithBytes(bbuf, 0, 128-len);
  GWEN_Buffer_AppendBytes(bbuf, (const char*)p, len);

  /* hash */
  md=GWEN_MDigest_Rmd160_new();
  assert(md);
  rv=GWEN_MDigest_Begin(md);
  if (rv<0) {
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(bbuf);
    return rv;
  }
  rv=GWEN_MDigest_Update(md,
			 (const uint8_t*)GWEN_Buffer_GetStart(bbuf),
			 GWEN_Buffer_GetUsedBytes(bbuf));
  if (rv<0) {
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(bbuf);
    return rv;
  }
  rv=GWEN_MDigest_End(md);
  if (rv<0) {
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(bbuf);
    return rv;
  }
  GWEN_Buffer_free(bbuf);

  /* add hash to buffer */
  GWEN_Buffer_AppendBytes(hbuf,
			  (const char*)GWEN_MDigest_GetDigestPtr(md),
			  GWEN_MDigest_GetDigestSize(md));
  GWEN_MDigest_free(md);

  return 0;
}



int EB_Key_Info_BuildSigHash_Sha256(const GWEN_CRYPT_TOKEN_KEYINFO *ki, GWEN_BUFFER *hbuf) {
  const uint8_t *p;
  char *t;
  uint32_t len;
  GWEN_BUFFER *bbuf;
  GWEN_BUFFER *xbuf;
  GWEN_MDIGEST *md;
  int rv;

  bbuf=GWEN_Buffer_new(0, 256, 0, 1);
  xbuf=GWEN_Buffer_new(0, 256, 0, 1);

  /* get exponent */
  p=GWEN_Crypt_Token_KeyInfo_GetExponentData(ki);
  len=GWEN_Crypt_Token_KeyInfo_GetExponentLen(ki);
  if (p==NULL || len==0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here");
    GWEN_Buffer_free(xbuf);
    GWEN_Buffer_free(bbuf);
    return GWEN_ERROR_NO_DATA;
  }
  rv=GWEN_Text_ToHexBuffer((const char*)p, len, bbuf, 0, 0, 0);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here");
    GWEN_Buffer_free(xbuf);
    GWEN_Buffer_free(bbuf);
    return GWEN_ERROR_NO_DATA;
  }

  p=(const uint8_t*)GWEN_Buffer_GetStart(bbuf);
  while(*p=='0')
    p++;
  GWEN_Buffer_AppendString(xbuf, (const char*)p);
  GWEN_Buffer_AppendByte(xbuf, ' ');
  GWEN_Buffer_Reset(bbuf);

  /* get modulus */
  p=GWEN_Crypt_Token_KeyInfo_GetModulusData(ki);
  len=GWEN_Crypt_Token_KeyInfo_GetModulusLen(ki);
  if (p==NULL || len==0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here");
    GWEN_Buffer_free(xbuf);
    GWEN_Buffer_free(bbuf);
    return GWEN_ERROR_NO_DATA;
  }

  rv=GWEN_Text_ToHexBuffer((const char*) p, len, bbuf, 0, 0, 0);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here");
    GWEN_Buffer_free(xbuf);
    GWEN_Buffer_free(bbuf);
    return GWEN_ERROR_NO_DATA;
  }

  p=(const uint8_t*)GWEN_Buffer_GetStart(bbuf);
  while(*p=='0')
    p++;
  GWEN_Buffer_AppendString(xbuf, (const char*) p);
  GWEN_Buffer_free(bbuf);

  /* lowercase */
  t=GWEN_Buffer_GetStart(xbuf);
  while(*t) {
    *t=tolower(*t);
    t++;
  }

  /* hash */
  md=GWEN_MDigest_Sha256_new();
  assert(md);
  rv=GWEN_MDigest_Begin(md);
  if (rv<0) {
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(xbuf);
    return rv;
  }
  rv=GWEN_MDigest_Update(md,
			 (const uint8_t*)GWEN_Buffer_GetStart(xbuf),
			 GWEN_Buffer_GetUsedBytes(xbuf));
  if (rv<0) {
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(xbuf);
    return rv;
  }
  rv=GWEN_MDigest_End(md);
  if (rv<0) {
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(xbuf);
    return rv;
  }
  GWEN_Buffer_free(xbuf);

  /* add hash to buffer */
  GWEN_Buffer_AppendBytes(hbuf,
			  (const char*)GWEN_MDigest_GetDigestPtr(md),
			  GWEN_MDigest_GetDigestSize(md));
  GWEN_MDigest_free(md);

  return 0;
}



EB_RC EB_Key_Info_ReadXml(GWEN_CRYPT_TOKEN_KEYINFO *ki, xmlNodePtr node) {
  const char *s;
  uint8_t eBuf[512];
  uint32_t eLen;
  uint8_t mBuf[512];
  uint32_t mLen;
  uint32_t keySize;

  s=EB_Xml_GetCharValue(node, "PubKeyValue/RSAKeyValue/Modulus", 0);
  if (s) {
    GWEN_BUFFER *b64buf;
    unsigned int nsize;
    const uint8_t *p;

    b64buf=GWEN_Buffer_new(0, 256, 0, 1);
    if (GWEN_Base64_Decode((const unsigned char*) s, 0, b64buf)) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not decode data");
      GWEN_Buffer_free(b64buf);
      return EB_RC_INVALID_REQUEST;
    }

    if (GWEN_Buffer_GetUsedBytes(b64buf)>sizeof(mBuf)) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Modulus too long");
      GWEN_Buffer_free(b64buf);
      return EB_RC_INVALID_REQUEST;
    }

    /* correctly calculate key length */
    nsize=GWEN_Buffer_GetUsedBytes(b64buf);
    p=(const uint8_t*)GWEN_Buffer_GetStart(b64buf);
    while(nsize && *p==0) {
      nsize--;
      p++;
    }
    DBG_INFO(AQEBICS_LOGDOMAIN, "Real key size is: %d/%d (from %d)",
	     nsize, nsize*8, GWEN_Buffer_GetUsedBytes(b64buf));
    nsize*=8;

    if (nsize>2048)
      nsize=4096;
    else if (nsize>1024)
      nsize=2048;
    else if (nsize>768)
      nsize=1024;
    else
      nsize=768;
    keySize=(nsize+7)/8;

    DBG_INFO(AQEBICS_LOGDOMAIN, "Adjusted key size is: %d", keySize);
    memmove(mBuf, p, keySize);
    mLen=keySize;
    GWEN_Buffer_free(b64buf);
  }
  else {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "No modulus in key data (%s)",
              node->name);
    return EB_RC_INVALID_REQUEST;
  }

  s=EB_Xml_GetCharValue(node, "PubKeyValue/RSAKeyValue/Exponent", 0);
  if (s) {
    GWEN_BUFFER *b64buf;

    b64buf=GWEN_Buffer_new(0, 256, 0, 1);
    if (GWEN_Base64_Decode((const unsigned char*) s, 0, b64buf)) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not decode data");
      GWEN_Buffer_free(b64buf);
      return EB_RC_INVALID_REQUEST;
    }

    if (GWEN_Buffer_GetUsedBytes(b64buf)>sizeof(eBuf)) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Exponent too long");
      GWEN_Buffer_free(b64buf);
      return EB_RC_INVALID_REQUEST;
    }

    memmove(eBuf,
	    GWEN_Buffer_GetStart(b64buf),
	    GWEN_Buffer_GetUsedBytes(b64buf));
    eLen=GWEN_Buffer_GetUsedBytes(b64buf);
    GWEN_Buffer_free(b64buf);
  }
  else {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "No exponent in key data");
    return EB_RC_INVALID_REQUEST;
  }

  /* store info */
  GWEN_Crypt_Token_KeyInfo_SetKeySize(ki, keySize);
  GWEN_Crypt_Token_KeyInfo_SetModulus(ki, mBuf, mLen);
  GWEN_Crypt_Token_KeyInfo_SetExponent(ki, eBuf, eLen);
  GWEN_Crypt_Token_KeyInfo_AddFlags(ki,
				    GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
                                    GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT);
  return 0;
}



int EB_Key_Info_toXml(const GWEN_CRYPT_TOKEN_KEYINFO *ki, xmlNodePtr node) {
  int rv;
  const uint8_t *p;
  uint32_t len;

  /* get modulus */
  p=GWEN_Crypt_Token_KeyInfo_GetModulusData(ki);
  len=GWEN_Crypt_Token_KeyInfo_GetModulusLen(ki);
  if (p==NULL || len==0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here");
    return GWEN_ERROR_NO_DATA;
  }
  else {
    GWEN_BUFFER *b64buf;
    GWEN_BUFFER *tmpbuf=NULL;
#if 0
    if (*p!=0) {
      tmpbuf=GWEN_Buffer_new(0, len+1, 0, 1);
      GWEN_Buffer_AppendByte(tmpbuf, 0);
      GWEN_Buffer_AppendBytes(tmpbuf, (const char*)p, len);
      p=(const uint8_t*)GWEN_Buffer_GetStart(tmpbuf);
      len++;
    }
#endif
    b64buf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_Base64_Encode(p, len, b64buf, 0);
    GWEN_Buffer_free(tmpbuf); tmpbuf=NULL;
    if (rv<0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not encode data (%d)", rv);
      GWEN_Buffer_free(b64buf);
      return EB_RC_INTERNAL_ERROR;
    }
    EB_Xml_SetCharValue(node,
			"PubKeyValue/ds:RSAKeyValue/Modulus",
			GWEN_Buffer_GetStart(b64buf));
    GWEN_Buffer_free(b64buf);
  }


  /* get exponent */
  p=GWEN_Crypt_Token_KeyInfo_GetExponentData(ki);
  len=GWEN_Crypt_Token_KeyInfo_GetExponentLen(ki);
  if (p==NULL || len==0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return GWEN_ERROR_NO_DATA;
  }
  else {
    GWEN_BUFFER *b64buf;

    b64buf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_Base64_Encode(p, len, b64buf, 0);
    if (rv<0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not encode data (%d)", rv);
      GWEN_Buffer_free(b64buf);
      return EB_RC_INTERNAL_ERROR;
    }
    EB_Xml_SetCharValue(node,
			"PubKeyValue/ds:RSAKeyValue/Exponent",
			GWEN_Buffer_GetStart(b64buf));
    GWEN_Buffer_free(b64buf);
  }

  return 0;
}



EB_RC EB_Key_Info_toBin(const GWEN_CRYPT_TOKEN_KEYINFO *ki,
			const char *userId,
			const char *version,
			int keySize,
			GWEN_BUFFER *buf) {
  int i;
  char numbuf[32];
  const uint8_t *p;
  uint32_t len;

  if (strlen(version)!=4) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Invalid version \"%s\"", version);
    return EB_RC_KEYMGMT_UNSUPPORTED_VERSION_SIGNATURE;
  }

  if (!userId || !*userId) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Missing key owner");
    return EB_RC_INVALID_REQUEST;
  }
  GWEN_Buffer_AppendString(buf, version);
  i=(int)strlen(userId);
  if (i>8) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "User id too long");
    return EB_RC_INVALID_REQUEST;
  }
  GWEN_Buffer_AppendString(buf, userId);
  if (i<8)
    GWEN_Buffer_FillWithBytes(buf, ' ', 8-i);

  /* get exponent */
  p=GWEN_Crypt_Token_KeyInfo_GetExponentData(ki);
  len=GWEN_Crypt_Token_KeyInfo_GetExponentLen(ki);
  if (p==NULL || len==0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here");
    return EB_RC_INVALID_REQUEST;
  }

  snprintf(numbuf, sizeof(numbuf), "%04d", keySize);
  GWEN_Buffer_AppendString(buf, numbuf);
  if (len<128)
    GWEN_Buffer_FillWithBytes(buf, 0, 128-len);
  GWEN_Buffer_AppendBytes(buf, (const char*)p, len);

  /* get modulus */
  p=GWEN_Crypt_Token_KeyInfo_GetModulusData(ki);
  len=GWEN_Crypt_Token_KeyInfo_GetModulusLen(ki);
  if (p==NULL || len==0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here");
    return EB_RC_INVALID_REQUEST;
  }
  snprintf(numbuf, sizeof(numbuf), "%04d", keySize);
  GWEN_Buffer_AppendString(buf, numbuf);
  if (len<128)
    GWEN_Buffer_FillWithBytes(buf, 0, 128-len);
  GWEN_Buffer_AppendBytes(buf, (const char*)p, len);

  GWEN_Buffer_FillWithBytes(buf, ' ', 236);

  return 0;
}




