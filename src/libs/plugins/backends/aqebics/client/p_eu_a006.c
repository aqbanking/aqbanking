/***************************************************************************
    begin       : Tue Jun 08 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/
#include <math.h>



int EBC_Eu_Sha256(const uint8_t *pData, uint32_t lData, GWEN_BUFFER *hbuf)
{
  GWEN_BUFFER *tbuf;
  GWEN_MDIGEST *md;
  int rv;

  tbuf=GWEN_Buffer_new(0, lData, 0, 1);
  while (lData--) {
    uint8_t c;

    c=*(pData++);
//    if (c!=13 && c!=10 && c!=26)
      GWEN_Buffer_AppendByte(tbuf, c);
  }

  /* hash (RMD160) */
  md=GWEN_MDigest_Sha256_new();
  rv=GWEN_MDigest_Begin(md);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(tbuf);
    return rv;
  }
  rv=GWEN_MDigest_Update(md,
                         (const uint8_t *)GWEN_Buffer_GetStart(tbuf),
                         GWEN_Buffer_GetUsedBytes(tbuf));
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(tbuf);
    return rv;
  }
  rv=GWEN_MDigest_End(md);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(tbuf);
    return rv;
  }
  GWEN_Buffer_AppendBytes(hbuf,
                          (const char *)GWEN_MDigest_GetDigestPtr(md),
                          GWEN_MDigest_GetDigestSize(md));
  GWEN_MDigest_free(md);
  GWEN_Buffer_free(tbuf);

  return 0;
}

int EBC_Provider_EuSign_A006(AB_PROVIDER *pro,
                             AB_USER *u,
                             const char *requestType,
                             const uint8_t *pMsg,
                             uint32_t lMsg,
                             GWEN_BUFFER *sbuf)
{
  EBC_PROVIDER *dp;
  GWEN_BUFFER *xbuf;
  GWEN_BUFFER *hbuf;
  GWEN_BUFFER *m2;
  GWEN_BUFFER *ps;
  GWEN_BUFFER *tbuf;
  GWEN_BUFFER *dbMask;
  GWEN_BUFFER *maskedDB;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  const GWEN_CRYPT_TOKEN_KEYINFO *ki;
  uint32_t keyId;
  int ksize;
  int rv;
  int hLen = 32; /*SHA256*/
  int emBits;
  int emLen;
  int sLen;
  int maskLen;
  char temp;
  char maskbyte[1];
  int count;
  
  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  /* get crypt token and context */
  rv=EBC_Provider_MountToken(pro, u, &ct, &ctx);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* get key id */
  keyId=GWEN_Crypt_Token_Context_GetSignKeyId(ctx);
  ki=GWEN_Crypt_Token_GetKeyInfo(ct,
                                 keyId,
                                 0xffffffff,
                                 0);
  if (ki==NULL) {
    DBG_INFO(AQEBICS_LOGDOMAIN,
             "Keyinfo %04x not found on crypt token [%s:%s]",
             keyId,
             GWEN_Crypt_Token_GetTypeName(ct),
             GWEN_Crypt_Token_GetTokenName(ct));
    GWEN_Crypt_Token_Close(ct, 0, 0);
    return GWEN_ERROR_NOT_FOUND;
  }

  xbuf=GWEN_Buffer_new(0, 40, 0, 1);
  EBC_Eu_Sha256(pMsg, lMsg, xbuf);

  /* add digestInfo to hash of SignedInfo */
  m2=GWEN_Buffer_new(0, 256, 0, 1);
  ksize=GWEN_Crypt_Token_KeyInfo_GetKeySize(ki);
  DBG_INFO(AQEBICS_LOGDOMAIN, "keysize is: %d", ksize); 

  emBits = 8 * ksize - 1;
  emLen  = (emBits + 1) >> 3; 
  sLen = hLen;

  if (emLen < hLen + sLen + 2) {
     DBG_ERROR(AQEBICS_LOGDOMAIN, "Encoding error");
     return GWEN_ERROR_INTERNAL;
  }
  
  char salt[sLen];  
  GWEN_Crypt_Random(2, salt, sLen);
/* TEST START */
if(0) {
  for (int i=0 ; i < sLen ; i++) {
    salt[i] = i; /* WARNING - SALT IS NOT RANDOM!!! */
  }
}
/* TEST END */
  
  for (int i=0 ; i < 8 ; i++) {
    GWEN_Buffer_AppendByte(m2, (char) 0);
  }
  GWEN_Buffer_AppendBuffer(m2, xbuf);
  for (int i=0 ; i < sLen ; i++) {
    GWEN_Buffer_AppendByte(m2, salt[i]);
  }
  
  hbuf=GWEN_Buffer_new(0, 40, 0, 1);

  DBG_INFO(AQEBICS_LOGDOMAIN, "m2 size is: %d", GWEN_Buffer_GetUsedBytes(m2)); 
  GWEN_Buffer_Rewind(m2);
  for(int i=0; i<GWEN_Buffer_GetUsedBytes(m2);i++) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "m2 pos %d is : %02hhx", i, GWEN_Buffer_ReadByte(m2));
  }

  EBC_Eu_Sha256((uint8_t*) GWEN_Buffer_GetStart(m2), GWEN_Buffer_GetUsedBytes(m2), hbuf);
  
  int psLen = emLen - sLen - hLen - 2;
  ps=GWEN_Buffer_new(0, psLen + 8, 0, 1);
  for (int i=0 ; i < psLen ; i++) {  
    GWEN_Buffer_AppendByte(ps, (char) 0);
  }
  GWEN_Buffer_AppendByte(ps, (char) 1);
  for (int i=0 ; i < sLen ; i++) {
    GWEN_Buffer_AppendByte(ps, salt[i]);
  }

  maskLen = emLen - hLen - 1;

