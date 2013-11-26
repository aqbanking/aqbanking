/***************************************************************************
    begin       : Wed May 14 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include <gwenhywfar/mdigest.h>
#include "msg/eu.h"



int EBC_Provider_EuSign_A004(AB_PROVIDER *pro,
			     AB_USER *u,
			     const char *requestType,
			     const uint8_t *pMsg,
			     uint32_t lMsg,
			     GWEN_BUFFER *sbuf) {
  EBC_PROVIDER *dp;
  GWEN_MDIGEST *md;
  GWEN_BUFFER *hbuf;
  GWEN_BUFFER *ebuf;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  const GWEN_CRYPT_TOKEN_KEYINFO *ki;
  uint32_t keyId;
  int ksize;
  uint32_t l;
  GWEN_CRYPT_PADDALGO *algo;
  EB_EU *eu;
  GWEN_TIME *ti;
  int rv;
  const char *userId;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  userId=AB_User_GetUserId(u);

  md=GWEN_MDigest_Rmd160_new();
  assert(md);
  rv=GWEN_MDigest_Begin(md);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    return rv;
  }
  rv=GWEN_MDigest_Update(md, pMsg, lMsg);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    return rv;
  }
  rv=GWEN_MDigest_End(md);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    return rv;
  }

  hbuf=GWEN_Buffer_new(0, GWEN_MDigest_GetDigestSize(md), 0, 1);
  GWEN_Buffer_AppendBytes(hbuf,
			  (const char*)GWEN_MDigest_GetDigestPtr(md),
			  GWEN_MDigest_GetDigestSize(md));
  GWEN_MDigest_free(md);

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
    GWEN_Buffer_free(hbuf);
    return GWEN_ERROR_NOT_FOUND;
  }

  ksize=GWEN_Crypt_Token_KeyInfo_GetKeySize(ki);

  /* select padd algo */
  algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_Iso9796_2);
  GWEN_Crypt_PaddAlgo_SetPaddSize(algo, ksize);

  /* actually sign */
  ebuf=GWEN_Buffer_new(0, ksize+16, 0, 1);
  l=GWEN_Buffer_GetMaxUnsegmentedWrite(ebuf);
  rv=GWEN_Crypt_Token_Sign(ct, keyId,
			   algo,
			   (const uint8_t*)GWEN_Buffer_GetStart(hbuf),
			   GWEN_Buffer_GetUsedBytes(hbuf),
			   (uint8_t*)GWEN_Buffer_GetPosPointer(ebuf),
			   &l,
			   NULL, /* ignore seq counter */
			   0);
  GWEN_Crypt_PaddAlgo_free(algo);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(ebuf);
    GWEN_Buffer_free(hbuf);
    return rv;
  }
  GWEN_Buffer_IncrementPos(ebuf, l);
  GWEN_Buffer_AdjustUsedBytes(ebuf);

  GWEN_Buffer_free(hbuf);

  /* assemble EU */
  eu=EB_Eu_new();
  EB_Eu_SetVersion(eu, "A004");
  EB_Eu_SetModLen(eu, ksize*8);
  EB_Eu_SetJobType(eu, requestType);
  EB_Eu_SetSignature(eu,
		     (const uint8_t*) GWEN_Buffer_GetStart(ebuf),
		     GWEN_Buffer_GetUsedBytes(ebuf));
  GWEN_Buffer_free(ebuf);
  ti=GWEN_CurrentTime();
  EB_Eu_SetCreationTime(eu, ti);
  EB_Eu_SetSignatureTime(eu, ti);
  GWEN_Time_free(ti);

  EB_Eu_SetUserId(eu, userId);

  /* store EU in given buffer */
  EB_Eu_toBuffer(eu, sbuf);
  EB_Eu_free(eu);

  return 0;
}



int EBC_Provider_MkEuZipDoc_A004(AB_PROVIDER *pro,
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
  rv=EBC_Provider_EuSign_A004(pro, u, requestType, pMsg, lMsg, tbuf);
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
	      BAD_CAST "http://www.ebics.org/H002",
	      NULL);
  assert(ns);
  ns=xmlNewNs(root_node,
	      BAD_CAST "http://www.w3.org/2001/XMLSchema-instance",
	      BAD_CAST "xsi");
  xmlNewNsProp(root_node,
	       ns,
	       BAD_CAST "schemaLocation", /* xsi:schemaLocation */
	       BAD_CAST "http://www.ebics.org/H002 "
	       "http://www.ebics.org/H002/ebics_orders.xsd");

  node=xmlNewTextChild(root_node, NULL,
		       BAD_CAST "OrderSignature",
		       BAD_CAST GWEN_Buffer_GetStart(bbuf));
  GWEN_Buffer_free(bbuf);

  xmlNewProp(node,
	     BAD_CAST "PartnerID",
	     BAD_CAST AB_User_GetCustomerId(u));


  rv=EB_Xml_CompressDoc(doc, sbuf);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    xmlFreeDoc(doc);
    return rv;
  }

  xmlFreeDoc(doc);


  return 0;
}



int EBC_Provider_MkEuCryptZipDoc_A004(AB_PROVIDER *pro,
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
  rv=EBC_Provider_MkEuZipDoc_A004(pro, u, requestType, pMsg, lMsg, tbuf);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  /* padd EU */
  rv=GWEN_Padd_PaddWithAnsiX9_23(tbuf);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  /* encrypt EU with the DES session key */
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






