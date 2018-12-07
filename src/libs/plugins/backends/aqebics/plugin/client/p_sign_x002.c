/***************************************************************************
    begin       : Tue Mar 04 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/




static int EBC_Provider_SignMessage_X002(AB_PROVIDER *pro,
					 EB_MSG *msg,
					 AB_USER *u,
					 xmlNodePtr node) {
  EBC_PROVIDER *dp;
  int rv;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  const GWEN_CRYPT_TOKEN_KEYINFO *ki;
  uint32_t keyId;
  GWEN_BUFFER *hbuf;
  GWEN_BUFFER *bbuf;
  xmlNodePtr nodeX = NULL;
  xmlNodePtr nodeXX = NULL;
  xmlNodePtr nodeXXX = NULL;
  xmlNodePtr nodeXXXX = NULL;
  xmlNsPtr ns;

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
  keyId=GWEN_Crypt_Token_Context_GetAuthSignKeyId(ctx);
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

  /* prepare signature nodes */
  ns=xmlSearchNs(EB_Msg_GetDoc(msg), node, BAD_CAST "ds");
  assert(ns);

  /* build hash */
  bbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=EB_Msg_BuildHashSha256(msg, bbuf);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not build hash");
    GWEN_Buffer_free(bbuf);
    return rv;
  }

  /* base64 encode */
  hbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_Base64_Encode((const uint8_t*)GWEN_Buffer_GetStart(bbuf),
			GWEN_Buffer_GetUsedBytes(bbuf),
			hbuf, 0);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(hbuf);
    GWEN_Buffer_free(bbuf);
    return rv;
  }
  GWEN_Buffer_free(bbuf);

  /* create signature node */
  nodeX=xmlNewChild(node, ns, BAD_CAST "SignedInfo", NULL);
  nodeXX=xmlNewChild(nodeX, ns, BAD_CAST "CanonicalizationMethod", NULL);
  xmlNewProp(nodeXX,
	     BAD_CAST "Algorithm",
	     BAD_CAST "http://www.w3.org/TR/2001/REC-xml-c14n-20010315");
  nodeXX=xmlNewChild(nodeX, ns, BAD_CAST "SignatureMethod", NULL);
  xmlNewProp(nodeXX,
	     BAD_CAST "Algorithm",
	     BAD_CAST "http://www.w3.org/2001/04/xmldsig-more#rsa-sha256");
  nodeXX=xmlNewChild(nodeX, ns, BAD_CAST "Reference", NULL);
  xmlNewProp(nodeXX,
	     BAD_CAST "URI",
	     BAD_CAST "#xpointer(//*[@authenticate='true'])");
  nodeXXX=xmlNewChild(nodeXX, ns, BAD_CAST "Transforms", NULL);
  nodeXXXX=xmlNewChild(nodeXXX, ns, BAD_CAST "Transform", NULL);
  xmlNewProp(nodeXXXX,
	     BAD_CAST "Algorithm",
	     BAD_CAST "http://www.w3.org/TR/2001/REC-xml-c14n-20010315");

  nodeXXX=xmlNewChild(nodeXX, ns, BAD_CAST "DigestMethod", NULL);
  xmlNewProp(nodeXXX,
	     BAD_CAST "Algorithm",
	     BAD_CAST "http://www.w3.org/2001/04/xmlenc#sha256");

  /* store hash value */
  xmlNewTextChild(nodeXX, ns,
		  BAD_CAST "DigestValue",
		  BAD_CAST GWEN_Buffer_GetStart(hbuf));
  GWEN_Buffer_free(hbuf);

  /* build hash over SignedInfo */
  bbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=EB_Xml_BuildNodeHashSha256(nodeX, "#xpointer(//*)", bbuf);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(bbuf);
    return rv;
  }

  /* sign hash */
  if (1) {
    GWEN_CRYPT_PADDALGO *algo;
    int ksize;
    uint32_t l;
    const uint8_t prefix[]={
      0x30, 0x31, 0x30, 0x0d,
      0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01,
      0x05, 0x00,
      0x04, 0x20};

    /* add prefix to hash of SignedInfo */
    hbuf=GWEN_Buffer_new(0, 256, 0, 1);
    ksize=GWEN_Crypt_Token_KeyInfo_GetKeySize(ki);
    GWEN_Buffer_AppendBytes(hbuf, (const char*)prefix, sizeof(prefix));
    GWEN_Buffer_AppendBuffer(hbuf, bbuf);
    GWEN_Buffer_Reset(bbuf);

    /* select padd algo */
    algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_Pkcs1_1);
    GWEN_Crypt_PaddAlgo_SetPaddSize(algo, ksize);

    /* actually sign */
    GWEN_Buffer_AllocRoom(bbuf, ksize+16);
    l=GWEN_Buffer_GetMaxUnsegmentedWrite(bbuf);
    rv=GWEN_Crypt_Token_Sign(ct, keyId,
			     algo,
			     (const uint8_t*)GWEN_Buffer_GetStart(hbuf),
			     GWEN_Buffer_GetUsedBytes(hbuf),
			     (uint8_t*)GWEN_Buffer_GetPosPointer(bbuf),
			     &l,
			     NULL, /* ignore seq counter */
			     0);
    GWEN_Crypt_PaddAlgo_free(algo);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(bbuf);
      GWEN_Buffer_free(hbuf);
      return rv;
    }
    GWEN_Buffer_IncrementPos(bbuf, l);
    GWEN_Buffer_AdjustUsedBytes(bbuf);

    /* base 64 encode signature */
    GWEN_Buffer_Reset(hbuf);
    rv=GWEN_Base64_Encode((const uint8_t*)GWEN_Buffer_GetStart(bbuf),
			  GWEN_Buffer_GetUsedBytes(bbuf),
			  hbuf, 0);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(hbuf);
      GWEN_Buffer_free(bbuf);
      return rv;
    }
    GWEN_Buffer_free(bbuf);

    /* store signature */
    xmlNewTextChild(node, ns,
		    BAD_CAST "SignatureValue",
		    BAD_CAST GWEN_Buffer_GetStart(hbuf));
    GWEN_Buffer_free(hbuf);
  }

  return 0;
}