//GWEN_BUFFER mgf1(GWEN_BUFFER mgfSeed, int maskLen)
  
  tbuf=GWEN_Buffer_new(0, 40, 0, 1);
  DBG_INFO(AQEBICS_LOGDOMAIN, "maskLen is: %d", maskLen);
  count = (int)ceil((double)maskLen / (double)hLen);
  DBG_INFO(AQEBICS_LOGDOMAIN, "count is: %d", count);
  for (uint32_t i = 0; i < count; i++) {
    GWEN_BUFFER *mgfhashbuf;
    GWEN_BUFFER *h2buf;
    h2buf=GWEN_Buffer_new(0, 40, 0, 1);
    mgfhashbuf=GWEN_Buffer_new(0, 40, 0, 1);
    GWEN_Buffer_AppendBuffer(h2buf, hbuf);
    DBG_INFO(AQEBICS_LOGDOMAIN, "h2buf size1 is: %d", GWEN_Buffer_GetUsedBytes(h2buf));
    GWEN_Buffer_AppendByte(h2buf, i>>24 & 255);
    GWEN_Buffer_AppendByte(h2buf, i>>16 & 255);
    GWEN_Buffer_AppendByte(h2buf, i>>8  & 255);
    GWEN_Buffer_AppendByte(h2buf, i     & 255);
    DBG_INFO(AQEBICS_LOGDOMAIN, "h2buf size2 is: %d", GWEN_Buffer_GetUsedBytes(h2buf));
    EBC_Eu_Sha256((uint8_t*) GWEN_Buffer_GetStart(h2buf), GWEN_Buffer_GetUsedBytes(h2buf), mgfhashbuf);
    DBG_INFO(AQEBICS_LOGDOMAIN, "mgfhashbuf size is: %d", GWEN_Buffer_GetUsedBytes(mgfhashbuf));
    GWEN_Buffer_AppendBuffer(tbuf, mgfhashbuf);
    GWEN_Buffer_free(h2buf);
    GWEN_Buffer_free(mgfhashbuf);
  }
  dbMask=GWEN_Buffer_new(0, 40, 0, 1);
  GWEN_Buffer_AppendBuffer(dbMask, tbuf);
  DBG_INFO(AQEBICS_LOGDOMAIN, "ps (db) size is: %d", GWEN_Buffer_GetUsedBytes(ps));
  DBG_INFO(AQEBICS_LOGDOMAIN, "dbMask  size is: %d", GWEN_Buffer_GetUsedBytes(dbMask));
  GWEN_Buffer_Rewind(ps);
  GWEN_Buffer_Rewind(dbMask);
  maskedDB=GWEN_Buffer_new(0, 40, 0, 1);
  for(int i=0; i<GWEN_Buffer_GetUsedBytes(ps);i++) {
    char mask=GWEN_Buffer_ReadByte(ps) ^ GWEN_Buffer_ReadByte(dbMask);   
    DBG_INFO(AQEBICS_LOGDOMAIN, "maskedDB pos %d is : %02hhx", i, mask);
    GWEN_Buffer_AppendByte(maskedDB, (char) mask);
  }
  GWEN_Buffer_Rewind(maskedDB);
  temp =(char) ~(0xFF << (emBits & 7)) & GWEN_Buffer_PeekByte(maskedDB);
  maskbyte[0] = temp;

  GWEN_Buffer_ReplaceBytes(maskedDB, 1, maskbyte, 1);
  DBG_INFO(AQEBICS_LOGDOMAIN, "sbuf size is: %d (pos %d)", GWEN_Buffer_GetUsedBytes(sbuf), GWEN_Buffer_GetPos(sbuf));
  GWEN_Buffer_AppendBuffer(sbuf, maskedDB);
  GWEN_Buffer_AppendBuffer(sbuf, hbuf);      
  GWEN_Buffer_AppendByte(sbuf, 0xbc);
  DBG_INFO(AQEBICS_LOGDOMAIN, "sbuf size is: %d (pos %d)", GWEN_Buffer_GetUsedBytes(sbuf), GWEN_Buffer_GetPos(sbuf));
 
  GWEN_Buffer_free(xbuf);
  GWEN_Buffer_free(hbuf);
  GWEN_Buffer_free(m2);
  GWEN_Buffer_free(ps);
  GWEN_Buffer_free(dbMask);
  GWEN_Buffer_free(maskedDB);

  return 0;
}


