/***************************************************************************
    begin       : Tue Jun 08 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



int EBC_Provider_EuSign_A005(AB_PROVIDER *pro,
			     AB_USER *u,
			     const char *requestType,
			     const uint8_t *pMsg,
			     uint32_t lMsg,
			     GWEN_BUFFER *sbuf) {
  EBC_PROVIDER *dp;
  GWEN_BUFFER *xbuf;
  GWEN_BUFFER *hbuf;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  const GWEN_CRYPT_TOKEN_KEYINFO *ki;
  uint32_t keyId;
  int ksize;
  uint32_t l;
  GWEN_CRYPT_PADDALGO *algo;
  int rv;
  const uint8_t prefix[]={
    0x30, 0x31, 0x30, 0x0d,
    0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01,
    0x05, 0x00,
    0x04, 0x20};

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
  EBC_Provider_Sha256(pMsg, lMsg, xbuf);

  /* add prefix to hash of SignedInfo */
  hbuf=GWEN_Buffer_new(0, 256, 0, 1);
  ksize=GWEN_Crypt_Token_KeyInfo_GetKeySize(ki);
  GWEN_Buffer_AppendBytes(hbuf, (const char*)prefix, sizeof(prefix));
  GWEN_Buffer_AppendBytes(hbuf, GWEN_Buffer_GetStart(xbuf), GWEN_Buffer_GetUsedBytes(xbuf));
  GWEN_Buffer_free(xbuf);

  /* select padd algo */
  algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_Pkcs1_2);
  GWEN_Crypt_PaddAlgo_SetPaddSize(algo, ksize);

  /* actually sign */
  GWEN_Buffer_AllocRoom(sbuf, ksize+16);
  l=GWEN_Buffer_GetMaxUnsegmentedWrite(sbuf);
  rv=GWEN_Crypt_Token_Sign(ct, keyId,
			   algo,
			   (const uint8_t*)GWEN_Buffer_GetStart(hbuf),
			   GWEN_Buffer_GetUsedBytes(hbuf),
			   (uint8_t*)GWEN_Buffer_GetPosPointer(sbuf),
			   &l,
			   NULL, /* ignore seq counter */
			   0);
  GWEN_Crypt_PaddAlgo_free(algo);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(hbuf);
    return rv;
  }
  GWEN_Buffer_IncrementPos(sbuf, l);
  GWEN_Buffer_AdjustUsedBytes(sbuf);

  GWEN_Buffer_free(hbuf);

  return 0;
}




int EBC_Provider_MkEuZipDoc_A005(AB_PROVIDER *pro,
				 AB_USER *u,
				 const char *requestType,
				 const uint8_t *pMsg,
				 uint32_t lMsg,
				 GWEN_BUFFER *sbuf) {
  int rv;
  xmlDocPtr doc;
  xmlNodePtr root_node;
  xmlNodePtr node;
  xmlNsPtr ns;
  GWEN_BUFFER *tbuf;
  GWEN_BUFFER *bbuf;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=EBC_Provider_EuSign_A005(pro, u, requestType, pMsg, lMsg, tbuf);
  if (rv) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  bbuf=GWEN_Buffer_new(0, (GWEN_Buffer_GetUsedBytes(tbuf)*3)/2, 0, 1);
  rv=GWEN_Base64_Encode((const uint8_t*)GWEN_Buffer_GetStart(tbuf),
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
		  BAD_CAST "A005");

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

  xmlFreeDoc(doc);


  return 0;
}



int EBC_Provider_MkEuCryptZipDoc_A005(AB_PROVIDER *pro,
				      AB_USER *u,
				      const char *requestType,
				      const uint8_t *pMsg,
				      uint32_t lMsg,
				      GWEN_CRYPT_KEY *skey,
				      GWEN_BUFFER *sbuf) {
  GWEN_BUFFER *tbuf;
  GWEN_BUFFER *ebuf;
  int rv;
  uint32_t l;

  DBG_INFO(AQEBICS_LOGDOMAIN, "Generating EU A005");
  tbuf=GWEN_Buffer_new(0, 512, 0, 1);
  rv=EBC_Provider_MkEuZipDoc_A005(pro, u, requestType, pMsg, lMsg, tbuf);
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
  GWEN_Buffer_free(tbuf);
  GWEN_Buffer_IncrementPos(ebuf, l);
  GWEN_Buffer_AdjustUsedBytes(ebuf);

  /* base64 encode encrypted EU into given buffer */
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



