/***************************************************************************
    begin       : Thu May 15 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



static int EBC_Provider_EncryptData_E001(GWEN_UNUSED AB_PROVIDER *pro,
					 GWEN_CRYPT_KEY *skey,
					 const uint8_t *pData,
					 uint32_t lData,
					 GWEN_BUFFER *sbuf) {
  GWEN_BUFFER *tbuf;
  GWEN_BUFFER *ebuf;
  int rv;
  uint32_t l;

  /* zip */
  tbuf=GWEN_Buffer_new(0, lData, 0, 1);
  rv=EB_Zip_Deflate((const char*)pData, lData, tbuf);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  /* padd */
  rv=GWEN_Padd_PaddWithAnsiX9_23(tbuf);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  /* encrypt */
  ebuf=GWEN_Buffer_new(0, GWEN_Buffer_GetUsedBytes(tbuf)+16, 0, 1);
  l=GWEN_Buffer_GetMaxUnsegmentedWrite(ebuf);

  /* reset IV !! */
  GWEN_Crypt_KeyDes3K_SetIV(skey, NULL, 0);
  rv=GWEN_Crypt_Key_Encipher(skey,
			     (uint8_t*)GWEN_Buffer_GetStart(tbuf),
			     GWEN_Buffer_GetUsedBytes(tbuf),
			     (uint8_t*)GWEN_Buffer_GetPosPointer(ebuf),
			     &l);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(ebuf);
    GWEN_Buffer_free(tbuf);
    return rv;
  }
  GWEN_Buffer_IncrementPos(ebuf, l);
  GWEN_Buffer_AdjustUsedBytes(ebuf);

  GWEN_Buffer_free(tbuf);

  /* base64 encode encrypted data into given buffer */
  rv=GWEN_Base64_Encode((const uint8_t*)GWEN_Buffer_GetStart(ebuf),
			GWEN_Buffer_GetUsedBytes(ebuf),
			sbuf, 0);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(ebuf);
    return rv;
  }
  GWEN_Buffer_free(ebuf);

  return 0;

}




static int EBC_Provider_EncryptKey_E001(AB_PROVIDER *pro,
					AB_USER *u,
					const GWEN_CRYPT_KEY *skey,
					GWEN_BUFFER *sbuf) {
  EBC_PROVIDER *dp;
  GWEN_BUFFER *kbuf;
  GWEN_BUFFER *ebuf;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  const GWEN_CRYPT_TOKEN_KEYINFO *ki;
  uint32_t keyId;
  int ksize;
  uint32_t l;
  GWEN_CRYPT_PADDALGO *algo;
  int rv;

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
  keyId=GWEN_Crypt_Token_Context_GetEncipherKeyId(ctx);
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

  ksize=GWEN_Crypt_Token_KeyInfo_GetKeySize(ki);

  /* get key data */
  kbuf=GWEN_Buffer_new(0, 32, 0, 1);
  GWEN_Buffer_AppendBytes(kbuf,
			  (const char*)GWEN_Crypt_KeyDes3K_GetKeyDataPtr(skey),
                          16);

  /* select padd algo */
  algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_Pkcs1_2);
  GWEN_Crypt_PaddAlgo_SetPaddSize(algo, ksize);

  /* encipher key data */
  ebuf=GWEN_Buffer_new(0, ksize+16, 0, 1);
  l=GWEN_Buffer_GetMaxUnsegmentedWrite(ebuf);
  rv=GWEN_Crypt_Token_Encipher(ct, keyId,
			       algo,
			       (const uint8_t*)GWEN_Buffer_GetStart(kbuf),
			       GWEN_Buffer_GetUsedBytes(kbuf),
			       (uint8_t*)GWEN_Buffer_GetStart(ebuf),
			       &l,
			       0);
  GWEN_Crypt_PaddAlgo_free(algo);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(ebuf);
    GWEN_Buffer_free(kbuf);
    return rv;
  }
  GWEN_Buffer_IncrementPos(ebuf, l);
  GWEN_Buffer_AdjustUsedBytes(ebuf);

  GWEN_Buffer_free(kbuf);

  /* base64 encode encrypted data into given buffer */
  rv=GWEN_Base64_Encode((const uint8_t*)GWEN_Buffer_GetStart(ebuf),
			GWEN_Buffer_GetUsedBytes(ebuf),
			sbuf, 0);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(ebuf);
    return rv;
  }
  GWEN_Buffer_free(ebuf);

  return 0;
}