int EBC_Provider_EuVerifySign_A006(AB_PROVIDER *pro,
                             AB_USER *u,
                             const char *requestType,
                             const uint8_t *pMsg,
                             uint32_t lMsg,
                             GWEN_BUFFER *sbuf)
{
  EBC_PROVIDER *dp;
  GWEN_BUFFER *xbuf;
  GWEN_BUFFER *hbuf;
  GWEN_BUFFER *m2;
  GWEN_BUFFER *tbuf;
  GWEN_BUFFER *dbMask;
  GWEN_BUFFER *maskedDB;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  const GWEN_CRYPT_TOKEN_KEYINFO *ki;
  uint32_t keyId;
  int rv;
  int hLen = 32; /*SHA256*/
  int emBits;
  int emLen;
  int sLen;
  int maskLen;
  char temp;
  char maskbyte[1];
  int count;
  
  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  /* get crypt token and context */
  rv=EBC_Provider_MountToken(pro, u, &ct, &ctx);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* get key id */
  keyId=GWEN_Crypt_Token_Context_GetSignKeyId(ctx);
  ki=GWEN_Crypt_Token_GetKeyInfo(ct,
                                 keyId,
                                 0xffffffff,
                                 0);
  if (ki==NULL) {
    DBG_INFO(AQEBICS_LOGDOMAIN,
             "Keyinfo %04x not found on crypt token [%s:%s]",
             keyId,
             GWEN_Crypt_Token_GetTypeName(ct),
             GWEN_Crypt_Token_GetTokenName(ct));
    GWEN_Crypt_Token_Close(ct, 0, 0);
    return GWEN_ERROR_NOT_FOUND;
  }

  /* Verify */
  emBits = 8 * GWEN_Crypt_Token_KeyInfo_GetKeySize(ki) - 1;
  emLen = (emBits + 7) >> 3; 
  sLen = hLen;
  char salt[sLen];  
  
  xbuf=GWEN_Buffer_new(0, 40, 0, 1);
  EBC_Eu_Sha256(pMsg, lMsg, xbuf);
  if (emLen < hLen + sLen + 2) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "emsaPssVerifyError: emLen %d <  hLen %d  + sLen %d + 2", emLen, hLen, sLen);
    return GWEN_ERROR_INTERNAL;
  }
   
  GWEN_Buffer_SetPos(sbuf, GWEN_Buffer_GetUsedBytes(sbuf)-1);
  if (GWEN_Buffer_PeekByte(sbuf) != 0xBC) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "emsaPssVerifyError: last Byte (%x) is not 0xbc", GWEN_Buffer_ReadByte(sbuf));
    return GWEN_ERROR_INTERNAL;
  }
  
  GWEN_Buffer_Rewind(sbuf);
  maskedDB=GWEN_Buffer_new(0, 40, 0, 1);;
  for(int i=0; i<GWEN_Buffer_GetUsedBytes(sbuf)-hLen-1; i++) {
    GWEN_Buffer_AppendByte(maskedDB, GWEN_Buffer_ReadByte(sbuf));
  }
  hbuf=GWEN_Buffer_new(0, 40, 0, 1);
  for(int i=0; i<hLen; i++) {
    GWEN_Buffer_AppendByte(hbuf, GWEN_Buffer_ReadByte(sbuf));
  }
  DBG_INFO(AQEBICS_LOGDOMAIN, "emBits; %d", emBits);
  temp = (char) (0xFF << (emBits & 7));
  GWEN_Buffer_Rewind(maskedDB);
  if ((~GWEN_Buffer_PeekByte(maskedDB) & temp) != temp) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "maskedDB Byte 0 mismatch: temp: %02hhx vs byte 0: %02hhx", temp, GWEN_Buffer_ReadByte(maskedDB));
    return GWEN_ERROR_INTERNAL;
  }

  maskLen = GWEN_Buffer_GetUsedBytes(sbuf) - hLen - 1;
  DBG_INFO(AQEBICS_LOGDOMAIN, "verify maskLen is: %d", maskLen);
  count = (int)ceil((double)maskLen / (double)hLen);
  DBG_INFO(AQEBICS_LOGDOMAIN, "verify count is: %d", count);

  dbMask=GWEN_Buffer_new(0, 40, 0, 1);
  for (uint32_t i = 0; i < count; i++) {
    GWEN_BUFFER *mgfhashbuf;
    GWEN_BUFFER *h2buf;
    h2buf=GWEN_Buffer_new(0, 40, 0, 1);
    mgfhashbuf=GWEN_Buffer_new(0, 40, 0, 1);
    GWEN_Buffer_AppendBuffer(h2buf, hbuf);
    DBG_INFO(AQEBICS_LOGDOMAIN, "h2buf size1 is: %d", GWEN_Buffer_GetUsedBytes(h2buf));
    GWEN_Buffer_AppendByte(h2buf, i>>24 & 255);
    GWEN_Buffer_AppendByte(h2buf, i>>16 & 255);
    GWEN_Buffer_AppendByte(h2buf, i>>8  & 255);
    GWEN_Buffer_AppendByte(h2buf, i     & 255);
    DBG_INFO(AQEBICS_LOGDOMAIN, "h2buf size2 is: %d", GWEN_Buffer_GetUsedBytes(h2buf));
    EBC_Eu_Sha256((uint8_t*) GWEN_Buffer_GetStart(h2buf), GWEN_Buffer_GetUsedBytes(h2buf), mgfhashbuf);
    DBG_INFO(AQEBICS_LOGDOMAIN, "mgfhashbuf size is: %d", GWEN_Buffer_GetUsedBytes(mgfhashbuf));
    GWEN_Buffer_AppendBuffer(dbMask, mgfhashbuf);
    GWEN_Buffer_free(h2buf);
    GWEN_Buffer_free(mgfhashbuf);
  }
/*
    private function mgf1($mgfSeed, $maskLen)
    {
        // if $maskLen would yield strings larger than 4GB, PKCS#1 suggests a "Mask too long" error be output.

        $t = '';
        $count = ceil($maskLen / $this->mgfHLen);
        for ($i = 0; $i < $count; $i++) {
            $c = pack('N', $i);
            $t .= $this->mgfHash->hash($mgfSeed . $c);
        }

        return substr($t, 0, $maskLen);
    }
*/
  DBG_INFO(AQEBICS_LOGDOMAIN, "mgfhashbuf size is: %d and maskLen is: %d", GWEN_Buffer_GetUsedBytes(dbMask), maskLen);

  GWEN_Buffer_Rewind(dbMask);
  GWEN_Buffer_Rewind(maskedDB);
  tbuf=GWEN_Buffer_new(0, 40, 0, 1);
  for(int i=0; i<GWEN_Buffer_GetUsedBytes(maskedDB);i++) {
    char mask=GWEN_Buffer_ReadByte(maskedDB) ^ GWEN_Buffer_ReadByte(dbMask);   
    DBG_INFO(AQEBICS_LOGDOMAIN, "tbuf pos %d is: %02hhx", i, mask);
    GWEN_Buffer_AppendByte(tbuf, (char) mask);
  }
  GWEN_Buffer_Rewind(tbuf);
  maskbyte[0] = (char) ~(0xFF << (emBits & 7)) & GWEN_Buffer_PeekByte(tbuf);
  GWEN_Buffer_ReplaceBytes(tbuf, 1, maskbyte, 1);
  GWEN_Buffer_Rewind(tbuf);
  for (int i = 0; i<GWEN_Buffer_GetUsedBytes(sbuf) - hLen - sLen - 2; i++) {
    int byte=GWEN_Buffer_ReadByte(tbuf);
    if (byte != 0) {
       DBG_ERROR(AQEBICS_LOGDOMAIN, "tbuf at pos %d is not 0x00: %02hhx", i, byte);
       return GWEN_ERROR_INTERNAL;
    }
  }
  if (GWEN_Buffer_PeekByte(tbuf) != 1) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "dbMask Byte is not 0x01: %02hhx", GWEN_Buffer_ReadByte(tbuf));
    return GWEN_ERROR_INTERNAL;
  }
  GWEN_Buffer_ReadByte(tbuf);

  m2=GWEN_Buffer_new(0, 40, 0, 1);
  for (int i=0 ; i < 8 ; i++) {
    GWEN_Buffer_AppendByte(m2, (char) 0);
  }
  GWEN_Buffer_AppendBuffer(m2, xbuf);
  DBG_INFO(AQEBICS_LOGDOMAIN, "rest is: %d ()should be 32", GWEN_Buffer_GetUsedBytes(tbuf) - (GWEN_Buffer_GetUsedBytes(sbuf) - hLen - sLen - 2 + 1));
  for (int i = 0; i<GWEN_Buffer_GetUsedBytes(tbuf) - (GWEN_Buffer_GetUsedBytes(sbuf) - hLen - sLen - 2 + 1) ; i++) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "salt[%d] was %02hhx", i, salt[i]);
    salt[i]=GWEN_Buffer_ReadByte(tbuf);
    DBG_INFO(AQEBICS_LOGDOMAIN, "salt[%d] set to %02hhx", i, salt[i]);
  }
  for (int i = 0 ; i<sLen ; i++) {
    GWEN_Buffer_AppendByte(m2, salt[i]);
  }
  DBG_INFO(AQEBICS_LOGDOMAIN, "m2 size is: %d", GWEN_Buffer_GetUsedBytes(m2)); 
  GWEN_Buffer_Reset(tbuf);
  EBC_Eu_Sha256((uint8_t*) GWEN_Buffer_GetStart(m2), GWEN_Buffer_GetUsedBytes(m2), tbuf);
  GWEN_Buffer_Rewind(hbuf);
  GWEN_Buffer_Rewind(tbuf);
  DBG_INFO(AQEBICS_LOGDOMAIN, "hbuf size is: %d", GWEN_Buffer_GetUsedBytes(hbuf)); 
  DBG_INFO(AQEBICS_LOGDOMAIN, "tbuf size is: %d", GWEN_Buffer_GetUsedBytes(tbuf)); 
  for (int i=0 ; i<GWEN_Buffer_GetUsedBytes(hbuf) ; i++) {
    int h=GWEN_Buffer_ReadByte(hbuf);
    int t=GWEN_Buffer_ReadByte(tbuf);
    if (h!=t) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "hash1 (%02hhx) != hash2 (%02hhx)  at pos %d", h, t, i);
      return GWEN_ERROR_INTERNAL;
    }      
  }
     
  GWEN_Buffer_free(xbuf);
  GWEN_Buffer_free(hbuf);
  GWEN_Buffer_free(tbuf);
  GWEN_Buffer_free(m2);
  GWEN_Buffer_free(dbMask);
  GWEN_Buffer_free(maskedDB);
/*
    public function emsaPssVerify($m, $em, $emBits = null)
    {

        $emLen = ($emBits + 7) >> 3; // ie. ceil($emBits / 8);
        $sLen = $this->sLen !== null ? $this->sLen : $this->hLen;

        $mHash = $this->hash->hash($m);
        if ($emLen < $this->hLen + $sLen + 2) {
            return false;
        }

        if ($em[strlen($em) - 1] != chr(0xBC)) {
            return false;
        }

        $maskedDB = substr($em, 0, -$this->hLen - 1);
        $h = substr($em, -$this->hLen - 1, $this->hLen);
        $temp = chr(0xFF << ($emBits & 7));
        if ((~$maskedDB[0] & $temp) != $temp) {
            return false;
        }

        $dbMask = $this->mgf1($h, $emLen - $this->hLen - 1);

        $db = $maskedDB ^ $dbMask;
        $db[0] = ~chr(0xFF << ($emBits & 7)) & $db[0];
        $temp = $emLen - $this->hLen - $sLen - 2;
        if (substr($db, 0, $temp) != str_repeat(chr(0), $temp) || ord($db[$temp]) != 1) {
            return false;
        }
        $salt = substr($db, $temp + 1); // should be $sLen long
        $m2 = "\0\0\0\0\0\0\0\0" . $mHash . $salt;
        $h2 = $this->hash->hash($m2);
        return $this->equals($h, $h2);
    }
*/
  DBG_INFO(AQEBICS_LOGDOMAIN, "verified correct signature");
  return 0;
}



int EBC_Provider_MkEuZipDoc_A006(AB_PROVIDER *pro,
                                 AB_USER *u,
                                 const char *requestType,
                                 const uint8_t *pMsg,
                                 uint32_t lMsg,
                                 GWEN_BUFFER *sbuf)
{
  int rv;
  xmlDocPtr doc;
  xmlNodePtr root_node;
  xmlNodePtr node;
  xmlNsPtr ns;
  GWEN_BUFFER *tbuf;
  GWEN_BUFFER *bbuf;

/* TEST START*/
if(0) {
  lMsg=1774;
  FILE * f = fopen ("/tmp/message.txt", "rb");
  if (f) {
    fseek (f, 0, SEEK_END);
    int length = ftell (f);
    fseek (f, 0, SEEK_SET);
    pMsg = malloc (length);
    if (pMsg) {
      fread (pMsg, 1, length, f);
    }
    fclose (f);
  }
}
/* TEST END */

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=EBC_Provider_EuSign_A006(pro, u, requestType, pMsg, lMsg, tbuf);
  if (rv) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  rv=EBC_Provider_EuVerifySign_A006(pro, u, requestType, pMsg, lMsg, tbuf);
  if (rv) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "signature failure (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  bbuf=GWEN_Buffer_new(0, (GWEN_Buffer_GetUsedBytes(tbuf)*3)/2, 0, 1);
  rv=GWEN_Base64_Encode((const uint8_t *)GWEN_Buffer_GetStart(tbuf),
                        GWEN_Buffer_GetUsedBytes(tbuf),
                        bbuf, 0);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(bbuf);
    GWEN_Buffer_free(tbuf);
    return rv;
  }
  GWEN_Buffer_free(tbuf);

  doc=xmlNewDoc(BAD_CAST "1.0");

  root_node=xmlNewNode(NULL, BAD_CAST "UserSignatureData");
  xmlDocSetRootElement(doc, root_node);
  ns=xmlNewNs(root_node,
              BAD_CAST "http://www.ebics.org/S001",
              NULL);
  assert(ns);
  ns=xmlNewNs(root_node,
              BAD_CAST "http://www.w3.org/2001/XMLSchema-instance",
              BAD_CAST "xsi");
  xmlNewNsProp(root_node,
               ns,
               BAD_CAST "schemaLocation", /* xsi:schemaLocation */
               BAD_CAST "http://www.ebics.org/S001 "
               "http://www.ebics.org/S001/ebics_signature.xsd");

  node=xmlNewChild(root_node, NULL, BAD_CAST "OrderSignatureData", NULL);

  xmlNewTextChild(node,
                  NULL,
                  BAD_CAST "SignatureVersion",
                  BAD_CAST "A006");

  xmlNewTextChild(node,
                  NULL,
                  BAD_CAST "SignatureValue",
                  BAD_CAST GWEN_Buffer_GetStart(bbuf));
  GWEN_Buffer_free(bbuf);

  xmlNewTextChild(node,
                  NULL,
                  BAD_CAST "PartnerID",
                  BAD_CAST AB_User_GetCustomerId(u));

  xmlNewTextChild(node,
                  NULL,
                  BAD_CAST "UserID",
                  BAD_CAST AB_User_GetUserId(u));

  
  rv=EB_Xml_CompressDoc(doc, sbuf);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    xmlFreeDoc(doc);
    return rv;
  }

/* TEST START */
if (0 && GWEN_Logger_GetLevel(AQEBICS_LOGDOMAIN)>=GWEN_LoggerLevel_Debug) {
  DBG_ERROR(AQEBICS_LOGDOMAIN, "Message:\n");
  GWEN_BUFFER *mbuf = GWEN_Buffer_new(0, (GWEN_Buffer_GetUsedBytes(sbuf)*3)/4, 0, 1);
  GWEN_Buffer_ReserveBytes(mbuf, lMsg+8);
  GWEN_Buffer_AppendBytes(mbuf, pMsg, lMsg);
  GWEN_Buffer_Dump(mbuf, 2);
  if (1) {
    FILE *f;

    f=fopen("/tmp/message.txt", "w+");
    if (f) {
      fwrite((const char *)GWEN_Buffer_GetStart(tbuf), GWEN_Buffer_GetUsedBytes(tbuf), 1, f);
      fclose(f);
    }
    else {
      DBG_ERROR(0, "Could not create file");
      assert(0);
    }
  }
  GWEN_Buffer_Reset(mbuf);

  DBG_ERROR(AQEBICS_LOGDOMAIN, "Signature:\n");
  GWEN_Buffer_Dump(sbuf, 2);
  DBG_ERROR(AQEBICS_LOGDOMAIN, "Message:\n");
  GWEN_Buffer_Rewind(sbuf);
  rv=EB_Zip_Inflate(GWEN_Buffer_GetStart(sbuf), GWEN_Buffer_GetUsedBytes(sbuf), mbuf);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not unzip doc (%d)", rv);
    GWEN_Buffer_free(mbuf);
    return -1;
  }
  GWEN_Buffer_Dump(mbuf, 2);
  if (1) {
    FILE *f;

    f=fopen("/tmp/signature.txt", "w+");
    if (f) {
      fwrite((const char *)GWEN_Buffer_GetStart(mbuf), GWEN_Buffer_GetUsedBytes(mbuf), 1, f);
      fclose(f);
    }
    else {
      DBG_ERROR(0, "Could not create file");
      assert(0);
    }
  }
}
/* TEST END */
  xmlFreeDoc(doc);

  return 0;
}



int EBC_Provider_MkEuCryptZipDoc_A006(AB_PROVIDER *pro,
                                      AB_USER *u,
                                      const char *requestType,
                                      const uint8_t *pMsg,
                                      uint32_t lMsg,
                                      GWEN_CRYPT_KEY *skey,
                                      GWEN_BUFFER *sbuf)
{
  GWEN_BUFFER *tbuf;
  GWEN_BUFFER *ebuf;
  int rv;
  uint32_t l;

  DBG_INFO(AQEBICS_LOGDOMAIN, "Generating EU A006");
  tbuf=GWEN_Buffer_new(0, 512, 0, 1);
  rv=EBC_Provider_MkEuZipDoc_A006(pro, u, requestType, pMsg, lMsg, tbuf);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  /* padd EU */
  rv=GWEN_Padd_PaddWithAnsiX9_23ToMultipleOf(tbuf, 16);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  /* encrypt EU with the DES session key */
  ebuf=GWEN_Buffer_new(0, GWEN_Buffer_GetUsedBytes(tbuf)+16, 0, 1);
  l=GWEN_Buffer_GetMaxUnsegmentedWrite(ebuf);
  /* reset IV !! */
  GWEN_Crypt_KeyAes128_SetIV(skey, NULL, 0);
  rv=GWEN_Crypt_Key_Encipher(skey,
                             (uint8_t *)GWEN_Buffer_GetStart(tbuf),
                             GWEN_Buffer_GetUsedBytes(tbuf),
                             (uint8_t *)GWEN_Buffer_GetPosPointer(ebuf),
                             &l);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(ebuf);
    GWEN_Buffer_free(tbuf);
    return rv;
  }
  GWEN_Buffer_free(tbuf);
  GWEN_Buffer_IncrementPos(ebuf, l);
  GWEN_Buffer_AdjustUsedBytes(ebuf);

  /* base64 encode encrypted EU into given buffer */
  rv=GWEN_Base64_Encode((const uint8_t *)GWEN_Buffer_GetStart(ebuf),
                        GWEN_Buffer_GetUsedBytes(ebuf),
                        sbuf, 0);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(ebuf);
    return rv;
  }
  GWEN_Buffer_free(ebuf);
  DBG_ERROR(AQEBICS_LOGDOMAIN, "STOP HERE!");
  return GWEN_ERROR_INTERNAL;
  return 0;
}



